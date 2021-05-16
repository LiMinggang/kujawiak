// Copyright (C) 2012  Augustino (augustino@users.sourceforge.net)
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

#include "ku.h"

BEGIN_EVENT_TABLE(kuGenericDirCtrl,wxGenericDirCtrl)
    EVT_TREE_SEL_CHANGED(wxID_ANY,kuGenericDirCtrl::OnTreeSelChanged)
    EVT_TREE_BEGIN_DRAG(wxID_ANY,kuGenericDirCtrl::OnTreeBeginDrag)
    EVT_SET_FOCUS(kuGenericDirCtrl::OnSetFocus)
    EVT_CONTEXT_MENU(kuGenericDirCtrl::OnContextMenu)
END_EVENT_TABLE()

// -------- kuGenericDirCtrl --------
kuGenericDirCtrl::kuGenericDirCtrl(wxWindow* parent, kuFrame* frame)
    :wxGenericDirCtrl(parent,wxID_ANY,wxDirDialogDefaultFolderStr,wxDefaultPosition,wxDefaultSize,
                      wxSUNKEN_BORDER|wxDIRCTRL_SHOW_FILTERS,
                      STRING_FILTER_STANDARD
                      + wxString::Format(wxT("(%s)"),kuFiWrapper::GetSupportedExtensions().AfterFirst('|').wc_str())
                      + kuFiWrapper::GetSupportedExtensions()
                      + STRING_FILTER_ARCHIVE + STRING_FILTER_ALLFILES) {
    mFrame = frame;
    mIsUNC = false;
    SetupPopupMenu();

	GetTreeCtrl()->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNFACE));
    ShowHidden(true);
    // connect filter command
    GetFilterListCtrl()->Connect(wxEVT_COMMAND_CHOICE_SELECTED,
                                 wxCommandEventHandler(kuGenericDirCtrl::OnChoice));
    bool setdef = true;
    for(size_t i=0; i<wxGetApp().mCmds.GetCount(); i++) {
        if(wxGetApp().mCmds[i].Lower().StartsWith(CMD_LOCATE)) {
            setdef = false;
            break;
        }
    }
    if(setdef)    SetDefaultPath(wxGetApp().mOptions.mFavorite);
}

void kuGenericDirCtrl::SetupPopupMenu() {
    /* don't delete it manually
    if(mMenu)   delete mMenu;
    */
    mMenu = new wxMenu();
    mMenu->Append(kuID_FILE_CUT,    STRING_MENU_FILE_CUT);
    mMenu->Append(kuID_FILE_COPY,   STRING_MENU_FILE_COPY);
    mMenu->Append(kuID_FILE_PASTE,  STRING_MENU_FILE_PASTE);
    mMenu->Append(kuID_FILE_DELETE, STRING_MENU_FILE_DELETE);
    mMenu->AppendSeparator();
    mMenu->Append(kuID_FILE_MOVETO, STRING_MENU_FILE_MOVETO);
    mMenu->Append(kuID_FILE_COPYTO, STRING_MENU_FILE_COPYTO);
    mMenu->AppendSeparator();
    mMenu->Append(kuID_MKDIR,       STRING_MENU_MKDIR);
    mMenu->Append(kuID_RENAME,      STRING_MENU_RENAME);
    mMenu->Append(kuID_SAVEAS,      STRING_MENU_SAVEAS);
    mMenu->AppendSeparator();
    mMenu->Append(kuID_PAGESETUP,   STRING_MENU_PAGESETUP);
    mMenu->Append(kuID_PREVIEW,     STRING_MENU_PREVIEW);
    mMenu->Append(kuID_PRINT,       STRING_MENU_PRINT);
    #ifdef __WXMSW__
    mMenu->AppendSeparator();
    mMenu->Append(kuID_EXPLORE,     STRING_MENU_EXPLORE);
    mMenu->AppendSeparator();
    mMenu->Append(kuID_PROPERTIES,  STRING_MENU_PROPERTIES);
    #endif
    mMenu->Append(kuID_METADATA,    STRING_MENU_METADATA);
}

void kuGenericDirCtrl::OnTreeSelChanged(wxTreeEvent& event) {
    mIsUNC = false;
    //wxMessageBox(wxString::Format(wxT("old = %d"), event.GetOldItem().m_pItem));
    // will receive twice on SelectItem(), one of them doesn't have old item
    if(event.GetOldItem().m_pItem == 0)    return;
    if(GetFilterIndex()==INDEX_FILTER_STANDARD
       || (GetFilterIndex()==INDEX_FILTER_ALLFILES&&!mFrame->mIsArchive)) {
        if(GetFilePath()==wxEmptyString) {
            wxGetApp().SetSaveAs(false);
            
            if(mFrame->mIsThumbnail) {
                if(wxGetApp().GetBusy())   mFrame->SetStatusText(STRING_ERROR_BUSY);
                else   mFrame->mMultiple->ReloadThumbs(GetPath(),false);
            }
            
        }
        else {
            if(!wxGetApp().mIsExploring)    mFrame->mSingle->ReloadImage(GetFilePath(),false);
            wxGetApp().SetSaveAs(true);
            // load siblings
            if(wxGetApp().mOptions.mPrefetch)    PrefetchSiblings();
        }
    }
    else   // enumerate to mVirtual
        mFrame->mVirtual->SetRoot(GetFilePath());
    // keep history
    mFrame->HistoryTracing(GetFilePath());
}

void kuGenericDirCtrl::PrefetchSiblings() {
    wxGetApp().mLoader->Clear();
    wxSize size = wxGetApp().mOptions.mLoadCompletely ? wxSize(0,0) : mFrame->mSingle->GetClientSize();
    int rotate = 0;
    if(wxGetApp().mOptions.mKeepRotate)    rotate = wxGetApp().mOptions.mRotate;
    wxTreeItemId nid = GetTreeCtrl()->GetSelection();
    if(nid.IsOk())
	for(int i=0; i<wxGetApp().mOptions.mLoadSiblings[0]; i++) {
	    nid = GetTreeCtrl()->GetNextSibling(nid);
	    if(!nid.IsOk())    break;
	    wxDirItemData* data = (wxDirItemData*)GetTreeCtrl()->GetItemData(nid);
	    //wxMessageBox(wxT("GenericDirCtrl: ")+data->m_path);
	    wxGetApp().mLoader->Append(data->m_path, false, size, rotate);
	}
    wxTreeItemId pid = GetTreeCtrl()->GetSelection();
    if(pid.IsOk())
	for(int i=0; i<wxGetApp().mOptions.mLoadSiblings[1]; i++) {
	    pid = GetTreeCtrl()->GetPrevSibling(pid);
	    if(!pid.IsOk())    break;
	    wxDirItemData* data = (wxDirItemData*)GetTreeCtrl()->GetItemData(pid);
	    //wxMessageBox(wxT("GenericDirCtrl: ")+data->m_path);
	    wxGetApp().mLoader->Append(data->m_path, false, size, rotate);
	}
    if(nid.IsOk())
	for(int i=0; i<wxGetApp().mOptions.mLoadSiblings[2]; i++) {
	    nid = GetTreeCtrl()->GetNextSibling(nid);
	    if(!nid.IsOk())    break;
	    wxDirItemData* data = (wxDirItemData*)GetTreeCtrl()->GetItemData(nid);
	    //wxMessageBox(wxT("GenericDirCtrl: ")+data->m_path);
	    wxGetApp().mLoader->Append(data->m_path, false, size, rotate);
	}
    if(pid.IsOk())
	for(int i=0; i<wxGetApp().mOptions.mLoadSiblings[3]; i++) {
	    pid = GetTreeCtrl()->GetPrevSibling(pid);
	    if(!pid.IsOk())    break;
	    wxDirItemData* data = (wxDirItemData*)GetTreeCtrl()->GetItemData(pid);
	    //wxMessageBox(wxT("GenericDirCtrl: ")+data->m_path);
	    wxGetApp().mLoader->Append(data->m_path, false, size, rotate);
	}
}

void kuGenericDirCtrl::OnTreeBeginDrag(wxTreeEvent& event) {
    DoDragDrop(event);
    event.Skip();
}

void kuGenericDirCtrl::OnSetFocus(wxFocusEvent& event) {
    // pass focus to its treectrl
    GetTreeCtrl()->SetFocus();
}

void kuGenericDirCtrl::OnChoice(wxCommandEvent& event) {
    switch (event.GetSelection()) {
        case INDEX_FILTER_STANDARD:
            wxGetApp().mFrame->ToggleArchive(false);
            break;
        case INDEX_FILTER_ARCHIVE:
            wxGetApp().mFrame->ToggleArchive(true);
            break;
    }
    event.Skip();
}

void kuGenericDirCtrl::OnContextMenu(wxContextMenuEvent& event) {
    PopupMenu(mMenu);
}

bool kuGenericDirCtrl::Locate(wxString location) {
    // check if UNC path
    wxChar sep = wxFileName::GetPathSeparator();
    if(kuApp::IsUNCPath(location)) {    // UNC path
        mIsUNC = true;
        wxString host = location.Mid(2).BeforeFirst(sep);
        wxString dir  = location.Mid(2).AfterFirst(sep).BeforeFirst(sep);
        wxString path = wxString::Format(wxT("%c%c%s%c%s"), sep, sep, host.wc_str(), sep, dir.wc_str());
        wxBeginBusyCursor();
        AddSection(path, path, wxFileIconsTable::computer);
        wxEndBusyCursor();
        wxSafeYield();
    }
    // initial
    SetPath(location);
    if(GetFilePath()==wxEmptyString) {   // directory
        // select first child
        wxTreeItemId id=GetTreeCtrl()->GetFirstChild(GetTreeCtrl()->GetSelection(),*(new wxTreeItemIdValue()));
        while(true) {
            // no other children
            if(!id.IsOk()) {
                SetPath(location);
                return false;
            }
            GetTreeCtrl()->SelectItem(id,true);
            // if child is a file
            if(GetFilePath()!=wxEmptyString)   break;
            // goto next child
            id=GetTreeCtrl()->GetNextSibling(id);
        }
    }
    return true;
}

wxArrayString* kuGenericDirCtrl::EnumerateChildren(wxString dirname) {
    // initial
    GetTreeCtrl()->Expand(GetTreeCtrl()->GetSelection());
    wxArrayString* children=NULL;
    if(GetFilePath()==wxEmptyString) {   // if it is directory
        children = new wxArrayString();
        wxTreeItemId id=GetTreeCtrl()->GetFirstChild(GetTreeCtrl()->GetSelection(),*(new wxTreeItemIdValue()));
        while(true) {
            // no other children
            if(!id.IsOk())   break;
            // if child is a file
            if(!((wxDirItemData*)GetTreeCtrl()->GetItemData(id))->m_isDir)
                children->Add(GetTreeCtrl()->GetItemText(id));
            // goto next child
            id=GetTreeCtrl()->GetNextSibling(id);
        }
    }
    return children;
}

wxString kuGenericDirCtrl::GetNeighbor() {
    wxTreeItemId current = GetTreeCtrl()->GetSelection();
    wxTreeItemId id;
    id = GetTreeCtrl()->GetNextSibling(current);
    if(!id.IsOk()) {
        id = GetTreeCtrl()->GetPrevSibling(current);
        if(!id.IsOk()) {
            id = GetTreeCtrl()->GetItemParent(current);
            if(!id.IsOk())    return wxEmptyString;
        }
    }
    return ((wxDirItemData*)GetTreeCtrl()->GetItemData(id))->m_path;
}


wxString kuGenericDirCtrl::GetDir() {
    if(GetFilePath()==wxEmptyString)    // is dir or item is invalid
        return GetPath();
    wxTreeItemId current = GetTreeCtrl()->GetSelection();
    wxTreeItemId parent = GetTreeCtrl()->GetItemParent(current);
    if(parent==GetRootId() || !parent.IsOk())    // no parent or parent is invalid
        return wxEmptyString;
    wxDirItemData* data = (wxDirItemData*) GetTreeCtrl()->GetItemData(parent);
    return data->m_path;
}

void kuGenericDirCtrl::Reload(bool parent) {
    wxTreeItemId id;
    if(GetFilePath()!=wxEmptyString || parent)   // is file or update parent
        id=GetTreeCtrl()->GetItemParent(GetTreeCtrl()->GetSelection());
    else   // id dir
        id=GetTreeCtrl()->GetSelection();
    CollapseDir(id);
    ExpandDir(id);
}

void kuGenericDirCtrl::SwitchFilter(int index) {
    wxString current=GetPath();
    GetFilterListCtrl()->SetSelection(index);
    SetFilterIndex(index);
    ReCreateTree();
    #ifdef __WXMSW__
    AddShortcuts();
    #endif
    ExpandPath(current);
}

void kuGenericDirCtrl::AddShortcuts() {
    // add home directory
    //AddSection(wxGetHomeDir(),STRING_SHORTCUT_HOME,wxFileIconsTable::folder);
}

void kuGenericDirCtrl::DoDragDrop(wxTreeEvent& event) {
    if(mFrame->mIsArchive)    return;    // not supported
    wxTreeItemId id = event.GetItem();
    if(!id)    return;
    wxDirItemData* item = (wxDirItemData*) GetTreeCtrl()->GetItemData(id);
    wxDropSource source(this);
    wxFileDataObject data;
    data.AddFile(item->m_path);
    source.SetData(data);
    source.DoDragDrop();
}

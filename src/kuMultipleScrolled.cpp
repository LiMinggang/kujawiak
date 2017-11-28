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

BEGIN_EVENT_TABLE(kuMultipleScrolled,wxScrolledWindow)
    EVT_KEY_DOWN(kuMultipleScrolled::OnKeyDown)
    EVT_LEFT_DOWN(kuMultipleScrolled::OnLeftDown)
    EVT_SIZE(kuMultipleScrolled::OnSize)
    EVT_CONTEXT_MENU(kuMultipleScrolled::OnContextMenu)
    EVT_MENU(wxID_DELETE,kuMultipleScrolled::OnDeleteThumb)
    EVT_MENU(wxID_CLEAR,kuMultipleScrolled::OnClearThumbs)
    EVT_MENU(wxID_FILE1,kuMultipleScrolled::OnAll)
    EVT_MENU(wxID_FILE2,kuMultipleScrolled::OnNone)
    EVT_MENU(wxID_FILE3,kuMultipleScrolled::OnInverse)
END_EVENT_TABLE()

// -------- kuMultipleScrolled --------
kuMultipleScrolled::kuMultipleScrolled(wxWindow* parent, kuFrame* frame)
     :wxScrolledWindow(parent,wxID_ANY,wxDefaultPosition,wxDefaultSize,wxVSCROLL|wxSUNKEN_BORDER) {
    SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_APPWORKSPACE));
    mFrame=frame;
    mFilenames.Clear();
    mSelections.Clear();
    SetupPopupMenu();
    SetSizer(new wxGridSizer(0));
    SetDropTarget(new kuScrolledDropTarget(mFrame));
}

void kuMultipleScrolled::SetupPopupMenu() {
    /* don't delete it manually
    if(mMenu)   delete mMenu;
    */
    mMenu = new wxMenu();
    mMenu->Append(wxID_CLEAR, STRING_MENU_THUMBS_CLEAR);
    mMenu->AppendSeparator();
    mMenu->Append(wxID_FILE1, STRING_SELBTN_ALL);
    mMenu->Append(wxID_FILE2, STRING_SELBTN_NONE);
    mMenu->Append(wxID_FILE3, STRING_SELBTN_INVERSE);
    wxWindowList list = GetChildren();
    for(size_t i=0; i<list.GetCount(); i++) {
        ((kuThumbButton*)list[i])->SetupPopupMenu();
    }
}

void kuMultipleScrolled::ReloadThumbs(wxString dirname, bool isurl) {
    if(dirname == mDirname)    return;
    mDirname = dirname;
    //wxTreeCtrl* tree=mFrame->mGeneric->GetTreeCtrl();
    //if(tree->GetItemParent(tree->GetSelection())==tree->GetRootItem())   return;
    wxArrayString* children;
    if(isurl)    children = mFrame->mVirtual->EnumerateChildren(dirname);
    else    children = mFrame->mGeneric->EnumerateChildren(dirname);
    if(children==NULL)   return;
    // delete old
    DestroyChildren();
    // add new
    int cols=GetSize().x/(THUMBNAIL_WIDTH+10);
    ((wxGridSizer*)GetSizer())->SetCols(cols);
    if(cols) ((wxGridSizer*)GetSizer())->SetRows(0);
    else ((wxGridSizer*)GetSizer())->SetRows(1);
    wxString dir;
    if(isurl)    dir = mFrame->mVirtual->GetFilePath(true,false)+wxFileName::GetPathSeparator();
    else {
        dir=mFrame->mGeneric->GetPath();
        if(dir.Last()!=wxFileName::GetPathSeparator())   dir=dir+wxFileName::GetPathSeparator();
    }
    wxGetApp().SetBusy(true);
    mFrame->SetStatusText(STRING_INFO_THUMBS);
    mFrame->mStatusBar->SetGaugeRange(children->GetCount());
    for(size_t i=0;i<children->GetCount();i++) {
        wxBeginBusyCursor();
        if(isurl)    GetSizer()->Add(new kuThumbButton(this,dir+children->Item(i),children->Item(i)));
        else    GetSizer()->Add(new kuThumbButton(this,dir+children->Item(i),wxEmptyString));
        mFilenames.Add(dir+children->Item(i));
        FitInside();
        mFrame->mStatusBar->IncrGaugeValue();
        wxGetApp().Yield();    // allow user to interrupt
        wxEndBusyCursor();
        // check interrupt
        if(wxGetApp().GetInterrupt())   break;
    }
    delete children;
    SetScrollRate(0,SCROLL_RATE_MULTIPLE);

    if(wxGetApp().GetInterrupt()) {
        if(wxGetApp().mQuit)    return;
        mFrame->SetStatusText(STRING_WARNING_INTERRUPTED);
        wxGetApp().SetInterrupt(false);
    } else   mFrame->SetStatusText(wxEmptyString);
    wxGetApp().SetBusy(false);
}

void kuMultipleScrolled::AddThumbs(wxArrayString& files) {
    // add new
    int cols=GetSize().x/(THUMBNAIL_WIDTH+10);
    ((wxGridSizer*)GetSizer())->SetCols(cols);
    if(cols) ((wxGridSizer*)GetSizer())->SetRows(0);
    else ((wxGridSizer*)GetSizer())->SetRows(1);
    wxGetApp().SetBusy(true);
    mFrame->SetStatusText(STRING_INFO_THUMBS);
    mFrame->mStatusBar->SetGaugeRange(files.GetCount());
    for(size_t i=0;i<files.GetCount();i++) {
        wxWindow* window = FindWindow(files[i]);
        if(window) {
            mFrame->mStatusBar->IncrGaugeValue();
            continue;
        }
        wxBeginBusyCursor();
        GetSizer()->Add(new kuThumbButton(this,files[i],wxEmptyString));
        mFilenames.Add(files[i]);
        FitInside();
        mFrame->mStatusBar->IncrGaugeValue();
        wxGetApp().Yield();    // allow user to interrupt
        wxEndBusyCursor();
        // check interrupt
        if(wxGetApp().GetInterrupt())   break;
    }
    SetScrollRate(0,SCROLL_RATE_MULTIPLE);

    if(wxGetApp().GetInterrupt()) {
        if(wxGetApp().mQuit)    return;
        mFrame->SetStatusText(STRING_WARNING_INTERRUPTED);
        wxGetApp().SetInterrupt(false);
    } else   mFrame->SetStatusText(wxEmptyString);
    wxGetApp().SetBusy(false);
}

void kuMultipleScrolled::RemoveThumb(wxString filename) {
    if(filename == wxEmptyString)    return;
    wxWindow* window = FindWindow(filename);
    if(!window)    return;
    mFilenames.Remove(filename);
    GetSizer()->Detach(window);
    GetSizer()->Layout();
    window->Destroy();
}

void kuMultipleScrolled::Select(wxString filename, bool ctrl, bool shift) {
    if(mFrame->mIsArchive)   // filename is url
        filename=filename.AfterLast(wxFileName::GetPathSeparator());
    if(mSelections.IsEmpty()) {
        mSelections.Add(filename);
    } else if(ctrl && shift) {
        AddSelections(mFilenames.Index(mSelections[0]), mFilenames.Index(filename));
        mSelections.Remove(filename);
        mSelections.Insert(filename, 0);
    } else if(!ctrl && shift) {
        mSelections.Clear();
        AddSelections(mFilenames.Index(mSelections[0]), mFilenames.Index(filename));
    } else if(ctrl && !shift) {
        if(mSelections.Index(filename) == wxNOT_FOUND)    mSelections.Insert(filename, 0);
        else    mSelections.Remove(filename);
    } else {
        mSelections.Clear();
        mSelections.Add(filename);
    }
    //wxMessageBox(wxString::Format(wxT("%d"), mSelections.GetCount()));
    RefreshThumbs();
}

void kuMultipleScrolled::AddSelections(int first, int last) {
    if(last < first) {
        int tmp = last;
        last = first;
        first = tmp;
    }
    for(size_t i=first; i<=last; i++) {    // TODO: avoid duplicated items
        mSelections.Add(mFilenames[i]);
    }
}

void kuMultipleScrolled::RefreshThumbs() {
    wxWindow* window;
    for(size_t i=0;i<GetChildren().GetCount();i++) {
        window = GetChildren().Item(i)->GetData();
        if(mSelections.Index(window->GetName()) == wxNOT_FOUND) {
            window->SetWindowStyle(wxBU_AUTODRAW);
        }
        else {
            window->SetWindowStyle(wxNO_BORDER);
        }
        window->Refresh();
    }
}

void kuMultipleScrolled::OnKeyDown(wxKeyEvent& event) {
    //wxMessageBox(wxString::Format(wxT("%d"),event.GetKeyCode()));
    event.Skip();
}

void kuMultipleScrolled::OnLeftDown(wxMouseEvent& event) {
    DoDragDrop();
    event.Skip();
}

void kuMultipleScrolled::OnSize(wxSizeEvent& event) {
    int cols=GetSize().x/(THUMBNAIL_WIDTH+10);
    ((wxGridSizer*)GetSizer())->SetCols(cols);
    if(cols) ((wxGridSizer*)GetSizer())->SetRows(0);
    else ((wxGridSizer*)GetSizer())->SetRows(1);
    FitInside();
    SetScrollRate(0,SCROLL_RATE_MULTIPLE);
}

void kuMultipleScrolled::OnContextMenu(wxContextMenuEvent& event) {
    PopupMenu(mMenu);
}

void kuMultipleScrolled::OnDeleteThumb(wxCommandEvent& event) {
    RemoveThumb(event.GetString());
    Refresh();
}

void kuMultipleScrolled::OnClearThumbs(wxCommandEvent& event) {
    mFilenames.Clear();
    DestroyChildren();
}

void kuMultipleScrolled::OnAll(wxCommandEvent& event) {
    mSelections = mFilenames;
    RefreshThumbs();
}

void kuMultipleScrolled::OnNone(wxCommandEvent& event) {
    mSelections.Clear();
    RefreshThumbs();
}

void kuMultipleScrolled::OnInverse(wxCommandEvent& event) {
    wxArrayString selects;
    for(size_t i=0; i<mFilenames.GetCount(); i++) {
        if(mSelections.Index(mFilenames[i]) == wxNOT_FOUND) {
            selects.Add(mFilenames[i]);
        }
    }
    mSelections = selects;
    RefreshThumbs();
}

void kuMultipleScrolled::DoDragDrop() {
    wxDropSource source(this);
    wxFileDataObject data;
    for(size_t i=0; i<mSelections.GetCount(); i++) {
        data.AddFile(mSelections[i]);
    }
    source.SetData(data);
    source.DoDragDrop();
}

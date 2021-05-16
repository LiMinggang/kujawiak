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

BEGIN_EVENT_TABLE(kuVirtualDirCtrl,wxTreeCtrl)
    EVT_TREE_SEL_CHANGED(wxID_ANY,kuVirtualDirCtrl::OnTreeSelChanged)
    //EVT_TREE_BEGIN_DRAG(wxID_ANY,kuVirtualDirCtrl::OnTreeBeginDrag)
END_EVENT_TABLE()

// -------- kuVirtualDirCtrl --------
kuVirtualDirCtrl::kuVirtualDirCtrl(wxWindow* parent, kuFrame* frame)
     :wxTreeCtrl(parent,wxID_ANY,wxDefaultPosition,wxDefaultSize,wxTR_HAS_BUTTONS|wxTR_HIDE_ROOT) {
    mFrame=frame;
    SetImageList(wxTheFileIconsTable->GetSmallImageList());
    SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNFACE));
}

void kuVirtualDirCtrl::OnTreeSelChanged(wxTreeEvent& event) {
    //wxMessageBox(wxString::Format(wxT("old = %d"), event.GetOldItem().m_pItem));
    // will receive twice on SelectItem(), one of them doesn't have old item
    // don't skip this since olditem always be 0 when open an archive
    //if(event.GetOldItem().m_pItem == 0)    return;
    if(mFrame->mIsThumbnail && GetFilePath(true,true)==wxEmptyString) {
        if(wxGetApp().GetBusy())   mFrame->SetStatusText(STRING_ERROR_BUSY);
        else   mFrame->mMultiple->ReloadThumbs(GetFilePath(true,false),true);
    }
    else if(!mIsDeleting) {   // do not show while DeleteAllItems()
        mFrame->mSingle->ReloadImage(GetFilePath(true,true),true);
        // load siblings
        if(wxGetApp().mOptions.mPrefetch)    PrefetchSiblings();
    }
}

void kuVirtualDirCtrl::PrefetchSiblings() {
    wxGetApp().mLoader->Clear();
    wxSize size = wxGetApp().mOptions.mLoadCompletely ? wxSize(0,0) : mFrame->mSingle->GetClientSize();
    int rotate = 0;
    if(wxGetApp().mOptions.mKeepRotate)    rotate = wxGetApp().mOptions.mRotate;
    wxTreeItemId nid = GetSelection();
    if(nid.IsOk())
	for(int i=0; i<wxGetApp().mOptions.mLoadSiblings[0]; i++) {
	    nid = GetNextSibling(nid);
	    if(!nid.IsOk())    break;
	    //wxMessageBox(wxT("VirtualDirCtrl: ")+GetFilePath(true,true,nid));
	    wxGetApp().mLoader->Append(GetFilePath(true,true,nid), true, size, rotate);
	}
    wxTreeItemId pid = GetSelection();
    if(pid.IsOk())
	for(int i=0; i<wxGetApp().mOptions.mLoadSiblings[1]; i++) {
	    pid = GetPrevSibling(pid);
	    if(!pid.IsOk())    break;
	    //wxMessageBox(wxT("VirtualDirCtrl: ")+GetFilePath(true,true,pid));
	    wxGetApp().mLoader->Append(GetFilePath(true,true,pid), true, size, rotate);
	}
    if(nid.IsOk())
	for(int i=0; i<wxGetApp().mOptions.mLoadSiblings[2]; i++) {
	    nid = GetNextSibling(nid);
	    if(!nid.IsOk())    break;
	    //wxMessageBox(wxT("VirtualDirCtrl: ")+GetFilePath(true,true,nid));
	    wxGetApp().mLoader->Append(GetFilePath(true,true,nid), true, size, rotate);
	}
    if(pid.IsOk())
	for(int i=0; i<wxGetApp().mOptions.mLoadSiblings[3]; i++) {
	    pid = GetPrevSibling(pid);
	    if(!pid.IsOk())    break;
	    //wxMessageBox(wxT("VirtualDirCtrl: ")+GetFilePath(true,true,pid));
	    wxGetApp().mLoader->Append(GetFilePath(true,true,pid), true, size, rotate);
	}
}
/*
void kuVirtualDirCtrl::OnTreeBeginDrag(wxTreeEvent& event) {
    //DoDragDrop();
    event.Skip();
}
*/
void kuVirtualDirCtrl::SetRoot(wxString archive) {
    mIsDeleting=true;
    mFrame->SetStatusText(STRING_INFO_DELETE_ARCHIVE);
    DeleteAllItems();
    mFrame->SetStatusText(wxEmptyString);
    mIsDeleting=false;
    if(archive==wxEmptyString)   return;
    AddRoot(archive.AfterLast(wxFileName::GetPathSeparator()),-1,-1,new wxDirItemData(archive,wxEmptyString,true));
    mArchive=wxFileSystem::FileNameToURL(wxFileName(archive));
    EnumerateArchive(archive);
}

void kuVirtualDirCtrl::EnumerateArchive(wxString archive) {
    //std::auto_ptr<wxInputStream> in(new wxFFileInputStream(archive));
    wxInputStream* in = new wxFFileInputStream(archive);
    if(!in->IsOk())    return;

    mFrame->SetStatusText(STRING_INFO_ENUMERATE_ARCHIVE);
    // look for a filter handler, e.g. for '.gz'
    const wxFilterClassFactory *fcf;
    fcf = wxFilterClassFactory::Find(archive, wxSTREAM_FILEEXT);
    if(fcf) {
        //in.reset(fcf->NewStream(in.release()));
        in = fcf->NewStream(in);
        // pop the extension, so if it was '.tar.gz' it is now just '.tar'
        archive = fcf->PopExtension(archive);
    }
    wxChar sep = wxFileName::GetPathSeparator();
    if(kuApp::IsImageFile(archive)) {   // for file like xpm.gz
        AppendItem(GetRootItem(),archive.AfterLast(sep),wxFileIconsTable::file,-1,new wxDirItemData(archive,archive.AfterLast(sep),false));
        mFrame->SetStatusText(wxEmptyString);
        return;
    }
    // look for a archive handler, e.g. for '.zip' or '.tar'
    const wxArchiveClassFactory *acf;
    acf = wxArchiveClassFactory::Find(archive, wxSTREAM_FILEEXT);
    if(acf) {
        wxString text, name;
        wxString pdir, pname;
        wxTreeItemId pid, aid;
        //std::auto_ptr<wxArchiveInputStream> arc(acf->NewStream(in.release()));
        //std::auto_ptr<wxArchiveEntry> entry;
        wxArchiveInputStream* arc = acf->NewStream(in);
        wxArchiveEntry* entry;
        // list the contents of the archive
        //while((entry.reset(arc->GetNextEntry())), entry.get() != NULL) {
        while(entry = arc->GetNextEntry()) {
            if(entry->IsDir())   continue;
            text = name = entry->GetName();
            aid = pid = GetRootItem();   // keep parent of parent
            pname = wxEmptyString;
            while((pdir=text.BeforeFirst(sep))!=wxEmptyString) {   // search down
                if(text.Find(sep)==-1)   break;   // since BeforeFirst return whole string if not found
                if(pname==wxEmptyString)    pname = pdir;
                else    pname = pname + sep + pdir;
                pid = FindTextInChildren(aid,pdir);
                if(!pid.IsOk()) {   // create parent
                    pid = AppendItem(aid,pdir,wxFileIconsTable::folder,-1,new wxDirItemData(pname,pdir,true));
                }
                aid = pid;
                text = text.AfterFirst(sep);
            }
            if(kuApp::IsImageFile(text))   // skip non-image file
                AppendItem(pid,text,wxFileIconsTable::file,-1,new wxDirItemData(name,text,false));
        }
    }
    mFrame->SetStatusText(wxEmptyString);
}

wxTreeItemId kuVirtualDirCtrl::FindTextInChildren(wxTreeItemId pid, wxString& text) {
    wxTreeItemIdValue val;
    wxTreeItemId id=GetFirstChild(pid,val);
    while(id.IsOk())  {
        if(GetItemText(id).IsSameAs(text))   break;
        id=GetNextChild(pid,val);
    }
    return id;
}

bool kuVirtualDirCtrl::Locate(wxString location) {
    wxTreeItemId id=GetSelection();
    if(!IsDir(id))   id=GetItemParent(id);
    if(!id.IsOk())   return false;
    SelectItem(FindTextInChildren(id,location),true);
    return true;
}

bool kuVirtualDirCtrl::IsDir(wxTreeItemId id) {
    if(((wxDirItemData*)GetItemData(id))->m_isDir)   return true;
    else   return false;
}

wxString kuVirtualDirCtrl::GetFilePath(bool isurl, bool fileonly, wxTreeItemId id) {
    //wxMessageBox(wxString::Format(wxT("id = %d"), id));
    if(!id)    id = GetSelection();
    if(!id.IsOk() || (fileonly&&IsDir(id)))   return wxEmptyString;
    else if(isurl) {   // return url for open
        wxString url=mArchive;
        wxString filename=((wxDirItemData*)GetItemData(id))->m_path;
        if(mFrame->mIsArchive) {
            if(url.AfterLast('.').IsSameAs(wxT("zip"),false)) {
                url = url + wxT("#zip:") + filename;
            } else if(url.AfterLast('.').IsSameAs(wxT("gz"),false)) {
                url += wxT("#gzip:");
                if(url.BeforeLast('.').AfterLast('.').IsSameAs(wxT("tar"),false))
                    url += wxT("#tar:") + filename;
            }
        }
        else   url=url+filename;
        //wxMessageBox(url);
        return url;
    }
    else   return GetItemText(id);   // return text for show
}

wxArrayString* kuVirtualDirCtrl::EnumerateChildren(wxString dirname) {
    // initial
    Expand(GetSelection());
    wxArrayString* children=NULL;
    if(GetFilePath(true,true)==wxEmptyString) {   // if it is directory
        children = new wxArrayString();
        wxTreeItemId id=GetFirstChild(GetSelection(),*(new wxTreeItemIdValue()));
        while(true) {
            // no other children
            if(!id.IsOk())   break;
            // if child is a file
            if(!((wxDirItemData*)GetItemData(id))->m_isDir)
                children->Add(GetItemText(id));
            // goto next child
            id=GetNextSibling(id);
        }
    }
    return children;
}
/*
void kuVirtualDirCtrl::DoDragDrop() {
    wxDropSource source(this);
    wxFileDataObject data;
    data.AddFile(GetFilePath(true,false));
    source.SetData(data);
    source.DoDragDrop();
}
*/

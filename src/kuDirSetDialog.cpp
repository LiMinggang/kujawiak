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

kuDirSetDialog::kuDirSetDialog(wxWindow* parent, wxString title, wxArrayString& dirs)
    :wxDialog(parent, wxID_ANY, title) {
    wxBoxSizer* topSizer = new wxBoxSizer(wxVERTICAL);
    topSizer->Add(new wxStaticText(this, wxID_ANY, STRING_DIRSET_MESSAGE), 0, wxEXPAND|wxTOP|wxLEFT|wxRIGHT, 5);

    mDirs = dirs;
    for(size_t i=0; i<mDirs.GetCount();i++) {
        mDirs[i] = wxString::Format(FORMAT_DIRSET_LISTITEM, i+1, mDirs[i]);
    }

    mListBox = new wxListBox(this, wxID_EDIT, wxDefaultPosition, wxDefaultSize, mDirs, wxLB_SINGLE|wxLB_HSCROLL|wxLB_NEEDED_SB);
    topSizer->Add(mListBox, 1, wxEXPAND|wxTOP|wxLEFT|wxRIGHT, 5);

    wxBoxSizer* btnSizer = new wxBoxSizer(wxHORIZONTAL);
    mAddBtn = new wxButton(this, wxID_ADD);
    btnSizer->Add(mAddBtn);
    mEditBtn = new wxButton(this, wxID_EDIT, kuFrame::StripCodes(STRING_MENU_EDIT));
    btnSizer->Add(mEditBtn);
    mDeleteBtn = new wxButton(this, wxID_DELETE);
    btnSizer->Add(mDeleteBtn);
    btnSizer->AddSpacer(10);
    btnSizer->Add(new wxButton(this, wxID_OK));
    btnSizer->Add(new wxButton(this, wxID_CANCEL));
    topSizer->Add(btnSizer, 0, wxALIGN_RIGHT|wxALL|wxEXPAND, 5);

    SetIcon(wxGetApp().mFrame->mIconApp);
    SetSizer(topSizer);
    Fit();
    SetSize(GetSize().x, 200);

    mAddBtn->Enable(mDirs.GetCount()<9);
    mEditBtn->Enable(mDirs.GetCount()>0);
    mDeleteBtn->Enable(mDirs.GetCount()>0);

    Connect(wxID_ADD,    wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(kuDirSetDialog::OnAdd));
    Connect(wxID_EDIT,   wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(kuDirSetDialog::OnEdit));
    Connect(wxID_DELETE, wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(kuDirSetDialog::OnDelete));
    Connect(wxID_EDIT,   wxEVT_COMMAND_LISTBOX_DOUBLECLICKED, wxCommandEventHandler(kuDirSetDialog::OnEdit));
}

wxArrayString& kuDirSetDialog::GetDirs() {
    for(size_t i=0; i<mDirs.GetCount(); i++) {
        mDirs[i] = mDirs[i].AfterFirst(' ');
    }
    return mDirs;
}

void kuDirSetDialog::OnAdd(wxCommandEvent& event) {
    wxDirDialog dialog(this);
    if(dialog.ShowModal()==wxID_CANCEL)    return;
    mDirs.Add(wxString::Format(FORMAT_DIRSET_LISTITEM, mDirs.GetCount()+1, dialog.GetPath()));
    mListBox->Append(mDirs.Last());
    mAddBtn->Enable(mDirs.GetCount()<9);
}

void kuDirSetDialog::OnEdit(wxCommandEvent& event) {
    int idx = mListBox->GetSelection();
    if(idx == wxNOT_FOUND)    return;
    wxDirDialog dialog(this);
    dialog.SetPath(mDirs[idx].AfterFirst(' '));
    if(dialog.ShowModal()==wxID_CANCEL)    return;
    mDirs[idx] = wxString::Format(FORMAT_DIRSET_LISTITEM, idx+1, dialog.GetPath());
    mListBox->SetString(idx, mDirs[idx]);
}

void kuDirSetDialog::OnDelete(wxCommandEvent& event) {
    int idx = mListBox->GetSelection();
    if(idx == wxNOT_FOUND)    return;
    mListBox->Delete(idx);
    mDirs.RemoveAt(idx);
    mEditBtn->Enable(mDirs.GetCount()>0);
    mDeleteBtn->Enable(mDirs.GetCount()>0);
}

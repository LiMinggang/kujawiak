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

kuEntryEditorDialog::kuEntryEditorDialog(wxWindow* parent, wxString title, wxString lname, wxString lvalue1, wxString lvalue2,
                                         wxArrayString& names, wxArrayString& value1s, wxArrayString& value2s)
    :wxDialog(parent, wxID_ANY, title) {
    mNames  = names;
    mValue1s = value1s;
    mValue2s = value2s;

    wxBoxSizer* topSizer = new wxBoxSizer(wxHORIZONTAL);

    // listbox and remove button
    wxBoxSizer* leftSizer = new wxBoxSizer(wxVERTICAL);
    mListBox = new wxListBox(this, wxID_EDIT, wxDefaultPosition, wxDefaultSize, names, wxLB_SINGLE|wxLB_HSCROLL|wxLB_NEEDED_SB);
    leftSizer->Add(mListBox, 1, wxEXPAND, 0);
    leftSizer->Add(new wxButton(this, wxID_DELETE), 0, wxALIGN_CENTER|wxTOP, 5);
    topSizer->Add(leftSizer, 0, wxEXPAND|wxALL, 5);

    topSizer->Add(new wxStaticLine(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_VERTICAL), 0, wxEXPAND);

    wxBoxSizer* rightSizer = new wxBoxSizer(wxVERTICAL);
    wxFlexGridSizer* entrySizer = new wxFlexGridSizer(3, 2, 5, 5);
    entrySizer->AddGrowableCol(1);
    // label and edit
    entrySizer->Add(new wxStaticText(this, wxID_ANY, lname, wxDefaultPosition, wxDefaultSize, wxALIGN_CENTRE),
                    0, wxALIGN_CENTER);
    mNameTextCtrl = new wxTextCtrl(this, wxID_ANY);
    entrySizer->Add(mNameTextCtrl, 0, wxEXPAND);
    entrySizer->Add(new wxStaticText(this, wxID_ANY, lvalue1, wxDefaultPosition, wxDefaultSize, wxALIGN_CENTRE),
                    0, wxALIGN_CENTER);
    mPathPanel = new kuPathPanel(this, false);
    entrySizer->Add(mPathPanel, 0, wxEXPAND);
    /*
    wxBoxSizer* value1Sizer = new wxBoxSizer(wxHORIZONTAL);
    mValue1TextCtrl = new wxTextCtrl(this, wxID_ANY);
    value1Sizer->Add(mValue1TextCtrl, 1, wxEXPAND);
    value1Sizer->Add(new wxBitmapButton(this, wxID_FILE, wxArtProvider::GetBitmap(wxART_FILE_OPEN)), 0, wxEXPAND|wxLEFT, 5);
    entrySizer->Add(value1Sizer, 0, wxEXPAND);
    */
    entrySizer->Add(new wxStaticText(this, wxID_ANY, lvalue2, wxDefaultPosition, wxDefaultSize, wxALIGN_CENTRE),
                    0, wxALIGN_CENTER);
    mValue2TextCtrl = new wxTextCtrl(this, wxID_ANY);
    entrySizer->Add(mValue2TextCtrl, 0, wxEXPAND);

    rightSizer->Add(entrySizer, 1, wxEXPAND);

    // add, modify, and close button
    wxBoxSizer* btnSizer = new wxBoxSizer(wxHORIZONTAL);
    btnSizer->Add(new wxButton(this, wxID_ADD));
    btnSizer->Add(new wxButton(this, wxID_REPLACE));
    btnSizer->Add(new wxButton(this, wxID_OK));
    btnSizer->Add(new wxButton(this, wxID_CANCEL));
    rightSizer->Add(btnSizer, 0, wxEXPAND|wxTOP, 5);

    topSizer->Add(rightSizer, 1, wxEXPAND|wxALL, 5);

    SetIcon(wxGetApp().mFrame->mIconApp);
    SetSizer(topSizer);
    Fit();

    // use connect instead of event table to avoid removing original binding like x button
    Connect(wxID_EDIT,    wxEVT_COMMAND_LISTBOX_SELECTED, wxCommandEventHandler(kuEntryEditorDialog::OnEdit));
    Connect(wxID_DELETE,  wxEVT_COMMAND_BUTTON_CLICKED,   wxCommandEventHandler(kuEntryEditorDialog::OnDelete));
    Connect(wxID_ADD,     wxEVT_COMMAND_BUTTON_CLICKED,   wxCommandEventHandler(kuEntryEditorDialog::OnAdd));
    Connect(wxID_REPLACE, wxEVT_COMMAND_BUTTON_CLICKED,   wxCommandEventHandler(kuEntryEditorDialog::OnReplace));
}

wxArrayString& kuEntryEditorDialog::GetNames() {
    return mNames;
}

wxArrayString& kuEntryEditorDialog::GetValue1s() {
    return mValue1s;
}

wxArrayString& kuEntryEditorDialog::GetValue2s() {
    return mValue2s;
}

void kuEntryEditorDialog::OnEdit(wxCommandEvent& event) {
    int idx = event.GetSelection();
    if(idx<0)    return;
    mNameTextCtrl->ChangeValue(mNames[idx]);
    mPathPanel->ChangePath(mValue1s[idx]);
    mValue2TextCtrl->ChangeValue(mValue2s[idx]);
}

void kuEntryEditorDialog::OnDelete(wxCommandEvent& event) {
    int idx = mListBox->GetSelection();
    mNames.RemoveAt(idx);
    mValue1s.RemoveAt(idx);
    mValue2s.RemoveAt(idx);
    mListBox->Delete(idx);
}

void kuEntryEditorDialog::OnAdd(wxCommandEvent& event) {
    wxString name = mNameTextCtrl->GetValue();
    if(name==wxEmptyString)    return;
    mNames.Add(name);
    mValue1s.Add(mPathPanel->GetPath());
    mValue2s.Add(mValue2TextCtrl->GetValue());
    mListBox->Append(name);
}

void kuEntryEditorDialog::OnReplace(wxCommandEvent& event) {
    int idx = mListBox->GetSelection();
    if(idx==wxNOT_FOUND)    return;
    wxString name = mNameTextCtrl->GetValue();
    if(name==wxEmptyString)    return;
    mNames[idx] = name;
    mValue1s[idx] = mPathPanel->GetPath();
    mValue2s[idx] = mValue2TextCtrl->GetValue();
    mListBox->SetString(idx, name);
}

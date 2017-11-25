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

kuCheckListDialog::kuCheckListDialog(wxWindow* parent, kuFrame* frame, const wxString& message, const wxString& caption, const wxArrayString& choices)
    :wxDialog(parent,wxID_ANY,caption) {
    mFrame=frame;
    wxBoxSizer* topSizer = new wxBoxSizer(wxVERTICAL);

    // message
    topSizer->Add(new wxStaticText(this,wxID_ANY,message), 0, wxALIGN_CENTER);

    // choices
    wxStaticBoxSizer* choiceSizer = new wxStaticBoxSizer(wxVERTICAL,this);
    mListBox = new wxCheckListBox(this,wxID_ANY,wxDefaultPosition,wxDefaultSize,choices);
    choiceSizer->Add(mListBox, 1, wxEXPAND);
    wxBoxSizer* selSizer = new wxBoxSizer(wxHORIZONTAL);
    selSizer->Add(new wxButton(this, kuID_CHECKDLG_ALL,     STRING_SELBTN_ALL),     0, wxEXPAND);
    selSizer->Add(new wxButton(this, kuID_CHECKDLG_NONE,    STRING_SELBTN_NONE),    0, wxEXPAND|wxLEFT, 5);
    selSizer->Add(new wxButton(this, kuID_CHECKDLG_INVERSE, STRING_SELBTN_INVERSE), 0, wxEXPAND|wxLEFT, 5);
    choiceSizer->Add(selSizer, 0, wxTOP, 5);
    topSizer->Add(choiceSizer, 1, wxEXPAND|wxALL, 5);

    // buttons
    wxSizer* btnSizer = CreateButtonSizer(wxOK|wxCANCEL);
    topSizer->Add(btnSizer, 0, wxALIGN_CENTER|wxBOTTOM, 5);
    SetSizer(topSizer);
    SetIcon(mFrame->mIconApp);
    Fit();

    Connect(kuID_CHECKDLG_ALL,     wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(kuCheckListDialog::OnAll));
    Connect(kuID_CHECKDLG_NONE,    wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(kuCheckListDialog::OnNone));
    Connect(kuID_CHECKDLG_INVERSE, wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(kuCheckListDialog::OnInverse));
}

void kuCheckListDialog::OnAll(wxCommandEvent& event) {
    for(unsigned int i=0; i<mListBox->GetCount(); i++) {
        mListBox->Check(i, true);
    }
}

void kuCheckListDialog::OnNone(wxCommandEvent& event) {
    for(unsigned int i=0; i<mListBox->GetCount(); i++) {
        mListBox->Check(i, false);
    }
}

void kuCheckListDialog::OnInverse(wxCommandEvent& event) {
    for(unsigned int i=0; i<mListBox->GetCount(); i++) {
        mListBox->Check(i, !mListBox->IsChecked(i));
    }
}

wxArrayInt kuCheckListDialog::GetSelections() {
    wxArrayInt selections;
    for(unsigned int i=0;i<mListBox->GetCount();i++) {
        if(mListBox->IsChecked(i))   selections.Add(i);
    }
    return selections;
}

void kuCheckListDialog::SetSelections(const wxArrayInt& selections) {
    for(size_t i=0;i<selections.GetCount();i++) {
        mListBox->Check(selections[i]);
    }
}

bool kuCheckListDialog::IsChecked(int index) {
    return mListBox->IsChecked(index);
}

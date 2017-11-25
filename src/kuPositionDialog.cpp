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

kuPositionDialog::kuPositionDialog(wxWindow* parent, wxString title, wxSize max, wxPoint current)
    :wxDialog(parent, wxID_ANY, title) {
    //wxMessageBox(wxString::Format(wxT("%d, %d"), current.x, current.y));
    mPosition = current;
    wxBoxSizer* topSizer = new wxBoxSizer(wxVERTICAL);
    wxFlexGridSizer* mainSizer = new wxFlexGridSizer(2, 2, 0, 0);
    mainSizer->AddGrowableCol(1);

    mainSizer->Add(new wxStaticText(this, wxID_ANY, STRING_POSITION_LEFT, wxDefaultPosition, wxDefaultSize, wxALIGN_CENTRE),
                                    0, wxALL|wxALIGN_CENTER, 5);
    wxSlider* left = new wxSlider(this, wxID_ANY, current.x, 0, max.x, wxDefaultPosition, wxDefaultSize,
                                  wxSL_HORIZONTAL|wxSL_LABELS, wxGenericValidator(&mPosition.x));
    mainSizer->Add(left, 0, wxEXPAND|wxALL|wxALIGN_CENTER, 5);

    mainSizer->Add(new wxStaticText(this, wxID_ANY, STRING_POSITION_TOP, wxDefaultPosition, wxDefaultSize, wxALIGN_CENTRE),
                                    0, wxALL|wxALIGN_CENTER, 5);
    wxSlider* top = new wxSlider(this, wxID_ANY, current.y, 0, max.y, wxDefaultPosition, wxDefaultSize,
                                 wxSL_HORIZONTAL|wxSL_LABELS, wxGenericValidator(&mPosition.y));
    mainSizer->Add(top, 0, wxEXPAND|wxALL|wxALIGN_CENTER, 5);

    topSizer->Add(mainSizer, 0, wxEXPAND);

    wxSizer* btnSizer = CreateButtonSizer(wxOK|wxCANCEL);
    topSizer->Add(btnSizer, 1, wxALIGN_CENTER|wxALL, 5);

    SetSizer(topSizer);
    Fit();
    SetSize(GetSize().x*2, GetSize().y);
}

wxPoint kuPositionDialog::GetPosition() {
    return mPosition;
}

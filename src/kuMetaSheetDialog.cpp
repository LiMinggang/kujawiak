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

kuMetaSheetDialog::kuMetaSheetDialog(wxWindow* parent, kuFrame* frame, const wxString& filename, const wxArrayString& metadata)
    :wxPropertySheetDialog(parent, wxID_ANY, wxT("Exif- ")+filename) {
    mFrame = frame;
    SetSheetStyle(wxPROPSHEET_DEFAULT);

    CreateButtons(wxOK);

    wxPanel* panel;
    int npages = metadata.GetCount()/2;
    for(int i=0; i<npages; i++) {
        panel = new wxPanel(GetBookCtrl(), wxID_ANY);
        wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);

        wxArrayString data;
        wxStringTokenizer tkz(metadata[2*i+1], METADATA_TAG_SEP);
        while(tkz.HasMoreTokens()) {
            data.Add(tkz.GetNextToken());
        }

        wxGrid* grid = new wxGrid(panel,wxID_ANY);
        grid->CreateGrid(data.GetCount(), 3);
        grid->SetColLabelValue(0, STRING_METADATA_LABEL_KEY);
        grid->SetColLabelValue(1, STRING_METADATA_LABEL_VALUE);
        grid->SetColLabelValue(2, STRING_METADATA_LABEL_DESCRIPTION);
        for(size_t j=0; j<data.GetCount(); j++) {
            wxStringTokenizer row(data[j],METADATA_COL_SEP);
            grid->SetCellValue(j, 0, row.GetNextToken());
            grid->SetCellValue(j, 1, row.GetNextToken());
            grid->SetCellValue(j, 2, row.GetNextToken());
        }
        grid->EnableEditing(false);
        grid->SetRowLabelSize(0);
        grid->AutoSize();

        //wxTextCtrl* textctrl = new wxTextCtrl(panel,wxID_ANY,metadata.Item(2*i+1),wxDefaultPosition,wxDefaultSize,wxTE_MULTILINE|wxTE_READONLY);
        sizer->Add(grid, 1, wxGROW|wxALIGN_CENTRE|wxALL, 5);
        panel->SetSizer(sizer);
        sizer->Fit(panel);
        GetBookCtrl()->AddPage(panel, metadata[2*i]);
    }

    LayoutDialog();
    SetSize(600,400);
}

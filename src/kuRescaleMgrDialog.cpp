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

kuRescaleMgrDialog::kuRescaleMgrDialog(kuFrame* frame, int id)
    :kuManagerDialog(frame, id, kuFrame::StripCodes(STRING_MENU_RESCALE_MANAGER)) {
    wxArrayInt defCols;
    defCols.Add(kuID_RESCALEMGR_COL_SRC);
    defCols.Add(kuID_RESCALEMGR_COL_DEST);
    defCols.Add(kuID_RESCALEMGR_COL_PROGRESS);
    defCols.Add(kuID_RESCALEMGR_COL_STATUS);
    wxArrayInt customCols;
    customCols.Add(kuID_RESCALEMGR_COL_SIZE);
    customCols.Add(kuID_RESCALEMGR_COL_FORMAT);
    customCols.Add(kuID_RESCALEMGR_COL_FILTER);
    wxArrayString customHeaders;
    customHeaders.Add(STRING_MANAGER_SIZE);
    customHeaders.Add(STRING_MANAGER_FORMAT);
    customHeaders.Add(STRING_MANAGER_FILTER);
    kuManagerDialog::Set(wxGetApp().mOptions.mRescaleMgrMaxCon, defCols, customCols, customHeaders);
}

kuManagedTask* kuRescaleMgrDialog::CreateTask() {
    kuRescaleTaskDialog dlg(this, wxID_ANY, kuFrame::StripCodes(STRING_TASKBTN_NEW), mFrame->mGeneric->GetDir(), wxEmptyString);
    if(dlg.ShowModal()!=wxID_OK)    return NULL;
    else    return dlg.GetTask();
}

wxThread* kuRescaleMgrDialog::CreateThread(kuManagedTask* task) {
    return new kuRescaleThread((kuRescaleTask*)task, this);
}

wxString kuRescaleMgrDialog::GetTaskInfo(kuManagedTask* task, int col) {
    kuRescaleTask* rtask = (kuRescaleTask*)task;
    wxString str = wxEmptyString;
    switch (col) {
        case kuID_RESCALEMGR_COL_SIZE:
            str = wxString::Format(FORMAT_MANAGER_SIZE, rtask->Size.x, rtask->Size.y);
            break;
        case kuID_RESCALEMGR_COL_FORMAT:
            str = rtask->Format.Upper();
            break;
        case kuID_RESCALEMGR_COL_FILTER:
            str = GetFilterString(rtask->Filter);
            break;
        default:
            break;
    }
    //wxMessageBox(wxString::Format(wxT("%d:%s"), col, str));
    return str;
}

wxString kuRescaleMgrDialog::GetErrorInfo(int errcode) {
    wxString errstr = wxEmptyString;
    switch (errcode) {
        case THREAD_ERROR_RESCALEFAIL:
            errstr = STRING_THREADERR_RESCALEFAIL;
            break;
        default:
            errstr = kuManagerDialog::GetErrorInfo(errcode);
            break;
    }
    return errstr;
}

// -------- kuRescaleTaskDialog --------
kuRescaleTaskDialog::kuRescaleTaskDialog(wxWindow* parent, wxWindowID id, wxString title, wxString src, wxString dest)
    :wxDialog(parent, id, title, wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER) {

    wxArrayString sizes;
    sizes.Add(wxString::Format(FORMAT_MANAGER_SIZE,  320,  240));
    sizes.Add(wxString::Format(FORMAT_MANAGER_SIZE,  480,  360));
    sizes.Add(wxString::Format(FORMAT_MANAGER_SIZE,  640,  480));
    sizes.Add(wxString::Format(FORMAT_MANAGER_SIZE,  800,  600));
    sizes.Add(wxString::Format(FORMAT_MANAGER_SIZE, 1024,  768));
    sizes.Add(wxString::Format(FORMAT_MANAGER_SIZE, 1280,  960));
    sizes.Add(wxString::Format(FORMAT_MANAGER_SIZE, 1400, 1050));
    sizes.Add(wxString::Format(FORMAT_MANAGER_SIZE, 1600, 1200));
    sizes.Add(wxString::Format(FORMAT_MANAGER_SIZE, 1280, 1024));
    sizes.Add(wxString::Format(FORMAT_MANAGER_SIZE, 1280,  800));
    sizes.Add(wxString::Format(FORMAT_MANAGER_SIZE, 1440,  900));
    sizes.Add(wxString::Format(FORMAT_MANAGER_SIZE, 1680, 1050));
    sizes.Add(wxString::Format(FORMAT_MANAGER_SIZE, 1920, 1200));
    sizes.Add(STRING_MANAGER_CUSTOM);
    wxString defsizestr = wxString::Format(FORMAT_MANAGER_SIZE,  640,  480);
    int defsize = mPrevSize = sizes.Index(defsizestr);
    wxArrayString formats;
    kuFiWrapper::GetSupportedExtensions(&formats, true);
    wxBeginBusyCursor();
    kuFiWrapper::GetAllSupportedFiles(src, &mFiles, &formats);
    wxEndBusyCursor();
    int defformat = formats.Index(wxT("JPG"));
    wxArrayString filters;
    filters.Add(kuFrame::StripCodes(STRING_MENU_RESCALE_BOX));
    filters.Add(kuFrame::StripCodes(STRING_MENU_RESCALE_BICUBIC));
    filters.Add(kuFrame::StripCodes(STRING_MENU_RESCALE_BILINEAR));
    filters.Add(kuFrame::StripCodes(STRING_MENU_RESCALE_BSPLINE));
    filters.Add(kuFrame::StripCodes(STRING_MENU_RESCALE_CATMULLROM));
    filters.Add(kuFrame::StripCodes(STRING_MENU_RESCALE_LANCZOS3));
    int deffilter = (int)wxGetApp().mOptions.mFilter;
    if(dest==wxEmptyString)    dest = wxFileName(src,defsizestr).GetFullPath();

    wxBoxSizer* topSizer = new wxBoxSizer(wxVERTICAL);
    wxBoxSizer* panelSizer = new wxBoxSizer(wxHORIZONTAL);

    // left panel: src
    wxStaticBoxSizer* leftSizer = new wxStaticBoxSizer(wxVERTICAL, this, STRING_MANAGER_SRC);
    wxBoxSizer* srcSizer = new wxBoxSizer(wxHORIZONTAL);
    mSrcTextCtrl = new wxTextCtrl(this, wxID_ANY, src, wxDefaultPosition, wxDefaultSize, wxTE_READONLY|wxTE_PROCESS_ENTER);
    srcSizer->Add(mSrcTextCtrl, 1, wxEXPAND);
    srcSizer->Add(new wxBitmapButton(this, wxID_OPEN, wxArtProvider::GetBitmap(wxART_FILE_OPEN)), 0, wxEXPAND|wxLEFT, 5);
    leftSizer->Add(srcSizer, 0, wxEXPAND);
    mFileCheckListBox = new wxCheckListBox(this, wxID_ANY, wxDefaultPosition, wxSize(wxDefaultSize.x, 10));    // set small min_height for align rightSizer
    for(size_t i=0; i<mFiles.GetCount(); i++) {
        mFileCheckListBox->Append(wxFileName(mFiles[i]).GetFullName());
    }
    leftSizer->Add(mFileCheckListBox, 1, wxEXPAND|wxTOP, 5);
    wxBoxSizer* selSizer = new wxBoxSizer(wxHORIZONTAL);
    selSizer->Add(new wxButton(this, kuID_RESCALEDLG_ALL,     STRING_SELBTN_ALL),     0, wxEXPAND);
    selSizer->Add(new wxButton(this, kuID_RESCALEDLG_NONE,    STRING_SELBTN_NONE),    0, wxEXPAND|wxLEFT, 5);
    selSizer->Add(new wxButton(this, kuID_RESCALEDLG_INVERSE, STRING_SELBTN_INVERSE), 0, wxEXPAND|wxLEFT, 5);
    leftSizer->Add(selSizer, 0, wxALIGN_CENTER|wxTOP, 5);

    // right panel: size, format.filter, dest
    wxBoxSizer* rightSizer = new wxBoxSizer(wxVERTICAL);
    rightSizer->Add(new wxStaticText(this, wxID_ANY, STRING_MANAGER_SIZE, wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT),
                    0, wxEXPAND);
    mSizeComboBox = new wxComboBox(this, kuID_RESCALEDLG_SIZE, sizes[defsize], wxDefaultPosition, wxDefaultSize, sizes);
    rightSizer->Add(mSizeComboBox, 0, wxEXPAND);
    rightSizer->Add(new wxStaticText(this, wxID_ANY, STRING_MANAGER_FORMAT, wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT),
                    0, wxEXPAND|wxTOP, 5);
    mFormatComboBox = new wxComboBox(this, wxID_ANY, formats[defformat], wxDefaultPosition, wxDefaultSize, formats);
    rightSizer->Add(mFormatComboBox, 0, wxEXPAND);
    rightSizer->Add(new wxStaticText(this, wxID_ANY, STRING_MANAGER_FILTER, wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT),
                    0, wxEXPAND|wxTOP, 5);
    mFilterComboBox = new wxComboBox(this, wxID_ANY, filters[deffilter], wxDefaultPosition, wxDefaultSize, filters);
    rightSizer->Add(mFilterComboBox, 0, wxEXPAND);
    rightSizer->Add(new wxStaticText(this, wxID_ANY, STRING_MANAGER_DEST, wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT),
                    0, wxEXPAND|wxTOP, 5);
    wxBoxSizer* destSizer = new wxBoxSizer(wxHORIZONTAL);
    mDestTextCtrl = new wxTextCtrl(this, kuID_RESCALEDLG_DEST, dest, wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER);
    destSizer->Add(mDestTextCtrl, 1, wxEXPAND);
    destSizer->Add(new wxBitmapButton(this, wxID_SAVE, wxArtProvider::GetBitmap(wxART_FILE_OPEN)), 0, wxEXPAND|wxLEFT, 5);
    rightSizer->Add(destSizer, 0, wxEXPAND);
    wxArrayString choices;
    choices.Add(STRING_MANAGER_RENAME);       // THREAD_WHENEXISTS_RENAME
    choices.Add(STRING_MANAGER_OVERWRITE);    // THREAD_WHENEXISTS_OVERWRITE
    choices.Add(STRING_MANAGER_SKIP);         // THREAD_WHENEXISTS_SKIP
    mExistsRadioBox = new wxRadioBox(this, wxID_ANY, STRING_MANAGER_EXISTS, wxDefaultPosition, wxDefaultSize, choices, 1);
    rightSizer->Add(mExistsRadioBox, 0, wxEXPAND|wxTOP, 5);

    panelSizer->Add(leftSizer,  1, wxEXPAND|wxALL, 5);
    panelSizer->Add(rightSizer, 0, wxEXPAND|wxALL, 5);
    topSizer->Add(panelSizer, 1, wxEXPAND|wxALL, 5);

    wxSizer* btnSizer = CreateButtonSizer(wxOK|wxCANCEL);
    topSizer->Add(btnSizer, 0, wxALIGN_RIGHT|wxLEFT|wxRIGHT|wxBOTTOM, 5);
    SetIcon(wxGetApp().mFrame->mIconApp);
    SetSizer(topSizer);
    Fit();

    wxCommandEvent dummy;
    OnAll(dummy);
    mDestUpdated = false;

    Connect(wxID_OPEN,               wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(kuRescaleTaskDialog::OnSrc));
    Connect(wxID_SAVE,               wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(kuRescaleTaskDialog::OnDest));
    Connect(kuID_RESCALEDLG_ALL,     wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(kuRescaleTaskDialog::OnAll));
    Connect(kuID_RESCALEDLG_NONE,    wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(kuRescaleTaskDialog::OnNone));
    Connect(kuID_RESCALEDLG_INVERSE, wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(kuRescaleTaskDialog::OnInverse));
    Connect(kuID_RESCALEDLG_SIZE,    wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler(kuRescaleTaskDialog::OnSizeSelected));
    Connect(kuID_RESCALEDLG_DEST,    wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler(kuRescaleTaskDialog::OnDestTextUpdated));
    Connect(wxID_OK,                 wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(kuRescaleTaskDialog::OnOK));
}

void kuRescaleTaskDialog::OnSrc(wxCommandEvent& event) {
    wxString dir = wxDirSelector(STRING_MANAGER_SRC, mSrcTextCtrl->GetValue());
    if(!dir.IsEmpty()) {
        mSrcTextCtrl->ChangeValue(dir);
        wxArrayString files;
        wxBeginBusyCursor();
        kuFiWrapper::GetAllSupportedFiles(dir, &files);
        wxEndBusyCursor();
        mFiles = files;
        mFileCheckListBox->Clear();
        for(size_t i=0; i<mFiles.GetCount(); i++) {
            mFileCheckListBox->Append(wxFileName(mFiles[i]).GetFullName());
        }
    }
}

void kuRescaleTaskDialog::OnDest(wxCommandEvent& event) {
    wxString target = mDestTextCtrl->GetValue();
    if(!wxDirExists(target))    target = wxFileName(target).GetPath();
    wxString dir = wxDirSelector(STRING_MANAGER_DEST, target);
    if(!dir.IsEmpty()) {
        mDestTextCtrl->SetValue(wxFileName(dir,mSizeComboBox->GetValue()).GetFullPath());    // causes wxEVT_COMMAND_TEXT_UPDATED
    }
}

void kuRescaleTaskDialog::OnAll(wxCommandEvent& event) {
    for(unsigned int i=0; i<mFileCheckListBox->GetCount(); i++) {
        mFileCheckListBox->Check(i, true);
    }
}

void kuRescaleTaskDialog::OnNone(wxCommandEvent& event) {
    for(unsigned int i=0; i<mFileCheckListBox->GetCount(); i++) {
        mFileCheckListBox->Check(i, false);
    }
}

void kuRescaleTaskDialog::OnInverse(wxCommandEvent& event) {
    for(unsigned int i=0; i<mFileCheckListBox->GetCount(); i++) {
        mFileCheckListBox->Check(i, !mFileCheckListBox->IsChecked(i));
    }
}

void kuRescaleTaskDialog::OnSizeSelected(wxCommandEvent& event) {
    wxComboBox* combo = (wxComboBox*)event.GetEventObject();
    if(event.GetString()==STRING_MANAGER_CUSTOM) {
        kuPositionDialog dlg(this, wxString(STRING_MANAGER_SIZE), wxSize(4000,3000), wxPoint(640,480));
        if(dlg.ShowModal() == wxID_OK) {
            combo->Insert(wxString::Format(FORMAT_MANAGER_SIZE,
                                           wxMax(dlg.GetPosition().x,dlg.GetPosition().y),
                                           wxMin(dlg.GetPosition().x,dlg.GetPosition().y)),
                          combo->GetCount()-1);
            combo->SetSelection(combo->GetCount()-2);
        } else {
            combo->SetSelection(mPrevSize);
            return;
        }

    }
    mPrevSize = combo->GetStrings().Index(combo->GetValue());
    if(!mDestUpdated) {
        mDestTextCtrl->ChangeValue(wxFileName(mSrcTextCtrl->GetValue(), combo->GetValue()).GetFullPath());
    }
}

void kuRescaleTaskDialog::OnDestTextUpdated(wxCommandEvent& event) {
    mDestUpdated = true;
}

void kuRescaleTaskDialog::OnOK(wxCommandEvent& event) {
    bool selected = false;
    for(unsigned int i=0; i<mFileCheckListBox->GetCount(); i++) {
        if(mFileCheckListBox->IsChecked(i)) {
            selected = true;
            break;
        }
    }
    if(!selected) {
        wxMessageBox(STRING_WARNING_ISNOTFILE, kuFrame::StripCodes(STRING_TASKBTN_NEW));
        return;
    }
    // assume src and dest are absolute path
    wxString dest = mDestTextCtrl->GetValue();
    if(dest==wxEmptyString || !wxIsAbsolutePath(dest)) {
        wxMessageBox(STRING_ERROR_FILE_DESINVALID+mDestTextCtrl->GetValue(), kuFrame::StripCodes(STRING_TASKBTN_NEW), wxICON_ERROR);
        mDestTextCtrl->SetFocus();
        return;
    }
    wxFileName dfn(dest);
    dfn.Normalize();
    wxFileName sfn(mSrcTextCtrl->GetValue());
    sfn.Normalize();
    if(sfn==dfn || (sfn.GetFullPath()+wxFileName::GetPathSeparator())==dfn.GetFullPath()) {
        if(wxMessageBox(STRING_ERROR_FILE_DESISSRC+wxString(STRING_CONTINUE_COMMON), STRING_CONTINUE_CAPTION, wxYES_NO|wxICON_EXCLAMATION)
           == wxNO)
           return;
    }
    EndModal(wxID_OK);
}

kuRescaleTask* kuRescaleTaskDialog::GetTask() {
    kuRescaleTask* task = new kuRescaleTask();
    task->Src = mSrcTextCtrl->GetValue();    // assume it always without wxFileName::GetPathSeparator()
    for(unsigned int i=0; i<mFileCheckListBox->GetCount(); i++) {
        if(mFileCheckListBox->IsChecked(i)) {
            //wxMessageBox(wxString::Format(wxT("%d: %s"), i, mFileCheckListBox->GetString(i)));
            task->Files.Add(mFiles[i]);
        }
    }
    wxFileName dest(mDestTextCtrl->GetValue());
    dest.Normalize();
    if(dest.GetFullPath().Right(1)==wxFileName::GetPathSeparator())    task->Dest = dest.GetPath();
    else    task->Dest = dest.GetFullPath();
    //wxMessageBox(task->Dest);
    task->Link = task->Dest;
    int x, y;
    wxSscanf(mSizeComboBox->GetValue(), FORMAT_MANAGER_SIZE, &x, &y);
    task->Size = wxSize(x, y);
    task->Format = mFormatComboBox->GetValue();
    task->Filter = (FREE_IMAGE_FILTER)mFilterComboBox->GetStrings().Index(mFilterComboBox->GetValue());
    task->WhenExists = mExistsRadioBox->GetSelection();    // assume items are added according to THREAD_WHENEXISTS_*
    return task;
}

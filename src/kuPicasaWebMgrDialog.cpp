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

#ifdef ENABLE_PICASAWEBMGR
kuPicasaWebMgrDialog::kuPicasaWebMgrDialog(kuFrame* frame, int id)
    :kuManagerDialog(frame, id, kuFrame::StripCodes(STRING_MENU_PICASAWEB_MANAGER)) {
    wxArrayInt defCols;
    defCols.Add(kuID_PICASAWEBMGR_COL_SRC);
    defCols.Add(kuID_PICASAWEBMGR_COL_DEST);
    defCols.Add(kuID_PICASAWEBMGR_COL_PROGRESS);
    defCols.Add(kuID_PICASAWEBMGR_COL_STATUS);
    wxArrayInt customCols;
    customCols.Add(kuID_PICASAWEBMGR_COL_ACTION);
    customCols.Add(kuID_PICASAWEBMGR_COL_LOGIN);
    wxArrayString customHeaders;
    customHeaders.Add(STRING_MANAGER_ACTION);
    customHeaders.Add(STRING_MANAGER_LOGIN);
    kuManagerDialog::Set(wxGetApp().mOptions.mPicasaWebMgrMaxCon, defCols, customCols, customHeaders);
}

kuManagedTask* kuPicasaWebMgrDialog::CreateTask() {
    mTaskValues[STRING_MANAGER_LOCAL] = mFrame->mGeneric->GetDir();
    kuPicasaWebTaskDialog dlg(this, wxID_ANY, kuFrame::StripCodes(STRING_TASKBTN_NEW), mTaskValues);
    int ret = dlg.ShowModal();
    dlg.GetValues(mTaskValues);
    if(ret != wxID_OK)    return NULL;
    return dlg.GetTask();
}

wxThread* kuPicasaWebMgrDialog::CreateThread(kuManagedTask* task) {
    return new kuPicasaWebThread((kuPicasaWebTask*)task, this);
}

wxString kuPicasaWebMgrDialog::GetTaskInfo(kuManagedTask* task, int col) {
    kuPicasaWebTask* ptask = (kuPicasaWebTask*)task;
    wxString str = wxEmptyString;
    switch (col) {
        case kuID_PICASAWEBMGR_COL_ACTION:
        {
            switch (ptask->Action) {
                case THREAD_ACTION_UPLOAD:
                    str = STRING_MANAGER_UPLOAD;
                    break;
                case THREAD_ACTION_DOWNLOAD:
                    str = STRING_MANAGER_DOWNLOAD;
                    break;
                case THREAD_ACTION_DELETE:
                    str = STRING_MANAGER_DELETE;
                    break;
                default:
                    break;
            }
            break;
        }
        case kuID_PICASAWEBMGR_COL_LOGIN:
            str = ptask->Login;
        default:
            break;
    }
    //wxMessageBox(wxString::Format(wxT("%d:%s"), col, str));
    return str;
}

wxString kuPicasaWebMgrDialog::GetErrorInfo(int errcode) {
    wxString errstr = wxEmptyString;
    switch (errcode) {
        default:
            errstr = kuManagerDialog::GetErrorInfo(errcode);
            break;
    }
    return errstr;
}

// -------- kuPicasaWebTaskDialog --------
kuPicasaWebTaskDialog::kuPicasaWebTaskDialog(wxWindow* parent, wxWindowID id, wxString title, kuStrHashMap& values)
    :wxDialog(parent, id, title, wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER) {
    wxString local   = values[STRING_MANAGER_LOCAL];
    wxString remote  = values[STRING_MANAGER_REMOTE];
    wxString account = values[STRING_MANAGER_ACCOUNT];
    wxString passwd  = values[STRING_MANAGER_PASSWD];
    wxString owner   = values[STRING_MANAGER_OWNER];
    mToken           = values[STRING_MANAGER_LOGIN];

    wxBeginBusyCursor();
    kuFiWrapper::GetAllSupportedFiles(local, &mFiles);
    wxEndBusyCursor();

    wxBoxSizer* topSizer = new wxBoxSizer(wxVERTICAL);

    wxArrayString choices;    // the string order should be the same as the switch in OnAction()
    choices.Add(STRING_MANAGER_UPLOAD);      // THREAD_ACTION_UPLOAD
    choices.Add(STRING_MANAGER_DOWNLOAD);    // THREAD_ACTION_DOWNLOAD
    choices.Add(STRING_MANAGER_DELETE);      // THREAD_ACTION_DELETE
    topSizer->Add(new wxRadioBox(this, kuID_PICASAWEBDLG_ACTION, STRING_MANAGER_ACTION, wxDefaultPosition, wxDefaultSize, choices, 3), 0, wxEXPAND|wxALL, 10);

    wxBoxSizer* panelSizer = new wxBoxSizer(wxHORIZONTAL);

    // left panel: local
    wxStaticBoxSizer* leftSizer = new wxStaticBoxSizer(wxVERTICAL, this, STRING_MANAGER_LOCAL);
    wxBoxSizer* localSizer = new wxBoxSizer(wxHORIZONTAL);
    mLocalTextCtrl = new wxTextCtrl(this, wxID_ANY, local, wxDefaultPosition, wxDefaultSize, wxTE_READONLY|wxTE_PROCESS_ENTER);
    localSizer->Add(mLocalTextCtrl, 1, wxEXPAND);
    localSizer->Add(new wxBitmapButton(this, wxID_OPEN, wxArtProvider::GetBitmap(wxART_FILE_OPEN)), 0, wxEXPAND|wxLEFT, 5);
    leftSizer->Add(localSizer, 0, wxEXPAND);
    mFileCheckListBox = new wxCheckListBox(this, wxID_ANY, wxDefaultPosition, wxSize(wxDefaultSize.x, 10));    // set small min_height for align rightSizer
    for(size_t i=0; i<mFiles.GetCount(); i++) {
        mFileCheckListBox->Append(wxFileName(mFiles[i]).GetFullName());
    }
    leftSizer->Add(mFileCheckListBox, 1, wxEXPAND|wxTOP, 5);
    mLSelSizer = new wxBoxSizer(wxHORIZONTAL);
    mLSelSizer->Add(new wxButton(this, kuID_PICASAWEBDLG_LSEL_ALL,     STRING_SELBTN_ALL),     0, wxEXPAND);
    mLSelSizer->Add(new wxButton(this, kuID_PICASAWEBDLG_LSEL_NONE,    STRING_SELBTN_NONE),    0, wxEXPAND|wxLEFT, 5);
    mLSelSizer->Add(new wxButton(this, kuID_PICASAWEBDLG_LSEL_INVERSE, STRING_SELBTN_INVERSE), 0, wxEXPAND|wxLEFT, 5);
    leftSizer->Add(mLSelSizer, 0, wxALIGN_CENTER|wxTOP, 5);

    // right panel: remote
    wxStaticBoxSizer* rightSizer = new wxStaticBoxSizer(wxVERTICAL, this, STRING_MANAGER_REMOTE);

    // login
    wxBoxSizer* loginSizer = new wxBoxSizer(wxHORIZONTAL);
    loginSizer->Add(new wxStaticText(this, wxID_ANY, STRING_MANAGER_ACCOUNT), 0, wxEXPAND|wxTOP, 5);
    mAccountTextCtrl = new wxTextCtrl(this, wxID_ANY, account);
    loginSizer->Add(mAccountTextCtrl, 1, wxEXPAND|wxLEFT, 5);
    loginSizer->Add(new wxStaticText(this, wxID_ANY, STRING_MANAGER_PASSWD), 0, wxEXPAND|wxLEFT|wxTOP, 5);
    mPasswdTextCtrl = new wxTextCtrl(this, kuID_PICASAWEBDLG_LOGIN, passwd, wxDefaultPosition, wxDefaultSize, wxTE_PASSWORD|wxTE_PROCESS_ENTER);
    loginSizer->Add(mPasswdTextCtrl, 0, wxEXPAND|wxLEFT, 5);
    mLoginButton = new wxButton(this, kuID_PICASAWEBDLG_LOGIN, STRING_MANAGER_LOGIN);
    loginSizer->Add(mLoginButton, 0, wxEXPAND|wxLEFT, 5);
    mAnonymousCheckBox = new wxCheckBox(this, kuID_PICASAWEBDLG_ANONYMOUS, STRING_MANAGER_ANONYMOUS);
    loginSizer->Add(mAnonymousCheckBox, 0, wxEXPAND|wxALL, 5);
    rightSizer->Add(loginSizer, 0, wxEXPAND);

    rightSizer->Add(new wxStaticLine(this), 0, wxEXPAND|wxTOP, 5);

    // query
    wxBoxSizer* querySizer = new wxBoxSizer(wxHORIZONTAL);

    wxBoxSizer* ownerSizer = new wxBoxSizer(wxVERTICAL);
    wxBoxSizer* oTxtSizer = new wxBoxSizer(wxHORIZONTAL);
    oTxtSizer->Add(new wxStaticText(this, wxID_ANY, STRING_MANAGER_OWNER), 0, wxEXPAND|wxTOP, 5);
    mOwnerTextCtrl = new wxTextCtrl(this, kuID_PICASAWEBDLG_QUERY, owner, wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER);
    oTxtSizer->Add(mOwnerTextCtrl, 1, wxEXPAND|wxLEFT, 5);
    ownerSizer->Add(oTxtSizer, 0, wxEXPAND);
    mQueryButton = new wxButton(this, kuID_PICASAWEBDLG_QUERY, STRING_MANAGER_QUERY);
    ownerSizer->Add(mQueryButton, 0, wxEXPAND|wxTOP, 5);
    querySizer->Add(ownerSizer, 0, wxEXPAND);

    querySizer->Add(new wxStaticLine(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_VERTICAL), 0, wxEXPAND|wxLEFT, 5);

    wxBoxSizer* albumSizer = new wxBoxSizer(wxVERTICAL);
    wxBoxSizer* aTxtSizer = new wxBoxSizer(wxHORIZONTAL);
    aTxtSizer->Add(new wxStaticText(this, wxID_ANY, STRING_MANAGER_ALBUM), 0, wxEXPAND|wxTOP, 5);
    mAlbumComboBox = new wxComboBox(this, kuID_PICASAWEBDLG_ALBUM, wxEmptyString, wxDefaultPosition, wxDefaultSize, NULL, wxCB_READONLY);
    aTxtSizer->Add(mAlbumComboBox, 1, wxEXPAND);
    albumSizer->Add(aTxtSizer, 0, wxEXPAND);
    mABtnSizer = new wxBoxSizer(wxHORIZONTAL);
    mABtnSizer->Add(new wxButton(this, wxID_NEW,    STRING_MANAGER_NEW),                    0, wxEXPAND|wxLEFT|wxRIGHT, 5);
    mABtnSizer->Add(new wxButton(this, wxID_EDIT,   kuFrame::StripCodes(STRING_MENU_EDIT)), 0, wxEXPAND|wxLEFT|wxRIGHT, 5);
    mABtnSizer->Add(new wxButton(this, wxID_DELETE, STRING_MANAGER_DELETE),                 0, wxEXPAND|wxLEFT|wxRIGHT, 5);
    albumSizer->Add(mABtnSizer, 0, wxALIGN_CENTER|wxTOP, 5);

    querySizer->Add(albumSizer, 1, wxEXPAND|wxLEFT, 5);

    rightSizer->Add(querySizer, 0, wxEXPAND|wxTOP, 5);

    // photo
    mPhotoCheckListBox = new wxCheckListBox(this, wxID_ANY, wxDefaultPosition, wxSize(wxDefaultSize.x, 10));    // set small min_height for align rightSizer
    rightSizer->Add(mPhotoCheckListBox, 1, wxEXPAND|wxTOP, 5);

    mRSelSizer = new wxBoxSizer(wxHORIZONTAL);
    mRSelSizer->Add(new wxButton(this, kuID_PICASAWEBDLG_RSEL_ALL,     STRING_SELBTN_ALL),     0, wxEXPAND);
    mRSelSizer->Add(new wxButton(this, kuID_PICASAWEBDLG_RSEL_NONE,    STRING_SELBTN_NONE),    0, wxEXPAND|wxLEFT, 5);
    mRSelSizer->Add(new wxButton(this, kuID_PICASAWEBDLG_RSEL_INVERSE, STRING_SELBTN_INVERSE), 0, wxEXPAND|wxLEFT, 5);
    rightSizer->Add(mRSelSizer, 0, wxALIGN_CENTER|wxTOP, 5);

    panelSizer->Add(leftSizer,  1, wxEXPAND|wxALL, 5);
    panelSizer->Add(rightSizer, 2, wxEXPAND|wxALL, 5);
    topSizer->Add(panelSizer, 1, wxEXPAND|wxLEFT|wxRIGHT|wxBOTTOM, 5);

    wxSizer* btnSizer = CreateButtonSizer(wxOK|wxCANCEL);
    mMkdirCheckBox = new wxCheckBox(this, kuID_PICASAWEBDLG_MKDIR, STRING_MANAGER_MKDIR);
    btnSizer->Insert(0, mMkdirCheckBox, 1, wxLEFT|wxRIGHT|wxEXPAND, 5);
    topSizer->Add(btnSizer, 0, wxALIGN_RIGHT|wxALL|wxEXPAND, 5);
    SetIcon(wxGetApp().mFrame->mIconApp);
    SetSizer(topSizer);
    Fit();
    SetSize(GetSize().x, 450);

    FindWindow(wxID_OK)->Enable(false);
    if(mToken == wxEmptyString) {
        mQueryButton->Enable(false);
        mAlbumComboBox->Enable(false);
        kuApp::EnableSizerChildren(mABtnSizer, false);
    } else {
        wxCommandEvent dummy;
        OnQuery(dummy);
        mAlbumComboBox->SetStringSelection(remote);
    }

    wxCommandEvent dummy;
    OnLSelAll(dummy);
    dummy.SetInt(0);
    OnAction(dummy);

    Connect(wxID_OPEN,                      wxEVT_COMMAND_BUTTON_CLICKED,    wxCommandEventHandler(kuPicasaWebTaskDialog::OnLocal));
    Connect(kuID_PICASAWEBDLG_LSEL_ALL,     wxEVT_COMMAND_BUTTON_CLICKED,    wxCommandEventHandler(kuPicasaWebTaskDialog::OnLSelAll));
    Connect(kuID_PICASAWEBDLG_LSEL_NONE,    wxEVT_COMMAND_BUTTON_CLICKED,    wxCommandEventHandler(kuPicasaWebTaskDialog::OnLSelNone));
    Connect(kuID_PICASAWEBDLG_LSEL_INVERSE, wxEVT_COMMAND_BUTTON_CLICKED,    wxCommandEventHandler(kuPicasaWebTaskDialog::OnLSelInverse));
    Connect(kuID_PICASAWEBDLG_ACTION,       wxEVT_COMMAND_RADIOBOX_SELECTED, wxCommandEventHandler(kuPicasaWebTaskDialog::OnAction));
    Connect(kuID_PICASAWEBDLG_LOGIN,        wxEVT_COMMAND_TEXT_ENTER,        wxCommandEventHandler(kuPicasaWebTaskDialog::OnLogin));
    Connect(kuID_PICASAWEBDLG_LOGIN,        wxEVT_COMMAND_BUTTON_CLICKED,    wxCommandEventHandler(kuPicasaWebTaskDialog::OnLogin));
    Connect(kuID_PICASAWEBDLG_ANONYMOUS,    wxEVT_COMMAND_CHECKBOX_CLICKED,  wxCommandEventHandler(kuPicasaWebTaskDialog::OnAnonymous));
    Connect(kuID_PICASAWEBDLG_QUERY,        wxEVT_COMMAND_TEXT_ENTER,        wxCommandEventHandler(kuPicasaWebTaskDialog::OnQuery));
    Connect(kuID_PICASAWEBDLG_QUERY,        wxEVT_COMMAND_BUTTON_CLICKED,    wxCommandEventHandler(kuPicasaWebTaskDialog::OnQuery));
    Connect(kuID_PICASAWEBDLG_ALBUM,        wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler(kuPicasaWebTaskDialog::OnAlbum));
    Connect(wxID_NEW,                       wxEVT_COMMAND_BUTTON_CLICKED,    wxCommandEventHandler(kuPicasaWebTaskDialog::OnNewAlbum));
    Connect(wxID_EDIT,                      wxEVT_COMMAND_BUTTON_CLICKED,    wxCommandEventHandler(kuPicasaWebTaskDialog::OnEditAlbum));
    Connect(wxID_DELETE,                    wxEVT_COMMAND_BUTTON_CLICKED,    wxCommandEventHandler(kuPicasaWebTaskDialog::OnDeleteAlbum));
    Connect(kuID_PICASAWEBDLG_RSEL_ALL,     wxEVT_COMMAND_BUTTON_CLICKED,    wxCommandEventHandler(kuPicasaWebTaskDialog::OnRSelAll));
    Connect(kuID_PICASAWEBDLG_RSEL_NONE,    wxEVT_COMMAND_BUTTON_CLICKED,    wxCommandEventHandler(kuPicasaWebTaskDialog::OnRSelNone));
    Connect(kuID_PICASAWEBDLG_RSEL_INVERSE, wxEVT_COMMAND_BUTTON_CLICKED,    wxCommandEventHandler(kuPicasaWebTaskDialog::OnRSelInverse));
    Connect(wxID_OK,                        wxEVT_COMMAND_BUTTON_CLICKED,    wxCommandEventHandler(kuPicasaWebTaskDialog::OnOK));
}

kuPicasaWebTaskDialog::~kuPicasaWebTaskDialog() {
    WX_CLEAR_ARRAY(mAlbums);
    WX_CLEAR_ARRAY(mPhotos);
}

void kuPicasaWebTaskDialog::OnLocal(wxCommandEvent& event) {
    wxString dir = wxDirSelector(STRING_MANAGER_LOCAL, mLocalTextCtrl->GetValue());
    if(!dir.IsEmpty()) {
        mLocalTextCtrl->ChangeValue(dir);
        wxArrayString files;
        wxBeginBusyCursor();
        kuPwWrapper::GetAllSupportedFiles(dir, &files);
        wxEndBusyCursor();
        mFiles = files;
        mFileCheckListBox->Clear();
        for(size_t i=0; i<mFiles.GetCount(); i++) {
            mFileCheckListBox->Append(wxFileName(mFiles[i]).GetFullName());
        }
    }
}

void kuPicasaWebTaskDialog::OnLSelAll(wxCommandEvent& event) {
    for(unsigned int i=0; i<mFileCheckListBox->GetCount(); i++) {
        mFileCheckListBox->Check(i, true);
    }
}

void kuPicasaWebTaskDialog::OnLSelNone(wxCommandEvent& event) {
    for(unsigned int i=0; i<mFileCheckListBox->GetCount(); i++) {
        mFileCheckListBox->Check(i, false);
    }
}

void kuPicasaWebTaskDialog::OnLSelInverse(wxCommandEvent& event) {
    for(unsigned int i=0; i<mFileCheckListBox->GetCount(); i++) {
        mFileCheckListBox->Check(i, !mFileCheckListBox->IsChecked(i));
    }
}

void kuPicasaWebTaskDialog::OnAction(wxCommandEvent& event) {
    mAction = event.GetSelection();
    switch (mAction) {
        case THREAD_ACTION_UPLOAD:
            mLocalTextCtrl->Enable(true);
            mFileCheckListBox->Enable(true);
            kuApp::EnableSizerChildren(mLSelSizer, true);
            mAnonymousCheckBox->Enable(false);
            mOwnerTextCtrl->Enable(false);
            kuApp::EnableSizerChildren(mABtnSizer, mAlbumComboBox->IsEnabled());
            mPhotoCheckListBox->Enable(false);
            kuApp::EnableSizerChildren(mRSelSizer, false);
            mMkdirCheckBox->Show(false);
            break;
        case THREAD_ACTION_DOWNLOAD:
            mLocalTextCtrl->Enable(true);
            mFileCheckListBox->Enable(false);
            kuApp::EnableSizerChildren(mLSelSizer, false);
            mAnonymousCheckBox->Enable(true);
            mOwnerTextCtrl->Enable(true);
            kuApp::EnableSizerChildren(mABtnSizer, false);
            mPhotoCheckListBox->Enable(true);
            kuApp::EnableSizerChildren(mRSelSizer, true);
            mMkdirCheckBox->Show(true);
            mMkdirCheckBox->SetValue(true);
            break;
        case THREAD_ACTION_DELETE:
            mLocalTextCtrl->Enable(false);
            mFileCheckListBox->Enable(false);
            kuApp::EnableSizerChildren(mLSelSizer, false);
            mAnonymousCheckBox->Enable(false);
            mOwnerTextCtrl->Enable(false);
            kuApp::EnableSizerChildren(mABtnSizer, true);
            mPhotoCheckListBox->Enable(true);
            kuApp::EnableSizerChildren(mRSelSizer, true);
            mMkdirCheckBox->Show(false);
            break;
        default:
            break;
    }
}

void kuPicasaWebTaskDialog::OnLogin(wxCommandEvent& event) {
    wxBeginBusyCursor();
    mToken = kuPwWrapper::Login(mAccountTextCtrl->GetValue(), mPasswdTextCtrl->GetValue());
    wxEndBusyCursor();
    if(mToken == wxEmptyString) {
        wxMessageBox(STRING_ERROR_LOGIN_FAILED);
        return;
    }
    mQueryButton->Enable(true);
    mOwnerTextCtrl->ChangeValue(mAccountTextCtrl->GetValue());
    if(!mOwnerTextCtrl->IsEnabled()) {
        wxCommandEvent dummy;
        OnQuery(dummy);
    }
}

void kuPicasaWebTaskDialog::OnAnonymous(wxCommandEvent& event) {
    bool anonymous = event.IsChecked();
    mAccountTextCtrl->Enable(!anonymous);
    mPasswdTextCtrl->Enable(!anonymous);
    mLoginButton->Enable(!anonymous);
    mQueryButton->Enable(anonymous);
    if(anonymous)    mToken = wxEmptyString;
}

void kuPicasaWebTaskDialog::OnQuery(wxCommandEvent& event) {
    mAlbumComboBox->Clear();
    WX_CLEAR_ARRAY(mAlbums);
    wxBeginBusyCursor();
    bool success = kuPwWrapper::GetAlbums(mOwnerTextCtrl->GetValue(), &mAlbums, mToken);
    wxEndBusyCursor();
    if(!success) {
        wxMessageBox(STRING_ERROR_ALBUM_QUERYFAILED);
        return;
    }
    mAlbumComboBox->Enable(true);
    kuApp::EnableSizerChildren(mABtnSizer, mAction!=THREAD_ACTION_DOWNLOAD);
    for(size_t i=0; i<mAlbums.GetCount(); i++) {
        mAlbumComboBox->Append(kuPwWrapper::GetProperty(mAlbums[i], STRING_PICASAWEB_PROP_TITLE));
    }
    if(mAlbumComboBox->GetCount()) {
        mAlbumComboBox->SetSelection(0);
    }
    wxCommandEvent dummy;
    OnAlbum(dummy);
}

void kuPicasaWebTaskDialog::OnAlbum(wxCommandEvent& event) {
    //wxMessageBox(wxString::Format(wxT("%d"), event.GetSelection()));
    int select = mAlbumComboBox->GetSelection();
    FindWindow(wxID_OK)->Enable(select!=wxNOT_FOUND);
    if(select == wxNOT_FOUND)    return;
    mPhotoCheckListBox->Clear();
    WX_CLEAR_ARRAY(mPhotos);
    wxBeginBusyCursor();
    bool success = kuPwWrapper::GetPhotos(mAlbums[select], &mPhotos, mToken);
    wxEndBusyCursor();
    if(!success) {
        wxMessageBox(STRING_ERROR_PHOTO_QUERYFAILED);
        return;
    }
    for(size_t i=0; i<mPhotos.GetCount(); i++) {
        mPhotoCheckListBox->Append(kuPwWrapper::GetProperty(mPhotos[i], STRING_PICASAWEB_PROP_TITLE));
    }
}

void kuPicasaWebTaskDialog::OnNewAlbum(wxCommandEvent& event) {
    kuStrHashMap defs;
    defs[STRING_PICASAWEB_PROP_ACCESS] = STRING_PICASAWEB_ACCESS_PUBLIC;
    kuPicasaWebAlbumDialog dlg(this, wxID_ANY, wxString::Format(FORMAT_MANAGER_DLGTITLE, STRING_MANAGER_NEW, STRING_MANAGER_ALBUM), defs);
    if(dlg.ShowModal() == wxID_OK) {
        kuStrHashMap* values = dlg.GetValues();
        wxXmlNode* created = kuPwWrapper::AddAlbum(mAccountTextCtrl->GetValue(),
                                                   mToken,
                                                   (*values)[STRING_PICASAWEB_PROP_TITLE],
                                                   (*values)[STRING_PICASAWEB_PROP_SUMMARY],
                                                   (*values)[STRING_PICASAWEB_PROP_LOCATION],
                                                   (*values)[STRING_PICASAWEB_PROP_ACCESS]);
        if(created) {
            mAlbums.Insert(created, 0);
            mAlbumComboBox->Insert(kuPwWrapper::GetProperty(created, STRING_PICASAWEB_PROP_TITLE), 0);
            mAlbumComboBox->SetSelection(0);
            wxCommandEvent dummy;
            OnAlbum(dummy);
        } else {
            wxMessageBox(STRING_ERROR_ALBUM_NEWFAILED);
        }
        delete values;
    }
}

void kuPicasaWebTaskDialog::OnEditAlbum(wxCommandEvent& event) {
    int select = mAlbumComboBox->GetSelection();
    wxXmlNode* album = mAlbums[select];
    kuStrHashMap defs;
    defs[STRING_PICASAWEB_PROP_TITLE]    = kuPwWrapper::GetProperty(album, STRING_PICASAWEB_PROP_TITLE);
    defs[STRING_PICASAWEB_PROP_SUMMARY]  = kuPwWrapper::GetProperty(album, STRING_PICASAWEB_PROP_SUMMARY);
    defs[STRING_PICASAWEB_PROP_LOCATION] = kuPwWrapper::GetProperty(album, STRING_PICASAWEB_PROP_LOCATION);
    defs[STRING_PICASAWEB_PROP_ACCESS]   = kuPwWrapper::GetProperty(album, STRING_PICASAWEB_PROP_ACCESS);
    kuPicasaWebAlbumDialog dlg(this, wxID_ANY, wxString::Format(FORMAT_MANAGER_DLGTITLE, kuFrame::StripCodes(STRING_MENU_EDIT), STRING_MANAGER_ALBUM), defs);
    if(dlg.ShowModal() == wxID_OK) {
        kuStrHashMap* values = dlg.GetValues();
        kuPwWrapper::SetProperty(album, STRING_PICASAWEB_PROP_TITLE,    (*values)[STRING_PICASAWEB_PROP_TITLE]);
        kuPwWrapper::SetProperty(album, STRING_PICASAWEB_PROP_SUMMARY,  (*values)[STRING_PICASAWEB_PROP_SUMMARY]);
        kuPwWrapper::SetProperty(album, STRING_PICASAWEB_PROP_LOCATION, (*values)[STRING_PICASAWEB_PROP_LOCATION]);
        kuPwWrapper::SetProperty(album, STRING_PICASAWEB_PROP_ACCESS,   (*values)[STRING_PICASAWEB_PROP_ACCESS]);
        wxXmlNode* modified = kuPwWrapper::ModifyAlbum(mToken, album);
        if(modified) {
            mAlbums.RemoveAt(select);
            mAlbums.Insert(modified, select);
            delete album;
            mAlbumComboBox->SetString(select, kuPwWrapper::GetProperty(modified, STRING_PICASAWEB_PROP_TITLE));
            mAlbumComboBox->SetSelection(0);
        } else {
            wxMessageBox(STRING_ERROR_ALBUM_EDITFAILED);
        }
        delete values;
    }
}

void kuPicasaWebTaskDialog::OnDeleteAlbum(wxCommandEvent& event) {
    if(wxMessageBox(STRING_ALBUM_DELETE_MESSAGE, wxString::Format(FORMAT_MANAGER_DLGTITLE, STRING_MANAGER_DELETE, STRING_MANAGER_ALBUM), wxYES_NO|wxICON_EXCLAMATION)
       == wxYES) {
        int select = mAlbumComboBox->GetSelection();
        wxXmlNode* album = mAlbums[select];
        if(kuPwWrapper::RemoveEntry(mToken, album)) {
            mAlbums.RemoveAt(select);
            delete album;
            mAlbumComboBox->Delete(select);
            if(mAlbumComboBox->GetCount())    mAlbumComboBox->SetSelection(0);
            wxCommandEvent dummy;
            OnAlbum(dummy);
        } else {
            wxMessageBox(STRING_ERROR_ALBUM_DELETEFAILED);
        }
    }
}

void kuPicasaWebTaskDialog::OnRSelAll(wxCommandEvent& event) {
    for(unsigned int i=0; i<mPhotoCheckListBox->GetCount(); i++) {
        mPhotoCheckListBox->Check(i, true);
    }
}

void kuPicasaWebTaskDialog::OnRSelNone(wxCommandEvent& event) {
    for(unsigned int i=0; i<mPhotoCheckListBox->GetCount(); i++) {
        mPhotoCheckListBox->Check(i, false);
    }
}

void kuPicasaWebTaskDialog::OnRSelInverse(wxCommandEvent& event) {
    for(unsigned int i=0; i<mPhotoCheckListBox->GetCount(); i++) {
        mPhotoCheckListBox->Check(i, !mPhotoCheckListBox->IsChecked(i));
    }
}

void kuPicasaWebTaskDialog::OnOK(wxCommandEvent& event) {
    switch (mAction) {
        case THREAD_ACTION_UPLOAD:
        {
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
            // TODO: check album
            break;
        }
        case THREAD_ACTION_DOWNLOAD:
        {
            wxString local = mLocalTextCtrl->GetValue();
            if(local==wxEmptyString || !wxIsAbsolutePath(local)) {
                wxMessageBox(STRING_ERROR_FILE_DESINVALID+mLocalTextCtrl->GetValue(), kuFrame::StripCodes(STRING_TASKBTN_NEW), wxICON_ERROR);
                mLocalTextCtrl->SetFocus();
                return;
            }
            if(mMkdirCheckBox->IsChecked()) {
                wxString sub = local+wxFileName::GetPathSeparator()+mAlbumComboBox->GetValue();
                while(!wxDirExists(sub) && !wxMkdir(sub)) {
                    if(!wxFileName::IsDirWritable(local)) {
                        wxMessageBox(STRING_THREADERR_READONLY+wxString(wxT("\n"))+local, STRING_MANAGER_MKDIR, wxICON_ERROR);
                        return;
                    }
                    wxString text = wxGetTextFromUser(STRING_MKDIR_MESSAGE, STRING_MANAGER_MKDIR, mAlbumComboBox->GetValue());
                    if(text == wxEmptyString)    return;
                    sub = local+wxFileName::GetPathSeparator()+text;
                }
                mLocalTextCtrl->SetValue(sub);
            }
            break;
        }
        case THREAD_ACTION_DELETE:
        {
            // TODO: check album
            break;
        }
        default:
            break;
    }
    EndModal(wxID_OK);
}

kuPicasaWebTask* kuPicasaWebTaskDialog::GetTask() {
    kuPicasaWebTask* task = new kuPicasaWebTask();
    task->Action = mAction;
    if(mAnonymousCheckBox->IsChecked())    task->Login = STRING_MANAGER_ANONYMOUS;
    else    task->Login = mAccountTextCtrl->GetValue();
    task->Token = mToken;
    task->Album = new wxXmlNode(*mAlbums[mAlbumComboBox->GetSelection()]);
    wxString local = mLocalTextCtrl->GetValue();    // assume it always without wxFileName::GetPathSeparator()
    wxString remote = wxString::Format(FORMAT_MANAGER_ALBUM, mOwnerTextCtrl->GetValue(), mAlbumComboBox->GetValue());
    switch (mAction) {
        case THREAD_ACTION_UPLOAD:
            task->Src = local;
            task->Dest = remote;
            task->Link = kuPwWrapper::GetAlbumLink(task->Album);
            for(unsigned int i=0; i<mFileCheckListBox->GetCount(); i++) {
                if(mFileCheckListBox->IsChecked(i)) {
                    task->Files.Add(mFiles[i]);
                }
            }
            break;
        case THREAD_ACTION_DOWNLOAD:
            task->Src = remote;
            task->Dest = local;
            task->Link = local;
            for(unsigned int i=0; i<mPhotoCheckListBox->GetCount(); i++) {
                if(mPhotoCheckListBox->IsChecked(i)) {
                    task->Files.Add(mPhotoCheckListBox->GetString(i));
                    task->Photos.Add(new wxXmlNode(*mPhotos[i]));
                }
            }
            break;
        case THREAD_ACTION_DELETE:
            task->Src = remote;
            task->Dest = wxEmptyString;
            task->Link = kuPwWrapper::GetAlbumLink(task->Album);;
            for(unsigned int i=0; i<mPhotoCheckListBox->GetCount(); i++) {
                if(mPhotoCheckListBox->IsChecked(i)) {
                    task->Files.Add(mPhotoCheckListBox->GetString(i));
                    task->Photos.Add(new wxXmlNode(*mPhotos[i]));
                }
            }
            break;
        default:
            break;
    }
    return task;
}

void kuPicasaWebTaskDialog::GetValues(kuStrHashMap& values) {
    values[STRING_MANAGER_LOCAL]   = mLocalTextCtrl->GetValue();
    values[STRING_MANAGER_REMOTE]  = mAlbumComboBox->GetValue();
    values[STRING_MANAGER_ACCOUNT] = mAccountTextCtrl->GetValue();
    values[STRING_MANAGER_PASSWD]  = mPasswdTextCtrl->GetValue();
    values[STRING_MANAGER_OWNER]   = mOwnerTextCtrl->GetValue();
    values[STRING_MANAGER_LOGIN]   = mToken;
}

// -------- kuPicasaWebAlbumDialog --------
kuPicasaWebAlbumDialog::kuPicasaWebAlbumDialog(wxWindow* parent, wxWindowID id, wxString title, kuStrHashMap& values)
    :wxDialog(parent, wxID_ANY, title) {
    wxString album    = values[STRING_PICASAWEB_PROP_TITLE];
    wxString summary  = values[STRING_PICASAWEB_PROP_SUMMARY];
    wxString location = values[STRING_PICASAWEB_PROP_LOCATION];
    wxString access   = values[STRING_PICASAWEB_PROP_ACCESS];
    mAccessStrs.Add(STRING_PICASAWEB_ACCESS_PUBLIC);
    mAccessStrs.Add(STRING_PICASAWEB_ACCESS_PRIVATE);
    mAccessStrs.Add(STRING_PICASAWEB_ACCESS_PROTECTED);
    wxBoxSizer* topSizer = new wxBoxSizer(wxVERTICAL);

    wxFlexGridSizer* entrySizer = new wxFlexGridSizer(4, 2, 5, 5);
    entrySizer->AddGrowableCol(1);
    // title
    entrySizer->Add(new wxStaticText(this, wxID_ANY, STRING_MANAGER_TITLE, wxDefaultPosition, wxDefaultSize, wxALIGN_CENTRE),
                    0, wxALIGN_CENTER);
    mTitleTextCtrl = new wxTextCtrl(this, wxID_EDIT);
    entrySizer->Add(mTitleTextCtrl, 0, wxEXPAND);
    // summary
    entrySizer->Add(new wxStaticText(this, wxID_ANY, kuFrame::StripCodes(STRING_MENU_SUMMARY), wxDefaultPosition, wxDefaultSize, wxALIGN_CENTRE),
                    0, wxALIGN_CENTER);
    mSummaryTextCtrl = new wxTextCtrl(this, wxID_ANY);
    entrySizer->Add(mSummaryTextCtrl, 0, wxEXPAND);
    // location
    entrySizer->Add(new wxStaticText(this, wxID_ANY, STRING_MANAGER_LOCATION, wxDefaultPosition, wxDefaultSize, wxALIGN_CENTRE),
                    0, wxALIGN_CENTER);
    mLocationTextCtrl = new wxTextCtrl(this, wxID_ANY);
    entrySizer->Add(mLocationTextCtrl, 0, wxEXPAND);
    // access
    entrySizer->Add(new wxStaticText(this, wxID_ANY, STRING_MANAGER_ACCESS, wxDefaultPosition, wxDefaultSize, wxALIGN_CENTRE),
                    0, wxALIGN_CENTER);
    mAccessComboBox = new wxComboBox(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, NULL, wxCB_READONLY);
    mAccessComboBox->Append(STRING_MANAGER_PUBLIC,    (void*)&mAccessStrs[0]);
    mAccessComboBox->Append(STRING_MANAGER_PRIVATE,   (void*)&mAccessStrs[1]);
    mAccessComboBox->Append(STRING_MANAGER_PROTECTED, (void*)&mAccessStrs[2]);
    entrySizer->Add(mAccessComboBox, 0, wxEXPAND);
    topSizer->Add(entrySizer, 1, wxEXPAND|wxALL, 5);

    wxSizer* btnSizer = CreateButtonSizer(wxOK|wxCANCEL);
    topSizer->Add(btnSizer, 0, wxEXPAND|wxALL, 5);

    SetIcon(wxGetApp().mFrame->mIconApp);
    SetSizer(topSizer);
    Fit();
    SetSize(300, GetSize().y);

    mTitleTextCtrl->ChangeValue(album);
    mSummaryTextCtrl->ChangeValue(summary);
    mLocationTextCtrl->ChangeValue(location);
    int select = mAccessStrs.Index(access);
    if(select != wxNOT_FOUND)    mAccessComboBox->SetSelection(select);
    FindWindow(wxID_OK)->Enable(album!=wxEmptyString);

    Connect(wxID_EDIT, wxEVT_COMMAND_TEXT_UPDATED,   wxCommandEventHandler(kuPicasaWebAlbumDialog::OnText));
    Connect(wxID_OK,   wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(kuPicasaWebAlbumDialog::OnOK));
}

void kuPicasaWebAlbumDialog::OnText(wxCommandEvent& event) {
    FindWindow(wxID_OK)->Enable(event.GetString()!=wxEmptyString);
}

void kuPicasaWebAlbumDialog::OnOK(wxCommandEvent& event) {
    int select = mAccessComboBox->GetSelection();
    if(select == wxNOT_FOUND) {
        // TODO: show message
        return;
    }
    EndModal(wxID_OK);
}

kuStrHashMap* kuPicasaWebAlbumDialog::GetValues() {
    kuStrHashMap* values = new kuStrHashMap();
    (*values)[STRING_PICASAWEB_PROP_TITLE]    = mTitleTextCtrl->GetValue();
    (*values)[STRING_PICASAWEB_PROP_SUMMARY]  = mSummaryTextCtrl->GetValue();
    (*values)[STRING_PICASAWEB_PROP_LOCATION] = mLocationTextCtrl->GetValue();
    (*values)[STRING_PICASAWEB_PROP_ACCESS]   = *((wxString*)mAccessComboBox->GetClientData(mAccessComboBox->GetSelection()));
    return values;
}
#endif

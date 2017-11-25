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

/* derived class have to implement CreateTask(), CreateThread(), GetTaskInfo(), GetErrorInfo(),
   and call Set() in constructor. */
kuManagerDialog::kuManagerDialog(kuFrame* frame, int id, wxString title)
    :wxDialog(frame, id, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER) {
    mFrame  = frame;
    mTitle  = title;
    mMaxCon = 1;
    mMaxCols     = 4;
    mColSrc      = 0;
    mColDest     = 1;
    mColProgress = 2;
    mColStatus   = 3;
    mCustomCols.Clear();
    mCustomHeaders.Clear();
}

bool kuManagerDialog::Set(int maxCon, wxArrayInt& defCols, wxArrayInt& customCols, wxArrayString& customHeaders) {
    if( (defCols.GetCount()!=mMaxCols && defCols.GetCount()!=0)
        || customCols.GetCount() != customHeaders.GetCount()
        || maxCon < 1 )
        return false;

    mMaxCon = maxCon;
    if(defCols.GetCount()) {
        mColSrc      = defCols[0];
        mColDest     = defCols[1];
        mColProgress = defCols[2];
        mColStatus   = defCols[3];
        mMaxCols = defCols.GetCount() + customHeaders.GetCount();
    } else {
        mMaxCols += customHeaders.GetCount();
    }
    mCustomCols    = customCols;
    mCustomHeaders = customHeaders;

    WX_CLEAR_ARRAY(mTasks);

    mTopSizer = new wxBoxSizer(wxVERTICAL);

    mGrid = new wxGrid(this, wxID_ANY);
    mGrid->CreateGrid(0, mMaxCols, wxGrid::wxGridSelectRows);
    SetupHeaders();
    mTopSizer->Add(mGrid, 1, wxEXPAND|wxTOP|wxLEFT|wxRIGHT, 5);

    mBtnSizer = NULL;
    SetupButtons();

    SetSizer(mTopSizer);

    SetupTitleAndSize();
    SetIcon(mFrame->mIconApp);

    mGrid->Connect(wxEVT_GRID_SELECT_CELL,      wxGridEventHandler(kuManagerDialog::OnGridSelectCell));
    mGrid->Connect(wxEVT_GRID_CELL_LEFT_DCLICK, wxGridEventHandler(kuManagerDialog::OnGridCellLeftDClick));

    Connect(kuID_THREAD_LOWEST, kuID_THREAD_HIGHEST, wxEVT_COMMAND_MENU_SELECTED,
            wxCommandEventHandler(kuManagerDialog::OnUpdateTask));

    Connect(wxEVT_SIZE, wxSizeEventHandler(kuManagerDialog::OnSize));

    return true;
}

kuManagedTask* kuManagerDialog::CreateTask() {
    return NULL;
}

wxThread* kuManagerDialog::CreateThread(kuManagedTask* task) {
    return NULL;
}

wxString kuManagerDialog::GetTaskInfo(kuManagedTask* task, int col) {
    return wxEmptyString;
}

wxString kuManagerDialog::GetErrorInfo(int errcode) {
    wxString errstr = wxEmptyString;
    switch (errcode) {
        case THREAD_ERROR_READFAIL:
            errstr = STRING_THREADERR_READFAIL;
            break;
        case THREAD_ERROR_READONLY:
            errstr = STRING_THREADERR_READONLY;
            break;
        case THREAD_ERROR_DISKFULL:
            errstr = STRING_THREADERR_DISKFULL;
            break;
        case THREAD_ERROR_UNKNOWN:
        default:
            errstr = STRING_THREADERR_UNKNOWN;
            break;
    }
    return errstr;
}

void kuManagerDialog::SetupHeaders() {
    mGrid->SetColLabelValue(mColSrc,      STRING_MANAGER_SRC);
    mGrid->SetColLabelValue(mColDest,     STRING_MANAGER_DEST);
    mGrid->SetColLabelValue(mColProgress, STRING_MANAGER_PROGRESS);
    mGrid->SetColLabelValue(mColStatus,   STRING_MANAGER_STATUS);
    for(size_t i=0; i<mCustomCols.GetCount(); i++) {
        mGrid->SetColLabelValue(mCustomCols[i], mCustomHeaders[i]);
    }
    mGrid->EnableEditing(false);
    mGrid->SetRowLabelSize(0);
    wxFont font = mGrid->GetLabelFont();
    font.SetWeight(wxFONTWEIGHT_NORMAL);
    mGrid->SetLabelFont(font);
}

void kuManagerDialog::SetupButtons() {
    if(mBtnSizer) {
        mBtnSizer->Clear(true);
        mTopSizer->Remove(mBtnSizer);
    }
    mBtnSizer = new wxBoxSizer(wxHORIZONTAL);
    mUpBtn = new wxBitmapButton(this, wxID_UP,   wxArtProvider::GetBitmap(wxART_GO_UP));
    mBtnSizer->Add(mUpBtn,   0, wxLEFT|wxRIGHT, 5);
    mDownBtn = new wxBitmapButton(this, wxID_DOWN, wxArtProvider::GetBitmap(wxART_GO_DOWN));
    mBtnSizer->Add(mDownBtn, 0, wxLEFT|wxRIGHT, 5);
    mBtnSizer->AddStretchSpacer();
    mBtnSizer->Add(new wxButton(this, kuID_TASK_NEW, STRING_TASKBTN_NEW), 0, wxLEFT|wxRIGHT, 5);
    mPauseBtn = new wxButton(this, kuID_TASK_PAUSE, STRING_TASKBTN_PAUSE);
    mBtnSizer->Add(mPauseBtn, 0, wxLEFT|wxRIGHT, 5);
    mCancelBtn = new wxButton(this, kuID_TASK_CANCEL, STRING_TASKBTN_CANCEL);
    mBtnSizer->Add(mCancelBtn, 0, wxLEFT|wxRIGHT, 5);
    mRemoveBtn = new wxButton(this, kuID_TASK_REMOVE, STRING_TASKBTN_REMOVE);
    mBtnSizer->Add(mRemoveBtn, 0, wxLEFT|wxRIGHT, 5);
    mBtnSizer->Add(new wxButton(this, wxID_CLOSE), 0, wxLEFT|wxRIGHT, 5);
    mTopSizer->Add(mBtnSizer, 0, wxEXPAND|wxALL, 5);
    SetTaskBtnStatus(false);

    SetAffirmativeId(wxID_CLOSE);
    // set escape id will block original binding of x button
    //SetEscapeId(wxID_CLOSE);
    Connect(wxID_UP,   wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(kuManagerDialog::OnUp));
    Connect(wxID_DOWN, wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(kuManagerDialog::OnDown));

    Connect(kuID_TASK_NEW,    wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(kuManagerDialog::OnNew));
    Connect(kuID_TASK_PAUSE,  wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(kuManagerDialog::OnPause));
    Connect(kuID_TASK_CANCEL, wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(kuManagerDialog::OnCancel));
    Connect(kuID_TASK_REMOVE, wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(kuManagerDialog::OnRemove));
    Connect(wxID_CLOSE,       wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(kuManagerDialog::OnClose));
}

void kuManagerDialog::SetupTitleAndSize() {
    SetTitle(mTitle);
    Fit();
    SetSize(GetSize().x, 300);
}

bool kuManagerDialog::Destroy() {
    for(size_t i=0; i<mTasks.GetCount(); i++) {
        if(mTasks[i]->Thread) {
            if(mTasks[i]->Thread->IsPaused())    mTasks[i]->Thread->Resume();
            mTasks[i]->Thread->Delete();
        }
    }
    WX_CLEAR_ARRAY(mTasks);
    return wxDialog::Destroy();
}

void kuManagerDialog::OnUp(wxCommandEvent& event) {
    int row = mGrid->GetGridCursorRow();
    if(row==0)    return;
    kuManagedTask* task = mTasks[row];
    mTasks.Remove(task);
    mTasks.Insert(task, row-1);
    mGrid->BeginBatch();
    mGrid->InsertRows(row-1);
    CopyRow(row+1, row-1);
    mGrid->DeleteRows(row+1);
    mGrid->MoveCursorUp(false);
    mGrid->SelectRow(row-1);
    mGrid->EndBatch();
    CheckConcurrentRun();
}

void kuManagerDialog::OnDown(wxCommandEvent& event) {
    int row = mGrid->GetGridCursorRow();
    if(row==mGrid->GetNumberRows()-1)    return;
    kuManagedTask* task = mTasks[row];
    mTasks.Remove(task);
    mTasks.Insert(task, row+1);
    mGrid->BeginBatch();
    mGrid->InsertRows(row+2);
    CopyRow(row, row+2);
    mGrid->DeleteRows(row);
    mGrid->MoveCursorDown(false);
    mGrid->SelectRow(row+1);
    mGrid->EndBatch();
    CheckConcurrentRun();
}

void kuManagerDialog::OnNew(wxCommandEvent& event) {
    // create task and set some fields to default
    kuManagedTask* task = CreateTask();
    if(!task)    return;
    task->Current = 0;
    task->Failed = 0;
    task->Thread = CreateThread(task);
    task->Queuing = false;
    task->Error = THREAD_ERROR_NONE;
    if(task->Thread->Create() != wxTHREAD_NO_ERROR) {
        wxMessageBox(wxT("failed to create thread"));
        delete task;
    } else {
        mTasks.Add(task);
        AppendRow(task);
        CheckConcurrentRun();
    }
}

void kuManagerDialog::OnPause(wxCommandEvent& event) {
    int row = mGrid->GetGridCursorRow();
    //wxMessageBox(wxString::Format(wxT("%d"), row));
    if(!mTasks[row]->Thread)    return;
    if(!mTasks[row]->Thread->IsAlive()) {    // queuing, not started yet
        mTasks[row]->Thread->SetPriority(30);
        mTasks[row]->Thread->Run();
        mTasks[row]->Thread->Pause();
        mTasks[row]->Queuing = true;
    }
    if(mTasks[row]->Thread->IsPaused()) {    // is paused or queuing
        if(mTasks[row]->Queuing) {    // is queuing, pause it
            mTasks[row]->Queuing = false;
            mPauseBtn->SetLabel(STRING_TASKBTN_RESUME);
            mGrid->SetCellValue(row, mColStatus, STRING_THREAD_PAUSED);
        } else {    // is paused, queuing it
            mTasks[row]->Queuing = true;
            mPauseBtn->SetLabel(STRING_TASKBTN_PAUSE);
            mGrid->SetCellValue(row, mColStatus, STRING_THREAD_QUEUING);
        }
    } else {    // is running, pause it
        mTasks[row]->Thread->Pause();
        mPauseBtn->SetLabel(STRING_TASKBTN_RESUME);
        mGrid->SetCellValue(row, mColStatus, STRING_THREAD_PAUSED);
    }
    CheckConcurrentRun();
}

void kuManagerDialog::OnCancel(wxCommandEvent& event) {
    int row = mGrid->GetGridCursorRow();
    if(!mTasks[row]->Thread)    return;
    mTasks[row]->Thread->Delete();
}

void kuManagerDialog::OnRemove(wxCommandEvent& event) {
    int row = mGrid->GetGridCursorRow();
    if(mTasks[row]->Thread)    return;
    mGrid->DeleteRows(row);
    mTasks.RemoveAt(row);
}

void kuManagerDialog::OnClose(wxCommandEvent& event) {
    Show(false);
}

void kuManagerDialog::OnGridSelectCell(wxGridEvent& event) {
    //wxMessageBox(wxString::Format(wxT("%d, %d"), event.GetRow(), event.GetCol()));
    size_t row = event.GetRow();
    wxGrid* grid = (wxGrid*)event.GetEventObject();
    kuManagerDialog* dialog = (kuManagerDialog*)grid->GetParent();
    if(row==0)
        dialog->SetMoveBtnStatus(false, true);
    else if(row==((wxGrid*)event.GetEventObject())->GetNumberRows()-1)
        dialog->SetMoveBtnStatus(true, false);
    else
        dialog->SetMoveBtnStatus(true, true);
    // have to get mTasks by wxGetApp()
    if(row<dialog->mTasks.GetCount() && dialog->mTasks[row]->Thread) {
        bool paused = dialog->mTasks[row]->Thread->IsPaused();
        if(paused && dialog->mTasks[row]->Queuing)    paused = false;
        dialog->SetTaskBtnStatus(true, paused);
    } else {
        dialog->SetTaskBtnStatus(false);
    }
    // let cursor be moved
    event.Skip();
}

void kuManagerDialog::OnGridCellLeftDClick(wxGridEvent& event) {
    size_t row = event.GetRow();
    wxGrid* grid = (wxGrid*)event.GetEventObject();
    kuManagerDialog* dialog = (kuManagerDialog*)grid->GetParent();
    wxString status = grid->GetCellValue(row,dialog->mColStatus);
    // open link
    #ifdef __WXMSW__
    wxString link = dialog->mTasks[row]->Link;
    if(link.Mid(0,7) == wxT("http://")) {
        ShellExecute(0, EXTERNAL_OPERATION_OPEN, link, 0, 0, SW_SHOW);
    } else {
        ShellExecute(0, 0, wxT("explorer"), wxT("/n,")+link, 0, SW_SHOW);
    }
    #endif
    // show error message
    if(status==STRING_THREAD_FAILED || status==STRING_THREAD_INCOMPLETE) {
        wxString err = GetErrorInfo(dialog->mTasks[row]->Error);
        wxMessageBox(err, status);
    }
    // let cursor be moved
    event.Skip();
}

void kuManagerDialog::OnSize(wxSizeEvent& event) {
    /*
    //wxMessageBox(wxString::Format(wxT("%d, %d"), mGrid->GetBestSize().x, mGrid->GetSize().x));
    int total = 0;
    for(int i=kuID_MANAGER_COL_SRC; i<kuID_MANAGER_COL_HIGHEST; i++) {
        total += mGrid->GetColSize(i);
    }
    //wxMessageBox(wxString::Format(wxT("total = %d"), total));
    int diff = mGrid->GetSize().x - total;
    if(diff>0) {
        //wxMessageBox(wxString::Format(wxT("diff = %d"), diff));
        mGrid->BeginBatch();
        mGrid->SetColSize(mGrid->GetColSize(kuID_MANAGER_COL_SRC) +diff/2, kuID_MANAGER_COL_SRC);
        mGrid->SetColSize(mGrid->GetColSize(kuID_MANAGER_COL_DEST)+diff/2, kuID_MANAGER_COL_DEST);
        mGrid->EndBatch();
    }
    */
    event.Skip();
}

void kuManagerDialog::AppendRow(kuManagedTask* task) {
    int row = mGrid->GetNumberRows();
    mGrid->BeginBatch();
    mGrid->AppendRows();
    mGrid->SetCellValue(row, mColSrc,      task->Src);
    mGrid->SetCellAlignment(row, mColSrc, wxALIGN_LEFT, wxALIGN_CENTRE);
    mGrid->SetCellValue(row, mColDest,     task->Dest);
    mGrid->SetCellAlignment(row, mColDest, wxALIGN_LEFT, wxALIGN_CENTRE);
    mGrid->SetCellValue(row, mColProgress, wxString::Format(FORMAT_MANAGER_PROGRESS, task->Current, task->Failed, task->Files.GetCount()));
    mGrid->SetCellRenderer(row, mColProgress, new kuGridCellProgressRender(mColStatus));
    mGrid->SetCellValue(row, mColStatus,   GetStatusString(task));
    mGrid->SetCellAlignment(row, mColStatus, wxALIGN_LEFT, wxALIGN_CENTRE);
    for(size_t i=0; i<mCustomCols.GetCount(); i++) {
        mGrid->SetCellValue(row, mCustomCols[i], GetTaskInfo(task, mCustomCols[i]));
        mGrid->SetCellAlignment(row, mCustomCols[i], wxALIGN_LEFT, wxALIGN_CENTRE);
    }
    mGrid->EndBatch();
}

void kuManagerDialog::CopyRow(int from, int to) {
    mGrid->SetCellValue(to, mColSrc,      mGrid->GetCellValue(from, mColSrc));
    mGrid->SetCellAlignment(to, mColSrc, wxALIGN_LEFT, wxALIGN_CENTRE);
    mGrid->SetCellValue(to, mColDest,     mGrid->GetCellValue(from, mColDest));
    mGrid->SetCellAlignment(to, mColDest, wxALIGN_LEFT, wxALIGN_CENTRE);
    mGrid->SetCellValue(to, mColProgress, mGrid->GetCellValue(from, mColProgress));
    mGrid->SetCellRenderer(to, mColProgress, new kuGridCellProgressRender(mColStatus));
    mGrid->SetCellValue(to, mColStatus,   mGrid->GetCellValue(from, mColStatus));
    mGrid->SetCellAlignment(to, mColStatus, wxALIGN_LEFT, wxALIGN_CENTRE);
    for(size_t i=0; i<mCustomCols.GetCount(); i++) {
        mGrid->SetCellValue(to, mCustomCols[i], mGrid->GetCellValue(from, mCustomCols[i]));
        mGrid->SetCellAlignment(to, mCustomCols[i], wxALIGN_LEFT, wxALIGN_CENTRE);
    }
}

void kuManagerDialog::OnUpdateTask(wxCommandEvent& event) {
    kuManagedTask* task = (kuManagedTask*)event.GetClientData();
    int row = mTasks.Index(task);
    if(row==wxNOT_FOUND)    return;
    mGrid->BeginBatch();
    switch (event.GetId()) {
        case kuID_THREAD_PROGRESS:
            mGrid->SetCellValue(row, mColProgress,
                                wxString::Format(FORMAT_MANAGER_PROGRESS, task->Current, task->Failed, task->Files.GetCount()));
            break;
        case kuID_THREAD_STATUS:
            mGrid->SetCellValue(row, mColStatus, GetStatusString(task));
            if(mTasks.Index(task)==mGrid->GetGridCursorRow()) {    // have to update button status
                SetTaskBtnStatus(false);
            }
            CheckConcurrentRun();
            break;
        default:
            break;
    }
    mGrid->EndBatch();
}

wxString kuManagerDialog::GetFilterString(int filter) {
    wxMenuItem* item = mFrame->GetMenuBar()->FindItem(kuID_RESCALE_BOX+filter);
    return item->GetItemLabelText();
}

wxString kuManagerDialog::GetStatusString(kuManagedTask* task) {
    if(task->Thread) {
        if(task->Thread->IsPaused()) {
            if(task->Queuing)    return STRING_THREAD_QUEUING;
            else    return STRING_THREAD_PAUSED;
        }
        else if(task->Thread->IsAlive())    return STRING_THREAD_RUNNING;
        else    return STRING_THREAD_QUEUING;
    } else {
        if(task->Current == task->Files.GetCount()) {
            if(task->Error == THREAD_ERROR_NONE)    return STRING_THREAD_COMPLETED;
            else    return STRING_THREAD_INCOMPLETE;
        } else {
            if(task->Error == THREAD_ERROR_NONE)    return STRING_THREAD_CANCELLED;
            else    return STRING_THREAD_FAILED;
        }
    }
}

void kuManagerDialog::SetMoveBtnStatus(bool up, bool down) {
    if(up && !mUpBtn->IsEnabled())         mUpBtn->Enable(true);
    else if(!up && mUpBtn->IsEnabled())    mUpBtn->Enable(false);
    if(down && !mDownBtn->IsEnabled())         mDownBtn->Enable(true);
    else if(!down && mDownBtn->IsEnabled())    mDownBtn->Enable(false);
}

void kuManagerDialog::SetTaskBtnStatus(bool running, bool paused) {
    mPauseBtn->Enable(running);
    mCancelBtn->Enable(running);
    mRemoveBtn->Enable(!running);
    if(paused)
        mPauseBtn->SetLabel(STRING_TASKBTN_RESUME);
    else
        mPauseBtn->SetLabel(STRING_TASKBTN_PAUSE);
}

bool kuManagerDialog::CheckConcurrentRun(int max) {
    if(max>0)    mMaxCon = max;
    bool found = false;
    int concurrent=0;
    for(size_t i=0;i<mTasks.GetCount(); i++) {
        if(mTasks[i]->Thread) {    // rinning or paused or queuing
            if(mTasks[i]->Thread->IsAlive()) {    // running or paused
                if(mTasks[i]->Thread->IsRunning()) {    // running
                    concurrent += 1;
                    if(concurrent > mMaxCon) {
                        mTasks[i]->Thread->Pause();
                        mTasks[i]->Queuing = true;
                        mGrid->SetCellValue(i, mColStatus, GetStatusString(mTasks[i]));
                        concurrent -= 1;
                    }
                } else {    // paused or queuing after paused
                    if(mTasks[i]->Queuing && concurrent<mMaxCon) {    // queuing after paused
                        mTasks[i]->Thread->Resume();
                        mTasks[i]->Queuing = false;
                        mGrid->SetCellValue(i, mColStatus, GetStatusString(mTasks[i]));
                        concurrent += 1;
                        found = true;
                    }
                }
            } else {    // queuing
                if(concurrent < mMaxCon) {
                    mTasks[i]->Thread->SetPriority(30);
                    mTasks[i]->Thread->Run();
                    mGrid->SetCellValue(i, mColStatus, GetStatusString(mTasks[i]));
                    concurrent += 1;
                    found = true;
                } else {
                    break;
                }
            }
        } else {    // completed or failed or cancelled
        }
    }
    return found;
}

// -------- kuGridCellProgressRender --------
kuGridCellProgressRender::kuGridCellProgressRender(int colStatus)
    :wxGridCellStringRenderer() {
    mColStatus = colStatus;
}

void kuGridCellProgressRender::Draw(wxGrid& grid, wxGridCellAttr& attr, wxDC& dc,
                              const wxRect& rect, int row, int col, bool isSelected) {
    wxGridCellRenderer::Draw(grid, attr, dc, rect, row, col, isSelected);
    wxString str = grid.GetCellValue(row, col);
    int current, failed, total;
    wxSscanf(str, FORMAT_MANAGER_PROGRESS, &current, &failed, &total);
    wxRect drect = rect;
    wxString status = grid.GetCellValue(row, mColStatus);
    if(status==STRING_THREAD_FAILED || status==STRING_THREAD_INCOMPLETE) {
        dc.GradientFillLinear(rect, wxColour(150,0,0), wxColour(250,50,50));
    } else {
        drect.width = drect.width*current/total;
        dc.GradientFillLinear(drect, wxColour(0,125,150), wxColour(0,200,250));
    }
    //wxGridCellStringRenderer::Draw(grid, attr, dc, rect, row, col, isSelected);
}

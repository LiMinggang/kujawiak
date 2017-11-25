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
kuPicasaWebThread::kuPicasaWebThread(kuPicasaWebTask* task, wxEvtHandler* handler)
    :wxThread() {
    mTask = task;
    mHandler = handler;
    wxString self = THREAD_NAME_CURRENT;
    wxLogDebug(wxT("%s kuPicasaWebThread: created..."), self.c_str());
}

void* kuPicasaWebThread::Entry() {
    wxString self = THREAD_NAME_CURRENT;

    mTask->Current = -1;
    mTask->Failed = 0;
    while(!TestDestroy()) {

        mTask->Current += 1;
        UpdateTask(kuID_THREAD_PROGRESS);

        if(mTask->Current >= mTask->Files.GetCount()) {    // all files are done
            break;
        }

        wxString filename = mTask->Files[mTask->Current];
        wxString type = wxFileName(filename).GetExt();
        if(type.Upper() == wxT("JPG"))    type = wxT("JPEG");

        bool success = false;
        // TODO: handle more errors
        switch (mTask->Action) {
            case THREAD_ACTION_UPLOAD:
                success = kuPwWrapper::AddPhoto(mTask->Album, filename, type.Lower(), mTask->Token);
                break;
            case THREAD_ACTION_DOWNLOAD:
                success = kuPwWrapper::GetMediaContent(mTask->Photos[mTask->Current], mTask->Dest, mTask->Token);
                break;
            case THREAD_ACTION_DELETE:
                success = kuPwWrapper::RemoveEntry(mTask->Token, mTask->Photos[mTask->Current]);
                break;
            default:
                break;
        }
        if(!success) {
            if(!success)    mTask->Error = THREAD_ERROR_UNKNOWN;
            mTask->Failed += 1;
            continue;
        }

        wxLogDebug(wxT("%s kuPicasaWebThread: completed %s"), self.c_str(), filename);
    }
    wxLogDebug(wxT("%s kuPicasaWebThread: exiting"), self.c_str());
    return NULL;
}

void kuPicasaWebThread::OnExit() {
    if((mTask->Current<mTask->Files.GetCount()) && mTask->Error==THREAD_ERROR_NONE) {    // it is cancelled or disk full
        // actually thread will exit directly when disk is full
        wxLongLong space = 0;
        wxGetDiskSpace(mTask->Dest, NULL, &space);
        if(space==0)    mTask->Error = THREAD_ERROR_DISKFULL;
    }
    // update status
    mTask->Thread = NULL;
    UpdateTask(kuID_THREAD_STATUS);
}

void kuPicasaWebThread::UpdateTask(int item) {
    wxCommandEvent event(wxEVT_COMMAND_MENU_SELECTED, item);
    event.SetClientData((void*)mTask);
    mHandler->AddPendingEvent(event);
}
#endif

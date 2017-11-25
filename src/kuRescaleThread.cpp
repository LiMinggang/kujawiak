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

kuRescaleThread::kuRescaleThread(kuRescaleTask* task, wxEvtHandler* handler)
    :wxThread() {
    mTask = task;
    mHandler = handler;
    wxString self = THREAD_NAME_CURRENT;
    wxLogDebug(wxT("%s kuRescaleThread: created..."), self.c_str());
}

void* kuRescaleThread::Entry() {
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

        FIBITMAP* orig = kuFiWrapper::GetFiBitmap(filename, false, wxSize(0,0), false);
        if(!orig) {    // failed to read source file
            mTask->Error = THREAD_ERROR_READFAIL;
            mTask->Failed += 1;
            continue;
        }

        FIBITMAP* bmp;
        if(FreeImage_GetWidth(orig) > FreeImage_GetHeight(orig))    // assume mTask->Size.x > mTask->Size.y
            bmp = FreeImage_Rescale(orig, mTask->Size.x, mTask->Size.y, mTask->Filter);
        else
            bmp = FreeImage_Rescale(orig, mTask->Size.y, mTask->Size.x, mTask->Filter);
        FreeImage_Unload(orig);
        if(!bmp) {    // failed to rescale bmp
            mTask->Error = THREAD_ERROR_RESCALEFAIL;
            mTask->Failed += 1;
            continue;
        }

        filename.Replace(mTask->Src, mTask->Dest);
        wxFileName target(filename);
        target.SetExt(mTask->Format);
        if(wxFileExists(target.GetFullPath())) {
            bool skip = false;
            switch (mTask->WhenExists) {
                case THREAD_WHENEXISTS_OVERWRITE:
                    break;
                case THREAD_WHENEXISTS_SKIP:
                    skip = true;
                    break;
                case THREAD_WHENEXISTS_RENAME:
                default:{
                    int idx = 1;
                    while(true) {
                        wxFileName fn = target;
                        fn.SetName(fn.GetName() + wxString::Format(wxT("_%d"), idx));
                        if(!wxFileExists(fn.GetFullPath())) {
                            target = fn;
                            break;
                        }
                        idx += 1;
                    }
                    break;
                }
            }
            if(skip) {
                FreeImage_Unload(bmp);
                continue;
            }
        }

        FreeImage_FlipVertical(bmp);
        wxFileName::Mkdir(target.GetPath(), 0777, wxPATH_MKDIR_FULL);
        bool success = SaveToFile(bmp, target.GetFullPath());
        FreeImage_Unload(bmp);
        if(success) {
            kuApp::SyncFileTime(mTask->Files[mTask->Current], target.GetFullPath());
        } else {    // failed to write target file
            if(!wxFileName::IsDirWritable(mTask->Dest)) {
                mTask->Error = THREAD_ERROR_READONLY;
                break;
            }
            wxString temp = wxFileName::CreateTempFileName(target.GetFullName());
            if(temp!=wxEmptyString) {
                success = SaveToFile(bmp, temp);
                if(success) {
                    kuApp::SyncFileTime(mTask->Files[mTask->Current], target.GetFullPath());
                    wxLongLong space;
                    wxGetDiskSpace(mTask->Dest, NULL, &space);
                    if(wxFileName::GetSize(temp).ToDouble() > space.ToDouble()) {
                        mTask->Error = THREAD_ERROR_DISKFULL;
                        break;
                    }
                }
            }
            mTask->Error = THREAD_ERROR_UNKNOWN;
            break;
        }

        wxLogDebug(wxT("%s kuRescaleThread: completed %s"), self.c_str(), target.GetFullPath());
    }
    wxLogDebug(wxT("%s kuRescaleThread: exiting"), self.c_str());
    return NULL;
}

bool kuRescaleThread::SaveToFile(FIBITMAP* bmp, wxString target) {
    int flags = 0;
    #ifdef __WXMSW__
    FREE_IMAGE_FORMAT fif = FreeImage_GetFIFFromFilenameU(target.wc_str(wxConvFile));
    bool success = FreeImage_SaveU(fif, bmp, target.wc_str(wxConvFile), flags);
    #else
    FREE_IMAGE_FORMAT fif = FreeImage_GetFIFFromFilename(target.mb_str(wxConvFile));
    bool success = FreeImage_Save(fif, bmp, target.mb_str(wxConvFile), flags);
    #endif
    return success;
}

void kuRescaleThread::OnExit() {
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

void kuRescaleThread::UpdateTask(int item) {
    wxCommandEvent event(wxEVT_COMMAND_MENU_SELECTED, item);
    event.SetClientData((void*)mTask);
    mHandler->AddPendingEvent(event);
}

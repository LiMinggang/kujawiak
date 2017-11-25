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

#include <wx/arrimpl.cpp>
WX_DEFINE_OBJARRAY(kuLoadArray);

int arraycompare(kuLoadTicket** arg1, kuLoadTicket** arg2) {
    return (*arg1)->Path.Cmp((*arg2)->Path);
}

kuLoadThread::kuLoadThread(kuFrame* frame)
    :wxThread() {
    mFrame = frame;
    mLoadQueue.Clear();
    mBmpHash.clear();
    mBmpCache.Clear();
    wxString self = THREAD_NAME_CURRENT;
    wxLogDebug(wxT("%s kuLoadThread: created..."), self.c_str());
}

void* kuLoadThread::Entry() {
    wxString self = THREAD_NAME_CURRENT;
    while(!TestDestroy()) {
        if(mLoadQueue.GetCount()) {    // has ticket to do
            // pop the first ticket
            mQueueMutex.Lock();
            kuLoadTicket ticket = mLoadQueue[0];
            mLoadQueue.RemoveAt(0);
            mQueueMutex.Unlock();

            // do the ticket
            // check if exists
            wxLogDebug(wxT("%s Entry: getting lock for check"), self.c_str());
            mBmpMutex.Lock();
            wxLogDebug(wxT("%s Entry: got lock for check"), self.c_str());
            if(mBmpHash.find(ticket.Path) != mBmpHash.end()) {    // has it already
                if(mBmpHash[ticket.Path].Size == ticket.Size) {
                   mBmpMutex.Unlock();
                   wxLogDebug(wxT("%s Entry: unlock"), self.c_str());
                   continue;
                } else {    // kill it since the size is incorrect
                    EraseBmpHash(ticket.Path);
                }
            }
            mBmpMutex.Unlock();
            wxLogDebug(wxT("%s Entry: unlock"), self.c_str());

            // load bmp
            wxLogDebug(wxT("%s Entry: %s, size=%dx%d"), self.c_str(), ticket.Path.wc_str(), ticket.Size.x, ticket.Size.y);
            FIBITMAP* bmp = kuFiWrapper::GetFiBitmap(ticket.Path, ticket.IsUrl, ticket.Size);
            FIBITMAP* tmp = Rotate(bmp, ticket.Rotate);
            if(tmp != bmp) {
                FreeImage_Unload(bmp);
                bmp = tmp;
            }
            kuBmpValue value;
            value.Size   = ticket.Size;
            value.Rotate = ticket.Rotate;
            value.Bmp    = bmp;

            // if queue is full, kill the first one
            wxLogDebug(wxT("%s Entry: getting lock for update"), self.c_str());
            mBmpMutex.Lock();
            wxLogDebug(wxT("%s Entry: got lock for update"), self.c_str());
            while(mBmpCache.GetCount() >= (size_t)wxGetApp().mOptions.mLoadCache) {
                wxLogDebug(wxT("%s Entry: remove %s"), self.c_str(), mBmpCache[0]);
                EraseBmpHash(mBmpCache[0]);
            }

            // put bmp into hash
            AddBmpHash(ticket.Path, value);
            mBmpMutex.Unlock();
            wxLogDebug(wxT("%s Entry: unlock"), self);

            // notify UI
            if(mFrame && !wxGetApp().mQuit) {
                wxCommandEvent event(wxEVT_LOADED_EVENT);
                event.SetString(ticket.Path);
				wxEvtHandler *fmevthdl = mFrame->GetEventHandler();
				fmevthdl->AddPendingEvent(event);
            }

        } else    wxThread::Sleep(200);
    }
    ClearBmpHash();
    wxGetApp().mWaitLoader = false;
    wxLogDebug(wxT("%s Entry: exiting"), self.c_str());
    return NULL;
}

/*
   clear bmps in hash
*/
void kuLoadThread::ClearBmpHash() {
    if(mBmpHash.empty())    return;

    wxString self = THREAD_NAME_CURRENT;
    wxLogDebug(wxT("%s ClearBmpHash: getting lock"), self.c_str());
    mBmpMutex.Lock();
    wxLogDebug(wxT("%s ClearBmpHash: got lock"), self.c_str());

    for(size_t i=0; i<mBmpCache.GetCount(); i++) {
        FIBITMAP* bmp = mBmpHash[mBmpCache[i]].Bmp;
        if(bmp)    FreeImage_Unload(bmp);
    }

    mBmpHash.clear();
    mBmpCache.Clear();

    mBmpMutex.Unlock();
    wxLogDebug(wxT("%s ClearBmpHash: unlock"), self.c_str());
}

/*
   add a bmp in hash
*/
void kuLoadThread::AddBmpHash(wxString filename, kuBmpValue& value) {
    wxString self = THREAD_NAME_CURRENT;
    // delete old one if exists in hash
    if(mBmpHash.find(filename) != mBmpHash.end()) {
        EraseBmpHash(filename);
    }
    mBmpHash[filename] = value;

    // add to cache
    if(mBmpCache.Index(filename) != wxNOT_FOUND) {
        wxLogDebug(wxT("%s AddBmpHash: has it in cache already?!"), self.c_str());
    } else    mBmpCache.Add(filename);

    wxLogDebug(wxT("%s AddBmpHash: added! #hash=%d, #cache=%d"), self.c_str(), int(mBmpHash.size()), int(mBmpCache.GetCount()));
}

/*
   erase a bmp in hash
*/
void kuLoadThread::EraseBmpHash(wxString filename) {
    if(mBmpHash.find(filename) == mBmpHash.end())    return;

    wxString self = THREAD_NAME_CURRENT;
    FIBITMAP* bmp = mBmpHash[filename].Bmp;
    // unload it if exists
    if(bmp) {
        if(mBmpCache.Index(filename) == wxNOT_FOUND)    wxLogDebug(wxT("%s EraseBmpHash: not in cache?!"), self.c_str());
        FreeImage_Unload(bmp);
        wxLogDebug(wxT("%s EraseBmpHash: erased! #hash=%d, #cache=%d"), self.c_str(), mBmpHash.size(), mBmpCache.GetCount());
    } else {
        wxLogDebug(wxT("%s EraseBmpHash: is null?!"), self.c_str());
    }
    // erase it
    mBmpHash.erase(filename);

    // remove from cache
    mBmpCache.Remove(filename);
    while(mBmpCache.Index(filename) != wxNOT_FOUND) {    // sometimes it is not removed...
        wxLogDebug(wxT("%s EraseBmpHash: is not removed from cache?!"), self.c_str());
        mBmpCache.Remove(filename);
    }
}

/*
   Clear() is used by UI thread
   clear ticket queue
*/
bool kuLoadThread::Clear(bool prior) {
    mQueueMutex.Lock();
    if(prior) {
        size_t idx = 0;
        while(idx < mLoadQueue.GetCount()) {
            if(mLoadQueue[idx].Prior)    idx += 1;
            else    mLoadQueue.RemoveAt(idx);
        }
    } else {
        mLoadQueue.Clear();
    }
    mQueueMutex.Unlock();
    return true;
}

/*
   Append() is used by UI thread
   append ticket to prefetch bmp
*/
bool kuLoadThread::Append(wxString filename, bool isurl, wxSize size, int rotate) {
    wxString self = THREAD_NAME_CURRENT;
    wxLogDebug(wxT("%s Append: %s, size=%dx%d"), self.c_str(), filename.wc_str(), size.x, size.y);
    // create ticket
    kuLoadTicket ticket;
    ticket.Path   = filename;
    ticket.IsUrl  = isurl;
    ticket.Size   = size;
    ticket.Rotate = rotate;
    ticket.Prior  = false;
    // append ticket to queue
    mQueueMutex.Lock();
    mLoadQueue.Add(ticket);
    mQueueMutex.Unlock();
    return true;
}

/*
   Prepend() is used by UI thread
   prepend ticket to prefetch bmp
*/
bool kuLoadThread::Prepend(wxString filename, bool isurl, wxSize size, int rotate) {
    wxString self = THREAD_NAME_CURRENT;
    wxLogDebug(self + wxString::Format(wxT("Prepend: %s, size=%dx%d"), filename.wc_str(), size.x, size.y));
    // create ticket
    kuLoadTicket ticket;
    ticket.Path   = filename;
    ticket.IsUrl  = isurl;
    ticket.Size   = size;
    ticket.Rotate = rotate;
    ticket.Prior  = true;
    // append ticket to queue
    mQueueMutex.Lock();
    mLoadQueue.Insert(ticket, 0);
    mQueueMutex.Unlock();
    return true;
}

/*
   Remove() is used by UI thread
   after rotate bmp and save to file, should remove the one in hash.
*/
bool kuLoadThread::Remove(wxString filename) {
    if(mBmpHash.find(filename) == mBmpHash.end())    return false;

    wxString self = THREAD_NAME_CURRENT;
    wxLogDebug(wxT("%s Remove: getting lock"), self.c_str());
    mBmpMutex.Lock();
    wxLogDebug(wxT("%s Remove: got lock"), self.c_str());
    EraseBmpHash(filename);
    mBmpMutex.Unlock();
    wxLogDebug(wxT("%s Remove: unlock"), self.c_str());
    return true;
}

/*
   Replace() is used by UI thread
   after rotate/scale bmp, should replace the one in hash.
*/
bool kuLoadThread::Replace(wxString filename, FIBITMAP* bmp, bool isrdiff, int rotate) {
    if(mBmpHash.find(filename) == mBmpHash.end())    return false;

    wxString self = THREAD_NAME_CURRENT;
    wxLogDebug(wxT("%s Replace: getting lock"), self.c_str());
    mBmpMutex.Lock();
    wxLogDebug(wxT("%s Replace: got lock"), self.c_str());
    FIBITMAP* tmp = mBmpHash[filename].Bmp;
    // unload it if exists
    if(tmp)    FreeImage_Unload(tmp);
    // clone it
    mBmpHash[filename].Bmp = FreeImage_Clone(bmp);
    if(isrdiff) {
        mBmpHash[filename].Rotate += rotate;
        mBmpHash[filename].Rotate %= 4;
    } else    mBmpHash[filename].Rotate = rotate;
    mBmpMutex.Unlock();
    wxLogDebug(wxT("%s Replace: unlock"), self.c_str());
    return true;
}

/*
   GetFiBitmap() is used by UI thread
   try to get it from hash. load and put into hash if not found.
*/
FIBITMAP* kuLoadThread::GetFiBitmap(wxString filename, bool isurl, wxSize size, int rotate, int fast) {
    FIBITMAP* bmp = NULL;
    bool hit = false;

    wxString self = THREAD_NAME_CURRENT;
    wxLogDebug(wxT("%s GetFiBitmap: getting %s, size=%dx%d, fast=%d"), self.c_str(), filename.wc_str(), size.x, size.y, fast);

    // search it
    wxLogDebug(wxT("%s GetFiBitmap: getting lock for check"), self.c_str());
    mBmpMutex.Lock();
    wxLogDebug(wxT("%s GetFiBitmap: got lock for check"), self.c_str());
    if(mBmpHash.find(filename) != mBmpHash.end()) {
        // kill old if its size is not correct
        if(mBmpHash[filename].Size == size)    hit = true;
        else    EraseBmpHash(filename);
    }

    if(hit) {
        wxLogDebug(wxT("%s GetFiBitmap: hit! %s"), self.c_str(), filename.c_str());
        wxLogDebug(wxT("%s GetFiBitmap: mRotate=%d, rotate=%d, Rotate=%d"), self.c_str(), wxGetApp().mOptions.mRotate, rotate, mBmpHash[filename].Rotate);
        bmp = mBmpHash[filename].Bmp;
        // rotate it
        int rdiff = rotate - mBmpHash[filename].Rotate;
        FIBITMAP* tmp = Rotate(bmp, rdiff);
        if(tmp != bmp) {
            //Replace(filename, tmp, true, rdiff);
            mBmpHash[filename].Bmp = tmp;
            mBmpHash[filename].Rotate += rdiff;
            mBmpHash[filename].Rotate %= 4;
            if(bmp)    FreeImage_Unload(bmp);
            bmp = tmp;
        }
        // move it to end in cache
        mBmpCache.Remove(filename);
        mBmpCache.Add(filename);
    } else {
        mBmpMutex.Unlock();
        wxLogDebug(wxT("%s GetFiBitmap: unlock"), self.c_str());
        wxLogDebug(wxT("%s GetFiBitmap: miss! hash_size=%d"), self.c_str(), int(mBmpHash.size()));
        // load it
        if(fast)    Prepend(filename, isurl, size, rotate);
        bmp = kuFiWrapper::GetFiBitmap(filename, isurl, size, fast);
        // rotate it
        FIBITMAP* tmp = Rotate(bmp, rotate);
        if(tmp != bmp) {
            FreeImage_Unload(bmp);
            bmp = tmp;
        }

        if(!fast) {
            kuBmpValue value;
            value.Size   = size;
            value.Rotate = rotate;
            value.Bmp    = bmp;

            // put it
            wxLogDebug(wxT("%s GetFiBitmap: getting lock for update"), self.c_str());
            mBmpMutex.Lock();
            wxLogDebug(wxT("%s GetFiBitmap: got lock for update"), self.c_str());
            AddBmpHash(filename, value);
        }
    }
    /* clone if it is managed by cache */
    FIBITMAP* clone;
    if(hit || !fast) {
        clone = FreeImage_Clone(bmp);
        mBmpMutex.Unlock();
        wxLogDebug(wxT("%s GetFiBitmap: unlock"), self.c_str());
    } else {
        clone = bmp;
    }
    // return a clone bmp. UI thread have to manage by itself
    return clone;
}

FIBITMAP* kuLoadThread::Rotate(FIBITMAP* bmp, int rotate) {
    if(!bmp)    return NULL;

    FIBITMAP* tmp = NULL;
    rotate %= 4;
    if(rotate<0)    rotate += 4;
    switch (rotate) {
        case 1:
            tmp = FreeImage_RotateClassic(bmp,90);
            break;
        case 2:
            tmp = FreeImage_RotateClassic(bmp,180);
            break;
        case 3:
            tmp = FreeImage_RotateClassic(bmp,270);
            break;
        case 0:
        default:
            return bmp;
    }
    return tmp;
}

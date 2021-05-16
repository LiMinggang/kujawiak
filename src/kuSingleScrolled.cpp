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

BEGIN_EVENT_TABLE(kuSingleScrolled,wxScrolledWindow)
    EVT_SIZE(kuSingleScrolled::OnSize)
    EVT_MOTION(kuSingleScrolled::OnMotion)
    EVT_LEAVE_WINDOW(kuSingleScrolled::OnLeaveWindow)
    EVT_LEFT_DOWN(kuSingleScrolled::OnLeftDown)
    EVT_LEFT_UP(kuSingleScrolled::OnLeftUp)
    EVT_KEY_DOWN(kuSingleScrolled::OnKeyDown)
    EVT_MOUSEWHEEL(kuSingleScrolled::OnMousewheel)
    EVT_CONTEXT_MENU(kuSingleScrolled::OnContextMenu)
    EVT_TIMER(kuID_TIMER_IDLE, kuSingleScrolled::OnIdleTimer)
END_EVENT_TABLE()

// -------- kuSingleScrolled --------
kuSingleScrolled::kuSingleScrolled(wxWindow* parent, kuFrame* frame)
     :wxScrolledWindow(parent,wxID_ANY), mIdleTimer(this, kuID_TIMER_IDLE) {
    SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNFACE));
    mFrame = frame;
    mOrigBmp = NULL;
    SetupPopupMenu();
    SetDropTarget(new kuScrolledDropTarget(mFrame));
}

kuSingleScrolled::~kuSingleScrolled() {
    if(mOrigBmp)    FreeImage_Unload(mOrigBmp);
};

void kuSingleScrolled::SetupPopupMenu() {
    /* don't delete it manually
    if(mMenu)   delete mMenu;
    */
    wxMenu* fileMenu = new wxMenu();
    fileMenu->Append(kuID_SAVE,       STRING_MENU_SAVE);
    fileMenu->Append(kuID_SAVEAS,     STRING_MENU_SAVEAS);
    fileMenu->AppendSeparator();
    fileMenu->Append(kuID_PREVIEW,    STRING_MENU_PAGESETUP);
    fileMenu->Append(kuID_PREVIEW,    STRING_MENU_PREVIEW);
    fileMenu->Append(kuID_PRINT,      STRING_MENU_PRINT);
    fileMenu->Append(kuID_RELOAD,     STRING_MENU_RELOAD);
    wxMenu* editMenu = new wxMenu();
    editMenu->Append(kuID_ROTATE_CCW, STRING_MENU_ROTATE_CCW);
    editMenu->Append(kuID_ROTATE_CW,  STRING_MENU_ROTATE_CW);
    editMenu->Append(kuID_RESCALE,STRING_MENU_RESCALE);
    wxMenu* viewMenu = new wxMenu();
    viewMenu->Append(kuID_ZOOM_IN,    STRING_MENU_ZOOM_IN);
    viewMenu->Append(kuID_ZOOM_OUT,   STRING_MENU_ZOOM_OUT);
    viewMenu->Append(kuID_ZOOM_100,   STRING_MENU_ZOOM_100);
    viewMenu->Append(kuID_ZOOM_FIT,   STRING_MENU_ZOOM_FIT);
    viewMenu->Append(kuID_ZOOM_EXT,   STRING_MENU_ZOOM_EXT);
    if(mFrame->CanSetTransparent()) {
        viewMenu->AppendSeparator();
        viewMenu->Append(kuID_OPAQUE, STRING_MENU_OPAQUE);
    }
    mMenu = new wxMenu();
    mMenu->Append(kuID_SLIDESHOW,  STRING_MENU_SLIDESHOW_START);
    mMenu->AppendSeparator();
    mMenu->Append(kuID_PREV,   STRING_MENU_PREV);
    mMenu->Append(kuID_NEXT,   STRING_MENU_NEXT);
    mMenu->Append(kuID_HOME,   STRING_MENU_HOME);
    mMenu->Append(kuID_END,    STRING_MENU_END);
    mMenu->AppendSeparator();
    mMenu->Append(kuID_FILE,       STRING_MENU_FILE,     fileMenu);
    mMenu->Append(kuID_EDIT,       STRING_MENU_EDIT,     editMenu);
    mMenu->Append(kuID_VIEW,       STRING_MENU_VIEW,     viewMenu);
    mMenu->AppendSeparator();
    mMenu->Append(kuID_EXPLORE,        STRING_MENU_EXPLORE);
    #ifdef __WXMSW__
    mMenu->Append(kuID_EXTERNAL_EDIT,  STRING_MENU_EXTERNAL_EDIT);
    #endif
    //mMenu->Append(kuID_EXTERNAL_OPEN,  STRING_MENU_EXTERNAL_OPEN);
    mMenu->AppendSeparator();
    #ifdef __WXMSW__
    mMenu->Append(kuID_WALLPAPER,  STRING_MENU_DESKTOP_WALLPAPER);
    mMenu->Append(kuID_NONEWP,     STRING_MENU_DESKTOP_NONEWP);
    mMenu->Append(kuID_BACKGROUND, STRING_MENU_DESKTOP_BACKGROUND);
    mMenu->AppendSeparator();
    mMenu->Append(kuID_PROPERTIES,       STRING_MENU_PROPERTIES);
    #endif
    mMenu->Append(kuID_METADATA,         STRING_MENU_METADATA);
    mMenu->AppendCheckItem(kuID_SUMMARY, STRING_MENU_SUMMARY);
    mMenu->AppendSeparator();
    mMenu->Append(wxID_EXIT,       STRING_MENU_EXIT);
}

void kuSingleScrolled::ClearExternalOpenMenu() {
    size_t old = wxGetApp().mOptions.mExternalOpenNames.GetCount();
    for(size_t i=0; i<old; i++) {
        if(mMenu->FindItem(kuID_EXTERNAL_START+i)) {
            mMenu->Destroy(kuID_EXTERNAL_START+i);
        }
    }
}
void kuSingleScrolled::SetupExternalOpenMenu() {
    // TODO: find the position after STRING_MENU_EXTERNAL_EDIT
    #ifdef __WXMSW__
    int itemidx = 13;
    #else
    int itemidx = 12;    // no edit
    #endif
    size_t count = wxGetApp().mOptions.mExternalOpenNames.GetCount();
    for(size_t i=0; i<count; i++) {
        wxString str = wxString(STRING_MENU_EXTERNAL_OPEN);
        str.Replace(wxT("%TOOL%"), wxGetApp().mOptions.mExternalOpenNames[i]);
        mMenu->Insert(itemidx+i, kuID_EXTERNAL_START+i, str);
    }
}

void kuSingleScrolled::OnDraw(wxDC& dc) {
    if(mDispImg.Ok()) {
        mLeftTop = wxPoint(0,0);
        int width = mDispSize.x;
        int height = mDispSize.y;
        int xratio = width/GetSize().x;
        int yratio = height/GetSize().y;
        if(xratio<1)    mLeftTop.x = (GetSize().x-width)/2;
        if(yratio<1)    mLeftTop.y = (GetSize().y-height)/2;
        //wxMessageBox(wxString::Format(wxT("ClientSize: %dx%d"),GetSize().x,GetSize().y));
        //wxMessageBox(wxString::Format(wxT("mLeftTop: %dx%d"),mLeftTop.x,mLeftTop.y));
        mRightBottom = wxPoint(mLeftTop.x+width, mLeftTop.y+height);
        if(mFrame->IsFullScreen()) {
            if(mFrame->mIsStatusbar) {
                wxRect view = mView;
                if(GetPosition().y + view.y + view.height > mFrame->GetStatusBar()->GetPosition().y)
                    view.height = mFrame->GetStatusBar()->GetPosition().y - view.y - GetPosition().y;
                dc.DrawBitmap(wxBitmap(mDispImg.GetSubImage(view)),
                              mLeftTop.x, mLeftTop.y, true);
                mFrame->GetStatusBar()->Refresh();
            } else {
                dc.DrawBitmap(wxBitmap(mDispImg.GetSubImage(mView)),
                              mLeftTop.x, mLeftTop.y, true);
            }
        } else {
            dc.DrawBitmap(wxBitmap(mDispImg),
                          mLeftTop.x, mLeftTop.y, true);
        }
        if(mFrame->mIsSummary)    DrawSummary(dc);
    }
}

void kuSingleScrolled::DrawSummary(wxDC& dc) {
    dc.SetTextForeground(wxGetApp().mOptions.mSummaryColor);
    wxPoint start = wxGetApp().mOptions.mSummaryPosition;
    int incr;
    dc.GetTextExtent(wxT("dp"), NULL, &incr);
    incr *=1.1;
    int maxh = start.y;
    int maxw = 0;
    int count = 0;
    int len = 0;
    FITAG* tag = NULL;
    for(size_t i=0; i<FIMD_CUSTOM; i++) {
        for(size_t j=0; j<wxGetApp().mOptions.mSummaryTags[i].GetCount(); j++) {
            FreeImage_GetMetadata((FREE_IMAGE_MDMODEL)i, mOrigBmp, wxGetApp().mOptions.mSummaryTags[i][j].mb_str(wxConvUTF8), &tag);
            if(tag) {
                wxString str = wxGetApp().mOptions.mSummaryTags[i][j] + wxT(": ")
                               + wxString(FreeImage_TagToString(FIMD_EXIF_MAIN, tag), wxConvUTF8);
                dc.DrawText(str, start.x, maxh);
                dc.GetTextExtent(str, &len, NULL);
                if(len > maxw)    maxw = len;
                count += 1;
                maxh += incr;
            }
        }
    }
    mSummaryRect = wxRect(start.x, start.y, maxw, maxh);
}

void kuSingleScrolled::ClearSummary() {
    RefreshRect(mSummaryRect);
}

void kuSingleScrolled::ReloadImage(wxString filename, bool isurl, bool loaded) {
    mFilename = filename;
    mIsUrl = isurl;
    if(filename==wxEmptyString)   return;

    wxBeginBusyCursor();
    if(mOrigBmp)    FreeImage_Unload(mOrigBmp);

    wxSize size = wxGetApp().mOptions.mLoadCompletely ? wxSize(0,0) : GetClientSize();
    int fast = loaded ? 0 : 64;
    //wxMessageBox(wxString::Format(wxT("size = %d, fast = %d"), size, fast));

    // keep rotate
    int rotate;
    if(wxGetApp().mOptions.mKeepRotate)     rotate = wxGetApp().mOptions.mRotate;
    else    rotate = wxGetApp().mOptions.mRotate = 0;

    wxStopWatch sw;
    if(loaded && !mCropRect.IsEmpty()) {
        mOrigBmp = wxGetApp().mLoader->GetFiBitmap(filename, isurl, wxSize(0,0), rotate);
        mOrigSize = wxSize(FreeImage_GetWidth(mOrigBmp), FreeImage_GetHeight(mOrigBmp));
        DrawCropRect(true);
        wxEndBusyCursor();
        return;
    } else {
        mOrigBmp = wxGetApp().mLoader->GetFiBitmap(filename, isurl, size, rotate, fast);
        mOrigSize = wxSize(FreeImage_GetWidth(mOrigBmp), FreeImage_GetHeight(mOrigBmp));
        mCropRect = wxRect(0,0,0,0);
    }
    mDispSize = mOrigSize;
    //wxMessageBox(wxString::Format(wxT("mOrigSize: %dx%d"),mOrigSize.x,mOrigSize.y));
    //wxMessageBox(wxString::Format(wxT("GetSize: %dx%d"),GetSize().x,GetSize().y));

    // always set edited to false whether load completely or not
    wxGetApp().SetEdited(false);

    if(mOrigBmp) {
        mScale = SCALE_BASE;
        mDragStart = wxPoint(0,0);
        // restore scale/lefttop
        SetScale(wxGetApp().mOptions.mKeepStyle, !loaded);

        float time = sw.Time()/float(1000);

        if(isurl)   mFrame->SetStatusText(filename.AfterLast(':'));
        else   mFrame->SetStatusText(filename);
        mFrame->SetStatusText(kuApp::GetReadableSize(filename), STATUS_FIELD_FILESIZE);
        mFrame->SetStatusText(GetBmpSizeString(fast), STATUS_FIELD_BMPSIZE);
        mFrame->SetStatusText(wxString::Format(wxT("%.3f s"),time), STATUS_FIELD_TIME);
    }
    //if(mFrame->mIsThumbnail) {
    //    mFrame->mMultiple->Select(filename, true, false);
    //}
    wxEndBusyCursor();
}

bool kuSingleScrolled::DrawCropRect(bool loaded) {
    wxRect rect = wxRect(mCropRect.GetLeft()*mOrigSize.x/SCALE_BASE, mCropRect.GetTop()*mOrigSize.y/SCALE_BASE,
                         mCropRect.GetWidth()*mOrigSize.x/SCALE_BASE, mCropRect.GetHeight()*mOrigSize.y/SCALE_BASE);
    if(rect.IsEmpty())    return false;
    /* have to flip to get correct top */
    FreeImage_FlipVertical(mOrigBmp);
    FIBITMAP* tmp = FreeImage_Copy(mOrigBmp, rect.GetLeft(), rect.GetTop(), rect.GetRight(), rect.GetBottom());
    if(!tmp || FreeImage_GetWidth(tmp)==0 || FreeImage_GetHeight(tmp)==0) {
        if(tmp)    FreeImage_Unload(tmp);
        FreeImage_FlipVertical(mOrigBmp);
        return false;
    }
    FreeImage_FlipVertical(tmp);
    FreeImage_Unload(mOrigBmp);
    mOrigBmp = tmp;
    mOrigSize = wxSize(FreeImage_GetWidth(mOrigBmp), FreeImage_GetHeight(mOrigBmp));
    return SetScale(SCALE_BESTFIT,!loaded);
}

wxString kuSingleScrolled::GetBmpSizeString(int fast) {
    wxString ret = wxEmptyString;
    int depth = FreeImage_GetBPP(mOrigBmp);
    if(wxGetApp().mOptions.mLoadCompletely && !fast) {    // it is real size
        ret = wxString::Format(FORMAT_BMPSIZE, mOrigSize.x, mOrigSize.y, depth);
    } else {
        // get size from comments
        FITAG* tagw = NULL;
        FITAG* tagh = NULL;
        FreeImage_GetMetadata(FIMD_COMMENTS, mOrigBmp, "OriginalJPEGWidth", &tagw);
        FreeImage_GetMetadata(FIMD_COMMENTS, mOrigBmp, "OriginalJPEGHeight", &tagh);
        if(!tagw || !tagh) {
            ret = wxString::Format(FORMAT_BMPSIZE, mOrigSize.x, mOrigSize.y, depth);
        } else {
            ret = wxString(FreeImage_TagToString(FIMD_COMMENTS, tagw),wxConvUTF8)
                  + wxT("x")
                  + wxString(FreeImage_TagToString(FIMD_COMMENTS, tagh),wxConvUTF8)
                  + wxString::Format(wxT("x%d"),depth);
        }
    }
    return ret;
}

bool kuSingleScrolled::SaveImage(wxString filename, bool ask) {
    if(filename==wxEmptyString)    return false;
    if(mOrigBmp) {
        FREE_IMAGE_FORMAT fif = kuFiWrapper::GetImageFormat(filename, false);
        FREE_IMAGE_FORMAT src = kuFiWrapper::GetImageFormat(mFilename);
        if(ask && wxFileName(filename)==wxFileName(mFilename) && wxMessageBox(STRING_ORIGINAL_MESSAGE,kuFrame::StripCodes(STRING_MENU_SAVE),wxOK|wxCANCEL|wxICON_EXCLAMATION)
           ==wxCANCEL) {
            mFrame->SetStatusText(STRING_INFO_CANCELED);
            return false;
        }
        // it may fail when the filename doesn't match locale, just let it falls to original method...
        if(ask && wxMessageBox(STRING_SAVE_MESSAGE,kuFrame::StripCodes(STRING_MENU_SAVE),wxOK|wxCANCEL|wxICON_EXCLAMATION)
           ==wxCANCEL) {
            mFrame->SetStatusText(STRING_INFO_CANCELED);
            return false;
        }
        if(fif!=FIF_UNKNOWN) {
            wxGetApp().CheckEdited();
            FreeImage_FlipVertical(mOrigBmp);
            int flags = 0;
            //if(fif==FIF_JPEG)    flags = JPEG_QUALITYSUPERB;
            #ifdef __WXMSW__
            bool success = FreeImage_SaveU(fif,mOrigBmp,filename.wc_str(wxConvFile),flags);
            #else
            bool success = FreeImage_Save(fif,mOrigBmp,filename.mb_str(wxConvFile),flags);
            #endif
            FreeImage_FlipVertical(mOrigBmp);
            if(success) {
                kuApp::SyncFileTime(mFilename, filename);
                mFrame->SetStatusText(STRING_INFO_SAVED+filename);
                wxGetApp().mLoader->Replace(mFilename, mOrigBmp, false, 0);
                return true;
            }
        }
    } else if(ask && wxMessageBox(STRING_SAVE_MESSAGE,kuFrame::StripCodes(STRING_MENU_SAVE),wxOK|wxCANCEL|wxICON_EXCLAMATION)
       ==wxCANCEL) {
        mFrame->SetStatusText(STRING_INFO_CANCELED);
    }
    return false;
}

wxString kuSingleScrolled::GetFilename() {
    return mFilename;
}

wxSize kuSingleScrolled::GetOrigSize(bool real) {
    if(real) {
        wxSize size = kuFiWrapper::GetOriginalJPEGSize(mOrigBmp);
        if(size != wxSize(0,0))    return size;
    }
    return mOrigSize;
}

FIBITMAP* kuSingleScrolled::GetOrigBmp() {
    if(mOrigBmp) {
        /*
        if(!wxGetApp().mOptions.mLoadCompletely) {
            wxGetApp().mOptions.mLoadCompletely = true;
            ReloadImage(mFilename,false);
            wxGetApp().mOptions.mLoadCompletely = false;
        }
        */
        return mOrigBmp;
    }
    return NULL;
}

void kuSingleScrolled::Rotate90(bool cw) {
    if(!mOrigBmp)    return;
    bool lossless = false;
    FREE_IMAGE_FORMAT fif = kuFiWrapper::GetImageFormat(mFilename);
    if(!wxGetApp().GetEdited() && fif==FIF_JPEG && wxFileName::IsFileWritable(mFilename)) {
        #ifdef __WXMSW__
        lossless = FreeImage_JPEGTransformU(mFilename.wc_str(wxConvFile), mFilename.wc_str(wxConvFile), cw?FIJPEG_OP_ROTATE_90:FIJPEG_OP_ROTATE_270, TRUE);
        #else
        lossless = FreeImage_JPEGTransform(mFilename.mb_str(wxConvFile), mFilename.mb_str(wxConvFile), cw?FIJPEG_OP_ROTATE_90:FIJPEG_OP_ROTATE_270, TRUE);
        #endif
    }
    FIBITMAP* dst = FreeImage_Rotate(mOrigBmp,cw?90:-90);
    if(dst) {
        if(mOrigBmp)    FreeImage_Unload(mOrigBmp);
        mOrigBmp = dst;
        mOrigSize = wxSize(FreeImage_GetWidth(mOrigBmp), FreeImage_GetHeight(mOrigBmp));
    }
    SetScale(SCALE_LASTUSED);
    mDispSize = wxSize(mDispImg.GetWidth(),mDispImg.GetHeight());
    mLeftTop = wxPoint(0,0);
    CheckViewSize();
    CheckViewStart();
    if(lossless)    wxGetApp().mLoader->Replace(mFilename, mOrigBmp, false, 0);
    else    wxGetApp().SetEdited(true);
}

void kuSingleScrolled::OnSize(wxSizeEvent& event) {
    /* auto zoom it for user. but looks worse than reload when not load completely. */
    if(wxApp::IsMainLoopRunning() && !wxGetApp().mQuit && mFrame->GetCurrentPath(true,true)!=wxEmptyString && wxGetApp().mAutoZoom)    SetScale(SCALE_AUTOFIT);
    else    Refresh();
    //wxMessageBox(wxString::Format(wxT("onSize: %d x %d"), GetSize().x, GetSize().y));
}

void kuSingleScrolled::OnMotion(wxMouseEvent& event) {
    mIdleTimer.Stop();
    if(!event.LeftIsDown())    HideCursor(false);
    if(mDispImg.Ok()) {
        if(event.GetX()>mLeftTop.x && event.GetX()<mRightBottom.x
           && event.GetY()>mLeftTop.y && event.GetY()<mRightBottom.y) {
           mFrame->SetStatusText(wxString::Format(wxT("%d,%d / %d,%d"),
                                                  event.GetX()-mLeftTop.x,
                                                  event.GetY()-mLeftTop.y,
                                                  mDispSize.x,
                                                  mDispSize.y),
                                 STATUS_FIELD_MOTION);
        }
        else   mFrame->SetStatusText(wxEmptyString,STATUS_FIELD_MOTION);
    }
    if(mFrame->IsFullScreen())    mIdleTimer.Start(3000, true);
}

void kuSingleScrolled::OnLeaveWindow(wxMouseEvent& event) {
    if(event.LeftIsDown())    DoDragDrop();
    event.Skip();
}

void kuSingleScrolled::OnLeftDown(wxMouseEvent& event) {
    if(event.ControlDown()) {
        #ifdef __WXMSW__
        SetCursor(wxCursor(wxT("CURSOR_MAGNIFIER"), wxBITMAP_TYPE_CUR_RESOURCE));
        #else
        SetCursor(wxCursor(wxCURSOR_MAGNIFIER));
        #endif
    } else    SetCursor(wxCursor(wxCURSOR_HAND));
    mDragStart=event.GetPosition();
    event.Skip();
}

void kuSingleScrolled::OnLeftUp(wxMouseEvent& event) {
    wxPoint dragStop(event.GetX(),event.GetY());
    if(mFrame->IsFullScreen()) {
        if(event.ControlDown()) {
            ZoomSelectedArea(mDragStart, dragStop, wxPoint(mView.x,mView.y));
            mView = wxRect(wxPoint(0,0),mDispSize);
        } else {
            wxRect view=mView;
            mView.x+=mDragStart.x-event.GetX();
            mView.y+=mDragStart.y-event.GetY();
            CheckViewStart();
            if(mView!=view)   Refresh();
            // keep lefttop
            wxGetApp().mOptions.mLeftTop=wxPoint(mView.x,mView.y);
        }
    }
    else {
        wxPoint start;
        GetViewStart(&start.x,&start.y);
        //wxMessageBox(wxString::Format(wxT("start=(%d,%$d)"),start.x,start.y));
        if(event.ControlDown()) {
            ZoomSelectedArea(mDragStart, dragStop, start);
        } else {
            start.x+=mDragStart.x-event.GetX();
            start.y+=mDragStart.y-event.GetY();
            Scroll(start.x,start.y);
            // keep lefttop
            wxGetApp().mOptions.mLeftTop=start;

        }
    }
    //wxMessageBox(wxString::Format(wxT("start=(%d,%$d)"),wxGetApp().mOptions.mViewStart.x,wxGetApp().mOptions.mViewStart.y));
    SetCursor(wxCursor(wxCURSOR_DEFAULT));
}

void kuSingleScrolled::ZoomSelectedArea(wxPoint start, wxPoint stop, wxPoint offset) {
    int width  = abs(start.x-stop.x);
    int height = abs(start.y-stop.y);
    int left = wxMin(start.x,stop.x);
    int top  = wxMin(start.y,stop.y);
    /* draw scale area */
    wxClientDC dc(this);
    dc.SetPen(*wxRED_PEN);
    dc.SetBrush(*wxTRANSPARENT_BRUSH);
    dc.DrawRectangle(left, top, width, height);
    /* load completely and crop it in kuFrame::OnLoaded() */
    wxGetApp().mLoader->Prepend(mFilename, mIsUrl, wxSize(0,0), wxGetApp().mOptions.mRotate);
    /* TODO: take PixelsPerUnit into account */
    left += -mLeftTop.x+offset.x;
    top  += -mLeftTop.y+offset.y;
    /* get rect in original bmp */
    left   *= SCALE_BASE/mScale;
    top    *= SCALE_BASE/mScale;
    width  *= SCALE_BASE/mScale;
    height *= SCALE_BASE/mScale;
    wxRect rect = mCropRect;
    mCropRect = wxRect(left*SCALE_BASE/mOrigSize.x, top*SCALE_BASE/mOrigSize.y,
                       width*SCALE_BASE/mOrigSize.x, height*SCALE_BASE/mOrigSize.y);
    if(!DrawCropRect()) {
        mCropRect = rect;
        Refresh();
    }
}

void kuSingleScrolled::CheckViewSize() {
    int width  = mDispSize.x;
    int height = mDispSize.y;
    if(mFrame->IsFullScreen())   SetScrollRate(0,0);
    else   SetScrollbars(SCROLL_RATE_SINGLE,SCROLL_RATE_SINGLE,width,height,0,0,true);
    mView=wxRect(0,0,width,height);
    if(width>GetClientSize().x)   mView.SetWidth(GetClientSize().x);
    if(height>GetClientSize().y)   mView.SetHeight(GetClientSize().y);
    //wxMessageBox(wxString::Format(wxT("%d,%d,%d,%d"),mView.x,mView.y,mView.GetWidth(),mView.GetHeight()));
}

void kuSingleScrolled::CheckViewStart() {
    if(mView.x<0)   mView.x=0;
    if(mView.y<0)   mView.y=0;
    int diff;
    diff = mDispSize.x-GetClientSize().x;
    if(diff<0)   mView.x = 0;
    else if(mView.x>diff)   mView.x = diff;
    diff = mDispSize.y-GetClientSize().y;
    if(diff<0)   mView.y = 0;
    else if(mView.y>diff)   mView.y = diff;
}

void kuSingleScrolled::RestoreLeftTop(wxPoint lefttop) {
    if(mFrame->IsFullScreen()) {
        wxRect view = mView;
        mView.x = lefttop.x;
        mView.y = lefttop.y;
        CheckViewStart();
        if(mView!=view)   Refresh();
    } else {
        Scroll(lefttop.x,lefttop.y);
    }
}

void kuSingleScrolled::SetFullScreen(bool full) {
    wxGetApp().mAutoZoom = false;
    mFrame->GetSizer()->Detach(mFrame->mTopSplitter);
    if(full) {   // switch to fullscreen
        mFrame->GetSizer()->Add(mFrame->mTopSplitter,1,wxEXPAND|wxALL,0);
        mFrame->ShowFullScreen(true);
        if(mFrame->mIsFilesystem)   mFrame->mTopSplitter->Unsplit(mFrame->mDirSplitter);
        if(mFrame->mIsThumbnail)   mFrame->mViewSplitter->Unsplit(mFrame->mMultiple);
        SetBackgroundColour(*wxBLACK);
        if(mFrame->CanSetTransparent())    mFrame->SetTransparent(wxGetApp().mOptions.mOpaque);
        mMenu->InsertCheckItem(mMenu->GetMenuItemCount()-2, kuID_STATUSBAR, STRING_MENU_STATUSBAR);
        //mFrame->PushPanelStatus();
    }
    else {
        //mFrame->PopPanelStatus();
        if(mFrame->CanSetTransparent()) {
            mFrame->SetTransparent(255);
            if(CanSetTransparent())    SetTransparent(wxGetApp().mOptions.mOpaque);
        }
        mMenu->Destroy(kuID_STATUSBAR);
        mFrame->GetSizer()->Add(mFrame->mTopSplitter,1,wxEXPAND|wxALL,2);
        if(wxGetApp().mInit==kuApp::kuID_INIT_FULL) {
            mFrame->Show(false);
            mFrame->ShowFullScreen(false);
            mFrame->Maximize(true);
            mFrame->SetupToolBar();
            mFrame->Show(true);
            //SetScale(SCALE_AUTOFIT);
            //wxSafeYield();
            wxGetApp().mInit = kuApp::kuID_INIT_SHOW;
        } else    mFrame->ShowFullScreen(false);
        if(mFrame->mIsFilesystem)   mFrame->mTopSplitter->SplitVertically(mFrame->mDirSplitter,mFrame->mViewSplitter,SPLITTER_TOP_WIDTH);
        if(mFrame->mIsThumbnail)   mFrame->mViewSplitter->SplitHorizontally(mFrame->mSingle,mFrame->mMultiple,-SPLITTER_VIEW_HEIGHT);
        SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNFACE));
    }
    HideCursor(full);
    if(mFrame->CanSetTransparent()) {
        mFrame->GetMenuBar()->Enable(kuID_OPAQUE,full||CanSetTransparent());
        mMenu->Enable(kuID_OPAQUE,full||CanSetTransparent());
    }
    wxGetApp().mAutoZoom = true;
    SetScale(SCALE_AUTOFIT);
}

bool kuSingleScrolled::SetScale(int diff, bool refresh) {
    //wxMessageBox(wxString::Format(wxT("diff=%d"),diff));
    // check if it is invalid
    if(!mOrigBmp)    return false;
    // best fit
    int width  = mOrigSize.x;
    int height = mOrigSize.y;
    if(width==0 || height==0)    return false;
    wxGetApp().mAutoZoom = false;
    wxSize clientSize = GetClientSize();
    switch (diff) {
        case SCALE_AUTOFIT: {
            wxSize origSize = GetOrigSize(true);
            int oratio = wxMax(origSize.x*SCALE_BASE/clientSize.x, origSize.y*SCALE_BASE/clientSize.y);
            int fratio;
            int xratio=width*SCALE_BASE/clientSize.x;
            int yratio=height*SCALE_BASE/clientSize.y;
            fratio = wxMax(xratio, yratio);
            //wxMessageBox(wxString::Format(wxT("%d,%d,%d,%d"),oratio,fratio,clientSize.x,clientSize.y));
            if(oratio>SCALE_BASE || fratio>SCALE_BASE) {    // original > client or current > client
                mScale = SCALE_BASE*SCALE_BASE/fratio;
            } else if(oratio) {    // current < orignal < client
                mScale = SCALE_BASE*oratio/fratio;
            } else {    // current = original
                mScale = SCALE_BASE;
            }
            break;
        }
        case SCALE_BESTFIT: {
            int fratio;
            int xratio=width*SCALE_BASE/clientSize.x;
            int yratio=height*SCALE_BASE/clientSize.y;
            //wxMessageBox(wxString::Format(wxT("%d,%d,%d,%d,%d,%d"),mImage.GetWidth(),mImage.GetHeight(),xratio,yratio,clientSize.x,clientSize.y));
            fratio = wxMax(xratio, yratio);
            mScale=SCALE_BASE*SCALE_BASE/fratio;
            break;
        }
        case SCALE_EXTEND: {
            int fratio;
            int xratio=width*SCALE_BASE/clientSize.x;
            int yratio=height*SCALE_BASE/clientSize.y;
            //wxMessageBox(wxString::Format(wxT("%d,%d,%d,%d,%d,%d"),mImage.GetWidth(),mImage.GetHeight(),xratio,yratio,clientSize.x,clientSize.y));
            fratio = wxMin(xratio, yratio);
            mScale=SCALE_BASE*SCALE_BASE/fratio;
            break;
        }
        case SCALE_ORIGINAL:
            mScale=SCALE_BASE;
            break;
        case SCALE_LASTUSED:
            mScale=wxGetApp().mOptions.mScale;
            break;
        default:
            mScale+=diff;
    }
    wxBeginBusyCursor();
    //wxMessageBox(wxString::Format(wxT("before %d"), wxMax(mDispSize.x, mDispSize.y)));
    mDispSize = wxSize((int)(width*mScale/SCALE_BASE),(int)(height*mScale/SCALE_BASE));
    //wxMessageBox(wxString::Format(wxT("after %d"), wxMax(mDispSize.x, mDispSize.y)));

    FIBITMAP* dispBmp;
    if(mScale==SCALE_BASE)    dispBmp = FreeImage_Clone(mOrigBmp);
    else {
        //wxMessageBox(wxString::Format(wxT("mScale=%lf"),mScale));
        dispBmp = FreeImage_Rescale(mOrigBmp, mDispSize.x, mDispSize.y, wxGetApp().mOptions.mFilter);
    }
    kuFiWrapper::FiBitmap2WxImage(dispBmp,&mDispImg);
    FreeImage_Unload(dispBmp);
    wxEndBusyCursor();
    //wxMessageBox(wxString::Format(wxT("mScale=%lf, mDispSize:%dx%d"),mScale,mDispSize.x,mDispSize.y));

    // show current scale and size
    mFrame->SetStatusText(wxString::Format(wxT("%3.0lf%%"),floor(mScale*100/SCALE_BASE+0.1)), STATUS_FIELD_SCALE);
    // keep scale
    wxGetApp().mOptions.mScale = mScale;

    CheckViewSize();
    CheckViewStart();

    // keep lefttop
    if(wxGetApp().mOptions.mKeepLeftTop) {
        RestoreLeftTop(wxGetApp().mOptions.mLeftTop);
    } else if(mLeftTop != wxPoint(0,0)) {
        RestoreLeftTop(mLeftTop);
    } else if(mDispSize.x > clientSize.x || mDispSize.y > clientSize.y) {
        wxPoint center((mDispSize.x-clientSize.x)/2,(mDispSize.y-clientSize.y)/2);
        RestoreLeftTop(center);
    }

    if(refresh || wxGetApp().mOptions.mKeepStyle==SCALE_LASTUSED) {    // TODO: avoid refresh when keep scale
        Refresh();
    } else {
        wxClientDC dc(this);
        OnDraw(dc);
    }
    wxGetApp().mAutoZoom = true;
    return true;
}

bool kuSingleScrolled::Rescale(double scale) {
    int width = (int)(mOrigSize.x * scale);
    int height = (int)(mOrigSize.y * scale);
    FIBITMAP* bmp = FreeImage_Rescale(mOrigBmp, width, height, wxGetApp().mOptions.mFilter);
    if(!bmp)    return false;
    if(mOrigBmp)    FreeImage_Unload(mOrigBmp);
    mOrigBmp = bmp;
    mDispSize = mOrigSize = wxSize(FreeImage_GetWidth(mOrigBmp), FreeImage_GetHeight(mOrigBmp));
    wxGetApp().SetEdited(true);
    SetScale(SCALE_ORIGINAL);
    return true;
}

void kuSingleScrolled::OnKeyDown(wxKeyEvent& event) {
    //wxMessageBox(wxString::Format(wxT("%d"),event.GetKeyCode()));
    if(!KeyAction(event.GetKeyCode()))   event.Skip();
}

bool kuSingleScrolled::KeyAction(int keycode) {
    //wxMessageBox(wxString::Format(wxT("%d"),keycode));
    wxSize virt,client;
    GetVirtualSize(&virt.x, &virt.y);
    GetClientSize(&client.x, &client.y);
    switch (keycode) {
        case WXK_UP:
            if(mFrame->GetCurrentPath()==mFilename && virt.y>client.y)    return false;
            if(mFrame->mIsFilesystem && !mFrame->IsFullScreen())    mFrame->Action(kuID_UP);
            else    mFrame->Action(kuID_PREV);
            break;
        case WXK_DOWN:
            if(mFrame->GetCurrentPath()==mFilename && virt.y>client.y)    return false;
            if(mFrame->mIsFilesystem && !mFrame->IsFullScreen())    mFrame->Action(kuID_DOWN);
            else    mFrame->Action(kuID_NEXT);
            break;
        case WXK_LEFT:
            if(mFrame->GetCurrentPath()==mFilename && virt.x>client.x)    return false;
            if(mFrame->mIsFilesystem && !mFrame->IsFullScreen())    mFrame->Action(kuID_LEFT);
            else    mFrame->Action(kuID_PREV);
            break;
        case WXK_RIGHT:
            if(mFrame->GetCurrentPath()==mFilename && virt.x>client.x)    return false;
            if(mFrame->mIsFilesystem && !mFrame->IsFullScreen())    mFrame->Action(kuID_RIGHT);
            else    mFrame->Action(kuID_NEXT);
            break;
        case WXK_PAGEUP:
            mFrame->Action(kuID_PREV);
            break;
        case WXK_PAGEDOWN:
            mFrame->Action(kuID_NEXT);
            break;
        case WXK_HOME:
            mFrame->Action(kuID_HOME);
            break;
        case WXK_END:
            mFrame->Action(kuID_END);
            break;
        default:
            return false;
    }
    return true;
}

void kuSingleScrolled::OnMousewheel(wxMouseEvent& event) {
    if(event.ControlDown()) {   // zoom
        wxPoint lt, real;
        GetViewStart(&lt.x, &lt.y);
        real.x = (int)((lt.x+event.GetX())*(mScale+SCALE_DIFF)/mScale);
        real.y = (int)((lt.y+event.GetY())*(mScale+SCALE_DIFF)/mScale);
        //wxMessageBox(wxString::Format(wxT("%d,%d"),real.x, real.y));
        real.x -= event.GetX();
        real.y -= event.GetY();
        //wxMessageBox(wxString::Format(wxT("%d,%d"),real.x, real.y));
        mLeftTop = real;
        if(event.GetWheelRotation()>0)   mFrame->Action(kuID_ZOOM_IN);
        else   mFrame->Action(kuID_ZOOM_OUT);
    } else {   // next or prev
        if(event.GetWheelRotation()>0)   mFrame->Action(kuID_PREV);
        else   mFrame->Action(kuID_NEXT);
    }
}

void kuSingleScrolled::OnContextMenu(wxContextMenuEvent& event) {
    PopupMenu(mMenu);
}

void kuSingleScrolled::OnIdleTimer(wxTimerEvent& event) {
    HideCursor();
}

void kuSingleScrolled::HideCursor(bool hide) {
    if(hide) {
        #ifdef __WXMSW__
        SetCursor(wxCursor(wxT("CURSOR_BLANK"), wxBITMAP_TYPE_CUR_RESOURCE));
        #else
        SetCursor(wxCursor(wxCURSOR_BLANK));
        #endif
    } else {
        SetCursor(wxCursor(wxCURSOR_DEFAULT));
    }
}

void kuSingleScrolled::DoDragDrop() {
    wxDropSource source(this);
    wxFileDataObject data;
    data.AddFile(mFilename);
    source.SetData(data);
    source.DoDragDrop();
}

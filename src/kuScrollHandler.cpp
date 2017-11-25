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

BEGIN_EVENT_TABLE(kuScrollHandler,wxEvtHandler)
    EVT_KEY_DOWN(kuScrollHandler::OnKeyDown)
    EVT_LEFT_DCLICK(kuScrollHandler::OnLeftDclick)
    EVT_MIDDLE_UP(kuScrollHandler::OnMiddleUp)
    EVT_ENTER_WINDOW(kuScrollHandler::OnEnterWindow)
    EVT_MENU_RANGE(kuID_LOWEST, kuID_HIGHEST, kuScrollHandler::OnMenuRange)
END_EVENT_TABLE()

// -------- kuScrollHandler --------
kuScrollHandler::kuScrollHandler(kuFrame* frame) {
    mFrame=frame;
}

void kuScrollHandler::OnKeyDown(wxKeyEvent& event) {
    //wxMessageBox(wxString::Format(wxT("%d"),event.GetKeyCode()));
    switch(event.GetKeyCode()) {
        case WXK_ESCAPE:
            if(mFrame->Action(kuID_ESCAPE))   return;
            break;
        case WXK_NUMPAD_ADD:
            if(mFrame->Action(kuID_ZOOM_IN))   return;
            break;
        case WXK_NUMPAD_SUBTRACT:
            if(mFrame->Action(kuID_ZOOM_OUT))   return;
            break;
        case WXK_NUMPAD_MULTIPLY:
            if(mFrame->Action(kuID_ZOOM_100))   return;
            break;
        case WXK_NUMPAD_DIVIDE:
            if(mFrame->Action(kuID_ZOOM_FIT))   return;
            break;
        default:
            break;
    }
    event.Skip();
}

void kuScrollHandler::OnLeftDclick(wxMouseEvent& event) {
    if(!mFrame->Action(kuID_FULLSCREEN))   event.Skip();
}

void kuScrollHandler::OnMiddleUp(wxMouseEvent& event) {
    if(!mFrame->Action(kuID_FULLSCREEN))   event.Skip();
}

void kuScrollHandler::OnEnterWindow(wxMouseEvent& event) {
    /*
    if((event.GetEventObject())->IsKindOf(CLASSINFO(wxWindow))) {
        ((wxWindow*)(event.GetEventObject()))->SetFocus();
    }
    */
    event.Skip();
}

void kuScrollHandler::OnMenuRange(wxCommandEvent& event) {
    mFrame->Action(event.GetId());
}

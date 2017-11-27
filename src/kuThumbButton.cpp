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

BEGIN_EVENT_TABLE(kuThumbButton,wxBitmapButton)
    EVT_BUTTON(wxID_ANY,kuThumbButton::OnButton)
    EVT_MOUSEWHEEL(kuThumbButton::OnMouseWheel)
    EVT_KEY_DOWN(kuThumbButton::OnKeyDown)
    EVT_ENTER_WINDOW(kuThumbButton::OnEnterWindow)
    EVT_LEAVE_WINDOW(kuThumbButton::OnLeaveWindow)
    EVT_CONTEXT_MENU(kuThumbButton::OnContextMenu)
    EVT_MENU(wxID_DELETE,kuThumbButton::OnDelete)
END_EVENT_TABLE()

// -------- kuThumbButton --------
kuThumbButton::kuThumbButton(wxWindow* parent, wxString filename, wxString url)
    /*:wxBitmapButton(parent,wxID_ANY,wxNullImage,
                    wxDefaultPosition,wxSize(0,0),
                    wxBU_AUTODRAW,wxDefaultValidator,filename)*/ {
    //wxMessageBox(filename);
    mIsDragging = false;
    mIsUrl = url==wxEmptyString ? false : true;

    wxImage* image = kuFiWrapper::GetWxImage(filename, mIsUrl, wxSize(THUMBNAIL_WIDTH,THUMBNAIL_WIDTH));
	Create(parent, wxID_ANY, *image,
		wxDefaultPosition, image->GetSize(),
		wxBU_AUTODRAW, wxDefaultValidator, filename);
    SetBitmapLabel(*image);
	delete image;

    SetupPopupMenu();
    SetSizeHints(GetBestSize());
    if(mIsUrl) {
        SetName(url);
        SetToolTip(url);
    } else {
        SetName(filename);
        SetToolTip(filename.AfterLast(wxFileName::GetPathSeparator()));
    }
}

void kuThumbButton::SetupPopupMenu() {
    /* don't delete it manually
    if(mMenu)   delete mMenu;
    */
    mMenu = new wxMenu();
    mMenu->Append(wxID_DELETE, STRING_MENU_THUMBS_DELETE);
}

void kuThumbButton::OnButton(wxCommandEvent& event) {
    if(mIsUrl)   // in fact, it is filename only
        wxGetApp().mFrame->mVirtual->Locate(GetName());
    else   wxGetApp().mFrame->mGeneric->Locate(GetName());
    wxGetApp().mFrame->mMultiple->Select(GetName(), wxGetKeyState(WXK_CONTROL), wxGetKeyState(WXK_SHIFT));
    mIsDragging = false;
}

void kuThumbButton::OnMouseWheel(wxMouseEvent& event) {
    event.Skip();
}

void kuThumbButton::OnKeyDown(wxKeyEvent& event) {
    event.Skip();
}

void kuThumbButton::OnEnterWindow(wxMouseEvent& event) {
    if(event.LeftIsDown())    mIsDragging = true;
    event.Skip();
}

void kuThumbButton::OnLeaveWindow(wxMouseEvent& event) {
    if(event.LeftIsDown() && !mIsDragging)    wxGetApp().mFrame->mMultiple->DoDragDrop();
    mIsDragging = false;
    event.Skip();
}

void kuThumbButton::OnContextMenu(wxContextMenuEvent& event) {
    PopupMenu(mMenu);
}

void kuThumbButton::OnDelete(wxCommandEvent& event) {
    event.SetString(GetName());
    this->GetParent()->GetEventHandler()->AddPendingEvent(event);
}

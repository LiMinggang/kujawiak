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

// -------- kuPrintout --------
kuPrintout::kuPrintout(kuSingleScrolled* single, wxPageSetupDialogData* data)
    :wxPrintout(single->GetFilename()) {
    mSingle           = single;
    mPageSetupDlgData = data;
    mPrintBmp         = NULL;
}

kuPrintout::~kuPrintout() {
    if(mPrintBmp)    FreeImage_Unload(mPrintBmp);
}

bool kuPrintout::OnPrintPage(int page) {
    wxDC* dc = GetDC();
    if(dc) {
        if(page)    DrawPage();
        //MapScreenSizeToPage();
        //dc->DrawText(mSingle->GetFilename(), 0, 0);
        return true;
    } else {
        return false;
    }
}

bool kuPrintout::HasPage(int page) {
    return page==1;
}

bool kuPrintout::OnBeginDocument(int startPage, int endPage) {
    if (!wxPrintout::OnBeginDocument(startPage, endPage))    return false;
    return true;
}

void kuPrintout::GetPageInfo(int *minPage, int *maxPage, int *selPageFrom, int *selPageTo) {
    *minPage     = 1;
    *maxPage     = 1;
    *selPageFrom = 1;
    *selPageTo   = 1;
}

void kuPrintout::DrawPage() {
    // map original to full page
    FitThisSizeToPageMargins(mSingle->GetOrigSize(), *mPageSetupDlgData);
    wxRect fitRect = GetLogicalPageMarginsRect(*mPageSetupDlgData);

    wxCoord xoff = (fitRect.width - mSingle->GetOrigSize().x) / 2;
    wxCoord yoff = (fitRect.height - mSingle->GetOrigSize().y) / 2;
    OffsetLogicalOrigin(xoff, yoff);

    // scale
    float scale = wxGetApp().mOptions.mPrintScale;
    if(scale!=1) {    // need scale
        int newx = (int)(mSingle->GetOrigSize().x * scale);
        int newy = (int)(mSingle->GetOrigSize().y * scale);
        if(mSingle->GetOrigBmp()) {
            if(mPrintBmp)    FreeImage_Unload(mPrintBmp);
            mPrintBmp = FreeImage_Rescale(mSingle->GetOrigBmp(), newx, newy, wxGetApp().mOptions.mFilter);
            kuFiWrapper::FiBitmap2WxImage(mPrintBmp, &mPrintImg);
        }
    } else {
        if(mSingle->GetOrigBmp()) {
            kuFiWrapper::FiBitmap2WxImage(mSingle->GetOrigBmp(), &mPrintImg);
        }
    }

    // align
    xoff = (mSingle->GetOrigSize().x - mPrintImg.GetWidth())/2;
    yoff = (mSingle->GetOrigSize().y - mPrintImg.GetHeight())/2;
    int x=0, y=0;
    switch (wxGetApp().mOptions.mPrintAlign) {
        case kuAlignSelector::kuID_ALIGN_LT:
            x = 0;         y = 0;
            break;
        case kuAlignSelector::kuID_ALIGN_CT:
            x = xoff;      y = 0;
            break;
        case kuAlignSelector::kuID_ALIGN_RT:
            x = xoff*2;    y = 0;
            break;
        case kuAlignSelector::kuID_ALIGN_LC:
            x = 0;         y = yoff;
            break;
        case kuAlignSelector::kuID_ALIGN_CC:
            x = xoff;      y = yoff;
            break;
        case kuAlignSelector::kuID_ALIGN_RC:
            x = xoff*2;    y = yoff;
            break;
        case kuAlignSelector::kuID_ALIGN_LB:
            x = 0;         y = yoff*2;
            break;
        case kuAlignSelector::kuID_ALIGN_CB:
            x = xoff;      y = yoff*2;
            break;
        case kuAlignSelector::kuID_ALIGN_RB:
            x = xoff*2;    y = yoff*2;
            break;
        default:
            break;
    }

    // draw
    GetDC()->DrawBitmap(wxBitmap(mPrintImg), x, y, true);
}

// -------- kuPreviewFrame --------
kuPreviewFrame::kuPreviewFrame(wxPrintPreview* preview, wxWindow* parent, const wxString& title)
    :wxPreviewFrame(preview, parent, title) {
}

void kuPreviewFrame::CreateControlBar() {
    long buttons = wxPREVIEW_ZOOM;
    if (m_printPreview->GetPrintoutForPrinting())
        buttons |= wxPREVIEW_PRINT;

    m_controlBar = new kuPreviewControlBar(m_printPreview, buttons, this, wxPoint(0,0), wxSize(400, 40));
    m_controlBar->CreateButtons();
}

// -------- kuPreviewControlBar --------
kuPreviewControlBar::kuPreviewControlBar(wxPrintPreviewBase *preview, long buttons,
                                         wxWindow *parent, const wxPoint& pos, const wxSize& size)
    :wxPreviewControlBar(preview, buttons, parent, pos, size) {
}

void kuPreviewControlBar::CreateButtons() {
    wxPreviewControlBar::CreateButtons();

    int navButtonStyle(wxBU_EXACTFIT);
    wxSize navButtonSize(m_closeButton->GetSize().x, m_closeButton->GetSize().y);
    // add align button
    m_alignButton = new wxButton(this, kuID_PREVIEW_BTNALIGN, STRING_BUTTON_ALIGN, wxDefaultPosition, navButtonSize, navButtonStyle);
    GetSizer()->Add(m_alignButton, 0, wxALIGN_CENTRE|wxALL, 5);
    m_scaleButton = new wxButton(this, kuID_PREVIEW_BTNSCALE, STRING_BUTTON_SCALE, wxDefaultPosition, navButtonSize, navButtonStyle);
    GetSizer()->Add(m_scaleButton, 0, wxALIGN_CENTRE|wxALL, 5);
    GetSizer()->Fit(this);

    Connect(kuID_PREVIEW_BTNALIGN, wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(kuPreviewControlBar::OnAlign));
    Connect(kuID_PREVIEW_BTNSCALE, wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(kuPreviewControlBar::OnScale));
}

void kuPreviewControlBar::OnAlign(wxCommandEvent& event) {
    kuAlignSelector dlg(GetPrintPreview()->GetFrame());
    dlg.SetAlign(wxGetApp().mOptions.mPrintAlign);
    if(dlg.ShowModal()==wxID_CANCEL)    return;
    wxGetApp().mOptions.mPrintAlign = dlg.GetAlign();
    UpdatePreview();
}

void kuPreviewControlBar::OnScale(wxCommandEvent& event) {
    double scale = kuApp::GetScaleFromUser(GetPrintPreview()->GetFrame(), wxGetApp().mOptions.mPrintScale);
    if(scale==-1)    return;
    wxGetApp().mOptions.mPrintScale = scale;
    UpdatePreview();
}

void kuPreviewControlBar::UpdatePreview() {
    // notify to repaint
    GetPrintPreview()->RenderPage(1);
    GetPrintPreview()->GetCanvas()->ClearBackground();
    GetPrintPreview()->GetCanvas()->Refresh();
    GetPrintPreview()->GetCanvas()->SetFocus();
}

kuAlignSelector::kuAlignSelector(wxWindow* parent)
    :wxDialog(parent,wxID_ANY,kuFrame::StripCodes(STRING_BUTTON_ALIGN),wxDefaultPosition,wxDefaultSize) {
    mAlign = kuID_ALIGN_CC;

    wxBoxSizer* topsizer = new wxBoxSizer(wxVERTICAL);
    // message
    topsizer->Add(new wxStaticText(this,wxID_ANY,STRING_ALIGN_MESSAGE,wxDefaultPosition,wxDefaultSize,wxALIGN_CENTRE),0,wxEXPAND|wxTOP,5);
    topsizer->Add(new wxStaticLine(this),0,wxEXPAND|wxTOP,5);
    // align
    int btnsize = wxMin(wxGetDisplaySize().x,wxGetDisplaySize().y)/15;
    wxPanel* panel = new wxPanel(this);
    wxGridSizer* alignsizer = new wxGridSizer(3,3,0,0);
    for(int i=kuID_ALIGN_LT; i<kuID_ALIGN_HIGHEST;i++) {
        wxButton* btn = new wxButton(panel,i,wxEmptyString,wxDefaultPosition,wxSize(btnsize,btnsize),wxNO_BORDER);
        Connect(i, wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(kuAlignSelector::OnButton));
        alignsizer->Add(btn,1,wxALIGN_CENTER|wxEXPAND|wxFIXED_MINSIZE);
    }
    panel->SetSizer(alignsizer);
    topsizer->Add(panel,0,wxEXPAND|wxALL,10);
    // text
    topsizer->Add(new wxStaticLine(this),0,wxEXPAND|wxBOTTOM,5);
    mSelect = new wxStaticText(this,wxID_ANY,wxEmptyString,wxDefaultPosition,wxDefaultSize,wxALIGN_CENTRE|wxST_NO_AUTORESIZE);
    topsizer->Add(mSelect,1,wxEXPAND|wxBOTTOM,5);
    // btn
    wxSizer* btnsizer = CreateSeparatedButtonSizer(wxOK|wxCANCEL);
    topsizer->Add(btnsizer,0,wxEXPAND|wxALIGN_CENTER);
    topsizer->Fit(this);
    SetSizer(topsizer);
}

bool kuAlignSelector::Show(bool show) {
    bool ret = wxDialog::Show(show);
    wxCommandEvent event;
    wxWindow* btn = FindWindow(mAlign);
    if(btn) {
        event.SetEventObject(btn);
        event.SetEventType(wxEVT_COMMAND_BUTTON_CLICKED);
        event.SetId(mAlign);
        event.SetTimestamp(wxGetLocalTime());
		wxEvtHandler *btnevthdl = btn->GetEventHandler();
		btnevthdl->AddPendingEvent(event);
        btn->SetFocus();
    }
    return ret;
}

void kuAlignSelector::OnButton(wxCommandEvent& event) {
    int id = event.GetId();
    //wxMessageBox(wxString::Format(wxT("%d"),id));
    wxString label;
    switch (id) {
        case kuID_ALIGN_LT:
            label = wxString::Format(wxT("%s, %s"), STRING_ALIGN_LEFT,   STRING_ALIGN_TOP);
            break;
        case kuID_ALIGN_CT:
            label = wxString::Format(wxT("%s, %s"), STRING_ALIGN_CENTER, STRING_ALIGN_TOP);
            break;
        case kuID_ALIGN_RT:
            label = wxString::Format(wxT("%s, %s"), STRING_ALIGN_RIGHT,  STRING_ALIGN_TOP);
            break;
        case kuID_ALIGN_LC:
            label = wxString::Format(wxT("%s, %s"), STRING_ALIGN_LEFT,   STRING_ALIGN_CENTER);
            break;
        case kuID_ALIGN_CC:
            label = wxString::Format(wxT("%s, %s"), STRING_ALIGN_CENTER, STRING_ALIGN_CENTER);
            break;
        case kuID_ALIGN_RC:
            label = wxString::Format(wxT("%s, %s"), STRING_ALIGN_RIGHT,  STRING_ALIGN_CENTER);
            break;
        case kuID_ALIGN_LB:
            label = wxString::Format(wxT("%s, %s"), STRING_ALIGN_LEFT,   STRING_ALIGN_BOTTOM);
            break;
        case kuID_ALIGN_CB:
            label = wxString::Format(wxT("%s, %s"), STRING_ALIGN_CENTER, STRING_ALIGN_BOTTOM);
            break;
        case kuID_ALIGN_RB:
            label = wxString::Format(wxT("%s, %s"), STRING_ALIGN_RIGHT,  STRING_ALIGN_BOTTOM);
            break;
        default:
            break;
    }
    // set label
    wxButton* btn = (wxButton*)(event.GetEventObject());
    kuAlignSelector* selector = (kuAlignSelector*)(btn->GetParent()->GetParent());
    selector->mSelect->SetLabel(label);
    mAlign = id;
}

int kuAlignSelector::GetAlign() {
    return mAlign;
}

void kuAlignSelector::SetAlign(int align) {
    mAlign = align;
}

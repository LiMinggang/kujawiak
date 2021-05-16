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

#ifndef __WXMSW__
    #include "../icons/xpm/ICON_APP.xpm"
    #include "../icons/xpm/ICON_ORG.xpm"
    #include "../icons/xpm/ICON_SAVE.xpm"
    #include "../icons/xpm/ICON_INTERRUPT.xpm"
    #include "../icons/xpm/ICON_RELOAD.xpm"
    #include "../icons/xpm/ICON_FILE_CUT.xpm"
    #include "../icons/xpm/ICON_FILE_COPY.xpm"
    #include "../icons/xpm/ICON_FILE_PASTE.xpm"
    #include "../icons/xpm/ICON_FILE_DELETE.xpm"
    #include "../icons/xpm/ICON_GOHOME.xpm"
    #include "../icons/xpm/ICON_GOFAVORITE.xpm"
    #include "../icons/xpm/ICON_PREV.xpm"
    #include "../icons/xpm/ICON_NEXT.xpm"
    #include "../icons/xpm/ICON_HOME.xpm"
    #include "../icons/xpm/ICON_END.xpm"
    #include "../icons/xpm/ICON_UP.xpm"
    #include "../icons/xpm/ICON_DOWN.xpm"
    #include "../icons/xpm/ICON_LEFT.xpm"
    #include "../icons/xpm/ICON_RIGHT.xpm"
    #include "../icons/xpm/ICON_FULLSCREEN.xpm"
    #include "../icons/xpm/ICON_ZOOM_IN.xpm"
    #include "../icons/xpm/ICON_ZOOM_OUT.xpm"
    #include "../icons/xpm/ICON_ZOOM_100.xpm"
    #include "../icons/xpm/ICON_ZOOM_FIT.xpm"
    #include "../icons/xpm/ICON_ROTATE_CCW.xpm"
    #include "../icons/xpm/ICON_ROTATE_CW.xpm"
    #include "../icons/xpm/ICON_FILESYSTEM.xpm"
    #include "../icons/xpm/ICON_ARCHIVE.xpm"
    #include "../icons/xpm/ICON_THUMBNAIL.xpm"
    #include "../icons/xpm/ICON_MINIMIZE.xpm"
    #include "../icons/xpm/ICON_EXIT.xpm"
#endif

class KuTranslationHelper : public wxDirTraverser
{
	wxArrayString m_LangCanonicalNames;
public:
	KuTranslationHelper() {}
	~KuTranslationHelper() {};
	wxArrayString & GetLangCanonicalNames() { return m_LangCanonicalNames; }
	virtual wxDirTraverseResult OnFile( const wxString& WXUNUSED( filename ) ) {
		return wxDIR_CONTINUE;
	}
	virtual wxDirTraverseResult OnDir( const wxString& dirName ) {
        m_LangCanonicalNames.Add(dirName.AfterLast(wxFileName::GetPathSeparator()));
		return wxDIR_CONTINUE;
	}
};

// -------- implement --------

DEFINE_EVENT_TYPE(wxEVT_LOADED_EVENT)
DEFINE_EVENT_TYPE(wxEVT_DROPPED_EVENT)

BEGIN_EVENT_TABLE(kuFrame,wxFrame)
    EVT_MENU(wxID_ABOUT,kuFrame::OnAbout)
    EVT_MENU(wxID_EXIT,kuFrame::OnQuit)
    EVT_TOOL(wxID_ANY,kuFrame::OnTool)
    EVT_SIZE(kuFrame::OnSize)
    EVT_CLOSE(kuFrame::OnClose)
    EVT_COMMAND(wxID_ANY, wxEVT_LOADED_EVENT, kuFrame::OnLoaded)
    EVT_COMMAND(wxID_ANY, wxEVT_DROPPED_EVENT, kuFrame::OnDropped)
    EVT_IDLE(kuFrame::OnIdle)
END_EVENT_TABLE()

BEGIN_EVENT_TABLE(kuTaskBarIcon,wxTaskBarIcon)
    EVT_TASKBAR_LEFT_DCLICK(kuTaskBarIcon::OnLeftDclick)
    EVT_MENU(kuID_RESTORE,kuTaskBarIcon::OnRestore)
    EVT_MENU(wxID_ABOUT,kuFrame::OnAbout)
    EVT_MENU(wxID_EXIT,kuFrame::OnQuit)
END_EVENT_TABLE()

BEGIN_EVENT_TABLE(kuStatusBar,wxStatusBar)
    EVT_SIZE(kuStatusBar::OnSize)
END_EVENT_TABLE()

BEGIN_EVENT_TABLE(kuPathPanel,wxPanel)
    EVT_BUTTON(wxID_OPEN,kuPathPanel::OnButton)
END_EVENT_TABLE()

// -------- kuFrame --------
kuFrame::kuFrame()
    :wxFrame(NULL,wxID_ANY,STRING_APPNAME) {
    mIsFilesystem = false;
    mIsArchive    = false;
    mIsThumbnail  = false;
    mRescaleMgrDlg = NULL;
    #ifdef ENABLE_PICASAWEBMGR
    mPicasaWebMgrDlg = NULL;
    #endif

    // icon
    mIconApp=wxIcon(wxICON(ICON_APP));
    SetIcon(mIconApp);

    // menuBar
    SetupMenuBar();

    // toolBar
    SetToolBar(NULL);
    SetupToolBar(true);

    // statusBar
    SetupStatusBar();

    // topSizer
    wxBoxSizer* topSizer = new wxBoxSizer(wxHORIZONTAL);
    SetSizer(topSizer);

    // topSplitter
    mTopSplitter = new wxSplitterWindow(this,wxID_ANY,wxDefaultPosition,wxDefaultSize,
                                        wxSP_NOBORDER|wxSP_PERMIT_UNSPLIT);
    mDirSplitter = new wxSplitterWindow(mTopSplitter,wxID_ANY,wxDefaultPosition,wxDefaultSize,
                                        wxSP_NOBORDER|wxSP_PERMIT_UNSPLIT);
    mViewSplitter = new wxSplitterWindow(mTopSplitter,wxID_ANY,wxDefaultPosition,wxDefaultSize,
                                        wxSP_NOBORDER|wxSP_PERMIT_UNSPLIT);
    mTopSplitter->SplitVertically(mDirSplitter,mViewSplitter,SPLITTER_TOP_WIDTH);
    // dirSplitter
    mGeneric = new kuGenericDirCtrl(mDirSplitter,this);
    mVirtual = new kuVirtualDirCtrl(mDirSplitter,this);
    mDirSplitter->SplitHorizontally(mGeneric,mVirtual,-SPLITTER_DIR_HEIGHT);
    // viewSplitter
    mSingle = new kuSingleScrolled(mViewSplitter,this);
    mMultiple = new kuMultipleScrolled(mViewSplitter,this);
    mViewSplitter->SplitHorizontally(mSingle,mMultiple,-SPLITTER_VIEW_HEIGHT);
    topSizer->Add(mTopSplitter,1,wxEXPAND|wxALL,2);
    // initial view
    ToggleFilesystem(false);
    ToggleArchive(false);
    ToggleThumbnail(false);
    mIsStatusbar = false;
    mIsSummary = false;
    mIsSlideshow = false;
    mIsHistory = false;
    mIsPause = false;
    mSlideshowDirs.Clear();
    mSlideshowIndex = 0;
    mHistoryMainFiles.Clear();
    mHistoryMainIndex = -1;
    mHistorySwapIndex = -1;
    mHistoryBranchFiles.Clear();
    mHistoryBranchIndex = -1;

    // handler
    mHandler = new kuScrollHandler(this);
    mGeneric->GetTreeCtrl()->PushEventHandler(mHandler);
    mVirtual->PushEventHandler(new kuScrollHandler(this));
    mSingle->PushEventHandler(new kuScrollHandler(this));
    mMultiple->PushEventHandler(new kuScrollHandler(this));

    SetupAccelerator();

    // taskbaricon
    mTaskBarIcon = new kuTaskBarIcon(this);

    // set printing defaults
    SetupPrinting();

    // setup managers
    SetupManagers();
}

void kuFrame::SetupManagers() {
    if(!mRescaleMgrDlg)    mRescaleMgrDlg = new kuRescaleMgrDialog(this, kuID_RESCALE_MANAGER);
    mRescaleMgrDlg->SetupHeaders();
    mRescaleMgrDlg->SetupButtons();
    mRescaleMgrDlg->SetupTitleAndSize();
    #ifdef ENABLE_PICASAWEBMGR
    if(!mPicasaWebMgrDlg)    mPicasaWebMgrDlg = new kuPicasaWebMgrDialog(this, kuID_PICASAWEB_MANAGER);
    mPicasaWebMgrDlg->SetupHeaders();
    mPicasaWebMgrDlg->SetupButtons();
    mPicasaWebMgrDlg->SetupTitleAndSize();
    #endif
}

void kuFrame::SetupMenuBar() {
    // don't delete it directly
    if(GetMenuBar()) {
        wxMenuBar* bar = GetMenuBar();
        SetMenuBar(NULL);
        delete bar;
    }

    wxMenu* fileMenu = new wxMenu();
    fileMenu->Append(kuID_SAVE,       STRING_MENU_SAVE);
    fileMenu->Append(kuID_SAVEAS,     STRING_MENU_SAVEAS);
    fileMenu->AppendSeparator();
    fileMenu->Append(kuID_PAGESETUP,  STRING_MENU_PAGESETUP);
    fileMenu->Append(kuID_PREVIEW,    STRING_MENU_PREVIEW);
    fileMenu->Append(kuID_PRINT,      STRING_MENU_PRINT);
    fileMenu->AppendSeparator();
    #ifdef __WXMSW__
    fileMenu->Append(kuID_PROPERTIES,       STRING_MENU_PROPERTIES);
    #endif
    fileMenu->Append(kuID_METADATA,         STRING_MENU_METADATA);
    fileMenu->AppendCheckItem(kuID_SUMMARY, STRING_MENU_SUMMARY);
    fileMenu->AppendSeparator();
    fileMenu->Append(kuID_INTERRUPT,  STRING_MENU_INTERRUPT);
    fileMenu->Append(kuID_RELOAD,     STRING_MENU_RELOAD);
    fileMenu->Append(kuID_MINIMIZE,   STRING_MENU_MINIMIZE);
    fileMenu->Append(wxID_EXIT,       STRING_MENU_EXIT);

    wxMenu* editMenu = new wxMenu();
    editMenu->Append(kuID_ROTATE_CCW, STRING_MENU_ROTATE_CCW);
    editMenu->Append(kuID_ROTATE_CW,  STRING_MENU_ROTATE_CW);
    editMenu->Append(kuID_RESCALE,STRING_MENU_RESCALE);

    wxMenu* traverseMenu = new wxMenu();
    traverseMenu->Append(kuID_GOHOME,     STRING_MENU_GOHOME);
    traverseMenu->Append(kuID_GOFAVORITE, STRING_MENU_GOFAVORITE);
    traverseMenu->AppendSeparator();
    traverseMenu->Append(kuID_PREV,   STRING_MENU_PREV);
    traverseMenu->Append(kuID_NEXT,   STRING_MENU_NEXT);
    traverseMenu->Append(kuID_HOME,   STRING_MENU_HOME);
    traverseMenu->Append(kuID_END,    STRING_MENU_END);
    traverseMenu->AppendSeparator();
    traverseMenu->Append(kuID_UP,     STRING_MENU_UP);
    traverseMenu->Append(kuID_DOWN,   STRING_MENU_DOWN);
    traverseMenu->Append(kuID_LEFT,   STRING_MENU_LEFT);
    traverseMenu->Append(kuID_RIGHT,  STRING_MENU_RIGHT);
    traverseMenu->AppendSeparator();
    traverseMenu->Append(kuID_BACK,    STRING_MENU_BACK);
    traverseMenu->Append(kuID_FORWARD, STRING_MENU_FORWARD);
    traverseMenu->Append(kuID_BRANCH,  STRING_MENU_BRANCH);

    wxMenu* viewMenu = new wxMenu();
    viewMenu->Append(kuID_FULLSCREEN, STRING_MENU_FULLSCREEN);
    viewMenu->AppendSeparator();
    viewMenu->Append(kuID_ZOOM_IN,    STRING_MENU_ZOOM_IN);
    viewMenu->Append(kuID_ZOOM_OUT,   STRING_MENU_ZOOM_OUT);
    viewMenu->Append(kuID_ZOOM_100,   STRING_MENU_ZOOM_100);
    viewMenu->Append(kuID_ZOOM_FIT,   STRING_MENU_ZOOM_FIT);
    viewMenu->Append(kuID_ZOOM_EXT,   STRING_MENU_ZOOM_EXT);
    viewMenu->AppendSeparator();
    if(CanSetTransparent()) {
        viewMenu->Append(kuID_OPAQUE, STRING_MENU_OPAQUE);
        viewMenu->AppendSeparator();
    }
    viewMenu->Append(kuID_SLIDESHOW,  STRING_MENU_SLIDESHOW_START);

    wxMenu* panelMenu = new wxMenu();
    panelMenu->AppendCheckItem(kuID_FILESYSTEM, STRING_MENU_FILESYSTEM);
    panelMenu->AppendCheckItem(kuID_ARCHIVE,    STRING_MENU_ARCHIVE);
    panelMenu->AppendCheckItem(kuID_THUMBNAIL,  STRING_MENU_THUMBNAIL);

    wxMenu* manageMenu = new wxMenu();
    manageMenu->Append(kuID_MKDIR,       STRING_MENU_MKDIR);
    manageMenu->Append(kuID_RENAME,      STRING_MENU_RENAME);
    manageMenu->AppendSeparator();
    manageMenu->Append(kuID_FILE_CUT,    STRING_MENU_FILE_CUT);
    manageMenu->Append(kuID_FILE_COPY,   STRING_MENU_FILE_COPY);
    manageMenu->Append(kuID_FILE_PASTE,  STRING_MENU_FILE_PASTE);
    manageMenu->Append(kuID_FILE_DELETE, STRING_MENU_FILE_DELETE);
    manageMenu->AppendSeparator();
    manageMenu->Append(kuID_FILE_MOVETO, STRING_MENU_FILE_MOVETO);
    manageMenu->Append(kuID_FILE_COPYTO, STRING_MENU_FILE_COPYTO);

    wxMenu* toolsMenu = new wxMenu();
    toolsMenu->Append(kuID_EXPLORE,          STRING_MENU_EXPLORE);
    #ifdef __WXMSW__
    toolsMenu->Append(kuID_EXTERNAL_EDIT,    STRING_MENU_EXTERNAL_EDIT);
    #endif
    //toolsMenu->Append(kuID_EXTERNAL_OPEN,    STRING_MENU_EXTERNAL_OPEN);
    toolsMenu->AppendSeparator();
    toolsMenu->Append(kuID_RESCALE_MANAGER,   STRING_MENU_RESCALE_MANAGER);
    #ifdef ENABLE_PICASAWEBMGR
    toolsMenu->Append(kuID_PICASAWEB_MANAGER, STRING_MENU_PICASAWEB_MANAGER);
    #endif

    wxMenu* optionMenu = new wxMenu();
    #ifdef __WXMSW__
    optionMenu->Append(kuID_SHELL,       STRING_MENU_SHELL);
    optionMenu->Append(kuID_ASSOCIATION, STRING_MENU_ASSOCIATION);
    wxMenu* optionDesktopMenu = new wxMenu();
    optionDesktopMenu->Append(kuID_WALLPAPER,  STRING_MENU_DESKTOP_WALLPAPER);
    optionDesktopMenu->Append(kuID_NONEWP,     STRING_MENU_DESKTOP_NONEWP);
    optionDesktopMenu->Append(kuID_BACKGROUND, STRING_MENU_DESKTOP_BACKGROUND);
    optionMenu->AppendSubMenu(optionDesktopMenu,   STRING_MENU_OPTION_DESKTOP);
    #endif
    optionMenu->Append(kuID_LANGUAGE,    STRING_MENU_LANGUAGE);
    wxMenu* optionTraverseMenu = new wxMenu();
    optionTraverseMenu->Append(kuID_FAVORITE, STRING_MENU_FAVORITE);
    optionMenu->AppendSubMenu(optionTraverseMenu,  STRING_MENU_OPTION_TRAVERSE);
    wxMenu* optionViewMenu = new wxMenu();
    wxMenu* optionViewStyleMenu = new wxMenu();
    optionViewStyleMenu->AppendRadioItem(kuID_FIXED_AUTOFIT, STRING_MENU_FIXED_AUTOFIT);
    optionViewStyleMenu->AppendRadioItem(kuID_FIXED_BESTFIT, STRING_MENU_FIXED_BESTFIT);
    optionViewStyleMenu->AppendRadioItem(kuID_FIXED_EXTEND,  STRING_MENU_FIXED_EXTEND);
    optionViewStyleMenu->AppendRadioItem(kuID_FIXED_SCALE,   STRING_MENU_FIXED_SCALE);
    optionViewMenu->AppendSubMenu(optionViewStyleMenu,  STRING_MENU_FIXED_STYLE);
    optionViewMenu->AppendCheckItem(kuID_FIXED_ROTATE,  STRING_MENU_FIXED_ROTATE);
    optionViewMenu->AppendCheckItem(kuID_FIXED_LEFTTOP, STRING_MENU_FIXED_LEFTTOP);
    optionViewMenu->AppendCheckItem(kuID_LOAD_COMPLETELY, STRING_MENU_LOAD_COMPLETELY);
    optionViewMenu->AppendCheckItem(kuID_PREFETCH,        STRING_MENU_PREFETCH);
    optionMenu->AppendSubMenu(optionViewMenu,  STRING_MENU_OPTION_VIEW);
    wxMenu* optionRescaleMenu = new wxMenu();
    optionRescaleMenu->AppendRadioItem(kuID_RESCALE_BOX,        STRING_MENU_RESCALE_BOX);
    optionRescaleMenu->AppendRadioItem(kuID_RESCALE_BICUBIC,    STRING_MENU_RESCALE_BICUBIC);
    optionRescaleMenu->AppendRadioItem(kuID_RESCALE_BILINEAR,   STRING_MENU_RESCALE_BILINEAR);
    optionRescaleMenu->AppendRadioItem(kuID_RESCALE_BSPLINE,    STRING_MENU_RESCALE_BSPLINE);
    optionRescaleMenu->AppendRadioItem(kuID_RESCALE_CATMULLROM, STRING_MENU_RESCALE_CATMULLROM);
    optionRescaleMenu->AppendRadioItem(kuID_RESCALE_LANCZOS3,   STRING_MENU_RESCALE_LANCZOS3);
    optionMenu->AppendSubMenu(optionRescaleMenu,  STRING_MENU_OPTION_RESCALE);
    wxMenu* optionSlideshowMenu = new wxMenu();
    optionSlideshowMenu->AppendCheckItem(kuID_SLIDESHOW_REPEAT,   STRING_MENU_SLIDESHOW_REPEAT);
    optionSlideshowMenu->Append(         kuID_SLIDESHOW_INTERVAL, STRING_MENU_SLIDESHOW_INTERVAL);
    optionMenu->AppendSubMenu(optionSlideshowMenu, STRING_MENU_OPTION_SLIDESHOW);
    wxMenu* optionSummaryMenu = new wxMenu();
    optionSummaryMenu->Append(kuID_SUMMARY_POSITION,  STRING_MENU_SUMMARY_POSITION);
    optionSummaryMenu->Append(kuID_SUMMARY_COLOR,     STRING_MENU_SUMMARY_COLOR);
    optionMenu->AppendSubMenu(optionSummaryMenu,   STRING_MENU_OPTION_SUMMARY);
    wxMenu* optionManageMenu = new wxMenu();
    optionManageMenu->Append(kuID_MANAGE_DIRSET,  STRING_MENU_MANAGE_DIRSET);
    optionMenu->AppendSubMenu(optionManageMenu,   STRING_MENU_OPTION_MANAGE);
    optionMenu->Append(kuID_EXTERNAL_TOOLS,    STRING_MENU_EXTERNAL_TOOLS);
    wxMenu* optionRescaleMgrMenu = new wxMenu();
    optionRescaleMgrMenu->Append(kuID_RESCALEMGR_MAXCON, STRING_MENU_MANAGER_MAXCON);
    optionMenu->AppendSubMenu(optionRescaleMgrMenu, STRING_MENU_OPTION_RESCALEMGR);
    #ifdef ENABLE_PICASAWEBMGR
    wxMenu* optionPicasaWebMgrMenu = new wxMenu();
    optionPicasaWebMgrMenu->Append(kuID_PICASAWEBMGR_MAXCON, STRING_MENU_MANAGER_MAXCON);
    optionMenu->AppendSubMenu(optionPicasaWebMgrMenu, STRING_MENU_OPTION_PICASAWEBMGR);
    #endif
    optionMenu->AppendSeparator();
    optionMenu->Append(kuID_CONFIG, STRING_MENU_CONFIG);

    wxMenu* helpMenu = new wxMenu();
    helpMenu->Append(wxID_ABOUT, STRING_MENU_ABOUT);

    wxMenuBar* menuBar = new wxMenuBar();
    menuBar->Append(fileMenu,     STRING_MENU_FILE);
    menuBar->Append(editMenu,     STRING_MENU_EDIT);
    menuBar->Append(traverseMenu, STRING_MENU_TRAVERSE);
    menuBar->Append(viewMenu,     STRING_MENU_VIEW);
    menuBar->Append(panelMenu,    STRING_MENU_PANEL);
    menuBar->Append(manageMenu,   STRING_MENU_MANAGE);
    menuBar->Append(toolsMenu,    STRING_MENU_TOOLS);
    menuBar->Append(optionMenu,   STRING_MENU_OPTION);
    menuBar->Append(helpMenu,     STRING_MENU_HELP);
    SetMenuBar(menuBar);
}

void kuFrame::SetupToolBar(bool create) {
    if(create)    CreateToolBar(wxNO_BORDER|wxTB_HORIZONTAL|wxTB_FLAT);

    wxToolBar* toolBar = GetToolBar();
    if(!toolBar)    return;

    int numbtns=28;
    int numseps=9;
    int scale;
    int bmps = toolBar->GetToolSize().x - toolBar->GetToolBitmapSize().x;
    if(!bmps)    bmps = 10;
    int sepw = toolBar->GetToolSeparation();
    if(!sepw)    sepw = 7;
    for(scale=20; scale<=32; scale+=2) {
        if((scale+bmps)*numbtns+sepw*numseps > GetClientSize().x) {
            break;
        }
    }
    scale-=4;

    if(scale == toolBar->GetToolBitmapSize().x)    return;
    toolBar->ClearTools();

    wxBitmap tbs[kuID_HIGHEST-kuID_LOWEST-1];
    tbs[kuID_SAVE     -kuID_LOWEST-1] = wxIcon(wxICON(ICON_SAVE));
    tbs[kuID_INTERRUPT-kuID_LOWEST-1] = wxIcon(wxICON(ICON_INTERRUPT));
    tbs[kuID_RELOAD   -kuID_LOWEST-1] = wxIcon(wxICON(ICON_RELOAD));

    tbs[kuID_FILE_CUT    -kuID_LOWEST-1] = wxIcon(wxICON(ICON_FILE_CUT));
    tbs[kuID_FILE_COPY   -kuID_LOWEST-1] = wxIcon(wxICON(ICON_FILE_COPY));
    tbs[kuID_FILE_PASTE  -kuID_LOWEST-1] = wxIcon(wxICON(ICON_FILE_PASTE));
    tbs[kuID_FILE_DELETE -kuID_LOWEST-1] = wxIcon(wxICON(ICON_FILE_DELETE));

    tbs[kuID_GOHOME    -kuID_LOWEST-1] = wxIcon(wxICON(ICON_GOHOME));
    tbs[kuID_GOFAVORITE-kuID_LOWEST-1] = wxIcon(wxICON(ICON_GOFAVORITE));
    tbs[kuID_PREV   -kuID_LOWEST-1] = wxIcon(wxICON(ICON_PREV));
    tbs[kuID_NEXT   -kuID_LOWEST-1] = wxIcon(wxICON(ICON_NEXT));
    tbs[kuID_HOME   -kuID_LOWEST-1] = wxIcon(wxICON(ICON_HOME));
    tbs[kuID_END    -kuID_LOWEST-1] = wxIcon(wxICON(ICON_END));

    tbs[kuID_UP       -kuID_LOWEST-1] = wxIcon(wxICON(ICON_UP));
    tbs[kuID_DOWN     -kuID_LOWEST-1] = wxIcon(wxICON(ICON_DOWN));
    tbs[kuID_LEFT     -kuID_LOWEST-1] = wxIcon(wxICON(ICON_LEFT));
    tbs[kuID_RIGHT    -kuID_LOWEST-1] = wxIcon(wxICON(ICON_RIGHT));

    tbs[kuID_FULLSCREEN -kuID_LOWEST-1] = wxIcon(wxICON(ICON_FULLSCREEN));
    tbs[kuID_ZOOM_IN  -kuID_LOWEST-1] = wxIcon(wxICON(ICON_ZOOM_IN));
    tbs[kuID_ZOOM_OUT -kuID_LOWEST-1] = wxIcon(wxICON(ICON_ZOOM_OUT));
    tbs[kuID_ZOOM_100 -kuID_LOWEST-1] = wxIcon(wxICON(ICON_ZOOM_100));
    tbs[kuID_ZOOM_FIT -kuID_LOWEST-1] = wxIcon(wxICON(ICON_ZOOM_FIT));

    tbs[kuID_ROTATE_CCW-kuID_LOWEST-1] = wxIcon(wxICON(ICON_ROTATE_CCW));
    tbs[kuID_ROTATE_CW -kuID_LOWEST-1] = wxIcon(wxICON(ICON_ROTATE_CW));

    tbs[kuID_FILESYSTEM -kuID_LOWEST-1] = wxIcon(wxICON(ICON_FILESYSTEM));
    tbs[kuID_ARCHIVE    -kuID_LOWEST-1] = wxIcon(wxICON(ICON_ARCHIVE));
    tbs[kuID_THUMBNAIL  -kuID_LOWEST-1] = wxIcon(wxICON(ICON_THUMBNAIL));

    tbs[kuID_MINIMIZE -kuID_LOWEST-1] = wxIcon(wxICON(ICON_MINIMIZE));
    tbs[kuID_HIGHEST-1-kuID_LOWEST-1] = wxIcon(wxICON(ICON_EXIT));

    if(scale!=32) {   // resize
        for(int i=0;i<(kuID_HIGHEST-kuID_LOWEST-1);i++) {
            if(tbs[i].Ok())    // it is not ok when startup
                tbs[i]=wxBitmap(tbs[i].ConvertToImage().Scale(scale,scale));
        }
    }
    toolBar->SetToolBitmapSize(wxSize(scale,scale));

    toolBar->AddTool(kuID_SAVE,      StripCodes(STRING_MENU_SAVE),      tbs[kuID_SAVE     -kuID_LOWEST-1], StripCodes(STRING_MENU_SAVE));
    toolBar->AddTool(kuID_INTERRUPT, StripCodes(STRING_MENU_INTERRUPT), tbs[kuID_INTERRUPT-kuID_LOWEST-1], StripCodes(STRING_MENU_INTERRUPT));
    toolBar->AddTool(kuID_RELOAD,    StripCodes(STRING_MENU_RELOAD),    tbs[kuID_RELOAD   -kuID_LOWEST-1], StripCodes(STRING_MENU_RELOAD));
    toolBar->AddSeparator();
    toolBar->AddTool(kuID_FILE_CUT,    StripCodes(STRING_MENU_FILE_CUT),    tbs[kuID_FILE_CUT-kuID_LOWEST-1],    StripCodes(STRING_MENU_FILE_CUT));
    toolBar->AddTool(kuID_FILE_COPY,   StripCodes(STRING_MENU_FILE_COPY),   tbs[kuID_FILE_COPY-kuID_LOWEST-1],   StripCodes(STRING_MENU_FILE_COPY));
    toolBar->AddTool(kuID_FILE_PASTE,  StripCodes(STRING_MENU_FILE_PASTE),  tbs[kuID_FILE_PASTE-kuID_LOWEST-1],  StripCodes(STRING_MENU_FILE_PASTE));
    toolBar->AddTool(kuID_FILE_DELETE, StripCodes(STRING_MENU_FILE_DELETE), tbs[kuID_FILE_DELETE-kuID_LOWEST-1], StripCodes(STRING_MENU_FILE_DELETE));
    toolBar->AddSeparator();
    toolBar->AddTool(kuID_GOHOME,     StripCodes(STRING_MENU_GOHOME),     tbs[kuID_GOHOME-kuID_LOWEST-1],     StripCodes(STRING_MENU_GOHOME));
    toolBar->AddTool(kuID_GOFAVORITE, StripCodes(STRING_MENU_GOFAVORITE), tbs[kuID_GOFAVORITE-kuID_LOWEST-1], StripCodes(STRING_MENU_GOFAVORITE));
    toolBar->AddSeparator();
    toolBar->AddTool(kuID_PREV,    StripCodes(STRING_MENU_PREV), tbs[kuID_PREV-kuID_LOWEST-1],    StripCodes(STRING_MENU_PREV));
    toolBar->AddTool(kuID_NEXT,    StripCodes(STRING_MENU_NEXT), tbs[kuID_NEXT-kuID_LOWEST-1],    StripCodes(STRING_MENU_NEXT));
    toolBar->AddTool(kuID_HOME,    StripCodes(STRING_MENU_HOME), tbs[kuID_HOME-kuID_LOWEST-1],    StripCodes(STRING_MENU_HOME));
    toolBar->AddTool(kuID_END,     StripCodes(STRING_MENU_END),  tbs[kuID_END -kuID_LOWEST-1],    StripCodes(STRING_MENU_END));
    toolBar->AddSeparator();
    toolBar->AddTool(kuID_UP,      StripCodes(STRING_MENU_UP),   tbs[kuID_UP   -kuID_LOWEST-1],   StripCodes(STRING_MENU_UP));
    toolBar->AddTool(kuID_DOWN,    StripCodes(STRING_MENU_DOWN), tbs[kuID_DOWN -kuID_LOWEST-1],   StripCodes(STRING_MENU_DOWN));
    toolBar->AddTool(kuID_LEFT,    StripCodes(STRING_MENU_LEFT), tbs[kuID_LEFT -kuID_LOWEST-1],   StripCodes(STRING_MENU_LEFT));
    toolBar->AddTool(kuID_RIGHT,   StripCodes(STRING_MENU_RIGHT),tbs[kuID_RIGHT-kuID_LOWEST-1],   StripCodes(STRING_MENU_RIGHT));
    toolBar->AddSeparator();
    toolBar->AddTool(kuID_FULLSCREEN,StripCodes(STRING_MENU_FULLSCREEN),tbs[kuID_FULLSCREEN-kuID_LOWEST-1],StripCodes(STRING_MENU_FULLSCREEN));
    toolBar->AddSeparator();
    toolBar->AddTool(kuID_ZOOM_IN, StripCodes(STRING_MENU_ZOOM_IN), tbs[kuID_ZOOM_IN -kuID_LOWEST-1],StripCodes(STRING_MENU_ZOOM_IN));
    toolBar->AddTool(kuID_ZOOM_OUT,StripCodes(STRING_MENU_ZOOM_OUT),tbs[kuID_ZOOM_OUT-kuID_LOWEST-1],StripCodes(STRING_MENU_ZOOM_OUT));
    toolBar->AddTool(kuID_ZOOM_100,StripCodes(STRING_MENU_ZOOM_100),tbs[kuID_ZOOM_100-kuID_LOWEST-1],StripCodes(STRING_MENU_ZOOM_100));
    toolBar->AddTool(kuID_ZOOM_FIT,StripCodes(STRING_MENU_ZOOM_FIT),tbs[kuID_ZOOM_FIT-kuID_LOWEST-1],StripCodes(STRING_MENU_ZOOM_FIT));
    toolBar->AddSeparator();
    toolBar->AddTool(kuID_ROTATE_CCW,StripCodes(STRING_MENU_ROTATE_CCW), tbs[kuID_ROTATE_CCW-kuID_LOWEST-1], StripCodes(STRING_MENU_ROTATE_CCW));
    toolBar->AddTool(kuID_ROTATE_CW, StripCodes(STRING_MENU_ROTATE_CW),  tbs[kuID_ROTATE_CW-kuID_LOWEST-1],  StripCodes(STRING_MENU_ROTATE_CW));
    toolBar->AddSeparator();
    toolBar->AddCheckTool(kuID_FILESYSTEM,StripCodes(STRING_MENU_FILESYSTEM), tbs[kuID_FILESYSTEM-kuID_LOWEST-1],
                          tbs[kuID_FILESYSTEM-kuID_LOWEST-1],StripCodes(STRING_MENU_FILESYSTEM));
    toolBar->AddCheckTool(kuID_ARCHIVE,StripCodes(STRING_MENU_ARCHIVE),       tbs[kuID_ARCHIVE-kuID_LOWEST-1],
                          tbs[kuID_ARCHIVE-kuID_LOWEST-1]   ,StripCodes(STRING_MENU_ARCHIVE));
    toolBar->AddCheckTool(kuID_THUMBNAIL,StripCodes(STRING_MENU_THUMBNAIL),   tbs[kuID_THUMBNAIL-kuID_LOWEST-1],
                          tbs[kuID_THUMBNAIL-kuID_LOWEST-1] ,StripCodes(STRING_MENU_THUMBNAIL));
    toolBar->AddSeparator();
    toolBar->AddTool(kuID_MINIMIZE, StripCodes(STRING_MENU_MINIMIZE), tbs[kuID_MINIMIZE-kuID_LOWEST-1],  StripCodes(STRING_MENU_MINIMIZE));
    toolBar->AddTool(wxID_EXIT,     StripCodes(STRING_MENU_EXIT),     tbs[kuID_HIGHEST-1-kuID_LOWEST-1], StripCodes(STRING_MENU_EXIT));
    toolBar->Realize();

    toolBar->EnableTool(kuID_SAVE,       wxGetApp().GetEdited());
    toolBar->EnableTool(kuID_INTERRUPT,  wxGetApp().GetBusy());
    toolBar->ToggleTool(kuID_FILESYSTEM, mIsFilesystem);
    toolBar->ToggleTool(kuID_ARCHIVE,    mIsArchive);
    toolBar->ToggleTool(kuID_THUMBNAIL,  mIsThumbnail);

    //wxMessageBox(wxString::Format(wxT("%d, %d"),toolBar->GetToolBitmapSize().x, scale));
}

void kuFrame::SetupStatusBar() {
    mStatusBar = new kuStatusBar(this);
    SetStatusBar(mStatusBar);
}

void kuFrame::SetupWindows(bool isdir, bool isarchive) {
    if(isdir)   ToggleFilesystem(true);
    else {
        ToggleFilesystem(false);
        if(isarchive)   ToggleArchive(true);
    }
}

void kuFrame::SetupAccelerator() {
    /* IN+,OUT-,100*,FIT/ on numpad are processed in kuScrollHandler::OnKeyDown()
     * FIT/,EXT\ are processed in menu item
     * 100.,AUTO' are processed here */
    // accelerator
    wxAcceleratorEntry entries[25];
    entries[0].Set(wxACCEL_NORMAL,  46,         kuID_ZOOM_100);     // '.'
    entries[1].Set(wxACCEL_NORMAL,  39,         kuID_ZOOM_AUTO);    // '''
    entries[2].Set(wxACCEL_SHIFT,   WXK_DELETE, kuID_FILE_DELETE);
    entries[3].Set(wxACCEL_CTRL,    82,         kuID_RENAME);       // 'r'
    entries[4].Set(wxACCEL_ALT,     WXK_RETURN, kuID_FULLSCREEN);
    entries[5].Set(wxACCEL_CTRL,    66,         kuID_STATUSBAR);    // 'b'
    entries[6].Set(wxACCEL_NORMAL,  WXK_BACK,   kuID_BACK);
    entries[7].Set(wxACCEL_CTRL,    (int)'1',   kuID_COPYTO_1);
    entries[8].Set(wxACCEL_ALT,     (int)'1',   kuID_MOVETO_1);
    entries[9].Set(wxACCEL_CTRL,    (int)'2',   kuID_COPYTO_2);
    entries[10].Set(wxACCEL_ALT,    (int)'2',   kuID_MOVETO_2);
    entries[11].Set(wxACCEL_CTRL,   (int)'3',   kuID_COPYTO_3);
    entries[12].Set(wxACCEL_ALT,    (int)'3',   kuID_MOVETO_3);
    entries[13].Set(wxACCEL_CTRL,   (int)'4',   kuID_COPYTO_4);
    entries[14].Set(wxACCEL_ALT,    (int)'4',   kuID_MOVETO_4);
    entries[15].Set(wxACCEL_CTRL,   (int)'5',   kuID_COPYTO_5);
    entries[16].Set(wxACCEL_ALT,    (int)'5',   kuID_MOVETO_5);
    entries[17].Set(wxACCEL_CTRL,   (int)'6',   kuID_COPYTO_6);
    entries[18].Set(wxACCEL_ALT,    (int)'6',   kuID_MOVETO_6);
    entries[19].Set(wxACCEL_CTRL,   (int)'7',   kuID_COPYTO_7);
    entries[20].Set(wxACCEL_ALT,    (int)'7',   kuID_MOVETO_7);
    entries[21].Set(wxACCEL_CTRL,   (int)'8',   kuID_COPYTO_8);
    entries[22].Set(wxACCEL_ALT,    (int)'8',   kuID_MOVETO_8);
    entries[23].Set(wxACCEL_CTRL,   (int)'9',   kuID_COPYTO_9);
    entries[24].Set(wxACCEL_ALT,    (int)'9',   kuID_MOVETO_9);
//    entries[].Set(wxACCEL_NORMAL,  WXK_SPACE,  kuID_SLIDESHOW);
//    entries[].Set(wxACCEL_NORMAL,  WXK_ESCAPE, kuID_ESCAPE);
//    entries[].Set(wxACCEL_ALT,   WXK_LEFT,   kuID_ROTATE_CCW);
//    entries[].Set(wxACCEL_ALT,   WXK_RIGHT,  kuID_ROTATE_CW);
    SetAcceleratorTable(wxAcceleratorTable(25, entries));

    Connect(kuID_COPYTO_1, kuID_COPYTO_9, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(kuFrame::OnCopyTo));
    Connect(kuID_MOVETO_1, kuID_MOVETO_9, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(kuFrame::OnMoveTo));
}

void kuFrame::SetupPrinting() {
    mPageSetupDlgData.SetPaperId(wxPAPER_A4);
    mPageSetupDlgData.SetDefaultMinMargins(true);
    mPageSetupDlgData.SetMarginTopLeft(wxPoint(10,10));
    mPageSetupDlgData.SetMarginBottomRight(wxPoint(10,10));
    mPageSetupDlgData.GetPrintData().SetBin(wxPRINTBIN_DEFAULT);
    mPageSetupDlgData.GetPrintData().SetColour(true);
    mPageSetupDlgData.GetPrintData().SetQuality(wxPRINT_QUALITY_HIGH);
}

void kuFrame::SetDefaults() {
    // check filesystem/archive/thumbnail/summary
    GetMenuBar()->Check(kuID_FILESYSTEM,      mIsFilesystem);
    GetToolBar()->ToggleTool(kuID_FILESYSTEM, mIsFilesystem);
    GetMenuBar()->Check(kuID_ARCHIVE,         mIsArchive);
    GetToolBar()->ToggleTool(kuID_ARCHIVE,    mIsArchive);
    GetMenuBar()->Check(kuID_THUMBNAIL,       mIsThumbnail);
    GetToolBar()->ToggleTool(kuID_THUMBNAIL,  mIsThumbnail);
    GetMenuBar()->Check(kuID_SUMMARY,         mIsSummary);

    // enable/disable menubar/toolbar
    GetMenuBar()->Enable(kuID_SAVE,      false);
    GetMenuBar()->Enable(kuID_SAVEAS,    false);
    GetMenuBar()->Enable(kuID_INTERRUPT, false);
    if(CanSetTransparent()) {    // TODO: maybe return true but no such item exists
        GetMenuBar()->Enable(kuID_OPAQUE, mSingle->CanSetTransparent());
    }
    GetMenuBar()->Enable(kuID_CONFIG, !wxGetApp().mIsImported);
    GetToolBar()->EnableTool(kuID_SAVE,      false);
    GetToolBar()->EnableTool(kuID_INTERRUPT, false);

    // check options
    switch (wxGetApp().mOptions.mKeepStyle) {
        case SCALE_AUTOFIT:
            GetMenuBar()->Check(kuID_FIXED_AUTOFIT, true);
            break;
        case SCALE_BESTFIT:
            GetMenuBar()->Check(kuID_FIXED_BESTFIT, true);
            break;
        case SCALE_EXTEND:
            GetMenuBar()->Check(kuID_FIXED_EXTEND,  true);
            break;
        case SCALE_LASTUSED:
            GetMenuBar()->Check(kuID_FIXED_SCALE,   true);
            break;
        default:
            GetMenuBar()->Check(kuID_FIXED_AUTOFIT, true);
    }
    GetMenuBar()->Check(kuID_FIXED_ROTATE,    wxGetApp().mOptions.mKeepRotate);
    GetMenuBar()->Check(kuID_FIXED_LEFTTOP,   wxGetApp().mOptions.mKeepLeftTop);
    GetMenuBar()->Check(kuID_LOAD_COMPLETELY, wxGetApp().mOptions.mLoadCompletely);
    GetMenuBar()->Check(kuID_PREFETCH,        wxGetApp().mOptions.mPrefetch);

    GetMenuBar()->Check(kuFiWrapper::GetIdByFilter(wxGetApp().mOptions.mFilter), true);

    GetMenuBar()->Check(kuID_SLIDESHOW_REPEAT, wxGetApp().mOptions.mSlideshowRepeat);

    // enable menu in mSingle
    mSingle->mMenu->Enable(kuID_SAVE,   false);
    mSingle->mMenu->Enable(kuID_SAVEAS, false);
    if(CanSetTransparent()) {
        mSingle->mMenu->Enable(kuID_OPAQUE, mSingle->CanSetTransparent());
    }
    // check menu in mSingle
    mSingle->mMenu->Check(kuID_SUMMARY, mIsSummary);
}

void kuFrame::ClearExternalOpenMenu() {
    // must remove the menu before add/delete its item
    int menuidx = GetMenuBar()->FindMenu(STRING_MENU_TOOLS);
    if(menuidx==wxNOT_FOUND)    return;
    wxMenu* toolsMenu = GetMenuBar()->Remove(menuidx);
    size_t old = wxGetApp().mOptions.mExternalOpenNames.GetCount();
    for(size_t i=0; i<old; i++) {
        if(toolsMenu->FindItem(kuID_EXTERNAL_START+i)) {
            toolsMenu->Destroy(kuID_EXTERNAL_START+i);
        }
    }
    GetMenuBar()->Insert(menuidx, toolsMenu, STRING_MENU_TOOLS);

    mSingle->ClearExternalOpenMenu();
}
void kuFrame::SetupExternalOpenMenu() {
    // must remove the menu before add/delete its item
    int menuidx = GetMenuBar()->FindMenu(STRING_MENU_TOOLS);
    if(menuidx==wxNOT_FOUND)    return;
    wxMenu* toolsMenu = GetMenuBar()->Remove(menuidx);
    size_t count = wxGetApp().mOptions.mExternalOpenNames.GetCount();
    // TODO: find the position after STRING_MENU_EXTERNAL_EDIT
    #ifdef __WXMSW__
    int itemidx = 2;
    #else
    int itemidx = 1;    // no edit
    #endif
    if(itemidx==wxNOT_FOUND) {
        GetMenuBar()->Insert(menuidx, toolsMenu, STRING_MENU_TOOLS);
        return;
    }
    for(size_t i=0; i<count; i++) {
        wxString str = wxString(STRING_MENU_EXTERNAL_OPEN);
        str.Replace(wxT("%TOOL%"), wxGetApp().mOptions.mExternalOpenNames[i]);
        toolsMenu->Insert(itemidx+i, kuID_EXTERNAL_START+i, str);
    }
    GetMenuBar()->Insert(menuidx, toolsMenu, STRING_MENU_TOOLS);
    Connect(kuID_EXTERNAL_START, kuID_EXTERNAL_START+count,
            wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(kuFrame::OnExternalOpen));

    mSingle->SetupExternalOpenMenu();
}

void kuFrame::OnQuit(wxCommandEvent& event) {
    wxGetApp().mFrame->Close();
}

void kuFrame::OnAbout(wxCommandEvent& event) {
    wxDialog about(this,wxID_ANY,StripCodes(STRING_MENU_ABOUT),wxDefaultPosition,wxDefaultSize);
    about.SetIcon(wxIcon(wxICON(ICON_APP)));
    wxStaticBoxSizer* topSizer = new wxStaticBoxSizer(wxVERTICAL,&about);
    wxFlexGridSizer* mainSizer = new wxFlexGridSizer(2,2, 0, 0);
    topSizer->Add(mainSizer,1,wxALL|wxALIGN_CENTER);
    // version
    topSizer->Add(new wxStaticText(&about,wxID_ANY,STRING_VERSION),0,wxALL|wxALIGN_CENTER,5);
    // library
    topSizer->Add(new wxStaticLine(&about),0,wxEXPAND);
    wxString wxverstr = wxVERSION_STRING;
    topSizer->Add(new wxStaticText(&about,wxID_ANY,wxverstr),0,wxALL|wxALIGN_CENTER,5);
    wxString fiverstr = wxT("FreeImage ") + wxString(FreeImage_GetVersion(),wxConvUTF8);
    topSizer->Add(new wxStaticText(&about,wxID_ANY,fiverstr),0,wxALL|wxALIGN_CENTER,5);
    // translator
    wxString translator = wxGetApp().mLocale->GetHeaderValue(wxT("Last-Translator"), wxT("kuview_")+wxGetApp().mLocale->GetCanonicalName());
    if(translator!=wxEmptyString) {
        topSizer->Add(new wxStaticLine(&about),0,wxEXPAND);
        wxString transstr = wxGetApp().mLocale->GetCanonicalName() + wxT(": ") + translator.BeforeFirst('<').BeforeLast(' ');
        topSizer->Add(new wxStaticText(&about,wxID_ANY,transstr),0,wxALL|wxALIGN_CENTER,5);
    }
    // ok button
    topSizer->Add(new wxStaticLine(&about),0,wxEXPAND);
    topSizer->Add(new wxButton(&about,wxID_OK,wxEmptyString,wxDefaultPosition,wxDefaultSize,wxNO_BORDER),0,wxALIGN_CENTER);
    about.SetSizer(topSizer);
    mainSizer->Add(new wxStaticBitmap(&about,wxID_ANY,wxIcon(wxICON(ICON_APP))),0,wxALL|wxALIGN_CENTER,10);
    mainSizer->Add(new wxStaticText(&about,wxID_ANY,STRING_PROJECT),1,wxRIGHT|wxALIGN_CENTER,10);
    mainSizer->Add(new wxStaticBitmap(&about,wxID_ANY,wxIcon(wxICON(ICON_ORG))),0,wxALL|wxALIGN_CENTER,10);
    mainSizer->Add(new wxStaticText(&about,wxID_ANY,STRING_AUTHOR),1,wxRIGHT|wxALIGN_CENTER,10);
    about.Fit();
    about.ShowModal();
}

void kuFrame::OnCopyTo(wxCommandEvent& event) {
    int idx = event.GetId() - kuID_COPYTO_1;
    //wxMessageBox(wxString::Format(wxT("idx=%d, count=%d"), idx, wxGetApp().mOptions.mHotkeyDirs.GetCount()));
    if(idx < wxGetApp().mOptions.mHotkeyDirs.GetCount()) {
        Action(kuID_FILE_COPYTO, wxGetApp().mOptions.mHotkeyDirs[idx]);
    }
}

void kuFrame::OnMoveTo(wxCommandEvent& event) {
    int idx = event.GetId() - kuID_MOVETO_1;
    //wxMessageBox(wxString::Format(wxT("idx=%d, count=%d"), idx, wxGetApp().mOptions.mHotkeyDirs.GetCount()));
    if(idx < wxGetApp().mOptions.mHotkeyDirs.GetCount()) {
        Action(kuID_FILE_MOVETO, wxGetApp().mOptions.mHotkeyDirs[idx]);
    }
}

void kuFrame::OnTool(wxCommandEvent& event) {
    //wxMessageBox(wxString::Format(wxT("%d"),event.GetId()));
    Action(event.GetId());
}

void kuFrame::OnExternalOpen(wxCommandEvent& event) {
    Action(kuID_EXTERNAL_OPEN, wxString::Format(FORMAT_EXTERNAL_OPEN, event.GetId()-kuID_EXTERNAL_START));
}

void kuFrame::OnSize(wxSizeEvent& event) {
    SetupToolBar();
    //wxMessageBox(wxString::Format(wxT("%d"),GetClientSize().x));
    event.Skip();
}

void kuFrame::OnClose(wxCloseEvent& event) {
    if(wxGetApp().GetBusy())   Action(kuID_INTERRUPT);
    wxGetApp().mQuit = true;
    if(!wxGetApp().mFrame->IsShown())   wxGetApp().mFrame->mTaskBarIcon->RemoveIcon();
    mRescaleMgrDlg->Show(false);
    mRescaleMgrDlg->Destroy();
    #ifdef ENABLE_PICASAWEBMGR
    mPicasaWebMgrDlg->Show(false);
    mPicasaWebMgrDlg->Destroy();
    #endif
    delete mTaskBarIcon;

    mGeneric->GetTreeCtrl()->PopEventHandler(true);
    mVirtual->PopEventHandler(true);
    mSingle->PopEventHandler(true);
    mMultiple->PopEventHandler(true);
    event.Skip();
}

void kuFrame::OnLoaded(wxCommandEvent& event) {
    wxString filename = event.GetString();
    wxString self = THREAD_NAME_CURRENT;
    wxLogDebug(wxT("%s OnLoaded: %s"), self.c_str(), filename.c_str());
    wxString current = GetCurrentPath(true,true);
    if(current==filename)    mSingle->ReloadImage(current, mIsArchive, true);
}

void kuFrame::OnDropped(wxCommandEvent& event) {
    wxArrayString* files = (wxArrayString*)event.GetClientData();
    wxString self = THREAD_NAME_CURRENT;
    // replace dir with files. url is not supported.
    size_t index = 0;
    while(index < files->GetCount()) {
        if(wxDirExists(files->Item(index))) {
            wxArrayString children;
            wxDir::GetAllFiles(files->Item(index), &children);
            children.Sort(true);
            if(!children.IsEmpty()) {
                for(size_t i=0; i<children.GetCount(); i++) {
                    if(kuApp::IsImageFile(children[i]))
                        files->Insert(children[i], index+1);
                }
            }
            files->RemoveAt(index);
            continue;
        }
        index++;
    }
    for(size_t i=0; i<files->GetCount(); i++)    wxLogDebug(wxT("%s OnDropFiles: %s"), self, files->Item(i));
    if(files->GetCount() > 0)    mGeneric->Locate(files->Item(0));
    if(files->GetCount()>1 && !mIsThumbnail)    ToggleThumbnail(true);
    if(mIsThumbnail)    mMultiple->AddThumbs(*files);
    delete files;
}

void kuFrame::OnIdle(wxIdleEvent& event) {
    if(wxGetApp().mIdleTasks.IsEmpty())    return;

    wxGetApp().mIsDoingIdleTask = true;
    wxString task = wxGetApp().mIdleTasks[0];

    wxStringTokenizer ttkz(task, wxT("|"));
    wxArrayString tokens;
    while(ttkz.HasMoreTokens())
        tokens.Add(ttkz.GetNextToken());

    wxBeginBusyCursor();
    if(tokens[1].CmpNoCase(CMD_EXPLORE)==0) {
        wxGetApp().mIsExploring = true;

        wxChar sep = wxFileName::GetPathSeparator();
        wxStringTokenizer ptkz(tokens[2], sep);
        wxArrayString dirs;
        while(ptkz.HasMoreTokens())
            dirs.Add(ptkz.GetNextToken());

        wxString path = wxEmptyString;
        size_t index = 1;
        for(size_t i=0; i<dirs.GetCount(); i++) {
            if(dirs[i]==wxEmptyString) {
                path += sep;
                continue;
            } else if(path.Last()==sep || path.IsEmpty()) {
                path += dirs[i];
            } else {
                path += sep + dirs[i];
            }
            //wxMessageBox(path);
            if(kuApp::IsUNCPath(tokens[2]) && i==2)    continue;                  // ignore \\host since it cannot be expanded in filesystem panel
            if(kuApp::IsUNCPath(tokens[2]) && i==3)    mGeneric->Locate(path);    // insert \\host\dir
            else    wxGetApp().mIdleTasks.Insert(wxString::Format(FORMAT_IDLETASK, i==(dirs.GetCount()-1)?0:1, wxString(CMD_LOCATE).wc_str(), path.wc_str()), index++);
        }
        if(index > 1)    event.RequestMore();
    }
    if(tokens[1].CmpNoCase(CMD_LOCATE)==0) {
        mGeneric->SetPath(tokens[2]);
        if(atoi(tokens[0].ToAscii()) == 1)    event.RequestMore();
        else    wxGetApp().mIsExploring = false;
    }
    wxEndBusyCursor();

    wxGetApp().mIdleTasks.RemoveAt(0);
    wxGetApp().mIsDoingIdleTask = false;
}

/* global function */
bool kuFrame::Action(int action, wxString arg1, wxString arg2) {
    switch (action) {
        // menu file
        case kuID_SAVE:
            if(mIsArchive)   break;   // don't support edit in archive
            if(mSingle->SaveImage(mGeneric->GetFilePath())) {
                wxGetApp().SetEdited(false);
            }
            break;
        case kuID_SAVEAS: {
            if(mIsArchive)   break;   // don't support edit in archive
            wxFileName oldname(mGeneric->GetFilePath());
            wxString newname=wxFileSelector(StripCodes(STRING_MENU_SAVEAS),
                                            wxEmptyString,oldname.GetFullName(),wxEmptyString,
                                            STRING_FILTER_STANDARD + wxString::Format(wxT("(%s)"),kuFiWrapper::GetSupportedExtensions().AfterFirst('|').wc_str()) + kuFiWrapper::GetSupportedExtensions(),
                                            #ifndef _WX_26_
                                            wxFD_SAVE|wxFD_OVERWRITE_PROMPT);
                                            #else
                                            wxSAVE|wxOVERWRITE_PROMPT);
                                            #endif
            //wxMessageBox(newname);
            if(newname.empty())   break;
            if(mSingle->SaveImage(newname)) {
                wxGetApp().SetEdited(false);
                // reload filesystem
                ReloadPath(newname,false);
            }
            break;
        }
        case kuID_PAGESETUP: {
            wxPageSetupDialog dlg(this, &mPageSetupDlgData);
            if(dlg.ShowModal()==wxID_OK)    mPageSetupDlgData = dlg.GetPageSetupData();
            break;
        }
        case kuID_PREVIEW: {
            // check selected item
            if(GetCurrentPath(false,true)==wxEmptyString) {
                   SetStatusText(STRING_WARNING_ISNOTFILE);
                   break;
            }
            // auto set orientation
            mSingle->GetOrigSize().x>mSingle->GetOrigSize().y ?
                mPageSetupDlgData.GetPrintData().SetOrientation(wxLANDSCAPE) : mPageSetupDlgData.GetPrintData().SetOrientation(wxPORTRAIT);
            // preview frame
            wxString filename = mGeneric->GetFilePath();
            wxPrintPreview* preview = new wxPrintPreview(new kuPrintout(mSingle,&mPageSetupDlgData),
                                                         new kuPrintout(mSingle,&mPageSetupDlgData),
                                                         new wxPrintData(mPageSetupDlgData.GetPrintData()));
            if(!preview->Ok()) {
                delete preview;
                wxMessageBox(STRING_ERROR_PREVIEW_FAILED);
                break;
            }
            kuPreviewFrame* frame = new kuPreviewFrame(preview, this, wxT("Preview"));
            frame->Initialize();
            frame->Maximize();
            frame->Show();
            break;
        }
        case kuID_PRINT: {
            // check selected item
            if(GetCurrentPath(false,true)==wxEmptyString) {
                   SetStatusText(STRING_WARNING_ISNOTFILE);
                   break;
            }
            // auto set orientation
            mSingle->GetOrigSize().x>mSingle->GetOrigSize().y ?
                mPageSetupDlgData.GetPrintData().SetOrientation(wxLANDSCAPE) : mPageSetupDlgData.GetPrintData().SetOrientation(wxPORTRAIT);
            // printer
            wxPrintDialogData data(mPageSetupDlgData.GetPrintData());
            wxPrinter printer(&data);
            kuPrintout printout(mSingle, &mPageSetupDlgData);
            if(printer.Print(this, &printout, true)) {
                mPageSetupDlgData.SetPrintData(printer.GetPrintDialogData().GetPrintData());
            } else {    // print failed
                if(wxPrinter::GetLastError() == wxPRINTER_ERROR)    wxMessageBox(STRING_ERROR_PRINT_FAILED);
                else    SetStatusText(STRING_INFO_CANCELED);
            }
            break;
        }
        case kuID_EXPLORE: {
            #ifdef __WXMSW__
            ShellExecute(0, 0, wxT("explorer"), wxT("/n,/select,")+mGeneric->GetPath(), 0, SW_SHOW);
            #else
            wxExecute(wxT("xdg-open ")+mGeneric->GetDir());    // doesn't support selecting file
            #endif
            break;
        }
        #ifdef __WXMSW__
        case kuID_EXTERNAL_EDIT:
            ShellExecute(0, wxT("edit"), mGeneric->GetPath(), 0, 0, SW_SHOW);
            break;
        #endif
        case kuID_EXTERNAL_OPEN: {
            //ShellExecute(0, 0, wxT("rundll32.exe"), wxT("C:\\WINDOWS\\system32\\shimgvw.dll,ImageView_Fullscreen ")+mGeneric->GetPath(), 0, SW_SHOW);
            int id;
            wxSscanf(arg1, FORMAT_EXTERNAL_OPEN, &id);
            wxString exec = wxGetApp().mOptions.mExternalOpenExecs[id];
            wxString args = wxGetApp().mOptions.mExternalOpenArgss[id];
            // replace filename and dirname
            args.Replace(wxString(EXTERNAL_VAR_FILE).Upper(), mGeneric->GetPath());
            args.Replace(wxString(EXTERNAL_VAR_FILE).Lower(), mGeneric->GetPath());
            args.Replace(wxString(EXTERNAL_VAR_DIR).Upper(),  mGeneric->GetDir());
            args.Replace(wxString(EXTERNAL_VAR_DIR).Lower(),  mGeneric->GetDir());
            // replace latitude and longitude
            double la = kuFiWrapper::GetGPSLatitude(mSingle->GetOrigBmp());
            wxString lastr = wxString::Format(wxT("%.6f"), la);
            int cau = args.Replace(wxString(EXTERNAL_VAR_LATITUDE).Upper(),lastr);
            int cal = args.Replace(wxString(EXTERNAL_VAR_LATITUDE).Lower(),lastr);
            double lo = kuFiWrapper::GetGPSLongitude(mSingle->GetOrigBmp());
            wxString lostr = wxString::Format(wxT("%.6f"), lo);
            int cou = args.Replace(wxString(EXTERNAL_VAR_LONGITUDE).Upper(),lostr);
            int col = args.Replace(wxString(EXTERNAL_VAR_LONGITUDE).Lower(),lostr);
            if(wxMax(cau,cal) && la==INVALID_LATITUDE || wxMax(cou,col) && lo==INVALID_LONGITUDE) {
                SetStatusText(STRING_INFO_GPSLL_NOTFOUND);
                break;
            }
            //wxMessageBox(exec);
            //wxMessageBox(args);
            #ifdef __WXMSW__
            if(exec==EXTERNAL_OPERATION_EDIT || exec==EXTERNAL_OPERATION_OPEN) {
                ShellExecute(0, exec, args, 0, 0, SW_SHOW);
            } else {
                ShellExecute(0, 0, exec, args, 0, SW_SHOW);
            }
            #else
            wxExecute(wxString::Format(wxT("%s %s"), exec.wc_str(), args.wc_str()));
            #endif
            break;
        }
        case kuID_EXTERNAL_TOOLS: {
            kuEntryEditorDialog dialog(this, StripCodes(STRING_MENU_EXTERNAL_TOOLS),
                                       wxString(CFG_EXTERNAL_NAME),
                                       wxString(CFG_EXTERNAL_EXEC),
                                       wxString(CFG_EXTERNAL_ARGS),
                                       wxGetApp().mOptions.mExternalOpenNames,
                                       wxGetApp().mOptions.mExternalOpenExecs,
                                       wxGetApp().mOptions.mExternalOpenArgss);
            if(dialog.ShowModal()==wxID_OK) {
                ClearExternalOpenMenu();
                wxGetApp().mOptions.mExternalOpenNames = dialog.GetNames();
                wxGetApp().mOptions.mExternalOpenExecs = dialog.GetValue1s();
                wxGetApp().mOptions.mExternalOpenArgss = dialog.GetValue2s();
                SetupExternalOpenMenu();
                Action(kuID_CONFIG);
            }
            break;
        }
        #ifdef __WXMSW__
        case kuID_PROPERTIES: {
            wxString filename = GetCurrentPath(false);
            SHELLEXECUTEINFO sei;
            ZeroMemory(&sei, sizeof(sei));
            sei.cbSize = sizeof(sei);
            sei.lpFile = filename.wc_str();
            sei.lpVerb = wxT("properties");
            sei.fMask = SEE_MASK_INVOKEIDLIST;
            ShellExecuteEx(&sei);
            break;
        }
        #endif
        case kuID_METADATA: {
            if(GetCurrentPath(false,true)==wxEmptyString){
                SetStatusText(STRING_WARNING_ISNOTFILE);
                return false;
            }
            wxArrayString* metadata = new wxArrayString();
            FITAG* tag = NULL;
            FIMETADATA* mdhandle = NULL;

            FIBITMAP* bmp = mSingle->GetOrigBmp();
            for(size_t i=0; i<FIMD_CUSTOM; i++) {
                if(!FreeImage_GetMetadataCount((FREE_IMAGE_MDMODEL)i, bmp))    continue;    // no data for this model
                metadata->Add(kuFiWrapper::GetMdModelString((FREE_IMAGE_MDTYPE)i));
                mdhandle = FreeImage_FindFirstMetadata((FREE_IMAGE_MDMODEL)i, bmp, &tag);
                wxString data;
                if(mdhandle) {
                    do {
                        data += wxString(FreeImage_GetTagKey(tag), wxConvUTF8) + METADATA_COL_SEP
                                + wxString(FreeImage_TagToString((FREE_IMAGE_MDMODEL)i, tag), wxConvUTF8) + METADATA_COL_SEP
                                + wxString(FreeImage_GetTagDescription(tag), wxConvUTF8) + METADATA_TAG_SEP;
                    } while(FreeImage_FindNextMetadata(mdhandle, &tag));
                    metadata->Add(data);
                }
                FreeImage_FindCloseMetadata(mdhandle);
            }
            if(metadata->Count()) {
                kuMetaSheetDialog dialog(this,this,mGeneric->GetFilePath(),*metadata);
                dialog.Centre();
                dialog.ShowModal();
            } else {
                SetStatusText(STRING_INFO_METADATA_NOTFOUND);
            }
            delete metadata;
            break;
        }
        case kuID_INTERRUPT:
            if(wxGetApp().SetInterrupt(true)) {
                GetMenuBar()->Enable(kuID_INTERRUPT,false);
                GetToolBar()->EnableTool(kuID_INTERRUPT,false);
            }
            break;
        case kuID_RELOAD:
            if(wxWindow::FindFocus()==mGeneric->GetTreeCtrl()) {
                ReloadPath(mGeneric->GetPath(),false);
            } else {
                mSingle->ReloadImage(GetCurrentPath(true,true),mIsArchive);
            }
            break;
        case kuID_ESCAPE: {
            if(mIsSlideshow) {
                Action(kuID_INTERRUPT);
                mIsPause = false;
                GetMenuBar()->SetLabel(kuID_SLIDESHOW,   STRING_MENU_SLIDESHOW_START);
                mSingle->mMenu->SetLabel(kuID_SLIDESHOW, STRING_MENU_SLIDESHOW_START);
                mSlideshowDirs.Clear();
                mSlideshowIndex = 0;
                break;
            }
            if(IsFullScreen()) {
                if(mIsPause) {
                    GetMenuBar()->SetLabel(kuID_SLIDESHOW,   STRING_MENU_SLIDESHOW_START);
                    mSingle->mMenu->SetLabel(kuID_SLIDESHOW, STRING_MENU_SLIDESHOW_START);
                    mSlideshowDirs.Clear();
                    mSlideshowIndex = 0;
                    mIsPause = false;
                }
                mSingle->SetFullScreen(false);
                break;
            }
            wxCommandEvent evt(wxEVT_COMMAND_MENU_SELECTED, wxID_EXIT);
            AddPendingEvent(evt);
            break;
        }
        case kuID_MINIMIZE:
            mTaskBarIcon->SetIcon(mIconApp,STRING_APPNAME);
            Show(false);
            break;
        // menu edit
        case kuID_ROTATE_CCW:
            wxGetApp().CheckEdited();
            mSingle->Rotate90(false);
            // keep rotate
            wxGetApp().mOptions.mRotate--;
            wxGetApp().mOptions.mRotate %= 4;
            break;
        case kuID_ROTATE_CW:
            wxGetApp().CheckEdited();
            mSingle->Rotate90(true);
            // keep rotate
            wxGetApp().mOptions.mRotate++;
            wxGetApp().mOptions.mRotate %= 4;
            break;
        case kuID_RESCALE: {
            wxGetApp().CheckEdited();
            double scale = kuApp::GetScaleFromUser(this);
            if(scale!=-1)    mSingle->Rescale(scale);
            break;
        }
        // menu traverse
        case kuID_GOHOME:
            mGeneric->ExpandPath(wxGetHomeDir());
            break;
        case kuID_GOFAVORITE:
            mGeneric->ExpandPath(wxGetApp().mOptions.mFavorite);
            break;
        case kuID_PREV:
        case kuID_NEXT:
        case kuID_HOME:
        case kuID_END:
        case kuID_UP:
        case kuID_DOWN:
        case kuID_LEFT:
        case kuID_RIGHT:
            if(mIsSlideshow) {    // stop first if in slideshow (and assume is fullscreen)
                Action(kuID_SLIDESHOW);
            }
            if(mIsArchive)   Traverse(mVirtual,action);
            else   Traverse(mGeneric->GetTreeCtrl(),action);
            break;
        case kuID_BACK:
            if(mHistoryMainIndex < 1) {
                SetStatusText(STRING_INFO_HISTORY_FIRST);
                break;
            }
            mIsHistory = true;
            mGeneric->Locate(mHistoryMainFiles[--mHistoryMainIndex]);
            mIsHistory = false;
            break;
        case kuID_FORWARD:
            if(mHistoryMainIndex+1 >= mHistoryMainFiles.GetCount()) {
                SetStatusText(STRING_INFO_HISTORY_LAST);
                break;
            }
            mIsHistory = true;
            mGeneric->Locate(mHistoryMainFiles[++mHistoryMainIndex]);
            mIsHistory = false;
            break;
        case kuID_BRANCH: {
            if(mHistoryMainIndex<=mHistorySwapIndex || mHistoryBranchFiles.IsEmpty()) {
                SetStatusText(STRING_INFO_HISTORY_NOBRANCH);
                break;
            }
            mIsHistory = true;
            wxArrayString tmpFiles = mHistoryBranchFiles;
            size_t tmpIndex = mHistoryBranchIndex;
            mHistoryBranchFiles.Clear();
            for(size_t i=mHistoryMainFiles.GetCount()-1; i>mHistorySwapIndex; i--) {
                mHistoryBranchFiles.Insert(mHistoryMainFiles[i], 0);
                mHistoryMainFiles.RemoveAt(i);
            }
            mHistoryBranchIndex = mHistoryMainIndex - mHistorySwapIndex - 1;
            for(size_t i=0; i<tmpFiles.GetCount(); i++) {
                mHistoryMainFiles.Add(tmpFiles[i]);
            }
            mHistoryMainIndex = mHistorySwapIndex + tmpIndex + 1;
            mGeneric->Locate(mHistoryMainFiles[mHistoryMainIndex]);
            mIsHistory = false;
            break;
        }
        // menu view
        case kuID_FULLSCREEN:
            if(GetCurrentPath(false,true)==wxEmptyString)
                return false;
            if(mIsSlideshow || mIsPause) {
                if(mIsSlideshow)    Action(kuID_INTERRUPT);
                else    mSingle->SetFullScreen(false);    // leave fullscreen for pause
                mIsPause = false;
                GetMenuBar()->SetLabel(kuID_SLIDESHOW,   STRING_MENU_SLIDESHOW_START);
                mSingle->mMenu->SetLabel(kuID_SLIDESHOW, STRING_MENU_SLIDESHOW_START);
                mSlideshowDirs.Clear();
                mSlideshowIndex = 0;
                break;    // interrupt slideshow has caused leaving fullscreen
            }
            if(!IsFullScreen())    mIsStatusbar = false;
            mSingle->SetFullScreen(!IsFullScreen());
            break;
        case kuID_ZOOM_IN:
            mSingle->SetScale(SCALE_DIFF);
            break;
        case kuID_ZOOM_OUT:
            mSingle->SetScale(-SCALE_DIFF);
            break;
        case kuID_ZOOM_100:
            wxGetApp().CheckEdited();
            mSingle->SetScale(SCALE_ORIGINAL);
            break;
        case kuID_ZOOM_FIT:
            mSingle->SetScale(SCALE_BESTFIT);
            break;
        case kuID_ZOOM_EXT:
            mSingle->SetScale(SCALE_EXTEND);
            break;
        case kuID_ZOOM_AUTO:
            mSingle->SetScale(SCALE_AUTOFIT);
            break;
        case kuID_OPAQUE: {
            long input = wxGetNumberFromUser(STRING_OPAQUE_MESSAGE,STRING_OPAQUE_PROMPT,StripCodes(STRING_MENU_OPAQUE),wxGetApp().mOptions.mOpaque,0,255);
            if(input==-1) {
                SetStatusText(STRING_INFO_CANCELED);
                return false;
            }
            if(IsFullScreen())    SetTransparent(input);
            else    mSingle->SetTransparent(input);
            wxGetApp().mOptions.mOpaque = input;
            break;
        }
        case kuID_SLIDESHOW: {
            // check selected item
            if(mIsArchive) {    // don't use mSlideshowDirs
               if(mVirtual->GetFilePath(false,true)==wxEmptyString) {
                   SetStatusText(STRING_WARNING_ISNOTFILE);
                   break;
               }
            } else {    // use mSlideshowDirs
               if(mSlideshowDirs.IsEmpty()) {   // mSlideshowDirs is empty
                   if(mGeneric->GetFilePath()==wxEmptyString) {    // cannot be added to mSlideshowDirs
                       SetStatusText(STRING_WARNING_ISNOTFILE);
                       break;
                   }
                   mSlideshowDirs.Add(mGeneric->GetDir());
                   mSlideshowIndex = 0;
               }
               // if current is not the next to play, and just play if in the same dir
               wxString target = mSlideshowDirs[mSlideshowIndex];
               if(target.Last()==wxFileName::GetPathSeparator() && mGeneric->GetDir().Last()!=wxFileName::GetPathSeparator())    target.RemoveLast();
               if(mGeneric->GetDir().CmpNoCase(target)) {
                   mGeneric->Locate(mSlideshowDirs[mSlideshowIndex]);
                   wxSafeYield();
               }
            }
            long interval = wxGetApp().mOptions.mSlideshowInterval;
            if(IsFullScreen()) {    // fullscreen, pause or continue
                // let ui update for getting correct splash background
                wxSafeYield();
                if(mIsSlideshow) {    // pause
                    wxImage img = wxBitmap(wxICON(ICON_INTERRUPT)).ConvertToImage().Rescale(64,64,wxIMAGE_QUALITY_HIGH);
                    wxSplashScreen* splash = new wxSplashScreen(wxBitmap(img), wxSPLASH_CENTRE_ON_SCREEN|wxSPLASH_TIMEOUT,
                                                                2000, this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxSTAY_ON_TOP);
                    Action(kuID_INTERRUPT);
                    mIsPause = true;
                    GetMenuBar()->SetLabel(kuID_SLIDESHOW,   STRING_MENU_SLIDESHOW_CONTINUE);
                    mSingle->mMenu->SetLabel(kuID_SLIDESHOW, STRING_MENU_SLIDESHOW_CONTINUE);
                    break;
                } else {    // continue
                    wxImage img = wxBitmap(wxICON(ICON_RIGHT)).ConvertToImage().Rescale(64,64,wxIMAGE_QUALITY_HIGH);
                    wxSplashScreen* splash = new wxSplashScreen(wxBitmap(img), wxSPLASH_CENTRE_ON_SCREEN|wxSPLASH_TIMEOUT,
                                                                2000, this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxSTAY_ON_TOP);
                }
                // avoid loss focus
                mSingle->SetFocus();
            } else {    // not fullscreen, start
                if(mIsSlideshow) {
                    Action(kuID_INTERRUPT);
                    break;
                }
                if(!wxGetApp().mIsDoingCmd) {
                    interval = kuApp::GetNumberFromUser(STRING_SLIDESHOW_MESSAGE, STRING_SLIDESHOW_CAPTION, &wxGetApp().mOptions.mSlideshowInterval, 0, 60);
                    if(interval<0)    return false;
                }
                mSingle->SetFullScreen(true);
                // focus on it for traverse keys
                mSingle->SetFocus();
                Action(kuID_RELOAD);
                wxSafeYield();
                // have to hide cursor manually
                mSingle->HideCursor();
            }
            PlaySlideshow(interval);
            break;
        }
        // menu panel
        case kuID_FILESYSTEM:
            ToggleFilesystem();
            break;
        case kuID_ARCHIVE:
            ToggleArchive();
            break;
        case kuID_THUMBNAIL:
            ToggleThumbnail();
            break;
        // menu manage
        case kuID_MKDIR: {
            wxString dirname=wxGetTextFromUser(STRING_MKDIR_MESSAGE,StripCodes(STRING_MENU_MKDIR),
                                               StripCodes(STRING_MENU_MKDIR));
            if(dirname==wxEmptyString)   break;   // user canceled
            wxFileName des(mGeneric->GetPath(),dirname);
            //wxMessageBox(des.GetFullPath());
            if(mGeneric->GetFilePath()!=wxEmptyString)   // des is file
                des.RemoveLastDir();
            //wxMessageBox(des.GetFullPath());
            if(wxDirExists(des.GetFullPath())) {
                SetStatusText(STRING_ERROR_FILE_EXIST + des.GetFullPath());
                wxBell();
                break;
            }
            if(!wxMkdir(des.GetFullPath(),0755)) {
                SetStatusText(STRING_ERROR_FILE_MKDIRFAILED + des.GetFullPath());
                wxBell();
                break;
            }
            // reload filesystem
            ReloadPath(des.GetFullPath(),false);
            break;
        }
        case kuID_RENAME: {
            wxTreeCtrl* tc=mGeneric->GetTreeCtrl();
            if(tc->GetItemParent(tc->GetSelection())==tc->GetRootItem()) {
                SetStatusText(STRING_ERROR_FILE_RENAMEFAILED+mGeneric->GetPath());
                wxBell();
                break;
            }
            wxString newname=wxGetTextFromUser(STRING_RENAME_MESSAGE,StripCodes(STRING_MENU_RENAME),
                                               wxFileName(mGeneric->GetPath()).GetFullName());
            if(newname==wxEmptyString)   break;   // user canceled
            wxBeginBusyCursor();
            wxFileName src(mGeneric->GetPath());
            wxFileName des=src;
            des.SetFullName(newname);
            RenameDirOrFile(src.GetFullPath(),des.GetFullPath());
            wxEndBusyCursor();
            // reload filesystem
            ReloadPath(des.GetFullPath(),false);
            break;
        }
        case kuID_FILE_CUT:
            if(CopyToFileClipboard(mGeneric->GetPath(),true))
                SetStatusText(STRING_INFO_FILE_CUT+mGeneric->GetPath());
            else {
                SetStatusText(STRING_ERROR_BUSY);
                wxBell();
            }
            break;
        case kuID_FILE_COPY:
            if(CopyToFileClipboard(mGeneric->GetPath(),false))
                SetStatusText(STRING_INFO_FILE_COPY+mGeneric->GetPath());
            else {
                SetStatusText(STRING_ERROR_BUSY);
                wxBell();
            }
            break;
        case kuID_FILE_PASTE: {
            //if(!GetSystemClipboard(mFileClipboard))    break;
            if(mFileClipboard.IsEmpty())    break;
            wxBeginBusyCursor();
            wxFileName des(mGeneric->GetPath(),wxEmptyString);
            if(mGeneric->GetFilePath()!=wxEmptyString)   // des is file
                des.RemoveLastDir();
            wxArrayString files=mFileClipboard;
            bool isMove=mFileIsMove;
            PasteDirOrFile(files,des.GetFullPath(),isMove);
            wxEndBusyCursor();
            // reload filesystem
            des.SetFullName(wxFileName(files.Last()).GetFullName());
            ReloadPath(des.GetFullPath(),isMove);
            break;
        }
        case kuID_FILE_DELETE: {
            if(wxWindow::FindFocus()->GetParent() == mMultiple) {
                mMultiple->RemoveThumb(mSingle->GetFilename());
                break;
            }
            wxString path = mGeneric->GetPath();
            wxString neighbor = mGeneric->GetNeighbor();
            bool recycle=false;
            #ifdef __WXMSW__
            if(!wxGetKeyState(WXK_SHIFT))   recycle=true;
            #else
            // confirm
            if(wxMessageBox(STRING_DELETE_MESSAGE+path,StripCodes(STRING_MENU_FILE_DELETE),wxOK|wxCANCEL|wxICON_QUESTION)
               ==wxCANCEL) {
                SetStatusText(STRING_INFO_CANCELED);
                break;
            }
            #endif
            wxArrayString files;
            files.Add(path);
            bool success;
            // start
            wxBeginBusyCursor();
            for(size_t i=0;i<files.GetCount();i++) {
                if(!wxFileExists(files[i]) && wxDirExists(files[i])) {   // src is dir
                    wxArrayString* children = new wxArrayString();
                    wxDir::GetAllFiles(files[i],children);
                    mStatusBar->SetGaugeRange(children->GetCount());
                    delete children;
                    success=DeleteDir(files[i],recycle);
                } else {
                    mStatusBar->SetGaugeRange(1);
                    success=DeleteFile(files[i],recycle);
                }
                if(success) {
                    if(wxDirExists(files[i]) || wxFileExists(files[i])) {
                        SetStatusText(STRING_INFO_CANCELED);
                    } else {
                        SetStatusText(STRING_INFO_FILE_DELETESUCCEED + files[i]);
                        // reload filesystem
                        ReloadPath(wxFileName(neighbor).GetFullPath(),false);
                    }
                }
                else {
                    SetStatusText(STRING_ERROR_FILE_DELETEFAILED + files[i]);
                    wxBell();
                }
            }
            wxEndBusyCursor();
            break;
        }
        case kuID_FILE_MOVETO: {
            if(arg1 == wxEmptyString) {
                wxDirDialog dialog(this,StripCodes(STRING_MENU_FILE_MOVETO),mLastDirPath,wxDD_DEFAULT_STYLE);
                if(dialog.ShowModal()==wxID_CANCEL)   break;
                arg1 = dialog.GetPath();
            }
            wxString neighbor = mGeneric->GetNeighbor();
            if(!CopyToFileClipboard(mGeneric->GetPath(),true)) {
                SetStatusText(STRING_ERROR_BUSY);
                wxBell();
                break;
            }
            wxBeginBusyCursor();
            mLastDirPath = arg1;
            wxArrayString files=mFileClipboard;
            PasteDirOrFile(files,mLastDirPath,mFileIsMove);
            wxEndBusyCursor();
            // reload filesystem
            ReloadPath(wxFileName(neighbor).GetFullPath(),false);
            break;
        }
        case kuID_FILE_COPYTO: {
            if(arg1 == wxEmptyString) {
                wxDirDialog dialog(this,StripCodes(STRING_MENU_FILE_COPYTO),mLastDirPath,wxDD_DEFAULT_STYLE);
                if(dialog.ShowModal()==wxID_CANCEL)   break;
                arg1 = dialog.GetPath();
            }
            if(!CopyToFileClipboard(mGeneric->GetPath(),false)) {
                SetStatusText(STRING_ERROR_BUSY);
                wxBell();
                break;
            }
            wxBeginBusyCursor();
            mLastDirPath = arg1;
            wxArrayString files=mFileClipboard;
            PasteDirOrFile(files,mLastDirPath,mFileIsMove);
            wxEndBusyCursor();
            // reload filesystem
            /*wxFileName path(dialog.GetPath(),wxFileName(files.Last()).GetFullName());
            ReloadPath(path.GetFullPath(),false);*/
            break;
        }
        // menu tools
        case kuID_RESCALE_MANAGER:
            mRescaleMgrDlg->Show(true);
            break;
        case kuID_PICASAWEB_MANAGER: {
            #ifdef ENABLE_PICASAWEBMGR
            mPicasaWebMgrDlg->Show(true);
            #endif
            break;
        }
        #ifdef __WXMSW__
        // menu option
        case kuID_SHELL: {
            wxArrayString choices;
            choices.Add(STRING_SHELL_CHOICE_DRIVE);
            choices.Add(STRING_SHELL_CHOICE_DIRECTORY);
            choices.Add(STRING_SHELL_CHOICE_FILE);
            kuCheckListDialog dialog(this,this,STRING_SHELL_MESSAGE,StripCodes(STRING_MENU_SHELL),choices);
            wxArrayInt selections = wxGetApp().ShellIntegration(wxArrayInt());
            dialog.SetSelections(selections);
            if(dialog.ShowModal()==wxID_OK)   wxGetApp().ShellIntegration(dialog.GetSelections());
            else   wxGetApp().ShellIntegration(selections);
            break;
        }
        case kuID_ASSOCIATION: {
            // get choices from handlers
            wxStringTokenizer tokenzr(kuFiWrapper::GetSupportedExtensions().AfterFirst('|'),wxT(";"));
            wxArrayString choices;
            while(tokenzr.HasMoreTokens()) {
                wxString token=tokenzr.GetNextToken();
                if(choices.Index(token)==wxNOT_FOUND)    // filter duplicated exts
                    choices.Add(token);
            }
            choices.Sort();
            wxArrayInt currents;
            for(size_t i=0; i<choices.GetCount(); i++) {
                if(wxGetApp().GetFileAssociation(choices[i].AfterFirst('*')))
                    currents.Add(i);
            }
            // show select dialog
            kuCheckListDialog dialog(this,this,STRING_ASSOCIATION_MESSAGE,StripCodes(STRING_MENU_ASSOCIATION),choices);
            dialog.SetSelections(currents);
            if(dialog.ShowModal()==wxID_OK) {
                wxGetApp().SetRegSection(!dialog.GetSelections().IsEmpty());
                for(size_t i=0; i<choices.GetCount(); i++) {
                    wxGetApp().SetFileAssociation(choices.Item(i).AfterFirst('*'), dialog.IsChecked(i));
                }
            }
            break;
        }
        case kuID_WALLPAPER: {
            if(mIsArchive) {
                SetStatusText(STRING_ERROR_WALLPAPER_ARCHIVE);
                return false;
            }
            if(mGeneric->GetFilePath()==wxEmptyString){
                SetStatusText(STRING_WARNING_ISNOTFILE);
                return false;
            }
            wxGetApp().CheckEdited(STRING_MENU_DESKTOP_WALLPAPER);
            wxArrayString choices;
            choices.Add(STRING_WALLPAPER_CHOICE_CENTER);
            choices.Add(STRING_WALLPAPER_CHOICE_TILE);
            choices.Add(STRING_WALLPAPER_CHOICE_STRETCH);
            choices.Add(STRING_WALLPAPER_CHOICE_BESTFIT);
            choices.Add(STRING_WALLPAPER_CHOICE_EXTEND);
            int pos = wxGetSingleChoiceIndex(STRING_WALLPAPER_MESSAGE,StripCodes(STRING_MENU_DESKTOP_WALLPAPER),choices,this);
            if(pos==-1) {
                SetStatusText(STRING_INFO_CANCELED);
                return false;
            }
            if(SetAsWallpaper(mGeneric->GetFilePath(),pos))    return false;
            break;
        }
        case kuID_NONEWP: {
            if(wxMessageBox(STRING_NONEWP_MESSAGE, StripCodes(STRING_MENU_DESKTOP_NONEWP), wxYES_NO|wxICON_QUESTION)==wxNO) {
                SetStatusText(STRING_INFO_CANCELED);
                return false;
            }
            SetWallpaperToNone();
            break;
        }
        case kuID_BACKGROUND: {
            wxColour color = wxSystemSettings::GetColour(wxSYS_COLOUR_BACKGROUND);
            color = wxGetColourFromUser(this, color, StripCodes(STRING_MENU_DESKTOP_BACKGROUND));
            if(!color.IsOk()) {
                SetStatusText(STRING_INFO_CANCELED);
                return false;
            }
            int elem = COLOR_DESKTOP;
            COLORREF ref = color.GetPixel();
            SetSysColors(1, &elem, &ref);
            mSingle->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BACKGROUND));
            break;
        }
        #endif
        case kuID_LANGUAGE: {
            wxArrayString choices;
            wxDir dir(wxGetApp().mPath + wxFILE_SEP_PATH + wxT("lang"));
            wxString filename,locale;
			KuTranslationHelper kutrans;
            // add default language
            choices.Add(wxT("English  (en)"));
            // search language files
            dir.Traverse(kutrans);
			wxArrayString &langCanonicalNames = kutrans.GetLangCanonicalNames();
			for( size_t i = 0; i < langCanonicalNames.GetCount(); ++i ) {
                locale=langCanonicalNames[i];
                const wxLanguageInfo* info=wxLocale::FindLanguageInfo(locale);
                if(info)   choices.Add(info->Description + wxT("  (") + info->CanonicalName + wxT(")"));
            }
            // set current language in select dialog
            wxSingleChoiceDialog dialog(this,STRING_LANGUAGE_MESSAGE,StripCodes(STRING_MENU_LANGUAGE),choices);
            dialog.SetIcon(mIconApp);
            int olang=choices.Index(wxString(wxGetApp().mLocale->GetLocale())
                                    + wxT("  (") + wxGetApp().mLocale->GetCanonicalName() + wxT(")"));
            if(olang==wxNOT_FOUND)   olang=0;
            dialog.SetSelection(olang);
            // show select dialog
            if(dialog.ShowModal()==wxID_OK) {
                wxString name=choices.Item(dialog.GetSelection()).AfterLast('(').BeforeLast(')');
                const wxLanguageInfo* info=wxLocale::FindLanguageInfo(name);
                wxGetApp().mOptions.mLanguage = info->Language;
                if(!wxGetApp().SwitchLocale(wxGetApp().mPath))
                    wxMessageBox(STRING_LANGUAGE_FAILED,StripCodes(STRING_MENU_LANGUAGE),wxOK|wxICON_EXCLAMATION);
                else
                    Action(kuID_CONFIG);
            }
            ToggleArchive(mIsArchive);
            break;
        }
        case kuID_FAVORITE: {
            wxDialog dlg(this, wxID_ANY, StripCodes(STRING_MENU_FAVORITE));
            wxBoxSizer* topSizer = new wxBoxSizer(wxVERTICAL);
            topSizer->Add(new wxStaticText(&dlg, wxID_ANY, STRING_FAVORITE_MESSAGE+wxGetApp().mOptions.mFavorite),
                                           1, wxEXPAND|wxALL, 5);
            wxSizer* btnSizer = dlg.CreateButtonSizer(wxOK|wxCANCEL);    // somehow have to CreateButtonSizer before setting kuPathPanel, or path will not show from left
            kuPathPanel* pathPanel = new kuPathPanel(&dlg, true, mGeneric->GetDir());
            topSizer->Add(pathPanel, 0, wxEXPAND|wxALL, 5);
            topSizer->Add(btnSizer, 0, wxEXPAND|wxALIGN_CENTER|wxALL, 5);
            dlg.SetSizer(topSizer);
            dlg.Fit();
            if(dlg.ShowModal()==wxID_OK) {
                wxGetApp().mOptions.mFavorite = pathPanel->GetPath();
                Action(kuID_CONFIG);
            }
            break;
        }
        case kuID_FIXED_AUTOFIT:
            wxGetApp().mOptions.mKeepStyle = SCALE_AUTOFIT;
            break;
        case kuID_FIXED_BESTFIT:
            wxGetApp().mOptions.mKeepStyle = SCALE_BESTFIT;
            break;
        case kuID_FIXED_EXTEND:
            wxGetApp().mOptions.mKeepStyle = SCALE_EXTEND;
            break;
        case kuID_FIXED_SCALE:
            wxGetApp().mOptions.mKeepStyle = SCALE_LASTUSED;
            break;
        case kuID_FIXED_ROTATE:
            wxGetApp().mOptions.mKeepRotate  = !wxGetApp().mOptions.mKeepRotate;
            if(!wxGetApp().mOptions.mKeepRotate)     wxGetApp().mOptions.mRotate=0;
            break;
        case kuID_FIXED_LEFTTOP:
            wxGetApp().mOptions.mKeepLeftTop = !wxGetApp().mOptions.mKeepLeftTop;
            if(!wxGetApp().mOptions.mKeepLeftTop)    wxGetApp().mOptions.mLeftTop=wxPoint(0,0);
            break;
        case kuID_LOAD_COMPLETELY:
            wxGetApp().mOptions.mLoadCompletely = !wxGetApp().mOptions.mLoadCompletely;
            break;
        case kuID_PREFETCH:
            wxGetApp().mOptions.mPrefetch = !wxGetApp().mOptions.mPrefetch;
            break;
        case kuID_RESCALE_BOX:
        case kuID_RESCALE_BICUBIC:
        case kuID_RESCALE_BILINEAR:
        case kuID_RESCALE_BSPLINE:
        case kuID_RESCALE_CATMULLROM:
        case kuID_RESCALE_LANCZOS3:
            wxGetApp().mOptions.mFilter = kuFiWrapper::GetFilterById(action);
            break;
        case kuID_SLIDESHOW_REPEAT:
            wxGetApp().mOptions.mSlideshowRepeat = ! wxGetApp().mOptions.mSlideshowRepeat;
            break;
        case kuID_SLIDESHOW_INTERVAL:
            kuApp::GetNumberFromUser(STRING_SLIDESHOW_MESSAGE, STRING_SLIDESHOW_CAPTION, &wxGetApp().mOptions.mSlideshowInterval, 0, 60);
            break;
        case kuID_SLIDESHOW_CURRENT:
        case kuID_SLIDESHOW_SIBLING:
            wxGetApp().mOptions.mSlideshowDirectory = action;
            break;
        case kuID_SUMMARY_POSITION: {
            kuPositionDialog dlg(this, StripCodes(STRING_MENU_SUMMARY_POSITION), mSingle->GetClientSize(), wxGetApp().mOptions.mSummaryPosition);
            if(dlg.ShowModal() == wxID_OK) {
                wxGetApp().mOptions.mSummaryPosition = dlg.GetPosition();
                if(mIsSummary) {
                    mSingle->ClearSummary();
                    wxClientDC dc(mSingle);
                    mSingle->DrawSummary(dc);
                }
            }
            break;
        }
        case kuID_SUMMARY_COLOR: {
            wxColour color = wxGetApp().mOptions.mSummaryColor;
            color = wxGetColourFromUser(this, color, StripCodes(STRING_MENU_SUMMARY_COLOR));
            if(!color.IsOk()) {
                SetStatusText(STRING_INFO_CANCELED);
                return false;
            }
            wxGetApp().mOptions.mSummaryColor = color;
            if(mIsSummary) {
                wxClientDC dc(mSingle);
                mSingle->DrawSummary(dc);
            }
            break;
        }
        case kuID_MANAGE_DIRSET: {
            kuDirSetDialog dialog(this, StripCodes(STRING_MENU_MANAGE_DIRSET), wxGetApp().mOptions.mHotkeyDirs);
            if(dialog.ShowModal()==wxID_OK) {
                wxGetApp().mOptions.mHotkeyDirs = dialog.GetDirs();
                Action(kuID_CONFIG);
            }
            break;
        }
        case kuID_RESCALEMGR_MAXCON:
            kuApp::GetNumberFromUser(STRING_MANAGER_MAXCON_MESSAGE, StripCodes(STRING_MENU_MANAGER_MAXCON), &wxGetApp().mOptions.mRescaleMgrMaxCon, 1, 10);
            mRescaleMgrDlg->CheckConcurrentRun(wxGetApp().mOptions.mRescaleMgrMaxCon);
            break;
        case kuID_PICASAWEBMGR_MAXCON:
            #ifdef ENABLE_PICASAWEBMGR
            kuApp::GetNumberFromUser(STRING_MANAGER_MAXCON_MESSAGE, StripCodes(STRING_MENU_MANAGER_MAXCON), &wxGetApp().mOptions.mPicasaWebMgrMaxCon, 1, 10);
            mPicasaWebMgrDlg->CheckConcurrentRun(wxGetApp().mOptions.mPicasaWebMgrMaxCon);
            #endif
            break;
        case kuID_CONFIG: {
            bool exported = wxGetApp().mOptions.Export();
            if(exported) {
                if(!wxGetApp().mIsImported) {
                    wxGetApp().mIsImported = true;
                    GetMenuBar()->Enable(kuID_CONFIG, false);
                    wxMessageBox(STRING_CONFIG_SUCCEED, StripCodes(STRING_MENU_CONFIG));
                }
            } else {
                SetStatusText(STRING_CONFIG_FAILED+wxGetApp().mPath);
            }
            break;
        }
        case kuID_LOCATE:
            if(arg1==wxEmptyString)    break;
            if(IsFullScreen()) {    // don't adjust panel when fullscreen
                SetupWindows(false,false);
                // it won't update immediately on linux, have to set size manually
                Maximize(true);
                //SetSize(wxGetDisplaySize());
            } else if(wxFileExists(arg1)) {    // is a file
                if(kuApp::IsArchiveFile(arg1)) {    // is a archive
                    SetupWindows(false,true);
                    Maximize(true);
                } else {    // normal file
                    SetupWindows(false,false);
                }
            } else {   // is a dir
                SetupWindows(true,false);
                Maximize(true);
            }
            if(kuApp::IsImageFile(arg1)) {
                mSingle->ReloadImage(arg1, false);
                wxGetApp().mIdleTasks.Add(wxString::Format(FORMAT_IDLETASK, 1, wxString(CMD_EXPLORE).wc_str(), arg1.wc_str()));
                SetStatusText(STRING_INFO_EXPLORE_FILESYSTEM);
            } else {
                mGeneric->Locate(arg1);
            }
            break;
        case kuID_STATUSBAR:
            if(!IsFullScreen())    break;
            ToggleStatusbar();
            break;
        case kuID_SUMMARY:
            mIsSummary = !mIsSummary;
            GetMenuBar()->Check(kuID_SUMMARY, mIsSummary);
            mSingle->mMenu->Check(kuID_SUMMARY, mIsSummary);
            if(mIsSummary) {
                wxClientDC dc(mSingle);
                mSingle->DrawSummary(dc);
            } else    mSingle->ClearSummary();
            break;
        default:
            return false;
    }
    return true;
}

void kuFrame::ReloadPath(wxString path, bool recreate) {
    // disable thumbnail temporarily
    bool isthumb=mIsThumbnail;
    mIsThumbnail=false;
    // reload filesystem
    if(recreate)   mGeneric->ReCreateTree();
    else {
        mGeneric->SetPath(wxFileName(path).GetPath());
        mGeneric->Reload(false);
    }
    // restore thumbnail
    mIsThumbnail=isthumb;
    if(mIsThumbnail)   mGeneric->ExpandPath(wxFileName(path).GetPath());
    else   mGeneric->ExpandPath(path);
}

void kuFrame::ToggleFilesystem(bool filesystem) {
    if(IsFullScreen())    return;
    mIsFilesystem=filesystem;
    // splitter
    if(mIsFilesystem) {
        mTopSplitter->SplitVertically(mDirSplitter,mViewSplitter,SPLITTER_TOP_WIDTH);
        mDirSplitter->SetSashGravity(1.0);
    }
    else {
        mTopSplitter->Unsplit(mDirSplitter);
    }
    // menuBar/toolBar
    GetMenuBar()->Check(kuID_FILESYSTEM,mIsFilesystem);
    GetToolBar()->ToggleTool(kuID_FILESYSTEM,mIsFilesystem);
}
void kuFrame::ToggleFilesystem() {
    ToggleFilesystem(!mIsFilesystem);
}

void kuFrame::ToggleArchive(bool archive) {
    if(IsFullScreen())    return;
    // avoid mixing type in kuMultipleScrolled
    if(mIsThumbnail)    mMultiple->DestroyChildren();
    mIsArchive=archive;
    // splitter/filter
    if(mIsArchive) {
        ToggleFilesystem(true);
        mDirSplitter->SplitHorizontally(mGeneric,mVirtual,-SPLITTER_DIR_HEIGHT);
        mDirSplitter->SetSashGravity(1.0);
        mGeneric->SwitchFilter(INDEX_FILTER_ARCHIVE);
    }
    else {
        mDirSplitter->Unsplit(mVirtual);
        mGeneric->SwitchFilter(INDEX_FILTER_STANDARD);
    }
    // menuBar/toolBar
    GetMenuBar()->Check(kuID_ARCHIVE,mIsArchive);
    GetToolBar()->ToggleTool(kuID_ARCHIVE,mIsArchive);
}
void kuFrame::ToggleArchive() {
    ToggleArchive(!mIsArchive);
}

void kuFrame::ToggleThumbnail(bool thumbnail) {
    if(IsFullScreen())    return;
    mIsThumbnail=thumbnail;
    // splitter
    if(mIsThumbnail) {
        mViewSplitter->SplitHorizontally(mSingle,mMultiple,-SPLITTER_VIEW_HEIGHT);
        mDirSplitter->SetSashGravity(1.0);
    }
    else {
        mViewSplitter->Unsplit(mMultiple);
    }
    GetMenuBar()->Check(kuID_THUMBNAIL,mIsThumbnail);
    GetToolBar()->ToggleTool(kuID_THUMBNAIL,mIsThumbnail);
}
void kuFrame::ToggleThumbnail() {
    ToggleThumbnail(!mIsThumbnail);
}

void kuFrame::ToggleStatusbar(bool statusbar) {
    mIsStatusbar = statusbar;
    mSingle->mMenu->Check(kuID_STATUSBAR, statusbar);
    if(mIsStatusbar) {
        mStatusBar->Show(true);
        PositionStatusBar();
        mStatusBar->Refresh();
    } else {
        wxRect rect = mStatusBar->GetRect();
        mStatusBar->Show(false);
        RefreshRect(rect);
    }
}
void kuFrame::ToggleStatusbar() {
    ToggleStatusbar(!mIsStatusbar);
}
/*
void kuFrame::PushPanelStatus() {
    mPanelStatus.IsFilesystem = mIsFilesystem;
    mPanelStatus.IsThumbnail = mIsThumbnail;
    mIsFilesystem = mIsThumbnail = false;
}
void kuFrame::PopPanelStatus() {
    mIsFilesystem = mPanelStatus.IsFilesystem;
    mIsThumbnail = mPanelStatus.IsThumbnail;
}
*/

bool kuFrame::PlaySlideshow(long interval) {
    // play
    mIsSlideshow = true;
    mIsPause = false;
    GetMenuBar()->SetLabel(kuID_SLIDESHOW,   STRING_MENU_SLIDESHOW_PAUSE);
    mSingle->mMenu->SetLabel(kuID_SLIDESHOW, STRING_MENU_SLIDESHOW_PAUSE);
    wxGetApp().SetBusy(true);
    wxString current = GetCurrentPath(false,true);
    while (true) {
        // check interrupt in sleep
        unsigned long sleep = 200;
        unsigned long total = (unsigned long)(interval*1000);
        while(total > 0) {
            // check interrupt
            if(wxGetApp().GetInterrupt())  break;
            if(total > sleep) {
                wxMilliSleep(sleep);
                total -= sleep;
            } else {
                wxMilliSleep(total);
                total = 0;
            }
            wxGetApp().Yield();    // call wxGetApp().Yield() instead of wxSafeYield() in this function to accept hotkey
        }
        // check interrupt
        if(wxGetApp().GetInterrupt())  break;
        // traverse
        if(mIsArchive)   Traverse(mVirtual,kuID_NEXT);
        else   Traverse(mGeneric->GetTreeCtrl(),kuID_NEXT);
        wxGetApp().Yield();
        // check repeat and move to next
        if(mIsArchive) {    // don't use mSlideshowDirs
            if(mVirtual->GetFilePath(false,true)==current) {
                if(wxGetApp().mOptions.mSlideshowRepeat)
                    Traverse(mVirtual,kuID_HOME);
                else    break;
            }
            current = mVirtual->GetFilePath(false,true);
        } else {    // use mSlideshowDirs
            if(mGeneric->GetFilePath()==current) {    // it is the last in current dir
                if(mSlideshowDirs.GetCount()>1) {    // has other dirs
                    mSlideshowIndex += 1;
                    if(mSlideshowIndex>=mSlideshowDirs.GetCount()) {
                        if(wxGetApp().mOptions.mSlideshowRepeat)    // repeat in all dirs
                            mSlideshowIndex %= mSlideshowDirs.GetCount();
                        else    break;
                    }
                    mGeneric->Locate(mSlideshowDirs[mSlideshowIndex]);
                } else if(wxGetApp().mOptions.mSlideshowRepeat) {    // if repeat in currentdir
                    Traverse(mGeneric->GetTreeCtrl(),kuID_HOME);
                } else
                    break;
            }
            current = mGeneric->GetFilePath();
        }
        wxGetApp().Yield();
    }
    if(wxGetApp().GetInterrupt()) {
        if(wxGetApp().mQuit)    return false;
        SetStatusText(STRING_WARNING_INTERRUPTED);
        wxGetApp().SetInterrupt(false);
    } else    SetStatusText(wxEmptyString);
    wxGetApp().SetBusy(false);
    if(IsFullScreen() && !mIsPause)    mSingle->SetFullScreen(false);
    mIsSlideshow = false;
    return true;
}

bool kuFrame::SetAsWallpaper(wxString filename, int pos) {
    #ifdef __WXMSW__
    // scale to fit/extend
    double rescale = 0;
    if(pos==WALLPAPER_BESTFIT || pos==WALLPAPER_EXTEND) {
        int xratio=mSingle->GetOrigSize().x*SCALE_BASE/wxGetDisplaySize().x;
        int yratio=mSingle->GetOrigSize().y*SCALE_BASE/wxGetDisplaySize().y;
        int fratio = 0;
        pos==WALLPAPER_BESTFIT ? fratio = wxMax(xratio, yratio) : fratio = wxMin(xratio, yratio);
        rescale = (double)SCALE_BASE/fratio;
        mSingle->Rescale(rescale);
    }
    wxRegKey* key;
    // convert to bmp
    if(filename.AfterLast('.').CmpNoCase(wxT("bmp"))) {    // it is not a bmp file
        mSingle->SaveImage(wxGetHomeDir()+STRING_WALLPAPER_BMPFILE, false);
        wxString newname = wxGetHomeDir()+STRING_WALLPAPER_BMPFILE;
        // desktop section
        key = new wxRegKey(STRING_REG_DESKTOP);
        if(!key->Exists())    key->Create();
        // set converted file name to newname to avoid reverting on theme change
        key->SetValue(STRING_REG_WP_CONVERTED, newname);
        key->SetValue(STRING_REG_WP_ORIGINAL,  newname);
        delete key;
        filename = newname;
    }
    // set style
    wxString tile(wxT("0")), style(wxT("0"));
    switch (pos) {
        case WALLPAPER_TILE:
            tile =  wxT("1");
            style = wxT("0");
            break;
        case WALLPAPER_STRETCH:
            tile =  wxT("0");
            style = wxT("2");
            break;
        case WALLPAPER_CENTER:
        case WALLPAPER_BESTFIT:
        case WALLPAPER_EXTEND:
        default:
            break;
    }
    // desktop section
    key = new wxRegKey(STRING_REG_DESKTOP);
    if(!key->Exists())    key->Create();
    key->SetValue(STRING_REG_WP_MAIN,   filename);
    key->SetValue(STRING_REG_WP_TILE,   tile);
    key->SetValue(STRING_REG_WP_STYLE,  style);
    delete key;
    // ie section
    key = new wxRegKey(STRING_REG_MSIE);
    if(!key->Exists())    key->Create();
    key->SetValue(STRING_REG_WP_BACKUP, filename);
    key->SetValue(STRING_REG_WP_SOURCE, filename);
    key->SetValue(STRING_REG_WP_MAIN,   filename);
    key->SetValue(STRING_REG_WP_TILE,   tile);
    key->SetValue(STRING_REG_WP_STYLE,  style);
    delete key;
    // theme section
    key = new wxRegKey(STRING_REG_THEME);
    if(!key->Exists())    key->Create();
    key->SetValue(STRING_REG_WP_MAIN,   filename);
    delete key;
    // notify to update
    wxExecute(wxGetOSDirectory()+STRING_WALLPAPER_UPDATE_COMMAND);
    if(rescale) {    // it is rescaled for BESTFIT/EXTEND
        //mSingle->ReloadImage(mSingle->GetFilename(),false);
        mSingle->SetScale(wxGetApp().mOptions.mKeepStyle);
    }
    #endif
    return true;
}

bool kuFrame::SetWallpaperToNone() {
    #ifdef __WXMSW__
    wxRegKey* key;
    wxArrayString keys;
    keys.Add(STRING_REG_DESKTOP);
    keys.Add(STRING_REG_MSIE);
    keys.Add(STRING_REG_THEME);
    for(size_t i=0; i<keys.GetCount(); i++) {
        key = new wxRegKey(keys[i]);
        if(!key->Exists())    key->Create();
        key->SetValue(STRING_REG_WP_MAIN, wxEmptyString);
        delete key;
    }
    // notify to update
    wxExecute(wxGetOSDirectory()+STRING_WALLPAPER_UPDATE_COMMAND);
    #endif
    return true;
}

bool kuFrame::GetSystemClipboard(wxArrayString& filenames) {
    filenames.Clear();
    if(wxTheClipboard->Open()) {
        if(wxTheClipboard->IsSupported(wxDF_FILENAME)) {
            wxFileDataObject data;
            wxTheClipboard->GetData(data);
            filenames = data.GetFilenames();
        }
        wxTheClipboard->Close();
    }
    if(filenames.IsEmpty())    return false;
    else    return true;
}
bool kuFrame::SetSystemClipboard(wxArrayString& filenames) {
    if(wxTheClipboard->Open()) {
        wxFileDataObject* data = new wxFileDataObject();
        for(size_t i=0; i<filenames.GetCount(); i++) {
            data->AddFile(filenames[i]);
        }
        wxTheClipboard->SetData(data);
        wxTheClipboard->Close();
    } else    return false;
    return true;
}

bool kuFrame::CopyToFileClipboard(wxString target, bool iscut) {
    mFileClipboard.Clear();
    mFileClipboard.Add(target);
    mFileIsMove=iscut;
    //SetSystemClipboard(mFileClipboard);
    return true;
}

bool kuFrame::PasteDirOrFile(wxArrayString& srcs, wxString desdir, bool ismove) {
    wxFileName des(desdir,wxEmptyString);
    bool isFinal=true;
    bool success=false;
    for(size_t i=0;i<srcs.GetCount();i++) {
        if(!wxFileExists(srcs.Item(i)) && wxDirExists(srcs.Item(i))) {   // src is dir
            // do some check
            wxFileName src(srcs.Item(i));
            wxFileName srcparent=src;
            srcparent.SetFullName(wxEmptyString);
            if(des.SameAs(srcparent)) {
                SetStatusText(STRING_ERROR_FILE_DESISSRC+src.GetFullPath());
                #ifndef __WXMSW__
                wxMessageBox(STRING_ERROR_FILE_DESISSRC+src.GetFullPath());
                return false;
                #endif
            }
            wxString finaldes=des.GetFullPath();
            if(DirIsParent(srcs.Item(i),des.GetFullPath())) {
                if(ismove) {
                    SetStatusText(STRING_ERROR_FILE_DESINSRC+des.GetFullPath());
                    wxMessageBox(STRING_ERROR_FILE_DESINSRC+des.GetFullPath());
                    return false;
                } else {
                    //wxMessageBox(wxT("finaldes:")+finaldes);
                    des=src;
                    des.SetFullName(wxT("tmp"));
                    while(wxDirExists(des.GetFullPath()))
                        des.SetFullName(des.GetFullName()+wxT(".tmp"));
                    //wxMessageBox(wxT("des:")+des.GetFullPath());
                    wxMkdir(des.GetFullPath());
                    //wxMessageBox(wxT("1: ")+srcs.Item(i)+wxT("->")+des.GetFullPath());
                    isFinal=false;
                }
            }
            wxArrayString* children = new wxArrayString();
            wxDir::GetAllFiles(srcs.Item(i),children);
            mStatusBar->SetGaugeRange(children->GetCount());
            delete children;
            #ifdef __WXMSW__
            success=MswCopyDirOrFile(srcs.Item(i),des.GetFullPath(),ismove);
            #else
            success=CopyDir(srcs.Item(i),des.GetFullPath(),ismove);
            #endif
            if(!isFinal) {
                des=wxFileName(des.GetFullPath(),src.GetFullName());
                //wxMessageBox(wxT("2: ")+des.GetFullPath()+wxT("->")+finaldes);
                #ifdef __WXMSW__
                success=MswCopyDirOrFile(des.GetFullPath(),finaldes,true);
                #else
                success=CopyDir(des.GetFullPath(),finaldes,true);
                #endif
                wxRmdir(des.GetPath());
            }
            des.SetFullName(srcs.Item(i));
        } else {   // src is file
            des.SetFullName(wxFileName(srcs.Item(i)).GetFullName());
            #ifdef __WXMSW__
            success=MswCopyDirOrFile(srcs.Item(i),des.GetFullPath(),ismove);
            #else
            if(des.SameAs(wxFileName(srcs.Item(i)))) {
                SetStatusText(STRING_ERROR_FILE_DESISSRC+des.GetFullPath());
                //wxMessageBox(STRING_ERROR_FILE_DESISSRC+des.GetFullPath());
                wxString init=StripCodes(STRING_MENU_FILE_COPY)+ wxT(" ");
                des.SetFullName(init + wxT("-")
                                + wxFileName(srcs.Item(i)).GetFullName());
                while(wxFileExists(des.GetFullPath()))
                    des.SetFullName(init + des.GetFullName());
            }
            mStatusBar->SetGaugeRange(1);
            success=CopyFile(srcs.Item(i),des.GetFullPath(),ismove);
            #endif
        }
        if(success) {
            if(ismove)   SetStatusText(STRING_INFO_FILE_MOVESUCCEED+des.GetFullPath());
            else   SetStatusText(STRING_INFO_FILE_COPYSUCCEED+des.GetFullPath());
        } else {
            if(ismove)   SetStatusText(STRING_ERROR_FILE_MOVEFAILED+des.GetFullPath());
            else   SetStatusText(STRING_ERROR_FILE_COPYFAILED+des.GetFullPath());
            wxBell();
        }
    }
    return true;
}

bool kuFrame::DeleteDir(wxString dirname, bool recycle) {
    #ifdef __WXMSW__
    if(!MswDeleteDirOrFile(dirname,recycle))   return false;
    #else
    wxDir* dir = new wxDir(dirname);
    wxString filename;
    bool cont=dir->GetFirst(&filename);
    wxFileName filepath(dirname,filename);   // the file or dir
    while(cont) {
        if(!wxFileExists(filepath.GetFullPath()) && wxDirExists(filepath.GetFullPath())) {   // is dir
            if(!DeleteDir(filepath.GetFullPath(),recycle))
                return false;
        }
        else {
            if(!DeleteFile(filepath.GetFullPath(),recycle))
                return false;
        }
        cont=dir->GetNext(&filename);
        filepath.Assign(dirname,filename);
    }
    delete dir;
    if(!wxRmdir(dirname)) {
        if(wxMessageBox(STRING_ERROR_FILE_RMDIRFAILED + dirname + STRING_CONTINUE_MESSAGE,
                        STRING_CONTINUE_CAPTION, wxYES_NO|wxICON_QUESTION)
           ==wxYES)   return true;
        else   return false;
    }
    #endif
    return true;
}

bool kuFrame::DeleteFile(wxString filename, bool recycle) {
    bool rtn;
    #ifdef __WXMSW__
    rtn=MswDeleteDirOrFile(filename,recycle);
    #else
    SetStatusText(STRING_INFO_FILE_DELETING+wxFileName(filename).GetFullName());
    if(!wxFileExists(filename)) {
        if(wxMessageBox(STRING_ERROR_FILE_NOTEXIST + filename + STRING_CONTINUE_MESSAGE,
                        STRING_CONTINUE_CAPTION, wxYES_NO|wxICON_QUESTION)
           ==wxYES)   rtn=true;
        else   rtn=false;
    } else if(!wxRemoveFile(filename)) {
        if(wxMessageBox(STRING_ERROR_FILE_DELETEFAILED + filename + STRING_CONTINUE_MESSAGE,
                        STRING_CONTINUE_CAPTION, wxYES_NO|wxICON_QUESTION)
           ==wxYES)   rtn=true;
        else   rtn=false;
    } else   rtn=true;
    mStatusBar->IncrGaugeValue();
    wxSafeYield();
    #endif
    return rtn;
}

#ifdef __WXMSW__
bool kuFrame::MswDeleteDirOrFile(wxString target, bool recycle) {
    int len=target.Length();
    wxChar* buffer = new wxChar[len+2];
    wcsncpy(buffer,target.wc_str(),len);
    buffer[len]=buffer[len+1]=0;
    //wxMessageBox(buffer);
    SHFILEOPSTRUCT op;
    memset(&op,0,sizeof(op));
    op.hwnd=(HWND)GetHWND();
    op.wFunc=FO_DELETE;
    op.pFrom=buffer;
    if(recycle)   op.fFlags=FOF_ALLOWUNDO;
    int ret = SHFileOperation(&op);
    delete buffer;
    if(ret!=0)   return false;
    return true;
}

bool kuFrame::MswCopyDirOrFile(wxString src, wxString des, bool ismove) {
    int len=src.Length();
    wxChar* srcbuf = new wxChar[len+2];
    wcsncpy(srcbuf,src.wc_str(),len);
    srcbuf[len]=srcbuf[len+1]=0;
    len=des.Length();
    wxChar* desbuf = new wxChar[len+2];
    wcsncpy(desbuf,des.wc_str(),len);
    desbuf[len]=desbuf[len+1]=0;
    //wxMessageBox(buffer);
    SHFILEOPSTRUCT op;
    memset(&op,0,sizeof(op));
    op.hwnd=(HWND)GetHWND();
    if(ismove)   op.wFunc=FO_MOVE;
    else   op.wFunc=FO_COPY;
    op.pFrom=srcbuf;
    op.pTo=desbuf;
    op.fFlags=FOF_ALLOWUNDO|FOF_RENAMEONCOLLISION;
    int ret = SHFileOperation(&op);
    delete srcbuf;
    delete desbuf;
    if(ret!=0)   return false;
    return true;
}

#else
bool kuFrame::CopyDir(wxString srcdir, wxString desdir, bool ismove) {
    //wxMessageBox(wxT("CopyDir: ")+srcdir+wxT("->")+desdir);
    wxDir* dir = new wxDir(srcdir);
    wxString filename;
    bool cont=dir->GetFirst(&filename);
    wxFileName filepath(srcdir,filename);   // the file or dir
    wxFileName newdes(desdir,wxFileName(srcdir).GetFullName());   // new des dir
    // check des dir exist
    if(!wxDirExists(newdes.GetFullPath())) {
        if(!wxMkdir(newdes.GetFullPath(),0755)) {
            if(wxMessageBox(STRING_ERROR_FILE_MKDIRFAILED + newdes.GetFullPath() + STRING_CONTINUE_MESSAGE,
                            STRING_CONTINUE_CAPTION,wxYES_NO|wxICON_QUESTION)
               ==wxYES)   return true;
            else   return false;
        }
    }
    while(cont) {
        if(!wxFileExists(filepath.GetFullPath()) && wxDirExists(filepath.GetFullPath())) {   // is dir
            if(!CopyDir(filepath.GetFullPath(),newdes.GetFullPath(),ismove))
                return false;
        }
        else {
            if(!CopyFile(filepath.GetFullPath(),wxFileName(newdes.GetFullPath(),filename).GetFullPath(),ismove))
                return false;
        }
        cont=dir->GetNext(&filename);
        filepath.Assign(srcdir,filename);
    }
    delete dir;
    if(ismove) {
        if(!wxRmdir(srcdir)) {
            if(wxMessageBox(STRING_ERROR_FILE_RMDIRFAILED + srcdir + STRING_CONTINUE_MESSAGE,
                            STRING_CONTINUE_CAPTION, wxYES_NO|wxICON_QUESTION)
               ==wxYES)   return true;
            else   return false;
        }
    }
    return true;
}

bool kuFrame::CopyFile(wxString srcfile, wxString desfile, bool ismove) {
    //wxMessageBox(wxT("CopyFile: ")+srcfile+wxT("->")+desfile);
    bool rtn;
    if(!wxFileExists(srcfile)) {   // check source exist
        if(wxMessageBox(STRING_ERROR_FILE_NOTEXIST + srcfile + STRING_CONTINUE_MESSAGE,
                        STRING_CONTINUE_CAPTION, wxYES_NO|wxICON_QUESTION)
           ==wxYES)   rtn=true;
        else   rtn=false;
    } else if(wxFileExists(desfile)) {   // check destination notexist
        if(wxMessageBox(STRING_ERROR_FILE_EXIST + desfile + STRING_CONTINUE_MESSAGE,
                        STRING_CONTINUE_CAPTION, wxYES_NO|wxICON_QUESTION)
           ==wxYES)   rtn=true;
        else   rtn=false;
    } else if(ismove) {   // move
        SetStatusText(STRING_INFO_FILE_MOVING+wxFileName(srcfile).GetFullName());
        if(!wxRenameFile(srcfile,desfile)) {
            if(wxMessageBox(STRING_ERROR_FILE_MOVEFAILED + srcfile + STRING_CONTINUE_MESSAGE,
                            STRING_CONTINUE_CAPTION, wxYES_NO|wxICON_QUESTION)
               ==wxYES)   rtn=true;
            else   rtn=false;
        } else rtn=true;
    } else {   // copy
        SetStatusText(STRING_INFO_FILE_COPYING+wxFileName(srcfile).GetFullName());
        if(!wxCopyFile(srcfile,desfile)) {
            if(wxMessageBox(STRING_ERROR_FILE_COPYFAILED + srcfile + STRING_CONTINUE_MESSAGE,
                            STRING_CONTINUE_CAPTION, wxYES_NO|wxICON_QUESTION)
               ==wxYES)   rtn=true;
            else   rtn=false;
        } else   rtn=true;
    }
    mStatusBar->IncrGaugeValue();
    wxSafeYield();
    return rtn;
}
#endif

bool kuFrame::RenameDirOrFile(wxString src, wxString des) {
    //wxMessageBox(wxT("Rename: ")+src+wxT("->")+des);
    /*
    if(!wxFileExists(src) && wxDirExists(src)) {   // src is dir
        if(wxDirExists(des)) {
            SetStatusText(STRING_ERROR_FILE_EXIST + des);
            return false;
        }
        else if(wxMkdir(des,0755)) {
            wxDir* dir = new wxDir(src);
            wxString filename;
            bool cont=dir->GetFirst(&filename);
            wxFileName filepath(src,filename);
            while(cont) {
                if(!wxFileExists(filepath.GetFullPath()) && wxDirExists(filepath.GetFullPath())) {   // filepath is dir
                    //wxMessageBox(wxT("Dir: ")+filepath.GetFullPath()+wxT("->")+des);
                    if(!CopyDir(filepath.GetFullPath(),des, true)) {
                        if(wxMessageBox(STRING_ERROR_FILE_MOVEFAILED + filepath.GetFullPath() + STRING_CONTINUE_MESSAGE,
                                        STRING_CONTINUE_CAPTION, wxYES_NO|wxICON_QUESTION)
                           ==wxYES)   continue;
                        else   break;
                    }
                }
                else {
                    //wxMessageBox(wxT("File: ")+filepath.GetFullPath()+wxT("->")+wxFileName(des,filename).GetFullPath());
                    if(!wxRenameFile(filepath.GetFullPath(),wxFileName(des,filename).GetFullPath())) {
                        if(wxMessageBox(STRING_ERROR_FILE_MOVEFAILED + filepath.GetFullPath() + STRING_CONTINUE_MESSAGE,
                                        STRING_CONTINUE_CAPTION, wxYES_NO|wxICON_QUESTION)
                           ==wxYES)   continue;
                        else   break;
                    }
                }
                cont=dir->GetNext(&filename);
                filepath.Assign(src,filename);
            }
            delete dir;
            if(!wxRmdir(src)) {
                SetStatusText(STRING_ERROR_FILE_RMDIRFAILED + src);
                return false;
            }
        }
        else {
            SetStatusText(STRING_ERROR_FILE_MKDIRFAILED + des);
            return false;
        }
    } else */
    if(wxRenameFile(src,des)) {
        SetStatusText(STRING_INFO_FILE_RENAMESUCCEED + des);
    } else {
        SetStatusText(STRING_ERROR_FILE_MOVEFAILED + des);
        wxBell();
        return false;
    }
    return true;
}

bool kuFrame::DirIsParent(wxString parent, wxString child) {
    wxFileName p(parent,wxEmptyString);
    wxFileName c(child,wxEmptyString);
    wxFileName tc;
    while(tc!=c) {
        //wxMessageBox(c.GetFullPath()+wxT("<->")+p.GetFullPath());
        if(c.SameAs(p))   return true;
        tc=c;
        c.RemoveLastDir();
    }
    return false;
}

bool kuFrame::Traverse(wxTreeCtrl* tree, int direct) {
    wxTreeItemId oid=tree->GetSelection();
    wxTreeItemId nid=oid;
    wxTreeItemId tmpid=oid;
    // traverse
    switch (direct) {
        case kuID_PREV:
            nid=tree->GetPrevSibling(oid);
            break;
        case kuID_NEXT:
            nid=tree->GetNextSibling(oid);
            break;
        case kuID_HOME:
            while(tmpid.IsOk() && !IsDir(tree, tmpid)) {
                nid=tmpid;
                tmpid=tree->GetPrevSibling(nid);
            }
            break;
        case kuID_END:
            while(tmpid.IsOk() && !IsDir(tree, tmpid)) {
                nid=tmpid;
                tmpid=tree->GetNextSibling(nid);
            }
            break;
        case kuID_UP:
            nid=tree->GetPrevSibling(oid);
            if(!nid.IsOk()) {   // it is first child
                nid=tree->GetItemParent(oid);
                // if traverse to root, skip
                if(nid==tree->GetRootItem())   nid=oid;
            }
            else {   // if expanded, traverse to last child
                while(tree->IsExpanded(nid))
                    nid=tree->GetLastChild(nid);
            }
            break;
        case kuID_DOWN:
            if(tree->IsExpanded(nid))   nid=tree->GetFirstChild(nid,*(new wxTreeItemIdValue()));
            else   nid=tree->GetNextSibling(oid);
            if(!nid.IsOk()) {   // it is last child
                nid=oid;
                // unless it is last child of root
                while(tree->GetItemParent(tmpid)!=tree->GetRootItem()) {
                    nid=tmpid=tree->GetItemParent(nid);
                    tmpid=tree->GetNextSibling(tmpid);
                    if(tmpid.IsOk()) {
                        nid=tmpid;
                        break;
                    } else {
                        if(tree->GetItemParent(nid)==tree->GetRootItem()) {    // no next sibling and cannot up
                            nid=tmpid;
                            break;
                        }
                        tmpid=nid;
                    }
                }
            }
            break;
        case kuID_LEFT:
            if(tree->IsExpanded(oid)) {
                tree->Collapse(oid);
            } else   nid=tree->GetItemParent(oid);
            break;
        case kuID_RIGHT:
            if(!tree->IsExpanded(oid)) {
                tree->Expand(oid);
            } else   nid=tree->GetFirstChild(oid,*(new wxTreeItemIdValue()));
            break;
        default:
            nid=wxTreeItemId();
    }
    // select
    switch (direct) {
        case kuID_PREV:
        case kuID_NEXT:
        case kuID_HOME:
        case kuID_END:
            if(nid.IsOk() && !IsDir(tree, nid)) {
                tree->SelectItem(nid);
                return true;
            } else   return false;
        case kuID_UP:
        case kuID_DOWN:
            if(nid.IsOk()) {
                tree->SelectItem(nid);
                return true;
            } else   return false;
        case kuID_LEFT:
        case kuID_RIGHT:
            if(nid.IsOk() && nid!=tree->GetRootItem()) {
                tree->SelectItem(nid);
                return true;
            }
            return false;
        default:
            return false;
    }
}

bool kuFrame::IsDir(wxTreeCtrl* tree, wxTreeItemId id) {
    wxDirItemData* data = (wxDirItemData*) tree->GetItemData(id);
    return data->m_isDir;
}

void kuFrame::HistoryTracing(wxString path) {
    if(mIsHistory || path==wxEmptyString)    return;
    if(!mHistoryMainFiles.IsEmpty() && mHistoryMainIndex < mHistoryMainFiles.GetCount()-1) {
        // move the tail of main to branch
        mHistoryBranchFiles.Clear();
        for(size_t i=mHistoryMainFiles.GetCount()-1; i>mHistoryMainIndex; i--) {
            mHistoryBranchFiles.Insert(mHistoryMainFiles[i], 0);
            mHistoryMainFiles.RemoveAt(i);
        }
        mHistoryBranchIndex = mHistoryBranchFiles.GetCount() - 1;
        mHistorySwapIndex = mHistoryMainIndex;
        mHistoryMainIndex = mHistoryMainFiles.GetCount() - 1;
    }
    mHistoryMainFiles.Add(path);
    mHistoryMainIndex++;
}

wxString kuFrame::GetCurrentPath(bool isurl, bool fileonly) {
    if(wxGetApp().mIsExploring) {
        return mSingle->GetFilename();
    } else if(mIsArchive) {
        return mVirtual->GetFilePath(isurl,fileonly);
    } else {
        if(fileonly)    return mGeneric->GetFilePath();
        else    return mGeneric->GetPath();
    }
}

wxString kuFrame::StripCodes(const wxChar* cptr) {
    wxString str=wxString(cptr);
    wxString rtn=wxMenuItem::GetLabelText(str);
    wxString codes=rtn.AfterLast('(').BeforeLast(')');
    if(codes.Length()==1 && codes.GetChar(0)==str.GetChar(str.First('&')+1))
        return rtn.BeforeLast('(');
    return rtn;
}

// -------- kuTaskBarIcon --------
kuTaskBarIcon::kuTaskBarIcon(kuFrame* frame)
    :wxTaskBarIcon() {
    mFrame=frame;
}

wxMenu* kuTaskBarIcon::CreatePopupMenu() {
    wxMenu* popMenu = new wxMenu(wxID_ANY);
    popMenu->Append(kuID_RESTORE,STRING_MENU_RESTORE);
    popMenu->Append(wxID_ABOUT,STRING_MENU_ABOUT);
    popMenu->Append(wxID_EXIT,STRING_MENU_EXIT);
    return popMenu;
}

void kuTaskBarIcon::OnLeftDclick(wxTaskBarIconEvent& event) {
    mFrame->Show(true);
    RemoveIcon();
}

void kuTaskBarIcon::OnRestore(wxCommandEvent& event) {
    mFrame->Show(true);
    RemoveIcon();
}

// -------- kuStatusBar --------
kuStatusBar::kuStatusBar(kuFrame* frame)
    :wxStatusBar(frame,wxID_ANY) {
    mFrame=frame;
    wxClientDC dc(this);
    wxCoord s1,s2,s4,s5,s6;
    dc.GetTextExtent(wxT(" 99999KB "), &s1, NULL);
    dc.GetTextExtent(wxT(" 99.999s "), &s2, NULL);
    dc.GetTextExtent(wxT(" 99999x99999x999 "), &s4, NULL);
    dc.GetTextExtent(wxT(" 9999% "), &s5, NULL);
    dc.GetTextExtent(wxT(" 99999,99999 / 99999,99999 "), &s6, NULL);
    // 0:filename, 1:filesize, 2:gauge, 3:bmpsize, 4:scale, 5:motion
    int widths[7]={-1, s1, s2, 100, s4, s5,s6};
    SetFieldsCount(7, widths);
    mGauge = new wxGauge(this,wxID_ANY,100,wxDefaultPosition,wxDefaultSize,wxGA_HORIZONTAL|wxGA_SMOOTH);
}

void kuStatusBar::SetGaugeRange(int range) {
    mGauge->SetRange(range);
    mGauge->SetValue(0);
}

void kuStatusBar::IncrGaugeValue() {
    mGauge->SetValue(mGauge->GetValue()+1);
    if(mGauge->GetValue()==mGauge->GetRange())   mGauge->SetValue(0);
}

void kuStatusBar::OnSize(wxSizeEvent& event) {
    wxRect rect;
    GetFieldRect(STATUS_FIELD_GAUGE,rect);
    mGauge->SetSize(rect);
    event.Skip();
}

// -------- kuPathPanel --------
kuPathPanel::kuPathPanel(wxWindow* parent, bool isdir, wxString defpath)
    :wxPanel(parent, wxID_ANY) {
    mIsDir = isdir;
    wxBoxSizer* topSizer = new wxBoxSizer(wxHORIZONTAL);
    mTextCtrl = new wxTextCtrl(this, wxID_ANY, defpath);
    topSizer->Add(mTextCtrl, 1, wxEXPAND);
    mButton = new wxBitmapButton(this, wxID_OPEN, wxArtProvider::GetBitmap(wxART_FILE_OPEN));
    topSizer->Add(mButton, 0, wxEXPAND|wxLEFT, 5);
    SetSizer(topSizer);
    Fit();
}

void kuPathPanel::OnButton(wxCommandEvent& event) {
    wxString path = wxEmptyString;
    wxString def = mTextCtrl->GetValue();
    if(mIsDir) {
        path = wxDirSelector(wxEmptyString, def);
    } else {
        path = wxFileSelector(wxEmptyString, def);
    }
    if(!path.IsEmpty()) {
        mTextCtrl->ChangeValue(path);
    }
}

wxString kuPathPanel::GetPath() {
    return mTextCtrl->GetValue();
}

void kuPathPanel::SetPath(wxString& path) {
    mTextCtrl->SetValue(path);
}

void kuPathPanel::ChangePath(wxString& path) {
    mTextCtrl->ChangeValue(path);
}

// -------- kuScrolledDropTarget --------
kuScrolledDropTarget::kuScrolledDropTarget(kuFrame* frame) {
    mFrame = frame;
}

bool kuScrolledDropTarget::OnDropFiles(wxCoord x, wxCoord y, const wxArrayString& items) {
    if(!wxGetApp().mQuit && wxGetApp().mFrame) {
        wxArrayString* data = new wxArrayString(items);
        wxCommandEvent event(wxEVT_DROPPED_EVENT);
        event.SetClientData(data);
		wxEvtHandler *fmevthdl = wxGetApp().mFrame->GetEventHandler();
		fmevthdl->AddPendingEvent(event);
    }
    return true;
}

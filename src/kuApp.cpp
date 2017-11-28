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

#include <wx/cmdline.h>
#include "ku.h"

// -------- implement --------

IMPLEMENT_APP(kuApp)

static const wxCmdLineEntryDesc g_cmdLineDesc[] =
{
	{
		wxCMD_LINE_SWITCH, "h", "help", "Displays help on the command line parameters",
		wxCMD_LINE_VAL_NONE, wxCMD_LINE_OPTION_HELP
	},
	{ wxCMD_LINE_OPTION, "v", "state", "Start with show | hide | full | icon, normally, hide, fullscreen, iconize" },
	{ wxCMD_LINE_OPTION, "l", "locate", "" },
	{ wxCMD_LINE_OPTION, "s", "slideshow", "" },
	{ wxCMD_LINE_OPTION, "r", "reg", "" },
	{ wxCMD_LINE_OPTION, "u", "unreg", "" },
	{ wxCMD_LINE_OPTION, "c", "rescale", "" },
	{ wxCMD_LINE_PARAM, nullptr, nullptr, "File(s) to be opened", wxCMD_LINE_VAL_STRING,  wxCMD_LINE_PARAM_OPTIONAL | wxCMD_LINE_PARAM_MULTIPLE },

	{ wxCMD_LINE_NONE }
};

wxString & GetExecutablePath()
{
	static bool found = false;
	static wxString path;

	if (!found) {
#ifdef __WXMSW__

		wxChar buf[512];
		*buf = wxT('\0');
		GetModuleFileName(nullptr, buf, 511);
		path = buf;

#elif defined(__WXMAC__)

		ProcessInfoRec processinfo;
		ProcessSerialNumber procno;
		FSSpec fsSpec;

		procno.highLongOfPSN = nullptr;
		procno.lowLongOfPSN = kCurrentProcess;
		processinfo.processInfoLength = sizeof(ProcessInfoRec);
		processinfo.processName = nullptr;
		processinfo.processAppSpec = &fsSpec;

		GetProcessInformation(&procno, &processinfo);
		path = wxMacFSSpec2MacFilename(&fsSpec);
#else
		wxString argv0 = wxTheApp->argv[0];

		if (wxIsAbsolutePath(argv0)) {
			path = argv0;
		}
		else {
			wxPathList pathlist;
			pathlist.AddEnvList(wxT("PATH"));
			path = pathlist.FindAbsoluteValidPath(argv0);
		}

		wxFileName filename(path);
		filename.Normalize();
		path = filename.GetFullPath();
#endif
		found = true;
	}

	return path;
}

void kuApp::OnInitCmdLine( wxCmdLineParser& cmdParser )
{
	cmdParser.SetDesc( g_cmdLineDesc );
	// must refuse '/' as parameter starter or cannot use "/path" style paths
	cmdParser.SetSwitchChars( wxT( "-" ) );
}

bool kuApp::OnCmdLineParsed( wxCmdLineParser& cmdParser )
{
#if 0
	m_SilentMode = cmdParser.Found( wxT( "s" ) );
	m_Exit = cmdParser.Found( wxT( "x" ) );
	m_ForceEdit  = cmdParser.Found( wxT( "f" ) );
	cmdParser.Found( wxT( "m" ), &m_MadPythonScript );
	cmdParser.Found( wxT( "d" ), &m_Delimiter );
	wxASSERT(m_Delimiter.Length() > 0);
	g_Delimiter = m_Delimiter[0];

	/*if( !m_MadPythonScript.IsEmpty() )
	{
		filename = m_MadPythonScript;

		if( ( filename.GetPath() ).IsEmpty() )
			m_MadPythonScript = g_MadEditHomeDir + wxT( "scripts" ) + wxFILE_SEP_PATH +  m_MadPythonScript;
	}*/

	// parse commandline to filenames, every file is with a trailing char g_MadConfigSeparator, ex: filename1|filename2|
	m_FileNames.Empty();
	// to get at your unnamed parameters use GetParam
	int flags = wxDIR_FILES | wxDIR_HIDDEN;
	wxString fname;
#ifdef __WXMSW__
	wxString escape(wxT("\\\\")), tfname, backslash(wxT("\\"));
#endif

	for( size_t i = 0; i < cmdParser.GetParamCount(); i++ )
	{
		fname = cmdParser.GetParam( i );
#ifdef __WXMSW__
		if( fname.StartsWith(escape, &tfname) )
		{
			tfname.Replace(escape, backslash);
			fname = escape + tfname;
		}
		else
		{
			fname.Replace(escape, backslash);
		}
#endif
		wxFileName filename(fname);

		filename.MakeAbsolute();
		fname = filename.GetFullName();

		if( cmdParser.Found( wxT( "w" ) ) )
		{
			//WildCard
			if( cmdParser.Found( wxT( "r" ) ) ) flags |= wxDIR_DIRS;

			wxArrayString files;
			size_t nums = wxDir::GetAllFiles( filename.GetPath(), &files, fname, flags );

			for( size_t j = 0; j < nums; ++j )
			{
				m_FileNames.Add( files[j] );
			}
		}
		else
		{
			// Support for name*linenum
			m_FileNames.Add( filename.GetFullPath() );
		}
	}

	// and other command line parameters
	// then do what you need with them.
#endif
	return true;
}

// -------- kuApp --------
bool kuApp::OnInit() {
    mLocale = NULL;
    mFrame = NULL;
    mInit = kuID_INIT_SHOW;

	if (!wxApp::OnInit())
		return false;
    // initialize FreeImage
    kuFiWrapper::Initialize();

    // init archive handlers
    wxFileSystem::AddHandler(new wxArchiveFSHandler);
    wxFileSystem::AddHandler(new wxFilterFSHandler);
    // init language
    wxFileName exec(GetExecutablePath());
    if(!exec.IsAbsolute())    // run from other dir
        exec.MakeAbsolute();
    //wxMessageBox(exec.GetFullPath());
    mPath=exec.GetPath();

    // parse argv
    if(!ParseArguments(argc, argv)) {
        PrintCmdHelp();
        return false;
    }
    mWaitLoader = false;
    mQuit = false;
    mIsDoingCmd = false;
    mAutoZoom = true;
    mIdleTasks.Clear();

    mIsInterrupted = false;
    mIsBusy = false;
    mIsEdited = false;
    mIsSaveAs = false;

    // set options
    SetOptions();

    SwitchLocale(mPath);

    // prepare for registry
    mAssociationExecute = wxString::Format(FORMAT_ASSOCIATION, exec.GetFullPath().wc_str());
    mShellExecute = wxString::Format(FORMAT_SHELL, exec.GetFullPath().wc_str());
    // set reg default
    SetRegDefault();

    // create mFrame
    mFrame = new kuFrame();
    mFrame->SetSize(1000,618);
    mFrame->Centre();
    mFrame->SetDefaults();
    mFrame->SetupExternalOpenMenu();

    // window state and init cmd
    switch(mInit) {
        case kuID_INIT_SHOW:
            mFrame->SetupWindows(true,false);
            mFrame->Show(true);
            break;
        case kuID_INIT_HIDE:
            mFrame->Show(false);
            break;
        case kuID_INIT_FULL:
            mFrame->mSingle->SetFullScreen(true);
            break;
        case kuID_INIT_ICON:
            mFrame->Action(kuID_MINIMIZE);
            break;
        default:
            mFrame->SetupWindows(true,false);
            mFrame->Show(true);
            break;
    }
    mFrame->mSingle->SetFocus();

    // run loader thread
    mLoader = new kuLoadThread(mFrame);
    if(mLoader->Create() != wxTHREAD_NO_ERROR)    wxMessageBox(wxT("failed to create thread"));
    else {
        mLoader->SetPriority(30);
        mLoader->Run();
    }

    wxSafeYield();

    // run action cmd
    for(size_t i=0; i<mCmds.GetCount(); i++) {
        DoActionCmd(mCmds[i]);
        // test if quit when doing command
        if(mQuit)    return false;
    }

    return true;
}

int kuApp::OnExit() {
    if(mIsImported)    mOptions.Export();
    ClearThread();
    // finalize FreeImage
    kuFiWrapper::Finalize();
    return wxApp::OnExit();
}

#if (wxUSE_ON_FATAL_EXCEPTION == 1) && (wxUSE_STACKWALKER == 1)
#include <wx/longlong.h>
void kuStackWalker::OnStackFrame(const wxStackFrame & frame)
{
	if (m_DumpFile)
	{
		wxULongLong address((size_t)frame.GetAddress());
#if defined(__x86_64__) || defined(__LP64__) || defined(_WIN64)
		wxString fmt(wxT("[%02u]:[%08X%08X] %s(%i)\t%s%s\n"));
#else
		wxString fmt(wxT("[%02u]:[%08X] %s(%i)\t%s%s\n"));
#endif
		wxString paramInfo(wxT("("));
#if defined(_WIN32)
		wxString type, name, value;
		size_t count = frame.GetParamCount(), i = 0;

		while (i < count)
		{
			frame.GetParam(i, &type, &name, &value);
			paramInfo += type + wxT(" ") + name + wxT(" = ") + value;

			if (++i < count) paramInfo += wxT(", ");
		}

#endif
		paramInfo += wxT(")");
		m_DumpFile->Write(wxString::Format(fmt,
			(unsigned)frame.GetLevel(),
#if defined(__x86_64__) || defined(__LP64__) || defined(_WIN64)
			address.GetHi(),
#endif
			address.GetLo(),
			frame.GetFileName().c_str(),
			(unsigned)frame.GetLine(),
			frame.GetName().c_str(),
			paramInfo.c_str())
		);
	}
}

void kuApp::OnFatalException()
{
	wxString name = GetExecutablePath() + wxString::Format(
		wxT("%s_%s_%lu.dmp"),
		wxTheApp ? (const wxChar*)wxTheApp->GetAppDisplayName().c_str()
		: wxT("wxwindows"),
		wxDateTime::Now().Format(wxT("%Y%m%dT%H%M%S")).c_str(),
#if defined(__WXMSW__)
		::GetCurrentProcessId()
#else
		(unsigned)getpid()
#endif
	);
	wxFile dmpFile(name.c_str(), wxFile::write);

	if (dmpFile.IsOpened())
	{
		m_StackWalker.SetDumpFile(&dmpFile);
		m_StackWalker.WalkFromException();
		dmpFile.Close();
	}
}
#endif

void kuApp::PrintCmdHelp() {
    wxString sep = wxT("\n");
    wxString msg = STRING_VERSION + sep + STRING_PROJECT + sep + STRING_AUTHOR + sep + sep
                   + wxT("Usage:") + sep
                   + wxT("kuview [init_state] [command [args]]") + sep
                   + wxT("\tinit_state = show | hide | full | icon") + sep
                   + wxT("\tcommand [args] = locate [dir|file] | slideshow [interval] [once] [dirs]") + sep
                   + wxT("\t\t           | reg [exts] | reg all | unreg [exts] | unreg all") + sep + sep
                   + wxT("example 1: start with fullscreen, and play slideshow with interval=5") + sep
                   + wxT("> kuview full slideshow 5 dir1 dir_2 \"dir 3\"") + sep
                   + wxT("example 2: start normally, locate file, and play slideshow") + sep
                   + wxT("> kuview locate file slideshow") + sep
                   + wxT("example 3: start normally, and play slideshow from file which is in dir1 ") + sep
                   + wxT("> kuview locate file slideshow dir1 dir2") + sep
                   + wxT("example 4: start normally, play slideshow without repeat, and locate file") + sep
                   + wxT("> kuview slideshow once dir1 dir2 locate file") + sep
                   + wxT("example 5: associate bmp, jpg, and jpeg") + sep
                   + wxT("> kuview reg bmp jpg jpeg") + sep
                   + wxT("example 6: associate all supported type, and avoid flashing window") + sep
                   + wxT("> kuview hide reg all") + sep
                   + wxT("example 7: unassociate all supported type, and avoid flashing window") + sep
                   + wxT("> kuview unreg all") + sep;
    wxMessageBox(msg, STRING_APPNAME, wxOK|wxICON_INFORMATION);
}

bool kuApp::ParseArguments(int argc, wxChar** argv) {
    if(argc<=1)    // no argv
        return true;

    wxChar** aptr = argv;
    aptr++;
    wxString first(*aptr);
    if(first.CmpNoCase(wxT("/?"))==0 || first.CmpNoCase(wxT("-h"))==0)    // help
        return false;

    // for init command
    int start = 1;
    if(DoInitCmd(*aptr)) {
        aptr++;
        start = 2;
    }

    mCmds.Clear();
    wxString cmd = wxEmptyString;
    for(int i=start; i<argc; i++) {
        if(IsActionCmd(*aptr)) {
            if(cmd!=wxEmptyString) {
                mCmds.Add(cmd);
                cmd.Clear();
            }
            cmd.Append(wxString(*aptr));
        } else if(IsPathCmd(*aptr)) {    // path arg
            wxFileName path(*aptr);
            if(path.IsRelative())    path.MakeAbsolute();
            if(cmd.CmpNoCase(CMD_LOCATE)==0) {    // locate command
                cmd.Append(CMD_SEP + path.GetFullPath());
            } else if(cmd.BeforeFirst(CMD_SEP).CmpNoCase(CMD_SLIDESHOW)==0) {    // slideshow command
                if(wxDirExists(path.GetFullPath()))    // path.IsDir() doesn't really check it
                    cmd.Append(CMD_SEP + path.GetFullPath());
                else
                    cmd.Append(CMD_SEP + path.GetPath());
            } else {    // add locate for it
                if(cmd!=wxEmptyString) {
                    mCmds.Add(cmd);
                    cmd.Clear();
                }
                cmd.Append(CMD_LOCATE);
                cmd.Append(CMD_SEP + path.GetFullPath());
            }
        } else {    // normal arg
            cmd.Append(CMD_SEP + wxString(*aptr));
        }
        aptr++;
    }
    if(cmd!=wxEmptyString)    mCmds.Add(cmd);
    //for(size_t i=0; i<mCmds.GetCount(); i++)    wxMessageBox(mCmds[i]);
    return true;
}

bool kuApp::DoInitCmd(wxChar* cmd) {
    wxArrayString enums;
    enums.Add(wxT("show"));
    enums.Add(wxT("hide"));
    enums.Add(wxT("full"));
    enums.Add(wxT("icon"));
    int index = enums.Index(cmd, false);
    if(index==wxNOT_FOUND) {
        mInit = kuID_INIT_SHOW;
        return false;
    }
    mInit = index;
    return true;
}

bool kuApp::IsPathCmd(wxChar* cmd) {
    wxString str(cmd);
    if(wxFileExists(str) || wxDirExists(str))
        return true;
    return false;
}

bool kuApp::IsActionCmd(wxChar* cmd) {
    wxArrayString enums;
    enums.Add(CMD_LOCATE);
    enums.Add(CMD_SLIDESHOW);
    enums.Add(CMD_REG);
    enums.Add(CMD_UNREG);
    enums.Add(CMD_RESCALE);
    if(enums.Index(cmd, false)==wxNOT_FOUND)
        return false;
    return true;
}

bool kuApp::DoActionCmd(wxString& cmd) {
    mIsDoingCmd = true;
    bool ret = false;
    wxStringTokenizer tkz(cmd, wxT("|"));
    wxArrayString tokens;
    while(tkz.HasMoreTokens())
        tokens.Add(tkz.GetNextToken());

    //for(size_t i=0; i<tokens.GetCount(); i++)    wxMessageBox(tokens[i]);

    if(tokens[0].CmpNoCase(CMD_LOCATE)==0) {
        if(tokens.GetCount()<2)    return false;
        ret = mFrame->Action(kuID_LOCATE, tokens[1]);
    } else if(tokens[0].CmpNoCase(CMD_SLIDESHOW)==0) {
        long interval;
        size_t start=1;
        if(tokens.GetCount()>start && tokens[start].IsNumber()) {
            if(tokens[start].ToLong(&interval))
                wxGetApp().mOptions.mSlideshowInterval = interval;
            start += 1;
        }
        if(tokens.GetCount()>start && tokens[start].CmpNoCase(wxT("once"))==0) {
            wxGetApp().mOptions.mSlideshowRepeat = false;
            start += 1;
        }
        for(size_t i=start; i<tokens.GetCount(); i++) {
            mFrame->mSlideshowDirs.Add(tokens[i]);
        }
        // let kuID_SLIDESHOW handle it
        //if(!mFrame->mSlideshowDirs.IsEmpty()) {
        //    mFrame->mSlideshowIndex = 0;
        //    mFrame->mGeneric->Locate(mFrame->mSlideshowDirs[mFrame->mSlideshowIndex]);
        //    wxSafeYield();
        //}
        //
        ret = mFrame->Action(kuID_SLIDESHOW);
    } else if(tokens[0].CmpNoCase(CMD_REG)==0) {
        mQuit = true;
        if(tokens.GetCount()<2)    return false;
        // get supported extensions
        wxArrayString supports;
        kuFiWrapper::GetSupportedExtensions(&supports);
        wxArrayString exts;
        if(tokens.GetCount()==2 && tokens[1]==wxT("all")) {
             exts = supports;
        } else {
            // filter unsupported extensions
            for(size_t i=1; i<tokens.GetCount(); i++) {
                if(supports.Index(tokens[i].Lower())!=wxNOT_FOUND)   exts.Add(tokens[i]);
            }
            if(exts.IsEmpty())    return false;
        }
        // reg section
        SetRegSection(true);
        // check and set reg
        for(size_t i=0; i<exts.GetCount(); i++) {
            if(!GetFileAssociation(exts[i]))    SetFileAssociation(exts[i], true);
        }
    } else if(tokens[0].CmpNoCase(CMD_UNREG)==0) {
        mQuit = true;
        if(tokens.GetCount()<2)    return false;
        // get supported extensions
        wxArrayString supports;
        kuFiWrapper::GetSupportedExtensions(&supports);
        wxArrayString exts;
        if(tokens.GetCount()==2 && tokens[1]==wxT("all")) {
             exts = supports;
        } else {
            // filter unsupported extensions
            for(size_t i=1; i<tokens.GetCount(); i++) {
                if(supports.Index(tokens[i].Lower())!=wxNOT_FOUND)   exts.Add(tokens[i]);
            }
            if(exts.IsEmpty())    return false;
        }
        // check and unset reg
        for(size_t i=0; i<exts.GetCount(); i++) {
            if(GetFileAssociation(exts[i]))    SetFileAssociation(exts[i], false);
        }
        // reg section
        bool clean = true;
        for(size_t i=0; i<supports.GetCount(); i++) {
            if(GetFileAssociation(supports[i])) {
                clean = false;
                break;
            }
        }
        if(clean)    SetRegSection(false);
    } else if(tokens[0].CmpNoCase(CMD_RESCALE)==0) {
        mQuit = true;
        // TODO
    }
    mIsDoingCmd = false;
    return ret;
}

void kuApp::SetOptions() {
    mOptions.mFilename = wxFileName(mPath, wxT("kuview.ini")).GetFullPath();

    mOptions.mLanguage = wxLANGUAGE_DEFAULT;

    mOptions.mFavorite = wxStandardPaths::Get().GetDocumentsDir();

    mOptions.mKeepStyle = SCALE_AUTOFIT;
    mOptions.mScale = SCALE_BASE;
    mOptions.mKeepRotate = false;
    mOptions.mRotate = 0;
    mOptions.mKeepLeftTop = false;
    mOptions.mLeftTop = wxPoint(0,0);
    mOptions.mLoadCompletely = false;
    mOptions.mPrefetch = true;
    mOptions.mSlideshowInterval = 2;
    mOptions.mOpaque = 255;
    mOptions.mPrintScale = 1.0;
    mOptions.mPrintAlign = kuAlignSelector::kuID_ALIGN_CC;
    mOptions.mFilter = FILTER_BILINEAR;
    mOptions.mSlideshowRepeat = true;
    mOptions.mSlideshowDirectory = kuID_SLIDESHOW_CURRENT;
    mOptions.mLoadSiblings[0] = 2;
    mOptions.mLoadSiblings[1] = 1;
    mOptions.mLoadSiblings[2] = 3;
    mOptions.mLoadSiblings[3] = 2;
    mOptions.mLoadCache = 10;

    mOptions.mSummaryPosition = wxPoint(10, 10);
    mOptions.mSummaryColor = *wxGREEN;
    for(size_t i=0; i<FIMD_CUSTOM; i++) {
        mOptions.mSummaryTags[i].Clear();
    }
    mOptions.mSummaryTags[FIMD_EXIF_MAIN].Add(wxT("DateTime"));
    mOptions.mSummaryTags[FIMD_EXIF_MAIN].Add(wxT("Make"));
    mOptions.mSummaryTags[FIMD_EXIF_MAIN].Add(wxT("Model"));
    mOptions.mSummaryTags[FIMD_EXIF_EXIF].Add(wxT("ExposureBiasValue"));
    mOptions.mSummaryTags[FIMD_EXIF_EXIF].Add(wxT("ExposureMode"));
    mOptions.mSummaryTags[FIMD_EXIF_EXIF].Add(wxT("ExposureTime"));
    mOptions.mSummaryTags[FIMD_EXIF_EXIF].Add(wxT("Flash"));
    mOptions.mSummaryTags[FIMD_EXIF_EXIF].Add(wxT("FNumber"));
    mOptions.mSummaryTags[FIMD_EXIF_EXIF].Add(wxT("FocalLength"));
    mOptions.mSummaryTags[FIMD_EXIF_EXIF].Add(wxT("ISOSpeedRatings"));
    mOptions.mSummaryTags[FIMD_EXIF_EXIF].Add(wxT("SceneCaptureType"));
    mOptions.mSummaryTags[FIMD_EXIF_EXIF].Add(wxT("ShutterSpeedValue"));

    mOptions.mExternalOpenNames.Clear();
    mOptions.mExternalOpenNames.Add(STRING_DEFAULT_EXTERNAL_OPEN_VIEW);
    mOptions.mExternalOpenNames.Add(STRING_DEFAULT_EXTERNAL_OPEN_MAPS);
    mOptions.mExternalOpenExecs.Clear();
    mOptions.mExternalOpenArgss.Clear();
    #ifdef __WXMSW__
    mOptions.mExternalOpenExecs.Add(wxT("rundll32.exe"));
    mOptions.mExternalOpenArgss.Add(wxT("C:\\WINDOWS\\system32\\shimgvw.dll,ImageView_Fullscreen %F"));
    mOptions.mExternalOpenExecs.Add(wxT("open"));
    mOptions.mExternalOpenArgss.Add(wxT("http://maps.google.com/?ie=UTF8&ll=%LA,%LO"));
    #else
    mOptions.mExternalOpenExecs.Add(wxT("xdg-open"));
    mOptions.mExternalOpenArgss.Add(wxT("%F"));
    mOptions.mExternalOpenExecs.Add(wxT("xdg-open"));
    mOptions.mExternalOpenArgss.Add(wxT("http://maps.google.com/?ie=UTF8&ll=%LA,%LO"));
    #endif

    mOptions.mRescaleMgrMaxCon = 2;
    mOptions.mPicasaWebMgrMaxCon = 2;

    mIsImported = mOptions.Import();
}

void kuApp::SetRegDefault() {
    #ifdef __WXMSW__
    mRegDefault[wxT(".bmp")] = wxT("Paint.Picture");
    mRegDefault[wxT(".gif")] = wxT("giffile");
    mRegDefault[wxT(".ico")] = wxT("icofile");
    mRegDefault[wxT(".jpe")] = mRegDefault[wxT(".jpeg")] = mRegDefault[wxT(".jpg")] = wxT("jpegfile");
    mRegDefault[wxT(".png")] = wxT("pngfile");
    mRegDefault[wxT(".tif")] = mRegDefault[wxT(".tiff")] = wxT("TIFImage.Document");
    #endif
}

void kuApp::ClearThread() {
    mWaitLoader = true;
    if(mLoader)    mLoader->Delete();
    while(mWaitLoader)    wxMilliSleep(100);
}

bool kuApp::SwitchLocale(wxString path) {
    if(mLocale!=NULL)   delete mLocale;
    mLocale = new wxLocale(mOptions.mLanguage);
    // load mo file
    mLocale->AddCatalogLookupPathPrefix(path);
    bool success=true;   // default is english
    if(mOptions.mLanguage!=wxLANGUAGE_ENGLISH) {
        // search CanonicalName
        success=mLocale->AddCatalog(wxT("kuview_")+wxLocale::GetLanguageInfo(mOptions.mLanguage)->CanonicalName);
        // search language code only
        if(!success)
            success=mLocale->AddCatalog(wxT("kuview_")+wxLocale::GetLanguageInfo(mOptions.mLanguage)->CanonicalName.BeforeFirst('_'));
        // fall to english if no appropriate mo file
        if(!success) {
            delete mLocale;
            mLocale = new wxLocale(wxLANGUAGE_ENGLISH);
        }
    }
    // regenerate menuBar/toolBar
    if(mFrame) {
        mFrame->SetupMenuBar();
        mFrame->SetupToolBar();
        mFrame->SetupManagers();
        mFrame->mSingle->SetupPopupMenu();
        mFrame->mGeneric->SetupPopupMenu();
        mFrame->mMultiple->SetupPopupMenu();
        mFrame->SetDefaults();
    }
    return success;
}

bool kuApp::GetInterrupt() {
    return mIsInterrupted;
}
bool kuApp::SetInterrupt(bool interrupt) {
    wxMutexLocker lock(mInterruptMutex);
    if(!lock.IsOk())   return false;
    mIsInterrupted=interrupt;
    return true;
}

bool kuApp::GetBusy() {
    return mIsBusy;
}
bool kuApp::SetBusy(bool busy) {
    wxMutexLocker lock(mBusyMutex);
    if(!lock.IsOk())   return false;
    mIsBusy=busy;
    if(mFrame!=NULL && mFrame->IsShown()) {
        mFrame->GetMenuBar()->Enable(kuID_INTERRUPT,busy);
        mFrame->GetToolBar()->EnableTool(kuID_INTERRUPT,busy);
    }
    #ifdef __WXMSW__
    SystemParametersInfo(SPI_SETSCREENSAVEACTIVE, !busy, 0, 0);
    #endif
    return true;
}

bool kuApp::GetEdited() {
    return mIsEdited;
}
bool kuApp::SetEdited(bool edited) {
    if(!mFrame || mFrame->mIsArchive)   return false;   // don't support edit in archive
    wxMutexLocker lock(mEditedMutex);
    if(!lock.IsOk())   return false;
    mIsEdited=edited;
    mFrame->GetMenuBar()->Enable(kuID_SAVE,edited);
    mFrame->GetToolBar()->EnableTool(kuID_SAVE,edited);
    mFrame->mSingle->mMenu->Enable(kuID_SAVE,edited);
    return true;
}

void kuApp::CheckEdited(const wxChar* caption) {
    if(caption && GetEdited() && wxMessageBox(STRING_LOAD_MESSAGE,mFrame->StripCodes(caption),wxYES_NO|wxICON_QUESTION)==wxYES
       || !GetEdited() && !mOptions.mLoadCompletely && kuFiWrapper::GetOriginalJPEGSize(mFrame->mSingle->GetOrigBmp())!=wxSize(0,0)) {
        mOptions.mLoadCompletely = true;
        mFrame->mSingle->ReloadImage(mFrame->mSingle->GetFilename(),false,true);
        mOptions.mLoadCompletely = false;
    }
}

bool kuApp::SetSaveAs(bool saveas) {
    if(!mFrame || mFrame->mIsArchive)   return false;   // don't support edit in archive
    wxMutexLocker lock(mSaveAsMutex);
    if(!lock.IsOk())   return false;
    mIsSaveAs=saveas;
    mFrame->GetMenuBar()->Enable(kuID_SAVEAS,saveas);
    mFrame->mGeneric->mMenu->Enable(kuID_SAVEAS,saveas);
    mFrame->mSingle->mMenu->Enable(kuID_SAVEAS,saveas);
    return true;
}

// -------- system related --------
wxArrayInt kuApp::ShellIntegration(wxArrayInt style) {
    #ifdef __WXMSW__
    wxRegKey* key = NULL;
    if(style.IsEmpty()) {   // clear registry
        wxArrayInt selections;
        key = new wxRegKey(STRING_REG_DRIVE);
        if(key->Exists()) {
            selections.Add(0);
            key->DeleteSelf();
        }
        delete key;
        key = new wxRegKey(STRING_REG_DIRECTORY);
        if(key->Exists()) {
            selections.Add(1);
            key->DeleteSelf();
        }
        delete key;
        key = new wxRegKey(STRING_REG_FILE);
        if(key->Exists()) {
            selections.Add(2);
            key->DeleteSelf();
        }
        delete key;
        return selections;
    }
    // write registry
    for(size_t i=0;i<style.GetCount();i++) {
        switch (style[i]) {
            case 0:
                key = new wxRegKey(STRING_REG_DRIVE);
                break;
            case 1:
                key = new wxRegKey(STRING_REG_DIRECTORY);
                break;
            case 2:
                key = new wxRegKey(STRING_REG_FILE);
                break;
            default:
                break;
        }
        if(!key->Exists())   key->Create();
        key->SetValue(wxEmptyString,STRING_SHELL_BROWSE);
        wxRegKey* command = new wxRegKey(*key,STRING_REG_COMMAND);
        if(!command->Exists())   command->Create();
        command->SetValue(wxEmptyString,mShellExecute);
        delete key;
        delete command;
    }
    #endif
    return wxArrayInt();
}

void kuApp::SetRegSection(bool set) {
    #ifdef __WXMSW__
    wxRegKey* key;
    if(set) {
        // avoid wrong path
        key = new wxRegKey(STRING_REG_APP);
        if(key->Exists())   key->DeleteSelf();
        delete key;
        // generate main section
        key = new wxRegKey(STRING_REG_MAIN);
        if(!key->Exists())   key->Create();
        key->SetValue(wxEmptyString,STRING_APPNAME);
        // shell\\open\\command
        wxRegKey* command = new wxRegKey(*key,STRING_REG_SHELLCMD);
        if(!command->Exists())   command->Create();
        command->SetValue(wxEmptyString,mShellExecute);
        delete command;
        // DefaultIcon
        wxRegKey* icon = new wxRegKey(*key,STRING_REG_ICON);
        if(!icon->Exists())   icon->Create();
        icon->SetValue(wxEmptyString,mAssociationExecute+wxT(",0"));
        delete icon;
        // Backup
        wxRegKey* backup = new wxRegKey(*key,STRING_REG_BACKUP);
        if(!backup->Exists())   icon->Create();
        delete backup;
    } else {
        key = new wxRegKey(STRING_REG_MAIN);
        if(key->Exists())   key->DeleteSelf();
    }
    delete key;
    #endif
}

bool kuApp::GetFileAssociation(wxString ext) {
    bool ret = false;
    #ifdef __WXMSW__
    if(!ext.StartsWith(wxT(".")))    ext = wxT(".") + ext;
    // query associate
    wxRegKey* key = new wxRegKey(STRING_REG_CLASSEXT+ext);
    wxString value=wxEmptyString;
    if(key->Exists() && key->HasValue(wxEmptyString) && key->QueryValue(wxEmptyString,value)) {
        //wxMessageBox(ext+wxT(": ")+value);
        if(value==STRING_REG_SHORTNAME)    ret = true;
    }
    delete key;
    #endif
    return ret;
}
void kuApp::SetFileAssociation(wxString ext, bool associate) {
    #ifdef __WXMSW__
    if(!ext.StartsWith(wxT(".")))    ext = wxT(".") + ext;
    if(GetFileAssociation(ext)==associate)    return;

    // start associate
    wxRegKey* key = new wxRegKey(STRING_REG_CLASSEXT+ext);
    wxRegKey* main = new wxRegKey(STRING_REG_MAIN);
    wxRegKey* backup = new wxRegKey(*main, STRING_REG_BACKUP);

    if(associate) {
        if(key->Exists()) {
            wxString value = wxEmptyString;
            if(key->QueryValue(wxEmptyString, value))    backup->SetValue(ext, value);
        } else {
            key->Create();
        }
        key->SetValue(wxEmptyString,STRING_REG_SHORTNAME);
        // remove userext
        wxRegKey* user = new wxRegKey(STRING_REG_USEREXT+ext);
        if(user->Exists())   user->DeleteSelf();
        delete user;
    }
    else {
        if(key->Exists()) {
            bool success = false;
            if(backup->HasValue(ext)) {    // has backup
                wxString value = wxEmptyString;
                backup->QueryValue(ext, value);
                wxRegKey* orig = new wxRegKey(STRING_REG_CLASSEXT+value);
                if(orig->Exists()) {    // has this item
                    key->SetValue(wxEmptyString, value);
                    backup->DeleteValue(ext);
                    success = true;
                }
                delete orig;
            }
            if(!success && wxGetApp().mRegDefault[ext]!=wxEmptyString) {    // no backup, has default
                key->SetValue(wxEmptyString, wxGetApp().mRegDefault[ext]);
                success = true;
            }
            if(!success) {    // no backup, no default
                key->DeleteSelf();
            }
        } else {    // remove backup
            if(backup->HasValue(ext))   backup->DeleteValue(ext);
        }
    }
    delete backup;
    delete main;
    delete key;
    #endif
}

// -------- utility --------
wxString kuApp::GetReadableSize(wxString filename) {
    wxULongLong lsize = wxFileName(filename).GetSize();
    double dsize = lsize.ToDouble();
    int isize = (int)ceil(dsize);
    int step = 1024;

    if(isize < step)    return wxString::Format(wxT("%s B"), lsize.ToString().c_str());

    dsize /= step;
    isize = (int)ceil(dsize);
    if(isize < step*9)    return wxString::Format(wxT("%d KB"), isize);

    dsize /= step;
    isize = (int)ceil(dsize);
    return wxString::Format(wxT("%d MB"), isize);
}

bool kuApp::IsImageFile(wxString& filename) {
    wxString wildcard = kuFiWrapper::GetSupportedExtensions() + wxT(";");
    wxString ext = filename.AfterLast('.');
    ext = wxT("*.") + ext + wxT(";");
    // check extension in extwildcard only
    if(wildcard.MakeLower().Contains(ext.MakeLower()))   return true;
    else   return false;
}

bool kuApp::IsArchiveFile(wxString& filename) {
    wxString ext = wxFileName(filename).GetExt();
    return (ext.CmpNoCase(wxT("zip"))==0 || ext.CmpNoCase(wxT("tar"))==0);
}

bool kuApp::IsUNCPath(wxString& path) {
    wxChar sep = wxFileName::GetPathSeparator();
    return (path[0]==sep && path[1]==sep && wxFileName(path).IsOk());
}

bool kuApp::SyncFileTime(wxString src, wxString dest) {
    wxDateTime dtCreate;
    wxDateTime dtMod;
    bool success = false;
    if(wxFileName(src).GetTimes(NULL, &dtMod, &dtCreate)) {
        success = wxFileName(dest).SetTimes(NULL, &dtMod, &dtCreate);
    }
    return success;
}

void kuApp::EnableSizerChildren(wxSizer* sizer, bool enable) {
    wxSizerItemList list = sizer->GetChildren();
    wxSizerItemList::iterator iter;
    for(iter=list.begin(); iter!=list.end(); iter++) {
        wxSizerItem* item = *iter;
        wxWindow* window = item->GetWindow();
        if(window)    window->Enable(enable);
    }
}

// -------- common UI --------
long kuApp::GetNumberFromUser(wxString message, wxString caption, long* target, long min, long max) {
    long value;
    while(true) {
        value = wxGetNumberFromUser(message, wxEmptyString, caption, *target, min, max);
        if(value==-1)    return -1;
        if(value>0)    break;
    }
    *target = value;
    return value;
}

double kuApp::GetScaleFromUser(wxWindow* parent, double current) {
    wxString str;
    double scale;
    while(true) {
        str = wxGetTextFromUser(STRING_RESCALE_MESSAGE,kuFrame::StripCodes(STRING_MENU_RESCALE),
                                wxString::Format(wxT("%.4f"),current),parent);
        if(str==wxEmptyString)    return -1;
        if(!str.ToDouble(&scale))    wxMessageBox(STRING_RESCALE_ERROR);
        else if(scale>0)    break;
    }
    return scale;
}

// -------- kuOptions --------
bool kuOptions::Export() {
    wxString updir = wxT("..");
    kuConfig* cfg = new kuConfig();

    cfg->SetPath(CFG_GROUP_GENERAL);
    cfg->Write(CFG_GENERAL_LANGUAGE,      mLanguage);
    cfg->SetPath(updir);

    cfg->SetPath(CFG_GROUP_TRAVERSE);
    cfg->Write(CFG_TRAVERSE_FAVORITE,     mFavorite);
    cfg->SetPath(updir);

    cfg->SetPath(CFG_GROUP_VIEW);
    cfg->Write(CFG_VIEW_KEEPSTYLE,        mKeepStyle);
    cfg->Write(CFG_VIEW_SCALE,            mScale);
    cfg->Write(CFG_VIEW_KEEPROTATE,       mKeepRotate);
    cfg->Write(CFG_VIEW_ROTATE,           mRotate);
    cfg->Write(CFG_VIEW_KEEPLEFTTOP,      mKeepLeftTop);
    cfg->WritePoint(CFG_VIEW_LEFTTOP,     mLeftTop);
    cfg->Write(CFG_VIEW_LOAD_COMPLETELY,  mLoadCompletely);
    cfg->Write(CFG_VIEW_PREFETCH,         mPrefetch);
    cfg->WriteFilter(CFG_VIEW_RESCALE,    mFilter);
    cfg->SetPath(updir);

    cfg->SetPath(CFG_GROUP_SLIDESHOW);
    cfg->Write(CFG_SLIDESHOW_REPEAT,   mSlideshowRepeat);
    cfg->Write(CFG_SLIDESHOW_INTERVAL, mSlideshowInterval);
    cfg->SetPath(updir);

    cfg->SetPath(CFG_GROUP_SUMMARY);
    cfg->WritePoint(CFG_SUMMARY_POSITION,   mSummaryPosition);
    cfg->WriteColour(CFG_SUMMARY_COLOR,     mSummaryColor);
    cfg->SetPath(updir);

    cfg->SetPath(CFG_GROUP_MANAGE);
    for(size_t i=0; i<mHotkeyDirs.GetCount(); i++) {
        cfg->Write(wxString::Format(CFG_MANAGE_DIRSET, i), mHotkeyDirs[i]);
    }
    cfg->SetPath(updir);

    for(size_t i=0; i<mExternalOpenNames.GetCount(); i++) {
        cfg->SetPath(wxString::Format(CFG_GROUP_EXTERNAL,i));
        cfg->Write(CFG_EXTERNAL_NAME, mExternalOpenNames[i]);
        cfg->Write(CFG_EXTERNAL_EXEC, mExternalOpenExecs[i]);
        cfg->Write(CFG_EXTERNAL_ARGS, mExternalOpenArgss[i]);
        cfg->SetPath(updir);
    }

    cfg->SetPath(CFG_GROUP_RESCALEMGR);
    cfg->Write(CFG_MANAGER_MAXCON, mRescaleMgrMaxCon);
    cfg->SetPath(updir);

    cfg->SetPath(CFG_GROUP_PICASAWEBMGR);
    cfg->Write(CFG_MANAGER_MAXCON, mPicasaWebMgrMaxCon);
    cfg->SetPath(updir);

    wxFileOutputStream os(mFilename);
    bool ret = cfg->Save(os);
    delete cfg;
    return ret;
}
bool kuOptions::Import() {
    if(!wxFileExists(mFilename))    return false;
    wxFileInputStream input(mFilename);
    if(!input.IsOk())    return false;

    wxString updir = wxT("..");
    wxPoint xy;
    kuConfig* cfg = new kuConfig(input);

    cfg->SetPath(CFG_GROUP_GENERAL);
    cfg->Read(CFG_GENERAL_LANGUAGE,     &mLanguage,       wxLANGUAGE_DEFAULT);
    cfg->SetPath(updir);

    cfg->SetPath(CFG_GROUP_TRAVERSE);
    cfg->Read(CFG_TRAVERSE_FAVORITE,    &mFavorite,       wxStandardPaths::Get().GetDocumentsDir());
    cfg->SetPath(updir);

    cfg->SetPath(CFG_GROUP_VIEW);
    cfg->Read(CFG_VIEW_KEEPSTYLE,       &mKeepStyle,      SCALE_AUTOFIT);
    cfg->Read(CFG_VIEW_SCALE,           &mScale,          SCALE_BASE);
    cfg->Read(CFG_VIEW_KEEPROTATE,      &mKeepRotate,     false);
    cfg->Read(CFG_VIEW_ROTATE,          &mRotate,         0);
    cfg->Read(CFG_VIEW_KEEPLEFTTOP,     &mKeepLeftTop,    false);
    cfg->ReadPoint(CFG_VIEW_LEFTTOP,    &mLeftTop,        wxPoint(0,0));
    cfg->Read(CFG_VIEW_LOAD_COMPLETELY, &mLoadCompletely, false);
    cfg->Read(CFG_VIEW_PREFETCH,        &mPrefetch,       true);
    cfg->ReadFilter(CFG_VIEW_RESCALE,   &mFilter,         FILTER_BILINEAR);
    cfg->SetPath(updir);

    cfg->SetPath(CFG_GROUP_SLIDESHOW);
    cfg->Read(CFG_SLIDESHOW_REPEAT,   &mSlideshowRepeat,   true);
    cfg->Read(CFG_SLIDESHOW_INTERVAL, &mSlideshowInterval, 2);
    cfg->SetPath(updir);

    cfg->SetPath(CFG_GROUP_SUMMARY);
    cfg->ReadPoint(CFG_SUMMARY_POSITION,  &mSummaryPosition, wxPoint(10,10));
    cfg->ReadColour(CFG_SUMMARY_COLOR,    &mSummaryColor,    *wxGREEN);
    cfg->SetPath(updir);

    cfg->SetPath(CFG_GROUP_MANAGE);
    mHotkeyDirs.Clear();
    size_t idx=0;
    while(true) {
        wxString entry = wxString::Format(CFG_MANAGE_DIRSET, idx);
        if(!cfg->HasEntry(entry))    break;
        wxString dir;
        cfg->Read(entry, &dir, wxEmptyString);
        if(dir != wxEmptyString)    mHotkeyDirs.Add(dir);
        idx++;
    }
    cfg->SetPath(updir);

    mExternalOpenNames.Clear();
    mExternalOpenExecs.Clear();
    mExternalOpenArgss.Clear();
    idx=0;
    while(true) {
        wxString grp = wxString::Format(CFG_GROUP_EXTERNAL,idx);
        if(!cfg->HasGroup(grp))    break;
        cfg->SetPath(grp);
        if(cfg->HasEntry(CFG_EXTERNAL_NAME) && cfg->HasEntry(CFG_EXTERNAL_EXEC)) {
            wxString name, exec, args;
            cfg->Read(CFG_EXTERNAL_NAME, &name, wxEmptyString);
            cfg->Read(CFG_EXTERNAL_EXEC, &exec, wxEmptyString);
            cfg->Read(CFG_EXTERNAL_ARGS, &args, wxEmptyString);
            if(name!=wxEmptyString && exec!=wxEmptyString) {
                mExternalOpenNames.Add(name);
                mExternalOpenExecs.Add(exec);
                mExternalOpenArgss.Add(args);
            }
        }
        cfg->SetPath(updir);
        idx++;
    }

    cfg->SetPath(CFG_GROUP_RESCALEMGR);
    cfg->Read(CFG_MANAGER_MAXCON, &mRescaleMgrMaxCon, 2);
    cfg->SetPath(updir);

    cfg->SetPath(CFG_GROUP_PICASAWEBMGR);
    cfg->Read(CFG_MANAGER_MAXCON, &mPicasaWebMgrMaxCon, 2);
    cfg->SetPath(updir);

    return true;
}

// -------- kuConfig --------
kuConfig::kuConfig()
    :wxFileConfig() {
}
kuConfig::kuConfig(wxFileInputStream& fis)
    :wxFileConfig(fis) {
}

bool kuConfig::WritePoint(const wxString& key, wxPoint& value) {
    return wxFileConfig::Write(key, wxString::Format(CFG_FORMAT_POINT, value.x, value.y));
}
bool kuConfig::ReadPoint(const wxString& key, wxPoint* value, const wxPoint& defval) {
    wxString str;
    wxFileConfig::Read(key, &str, wxString::Format(CFG_FORMAT_POINT, defval.x, defval.y));
    if(wxSscanf(str.wc_str(), CFG_FORMAT_POINT, &(value->x), &(value->y))==2) {
        return true;
    } else {
        *value = defval;
        return false;
    }
}

bool kuConfig::WriteFilter(const wxString& key, FREE_IMAGE_FILTER& value) {
    return wxFileConfig::Write(key, (int)value);
}
bool kuConfig::ReadFilter(const wxString& key, FREE_IMAGE_FILTER* value, const FREE_IMAGE_FILTER defval) {
    int num;
    if(wxFileConfig::Read(key, &num, (int)defval)) {
        *value = (FREE_IMAGE_FILTER)num;
        return true;
    } else {
        *value = defval;
        return false;
    }
}

bool kuConfig::WriteColour(const wxString& key, wxColour& value) {
    return wxFileConfig::Write(key, value.GetAsString(wxC2S_HTML_SYNTAX));
}
bool kuConfig::ReadColour(const wxString& key, wxColour* value, const wxColour& defval) {
    wxString str;
    if(wxFileConfig::Read(key, &str, defval.GetAsString(wxC2S_HTML_SYNTAX))) {
        value->Set(str);
        return true;
    } else {
        *value = defval;
        return false;
    }
}

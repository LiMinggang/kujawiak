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

#include <wx/wx.h>
#include <wx/artprov.h>
#include <wx/bookctrl.h>
#include <wx/clipbrd.h>
#include <wx/cmdline.h>
#include <wx/colordlg.h>
#include <wx/config.h>
#include <wx/dir.h>
#include <wx/dirctrl.h>
#include <wx/dnd.h>
#include <wx/dynarray.h>
#include <wx/dynlib.h>
#include <wx/fileconf.h>
#include <wx/fs_filter.h>
#include <wx/fs_arc.h>
#include <wx/grid.h>
#include <wx/image.h>
#include <wx/ipc.h>
#include <wx/mimetype.h>
#include <wx/numdlg.h>
#include <wx/print.h>
#include <wx/printdlg.h>
#include <wx/propdlg.h>
#include <wx/slider.h>
#include <wx/splash.h>
#include <wx/splitter.h>
#include <wx/sstream.h>
#include <wx/stackwalk.h>
#include <wx/statline.h>
#include <wx/stdpaths.h>
#include <wx/tarstrm.h>
#include <wx/taskbar.h>
#include <wx/thread.h>
#include <wx/timer.h>
#include <wx/tokenzr.h>
#include <wx/treectrl.h>
#include <wx/valgen.h>
#include <wx/wfstream.h>
#include <wx/xml/xml.h>
#include <wx/zipstrm.h>

#include <FreeImage.h>

#ifdef ENABLE_PICASAWEBMGR
#include <curl/curl.h>
#endif

// -------- constant --------
#define SCALE_BASE      (10000)
#define SCALE_DIFF      (1000)
#define SCALE_ORIGINAL  (0)
#define SCALE_BESTFIT   (-1)
#define SCALE_EXTEND    (-2)
#define SCALE_AUTOFIT   (-3)
#define SCALE_LASTUSED  (-4)

#define WALLPAPER_CENTER  (0)
#define WALLPAPER_TILE    (1)
#define WALLPAPER_STRETCH (2)
#define WALLPAPER_BESTFIT (3)
#define WALLPAPER_EXTEND  (4)

//0 is filename and message
#define STATUS_FIELD_FILESIZE   (1)
#define STATUS_FIELD_TIME       (2)
#define STATUS_FIELD_GAUGE      (3)
#define STATUS_FIELD_BMPSIZE    (4)
#define STATUS_FIELD_SCALE      (5)
#define STATUS_FIELD_MOTION     (6)

#define SCROLL_RATE_SINGLE   (1)
#define SCROLL_RATE_MULTIPLE (5)

#define SPLITTER_TOP_WIDTH   (250)
#define SPLITTER_DIR_HEIGHT  (200)
#define SPLITTER_VIEW_HEIGHT (200)
#define THUMBNAIL_WIDTH      (100)

// -------- format string --------
#define FORMAT_ASSOCIATION       wxT("\"%s\"")
#define FORMAT_SHELL             wxT("\"%s\" \"%%1\"")
#define FORMAT_EXTERNAL_OPEN     wxT("%d")
#define FORMAT_BMPSIZE           wxT("%dx%dx%d")
#define FORMAT_MANAGER_SIZE      wxT("%d x %d")
#define FORMAT_MANAGER_ALBUM     wxT("%s: %s")
#define FORMAT_MANAGER_PROGRESS  wxT("%d / %d / %d")
#define FORMAT_MANAGER_DLGTITLE  wxT("%s %s")
#define FORMAT_IDLETASK          wxT("%d|%s|%s")
#define FORMAT_DIRSET_LISTITEM   wxT("%d %s")


// -------- about string --------
#define STRING_REV  wxT("$Rev: 200 $")

#define STRING_APPNAME wxT("kuView")
#define STRING_VERSION wxT("Version: 1.8 [") + wxString(STRING_REV).AfterFirst(' ').BeforeLast(' ') + wxT("]")
//#define STRING_VERSION wxT("Version: 1.6")
#define STRING_PROJECT wxT("Project: http://kujawiak.sourceforge.net/")
#define STRING_AUTHOR  wxT("Author: augustino@users.sourceforge.net")


// -------- status string --------
#define STRING_INFO_EXPLORE_FILESYSTEM  _("Exploring the file in Filesystem Panel...")
#define STRING_INFO_ENUMERATE_ARCHIVE   _("Enumerating items in Archive Panel...")
#define STRING_INFO_DELETE_ARCHIVE      _("Deleting all items in Archive Panel...")
#define STRING_INFO_THUMBS              _("Loading thumbs in selected directory...")
#define STRING_INFO_SAVED               _("File has been saved: ")
#define STRING_INFO_CANCELED            _("Operation has been canceled.")
#define STRING_INFO_FILE_CUT            _("Cut: ")
#define STRING_INFO_FILE_COPY           _("Copy: ")
#define STRING_INFO_FILE_COPYING        _("Copying: ")
#define STRING_INFO_FILE_MOVING         _("Moving: ")
#define STRING_INFO_FILE_DELETING       _("Deleting: ")
#define STRING_INFO_FILE_RENAMESUCCEED  _("File has been renamed: ")
#define STRING_INFO_FILE_COPYSUCCEED    _("File has been copied: ")
#define STRING_INFO_FILE_MOVESUCCEED    _("File has been moved: ")
#define STRING_INFO_FILE_DELETESUCCEED  _("File has been deleted: ")
#define STRING_INFO_METADATA_NOTFOUND   _("File doesn't contain any metadata...")
#define STRING_INFO_GPSLL_NOTFOUND      _("File doesn't contain latitude/longitude data...")
#define STRING_INFO_HISTORY_FIRST       _("This is the first in history.")
#define STRING_INFO_HISTORY_LAST        _("This is the last in history.")
#define STRING_INFO_HISTORY_NOBRANCH    _("No branch before this in history.")

#define STRING_WARNING_INTERRUPTED      _("Interrupted by user.")
#define STRING_WARNING_ISNOTFILE        _("Please select a picture.")
#define STRING_WARNING_DIRNOFILES       _("Please select a directory which contains pictures.")

#define STRING_ERROR_BUSY               _("Please wait or interrupt current operation!")
#define STRING_ERROR_SAVEFAILED         _("Cannot save: ")
#define STRING_ERROR_FILE_RENAMEFAILED  _("Cannot rename: ")
#define STRING_ERROR_FILE_COPYFAILED    _("Cannot copy: ")
#define STRING_ERROR_FILE_MOVEFAILED    _("Cannot move: ")
#define STRING_ERROR_FILE_DELETEFAILED  _("Cannot delete: ")
#define STRING_ERROR_FILE_MKDIRFAILED   _("Cannot mkdir: ")
#define STRING_ERROR_FILE_RMDIRFAILED   _("Cannot rmdir: ")
#define STRING_ERROR_FILE_EXIST         _("Already exists: ")
#define STRING_ERROR_FILE_NOTEXIST      _("Doesn't exist: ")
#define STRING_ERROR_FILE_DESISSRC      _("Destination is source!\n")
#define STRING_ERROR_FILE_DESINSRC      _("Destination is in source!\n")
#define STRING_ERROR_FILE_DESINVALID    _("Destination is invalid!\n")
#define STRING_ERROR_WALLPAPER_ARCHIVE  _("Setting a picture in archive as wallpaper is not supported!")
#define STRING_ERROR_PREVIEW_FAILED     _("Cannot preview! Please check printer setting...")
#define STRING_ERROR_PRINT_FAILED       _("There was an error during printing!\nPlease check printer and try again...")
#define STRING_ERROR_LOGIN_FAILED       _("Failed to login!")
#define STRING_ERROR_ALBUM_QUERYFAILED  _("Failed to get albums!")
#define STRING_ERROR_ALBUM_NEWFAILED    _("Failed to create albums!")
#define STRING_ERROR_ALBUM_EDITFAILED   _("Failed to modify albums!")
#define STRING_ERROR_ALBUM_DELETEFAILED _("Failed to delete albums!")
#define STRING_ERROR_PHOTO_QUERYFAILED  _("Failed to get photos!")


// -------- menu string --------
#define STRING_MENU_FILE       _("&File")
#define STRING_MENU_SAVE       _("&Save\tCtrl+S")
#define STRING_MENU_SAVEAS     _("Save &As...")
#define STRING_MENU_PAGESETUP  _("Pa&ge Setup")
#define STRING_MENU_PREVIEW    _("Print Pre&view...\tShift+P")
#define STRING_MENU_PRINT      _("&Print\tCtrl+P")
#define STRING_MENU_EXPLORE       _("Explore")
#ifdef __WXMSW__
#define STRING_MENU_EXTERNAL_EDIT _("External Editor")
#endif
#define STRING_MENU_EXTERNAL_OPEN _("Open with %TOOL%")
#ifdef __WXMSW__
#define STRING_MENU_PROPERTIES    _("Prope&rties\t[")
#endif
#define STRING_MENU_METADATA   _("M&etadata\t]")
#define STRING_MENU_SUMMARY    _("S&ummary\t`")
#define STRING_MENU_STATUSBAR  _("Status &Bar\tCtrl+B")
#define STRING_MENU_INTERRUPT  _("&Interrupt\tCtrl+I")
#define STRING_MENU_RELOAD     _("&Reload\tF5")
#define STRING_MENU_MINIMIZE   _("Minimi&ze\tCtrl+Z")
#define STRING_MENU_RESTORE    _("&Restore")
#define STRING_MENU_EXIT       _("&Quit\tCtrl+Q")

#define STRING_MENU_EDIT       _("&Edit")
#define STRING_MENU_ROTATE_CCW _("Rotate -90\tAlt+LEFT")
#define STRING_MENU_ROTATE_CW  _("Rotate +90\tAlt+RIGHT")
#define STRING_MENU_RESCALE    _("&Scale")

#define STRING_MENU_TRAVERSE   _("&Traverse")
#define STRING_MENU_GOHOME     _("Go &Home\tAlt+HOME")
#define STRING_MENU_GOFAVORITE _("Fa&vorite\tAlt+END")
#define STRING_MENU_PREV       _("&Previous in Directory")
#define STRING_MENU_NEXT       _("&Next in Directory")
#define STRING_MENU_HOME       _("&First in Directory")
#define STRING_MENU_END        _("La&st in Directory")
#define STRING_MENU_UP         _("Move &Up in Tree")
#define STRING_MENU_DOWN       _("Move &Down in Tree")
#define STRING_MENU_LEFT       _("Move &Left in Tree")
#define STRING_MENU_RIGHT      _("Move &Right in Tree")
#define STRING_MENU_BACK       _("Back in History\tCtrl+UP")
#define STRING_MENU_FORWARD    _("Forward in History\tCtrl+DOWN")
#define STRING_MENU_BRANCH     _("Jump to Branch in History\tCtrl+BACK")

#define STRING_MENU_VIEW       _("&View")
#define STRING_MENU_FULLSCREEN _("F&ull Screen\tENTER")
#define STRING_MENU_ZOOM_IN    _("Zoom &In\t+")
#define STRING_MENU_ZOOM_OUT   _("Zoom &Out\t-")
#define STRING_MENU_ZOOM_100   _("Zoom &100%\t*")
#define STRING_MENU_ZOOM_FIT   _("Zoom &Fit\t/")
#define STRING_MENU_ZOOM_EXT   _("Zoom &Extend\t\\")
#define STRING_MENU_OPAQUE     _("Opa&que")
#define STRING_MENU_SLIDESHOW_START    _("&Slide Show\tSPACE")
#define STRING_MENU_SLIDESHOW_PAUSE    _("Pause &Slide Show\tSPACE")
#define STRING_MENU_SLIDESHOW_CONTINUE _("Continue &Slide Show\tSPACE")

#define STRING_MENU_PANEL       _("&Panel")
#define STRING_MENU_FILESYSTEM  _("&Filesystem\tF10")
#define STRING_MENU_ARCHIVE     _("&Archive\tF11")
#define STRING_MENU_THUMBNAIL   _("&Thumbnail\tF12")

#define STRING_MENU_MANAGE      _("&Manage")
#define STRING_MENU_MKDIR       _("&New Directory")
#define STRING_MENU_RENAME      _("&Rename File\tF2")
#define STRING_MENU_FILE_CUT    _("Cu&t File\tCtrl+X")
#define STRING_MENU_FILE_COPY   _("&Copy File\tCtrl+C")
#define STRING_MENU_FILE_PASTE  _("&Paste File\tCtrl+V")
#define STRING_MENU_FILE_DELETE _("&Delete File\tDEL")
#define STRING_MENU_FILE_MOVETO _("&Move File To...")
#define STRING_MENU_FILE_COPYTO _("Copy File &To...")

#define STRING_MENU_TOOLS             _("Tool&s")
#define STRING_MENU_RESCALE_MANAGER   _("&Rescale Manager")
#define STRING_MENU_PICASAWEB_MANAGER _("&PicasaWeb Manager")

#define STRING_MENU_OPTION      _("&Option")
#define STRING_MENU_SHELL       _("&Shell Integration")
#define STRING_MENU_ASSOCIATION _("File &Association")
#define STRING_MENU_OPTION_DESKTOP     _("&Desktop")
#define STRING_MENU_DESKTOP_WALLPAPER  _("Set as &Wallpaper")
#define STRING_MENU_DESKTOP_NONEWP     _("&None Wallpaper")
#define STRING_MENU_DESKTOP_BACKGROUND _("&Background Color")
#define STRING_MENU_LANGUAGE    _("&Language")
#define STRING_MENU_OPTION_TRAVERSE _("&Traverse")
#define STRING_MENU_FAVORITE        _("&Favorite")
#define STRING_MENU_OPTION_VIEW _("&View")
#define STRING_MENU_FIXED_STYLE   _("Fixed &Style")
#define STRING_MENU_FIXED_AUTOFIT _("&Auto Fit")
#define STRING_MENU_FIXED_BESTFIT _("Zoom &Fit")
#define STRING_MENU_FIXED_EXTEND  _("Zoom &Extend")
#define STRING_MENU_FIXED_SCALE   _("Fixed &Scale")
#define STRING_MENU_FIXED_ROTATE  _("Fixed &Rotation")
#define STRING_MENU_FIXED_LEFTTOP _("Fixed &LeftTop")
#define STRING_MENU_LOAD_COMPLETELY _("Load &Completely")
#define STRING_MENU_PREFETCH        _("&Prefetch")
#define STRING_MENU_OPTION_RESCALE     _("Scale &Filter")
#define STRING_MENU_RESCALE_BOX        _("Bo&x")
#define STRING_MENU_RESCALE_BICUBIC    _("&Bicubic")
#define STRING_MENU_RESCALE_BILINEAR   _("Bi&linear")
#define STRING_MENU_RESCALE_BSPLINE    _("B-&Spline")
#define STRING_MENU_RESCALE_CATMULLROM _("&Catmull-Rom")
#define STRING_MENU_RESCALE_LANCZOS3   _("&Lanczos")
#define STRING_MENU_OPTION_SLIDESHOW   _("Slide Sho&w")
#define STRING_MENU_SLIDESHOW_REPEAT   _("Repeat")
#define STRING_MENU_SLIDESHOW_INTERVAL _("Interval")
#define STRING_MENU_OPTION_SUMMARY     _("S&ummary")
#define STRING_MENU_SUMMARY_POSITION   _("&Position")
#define STRING_MENU_SUMMARY_COLOR      _("&Color")
#define STRING_MENU_OPTION_MANAGE      _("&Manage")
#define STRING_MENU_MANAGE_DIRSET      _("Set &Directories for Hotkey")
#define STRING_MENU_EXTERNAL_TOOLS     _("&External Tools")
#define STRING_MENU_OPTION_RESCALEMGR    _("&Rescale Manager")
#define STRING_MENU_OPTION_PICASAWEBMGR  _("&PicasaWeb Manager")
#define STRING_MENU_MANAGER_MAXCON     _("&Max Concurrent")
#define STRING_MENU_CONFIG             _("&Generate Config File")

#define STRING_MENU_HELP     _("&Help")
#define STRING_MENU_ABOUT    _("&About")

#define STRING_MENU_THUMBS_DELETE _("&Remove It")
#define STRING_MENU_THUMBS_CLEAR  _("&Clear All")


// -------- dialog string --------
#define STRING_LANGUAGE_FAILED  wxT("Cannot find its mo file!")
#define STRING_LANGUAGE_MESSAGE _("Please select language:")

#define STRING_CONTINUE_COMMON  _("Do you want to continue?")
#define STRING_CONTINUE_MESSAGE _("\nDo you want to skip it and continue?")
#define STRING_CONTINUE_CAPTION _("Confirm")
#define STRING_LOSSLESS_MESSAGE _("Rotate/Clone original file losslessly?")
#define STRING_SAVE_MESSAGE     _("This function is NOT complete!\nFile quality and information may be STRIPPED!!\n")+wxString(STRING_CONTINUE_COMMON)
#define STRING_ORIGINAL_MESSAGE _("The original file will be overwritten!!\n")+wxString(STRING_CONTINUE_COMMON)
#define STRING_LOAD_MESSAGE     _("Current file has been modified,\nreload original file now?")
#define STRING_MKDIR_MESSAGE    _("Please input directory name:")
#define STRING_RENAME_MESSAGE   _("Please input new name:")
#define STRING_DELETE_MESSAGE   _("The file will be deleted!\n")
#define STRING_FAVORITE_MESSAGE _("Favorite is default location on startup, and can be found quickly by clicking Favorite button/menu.\nCurrent: ")
#ifdef __WXMSW__
#define STRING_SHELL_CHOICE_DRIVE     _("Drive")
#define STRING_SHELL_CHOICE_DIRECTORY _("Directory")
#define STRING_SHELL_CHOICE_FILE      _("File")
#define STRING_SHELL_MESSAGE _("Please select where to add\n\"Browse with kuView\" entry:\n(ContextMenu on right click)")
#define STRING_SHELL_BROWSE  _("Browse with kuView")
#define STRING_ASSOCIATION_MESSAGE _("Please select file extension:")
#define STRING_WALLPAPER_CHOICE_CENTER  _("Center")
#define STRING_WALLPAPER_CHOICE_TILE    _("Tile")
#define STRING_WALLPAPER_CHOICE_STRETCH _("Stretch")
#define STRING_WALLPAPER_CHOICE_BESTFIT _("Scale to Fit")
#define STRING_WALLPAPER_CHOICE_EXTEND  _("Scale to Extend")
#define STRING_WALLPAPER_MESSAGE        _("Please select how to set wallpaper:")
#define STRING_NONEWP_MESSAGE           _("Do you want to disable wallpaper?")
#endif
#define STRING_OPAQUE_MESSAGE    _("Please input a number between 0 and 255:\n(0 is fully transparent, and 255 is fully opaque)")
#define STRING_OPAQUE_PROMPT     _("opaque value:")
#define STRING_SLIDESHOW_MESSAGE _("Please input interval in second:")
#define STRING_SLIDESHOW_CAPTION _("Interval")
#define STRING_MANAGER_MAXCON_MESSAGE _("Please input the maximum number of concurrent tasks:")
#define STRING_BUTTON_CLOSE   _("&Close")
#define STRING_BUTTON_PRINT   _("&Print...")
#define STRING_BUTTON_ALIGN   _("&Align...")
#define STRING_BUTTON_SCALE   _("&Scale...")
#define STRING_ALIGN_MESSAGE  _("Please select the position button for alignment:")
#define STRING_ALIGN_CENTER   _("Center")
#define STRING_ALIGN_LEFT     _("Left")
#define STRING_ALIGN_RIGHT    _("Right")
#define STRING_ALIGN_TOP      _("Top")
#define STRING_ALIGN_BOTTOM   _("Bottom")
#define STRING_RESCALE_MESSAGE  _("Please enter the scale ratio:")
#define STRING_RESCALE_ERROR    _("It is not a valid number!\nPlease try again...")
#define STRING_POSITION_LEFT  _("Left")
#define STRING_POSITION_TOP   _("Top")
#define STRING_DIRSET_MESSAGE _("Current file can be copied or moved to specified directories by hotkey:\n    Ctrl + NUM\tcopy to directory NUM\n    Alt + NUM\tmove to directory NUM\nPlease add favorite directories into the list: (the maximum is 9)")
#define STRING_CONFIG_SUCCEED _("Config file is generated successfully. It will be imported automatically on startup.")
#define STRING_CONFIG_FAILED  _("Failed to generate config file. Please check working directory: ")
#define STRING_ALBUM_DELETE_MESSAGE _("The album will be deleted!\n")+wxString(STRING_CONTINUE_COMMON)


// -------- for stock id --------
#define STRING_STOCK_ADD     _("Add")
#define STRING_STOCK_DELETE  _("&Delete")
#define STRING_STOCK_REPLACE _("Rep&lace")
#define STRING_STOCK_OK      _("&OK")
#define STRING_STOCK_CANCEL  _("&Cancel")


// -------- PicasaWeb --------
#define STRING_PICASAWEB_PROP_TITLE       wxT("title")
#define STRING_PICASAWEB_PROP_SUMMARY     wxT("summary")
#define STRING_PICASAWEB_PROP_LOCATION    wxT("gphoto:location")
#define STRING_PICASAWEB_PROP_ACCESS      wxT("gphoto:access")
#define STRING_PICASAWEB_ACCESS_PUBLIC    wxT("public")
#define STRING_PICASAWEB_ACCESS_PRIVATE   wxT("private")
#define STRING_PICASAWEB_ACCESS_PROTECTED wxT("protected")


// -------- registry --------
#ifdef __WXMSW__
#define STRING_REG_SHORTNAME wxT("kuView")
#define STRING_REG_COMMAND   wxT("command")
#define STRING_REG_OPEN      wxT("open")
#define STRING_REG_ICON      wxT("DefaultIcon")
#define STRING_REG_BACKUP    wxT("Backup")
#define STRING_REG_SHELLCMD  wxT("shell\\open\\command")
#define STRING_REG_MAIN      wxT("HKEY_CLASSES_ROOT\\kuView")
#define STRING_REG_CLASSEXT  wxT("HKEY_CLASSES_ROOT\\")
#define STRING_REG_USEREXT   wxT("HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\FileExts\\")
#define STRING_REG_APP       wxT("HKEY_CLASSES_ROOT\\Applications\\kuview.exe")
#define STRING_REG_DRIVE     wxT("HKEY_CLASSES_ROOT\\Drive\\shell\\kuView")
#define STRING_REG_DIRECTORY wxT("HKEY_CLASSES_ROOT\\Directory\\shell\\kuView")
#define STRING_REG_FILE      wxT("HKEY_CLASSES_ROOT\\*\\shell\\kuView")
#define STRING_REG_DESKTOP   wxT("HKEY_CURRENT_USER\\Control Panel\\Desktop\\")
#define STRING_REG_MSIE      wxT("HKEY_CURRENT_USER\\Software\\Microsoft\\Internet Explorer\\Desktop\\General\\")
#define STRING_REG_THEME     wxT("HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\LastTheme\\")
#define STRING_REG_WP_MAIN      wxT("Wallpaper")
#define STRING_REG_WP_STYLE     wxT("WallpaperStyle")
#define STRING_REG_WP_TILE      wxT("TileWallpaper")
#define STRING_REG_WP_BACKUP    wxT("BackupWallpaper")
#define STRING_REG_WP_CONVERTED wxT("ConvertedWallpaper")
#define STRING_REG_WP_ORIGINAL  wxT("OriginalWallpaper")
#define STRING_REG_WP_SOURCE    wxT("WallpaperSource")


// -------- wallpaper --------
//#define STRING_WALLPAPER_BMPFILE        wxT("\\Local Settings\\Application Data\\Microsoft\\Wallpaper1.bmp")
#define STRING_WALLPAPER_BMPFILE        wxT("\\.kuView_Wallpaper.bmp")
#define STRING_WALLPAPER_UPDATE_COMMAND wxT("\\System32\\RUNDLL32.EXE user32.dll UpdatePerUserSystemParameters 1 True")
#endif


// -------- shortcut --------
//#define STRING_SHORTCUT_HOME _("Home Directory")


// -------- filter --------
#define STRING_FILTER_STANDARD         wxT("Image Files")
#define STRING_FILTER_ARCHIVE          wxT("|Archive Files(*.zip;*.tar;*.gz)|*.zip;*.tar;*.gz")
#define STRING_FILTER_ALLFILES         wxT("|All Files(*.*)|*.*")

#define INDEX_FILTER_STANDARD (0)
#define INDEX_FILTER_ARCHIVE  (1)
#define INDEX_FILTER_ALLFILES (2)


// -------- metadata --------
#define STRING_METADATA_COMMENTS  _("Comments")         // 0, single comment or keywords
#define STRING_METADATA_MAIN      _("Main")             // 1, Exif-TIFF metadata
#define STRING_METADATA_EXIF      _("Exif")             // 2, Exif-specific metadata
#define STRING_METADATA_GPS       _("GPS")              // 3, Exif GPS metadata
#define STRING_METADATA_MAKERNOTE _("Maker Note")       // 4, Exif maker note metadata
#define STRING_METADATA_INTEROP   _("Interoperability") // 5, Exif interoperability metadata
#define STRING_METADATA_IPTC      _("IPTC")             // 6, IPTC/NAA metadata
#define STRING_METADATA_XMP       _("XMP")              // 7, Abobe XMP metadata
#define STRING_METADATA_GEOTIFF   _("GeoTIFF")          // 8, GeoTIFF metadata
#define STRING_METADATA_ANIMATION _("Animation")        // 9, Animation metadata
#define STRING_METADATA_LABEL_KEY         _("Tag")
#define STRING_METADATA_LABEL_VALUE       _("Value")
#define STRING_METADATA_LABEL_DESCRIPTION _("Description")

#define METADATA_TAG_SEP wxT("\n")
#define METADATA_COL_SEP wxT("\t")

// -------- macro for thread --------
#define THREAD_NAME_MAIN    wxT("[main]")
#define THREAD_NAME_WORKER  wxT("[worker]")
#define THREAD_NAME_CURRENT (wxThread::IsMain() ? THREAD_NAME_MAIN : THREAD_NAME_WORKER)

// -------- constant for thread --------
#define THREAD_ERROR_NONE        (0)
#define THREAD_ERROR_DISKFULL    (1)
#define THREAD_ERROR_READONLY    (2)
#define THREAD_ERROR_READFAIL    (3)
#define THREAD_ERROR_RESCALEFAIL (4)
#define THREAD_ERROR_UNKNOWN     (5)

#define THREAD_WHENEXISTS_RENAME    (0)
#define THREAD_WHENEXISTS_OVERWRITE (1)
#define THREAD_WHENEXISTS_SKIP      (2)

#define THREAD_ACTION_UPLOAD    (0)
#define THREAD_ACTION_DOWNLOAD  (1)
#define THREAD_ACTION_DELETE    (2)


// -------- task manager --------
#define STRING_MANAGER_SRC         _("Source")
#define STRING_MANAGER_DEST        _("Destination")
#define STRING_MANAGER_PROGRESS    _("Progress")
#define STRING_MANAGER_STATUS      _("Status")
#define STRING_MANAGER_SIZE        _("Size")
#define STRING_MANAGER_FORMAT      _("Format")
#define STRING_MANAGER_FILTER      _("Filter")
#define STRING_MANAGER_EXISTS      _("When Filename Exists")
#define STRING_MANAGER_RENAME      _("Auto Rename")
#define STRING_MANAGER_OVERWRITE   _("Overwrite")
#define STRING_MANAGER_SKIP        _("Skip")
#define STRING_MANAGER_CUSTOM      _("Custom...")
#define STRING_MANAGER_ACTION      _("Action")
#define STRING_MANAGER_UPLOAD      _("Upload")
#define STRING_MANAGER_DOWNLOAD    _("Download")
#define STRING_MANAGER_DELETE      _("Delete")
#define STRING_MANAGER_ACCOUNT     _("Account")
#define STRING_MANAGER_PASSWD      _("Password")
#define STRING_MANAGER_LOCAL       _("Local")
#define STRING_MANAGER_REMOTE      _("Remote")
#define STRING_MANAGER_LOGIN       _("Login")
#define STRING_MANAGER_ANONYMOUS   _("Anonymous")
#define STRING_MANAGER_OWNER       _("Owner")
#define STRING_MANAGER_ALBUM       _("Album")
#define STRING_MANAGER_QUERY       _("Query")
#define STRING_MANAGER_NEW         _("New")
#define STRING_MANAGER_MKDIR       _("Create a subdirectory for the album")
#define STRING_MANAGER_TITLE       _("Title")
#define STRING_MANAGER_LOCATION    _("Location")
#define STRING_MANAGER_ACCESS      _("Access")
#define STRING_MANAGER_PUBLIC      _("Public")
#define STRING_MANAGER_PRIVATE     _("Private")
#define STRING_MANAGER_PROTECTED   _("Protected")

#define STRING_THREAD_RUNNING    _("Running")
#define STRING_THREAD_QUEUING    _("Queuing")
#define STRING_THREAD_PAUSED     _("Paused")
#define STRING_THREAD_CANCELLED  _("Cancelled")
#define STRING_THREAD_FAILED     _("Failed")
#define STRING_THREAD_INCOMPLETE _("Incomplete")
#define STRING_THREAD_COMPLETED  _("Completed")
#define STRING_THREADERR_DISKFULL    _("Not enough space on destination.")
#define STRING_THREADERR_READONLY    _("Destination is not writable.")
#define STRING_THREADERR_READFAIL    _("Some files cannot be read.")
#define STRING_THREADERR_RESCALEFAIL _("Some files cannot be rescaled.")
#define STRING_THREADERR_UNKNOWN     _("Failed to write on destination.")
#define STRING_TASKBTN_NEW      _("&New Task")
#define STRING_TASKBTN_PAUSE    _("&Pause Task")
#define STRING_TASKBTN_RESUME   _("&Resume Task")
#define STRING_TASKBTN_CANCEL   _("&Cancel Task")
#define STRING_TASKBTN_REMOVE   _("&Remove Task")
#define STRING_SELBTN_ALL     _("&All")
#define STRING_SELBTN_NONE    _("&None")
#define STRING_SELBTN_INVERSE _("&Inverse")


// -------- command interface --------
#define CMD_SEP       wxT('|')
#define CMD_EXPLORE   wxT("explore")    // for idle task only
#define CMD_LOCATE    wxT("locate")
#define CMD_SLIDESHOW wxT("slideshow")
#define CMD_REG       wxT("reg")
#define CMD_UNREG     wxT("unreg")
#define CMD_RESCALE   wxT("rescale")


// -------- external tools --------
#ifdef __WXMSW__
#define EXTERNAL_OPERATION_EDIT wxT("edit")
#define EXTERNAL_OPERATION_OPEN wxT("open")
#define STRING_DEFAULT_EXTERNAL_OPEN_VIEW _("Picture and Fax Viewer")
#else
#define STRING_DEFAULT_EXTERNAL_OPEN_VIEW _("Default Viewer")
#endif
#define STRING_DEFAULT_EXTERNAL_OPEN_MAPS _("Google Maps")
#define EXTERNAL_VAR_FILE      wxT("%f")
#define EXTERNAL_VAR_DIR       wxT("%d")
#define EXTERNAL_VAR_LATITUDE  wxT("%la")
#define EXTERNAL_VAR_LONGITUDE wxT("%lo")


// -------- config text --------
#define CFG_GROUP_GENERAL         wxT("General")
#define CFG_GENERAL_LANGUAGE      wxT("Language")
#define CFG_GROUP_TRAVERSE        wxT("Traverse")
#define CFG_TRAVERSE_FAVORITE     wxT("Favorite")
#define CFG_GROUP_VIEW            wxT("View")
#define CFG_VIEW_KEEPSTYLE        wxT("FixedStyle")
#define CFG_VIEW_SCALE            wxT("Scale")
#define CFG_VIEW_KEEPROTATE       wxT("FixedRotation")
#define CFG_VIEW_ROTATE           wxT("Rotation")
#define CFG_VIEW_KEEPLEFTTOP      wxT("FixedLeftTop")
#define CFG_VIEW_LEFTTOP          wxT("LeftTop")
#define CFG_VIEW_LOAD_COMPLETELY  wxT("LoadCompletely")
#define CFG_VIEW_PREFETCH         wxT("Prefetch")
#define CFG_VIEW_RESCALE          wxT("ScaleFilter")
#define CFG_GROUP_SLIDESHOW       wxT("SlideShow")
#define CFG_SLIDESHOW_REPEAT      wxT("Repeat")
#define CFG_SLIDESHOW_INTERVAL    wxT("Interval")
#define CFG_GROUP_SUMMARY         wxT("Summary")
#define CFG_SUMMARY_POSITION      wxT("Position")
#define CFG_SUMMARY_COLOR         wxT("Color")
#define CFG_GROUP_MANAGE          wxT("Manage")
#define CFG_MANAGE_DIRSET         wxT("DirSet_%d")
#define CFG_GROUP_EXTERNAL        wxT("ExternalTool_%d")
#define CFG_EXTERNAL_NAME         wxT("Name")
#define CFG_EXTERNAL_EXEC         wxT("Exec")
#define CFG_EXTERNAL_ARGS         wxT("Args")
#define CFG_GROUP_RESCALEMGR      wxT("RescaleManager")
#define CFG_GROUP_PICASAWEBMGR    wxT("PicasaWebManager")
#define CFG_MANAGER_MAXCON        wxT("MaxConcurrent")
#define CFG_FORMAT_POINT          wxT("%d,%d")


// -------- uninitialized value --------
#define INVALID_LATITUDE  (100.0)
#define INVALID_LONGITUDE (200.0)


// -------- id --------
enum {
    kuID_LOWEST=wxID_HIGHEST,
    // menu file
    kuID_FILE,
    kuID_SAVE,
    kuID_SAVEAS,
    kuID_PAGESETUP,
    kuID_PREVIEW,
    kuID_PRINT,
    kuID_EXPLORE,
    #ifdef __WXMSW__
    kuID_EXTERNAL_EDIT,
    #endif
    kuID_EXTERNAL_OPEN,
    #ifdef __WXMSW__
    kuID_PROPERTIES,
    #endif
    kuID_METADATA,
    kuID_INTERRUPT,
    kuID_RELOAD,
    kuID_ESCAPE,
    kuID_MINIMIZE,
    kuID_RESTORE,
    // menu edit
    kuID_EDIT,
    kuID_ROTATE_CCW,
    kuID_ROTATE_CW,
    kuID_RESCALE,
    // menu traverse
    kuID_TRAVERSE,
    kuID_GOHOME,
    kuID_GOFAVORITE,
    kuID_PREV,
    kuID_NEXT,
    kuID_HOME,
    kuID_END,
    kuID_UP,
    kuID_DOWN,
    kuID_LEFT,
    kuID_RIGHT,
    kuID_BACK,
    kuID_FORWARD,
    kuID_BRANCH,
    // menu view
    kuID_VIEW,
    kuID_FULLSCREEN,
    kuID_ZOOM_IN,
    kuID_ZOOM_OUT,
    kuID_ZOOM_100,
    kuID_ZOOM_FIT,
    kuID_ZOOM_EXT,
    kuID_ZOOM_AUTO,
    kuID_OPAQUE,
    kuID_SLIDESHOW,
    // menu panel
    kuID_PANEL,
    kuID_FILESYSTEM,
    kuID_ARCHIVE,
    kuID_THUMBNAIL,
    // menu manage
    kuID_MANAGE,
    kuID_MKDIR,
    kuID_RENAME,
    kuID_FILE_CUT,
    kuID_FILE_COPY,
    kuID_FILE_PASTE,
    kuID_FILE_DELETE,
    kuID_FILE_MOVETO,
    kuID_FILE_COPYTO,
    // menu tools
    kuID_RESCALE_MANAGER,
    kuID_PICASAWEB_MANAGER,
    // menu option
    kuID_OPTION,
    kuID_SHELL,
    kuID_ASSOCIATION,
    kuID_WALLPAPER,
    kuID_NONEWP,
    kuID_BACKGROUND,
    kuID_LANGUAGE,
    kuID_OPTION_TRAVERSE,
    kuID_FAVORITE,
    kuID_OPTION_VIEW,
    kuID_FIXED_AUTOFIT,
    kuID_FIXED_BESTFIT,
    kuID_FIXED_EXTEND,
    kuID_FIXED_SCALE,
    kuID_FIXED_ROTATE,
    kuID_FIXED_LEFTTOP,
    kuID_LOAD_COMPLETELY,
    kuID_PREFETCH,
    kuID_OPTION_RESCALE,
    kuID_RESCALE_BOX,
    kuID_RESCALE_BICUBIC,
    kuID_RESCALE_BILINEAR,
    kuID_RESCALE_BSPLINE,
    kuID_RESCALE_CATMULLROM,
    kuID_RESCALE_LANCZOS3,
    kuID_SLIDESHOW_REPEAT,
    kuID_SLIDESHOW_INTERVAL,
    kuID_SLIDESHOW_CURRENT,
    kuID_SLIDESHOW_SIBLING,
    kuID_SUMMARY_POSITION,
    kuID_SUMMARY_COLOR,
    kuID_MANAGE_DIRSET,
    kuID_EXTERNAL_TOOLS,
    kuID_RESCALEMGR_MAXCON,
    kuID_PICASAWEBMGR_MAXCON,
    kuID_CONFIG,
    kuID_TIMER_IDLE,
    // other
    kuID_LOCATE,
    kuID_STATUSBAR,
    kuID_SUMMARY,
    // manager
    kuID_TASK_NEW,
    kuID_TASK_PAUSE,
    kuID_TASK_CANCEL,
    kuID_TASK_REMOVE,
    // thread
    kuID_THREAD_LOWEST,
    kuID_THREAD_PROGRESS,
    kuID_THREAD_STATUS,
    kuID_THREAD_HIGHEST,
    kuID_HIGHEST,
    // dir hotkey
    kuID_COPYTO_1,
    kuID_COPYTO_2,
    kuID_COPYTO_3,
    kuID_COPYTO_4,
    kuID_COPYTO_5,
    kuID_COPYTO_6,
    kuID_COPYTO_7,
    kuID_COPYTO_8,
    kuID_COPYTO_9,
    kuID_MOVETO_1,
    kuID_MOVETO_2,
    kuID_MOVETO_3,
    kuID_MOVETO_4,
    kuID_MOVETO_5,
    kuID_MOVETO_6,
    kuID_MOVETO_7,
    kuID_MOVETO_8,
    kuID_MOVETO_9,
    kuID_EXTERNAL_START
};


// -------- class --------
class kuFrame;

// kuGenericDirCtrl
class kuGenericDirCtrl: public wxGenericDirCtrl {
private:
    kuFrame* mFrame;
    bool     mIsUNC;
    void OnTreeSelChanged(wxTreeEvent& event);
    void OnTreeBeginDrag(wxTreeEvent& event);
    void OnSetFocus(wxFocusEvent& event);
    void OnChoice(wxCommandEvent& event);
    void OnContextMenu(wxContextMenuEvent& event);
    void PrefetchSiblings();
public:
    kuGenericDirCtrl(wxWindow* parent, kuFrame* frame);
    void           SetupPopupMenu();
    bool           Locate(wxString location);
    wxArrayString* EnumerateChildren(wxString dirname);
    wxString       GetNeighbor();
    wxString       GetDir();
    void           Reload(bool parent);
    void SwitchFilter(int index);
    void AddShortcuts();
    void DoDragDrop(wxTreeEvent& event);
    wxMenu* mMenu;
DECLARE_EVENT_TABLE()
};

// kuVirtualDirCtrl
class kuVirtualDirCtrl: public wxTreeCtrl {
private:
    kuFrame*     mFrame;
    wxString     mArchive;
    bool         mIsDeleting;
    void OnTreeSelChanged(wxTreeEvent& event);
    //void OnTreeBeginDrag(wxTreeEvent& event);
    void EnumerateArchive(wxString archive);
    bool IsDir(wxTreeItemId id);
    void PrefetchSiblings();
public:
    kuVirtualDirCtrl(wxWindow* parent, kuFrame* frame);
    void           SetRoot(wxString archive);
    wxTreeItemId   FindTextInChildren(wxTreeItemId pid, wxString& text);
    bool           Locate(wxString location);
    wxString       GetFilePath(bool isurl, bool fileonly, wxTreeItemId id=0);
    wxArrayString* EnumerateChildren(wxString dirname);
    //void           DoDragDrop();
DECLARE_EVENT_TABLE()
};

// kuSingleScrolled
class kuSingleScrolled: public wxScrolledWindow {
private:
    kuFrame*  mFrame;
    wxImage   mDispImg;
    FIBITMAP* mOrigBmp;
    wxSize    mOrigSize;
    wxSize    mDispSize;
    wxRect    mCropRect;
    wxRect    mView;
    wxPoint   mDragStart;
    wxPoint   mLeftTop;
    wxPoint   mRightBottom;
    double    mScale;
    wxString  mFilename;
    bool      mIsUrl;
    wxTimer   mIdleTimer;
    wxRect    mSummaryRect;
    void OnSize(wxSizeEvent& event);
    void OnMotion(wxMouseEvent& event);
    void OnLeaveWindow(wxMouseEvent& event);
    void OnLeftDown(wxMouseEvent& event);
    void OnLeftUp(wxMouseEvent& event);
    void CheckViewSize();
    void CheckViewStart();
    wxString GetBmpSizeString(int fast);
    void     RestoreLeftTop(wxPoint lefttop);
    void OnMousewheel(wxMouseEvent& event);
    void OnContextMenu(wxContextMenuEvent& event);
    void OnIdleTimer(wxTimerEvent& event);
    void OnKeyDown(wxKeyEvent& event);
    bool KeyAction(int keycode);
public:
    kuSingleScrolled(wxWindow* parent, kuFrame* frame);
    ~kuSingleScrolled();
    void           SetupPopupMenu();
    void           ClearExternalOpenMenu();
    void           SetupExternalOpenMenu();
    virtual void   OnDraw(wxDC& dc);
    void           DrawSummary(wxDC& dc);
    void           ClearSummary();
    bool           DrawCropRect(bool loaded=false);
    void           ZoomSelectedArea(wxPoint start, wxPoint stop, wxPoint offset);
    void           ReloadImage(wxString filename, bool isurl, bool loaded=false);
    bool           SaveImage(wxString filename, bool ask=true);
    wxString       GetFilename();
    wxSize         GetOrigSize(bool real=false);
    FIBITMAP*      GetOrigBmp();
    void Rotate90(bool cw);
    void SetFullScreen(bool full);
    bool SetScale(int diff, bool refresh=true);
    bool Rescale(double scale);
    void HideCursor(bool hide=true);
    void DoDragDrop();
    wxMenu* mMenu;
DECLARE_EVENT_TABLE()
};

// kuMultipleScrolled
class kuMultipleScrolled: public wxScrolledWindow {
private:
    kuFrame*  mFrame;
    wxMenu*   mMenu;
    wxString  mDirname;
    wxArrayString mFilenames;
    wxArrayString mSelections;
    void OnKeyDown(wxKeyEvent& event);
    void OnLeftDown(wxMouseEvent& event);
    void OnSize(wxSizeEvent& event);
    void OnContextMenu(wxContextMenuEvent& event);
    void OnDeleteThumb(wxCommandEvent& event);
    void OnClearThumbs(wxCommandEvent& event);
    void OnAll(wxCommandEvent& event);
    void OnNone(wxCommandEvent& event);
    void OnInverse(wxCommandEvent& event);
public:
    kuMultipleScrolled(wxWindow* parent, kuFrame* frame);
    void SetupPopupMenu();
    void ReloadThumbs(wxString dirname, bool isurl);
    void AddThumbs(wxArrayString& files);
    void RemoveThumb(wxString filename=wxEmptyString);
    void Select(wxString filename, bool ctrl, bool shift);
    void AddSelections(int first, int last);
    void RefreshThumbs();
    void DoDragDrop();
DECLARE_EVENT_TABLE()
};

// kuScrollHandler
class kuScrollHandler: public wxEvtHandler {
private:
    kuFrame* mFrame;
    void OnKeyDown(wxKeyEvent& event);
    void OnLeftDclick(wxMouseEvent& event);
    void OnMiddleUp(wxMouseEvent& event);
    void OnEnterWindow(wxMouseEvent& event);
    void OnMenuRange(wxCommandEvent& event);
public:
    kuScrollHandler(kuFrame* frame);
DECLARE_EVENT_TABLE()
};

// kuThumbButton
class kuThumbButton: public wxBitmapButton {
private:
    kuFrame* mFrame;
    wxMenu*  mMenu;
    bool     mIsDragging;
    bool     mIsUrl;
    void OnButton(wxCommandEvent& event);
    void OnMouseWheel(wxMouseEvent& event);
    void OnKeyDown(wxKeyEvent& event);
    void OnEnterWindow(wxMouseEvent& event);
    void OnLeaveWindow(wxMouseEvent& event);
    void OnContextMenu(wxContextMenuEvent& event);
    void OnDelete(wxCommandEvent& event);
public:
    kuThumbButton(wxWindow* parent, wxString filename, wxString url);
    //kuThumbButton(wxWindow* parent, wxInputStream& stream, wxString name);
    void SetupPopupMenu();
DECLARE_EVENT_TABLE()
};

// kuCheckListDialog
class kuCheckListDialog: public wxDialog {
private:
    kuFrame* mFrame;
    wxCheckListBox* mListBox;
public:
    enum {
        kuID_CHECKDLG_ALL = kuID_HIGHEST,
        kuID_CHECKDLG_NONE,
        kuID_CHECKDLG_INVERSE
    };
    kuCheckListDialog(wxWindow* parent, kuFrame* frame, const wxString& message, const wxString& caption, const wxArrayString& choices);
    void OnAll(wxCommandEvent& event);
    void OnNone(wxCommandEvent& event);
    void OnInverse(wxCommandEvent& event);
    wxArrayInt GetSelections();
    void       SetSelections(const wxArrayInt& selections);
    bool       IsChecked(int index);
};

// kuMetaSheetDialog
class kuMetaSheetDialog: public wxPropertySheetDialog {
private:
    kuFrame* mFrame;
public:
    kuMetaSheetDialog(wxWindow* parent, kuFrame* frame, const wxString& filename, const wxArrayString& metadata);
};

// kuPositionDialog
class kuPositionDialog: public wxDialog {
private:
    wxPoint mPosition;
public:
    kuPositionDialog(wxWindow* parent, wxString title, wxSize max, wxPoint current);
    wxPoint GetPosition();
};

class kuPathPanel;
// kuEntryEditorDialog
class kuEntryEditorDialog: public wxDialog {
private:
    wxListBox*    mListBox;
    wxTextCtrl*   mNameTextCtrl;
    kuPathPanel*  mPathPanel;
    wxTextCtrl*   mValue2TextCtrl;
    wxArrayString mNames;
    wxArrayString mValue1s;
    wxArrayString mValue2s;
public:
    kuEntryEditorDialog(wxWindow* parent, wxString title, wxString lname, wxString lvalue1, wxString lvalue2,
                        wxArrayString& names, wxArrayString& value1s, wxArrayString& value2s);
    wxArrayString& GetNames();
    wxArrayString& GetValue1s();
    wxArrayString& GetValue2s();
    void OnEdit(wxCommandEvent& event);
    void OnDelete(wxCommandEvent& event);
    void OnAdd(wxCommandEvent& event);
    void OnReplace(wxCommandEvent& event);
};

// kuDirSetDialog
class kuDirSetDialog: public wxDialog {
private:
    wxListBox*     mListBox;
    wxButton*      mAddBtn;
    wxButton*      mEditBtn;
    wxButton*      mDeleteBtn;
    wxArrayString  mDirs;
public:
    kuDirSetDialog(wxWindow* parent, wxString title, wxArrayString& dirs);
    wxArrayString& GetDirs();
    void OnAdd(wxCommandEvent& event);
    void OnEdit(wxCommandEvent& event);
    void OnDelete(wxCommandEvent& event);
};

// kuManagerDialog related
class kuManagedTask: public wxObject {
public:
    size_t            Current;
    size_t            Failed;
    wxArrayString     Files;
    wxString          Src;
    wxString          Dest;
    wxString          Link;
    wxThread*         Thread;
    bool              Queuing;
    int               Error;
};
WX_DEFINE_ARRAY_PTR(kuManagedTask*, kuManagedArray);

class kuManagerDialog: public wxDialog {
private:
    wxBoxSizer*     mTopSizer;
    wxBoxSizer*     mBtnSizer;
    wxGrid*         mGrid;
    wxBitmapButton* mUpBtn;
    wxBitmapButton* mDownBtn;
    wxButton*       mPauseBtn;
    wxButton*       mCancelBtn;
    wxButton*       mRemoveBtn;
    wxString        mTitle;
    int             mMaxCon;
    int             mMaxCols;
    wxArrayInt      mCustomCols;
    wxArrayString   mCustomHeaders;
protected:
    kuFrame* mFrame;
public:
    kuManagerDialog(kuFrame* frame, int id, wxString title);
    int  mColSrc;
    int  mColDest;
    int  mColProgress;
    int  mColStatus;
    bool Set(int maxCon, wxArrayInt& defCols, wxArrayInt& customCols, wxArrayString& customHeaders);
    void SetupHeaders();
    void SetupButtons();
    void SetupTitleAndSize();
    virtual bool Destroy();
    void OnUp(wxCommandEvent& event);
    void OnDown(wxCommandEvent& event);
    void OnNew(wxCommandEvent& event);
    void OnPause(wxCommandEvent& event);
    void OnCancel(wxCommandEvent& event);
    void OnRemove(wxCommandEvent& event);
    void OnClose(wxCommandEvent& event);
    void OnGridSelectCell(wxGridEvent& event);
    void OnGridCellLeftDClick(wxGridEvent& event);
    void OnSize(wxSizeEvent& event);
    void AppendRow(kuManagedTask* task);
    void CopyRow(int from, int to);
    void OnUpdateTask(wxCommandEvent& event);
    wxString GetFilterString(int filter);
    wxString GetStatusString(kuManagedTask* task);
    void     SetMoveBtnStatus(bool up, bool down);
    void     SetTaskBtnStatus(bool running, bool paused=false);
    bool     CheckConcurrentRun(int max=0);
    kuManagedArray mTasks;
    virtual kuManagedTask* CreateTask();
    virtual wxThread*      CreateThread(kuManagedTask* task);
    virtual wxString       GetTaskInfo(kuManagedTask* task, int col);
    virtual wxString       GetErrorInfo(int errcode);
};

class kuGridCellProgressRender: public wxGridCellStringRenderer {
private:
    int mColStatus;
public:
    kuGridCellProgressRender(int colStatus);
    void Draw(wxGrid& grid, wxGridCellAttr& attr, wxDC& dc,
              const wxRect& rect, int row, int col, bool isSelected);
};

// kuRescaleMgrDialog related
class kuRescaleTask: public kuManagedTask {
public:
    int               WhenExists;
    wxSize            Size;
    wxString          Format;
    FREE_IMAGE_FILTER Filter;
};

class kuRescaleThread: public wxThread {
private:
    kuRescaleTask* mTask;
    wxEvtHandler*  mHandler;
    void UpdateTask(int item);
public:
    kuRescaleThread(kuRescaleTask* task, wxEvtHandler* handler);
    virtual void* Entry();
    virtual void  OnExit();
    bool SaveToFile(FIBITMAP* bmp, wxString target);
};

class kuRescaleMgrDialog: public kuManagerDialog {
public:
    enum {
        kuID_RESCALEMGR_COL_SRC = 0,
        kuID_RESCALEMGR_COL_DEST,
        kuID_RESCALEMGR_COL_SIZE,
        kuID_RESCALEMGR_COL_FORMAT,
        kuID_RESCALEMGR_COL_FILTER,
        kuID_RESCALEMGR_COL_PROGRESS,
        kuID_RESCALEMGR_COL_STATUS,
    };
    kuRescaleMgrDialog(kuFrame* frame, int id);
    virtual kuManagedTask* CreateTask();
    virtual wxThread*      CreateThread(kuManagedTask* task);
    virtual wxString       GetTaskInfo(kuManagedTask* task, int col);
    virtual wxString       GetErrorInfo(int errcode);
};

class kuRescaleTaskDialog: public wxDialog {
private:
    wxTextCtrl*     mSrcTextCtrl;
    wxCheckListBox* mFileCheckListBox;
    wxComboBox*     mSizeComboBox;
    wxComboBox*     mFormatComboBox;
    wxComboBox*     mFilterComboBox;
    wxTextCtrl*     mDestTextCtrl;
    wxRadioBox*     mExistsRadioBox;
    bool            mDestUpdated;
    int             mPrevSize;
    wxArrayString   mFiles;
public:
    enum {
        kuID_RESCALEDLG_ALL = kuID_HIGHEST,
        kuID_RESCALEDLG_NONE,
        kuID_RESCALEDLG_INVERSE,
        kuID_RESCALEDLG_SIZE,
        kuID_RESCALEDLG_DEST
    };
    kuRescaleTaskDialog(wxWindow* parent, wxWindowID id, wxString title, wxString src, wxString dest);
    void OnSrc(wxCommandEvent& event);
    void OnDest(wxCommandEvent& event);
    void OnAll(wxCommandEvent& event);
    void OnNone(wxCommandEvent& event);
    void OnInverse(wxCommandEvent& event);
    void OnSizeSelected(wxCommandEvent& event);
    void OnDestTextUpdated(wxCommandEvent& event);
    void OnOK(wxCommandEvent& event);
    kuRescaleTask* GetTask();
};

WX_DECLARE_STRING_HASH_MAP(wxString, kuStrHashMap);

#ifdef ENABLE_PICASAWEBMGR
WX_DEFINE_ARRAY_PTR(wxXmlNode*, kuXmlNodeArray);

// kuPicasaWebMgrDialog related
class kuPicasaWebTask: public kuManagedTask {
public:
    ~kuPicasaWebTask() {WX_CLEAR_ARRAY(Photos);};
    int               Action;
    wxString          Login;
    wxString          Token;
    wxXmlNode*        Album;
    kuXmlNodeArray    Photos;
};

class kuPicasaWebThread: public wxThread {
private:
    kuPicasaWebTask* mTask;
    wxEvtHandler*    mHandler;
    void UpdateTask(int item);
public:
    kuPicasaWebThread(kuPicasaWebTask* task, wxEvtHandler* handler);
    virtual void* Entry();
    virtual void  OnExit();
};

class kuPicasaWebMgrDialog: public kuManagerDialog {
private:
    kuStrHashMap mTaskValues;
public:
    enum {
        kuID_PICASAWEBMGR_COL_ACTION = 0,
        kuID_PICASAWEBMGR_COL_LOGIN,
        kuID_PICASAWEBMGR_COL_SRC,
        kuID_PICASAWEBMGR_COL_DEST,
        kuID_PICASAWEBMGR_COL_PROGRESS,
        kuID_PICASAWEBMGR_COL_STATUS,
    };
    kuPicasaWebMgrDialog(kuFrame* frame, int id);
    virtual kuManagedTask* CreateTask();
    virtual wxThread*      CreateThread(kuManagedTask* task);
    virtual wxString       GetTaskInfo(kuManagedTask* task, int col);
    virtual wxString       GetErrorInfo(int errcode);
};

class kuPicasaWebTaskDialog: public wxDialog {
private:
    wxTextCtrl*     mLocalTextCtrl;
    wxCheckListBox* mFileCheckListBox;
    wxBoxSizer*     mLSelSizer;
    wxTextCtrl*     mAccountTextCtrl;
    wxTextCtrl*     mPasswdTextCtrl;
    wxButton*       mLoginButton;
    wxCheckBox*     mAnonymousCheckBox;
    wxTextCtrl*     mOwnerTextCtrl;
    wxButton*       mQueryButton;
    wxBoxSizer*     mABtnSizer;
    wxComboBox*     mAlbumComboBox;
    wxCheckListBox* mPhotoCheckListBox;
    wxBoxSizer*     mRSelSizer;
    wxCheckBox*     mMkdirCheckBox;
    int             mAction;
    wxArrayString   mFiles;
    wxString        mToken;
    kuXmlNodeArray  mAlbums;
    kuXmlNodeArray  mPhotos;
public:
    enum {
        kuID_PICASAWEBDLG_LSEL_ALL = kuID_HIGHEST,
        kuID_PICASAWEBDLG_LSEL_NONE,
        kuID_PICASAWEBDLG_LSEL_INVERSE,
        kuID_PICASAWEBDLG_RSEL_ALL,
        kuID_PICASAWEBDLG_RSEL_NONE,
        kuID_PICASAWEBDLG_RSEL_INVERSE,
        kuID_PICASAWEBDLG_ACTION,
        kuID_PICASAWEBDLG_LOGIN,
        kuID_PICASAWEBDLG_ANONYMOUS,
        kuID_PICASAWEBDLG_QUERY,
        kuID_PICASAWEBDLG_ALBUM,
        kuID_PICASAWEBDLG_MKDIR,
    };
    kuPicasaWebTaskDialog(wxWindow* parent, wxWindowID id, wxString title, kuStrHashMap& values);
    ~kuPicasaWebTaskDialog();
    void OnLocal(wxCommandEvent& event);
    void OnLSelAll(wxCommandEvent& event);
    void OnLSelNone(wxCommandEvent& event);
    void OnLSelInverse(wxCommandEvent& event);
    void OnAction(wxCommandEvent& event);
    void OnLogin(wxCommandEvent& event);
    void OnAnonymous(wxCommandEvent& event);
    void OnQuery(wxCommandEvent& event);
    void OnAlbum(wxCommandEvent& event);
    void OnNewAlbum(wxCommandEvent& event);
    void OnEditAlbum(wxCommandEvent& event);
    void OnDeleteAlbum(wxCommandEvent& event);
    void OnRSelAll(wxCommandEvent& event);
    void OnRSelNone(wxCommandEvent& event);
    void OnRSelInverse(wxCommandEvent& event);
    void OnOK(wxCommandEvent& event);
    kuPicasaWebTask* GetTask();
    void             GetValues(kuStrHashMap& values);
};

class kuPicasaWebAlbumDialog: public wxDialog {
private:
    wxTextCtrl*   mTitleTextCtrl;
    wxTextCtrl*   mSummaryTextCtrl;
    wxTextCtrl*   mLocationTextCtrl;
    wxComboBox*   mAccessComboBox;
    wxArrayString mAccessStrs;
public:
    kuPicasaWebAlbumDialog(wxWindow* parent, wxWindowID id, wxString title, kuStrHashMap& values);
    void OnText(wxCommandEvent& event);
    void OnOK(wxCommandEvent& event);
    kuStrHashMap* GetValues();
};
#endif

// kuPrintout
class kuPrintout: public wxPrintout {
private:
    kuSingleScrolled*      mSingle;
    wxPageSetupDialogData* mPageSetupDlgData;
    FIBITMAP* mPrintBmp;
    wxImage   mPrintImg;
public:
    kuPrintout(kuSingleScrolled* single, wxPageSetupDialogData* data);
    ~kuPrintout();
    bool OnPrintPage(int page);
    bool HasPage(int page);
    bool OnBeginDocument(int startPage, int endPage);
    void GetPageInfo(int *minPage, int *maxPage, int *selPageFrom, int *selPageTo);
    void DrawPage();
};

// kuPreviewFrame
class kuPreviewFrame: public wxPreviewFrame {
private:
public:
    kuPreviewFrame(wxPrintPreview* preview, wxWindow* parent, const wxString& title);
    void CreateControlBar();
};

// kuPreviewControlBar
class kuPreviewControlBar: public wxPreviewControlBar {
private:
    wxButton* m_alignButton;
    wxButton* m_scaleButton;
    enum {
        kuID_PREVIEW_BTNALIGN = wxID_PREVIEW_GOTO + 1,
        kuID_PREVIEW_BTNSCALE
    };
public:
    kuPreviewControlBar(wxPrintPreviewBase *preview, long buttons, wxWindow *parent, const wxPoint& pos, const wxSize& size);
    void CreateButtons();
    void OnAlign(wxCommandEvent& event);
    void OnScale(wxCommandEvent& event);
    void UpdatePreview();
};

// kuAlignSelector
class kuAlignSelector: public wxDialog {
private:
    wxStaticText* mSelect;
    int           mAlign;
public:
    kuAlignSelector(wxWindow* parent);
    bool Show(bool show=true);
    void OnButton(wxCommandEvent& event);
    int  GetAlign();
    void SetAlign(int align);
    enum {
        kuID_ALIGN_LOWEST = kuID_HIGHEST,
        kuID_ALIGN_LT,
        kuID_ALIGN_CT,
        kuID_ALIGN_RT,
        kuID_ALIGN_LC,
        kuID_ALIGN_CC,
        kuID_ALIGN_RC,
        kuID_ALIGN_LB,
        kuID_ALIGN_CB,
        kuID_ALIGN_RB,
        kuID_ALIGN_HIGHEST
    };
};

// kuTaskBarIcon
class kuTaskBarIcon: public wxTaskBarIcon {
private:
    kuFrame* mFrame;
    virtual wxMenu* CreatePopupMenu();
    void OnLeftDclick(wxTaskBarIconEvent& event);
    void OnRestore(wxCommandEvent& event);
public:
    kuTaskBarIcon(kuFrame* frame);
DECLARE_EVENT_TABLE()
};

// kuStatusBar
class kuStatusBar: public wxStatusBar {
private:
    kuFrame* mFrame;
    wxGauge* mGauge;
    void OnSize(wxSizeEvent& event);
public:
    kuStatusBar(kuFrame* frame);
    void SetGaugeRange(int range);
    void IncrGaugeValue();
    void ClearGaugeValue();
DECLARE_EVENT_TABLE()
};

// kuPathPanel
class kuPathPanel: public wxPanel {
private:
    wxTextCtrl*     mTextCtrl;
    wxBitmapButton* mButton;
    bool mIsDir;
public:
    kuPathPanel(wxWindow* parent, bool isdir, wxString defpath=wxEmptyString);
    void     OnButton(wxCommandEvent& event);
    wxString GetPath();
    void     SetPath(wxString& path);
    void     ChangePath(wxString& path);
DECLARE_EVENT_TABLE()
};

// kuScrolledDropTarget
DECLARE_EVENT_TYPE(wxEVT_DROPPED_EVENT, wxID_ANY)

class kuScrolledDropTarget: public wxFileDropTarget {
private:
    kuFrame* mFrame;
public:
    kuScrolledDropTarget(kuFrame* frame);
    virtual bool OnDropFiles(wxCoord x, wxCoord y, const wxArrayString& items);
};

class kuFrame: public wxFrame {
private:
    struct kuPanelStatus {
        bool IsFilesystem;
        bool IsThumbnail;
    } mPanelStatus;
    wxArrayString mFileClipboard;
    bool          mFileIsMove;
    wxString      mLastDirPath;
    wxPageSetupDialogData mPageSetupDlgData;
    void       ToggleFilesystem(bool filesystem);
    void       ToggleFilesystem();
    void       ToggleArchive();
    void       ToggleThumbnail();
    void       ToggleStatusbar(bool statusbar);
    void       ToggleStatusbar();
    bool       PlaySlideshow(long interval);
    bool       SetAsWallpaper(wxString filename, int pos);
    bool       SetWallpaperToNone();
    bool       GetSystemClipboard(wxArrayString& filenames);
    bool       SetSystemClipboard(wxArrayString& filenames);
    bool       CopyToFileClipboard(wxString target, bool iscut);
    bool       PasteDirOrFile(wxArrayString& srcs, wxString desdir, bool ismove);
    bool       DeleteDir(wxString dirname, bool recycle);
    bool       DeleteFile(wxString filename, bool recycle);
#ifdef __WXMSW__
    bool       MswCopyDirOrFile(wxString src, wxString des, bool ismove);
    bool       MswDeleteDirOrFile(wxString target, bool recycle);
#else
    bool       CopyDir(wxString srcdir, wxString desdir, bool ismove);
    bool       CopyFile(wxString srcfile, wxString desfile, bool ismove);
#endif
    bool       RenameDirOrFile(wxString src, wxString des);
    bool       DirIsParent(wxString parent, wxString child);
    bool       Traverse(wxTreeCtrl* tree, int direct);
    bool       IsDir(wxTreeCtrl* tree, wxTreeItemId id);
    void       ReloadPath(wxString path, bool recreate);
    void       CheckEdited();
    void       OnNewRescaleTask(wxCommandEvent& event);
    void       OnRemoveRescaleTask(wxCommandEvent& event);

public:
    kuFrame();
    void     OnQuit(wxCommandEvent& event);
    void     OnTool(wxCommandEvent& event);
    void     OnCopyTo(wxCommandEvent& event);
    void     OnMoveTo(wxCommandEvent& event);
    void     OnExternalOpen(wxCommandEvent& event);
    void     OnSize(wxSizeEvent& event);
    void     OnClose(wxCloseEvent& event);
    void     OnLoaded(wxCommandEvent& event);
    void     OnDropped(wxCommandEvent& event);
    void     OnIdle(wxIdleEvent& event);
    void     OnAbout(wxCommandEvent& event);
    void     SetupManagers();
    void     SetupMenuBar();
    void     SetupToolBar(bool create=false);
    void     SetupStatusBar();
    void     SetupWindows(bool isdir, bool isarchive);
    void     SetupAccelerator();
    void     SetupPrinting();
    void     SetDefaults();
    void     ClearExternalOpenMenu();
    void     SetupExternalOpenMenu();
    bool     Action(int action, wxString arg1=wxEmptyString, wxString arg2=wxEmptyString);
    void     ToggleArchive(bool archive);
    void     ToggleThumbnail(bool thumbnail);
    void     HistoryTracing(wxString path);
    wxString GetCurrentPath(bool isurl=true, bool fileonly=false);
    /*
    void     PushPanelStatus();
    void     PopPanelStatus();
    */
    static wxString StripCodes(const wxChar* cptr);
    wxSplitterWindow*    mTopSplitter;
    wxSplitterWindow*    mDirSplitter;
    wxSplitterWindow*    mViewSplitter;
    kuGenericDirCtrl*    mGeneric;
    kuVirtualDirCtrl*    mVirtual;
    kuSingleScrolled*    mSingle;
    kuMultipleScrolled*  mMultiple;
    kuScrollHandler*     mHandler;
    kuTaskBarIcon*       mTaskBarIcon;
    kuStatusBar*         mStatusBar;
    kuRescaleMgrDialog*    mRescaleMgrDlg;
    #ifdef ENABLE_PICASAWEBMGR
    kuPicasaWebMgrDialog*  mPicasaWebMgrDlg;
    #endif
    wxArrayString     mSlideshowDirs;
    size_t            mSlideshowIndex;
    wxArrayString     mHistoryMainFiles;
    int               mHistoryMainIndex;
    int               mHistorySwapIndex;
    wxArrayString     mHistoryBranchFiles;
    int               mHistoryBranchIndex;
    wxIcon            mIconApp;
    bool              mIsFilesystem;
    bool              mIsArchive;
    bool              mIsThumbnail;
    bool              mIsStatusbar;
    bool              mIsSummary;
    bool              mIsSlideshow;
    bool              mIsHistory;
    bool              mIsPause;

    DECLARE_EVENT_TABLE()
};

// kuLoadThread
DECLARE_EVENT_TYPE(wxEVT_LOADED_EVENT, wxID_ANY)

struct kuBmpValue {
    wxSize    Size;
    int       Rotate;
    FIBITMAP* Bmp;
};
WX_DECLARE_STRING_HASH_MAP(kuBmpValue, kuBmpHash);

struct kuLoadTicket {
    wxString  Path;
    bool      IsUrl;
    wxSize    Size;
    int       Rotate;
    bool      Prior;
};
WX_DECLARE_OBJARRAY(kuLoadTicket, kuLoadArray);

class kuLoadThread: public wxThread {
private:
    kuFrame*      mFrame;
    wxArrayString mBmpCache;
    kuBmpHash     mBmpHash;
    wxMutex       mBmpMutex;
    kuLoadArray   mLoadQueue;
    wxMutex       mQueueMutex;
    void AddBmpHash(wxString filename, kuBmpValue& value);
    void EraseBmpHash(wxString filename);
    FIBITMAP* Rotate(FIBITMAP* bmp, int rotate);
public:
    kuLoadThread(kuFrame* frame);
    virtual void* Entry();
    bool      Clear(bool prior=true);
    bool      Append(wxString filename, bool isurl, wxSize size, int rotate);
    bool      Prepend(wxString filename, bool isurl, wxSize size, int rotate);
    bool      Remove(wxString filename);
    bool      Replace(wxString filename, FIBITMAP* bmp, bool isrdiff, int rotate=0);
    FIBITMAP* GetFiBitmap(wxString filename, bool isurl, wxSize size, int rotate, int fast=0);
    void      ClearBmpHash();
};

class kuConfig: public wxFileConfig {
public:
    kuConfig();
    kuConfig(wxFileInputStream& fis);
    bool WritePoint (const wxString& key, wxPoint& value);
    bool ReadPoint  (const wxString& key, wxPoint* value,           const wxPoint& defval);
    bool WriteFilter(const wxString& key, FREE_IMAGE_FILTER& value);
    bool ReadFilter (const wxString& key, FREE_IMAGE_FILTER* value, const FREE_IMAGE_FILTER defval);
    bool WriteColour(const wxString& key, wxColour& value);
    bool ReadColour (const wxString& key, wxColour* value,          const wxColour& defval);
};

// kuOptions
class kuOptions {
public:
    wxString mFilename;
    int      mLanguage;
    wxString mFavorite;
    int      mKeepStyle;
    double   mScale;
    bool     mKeepRotate;
    int      mRotate;
    bool     mKeepLeftTop;
    wxPoint  mLeftTop;
    bool     mLoadCompletely;
    bool     mPrefetch;
    long     mOpaque;
    double   mPrintScale;
    int      mPrintAlign;
    FREE_IMAGE_FILTER mFilter;
    bool     mSlideshowRepeat;
    long     mSlideshowInterval;
    int      mSlideshowDirectory;
    int      mLoadSiblings[4];
    int      mLoadCache;
    wxArrayString mSummaryTags[FIMD_CUSTOM];
    wxPoint       mSummaryPosition;
    wxColour      mSummaryColor;
    wxArrayString mHotkeyDirs;
    wxArrayString mExternalOpenNames;
    wxArrayString mExternalOpenExecs;
    wxArrayString mExternalOpenArgss;
    long     mRescaleMgrMaxCon;
    long     mPicasaWebMgrMaxCon;
    bool Import();
    bool Export();
};

// kuFiWrapper
class kuFiWrapper {
private:
public:
    static bool Initialize();
    static bool Finalize();
    static void               ErrorHandler(FREE_IMAGE_FORMAT fif, const char *message);
    static wxString           GetSupportedExtensions();
    static void               GetSupportedExtensions(wxArrayString* exts, bool upper=false);
    static void               GetAllSupportedFiles(wxString& src, wxArrayString* files, wxArrayString* formats=NULL);
    static bool               IsSupportedExtension(wxString ext);
    static unsigned           ReadProc(void* buffer, unsigned size, unsigned count, fi_handle handle);
    static unsigned           WriteProc(void* buffer, unsigned size, unsigned count, fi_handle handle);
    static int                SeekProc(fi_handle handle, long offset, int origin);
    static long               TellProc(fi_handle handle);
    static wxString           GetMdModelString(FREE_IMAGE_MDTYPE type);
    static FREE_IMAGE_FILTER  GetFilterById(int id);
    static int                GetIdByFilter(FREE_IMAGE_FILTER filter);
    static wxImage*           GetWxImage(wxString filename, bool isurl, wxSize size);
    static FIBITMAP*          GetFiBitmap(wxString filename, bool isurl, wxSize size, int fast=0);
    static bool               FiBitmap2WxImage(FIBITMAP* bmp, wxImage* image);
    static wxImage*           FiBitmap2WxImage(FIBITMAP* bmp);
    static wxSize             GetOriginalJPEGSize(FIBITMAP* bmp);
    static bool               SetOriginalJPEGSize(FIBITMAP* bmp, wxSize size);
    static double             GetGPSLatitude(FIBITMAP* bmp);
    static double             GetGPSLongitude(FIBITMAP* bmp);
    static FREE_IMAGE_FORMAT  GetImageFormat(wxString filename, bool byfile=true);
};

#ifdef ENABLE_PICASAWEBMGR
// kuPwWrapper
class kuPwWrapper {
private:
    static void       ErrorHandler(CURLcode res);
    static wxString   GetLink(wxXmlNode* entry, wxString name);
    static bool       GetEntries(wxString url, kuXmlNodeArray* entries, wxString token);
    static wxXmlNode* GetChild(wxXmlNode* entry, wxString name);
    static bool       GetFile(wxString url, wxString filename, wxString token);
public:
    static void       GetAllSupportedFiles(wxString& src, wxArrayString* files);
    static int        Debug(CURL* curl, curl_infotype type, char* info, size_t size, wxStringOutputStream* stream);
    static size_t     WriteData(void* buffer, size_t size, size_t nmemb, wxStringOutputStream* stream);
    static wxString   GetProperty(wxXmlNode* entry, wxString name, wxString prop=wxEmptyString);
    static bool       SetProperty(wxXmlNode* entry, wxString name, wxString value);
    static wxString   Login(wxString userid, wxString passwd);
    static bool       GetAlbums(wxString userid, kuXmlNodeArray* albums, wxString token=wxEmptyString);
    static wxXmlNode* AddAlbum(wxString userid, wxString token, wxString title, wxString summary, wxString location, wxString access=wxT("public"));
    static wxXmlNode* ModifyAlbum(wxString token, wxXmlNode* entry);
    static bool       GetPhotos(wxXmlNode* album, kuXmlNodeArray* photos, wxString token=wxEmptyString);
    static bool       AddPhoto(wxXmlNode* album, wxString filename, wxString type, wxString token);
    static bool       GetMediaContent(wxXmlNode* photo, wxString dir, wxString token=wxEmptyString);
    static wxString   GetAlbumLink(wxXmlNode* album);
    static bool       RemoveEntry(wxString token, wxXmlNode* entry);
};
#endif

#if (wxUSE_ON_FATAL_EXCEPTION == 1) && (wxUSE_STACKWALKER == 1)
class kuStackWalker : public wxStackWalker
{
	wxFile * m_DumpFile;
public:
	kuStackWalker() :m_DumpFile(nullptr) {}
	void SetDumpFile(wxFile * file) { m_DumpFile = file; }
protected:
	inline void OnStackFrame(const wxStackFrame & frame);
};
#endif

// kuApp
class kuApp: public wxApp {
private:
    bool     mIsInterrupted;
    wxMutex  mInterruptMutex;
    bool     mIsBusy;
    wxMutex  mBusyMutex;
    bool     mIsEdited;
    wxMutex  mEditedMutex;
    bool     mIsSaveAs;
    wxMutex  mSaveAsMutex;
    wxString mShellExecute;
    wxString mAssociationExecute;
public:
    enum {
        kuID_INIT_SHOW=0,
        kuID_INIT_HIDE,
        kuID_INIT_FULL,
        kuID_INIT_ICON,
    };
    kuOptions     mOptions;
    #ifdef __WXMSW__
    kuStrHashMap  mRegDefault;
    #endif
    bool          mIsImported;
    kuFrame*      mFrame;
    kuLoadThread* mLoader;
    wxLocale*     mLocale;
    wxString      mPath;
    bool          mWaitLoader;
    int           mInit;
    bool          mQuit;
    wxArrayString mCmds;
    bool          mIsDoingCmd;
    bool          mAutoZoom;
    wxArrayString mIdleTasks;
    bool          mIsDoingIdleTask;
    bool          mIsExploring;
#if (wxUSE_ON_FATAL_EXCEPTION == 1) && (wxUSE_STACKWALKER == 1)
	kuStackWalker m_StackWalker;
#endif
    virtual bool OnInit();
    virtual int  OnExit();
	virtual void OnInitCmdLine(wxCmdLineParser& cmdParser);
	virtual bool OnCmdLineParsed(wxCmdLineParser& cmdParser);
#if (wxUSE_ON_FATAL_EXCEPTION == 1) && (wxUSE_STACKWALKER == 1)
	void OnFatalException();
#endif
    void PrintCmdHelp();
    bool ParseArguments(int argc, wxChar** argv);
    bool DoInitCmd(wxChar* cmd);
    bool IsPathCmd(wxChar* cmd);
    bool IsActionCmd(wxChar* cmd);
    bool DoActionCmd(wxString& cmd);
    void SetOptions();
    void SetRegDefault();
    void ClearThread();
    bool SwitchLocale(wxString path);
    bool GetInterrupt();
    bool SetInterrupt(bool interrupt);
    bool GetBusy();
    bool SetBusy(bool busy);
    bool GetEdited();
    bool SetEdited(bool edited);
    void CheckEdited(const wxChar* caption=NULL);
    bool SetSaveAs(bool saveas);
    wxArrayInt    ShellIntegration(wxArrayInt style);
    void          SetRegSection(bool set);
    bool          GetFileAssociation(wxString ext);
    void          SetFileAssociation(wxString ext, bool associate);
    static long     GetNumberFromUser(wxString message, wxString caption, long* target, long min, long max);
    static double   GetScaleFromUser(wxWindow* parent, double current=1.0);
    static wxString GetReadableSize(wxString filename);
    static bool     IsImageFile(wxString& filename);
    static bool     IsArchiveFile(wxString& filename);
    static bool     IsUNCPath(wxString& path);
    static bool     SyncFileTime(wxString src, wxString dest);
    static void     EnableSizerChildren(wxSizer* sizer, bool enable);
};
DECLARE_APP(kuApp)

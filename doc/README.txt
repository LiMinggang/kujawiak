------------------------------
 Kujawiak Viewer Release Note
------------------------------

What is Kujawiak?
Kujawiak Viewer (kuView) is a picture viewer which focuses on browsing and managing files.
It provides some useful features including Unicode support, EXIF extraction, better scaling filter, shell integration, and browsing in archive.
Of course it also provides a lot of convenient functionalities like zooming, rotation, printing, and setting wallpaper with more options.
This project's goal is to provide a clear and efficient picture viewer on Windows. It may be used as a replacement for built-in viewer or ACDSee-like on Windows.
kuView is written in C++ and compiled into tiny executable, and is portable and green.
It also could be compiled on GNU/Linux, although it provides most features on Windows platform.

Project Info:
http://sourceforge.net/projects/kujawiak

Feature:
 ` File Management
	` Browse directories and files by tree
	` Copy/Paste/Delete/Rename files
	` Rescale manager
 ` Image View
	` Link FreeImage statically for better scaling and faster thumbnails loading
	` Prefetch files in background
	` BMP/PNG/JPG/TIF/GIF/PNM/PCX/ICO/CUR/ANI/XPM/RAW
	` Save and save as supported picture format
	` Rotate JPEG file losslessly
	` Full Screen
	` Slide Show
	` Zoom in/out/fit/extend/100%
	` Zoom selected area
	` Rotate picture ccw/cw
	` Keep Scale/Rotation/LeftTop when browsing files
 ` EXIF support
	` Extract various metadata model like EXIF/GPS/MakerNote/IPTC/XMP/GeoTIFF
	` Automatically rotate according to EXIF
	` Draw text of EXIF subset as summary
 ` Archive Support
	` Browse pictures in archive (zip/tar)
 ` Environment Integration
	` Take a directory or file as parameter to open
	` Shell Integration (may add "Browse with kuView" entry to ContextMenu)
	` File Association
	` Wallpaper Setting with more options than default (scale to fit/extend)
	` Minimize to system tray
	` Popup menu on system tray icon
	` Resize ToolBar icon automatically according to screen width
	` Accept UNC path
 ` Printing
	` With preview and page setup
	` Can be scaled and aligned intuitively
 ` Easy To Use
	` Filesystem/Archive/Thumbnail panels
	` Show progress and busy cursor when loading/copying/moving/deleting
	` Thumbnails loading can be interrupted
	` Switch view mode automatically when opening file/dir
	` Browse in a directory easily (PageUp/PageDown/Home/End, MouseScroll)
	` All operations can be controlled by ToolBar/Mouse or MenuBar/Key
	` Command Line Interface
	` Explore and set external tools
	` Traverse in history
	` Drag and drop
 ` Multilingual
	` Unicode file/directory name compatible
	` Simplified Chinese
	` Traditional Chinese
	` English
	` French
	` Italian
	` Japanese
	` Polish
	` Russian
	` Ukrainian
	` Language modules detecting
 ` Additional
	` Installation is NOT Necessary
	` Single and small executable
	` Export options to file and load it if exists
	` Linux Version (gtk)

Usage:
Simply run kuview.exe or pass a file(directory) as parameter

Key:
Alt+Home	go home
PageUp		page up in tree / previous file in the same directory
PageDown	page down in tree / next file in the same directory
Home		home in tree / first file in the same directory
End			end in tree / last file in the same directory
Up			move up in tree / move up on picture
Down		move down in tree / move down on picture
Left		move left in tree / move left on picture
Right		move right in tree / move right on picture
Ctrl+Up		back in history
Ctrl+Down	forward in history
Ctrl+Back	jump to branch in history
F2			rename file/directory
F5			reload
F10			filesystem panel
F11			archive panel
F12			thumbnail panel
TAB			switch between panels
Enter		enter fullscreen
ESC			exit fullscreen / exit
SPACE		start/pause slideshow
+			zoom in
-			zoom out
* or .		original size
/			best fit
\			zoom extend
'			fit when larger
[			file properties
]			file EXIF metadata
`			file summary
CTRL+B		statusbar (fullscreen only)
Alt+Left	rotate -90
Alt+Right	rotate +90
Ctrl+X		cut file/directory
Ctrl+C		copy file/directory
Ctrl+V		paste file/directory
DEL			delete file/directory
Ctrl+R		rename file/directory
Ctrl+P		print file
Shift+P		print preview
Ctrl+S		save file
Ctrl+I		interrupt operation
Ctrl+Z		minimize to system tray
Ctrl+Q		exit

Mouse:
LeftDoubleClick		enter fullscreen
MiddleClick			enter fullscreen
RightClick			popup menu
ScrollUp			previous file in the same directory
ScrollDown			next file in the same directory
Ctrl+ScrollUp		zoom in with fixed mouse position
Ctrl+ScrollDown		zoom out with fixed mouse position
Ctrl+SelectArea		zoom in by fitting selected area

Command line interface may be used to create your own photo slideshow CD/DVD:
 ` for Windows
	` step1. create autorun.inf as follows:
			[AUTORUN]
			OPEN=autoexec.bat
			ICON=kuview\kuview.exe,0
	` step2. create autoexec.bat as follows:
			\kuview\kuview.exe full slideshow 3 \photodir1 \photodir2 \photodir3
	` step3. burn autorun.inf, autoexec.bat, kuView dir, and photo dirs as data CD/DVD.
	` type kuview.exe /? in console for more information
 ` for Linux
	` step1. create .autorun as follows:
			#!/bin/sh
			./kuView/kuview.bin full 3 slideshow ./photodir1 ./photodir2 ./photodir3
	` step2. give .autorun execute permission:
			> chmod +x .autorun
	` step3. burn .autorun, kuView dir, and photo dirs as data CD/DVD.
	` type kuview.bin -h in console for more information

Library/Tool/Icon Used by Kujawiak Viewer:
wxWidgets (http://www.wxwidgets.org/)
FreeImage (http://freeimage.sourceforge.net/)
Microsoft Windows SDK (http://msdn.microsoft.com/en-us/windowsserver/bb980924.aspx)
MinGW (http://www.mingw.org/)
UPX (http://upx.sourceforge.net/)
poEdit (http://www.poedit.org/)
NSIS (http://nsis.sourceforge.net/)
Gorilla (http://www.kde-look.org/content/show.php?content=6927)
Korilla (http://www.kde-look.org/content/show.php?content=7264)

Author:
Augustino (augustino@users.sourceforge.net)

Translator:
de:    arkot <arkot@users.sourceforge.net>
fr:    Chrmichlefevre <chrmichlefevre@aol.com>
it_IT: Giacomo Margarito <giacomomargarito@gmail.com>
ja:    Augustino Zhuang <augustino@users.sourceforge.net>
pl:    SuperCD Team <kontakt@supercd.pl>
ru:    Alex Po <alpobrz@gmail.com>
uk:    Alex Po <alpobrz@gmail.com>
zh_CN: Augustino Zhuang <augustino@users.sourceforge.net>
zh_TW: Augustino Zhuang <augustino@users.sourceforge.net>
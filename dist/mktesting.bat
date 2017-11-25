set target=kuView
if exist %target% rd /s/q %target%
mkdir %target%
xcopy ..\doc\*.txt %target%\
xcopy ..\build\win32\bin\Release\kuview.exe* %target%\
xcopy ..\lang\*.mo %target%\
touch %target%\_testing_version_
set stamp=%date:~0,4%%date:~5,2%%date:~8,2%
set filename=%target%-%stamp%.zip
if exist %filename% del %filename%
zip -r %filename% %target%
rd /s/q %target%
pause

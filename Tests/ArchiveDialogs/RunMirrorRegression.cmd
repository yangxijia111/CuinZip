@echo off
setlocal
cd /d "%~dp0\..\.."
if not exist "Output\Tests\ArchiveDialogs" mkdir "Output\Tests\ArchiveDialogs"
if not defined VCToolsInstallDir call "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvarsall.bat" amd64
if errorlevel 1 exit /b %errorlevel%
rc /nologo /fo "Output\Tests\ArchiveDialogs\MirrorRegression.res" "Tests\ArchiveDialogs\MirrorRegression.rc"
if errorlevel 1 exit /b %errorlevel%
cl /nologo /std:c++20 /EHsc /DUNICODE /D_UNICODE %* /I "NanaZip.Modern" /I "NanaZip.Universal\SevenZip\CPP\7zip\UI\GUI" /Fo"Output\Tests\ArchiveDialogs\MirrorRegression.obj" /Fe"Output\Tests\ArchiveDialogs\MirrorRegression.exe" "Tests\ArchiveDialogs\MirrorRegression.cpp" "Output\Tests\ArchiveDialogs\MirrorRegression.res" user32.lib comctl32.lib windowsapp.lib
if errorlevel 1 exit /b %errorlevel%
"Output\Tests\ArchiveDialogs\MirrorRegression.exe"
exit /b %errorlevel%

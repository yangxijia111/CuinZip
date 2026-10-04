@echo off
setlocal
cd /d "%~dp0\..\.."
if not exist "Output\Tests\ArchiveDialogs" mkdir "Output\Tests\ArchiveDialogs"
if not defined VCToolsInstallDir call "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvarsall.bat" amd64
if errorlevel 1 exit /b %errorlevel%
cl /nologo /std:c++20 /EHsc /DUNICODE /D_UNICODE /I "NanaZip.Modern" /Fo"Output\Tests\ArchiveDialogs\FileAssociationRegression.obj" /Fe"Output\Tests\ArchiveDialogs\FileAssociationRegression.exe" "Tests\ArchiveDialogs\FileAssociationRegression.cpp" user32.lib shlwapi.lib windowsapp.lib
if errorlevel 1 exit /b %errorlevel%
"Output\Tests\ArchiveDialogs\FileAssociationRegression.exe"
exit /b %errorlevel%

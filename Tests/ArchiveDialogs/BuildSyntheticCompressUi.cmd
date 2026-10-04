@echo off
setlocal
cd /d "%~dp0\..\.."
if not exist "Output\Tests\ArchiveDialogs\UiRuntime" mkdir "Output\Tests\ArchiveDialogs\UiRuntime"
if not defined VCToolsInstallDir call "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvarsall.bat" amd64
if errorlevel 1 exit /b %errorlevel%
cl /nologo /std:c++20 /EHsc /DUNICODE /D_UNICODE /Fo"Output\Tests\ArchiveDialogs\SyntheticCompressUi.obj" /Fe"Output\Tests\ArchiveDialogs\UiRuntime\NanaZip.Modern.FileManager.exe" "Tests\ArchiveDialogs\SyntheticCompressUi.cpp" ole32.lib /link /MANIFEST:EMBED /MANIFESTINPUT:"NanaZip.Modern\NanaZip.Modern.manifest"
exit /b %errorlevel%

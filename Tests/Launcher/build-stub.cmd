@echo off
setlocal
set "TASK_OUTPUT=%~dp0..\..\Output\Tests\LauncherContract"
if not exist "%TASK_OUTPUT%\temp" mkdir "%TASK_OUTPUT%\temp"
set "TEMP=%TASK_OUTPUT%\temp"
set "TMP=%TASK_OUTPUT%\temp"
call "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvarsall.bat" amd64
if errorlevel 1 exit /b 1
pushd "%TASK_OUTPUT%"
cl /nologo /W4 /WX /utf-8 /std:c++20 /MT /EHsc /DUNICODE /D_UNICODE /Fe:child.exe /Fo:child.obj /Fd:child.pdb "%~dp0child.cpp" /link /SUBSYSTEM:WINDOWS
set "EC=%ERRORLEVEL%"
popd
exit /b %EC%

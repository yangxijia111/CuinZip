@setlocal
@echo off

rem Change to the current folder.
call "%~dp0BuildAllTargets.cmd" -p:NanaZipBuildPreviewRelease=false %*
set "CuinZipBuildExitCode=%ERRORLEVEL%"
@endlocal & exit /b %CuinZipBuildExitCode%

@setlocal
@echo off

rem Change to the current folder.
call "%~dp0BuildAllTargets.cmd" -p:NanaZipBuildWithSanitizers=true %*
set "CuinZipBuildExitCode=%ERRORLEVEL%"
@endlocal & exit /b %CuinZipBuildExitCode%

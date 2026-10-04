@setlocal
@echo off

rem Change to the current folder.
cd /d "%~dp0." || exit /b 1

rem Initialize Visual Studio environment
set VisualStudioInstallerFolder="%ProgramFiles(x86)%\Microsoft Visual Studio\Installer"
if %PROCESSOR_ARCHITECTURE%==x86 set VisualStudioInstallerFolder="%ProgramFiles%\Microsoft Visual Studio\Installer"
pushd %VisualStudioInstallerFolder% || exit /b 1
for /f "usebackq tokens=*" %%i in (`vswhere -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath`) do (
  set VisualStudioInstallDir=%%i
)
popd
if not defined VisualStudioInstallDir exit /b 1
call "%VisualStudioInstallDir%\VC\Auxiliary\Build\vcvarsall.bat" amd64
if errorlevel 1 exit /b 1

rem Build all targets
MSBuild -t:Restore BuildAllTargets.proj
set "CuinZipBuildExitCode=%ERRORLEVEL%"

@endlocal & exit /b %CuinZipBuildExitCode%

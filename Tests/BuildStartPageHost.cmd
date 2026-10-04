@echo off
setlocal
call "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvarsall.bat" amd64
if errorlevel 1 exit /b 1
cd /d "%~dp0.."
if not exist "Output\Tests" mkdir "Output\Tests"
cl /nologo /EHsc /W4 /std:c++17 /utf-8 /MD Tests\StartPageHost.cpp /FoOutput\Tests\StartPageHost.obj /FdOutput\Tests\StartPageHost.pdb /FeOutput\Tests\StartPageHost.exe /link /PDB:Output\Tests\StartPageHost.pdb windowsapp.lib ole32.lib oleaut32.lib uiautomationcore.lib user32.lib
exit /b %ERRORLEVEL%

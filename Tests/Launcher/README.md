# Native launcher contract tests

These tests use a small native child process in a fixture under
`Output/Tests/LauncherContract`. They do not open the real file manager,
install the application, or register file associations.

1. Build `Tools/CuinZip.Launcher/CuinZip.vcxproj` as x64 Release.
2. Run `Tests/Launcher/build-stub.cmd` with Visual Studio 2022 Build Tools available.
3. Run `powershell -NoProfile -File Tests/Launcher/test-launcher.ps1`.

The checks cover no arguments, quoted and Unicode arguments, shell punctuation,
the caller's working directory, the old installer entry name, missing runtime
files, and an invalid child EXE.
The error dialog checks read and close only the test launcher's own windows.
Results are saved to `Output/Tests/LauncherContract/result.json`.

The launcher path can also be passed with `-LauncherPath <path>`. The stub build
script uses the Visual Studio Build Tools path verified for this development
machine; adjust that invocation for a different toolchain location.

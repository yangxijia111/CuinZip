# Home regression host

`StartPageHost.cpp` exercises the Home page independently of the file manager.
The recent-archive callback supplies only command-line fixtures. The host does
not load or save file-manager settings, register file associations, invoke
compression/extraction, or open a native file picker.

Build from the repository root with `Tests\BuildStartPageHost.cmd`. The build
outputs stay in `Output\Tests`.

Run `Output\Tests\StartPageHost.exe --paths-only` to check the actual Home
path-transfer helper. Tests cover Unicode paths, the exact capacity boundary,
overflow rejection without silently omitting selected files, a long path,
missing buffers, empty selections, and invalid NUL-containing paths.

For resource/UI checks, copy the host EXE beside the freshly packaged
`NanaZip.Modern.dll`, `resources.pri`, and its runtime dependencies inside the
working directory. For example, with runtime directory
`Output\Binaries\Release\NanaZipPackage\x64`:

```powershell
$runtime = Resolve-Path .\Output\Binaries\Release\NanaZipPackage\x64
Copy-Item .\Output\Tests\StartPageHost.exe "$runtime\StartPageHost.exe"
& "$runtime\StartPageHost.exe" $runtime en --strings
& "$runtime\StartPageHost.exe" $runtime zh-Hans --strings
& "$runtime\StartPageHost.exe" $runtime zh-Hans --ui
```

`--strings` checks the requested language for Home strings, the new folder
selection action, and a legacy dialog label. It changes only the process's
`ResourceContext.Languages`; it does not change Windows language settings or
`ApplicationLanguages.PrimaryLanguageOverride`.

`--ui` uses UI Automation to open Create Archive, check the file/folder
selection actions, check that Next and Remove are disabled for empty input,
return to Home, and close the host. Optional paths after `--ui` appear as fake
recent entries if they exist. The test does not click them or call the picker.
Actual file-picker selection, recent-item clicks, and drag-and-drop still need
interactive verification; the automated tests do not claim to cover them.

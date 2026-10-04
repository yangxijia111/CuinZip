#define AppName "CuinZip"
#define AppPublisher "CuinZip Project"
#define AppCopyright "© CuinZip Project and Contributors. Based on NanaZip (M2-Team) and 7-Zip (Igor Pavlov). All rights reserved."
#define AppURL "https://github.com/yangxijia111/CuinZip"
#define AppExeName "CuinZip.exe"

#ifndef AppVersion
#define AppVersion "0.1.0.1"
#endif

#ifndef AppSourceDir
#define AppSourceDir "..\Output\Binaries\Root\CuinZip_" + AppVersion + "_x64"
#endif

[Setup]
AppId={{9BBA69D3-509F-4EA5-A78D-86BD39492F37}
AppName={#AppName}
AppCopyright={#AppCopyright}
AppVersion={#AppVersion}
AppPublisher={#AppPublisher}
AppPublisherURL={#AppURL}
AppSupportURL={#AppURL}
AppUpdatesURL={#AppURL}
DefaultDirName={autopf}\{#AppName}
DefaultGroupName={#AppName}
DisableDirPage=yes
DisableProgramGroupPage=yes
VersionInfoVersion={#AppVersion}
VersionInfoDescription={#AppName} {#AppVersion} (Unsigned Preview Build)
VersionInfoProductName={#AppName}
UninstallDisplayName={#AppName} {#AppVersion}
UninstallDisplayIcon={app}\{#AppExeName}
SetupIconFile=..\Assets\CuinZip.ico

; 每用户安装,无需管理员权限;候选关联只写当前用户。
PrivilegesRequired=lowest
ChangesAssociations=yes

OutputBaseFilename=CuinZip_{#AppVersion}_x64_Setup
SolidCompression=yes
WizardStyle=modern
Compression=lzma2

ArchitecturesAllowed=x64compatible
ArchitecturesInstallIn64BitMode=x64compatible

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"

[Files]
Source: "{#AppSourceDir}\{#AppExeName}"; DestDir: "{app}"; Flags: ignoreversion
; 旧安装版曾把 FM 放在根目录；升级时用同一启动器替换该已存在入口，
; 使用户以前选择的打开方式继续启动新版 x64 FM。全新安装不添加此别名。
Source: "{#AppSourceDir}\{#AppExeName}"; DestDir: "{app}"; DestName: "NanaZip.Modern.FileManager.exe"; Flags: ignoreversion onlyifdestfileexists
; x64 运行时文件(与 Portable ZIP 同源,由构建流程暂存)
Source: "{#AppSourceDir}\x64\*"; DestDir: "{app}\x64"; \
    Flags: ignoreversion recursesubdirs createallsubdirs
; 文档
Source: "{#AppSourceDir}\License.txt"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#AppSourceDir}\ReadMe.txt"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#AppSourceDir}\ReleaseNotes.txt"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#AppSourceDir}\快速上手.txt"; DestDir: "{app}"; Flags: ignoreversion

[Registry]
; 注册候选程序；不设置扩展名默认值，不写 UserChoice，不抢占现有默认应用。
Root: HKCU; Subkey: "Software\Classes\CuinZip.Archive"; ValueType: string; ValueData: "CuinZip Archive"; Flags: uninsdeletekey
Root: HKCU; Subkey: "Software\Classes\CuinZip.Archive\DefaultIcon"; ValueType: string; ValueData: """{app}\{#AppExeName}"",0"
Root: HKCU; Subkey: "Software\Classes\CuinZip.Archive\shell\open\command"; ValueType: string; ValueData: """{app}\{#AppExeName}"" ""%1"""
Root: HKCU; Subkey: "Software\Classes\CuinZip.Archive\Application"; ValueType: string; ValueName: "ApplicationName"; ValueData: "CuinZip"
Root: HKCU; Subkey: "Software\Classes\Applications\CuinZip.exe"; ValueType: string; ValueName: "FriendlyAppName"; ValueData: "CuinZip"; Flags: uninsdeletekey
Root: HKCU; Subkey: "Software\Classes\Applications\CuinZip.exe\shell\open\command"; ValueType: string; ValueData: """{app}\{#AppExeName}"" ""%1"""
Root: HKCU; Subkey: "Software\CuinZip\Capabilities"; ValueType: string; ValueName: "ApplicationName"; ValueData: "CuinZip"; Flags: uninsdeletekey
Root: HKCU; Subkey: "Software\CuinZip\Capabilities"; ValueType: string; ValueName: "ApplicationDescription"; ValueData: "Open, create and extract archives with CuinZip."
Root: HKCU; Subkey: "Software\CuinZip\Capabilities"; ValueType: string; ValueName: "ApplicationIcon"; ValueData: """{app}\{#AppExeName}"",0"
Root: HKCU; Subkey: "Software\RegisteredApplications"; ValueType: string; ValueName: "CuinZip"; ValueData: "Software\CuinZip\Capabilities"; Flags: uninsdeletevalue
Root: HKCU; Subkey: "Software\CuinZip\Capabilities\FileAssociations"; ValueType: string; ValueName: ".7z"; ValueData: "CuinZip.Archive"
Root: HKCU; Subkey: "Software\CuinZip\Capabilities\FileAssociations"; ValueType: string; ValueName: ".zip"; ValueData: "CuinZip.Archive"
Root: HKCU; Subkey: "Software\CuinZip\Capabilities\FileAssociations"; ValueType: string; ValueName: ".rar"; ValueData: "CuinZip.Archive"
Root: HKCU; Subkey: "Software\CuinZip\Capabilities\FileAssociations"; ValueType: string; ValueName: ".tar"; ValueData: "CuinZip.Archive"
Root: HKCU; Subkey: "Software\CuinZip\Capabilities\FileAssociations"; ValueType: string; ValueName: ".gz"; ValueData: "CuinZip.Archive"
Root: HKCU; Subkey: "Software\CuinZip\Capabilities\FileAssociations"; ValueType: string; ValueName: ".bz2"; ValueData: "CuinZip.Archive"
Root: HKCU; Subkey: "Software\CuinZip\Capabilities\FileAssociations"; ValueType: string; ValueName: ".xz"; ValueData: "CuinZip.Archive"
Root: HKCU; Subkey: "Software\CuinZip\Capabilities\FileAssociations"; ValueType: string; ValueName: ".zst"; ValueData: "CuinZip.Archive"
Root: HKCU; Subkey: "Software\CuinZip\Capabilities\FileAssociations"; ValueType: string; ValueName: ".cab"; ValueData: "CuinZip.Archive"
Root: HKCU; Subkey: "Software\CuinZip\Capabilities\FileAssociations"; ValueType: string; ValueName: ".iso"; ValueData: "CuinZip.Archive"
Root: HKCU; Subkey: "Software\Classes\.7z\OpenWithProgids"; ValueType: string; ValueName: "CuinZip.Archive"; ValueData: ""; Flags: uninsdeletevalue uninsdeletekeyifempty
Root: HKCU; Subkey: "Software\Classes\.zip\OpenWithProgids"; ValueType: string; ValueName: "CuinZip.Archive"; ValueData: ""; Flags: uninsdeletevalue uninsdeletekeyifempty
Root: HKCU; Subkey: "Software\Classes\.rar\OpenWithProgids"; ValueType: string; ValueName: "CuinZip.Archive"; ValueData: ""; Flags: uninsdeletevalue uninsdeletekeyifempty
Root: HKCU; Subkey: "Software\Classes\.tar\OpenWithProgids"; ValueType: string; ValueName: "CuinZip.Archive"; ValueData: ""; Flags: uninsdeletevalue uninsdeletekeyifempty
Root: HKCU; Subkey: "Software\Classes\.gz\OpenWithProgids"; ValueType: string; ValueName: "CuinZip.Archive"; ValueData: ""; Flags: uninsdeletevalue uninsdeletekeyifempty
Root: HKCU; Subkey: "Software\Classes\.bz2\OpenWithProgids"; ValueType: string; ValueName: "CuinZip.Archive"; ValueData: ""; Flags: uninsdeletevalue uninsdeletekeyifempty
Root: HKCU; Subkey: "Software\Classes\.xz\OpenWithProgids"; ValueType: string; ValueName: "CuinZip.Archive"; ValueData: ""; Flags: uninsdeletevalue uninsdeletekeyifempty
Root: HKCU; Subkey: "Software\Classes\.zst\OpenWithProgids"; ValueType: string; ValueName: "CuinZip.Archive"; ValueData: ""; Flags: uninsdeletevalue uninsdeletekeyifempty
Root: HKCU; Subkey: "Software\Classes\.cab\OpenWithProgids"; ValueType: string; ValueName: "CuinZip.Archive"; ValueData: ""; Flags: uninsdeletevalue uninsdeletekeyifempty
Root: HKCU; Subkey: "Software\Classes\.iso\OpenWithProgids"; ValueType: string; ValueName: "CuinZip.Archive"; ValueData: ""; Flags: uninsdeletevalue uninsdeletekeyifempty

[Icons]
Name: "{group}\{#AppName}"; Filename: "{app}\{#AppExeName}"
Name: "{group}\{#AppName} Command Line"; Filename: "{app}\x64\NanaZip.Universal.Console.exe"
Name: "{group}\CuinZip 快速上手"; Filename: "{app}\快速上手.txt"
Name: "{group}\Uninstall {#AppName}"; Filename: "{uninstallexe}"
Name: "{commondesktop}\{#AppName}"; Filename: "{app}\{#AppExeName}"; Tasks: desktopicon

[Run]
Filename: "{app}\{#AppExeName}"; Description: "{cm:LaunchProgram,{#AppName}}"; \
    Flags: nowait postinstall skipifsilent

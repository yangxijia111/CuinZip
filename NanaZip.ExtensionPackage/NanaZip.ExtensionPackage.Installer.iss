#define AppName "CuinZip"
#define AppPublisher "CuinZip Project"
#define AppCopyright "© CuinZip Project and Contributors. Based on NanaZip (M2-Team) and 7-Zip (Igor Pavlov). All rights reserved."
#define AppURL "https://github.com/yangxijia111/CuinZip"
#define AppExeName "NanaZip.Modern.FileManager.exe"

#ifndef AppVersion
#define AppVersion "0.1.0.0"
#endif

#ifndef AppSourceDir
#define AppSourceDir "..\Output\Binaries\Root\Portable"
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

; 每用户安装,无需管理员权限;也允许用户在对话框中提升为全局安装
PrivilegesRequired=lowest
PrivilegesRequiredOverridesAllowed=dialog

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
; x64 运行时文件(与 Portable ZIP 同源,由构建流程暂存)
Source: "{#AppSourceDir}\x64\*"; DestDir: "{app}"; \
    Flags: ignoreversion recursesubdirs createallsubdirs
; 文档
Source: "{#AppSourceDir}\License.txt"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#AppSourceDir}\ReadMe.txt"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#AppSourceDir}\ReleaseNotes.txt"; DestDir: "{app}"; Flags: ignoreversion

[Icons]
Name: "{group}\{#AppName}"; Filename: "{app}\{#AppExeName}"
Name: "{group}\{#AppName} Command Line"; Filename: "{app}\NanaZip.Universal.Console.exe"
Name: "{group}\Uninstall {#AppName}"; Filename: "{uninstallexe}"
Name: "{commondesktop}\{#AppName}"; Filename: "{app}\{#AppExeName}"; Tasks: desktopicon

[Run]
Filename: "{app}\{#AppExeName}"; Description: "{cm:LaunchProgram,{#AppName}}"; \
    Flags: nowait postinstall skipifsilent

#define AppName "CuinZip Extension Package"
#define AppPublisher "CuinZip Project"
#define AppCopyright "© CuinZip Project and Contributors. Based on NanaZip (M2-Team) and 7-Zip (Igor Pavlov). All rights reserved."
#define AppURL "https://github.com/yangxijia111/CuinZip"

#ifndef AppVersion
#define AppVersion "5.1.0.0"
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
DisableDirPage=yes
DefaultGroupName={#AppName}
DisableProgramGroupPage=yes
VersionInfoVersion={#AppVersion}

PrivilegesRequired=lowest
PrivilegesRequiredOverridesAllowed=dialog
OutputBaseFilename=CuinZip.ExtensionPackage_{#AppVersion}
SolidCompression=yes
WizardStyle=modern

ArchitecturesAllowed=x64compatible or arm64
ArchitecturesInstallIn64BitMode=x64compatible or arm64

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"

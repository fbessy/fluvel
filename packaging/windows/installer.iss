#ifndef MyAppVersion
  #define MyAppVersion "dev"
#endif

#ifndef MyVersionInfo
  #define MyVersionInfo "1.0.0.0"
#endif

[Setup]
AppName=Fluvel Set
AppVersion={#MyAppVersion}
AppPublisher=Fluvel

VersionInfoVersion={#MyVersionInfo}
VersionInfoCompany=Fluvel
VersionInfoDescription=Fluvel Set Installer
VersionInfoCopyright=Copyright (C) 2010-2026 Fabien Bessy

DefaultDirName={autopf}\Fluvel Set
ArchitecturesAllowed=x64compatible
ArchitecturesInstallIn64BitMode=x64compatible
DefaultGroupName=Fluvel Set
OutputDir=Output
OutputBaseFilename=FluvelSet-Setup
Compression=lzma
SolidCompression=yes
WizardStyle=modern

; Icône de l’installateur
SetupIconFile=Fluvel.ico

; Licence (active la page)
LicenseFile=Licence_CeCILL_V2.1-en.txt

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"; LicenseFile: "Licence_CeCILL_V2.1-en.txt"
Name: "french"; MessagesFile: "compiler:Languages\French.isl"; LicenseFile: "Licence_CeCILL_V2.1-fr.txt"

[Files]
Source: "..\..\build\src\app\Release\*"; DestDir: "{app}"; Excludes: "*.lib,*.exp"; Flags: recursesubdirs

[Icons]
Name: "{group}\Fluvel Set"; Filename: "{app}\FluvelSet.exe"
Name: "{autodesktop}\Fluvel Set"; Filename: "{app}\FluvelSet.exe"; Tasks: desktopicon

[Tasks]
Name: "desktopicon"; Description: "Create a desktop shortcut"; GroupDescription: "Additional icons:"

[Run]
Filename: "{app}\FluvelSet.exe"; Description: "Launch Fluvel Set"; Flags: nowait postinstall skipifsilent

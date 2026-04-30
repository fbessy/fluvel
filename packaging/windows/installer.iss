[Setup]
AppName=Fluvel
AppVersion=1.0.0
AppPublisher=Fluvel
DefaultDirName={autopf}\Fluvel
DefaultGroupName=Fluvel
OutputDir=Output
OutputBaseFilename=FluvelSetup
Compression=lzma
SolidCompression=yes
WizardStyle=modern

; Icône de l’installateur
SetupIconFile=packaging\windows\Fluvel.ico

; 🔥 Licence dynamique selon langue
LicenseFile={code:GetLicenseFile}

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"
Name: "french"; MessagesFile: "compiler:Languages\French.isl"

[Files]
; Ton app + DLL Qt
Source: "build\src\app\Release\*"; DestDir: "{app}"; Flags: recursesubdirs

[Icons]
; Menu démarrer
Name: "{group}\Fluvel"; Filename: "{app}\Fluvel.exe"

; Bureau
Name: "{autodesktop}\Fluvel"; Filename: "{app}\Fluvel.exe"; Tasks: desktopicon

[Tasks]
Name: "desktopicon"; Description: "Create a desktop shortcut"; GroupDescription: "Additional icons:"

[Run]
Filename: "{app}\Fluvel.exe"; Description: "Launch Fluvel"; Flags: nowait postinstall skipifsilent

; =========================
; 🔥 PARTIE DYNAMIQUE
; =========================

[Code]

function GetLicenseFile(Param: String): String;
begin
  if ActiveLanguage = 'french' then
    Result := ExpandConstant('{src}\packaging\windows\Licence_CeCILL_V2.1-fr.txt')
  else
    Result := ExpandConstant('{src}\packaging\windows\Licence_CeCILL_V2.1-en.txt');
end;

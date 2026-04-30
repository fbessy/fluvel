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
SetupIconFile=Fluvel.ico

; Licence (active la page)
LicenseFile=Licence_CeCILL_V2.1-en.txt

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"
Name: "french"; MessagesFile: "compiler:Languages\French.isl"

[Files]
Source: "build\src\app\Release\*"; DestDir: "{app}"; Flags: recursesubdirs

[Icons]
Name: "{group}\Fluvel"; Filename: "{app}\Fluvel.exe"
Name: "{autodesktop}\Fluvel"; Filename: "{app}\Fluvel.exe"; Tasks: desktopicon

[Tasks]
Name: "desktopicon"; Description: "Create a desktop shortcut"; GroupDescription: "Additional icons:"

[Run]
Filename: "{app}\Fluvel.exe"; Description: "Launch Fluvel"; Flags: nowait postinstall skipifsilent

[Code]

procedure CurPageChanged(CurPageID: Integer);
var
  LicensePath: String;
begin
  if CurPageID = wpLicense then
  begin
    if ActiveLanguage = 'french' then
      LicensePath := ExpandConstant('{src}\Licence_CeCILL_V2.1-fr.txt')
    else
      LicensePath := ExpandConstant('{src}\Licence_CeCILL_V2.1-en.txt');

    if FileExists(LicensePath) then
      WizardForm.LicenseMemo.Lines.LoadFromFile(LicensePath);
  end;
end;

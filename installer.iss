#ifndef MyAppVersion
  #error "MyAppVersion must be defined. Use /DMyAppVersion=x.y.z"
#endif

[Setup]
AppId={{E5A2B2A0-3D6F-4E8C-9F1A-7C5D8E2F4A6B}}
AppName=ClipPing
AppVersion={#MyAppVersion}
AppVerName=ClipPing
AppPublisher=Kevin Gosse
AppPublisherURL=https://github.com/kevingosse/ClipPing
DefaultDirName={autopf}\ClipPing
DefaultGroupName=ClipPing
UninstallDisplayIcon={app}\ClipPing.exe
OutputBaseFilename=ClipPing-Setup
OutputDir=build
SetupIconFile=src\ClipPing\clipboard.ico
Compression=lzma2
SolidCompression=yes
ArchitecturesAllowed=x64compatible
ArchitecturesInstallIn64BitMode=x64compatible
CloseApplications=yes
RestartApplications=yes
PrivilegesRequired=lowest

[Files]
Source: "build\ClipPing.exe"; DestDir: "{app}"; Flags: ignoreversion

[Icons]
Name: "{group}\ClipPing"; Filename: "{app}\ClipPing.exe"

[Run]
Filename: "{app}\ClipPing.exe"; Description: "Launch ClipPing"; Flags: nowait postinstall skipifsilent

[UninstallDelete]
Type: files; Name: "{app}\ClipPing.exe"
Type: files; Name: "{localappdata}\ClipPing\settings.ini"
Type: dirifempty; Name: "{localappdata}\ClipPing"

[Registry]
Root: HKCU; Subkey: "Software\Microsoft\Windows\CurrentVersion\Run"; ValueName: "ClipPing"; Flags: uninsdeletevalue

[Code]
procedure MigrateAutoStartEntry();
var
  OldValue: String;
  QuotedPath: String;
begin
  QuotedPath := '"' + ExpandConstant('{app}\ClipPing.exe') + '"';

  if RegQueryStringValue(HKEY_CURRENT_USER,
    'Software\Microsoft\Windows\CurrentVersion\Run',
    'ClipPing', OldValue) then
  begin
    if CompareText(OldValue, QuotedPath) <> 0 then
    begin
      RegWriteStringValue(HKEY_CURRENT_USER,
        'Software\Microsoft\Windows\CurrentVersion\Run',
        'ClipPing', QuotedPath);
    end;
  end;
end;

function InitializeUninstall(): Boolean;
var
  ResultCode: Integer;
begin
  Exec('taskkill', '/f /im ClipPing.exe', '', SW_HIDE, ewWaitUntilTerminated, ResultCode);
  Result := True;
end;

procedure CurStepChanged(CurStep: TSetupStep);
begin
  if CurStep = ssPostInstall then
    MigrateAutoStartEntry();
end;

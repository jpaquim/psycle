[Setup]
AppName=Psycle
AppVerName=Psycle 1.60 Final Release
AppPublisher=Psycledelics
AppPublisherURL=http://www.pastnotecut.org/psycle/
AppSupportURL=http://www.pastnotecut.org/psycle/
AppUpdatesURL=http://www.pastnotecut.org/psycle/
DefaultDirName={pf}\Psycle
DefaultGroupName=Psycle
Compression=zip/9
OutputDir=.\
SourceDir=.\
DisableStartupPrompt=true
AppCopyright=© Psychedelics 2000-2002
InfoBeforeFile=docs\whatsnew.txt
OutputBaseFilename=PsycleInstaller
AllowNoIcons=true
AlwaysShowDirOnReadyPage=true
AlwaysShowGroupOnReadyPage=true
WindowResizable=false
WindowStartMaximized=false
ChangesAssociations=true
UninstallDisplayIcon={app}\Psycle.exe
AppVersion=1.60
AlwaysShowComponentsList=true
ShowComponentSizes=true
FlatComponentsList=true
AllowRootDirectory=false
AppID=Psycle Uninstall

[Tasks]
Name: desktopicon; Description: Create a &desktop icon; GroupDescription: Additional icons:; MinVersion: 4,4; Components: Application
Name: quicklaunchicon; Description: Create a &Quick Launch icon; GroupDescription: Additional icons:; MinVersion: 4,4; Flags: unchecked; Components: Application

[Files]
Source: SetupUtils.exe; DestDir: {tmp}; CopyMode: alwaysoverwrite; Flags: deleteafterinstall
Source: Psycle.exe; DestDir: {app}; Components: Application
Source: Docs\whatsnew.txt; DestDir: {app}\Docs; Components: Documents
Source: Docs\FAQ.txt; DestDir: {app}\Docs; Components: Documents
Source: Docs\keys.txt; DestDir: {app}\Docs; Components: Documents
Source: Docs\previous User Readme.txt; DestDir: {app}\Docs; Components: Documents
Source: Docs\Readme.txt; DestDir: {app}\Docs; Components: Documents
Source: Docs\todo.txt; DestDir: {app}\Docs; Components: Documents
Source: Docs\tweakings and commands.txt; DestDir: {app}\Docs; Components: Documents
Source: Docs\MIDI Help\PsycleMIDIChart.TXT; DestDir: {app}\Docs\MIDI Help; Components: Documents
Source: Docs\MIDI Help\midiconfiguration.jpg; DestDir: {app}\Docs\MIDI Help; Components: Documents
Source: Docs\MIDI Help\midimonitor.jpg; DestDir: {app}\Docs\MIDI Help; Components: Documents
Source: Docs\MIDI Help\midiout.jpg; DestDir: {app}\Docs\MIDI Help; Components: Documents
Source: Docs\MIDI Help\psyclemidi.html; DestDir: {app}\Docs\MIDI Help; Components: Documents
Source: Docs\upwego\upwego4.html; DestDir: {app}\Docs\Upwego
Source: Docs\upwego\images\2pfilter.gif; DestDir: {app}\Docs\Upwego\Images; Components: Documents
Source: Docs\upwego\images\connections.gif; DestDir: {app}\Docs\Upwego\Images; Components: Documents
Source: Docs\upwego\images\connections2.gif; DestDir: {app}\Docs\Upwego\Images; Components: Documents
Source: Docs\upwego\images\loadsample.gif; DestDir: {app}\Docs\Upwego\Images; Components: Documents
Source: Docs\upwego\images\newmachine.gif; DestDir: {app}\Docs\Upwego\Images; Components: Documents
Source: Docs\upwego\images\newmachine2.gif; DestDir: {app}\Docs\Upwego\Images; Components: Documents
Source: Docs\upwego\images\parameters.gif; DestDir: {app}\Docs\Upwego\Images; Components: Documents
Source: Docs\upwego\images\parameters2.gif; DestDir: {app}\Docs\Upwego\Images; Components: Documents
Source: Docs\upwego\images\seqview.gif; DestDir: {app}\Docs\Upwego\Images; Components: Documents
Source: Docs\upwego\images\tempotpb.gif; DestDir: {app}\Docs\Upwego\Images; Components: Documents
Source: Docs\upwego\images\title.jpg; DestDir: {app}\Docs\Upwego\Images; Components: Documents
Source: Docs\upwego\images\trackview.gif; DestDir: {app}\Docs\Upwego\Images; Components: Documents
Source: Docs\upwego\images\tweakeffect.gif; DestDir: {app}\Docs\Upwego\Images; Components: Documents
Source: Docs\upwego\images\tweaking.gif; DestDir: {app}\Docs\Upwego\Images; Components: Documents
Source: Docs\upwego\images\tweakingeffect.gif; DestDir: {app}\Docs\Upwego\Images; Components: Documents
Source: Docs\upwego\images\viewmachines.gif; DestDir: {app}\Docs\Upwego\Images; Components: Documents
Source: Docs\upwego\images\viewpattern.gif; DestDir: {app}\Docs\Upwego\Images; Components: Documents
Source: Plugins\Zephod SuperFM.dll; DestDir: {app}\Plugins; Components: Plugins
Source: Plugins\CyanPhase_VibraSynth_1P.dll; DestDir: {app}\Plugins; Components: Plugins
Source: Plugins\FeedMe.prs; DestDir: {app}\Plugins; Components: Plugins
Source: Plugins\FeedMe.dll; DestDir: {app}\Plugins; Components: Plugins
Source: Plugins\JMDrum.prs; DestDir: {app}\Plugins; Components: Plugins
Source: Plugins\JMDrum readme.txt; DestDir: {app}\Plugins; Components: Plugins
Source: Plugins\JMDrum.dll; DestDir: {app}\Plugins; Components: Plugins
Source: Plugins\KarLKoX Surround.dll; DestDir: {app}\Plugins; Components: Plugins
Source: Plugins\Arguru\Arguru Distortion.dll; DestDir: {app}\Plugins\Arguru; Components: Plugins
Source: Plugins\Arguru\Arguru Freeverb.dll; DestDir: {app}\Plugins\Arguru; Components: Plugins
Source: Plugins\Arguru\Arguru Goaslicer.dll; DestDir: {app}\Plugins\Arguru; Components: Plugins
Source: Plugins\Arguru\Arguru Reverb.dll; DestDir: {app}\Plugins\Arguru; Components: Plugins
Source: Plugins\Arguru\Arguru Synth 2f.prs; DestDir: {app}\Plugins\Arguru; Components: Plugins
Source: Plugins\Arguru\Arguru Synth 2f.dll; DestDir: {app}\Plugins\Arguru; Components: Plugins
Source: Plugins\Arguru\Arguru xFilter.dll; DestDir: {app}\Plugins\Arguru; Components: Plugins
Source: Plugins\Arguru\AS22toAS2f.txt; DestDir: {app}\Plugins\Arguru; Components: Plugins
Source: Plugins\Old\Synth22.prs; DestDir: {app}\Plugins\Old; Components: Plugins
Source: Plugins\Old\Synth22.dll; DestDir: {app}\Plugins\Old; Components: Plugins
Source: Plugins\Pooplog\Pooplog FM.txt; DestDir: {app}\Plugins\Pooplog; Components: Plugins
Source: Plugins\Pooplog\Pooplog FM Laboratory.dll; DestDir: {app}\Plugins\Pooplog; Components: Plugins
Source: Plugins\Pooplog\Pooplog FM Light.dll; DestDir: {app}\Plugins\Pooplog; Components: Plugins
Source: Plugins\Pooplog\Pooplog FM UltraLight.dll; DestDir: {app}\Plugins\Pooplog; Components: Plugins
Source: in_psycle.dll; DestDir: {code:GetWinampPluginFolder}; Components: Winamp_Plugin

[INI]
Filename: {app}\Psycle.url; Section: InternetShortcut; Key: URL; String: http://psycle.project-psy.com; Components: Documents

[Icons]
Name: {group}\Psycle; Filename: {app}\Psycle.exe; IconIndex: 0; Components: Application
Name: {group}\Psycle on the Web; Filename: {app}\Psycle.url; IconIndex: 0; Components: Documents
Name: {userdesktop}\Psycle; Filename: {app}\Psycle.exe; MinVersion: 4,4; Tasks: desktopicon; IconIndex: 0; Components: Application
Name: {userappdata}\Microsoft\Internet Explorer\Quick Launch\Psycle; Filename: {app}\Psycle.exe; MinVersion: 4,4; Tasks: quicklaunchicon; IconIndex: 0; Components: Application
Name: {group}\Pattern Keys; Filename: {app}\Docs\keys.txt; WorkingDir: {app}; IconIndex: 0; Components: Documents
Name: {group}\Psycle FAQ; Filename: {app}\Docs\FAQ.txt; WorkingDir: {app}; IconIndex: 0; Components: Documents
Name: {group}\Psycle Commands; Filename: {app}\Docs\tweakings and commands.txt; WorkingDir: {app}; IconIndex: 0; Components: Documents
Name: {group}\Version History; Filename: {app}\Docs\whatsnew.txt; WorkingDir: {app}; IconIndex: 0; Components: Documents
Name: {group}\Readme; Filename: {app}\Docs\Readme.txt; IconIndex: 0; WorkingDir: {app}

[Run]
Filename: {app}\Psycle.exe; Description: Launch Psycle; Flags: nowait postinstall skipifsilent; Components: Application

[InstallDelete]
Name: {app}\cache.map; Type: files

[UninstallDelete]
Type: files; Name: {app}\Psycle.url; Components: Documents

[_ISTool]
EnableISX=true
UseAbsolutePaths=false

[Dirs]
Name: {app}\Docs; Components: Documents
Name: {app}\Docs\MIDI Help; Components: Documents
Name: {app}\Docs\Upwego; Components: Documents
Name: {app}\Docs\Upwego\Images; Components: Documents
Name: {app}\Plugins; Components: Plugins
Name: {app}\Plugins\Arguru; Components: Plugins
Name: {app}\Plugins\Old; Components: Plugins
Name: {app}\Plugins\Pooplog; Components: Plugins
Name: {app}\Samples; Components: Application
Name: {app}\Songs; Components: Application
Name: {app}\Vst; Components: Application

[Registry]
Root: HKCU; Subkey: Software\AAS; Flags: uninsdeletekey deletekey
Root: HKCR; SubKey: .psy; ValueType: string; ValueData: Psycle Music Archive; Flags: uninsdeletekey
Root: HKCR; SubKey: Psycle Music Archive; ValueType: string; ValueData: Psycle Music Archive; Flags: uninsdeletekey
Root: HKCR; SubKey: Psycle Music Archive\Shell\Open\Command; ValueType: string; ValueData: """{app}\Psycle.exe"" ""%1"""; Flags: uninsdeletevalue
Root: HKCR; Subkey: Psycle Music Archive\DefaultIcon; ValueType: string; ValueData: {app}\Psycle.exe,0; Flags: uninsdeletevalue

[Types]
Name: Full; Description: Full Psycle Install
Name: Custom; Description: Custom Psycle Install; Flags: iscustom

[Components]
Name: Application; Description: Psycle Main Application; Types: Full
Name: Plugins; Description: Psycle Plugins; Types: Full
Name: Documents; Description: Documents; Types: Full
Name: Winamp_Plugin; Description: Psycle Winamp Plugin; Types: Full


[Code]
program Setup;

function NextButtonClick(CurPage: Integer): Boolean;
var
  Backup: Boolean;
  ExecResult: Integer;
  ApplPath, ExecFile, ExecParam: String;
begin
  if CurPage = wpReady then
  begin
    ApplPath := AddBackslash(WizardDirValue);

    if DirExists(ApplPath) then
    begin
      if DirExists(ApplPath + 'Backup') then
        Deltree(ApplPath + 'Backup', True, True, True);

      CreateDir(ApplPath + 'Backup');
      ExtractTemporaryFile('setuputils.exe');

      ExecFile := ChangeDirConst('{tmp}') + '\setuputils.exe';

      ExecParam := '1 "' + ApplPath + 'psycle.exe" "' + ApplPath + 'Backup\psycle.exe"';
      InstExec(ExecFile, ExecParam, '', True, True, 0,  ExecResult);

      ExecParam := '1 "' + ApplPath + 'plugins\" "' + ApplPath + 'Backup\plugins\"';
      InstExec(ExecFile, ExecParam, '', True, True, 0, ExecResult);
    end;
  end;

  Result := True;
end;

function GetWinampPluginFolder(S: String): String;
var
  Folder: String;
begin
  RegQueryStringValue(HKLM, 'SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\Winamp',
    'UninstallString', Folder);
  Folder := ExtractFilePath(Folder) + 'Plugins';
  Delete(Folder, 1, 1);
  Result := Folder;
end;

begin
end.

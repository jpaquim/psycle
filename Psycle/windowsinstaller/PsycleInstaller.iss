;Install script (C) Haralabos Michael 2002

[Setup]
AppName=Psycle
AppVerName=Psycle 1.66.2
AppPublisher=Psycledelics
AppPublisherURL=http://psycle.pastnotecut.org/
AppSupportURL=http://psycle.pastnotecut.org/
AppUpdatesURL=http://psycle.pastnotecut.org/
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
AppID=Psycle

[Tasks]
Name: desktopicon; Description: Create a &desktop icon; GroupDescription: Additional icons:; MinVersion: 4.01.1998,5.00.2195; Components: Application
Name: quicklaunchicon; Description: Create a &Quick Launch icon; GroupDescription: Additional icons:; MinVersion: 4.01.1998,5.00.2195; Flags: unchecked; Components: Application
Name: ClearPsycleSetting; Description: Clear Psycle Settings; GroupDescription: Miscellaneous:; Flags: unchecked; Components: Application

[Files]
Source: SetupUtils.exe; DestDir: {tmp}; CopyMode: alwaysoverwrite; Flags: deleteafterinstall
Source: in_psycle.dll; DestDir: {code:GetWinampPluginFolder}; Components: Winamp_Plugin
Source: Psycle.exe; DestDir: {app}; Components: Application
Source: Psycle.exe.manifest; DestDir: {app}; Components: Windows_XP_Theme_Support
Source: Docs\FAQ.txt; DestDir: {app}\Docs; Components: Documents
Source: Docs\HowTo.txt; DestDir: {app}\Docs; Components: Documents
Source: Docs\keys.txt; DestDir: {app}\Docs; Components: Documents
Source: Docs\previous User Readme.txt; DestDir: {app}\Docs; Components: Documents
Source: Docs\Readme.txt; DestDir: {app}\Docs; Components: Documents
Source: Docs\readme_presets.txt; DestDir: {app}\Docs; Components: Documents
Source: Docs\readme_themes.txt; DestDir: {app}\Docs; Components: Documents
Source: Docs\todo.txt; DestDir: {app}\Docs; Components: Documents
Source: Docs\whatsnew.txt; DestDir: {app}\Docs; Components: Documents
Source: Docs\tech_docs\AS22toAS2f.txt; DestDir: {app}\Docs\Tech_Docs; Components: Documents
Source: Docs\tech_docs\JMDrum readme.txt; DestDir: {app}\Docs\Tech_Docs; Components: Documents
Source: Docs\tech_docs\Pooplog FM.txt; DestDir: {app}\Docs\Tech_Docs; Components: Documents
Source: Docs\tech_docs\tuning for AS2f & Poop Synths.txt; DestDir: {app}\Docs\Tech_Docs; Components: Documents
Source: Docs\tech_docs\tweakings and commands.txt; DestDir: {app}\Docs\Tech_Docs; Components: Documents
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
Source: Plugins\Arguru\arguru compressor.dll; DestDir: {app}\Plugins\Arguru; Components: Arguru_Plug-ins
Source: Plugins\Arguru\Arguru Distortion.dll; DestDir: {app}\Plugins\Arguru; Components: Arguru_Plug-ins
Source: Plugins\Arguru\Arguru Freeverb.dll; DestDir: {app}\Plugins\Arguru; Components: Arguru_Plug-ins
Source: Plugins\Arguru\Arguru Goaslicer.dll; DestDir: {app}\Plugins\Arguru; Components: Arguru_Plug-ins
Source: Plugins\Arguru\Arguru Reverb.dll; DestDir: {app}\Plugins\Arguru; Components: Arguru_Plug-ins
Source: Plugins\Arguru\Arguru Synth 2f.dll; DestDir: {app}\Plugins\Arguru; Components: Arguru_Plug-ins
Source: Plugins\Arguru\Arguru Synth 2f.prs; DestDir: {app}\Plugins\Arguru; Components: Arguru_Plug-ins; CopyMode: onlyifdoesntexist
Source: Plugins\Arguru\Arguru xFilter.dll; DestDir: {app}\Plugins\Arguru; Components: Arguru_Plug-ins
Source: Plugins\Druttis\FeedMe.dll; DestDir: {app}\Plugins\Druttis; Components: Druttis_Plug-ins
Source: Plugins\Druttis\FeedMe.prs; DestDir: {app}\Plugins\Druttis; Components: Druttis_Plug-ins; CopyMode: onlyifdoesntexist
Source: Plugins\Druttis\Phantom.dll; DestDir: {app}\Plugins\Druttis; Components: Druttis_Plug-ins
Source: Plugins\Druttis\Phantom.prs; DestDir: {app}\Plugins\Druttis; Components: Druttis_Plug-ins; CopyMode: onlyifdoesntexist
Source: Plugins\Druttis\PluckedString.dll; DestDir: {app}\Plugins\Druttis; Components: Druttis_Plug-ins
Source: Plugins\Misc\CyanPhase_VibraSynth_1P.dll; DestDir: {app}\Plugins\Misc; Components: Miscellaneous_Plug-ins
Source: Plugins\Misc\JMDrum.dll; DestDir: {app}\Plugins\Misc; Components: Miscellaneous_Plug-ins
Source: Plugins\Misc\JMDrum.prs; DestDir: {app}\Plugins\Misc; Components: Miscellaneous_Plug-ins; CopyMode: onlyifdoesntexist
Source: Plugins\Misc\KarLKoX Surround.dll; DestDir: {app}\Plugins\Misc; Components: Miscellaneous_Plug-ins
Source: Plugins\Misc\M3.dll; DestDir: {app}\Plugins\Misc; Components: Miscellaneous_Plug-ins
Source: Plugins\Misc\SingleFrequency.dll; DestDir: {app}\Plugins\Misc; Components: Miscellaneous_Plug-ins
Source: Plugins\Misc\vdNoiseGate.dll; DestDir: {app}\Plugins\Misc; Components: Miscellaneous_Plug-ins
Source: Plugins\Misc\Zephod SuperFM.dll; DestDir: {app}\Plugins\Misc; Components: Miscellaneous_Plug-ins
Source: Plugins\Old\Synth22.prs; DestDir: {app}\Plugins\Old; Components: Old_Plug-ins; CopyMode: onlyifdoesntexist
Source: Plugins\Old\Synth22.dll; DestDir: {app}\Plugins\Old; Components: Old_Plug-ins
Source: Plugins\Pooplog\Pooplog FM Laboratory.dll; DestDir: {app}\Plugins\Pooplog; Components: Pooplog_Plug-ins
Source: Plugins\Pooplog\Pooplog FM Light.dll; DestDir: {app}\Plugins\Pooplog; Components: Pooplog_Plug-ins
Source: Plugins\Pooplog\Pooplog FM UltraLight.dll; DestDir: {app}\Plugins\Pooplog; Components: Pooplog_Plug-ins
Source: Plugins\Pooplog\Pooplog FM UltraLight.prs; DestDir: {app}\Plugins\Pooplog; Components: Pooplog_Plug-ins; CopyMode: onlyifdoesntexist
Source: Plugins\Sond\s_filter.dll; DestDir: {app}\Plugins\Sond; Components: Sond_Plug-ins
Source: Plugins\Sond\s_phaser.dll; DestDir: {app}\Plugins\Sond; Components: Sond_Plug-ins
Source: Plugins\Sond\s_vld.dll; DestDir: {app}\Plugins\Sond; Components: Sond_Plug-ins
Source: Plugins\Sond\s_reverb.dll; DestDir: {app}\Plugins\Sond; Components: Sond_Plug-ins
Source: Plugins\Sond\softsynth_psycle_plugin.dll; DestDir: {app}\Plugins\Sond; Components: Sond_Plug-ins

[INI]
Filename: {app}\Psycle.url; Section: InternetShortcut; Key: URL; String: http://psycle.project-psy.com; Components: Documents

[Icons]
Name: {group}\Psycle; Filename: {app}\Psycle.exe; IconIndex: 0; Components: Application
Name: {group}\Psycle on the Web; Filename: {app}\Psycle.url; IconIndex: 0; Components: Documents
Name: {userdesktop}\Psycle; Filename: {app}\Psycle.exe; MinVersion: 4,4; Tasks: desktopicon; IconIndex: 0; Components: Application
Name: {userappdata}\Microsoft\Internet Explorer\Quick Launch\Psycle; Filename: {app}\Psycle.exe; MinVersion: 4,4; Tasks: quicklaunchicon; IconIndex: 0; Components: Application
Name: {group}\Documents\Pattern Keys; Filename: {app}\Docs\keys.txt; WorkingDir: {app}\Docs; IconIndex: 0; Components: Documents
Name: {group}\Documents\Psycle FAQ; Filename: {app}\Docs\FAQ.txt; WorkingDir: {app}\Docs; IconIndex: 0; Components: Documents
Name: {group}\Version History; Filename: {app}\Docs\whatsnew.txt; WorkingDir: {app}\Docs; IconIndex: 0; Components: Documents
Name: {group}\Documents\Readme; Filename: {app}\Docs\Readme.txt; IconIndex: 0; WorkingDir: {app}\Docs
Name: {group}\Documents\How To; Filename: {app}\Docs\HowTo.txt; IconIndex: 0; WorkingDir: {app}\Docs
Name: {group}\Documents\Technican\AS22toAS2f Convert; Filename: {app}\Docs\Tech_Docs\AS22toAS2f.txt; IconIndex: 0
Name: {group}\Documents\Technican\JMDrum Readme; Filename: {app}\Docs\Tech_Docs\JMDrum readme.txt; IconIndex: 0
Name: {group}\Documents\Technican\Pooplog FM Readme; Filename: {app}\Docs\Tech_Docs\Pooplog FM.txt; IconIndex: 0
Name: {group}\Documents\Technican\Tuning For AS2f & Poop Synths; Filename: {app}\Docs\Tech_Docs\tuning for AS2f & Poop Synths.txt; IconIndex: 0
Name: {group}\Documents\Technican\Tweakings And Commands; Filename: {app}\Docs\Tech_Docs\tweakings and commands.txt; IconIndex: 0

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
Name: {app}\Docs\Tech_Docs
Name: {app}\Docs\MIDI Help; Components: Documents
Name: {app}\Docs\Upwego; Components: Documents
Name: {app}\Docs\Upwego\Images; Components: Documents
Name: {app}\Samples; Components: Application
Name: {app}\Songs; Components: Application
Name: {app}\Vst; Components: Application
Name: {app}\Plugins; Components: Sond_Plug-ins Pooplog_Plug-ins Old_Plug-ins Miscellaneous_Plug-ins Druttis_Plug-ins Arguru_Plug-ins
Name: {app}\Plugins\Arguru; Components: Arguru_Plug-ins
Name: {app}\Plugins\Druttis; Components: Druttis_Plug-ins
Name: {app}\Plugins\Misc; Components: Miscellaneous_Plug-ins
Name: {app}\Plugins\Old; Components: Old_Plug-ins
Name: {app}\Plugins\Pooplog; Components: Pooplog_Plug-ins
Name: {app}\Plugins\Sond; Components: Sond_Plug-ins

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
Name: Application; Description: Main Application; Types: Full; ExtraDiskSpaceRequired: 1000000
Name: Documents; Description: Application Documents; Types: Full; ExtraDiskSpaceRequired: 420000
Name: Windows_XP_Theme_Support; Description: Windows XP Theme Support; ExtraDiskSpaceRequired: 10000
Name: Winamp_Plugin; Description: Winamp Support Plugin: Play Psycle songs through Winamp 2; Types: Full; ExtraDiskSpaceRequired: 75000
Name: Arguru_Plug-ins; Description: Arguru Plugins: Compressor, Cross Delay, Distorsion, Goa Slicer, Reverb, Synth 2; ExtraDiskSpaceRequired: 165000; Types: Full
Name: Druttis_Plug-ins; Description: Druttis Plugins: Phantom, Feed Me, Plucked String; ExtraDiskSpaceRequired: 90000; Types: Full
Name: Miscellaneous_Plug-ins; Description: Miscellaneous Plugins: Jaz Drum Synth, CyanPhase Vibra Synth, KarLKoX Surround, M3, Zephod SuperFM; ExtraDiskSpaceRequired: 410000; Types: Full
Name: Old_Plug-ins; Description: Old Plugins: Arguru Synth 2; ExtraDiskSpaceRequired: 35000; Types: Full
Name: Pooplog_Plug-ins; Description: Pooplog Plugins: Laboratory, FM Light, FM UltraLight; ExtraDiskSpaceRequired: 490000; Types: Full
Name: Sond_Plug-ins; Description: Sond Plugins: Filter, Phazer, Vld Effect, Sond Synth; ExtraDiskSpaceRequired: 140000; Types: Full


[Code]
program Setup;

function NextButtonClick(CurPage: Integer): Boolean;
var
  ExecResult: Integer;
  ApplPath, Tasks, ExecFile, ExecParam: String;
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

      ExecFile := ExpandConstant('{tmp}') + '\setuputils.exe';

      ExecParam := '1 "' + ApplPath + 'psycle.exe" "' + ApplPath + 'Backup\psycle.exe"';
      InstExec(ExecFile, ExecParam, '', True, True, 0,  ExecResult);

      ExecParam := '1 "' + ApplPath + 'plugins\" "' + ApplPath + 'Backup\plugins\"';
      InstExec(ExecFile, ExecParam, '', True, True, 0, ExecResult);

      Tasks := WizardSelectedTasks(false);

      if Pos('ClearPsycleSetting', Tasks)>0 then
      begin
        ExecParam := '2';
        InstExec(ExecFile, ExecParam, '', True, True, 0,  ExecResult);
      end;
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

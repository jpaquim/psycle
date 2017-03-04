; Script generated by the Inno Setup Script Wizard.
; SEE THE DOCUMENTATION FOR DETAILS ON CREATING INNO SETUP SCRIPT FILES!
#include <it_download.iss>

[Setup]
; NOTE: The value of AppId uniquely identifies this application.
; Do not use the same AppId value in installers for other applications.
; (To generate a new GUID, click Tools | Generate GUID inside the IDE.)
AppID={{8E7D0A7F-B85F-44DC-8C1C-2A2C27BAEA0B}
AppName=Psycle Modular Music Creation Studio
AppVersion=1.12.2 beta
;AppVerName=Psycle Modular Music Creation Studio 1.8.8
AppPublisher=psycledelics
AppPublisherURL=http://psycle.sourceforge.net/
AppSupportURL=http://psycle.sourceforge.net/
AppUpdatesURL=http://psycle.sourceforge.net/
DefaultDirName={pf}\Psycle Modular Music Studio
DefaultGroupName=Psycle
AllowNoIcons=true
InfoBeforeFile=..\..\psycle\doc\for-end-users\readme.txt
InfoAfterFile=..\..\psycle\doc\for-end-users\whatsnew.txt
OutputBaseFilename=PsycleInstallerx64
SetupIconFile=..\..\psycle\pixmaps\psycleinstaler.ico
Compression=lzma/Max
SolidCompression=true
MinVersion=0,5.2.3790
AppCopyright=2000-2017 psycledelics
AppVerName=Psycle 1.12.2 beta 64 bits
PrivilegesRequired=poweruser
TimeStampsInUTC=true
DisableReadyPage=true
ArchitecturesInstallIn64BitMode=x64
ArchitecturesAllowed=x64
ChangesAssociations=true
ShowLanguageDialog=no
WizardImageFile="installer_left image.bmp"
WizardSmallImageFile=installer_top_right.bmp

[Languages]
Name: english; MessagesFile: compiler:Default.isl

[Tasks]
Name: desktopicon; Description: {cm:CreateDesktopIcon}; GroupDescription: {cm:AdditionalIcons}; Flags: unchecked
Name: quicklaunchicon; Description: {cm:CreateQuickLaunchIcon}; GroupDescription: {cm:AdditionalIcons}; Flags: unchecked; OnlyBelowVersion: 0,6.1
Name: deleteCacheFile; Description: deletes the cache of plugin names to force it to be regenerated.; Languages: ; Flags: unchecked
Name: deleteRegistrySettings; Description: deletes the existing settings in the registry, allowing psycle to regenerate the defaults.; Flags: unchecked

[Files]
Source: ..\msvc\output\x64\release\bin\boost_*.dll; DestDir: {app}; Flags: 64bit ignoreversion; Excludes: boost_unit_test_framework-*.dll; Components: Application; 
Source: ..\msvc\output\x64\release\bin\psycle.exe; DestDir: {app}; Flags: 64bit ignoreversion; Components: Application; 
Source: ..\msvc\output\x64\release\bin\universalis.dll; DestDir: {app}; Flags: 64bit ignoreversion; Components: Application; 
Source: ..\msvc\output\x64\release\bin\SciLexer.dll; DestDir: {app}; Flags: 64bit ignoreversion; Components: Application; 
Source: ..\msvc\output\x64\release\bin\psycle-plugins\*.dll; DestDir: {app}\PsyclePlugins; Flags: 64bit ignoreversion; Excludes: crasher.dll; Components: Open_Source_Plugins; 
Source: ..\..\psycle\doc\for-end-users\*; DestDir: {app}\Docs; Flags: ignoreversion recursesubdirs createallsubdirs; Excludes: .svn, Log1.log; Components: Documentation; 
; NOTE: Don't use "Flags: ignoreversion" on any shared system files
Source: ..\..\psycle-plugins\src\psycle\plugins\*.txt; DestDir: {app}\Docs; Excludes: license.txt; Flags: recursesubdirs ignoreversion; Components: Documentation; 
Source: ..\..\psycle\Skins\*; DestDir: {app}\Skins; Excludes: *.txt; Flags: recursesubdirs ignoreversion createallsubdirs; Components: Skins; 
Source: ..\..\psycle\doc\*.psy; DestDir: "{commondocs}\Psycle Songs"; Flags: ignoreversion; Components: Demo_Songs; 
Source: ..\..\psycle-plugins\presets\*.prs; DestDir: {app}\PsyclePlugins; Flags: ignoreversion onlyifdoesntexist; Components: Presets; 
Source: ..\..\psycle\LuaScripts\psycle\*.*; DestDir: {app}\LuaScripts; Flags: IgnoreVersion recursesubdirs promptifolder createallsubdirs; Components: Application; 
Source: ..\..\psycle\LuaScripts\start.lua; DestDir: {app}\LuaScripts; Flags: IgnoreVersion recursesubdirs promptifolder createallsubdirs; Components: Application; 
Source: ..\..\psycle\LuaScripts\*.*; DestDir: {app}\LuaScripts; Flags: IgnoreVersion recursesubdirs promptifolder createallsubdirs; Excludes: psycle, start.lua; Components: LuaScripts; 

[Icons]
Name: {group}\Psycle Modular Music Creation Studio 64; Filename: {app}\psycle.exe
Name: "{group}\{cm:ProgramOnTheWeb,Psycledelics site}"; Filename: http://psycle.pastnotecut.org/; 
Name: "{group}\{cm:ProgramOnTheWeb,Psycle development}"; Filename: http://sourceforge.net/projects/psycle/; 
Name: "{group}\{cm:UninstallProgram,Psycle 64}"; Filename: {uninstallexe};
Name: {commondesktop}\Psycle Modular Music Creation Studio 64; Filename: {app}\psycle.exe; Tasks: desktopicon
Name: {userappdata}\Microsoft\Internet Explorer\Quick Launch\Psycle Modular Music Creation Studio 64; Filename: {app}\psycle.exe; Tasks: quicklaunchicon
Name: "{group}\Documents\How to Skin Psycle"; Filename: {app}\Docs\how-to-skin-psycle.txt; WorkingDir: {app}\Docs; Components: Documentation; 
Name: {group}\Documents\Version History; Filename: {app}\Docs\whatsnew.txt; WorkingDir: {app}\Docs; Components: Documentation
Name: {group}\Documents\Pattern Keys; Filename: {app}\Docs\keys.txt; WorkingDir: {app}\Docs; Components: Documentation
Name: "{group}\Documents\MIDI Help"; Filename: {app}\Docs\psycle-midi-2.2b\psyclemidi.html; WorkingDir: {app}\Docs\psycle-midi-2.2b; Components: Documentation; 
Name: "{group}\Documents\Tweakings And Commands"; Filename: {app}\Docs\tweaking.txt; WorkingDir: {app}\Docs\;  Components: Documentation
Name: "{group}\Documents\Lua Scripting Manual"; Filename: {app}\Docs\LuaScriptingManual.pdf; Components: Documentation; 
Name: "{group}\Demo Songs Folder"; Filename: "{commondocs}\Psycle Songs\"; 

[Run]
Filename: {app}\psycle.exe; Description: {cm:LaunchProgram,Psycle Modular Music Creation Studio}; Flags: nowait postinstall skipifsilent; Tasks: ; Components: Application
Filename: {tmp}\Vst-Bundle.exe; WorkingDir: {tmp}; StatusMsg: Select the location of your 32bit VST Plugins Dir (use Psycle\VstPlugins if in doubt); Flags: runascurrentuser; Components: " VstPack"; Tasks: ; Languages: 
Filename: {tmp}\vcredist_x64.exe; WorkingDir: {tmp}; Flags: 64bit runascurrentuser; Components: InstallMSRuntimes_x64

[Components]
Name: Application; Description: Main Application and needed dlls; Flags: fixed; Types: custom compact full; Languages: 
Name: InstallMSRuntimes_x64; Description: Download and Install Microsoft Visual c++ 2008 runtimes (x64); Types: custom full; ExtraDiskSpaceRequired: 5000000; Languages: ; Check: Is64bitInstallMode
Name: Open_Source_Plugins; Description: Install those plugins which the developer made their sources available; Types: custom compact full
Name: VstPack; Description: Download and install the freely available VST Pack.; Types: custom full; ExtraDiskSpaceRequired: 19038208
Name: Presets; Description: Install presets for selected plugins (does not overwrite existing files); Types: custom full
Name: Documentation; Description: Install the documentation of the project; Types: custom full
Name: Skins; Description: Skins that change the look and feel of Psycle; Types: custom full
Name: Demo_Songs; Description: Demo songs to show what psycle can do; Types: custom full
Name: LuaScripts; Description: Lua utility and example scripts; Types: custom compact full
[Dirs]
Name: {app}\PsyclePlugins; Flags: uninsalwaysuninstall; Components: Open_Source_Plugins; 
Name: {app}\Skins; Flags: uninsalwaysuninstall; Components: Skins; 
Name: {app}\VstPlugins; Components: Application; 
Name: {app}\VstPlugins64; Components: Application; 
Name: {app}\Docs; Flags: uninsalwaysuninstall; Components: Documentation; 
Name: "{commondocs}\Psycle Songs"; Components: " Demo_Songs"; 
Name: {app}\LuaScripts; Flags: UninsAlwaysUninstall; Components: LuaScripts; 
[Registry]
Root: HKCU; Subkey: software\psycle; Flags: uninsdeletekey
Root: HKCU; Subkey: software\psycle; Tasks: " deleteRegistrySettings"; Flags: deletekey
Root: HKCU; Subkey: software\AAS\Psycle; Flags: deletekey; Tasks: " deleteRegistrySettings"
Root: HKCR; Subkey: .psy; ValueType: string; ValueData: Psycle.Music.File; Flags: uninsdeletekey
Root: HKCR; SubKey: Psycle.Music.File; ValueType: none; Flags: uninsdeletekey
Root: HKCR; SubKey: Psycle.Music.File\Shell\Open\Command; ValueType: string; ValueData: """{app}\psycle.exe"" ""%1"""; Flags: uninsdeletevalue
Root: HKCR; Subkey: Psycle.Music.File\DefaultIcon; ValueType: string; ValueData: """{app}\psycle.exe"",0"; Flags: uninsdeletevalue
[InstallDelete]
Name: {userappdata}\..\.psycle\; Type: filesandordirs; Tasks: " deleteCacheFile"; Components: Application
Name: {win}\Psyclekeys.ini; Type: files; Tasks: " deleteCacheFile"; Components: Application
Name: {userappdata}\psycle\; Type: filesandordirs; Tasks: " deleteCacheFile"; Components: Application
[UninstallDelete]
Name: {userappdata}\..\.psycle\; Type: filesandordirs; Components: Application
Name: {userappdata}\psycle\; Type: filesandordirs; Components: Application
[Code]
procedure InitializeWizard();
begin
  ITD_Init();
  ITD_DownloadAfter(wpReady);
end;

function NextButtonClick(CurPage: Integer): Boolean;
begin
  if CurPage = wpSelectTasks then
  begin
    ITD_ClearFiles();
    if IsComponentSelected('VstPack') then
    begin
       ITD_AddFile('http://heanet.dl.sourceforge.net/project/psycle/psycle/1.8.5/psycle-1.8.5-vst-bundle.exe',ExpandConstant('{tmp}\Vst-Bundle.exe'));
    end;
    if IsComponentSelected('InstallMSRuntimes_x64') then
    begin
      ITD_AddFile('http://download.microsoft.com/download/2/d/6/2d61c766-107b-409d-8fba-c39e61ca08e8/vcredist_x64.exe',ExpandConstant('{tmp}\vcredist_x64.exe'));
    end;
  end;
  Result := True;
end;

function Is32bitInstallMode() : Boolean;
begin
	if Is64BitInstallMode then
		Result := False
	else
		Result := True;
end;

[InnoIDE_Settings]
UseRelativePaths=true

[Types]
Name: compact; Description: "Minimal installation"; 
Name: custom; Description: "Custom installation"; Flags: IsCustom;
Name: full; Description: "Full installation";

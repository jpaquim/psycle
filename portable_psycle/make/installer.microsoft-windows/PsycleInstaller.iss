; Script generated by the Inno Setup Script Wizard.
; SEE THE DOCUMENTATION FOR DETAILS ON CREATING INNO SETUP SCRIPT FILES!

[Setup]
AppName=Psycle Modular Music Creation Studio
AppVerName=Psycle 1.8
AppPublisher=Psycledelics
AppPublisherURL=http://psycle.pastnotecut.org/
AppSupportURL=http://psycle.pastnotecut.org/
AppUpdatesURL=http://psycle.pastnotecut.org/
DefaultDirName={pf}\Psycle
DefaultGroupName=Psycle
AllowNoIcons=true
InfoBeforeFile=I:\Documents and Settings\All Users\Documentos\Visual C++\portable_psycle\doc\for-end-users\readme.txt
InfoAfterFile=I:\Documents and Settings\All Users\Documentos\Visual C++\portable_psycle\doc\for-end-users\whatsnew.txt
OutputBaseFilename=PsycleInstaller
SetupIconFile=I:\Documents and Settings\All Users\Documentos\Visual C++\portable_psycle\src\psycle\host\resources\Psycle.ico
Compression=lzma
SolidCompression=true
AppCopyright=� Psycledelics 2000-2005
ChangesAssociations=true
ShowLanguageDialog=no
WizardImageFile=compiler:wizmodernimage-IS.bmp
WizardSmallImageFile=compiler:wizmodernsmallimage-IS.bmp
AppVersion=1.8
AppID={{8E7D0A7F-B85F-44DC-8C1C-2A2C27BAEA0B}
UninstallDisplayIcon={app}\psycle.exe

[Languages]
Name: eng; MessagesFile: compiler:Default.isl

[Tasks]
Name: desktopicon; Description: {cm:CreateDesktopIcon}; GroupDescription: {cm:AdditionalIcons}; Flags: unchecked
Name: quicklaunchicon; Description: {cm:CreateQuickLaunchIcon}; GroupDescription: {cm:AdditionalIcons}; Flags: unchecked
Name: Clear_Plugin_Cache; Description: Delete the plugin cache. Useful to reparse all the information; Flags: unchecked
Name: Clear_Registry_Settings; Description: Delete the configuration data from the Windows Registry. Useful to restore to the default settings; Flags: unchecked

[Files]
Source: ..\msvc_7.1\output\release.g7\bin\psycle.exe; DestDir: {app}; Flags: ignoreversion; Components: Application
Source: ..\msvc_7.1\output\release.g7\bin\psycle.plugins\*.dll; DestDir: {app}\PsyclePlugins; Flags: ignoreversion; Components: Open_Source_Plugins
Source: ..\msvc_7.1\output\release.g7\bin\msvcp71.dll; DestDir: {app}; Flags: ignoreversion; Components: Microsoft_dlls; Tasks: ; Languages: 
Source: ..\msvc_7.1\output\release.g7\bin\msvcr71.dll; DestDir: {app}; Flags: ignoreversion; Components: Microsoft_dlls
Source: ..\msvc_7.1\output\release.g7\bin\asio.dll; DestDir: {app}; Flags: ignoreversion; Components: Application
Source: ..\msvc_7.1\output\release.g7\bin\boost_filesystem-vc71-mt-1_32.dll; DestDir: {app}; Flags: ignoreversion; Components: Application
Source: ..\msvc_7.1\output\release.g7\bin\boost_thread-vc71-mt-1_32.dll; DestDir: {app}; Flags: ignoreversion; Components: Application
Source: ..\msvc_7.1\output\release.g7\bin\MFC71.dll; DestDir: {app}; Flags: ignoreversion; Components: Microsoft_dlls; Tasks: ; Languages: 
Source: ..\..\doc\for-end-users\*; DestDir: {app}\Docs; Flags: ignoreversion recursesubdirs createallsubdirs; Components: Documentation
; NOTE: Don't use "Flags: ignoreversion" on any shared system files
Source: Psycle.exe.manifest; DestDir: {app}; Components: " Windows_XP_theme_support"
Source: ..\..\closed-source\*; DestDir: {app}\PsyclePlugins\closed-source; Flags: ignoreversion createallsubdirs recursesubdirs; Components: Closed_Source_Plugins
Source: ..\msvc_7.1\output\release.g7\bin\Skins\*; DestDir: {app}\Skins; Flags: confirmoverwrite recursesubdirs ignoreversion createallsubdirs; Components: Skins

[INI]
Filename: {app}\psycle.url; Section: InternetShortcut; Key: URL; String: http://psycle.pastnotecut.org; Flags: uninsdeleteentry uninsdeletesectionifempty; Tasks: ; Languages: 

[Icons]
Name: {group}\Psycle Modular Music Creation Studio; Filename: {app}\psycle.exe; IconIndex: 0; Components: Application
Name: {group}\{cm:ProgramOnTheWeb,Psycle Modular Music Creation Studio}; Filename: {app}\psycle.url
Name: {group}\{cm:UninstallProgram,Psycle Modular Music Creation Studio}; Filename: {uninstallexe}
Name: {userdesktop}\Psycle Modular Music Creation Studio; Filename: {app}\psycle.exe; Tasks: desktopicon; IconIndex: 0; Components: Application
Name: {userappdata}\Microsoft\Internet Explorer\Quick Launch\Psycle Modular Music Creation Studio; Filename: {app}\psycle.exe; Tasks: quicklaunchicon
Name: {group}\Documents\How to Skin Psycle; Filename: {app}\Docs\how to skin psycle.txt; WorkingDir: {app}\Docs; IconIndex: 0; Components: Documentation
Name: {group}\Documents\Version History; Filename: {app}\Docs\whatsnew.txt; WorkingDir: {app}\Docs; IconIndex: 0; Components: Documentation
Name: {group}\Documents\Pattern Keys; Filename: {app}\Docs\keys.txt; WorkingDir: {app}\Docs; IconIndex: 0; Components: Documentation
Name: {group}\Documents\MIDI Help; Filename: {app}\Docs\MIDI Help\psyclemidi.html; IconIndex: 0; WorkingDir: {app}\Docs\MIDI input Docs; Components: Documentation
Name: {group}\Documents\Upwego Tutorial; Filename: {app}\Docs\Upwego\upwego5.html; IconIndex: 0; WorkingDir: {app}\Docs\MIDI input Docs; Components: Documentation
Name: {group}\Documents\Tweakings And Commands; Filename: {app}\Docs\tweakings and commands.txt; IconIndex: 0

[Run]
Filename: {app}\psycle.exe; Description: {cm:LaunchProgram,Psycle Modular Music Creation Studio}; Flags: nowait postinstall skipifsilent

[UninstallDelete]
Type: files; Name: {app}\psycle.url
Name: {app}\psycle.plugin-scan.cache; Type: files; Components: ; Tasks: ; Languages: 
Name: {app}\psycle.plugin-scan.log.txt; Type: files
Name: {app}\psycle.log.txt; Type: files
Name: {app}; Type: dirifempty
[_ISTool]
UseAbsolutePaths=false
[Registry]
Root: HKCU; Subkey: software\psycle; Flags: uninsdeletekey
Root: HKCR; Subkey: .psy; Flags: uninsdeletekey; ValueType: string; ValueData: Psycle Music Archive
Root: HKCR; Subkey: Psycle Music Archive; Flags: uninsdeletekey; ValueType: string; ValueData: Psycle Music Archive
Root: HKCR; Subkey: Psycle Music Archive\Shell\Open\Command; ValueType: string; ValueData: """""{app}\Psycle.exe"""" """"%1"""""; Flags: uninsdeletevalue
Root: HKCR; Subkey: Psycle Music Archive\DefaultIcon; ValueType: string; ValueData: {app}\Psycle.exe; Flags: uninsdeletevalue
[Types]
Name: Full_Instalation; Description: Install all the components provided with the installer
Name: Custom; Description: Custom Installation; Flags: iscustom
[Components]
Name: Application; Description: Installs the executable file; Types: Full_Instalation Custom; Flags: fixed
Name: Documentation; Description: Installs the documentation provided for Psycle; Types: Custom Full_Instalation
Name: Open_Source_Plugins; Description: Installs the Psycle plugins of which the source code is open.; Types: Custom Full_Instalation
Name: Closed_Source_Plugins; Description: Installs the rest of Native Plugins, which includes those which the source code isn't open; Types: Full_Instalation
Name: Windows_XP_theme_support; Description: "Installs the ""manifest"" file which makes psycle show an interface with XP style."; Types: Full_Instalation
Name: Microsoft_dlls; Description: Installs the redistributable dll's from Microsoft for use with C++ applications; Types: Full_Instalation
Name: Skins; Description: Adds some skins for Psycle; Types: Full_Instalation
[Dirs]
Name: {app}\Skins; Components: " Skins"; Tasks: ; Languages: 
Name: {app}\Songs
Name: {app}\VstPlugins
Name: {app}\PsyclePlugins
Name: {app}\Docs; Components: Documentation
[Code]
program Setup;

function NextButtonClick(CurPage: Integer): Boolean;
var
  ExecResult: Integer;
  ApplPath, Tasks, ExecFile, ExecParam, CacheFile: String;
begin
  if CurPage = wpReady then
  begin

	  Tasks := WizardSelectedTasks(false)
	  if Pos('Clear_Registry_Settings', Tasks)>0 then
	  begin
		if RegKeyExists(HKEY_CURRENT_USER, 'Software\psycle') then
			RegDeleteKeyIncludingSubkeys(HKEY_CURRENT_USER, 'Software\psycle')
	  end;
      if Pos('Clear_Plugin_Cache', Tasks)>0 then
      begin
        CacheFile := ApplPath + 'psycle.plugin-scan.cache';
        Delete(CacheFile,1,1);
        CacheFile := ApplPath + 'psycle.plugin-scan.log.txt';
        Delete(CacheFile,1,1);
        CacheFile := ApplPath + 'psycle.log.txt';
        Delete(CacheFile,1,1);
      end;
  end;
  Result := True;
end;



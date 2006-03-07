#ifndef _CONFIGURATION_H
#define _CONFIGURATION_H
/** @file
 *  @brief header file
 *  $Date: 2005/01/15 22:44:19 $
 *  $Revision: 1.5 $
 */

#include "AudioDriver.h"
#include "IConfiguration.h"
#include "iController.h"

#define CONFIG_ROOT_KEY _T("Software\\AAS\\Psycle\\CurrentVersion")
#define SOFTWARE_ROOT_KEY _T("Software\\AAS\\Psycle")
#define CONFIG_KEY _T("CurrentVersion")
#define DEFAULT_INSTRUMENT_DIR _T("Instruments")
#define DEFAULT_SONG_DIR _T("Songs")
#define DEFAULT_PLUGIN_DIR _T("Plugins")
#define DEFAULT_VST_DIR _T("Vst")
#define DEFAULT_SKIN_DIR _T("Skins")
#define DEFAULT_PATTERN_HEADER_SKIN _T("Psycle Default (internal)")
#define DEFAULT_MACHINE_SKIN _T("Psycle Default (internal)")

class CMidiInput;	// MIDI IMPLEMENTATION
namespace configuration {

class configuration : public IConfiguration
{
public:
	configuration(iController *pController);
	~configuration();

//	void CreateFonts();
//	const bool CreatePsyFont(CFont&f, const TCHAR* sFontFace,const int HeightPx, const bool bBold, const bool bItalic);
	const bool Initialized() { return _initialized; }

	string& InstrumentDir() { return m_InstrumentDir; };
	void InstrumentDir(const string& dir){m_InstrumentDir = dir;};

	string& InitialInstrumentDir() { return m_InitialInstrumentDir; };
	void InitialInstrumentDir(const string& dir){m_InitialInstrumentDir = dir;};

	string& SongDir(void) { return m_SongDir; };
	void SongDir(const string& dir){m_SongDir = dir;};

	string& InitialSongDir(void) { return m_InitialSongDir; };
	void InitialSongDir(const string& dir){m_InitialSongDir = dir;};

	void SkinDir(const string& dir){m_SkinDir = dir;};
	string& SkinDir(void) { return m_SkinDir; };

	void InitialSkinDir(const string& dir){m_InitialSkinDir = dir;};
	string& InitialSkinDir(void) { return m_InitialSkinDir; };

	void PluginDir(const string& dir);
	string& PluginDir(void) { return m_PluginDir; };

	void InitialPluginDir(const string& dir);
	string& InitialPluginDir(void) { return m_InitialPluginDir; }

	void VstDir(const string& dir){m_VstDir = dir;};
	string& VstDir(void) { return m_VstDir; };

	void InitialVstDir(const string& dir){m_InitialVstDir = dir;};
	string& InitialVstDir(void) { return m_InitialVstDir; };

	void IsFileSaveRemind(const bool value){m_bFileSaveReminders = value;};
	const bool IsFileSaveRemind(){return m_bFileSaveReminders;};

	void IsShowSongInfoOnLoad(const bool value){bShowSongInfoOnLoad = value;};
	const bool IsShowSongInfoOnLoad(){return bShowSongInfoOnLoad ;};

private:
	iController * m_pController;
	bool _initialized;

	string m_InitialInstrumentDir;
	string m_InstrumentDir;
	string m_InitialSongDir;
	string m_SongDir;
	string m_InitialSkinDir;
	string m_SkinDir;
	string m_InitialPluginDir;
	string m_PluginDir;
	string m_InitialVstDir;
	string m_VstDir;

	bool autoStopMachines;

	COLORREF vu1;
	COLORREF vu2;
	COLORREF vu3;

	bool _midiMachineViewSeqMode;

	bool _RecordUnarmed;
	bool _RecordMouseTweaksSmooth;

	bool useDoubleBuffer;

	bool _showAboutAtStart;

	CFont seqFont;

	int _outputDriverIndex;

	bool bShowSongInfoOnLoad;
	bool m_bFileSaveReminders;

	TCHAR appPath[_MAX_PATH];
};
}

#endif
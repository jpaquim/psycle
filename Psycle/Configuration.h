#ifndef _CONFIGURATION_H
#define _CONFIGURATION_H

#if !defined(_WINAMP_PLUGIN_)
	#include "AudioDriver.h"
#endif // ndef _WINAMP_PLUGIN_

#define CONFIG_ROOT_KEY "Software\\AAS\\Psycle\\CurrentVersion"
#define SOFTWARE_ROOT_KEY "Software\\AAS\\Psycle"
#define CONFIG_KEY "CurrentVersion"
#define DEFAULT_INSTRUMENT_DIR "Instruments"
#define DEFAULT_SONG_DIR "Songs"
#define DEFAULT_PLUGIN_DIR "Plugins"
#define DEFAULT_VST_DIR "Vst"
#define DEFAULT_SKIN_DIR "Skins"
#define DEFAULT_PATTERN_HEADER_SKIN "Psycle Default (internal)"
#define DEFAULT_MACHINE_SKIN "Psycle Default (internal)"

#if !defined(_WINAMP_PLUGIN_)
	class CMidiInput;	// MIDI IMPLEMENTATION 
#endif // ndef _WINAMP_PLUGIN_

class Configuration
{
public:
	void CreateFonts();

bool CreatePsyFont(CFont&f, char*sFontFace,int HeightPx, bool bBold, bool bItalic);

#if !defined(_WINAMP_PLUGIN_)

	bool autoStopMachines;
	COLORREF mv_colour;
	COLORREF mv_wirecolour;
	COLORREF mv_wireaacolour;
	COLORREF mv_wireaacolour2;
	COLORREF mv_polycolour;
	COLORREF mv_generator_fontcolour;
	COLORREF mv_effect_fontcolour;

	COLORREF pvc_separator;
	COLORREF pvc_separator2;
	COLORREF pvc_background;
	COLORREF pvc_background2;
	COLORREF pvc_row4beat;
	COLORREF pvc_row4beat2;
	COLORREF pvc_rowbeat;
	COLORREF pvc_rowbeat2;
	COLORREF pvc_row;
	COLORREF pvc_row2;
	COLORREF pvc_font;
	COLORREF pvc_font2;
	COLORREF pvc_fontPlay;
	COLORREF pvc_fontPlay2;
	COLORREF pvc_fontCur;
	COLORREF pvc_fontCur2;
	COLORREF pvc_fontSel;
	COLORREF pvc_fontSel2;
	COLORREF pvc_selection;
	COLORREF pvc_selection2;
	COLORREF pvc_playbar;
	COLORREF pvc_playbar2;
	COLORREF pvc_cursor;
	COLORREF pvc_cursor2;

	COLORREF vu1;
	COLORREF vu2;
	COLORREF vu3;

	int mv_wireaa;
	int mv_wirewidth;
	bool _wrapAround;
	bool _centerCursor;
	bool _cursorAlwaysDown;
	bool _midiMachineViewSeqMode;
	bool _RecordNoteoff;
	bool _RecordTweaks;
	bool _notesToEffects;
	bool _RecordUnarmed;
	bool _RecordMouseTweaksSmooth;
	bool useDoubleBuffer;
	bool _showAboutAtStart;

	int mv_triangle_size;

	bool bBmpBkg;
	char szBmpBkgFilename[MAX_PATH];

	char pattern_fontface[64];
	char pattern_header_skin[64];
	int pattern_font_point;
	int pattern_font_x;
	int pattern_font_y;

	UINT pattern_font_flags;
	UINT generator_font_flags;
	UINT effect_font_flags;

	bool pattern_draw_empty_data;
	bool draw_mac_index;

	char generator_fontface[64];
	int generator_font_point;
	char effect_fontface[64];
	int effect_font_point;

	char machine_skin[64];

	CFont seqFont;
	CFont generatorFont;
	CFont effectFont;

	bool _midiRecordVel;
	int _midiTypeVel;
	int _midiCommandVel;
	int _midiFromVel;
	int _midiToVel;

	bool _midiRecordPit;
	int _midiTypePit;
	int _midiCommandPit;
	int _midiFromPit;
	int _midiToPit;

	bool _midiRecord0;
	int _midiType0;
	int _midiMessage0;
	int _midiCommand0;
	int _midiFrom0;
	int _midiTo0;

	bool _midiRecord1;
	int _midiType1;
	int _midiMessage1;
	int _midiCommand1;
	int _midiFrom1;
	int _midiTo1;

	bool _midiRecord2;
	int _midiType2;
	int _midiMessage2;
	int _midiCommand2;
	int _midiFrom2;
	int _midiTo2;

	bool _midiRecord3;
	int _midiType3;
	int _midiMessage3;
	int _midiCommand3;
	int _midiFrom3;
	int _midiTo3;

	bool _midiRecord4;
	int _midiType4;
	int _midiMessage4;
	int _midiCommand4;
	int _midiFrom4;
	int _midiTo4;

	bool _midiRecord5;
	int _midiType5;
	int _midiMessage5;
	int _midiCommand5;
	int _midiFrom5;
	int _midiTo5;

	bool _midiRecord6;
	int _midiType6;
	int _midiMessage6;
	int _midiCommand6;
	int _midiFrom6;
	int _midiTo6;

	bool _midiRecord7;
	int _midiType7;
	int _midiMessage7;
	int _midiCommand7;
	int _midiFrom7;
	int _midiTo7;

	bool _midiRecord8;
	int _midiType8;
	int _midiMessage8;
	int _midiCommand8;
	int _midiFrom8;
	int _midiTo8;

	bool _midiRecord9;
	int _midiType9;
	int _midiMessage9;
	int _midiCommand9;
	int _midiFrom9;
	int _midiTo9;

	bool _midiRecord10;
	int _midiType10;
	int _midiMessage10;
	int _midiCommand10;
	int _midiFrom10;
	int _midiTo10;

	bool _midiRecord11;
	int _midiType11;
	int _midiMessage11;
	int _midiCommand11;
	int _midiFrom11;
	int _midiTo11;

	bool _midiRecord12;
	int _midiType12;
	int _midiMessage12;
	int _midiCommand12;
	int _midiFrom12;
	int _midiTo12;

	bool _midiRecord13;
	int _midiType13;
	int _midiMessage13;
	int _midiCommand13;
	int _midiFrom13;
	int _midiTo13;

	bool _midiRecord14;
	int _midiType14;
	int _midiMessage14;
	int _midiCommand14;
	int _midiFrom14;
	int _midiTo14;

	bool _midiRecord15;
	int _midiType15;
	int _midiMessage15;
	int _midiCommand15;
	int _midiFrom15;
	int _midiTo15;

	bool _linenumbers;
	bool _linenumbersHex;
	bool _linenumbersCursor;
	bool _followSong;

	int _numOutputDrivers;
	int _outputDriverIndex;
	int _midiDriverIndex;		// MIDI IMPLEMENTATION
	int _syncDriverIndex;
	int _midiHeadroom;

	int defaultPatLines;
	bool bShowSongInfoOnLoad;
	bool bFileSaveReminders;
	bool autosaveSong;
	int autosaveSongTime;

	AudioDriver** _ppOutputDrivers;
	AudioDriver* _pOutputDriver;
	CMidiInput* _pMidiInput;	// MIDI IMPLEMENTATION

#else
	int _samplesPerSec;
#endif // ndef _WINAMP_PLUGIN_

	Configuration();
	~Configuration();

	bool Initialized() { return _initialized; }
	bool Read();
	void Write();

	inline const int GetSamplesPerSec() const throw()
	{
		return
#if !defined(_WINAMP_PLUGIN_)
			_pOutputDriver->
#endif
		_samplesPerSec;
	}

#if !defined(_WINAMP_PLUGIN_)

	char* GetInstrumentDir() { return _psInstrumentDir; }
	char* GetInitialInstrumentDir() { return _psInitialInstrumentDir; }
	void SetInstrumentDir(const char* psDir);
	void SetInitialInstrumentDir(const char* psDir);
	char* GetSongDir(void) { return _psSongDir; }
	char* GetInitialSongDir(void) { return _psInitialSongDir; }
	void SetSongDir(const char* psDir);
	void SetInitialSongDir(const char* psDir);
	void SetSkinDir(const char* psDir);
	void SetInitialSkinDir(const char* psDir);
	char* GetSkinDir(void) { return _psSkinDir; }
	char* GetInitialSkinDir(void) { return _psInitialSkinDir; }

#endif // ndef _WINAMP_PLUGIN_

	void SetPluginDir(const char* psDir);
	void SetInitialPluginDir(const char* psDir);
	char* GetPluginDir(void) { return _psPluginDir; }
	char* GetInitialPluginDir(void) { return _psInitialPluginDir; }
	void SetVstDir(const char* psDir);
	void SetInitialVstDir(const char* psDir);
	char* GetVstDir(void) { return _psVstDir; }
	char* GetInitialVstDir(void) { return _psInitialVstDir; }

	bool _initialized;
protected:

#if !defined(_WINAMP_PLUGIN_)
	char* _psInitialInstrumentDir;
	char* _psInstrumentDir;
	char* _psInitialSongDir;
	char* _psSongDir;
	char* _psInitialSkinDir;
	char* _psSkinDir;
#endif // ndef _WINAMP_PLUGIN_

	char* _psInitialPluginDir;
	char* _psPluginDir;
	char* _psInitialVstDir;
	char* _psVstDir;

	void Error(	char const* psMsg);

};

#endif
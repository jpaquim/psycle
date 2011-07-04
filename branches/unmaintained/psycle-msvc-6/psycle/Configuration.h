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

#if !defined(_WINAMP_PLUGIN_)
	class CMidiInput;	// MIDI IMPLEMENTATION 
#endif // ndef _WINAMP_PLUGIN_

class Configuration
{
public:

#if !defined(_WINAMP_PLUGIN_)

	bool autoStopMachines;
	COLORREF mv_colour;
	COLORREF pvc_background;
	COLORREF pvc_row4beat;
	COLORREF pvc_rowbeat;
	COLORREF pvc_row;
	COLORREF pvc_font;
	COLORREF gen_colour;
	COLORREF eff_colour;
	COLORREF mas_colour;
	COLORREF plg_colour;
	COLORREF vu1;
	COLORREF vu2;
	COLORREF vu3;
	int _keyboardMode; // Currently Not Used (was used for hardcoded keyb types)
	bool _wrapAround;
	bool _centerCursor;
	bool _cursorAlwaysDown;
	bool _midiMachineViewSeqMode;
	bool _midiRecordNoteoff;
	bool useDoubleBuffer;
	bool _showAboutAtStart;

	int _numOutputDrivers;
	int _outputDriverIndex;
	int _midiDriverIndex;		// MIDI IMPLEMENTATION
	int _syncDriverIndex;
	int _midiHeadroom;
	AudioDriver** _ppOutputDrivers;
	AudioDriver* _pOutputDriver;
	CMidiInput* _pMidiInput;	// MIDI IMPLEMENTATION

#else
	int _samplesPerSec;
#endif // ndef _WINAMP_PLUGIN_

	Configuration();
	~Configuration();

	bool Initialized(
		void) { return _initialized; }
	bool Read(
		void);
	void Write(
		void);

#if !defined(_WINAMP_PLUGIN_)

	char* GetInstrumentDir(
		void) { return _psInstrumentDir; }
	char* GetInitialInstrumentDir(
		void) { return _psInitialInstrumentDir; }
	void SetInstrumentDir(
		const char* psDir);
	void SetInitialInstrumentDir(
		const char* psDir);
	char* GetSongDir(
		void) { return _psSongDir; }
	char* GetInitialSongDir(
		void) { return _psInitialSongDir; }
	void SetSongDir(
		const char* psDir);
	void SetInitialSongDir(
		const char* psDir);

#endif // ndef _WINAMP_PLUGIN_

	void SetPluginDir(
		const char* psDir);
	void SetInitialPluginDir(
		const char* psDir);
	char* GetPluginDir(
		void) { return _psPluginDir; }
	char* GetInitialPluginDir(
		void) { return _psInitialPluginDir; }
	void SetVstDir(
		const char* psDir);
	void SetInitialVstDir(
		const char* psDir);
	char* GetVstDir(
		void) { return _psVstDir; }
	char* GetInitialVstDir(
		void) { return _psInitialVstDir; }

protected:
	bool _initialized;

#if !defined(_WINAMP_PLUGIN_)
	char* _psInitialInstrumentDir;
	char* _psInstrumentDir;
	char* _psInitialSongDir;
	char* _psSongDir;
#endif // ndef _WINAMP_PLUGIN_

	char* _psInitialPluginDir;
	char* _psPluginDir;
	char* _psInitialVstDir;
	char* _psVstDir;

	void Error(
		char const* psMsg);

};

#endif
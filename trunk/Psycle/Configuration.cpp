#include "stdafx.h"

#if defined(_WINAMP_PLUGIN_)
//	#include <windows.h>
	#include "Configuration.h"
	#include "Registry.h"
#else
	#include "Configuration.h"
	#include "Registry.h"
	#include "WaveOut.h"
	#include "DirectSound.h"
	#include "PortAudioASIO.h"
	#include "MidiInput.h"
#include "Song.h"
#include "NewMachine.h"
#endif //_WINAMP_PLUGIN_


Configuration::Configuration()
{
	_initialized = false;

#if defined(_WINAMP_PLUGIN_)
	_psInitialPluginDir = NULL;
	_psPluginDir = NULL;
	_psInitialVstDir = NULL;
	_psVstDir = NULL;
	_samplesPerSec = 44100;
#else
	_wrapAround = true;
	_centerCursor = false;
	_cursorAlwaysDown = false;
	_RecordNoteoff = false;
	_RecordTweaks = false;
	_notesToEffects = false;
	_RecordMouseTweaksSmooth = FALSE;
	_RecordUnarmed = true;
	_midiMachineViewSeqMode = false;
	autoStopMachines = false;
	useDoubleBuffer = true;
	_linenumbers = true;
	_linenumbersHex = false;
	_linenumbersCursor = false;
	_showAboutAtStart = true;
	_followSong = false;
	strcpy(pattern_fontface,"Tahoma");
	strcpy(pattern_header_skin,DEFAULT_PATTERN_HEADER_SKIN);
	pattern_font_point = 85;
	pattern_font_x = 9;
	pattern_font_y = 12;
	pattern_draw_empty_data = TRUE;
	draw_mac_index = TRUE;
	strcpy(generator_fontface,"Tahoma");
	generator_font_point = 90;
	strcpy(effect_fontface,"Tahoma");
	effect_font_point = 90;

	pattern_font_flags = 0;
	generator_font_flags = 0;
	effect_font_flags = 0;

	strcpy(machine_skin,DEFAULT_MACHINE_SKIN);

	mv_colour =	0x009a887c;
	mv_wirecolour =	0x00000000;
	mv_polycolour =	0x00ffffff;
	mv_generator_fontcolour = 0x00000000;
	mv_effect_fontcolour = 0x00000000;
	mv_wireaa = 1;
	mv_triangle_size = 22;
	mv_wirewidth = 1;
	mv_wireaacolour = ((((mv_wirecolour&0x00ff0000) + ((mv_colour&0x00ff0000)*4))/5)&0x00ff0000) +
			((((mv_wirecolour&0x00ff00) + ((mv_colour&0x00ff00)*4))/5)&0x00ff00) +
			((((mv_wirecolour&0x00ff) + ((mv_colour&0x00ff)*4))/5)&0x00ff);

	mv_wireaacolour2 = (((((mv_wirecolour&0x00ff0000)) + ((mv_colour&0x00ff0000)))/2)&0x00ff0000) +
			(((((mv_wirecolour&0x00ff00)) + ((mv_colour&0x00ff00)))/2)&0x00ff00) +
			(((((mv_wirecolour&0x00ff)) + ((mv_colour&0x00ff)))/2)&0x00ff);

	pvc_separator  = 0x00400000;
	pvc_separator2  = 0x00004000;
	pvc_background  = 0x009a887c;
	pvc_background2  = 0x00aa786c;
	pvc_row4beat  = 0x00d5ccc6;
	pvc_row4beat2 = 0x00fdfcf6;
	pvc_rowbeat  = 0x00c9beb8;
	pvc_rowbeat2 = 0x00f9eee8;
	pvc_row  = 0x00c1b5aa;
	pvc_row2 = 0x00f1e5da;
	pvc_font  = 0x00000000;
	pvc_font2  = 0x00000000;
	pvc_fontPlay  = 0x00ffffff;
	pvc_fontPlay2  = 0x00ffffff;
	pvc_fontCur  = 0x00ffffff;
	pvc_fontCur2  = 0x00ffffff;
	pvc_fontSel  = 0x00ffffff;
	pvc_fontSel2  = 0x00ffffff;
	pvc_selection  = 0x00e00000;
	pvc_selection2 = 0x00ff5050;
	pvc_playbar  = 0x0000e000;
	pvc_playbar2 = 0x005050e0;
	pvc_cursor  = 0x000000e0;
	pvc_cursor2 = 0x005050ff;

	// If you change the initial colour values, change it also in "CSkinDlg::OnResetcolours()"
	vu1 = 0x00f1c992;
	vu2 = 0x00403731;
	vu3 = 0x00262bd7;

	_numOutputDrivers = 4;
	_outputDriverIndex = 1;
	_midiDriverIndex = 0;			// MIDI IMPLEMENTATION
	_syncDriverIndex = 0;
	_midiHeadroom = 100;
	_ppOutputDrivers = new AudioDriver*[_numOutputDrivers];
	_ppOutputDrivers[0] = new AudioDriver;
	_ppOutputDrivers[1] = new WaveOut;
	_ppOutputDrivers[2] = new DirectSound;
	_ppOutputDrivers[3] = new PortAudioASIO;
	_pOutputDriver = _ppOutputDrivers[_outputDriverIndex];

	_pMidiInput = new CMidiInput;	// MIDI IMPLEMENTATION

	_psInitialInstrumentDir = NULL;
	_psInstrumentDir = NULL;
	_psInitialSongDir = NULL;
	_psSongDir = NULL;
	_psInitialPluginDir = NULL;
	_psPluginDir = NULL;
	_psInitialVstDir = NULL;
	_psVstDir = NULL;
	_psInitialSkinDir = NULL;
	_psSkinDir = NULL;

	_midiRecordVel = FALSE;
	_midiTypeVel = 0;
	_midiCommandVel = 0x0c;
	_midiFromVel = 0x00;
	_midiToVel = 0xff;

	_midiRecordPit = FALSE;
	_midiTypePit = 0;
	_midiCommandPit = 1;
	_midiFromPit = 0;
	_midiToPit = 0xff;

	_midiRecord0 = FALSE;
	_midiType0 = 0;
	_midiMessage0 = 1;
	_midiCommand0 = 1;
	_midiFrom0 = 0;
	_midiTo0 = 0xff;

	_midiRecord1 = FALSE;
	_midiType1 = 0;
	_midiMessage1 = 2;
	_midiCommand1 = 2;
	_midiFrom1 = 0;
	_midiTo1 = 0xff;

	_midiRecord2= FALSE;
	_midiType2 = 0;
	_midiMessage2 = 3;
	_midiCommand2 = 3;
	_midiFrom2 = 0;
	_midiTo2 = 0xff;

	_midiRecord3 = FALSE;
	_midiType3 = 0;
	_midiMessage3 = 4;
	_midiCommand3 = 4;
	_midiFrom3 = 0;
	_midiTo3 = 0xff;

	_midiRecord4 = FALSE;
	_midiType4 = 0;
	_midiMessage4 = 5;
	_midiCommand4 = 5;
	_midiFrom4 = 0;
	_midiTo4 = 0xff;

	_midiRecord5 = FALSE;
	_midiType5 = 0;
	_midiMessage5 = 6;
	_midiCommand5 = 6;
	_midiFrom5 = 0;
	_midiTo5 = 0xff;

	_midiRecord6 = FALSE;
	_midiType6 = 0;
	_midiMessage6 = 7;
	_midiCommand6 = 7;
	_midiFrom6 = 0;
	_midiTo6 = 0xff;

	_midiRecord7 = FALSE;
	_midiType7 = 0;
	_midiMessage7 = 8;
	_midiCommand7 = 8;
	_midiFrom7 = 0;
	_midiTo7 = 0xff;

	_midiRecord8 = FALSE;
	_midiType8 = 0;
	_midiMessage8 = 9;
	_midiCommand8 = 9;
	_midiFrom8 = 0;
	_midiTo8 = 0xff;

	_midiRecord9 = FALSE;
	_midiType9 = 0;
	_midiMessage9 = 10;
	_midiCommand9 = 10;
	_midiFrom9 = 0;
	_midiTo9 = 0xff;

	_midiRecord10 = FALSE;
	_midiType10 = 0;
	_midiMessage10 = 11;
	_midiCommand10 = 11;
	_midiFrom10 = 0;
	_midiTo10 = 0xff;

	_midiRecord11 = FALSE;
	_midiType11 = 0;
	_midiMessage11 = 12;
	_midiCommand11 = 12;
	_midiFrom11 = 0;
	_midiTo11 = 0xff;

	_midiRecord12 = FALSE;
	_midiType12 = 0;
	_midiMessage12 = 13;
	_midiCommand12 = 13;
	_midiFrom12 = 0;
	_midiTo12 = 0xff;

	_midiRecord13 = FALSE;
	_midiType13 = 0;
	_midiMessage13 = 14;
	_midiCommand13 = 14;
	_midiFrom13 = 0;
	_midiTo13 = 0xff;

	_midiRecord14 = FALSE;
	_midiType14 = 0;
	_midiMessage14 = 15;
	_midiCommand14 = 15;
	_midiFrom14 = 0;
	_midiTo14 = 0xff;

	_midiRecord15 = FALSE;
	_midiType15 = 0;
	_midiMessage15 = 16;
	_midiCommand15 = 16;
	_midiFrom15 = 0;
	_midiTo15 = 0xff;

	defaultPatLines = 64;
	bShowSongInfoOnLoad = true;
	bFileSaveReminders = true;
	autosaveSong = true;
	autosaveSongTime = 10;

	bBmpBkg = FALSE;
	sprintf(szBmpBkgFilename,"psycle.bmp");


#endif // _WINAMP_PLUGIN
}

Configuration::~Configuration()
{

#if !defined(_WINAMP_PLUGIN_)

	seqFont.DeleteObject();
	generatorFont.DeleteObject();
	effectFont.DeleteObject();
	if (_ppOutputDrivers != NULL)
	{
		for (int i=0; i<_numOutputDrivers; i++)
		{
			if (_ppOutputDrivers[i] != NULL)
			{
				delete _ppOutputDrivers[i];	//Deletes each Driver
			}
		}
		delete[] _ppOutputDrivers;	// And deletes the dinamically created table of pointers.
	}
	// MIDI IMPLEMENTATION
	if (_pMidiInput != NULL)
	{
		delete _pMidiInput;
	}

	if (_psInitialInstrumentDir != NULL)
	{
		delete _psInitialInstrumentDir;
	}
	if (_psInstrumentDir != NULL)
	{
		delete _psInstrumentDir;
	}
	if (_psInitialSongDir != NULL)
	{
		delete _psInitialSongDir;
	}
	if (_psSongDir != NULL)
	{
		delete _psSongDir;
	}
	if (_psInitialSkinDir != NULL)
	{
		delete _psInitialSkinDir;
	}
	if (_psSkinDir != NULL)
	{
		delete _psSkinDir;
	}
#endif // ndef _WINAMP_PLUGIN_

	if (_psInitialPluginDir != NULL)
	{
		delete _psInitialPluginDir;
	}
	if (_psPluginDir != NULL)
	{
		delete _psPluginDir;
	}
	if (_psInitialVstDir != NULL)
	{
		delete _psInitialVstDir;
	}
	if (_psVstDir != NULL)
	{
		delete _psVstDir;
	}
}

bool
Configuration::Read()
{
	bool existed = true;
	Registry reg;
	LONG result;
#if !defined(_WINAMP_PLUGIN_)
	int output;
	int midiInput;		// MIDI IMPLEMENTATION
	int syncDriver;
	int midiHeadroom;
#endif // ndef _WINAMP_PLUGIN_
	char string[256];
	DWORD type;
	DWORD numData;

	GetCurrentDirectory(sizeof(appPath), &appPath[0]);
	strcat(appPath, "\\");

	_initialized = true;
	if (reg.OpenRootKey(HKEY_CURRENT_USER, SOFTWARE_ROOT_KEY) != ERROR_SUCCESS)
	{
		return false;
	}
	result = reg.OpenKey(CONFIG_KEY);
	if (result != ERROR_SUCCESS)
	{
		if (result == ERROR_FILE_NOT_FOUND)
		{
			existed = false;
		}
		else
		{
			return false;
		}
	}
#if !defined(_WINAMP_PLUGIN_)

	
	numData = sizeof(CNewMachine::pluginOrder);
	reg.QueryValue("NewMacDlgpluginOrder", &type, (BYTE*)&CNewMachine::pluginOrder, &numData);
	numData = sizeof(CNewMachine::pluginName);
	reg.QueryValue("NewMacDlgpluginName", &type, (BYTE*)&CNewMachine::pluginName, &numData);

	numData = sizeof(_wrapAround);
	reg.QueryValue("WrapAround", &type, (BYTE*)&_wrapAround, &numData);
	numData = sizeof(_centerCursor);
	reg.QueryValue("CenterCursor", &type, (BYTE*)&_centerCursor, &numData);
	numData = sizeof(_followSong);
	reg.QueryValue("FollowSong", &type, (BYTE*)&_followSong, &numData);

	numData = sizeof(_cursorAlwaysDown);
	reg.QueryValue("CursorAlwaysDown", &type, (BYTE*)&_cursorAlwaysDown, &numData);
	numData = sizeof(useDoubleBuffer);
	reg.QueryValue("useDoubleBuffer", &type, (BYTE*)&useDoubleBuffer, &numData);
	numData = sizeof(_linenumbers);
	reg.QueryValue("DisplayLineNumbers", &type, (BYTE*)&_linenumbers, &numData);
	numData = sizeof(_linenumbersHex);
	reg.QueryValue("DisplayLineNumbersHex", &type, (BYTE*)&_linenumbersHex, &numData);
	numData = sizeof(_linenumbersCursor);
	reg.QueryValue("DisplayLineNumbersCursor", &type, (BYTE*)&_linenumbersCursor, &numData);
	numData = sizeof(_showAboutAtStart);
	reg.QueryValue("showAboutAtStart", &type, (BYTE*)&_showAboutAtStart, &numData);
	numData = sizeof(_RecordNoteoff);
	reg.QueryValue("RecordNoteoff", &type, (BYTE*)&_RecordNoteoff, &numData);
	numData = sizeof(_RecordTweaks);
	reg.QueryValue("RecordTweaks", &type, (BYTE*)&_RecordTweaks, &numData);
	numData = sizeof(_RecordMouseTweaksSmooth);
	reg.QueryValue("RecordMouseTweaksSmooth", &type, (BYTE*)&_RecordMouseTweaksSmooth, &numData);
	numData = sizeof(_notesToEffects);
	reg.QueryValue("notesToEffects", &type, (BYTE*)&_notesToEffects, &numData);
	numData = sizeof(_RecordUnarmed);
	reg.QueryValue("RecordUnarmed", &type, (BYTE*)&_RecordUnarmed, &numData);

	numData = sizeof(_midiMachineViewSeqMode);
	reg.QueryValue("MidiMachineViewSeqMode", &type, (BYTE*)&_midiMachineViewSeqMode, &numData);

	numData = sizeof(_midiRecordVel);
	reg.QueryValue("MidiRecordVel", &type, (BYTE*)&_midiRecordVel, &numData);
	numData = sizeof(_midiTypeVel);
	reg.QueryValue("MidiTypeVel", &type, (BYTE*)&_midiTypeVel, &numData);
	numData = sizeof(_midiCommandVel);
	reg.QueryValue("MidiCommandVel", &type, (BYTE*)&_midiCommandVel, &numData);
	numData = sizeof(_midiFromVel);
	reg.QueryValue("MidiFromVel", &type, (BYTE*)&_midiFromVel, &numData);
	numData = sizeof(_midiToVel);
	reg.QueryValue("MidiToVel", &type, (BYTE*)&_midiToVel, &numData);

	numData = sizeof(_midiRecordPit);
	reg.QueryValue("MidiRecordPit", &type, (BYTE*)&_midiRecordPit, &numData);
	numData = sizeof(_midiTypePit);
	reg.QueryValue("MidiTypePit", &type, (BYTE*)&_midiTypePit, &numData);
	numData = sizeof(_midiCommandPit);
	reg.QueryValue("MidiCommandPit", &type, (BYTE*)&_midiCommandPit, &numData);
	numData = sizeof(_midiFromPit);
	reg.QueryValue("MidiFromPit", &type, (BYTE*)&_midiFromPit, &numData);
	numData = sizeof(_midiToPit);
	reg.QueryValue("MidiToPit", &type, (BYTE*)&_midiToPit, &numData);

	numData = sizeof(_midiRecord0);
	reg.QueryValue("MidiRecord0", &type, (BYTE*)&_midiRecord0, &numData);
	numData = sizeof(_midiMessage0);
	reg.QueryValue("MidiMessage0", &type, (BYTE*)&_midiMessage0, &numData);
	numData = sizeof(_midiType0);
	reg.QueryValue("MidiType0", &type, (BYTE*)&_midiType0, &numData);
	numData = sizeof(_midiCommand0);
	reg.QueryValue("MidiCommand0", &type, (BYTE*)&_midiCommand0, &numData);
	numData = sizeof(_midiFrom0);
	reg.QueryValue("MidiFrom0", &type, (BYTE*)&_midiFrom0, &numData);
	numData = sizeof(_midiTo0);
	reg.QueryValue("MidiTo0", &type, (BYTE*)&_midiTo0, &numData);

	numData = sizeof(_midiRecord1);
	reg.QueryValue("MidiRecord1", &type, (BYTE*)&_midiRecord1, &numData);
	numData = sizeof(_midiMessage1);
	reg.QueryValue("MidiMessage1", &type, (BYTE*)&_midiMessage1, &numData);
	numData = sizeof(_midiType1);
	reg.QueryValue("MidiType1", &type, (BYTE*)&_midiType1, &numData);
	numData = sizeof(_midiCommand1);
	reg.QueryValue("MidiCommand1", &type, (BYTE*)&_midiCommand1, &numData);
	numData = sizeof(_midiFrom1);
	reg.QueryValue("MidiFrom1", &type, (BYTE*)&_midiFrom1, &numData);
	numData = sizeof(_midiTo1);
	reg.QueryValue("MidiTo1", &type, (BYTE*)&_midiTo1, &numData);

	numData = sizeof(_midiRecord2);
	reg.QueryValue("MidiRecord2", &type, (BYTE*)&_midiRecord2, &numData);
	numData = sizeof(_midiMessage2);
	reg.QueryValue("MidiMessage2", &type, (BYTE*)&_midiMessage2, &numData);
	numData = sizeof(_midiType2);
	reg.QueryValue("MidiType2", &type, (BYTE*)&_midiType2, &numData);
	numData = sizeof(_midiCommand2);
	reg.QueryValue("MidiCommand2", &type, (BYTE*)&_midiCommand2, &numData);
	numData = sizeof(_midiFrom2);
	reg.QueryValue("MidiFrom2", &type, (BYTE*)&_midiFrom2, &numData);
	numData = sizeof(_midiTo2);
	reg.QueryValue("MidiTo2", &type, (BYTE*)&_midiTo2, &numData);

	numData = sizeof(_midiRecord3);
	reg.QueryValue("MidiRecord3", &type, (BYTE*)&_midiRecord3, &numData);
	numData = sizeof(_midiMessage3);
	reg.QueryValue("MidiMessage3", &type, (BYTE*)&_midiMessage3, &numData);
	numData = sizeof(_midiType3);
	reg.QueryValue("MidiType3", &type, (BYTE*)&_midiType3, &numData);
	numData = sizeof(_midiCommand3);
	reg.QueryValue("MidiCommand3", &type, (BYTE*)&_midiCommand3, &numData);
	numData = sizeof(_midiFrom3);
	reg.QueryValue("MidiFrom3", &type, (BYTE*)&_midiFrom3, &numData);
	numData = sizeof(_midiTo3);
	reg.QueryValue("MidiTo3", &type, (BYTE*)&_midiTo3, &numData);

	numData = sizeof(_midiRecord4);
	reg.QueryValue("MidiRecord4", &type, (BYTE*)&_midiRecord4, &numData);
	numData = sizeof(_midiMessage4);
	reg.QueryValue("MidiMessage4", &type, (BYTE*)&_midiMessage4, &numData);
	numData = sizeof(_midiType4);
	reg.QueryValue("MidiType4", &type, (BYTE*)&_midiType4, &numData);
	numData = sizeof(_midiCommand4);
	reg.QueryValue("MidiCommand4", &type, (BYTE*)&_midiCommand4, &numData);
	numData = sizeof(_midiFrom4);
	reg.QueryValue("MidiFrom4", &type, (BYTE*)&_midiFrom4, &numData);
	numData = sizeof(_midiTo4);
	reg.QueryValue("MidiTo4", &type, (BYTE*)&_midiTo4, &numData);

	numData = sizeof(_midiRecord5);
	reg.QueryValue("MidiRecord5", &type, (BYTE*)&_midiRecord5, &numData);
	numData = sizeof(_midiMessage5);
	reg.QueryValue("MidiMessage5", &type, (BYTE*)&_midiMessage5, &numData);
	numData = sizeof(_midiType5);
	reg.QueryValue("MidiType5", &type, (BYTE*)&_midiType5, &numData);
	numData = sizeof(_midiCommand5);
	reg.QueryValue("MidiCommand5", &type, (BYTE*)&_midiCommand5, &numData);
	numData = sizeof(_midiFrom5);
	reg.QueryValue("MidiFrom5", &type, (BYTE*)&_midiFrom5, &numData);
	numData = sizeof(_midiTo5);
	reg.QueryValue("MidiTo5", &type, (BYTE*)&_midiTo5, &numData);

	numData = sizeof(_midiRecord6);
	reg.QueryValue("MidiRecord6", &type, (BYTE*)&_midiRecord6, &numData);
	numData = sizeof(_midiMessage6);
	reg.QueryValue("MidiMessage6", &type, (BYTE*)&_midiMessage6, &numData);
	numData = sizeof(_midiType6);
	reg.QueryValue("MidiType6", &type, (BYTE*)&_midiType6, &numData);
	numData = sizeof(_midiCommand6);
	reg.QueryValue("MidiCommand6", &type, (BYTE*)&_midiCommand6, &numData);
	numData = sizeof(_midiFrom6);
	reg.QueryValue("MidiFrom6", &type, (BYTE*)&_midiFrom6, &numData);
	numData = sizeof(_midiTo6);
	reg.QueryValue("MidiTo6", &type, (BYTE*)&_midiTo6, &numData);

	numData = sizeof(_midiRecord7);
	reg.QueryValue("MidiRecord7", &type, (BYTE*)&_midiRecord7, &numData);
	numData = sizeof(_midiMessage7);
	reg.QueryValue("MidiMessage7", &type, (BYTE*)&_midiMessage7, &numData);
	numData = sizeof(_midiType7);
	reg.QueryValue("MidiType7", &type, (BYTE*)&_midiType7, &numData);
	numData = sizeof(_midiCommand7);
	reg.QueryValue("MidiCommand7", &type, (BYTE*)&_midiCommand7, &numData);
	numData = sizeof(_midiFrom7);
	reg.QueryValue("MidiFrom7", &type, (BYTE*)&_midiFrom7, &numData);
	numData = sizeof(_midiTo7);
	reg.QueryValue("MidiTo7", &type, (BYTE*)&_midiTo7, &numData);

	numData = sizeof(_midiRecord8);
	reg.QueryValue("MidiRecord8", &type, (BYTE*)&_midiRecord8, &numData);
	numData = sizeof(_midiMessage8);
	reg.QueryValue("MidiMessage8", &type, (BYTE*)&_midiMessage8, &numData);
	numData = sizeof(_midiType8);
	reg.QueryValue("MidiType8", &type, (BYTE*)&_midiType8, &numData);
	numData = sizeof(_midiCommand8);
	reg.QueryValue("MidiCommand8", &type, (BYTE*)&_midiCommand8, &numData);
	numData = sizeof(_midiFrom8);
	reg.QueryValue("MidiFrom8", &type, (BYTE*)&_midiFrom8, &numData);
	numData = sizeof(_midiTo8);
	reg.QueryValue("MidiTo8", &type, (BYTE*)&_midiTo8, &numData);

	numData = sizeof(_midiRecord9);
	reg.QueryValue("MidiRecord9", &type, (BYTE*)&_midiRecord9, &numData);
	numData = sizeof(_midiMessage9);
	reg.QueryValue("MidiMessage9", &type, (BYTE*)&_midiMessage9, &numData);
	numData = sizeof(_midiType9);
	reg.QueryValue("MidiType9", &type, (BYTE*)&_midiType9, &numData);
	numData = sizeof(_midiCommand9);
	reg.QueryValue("MidiCommand9", &type, (BYTE*)&_midiCommand9, &numData);
	numData = sizeof(_midiFrom9);
	reg.QueryValue("MidiFrom9", &type, (BYTE*)&_midiFrom9, &numData);
	numData = sizeof(_midiTo9);
	reg.QueryValue("MidiTo9", &type, (BYTE*)&_midiTo9, &numData);

	numData = sizeof(_midiRecord10);
	reg.QueryValue("MidiRecord10", &type, (BYTE*)&_midiRecord10, &numData);
	numData = sizeof(_midiMessage10);
	reg.QueryValue("MidiMessage10", &type, (BYTE*)&_midiMessage10, &numData);
	numData = sizeof(_midiType10);
	reg.QueryValue("MidiType10", &type, (BYTE*)&_midiType10, &numData);
	numData = sizeof(_midiCommand10);
	reg.QueryValue("MidiCommand10", &type, (BYTE*)&_midiCommand10, &numData);
	numData = sizeof(_midiFrom10);
	reg.QueryValue("MidiFrom10", &type, (BYTE*)&_midiFrom10, &numData);
	numData = sizeof(_midiTo10);
	reg.QueryValue("MidiTo10", &type, (BYTE*)&_midiTo10, &numData);

	numData = sizeof(_midiRecord11);
	reg.QueryValue("MidiRecord11", &type, (BYTE*)&_midiRecord11, &numData);
	numData = sizeof(_midiMessage11);
	reg.QueryValue("MidiMessage11", &type, (BYTE*)&_midiMessage11, &numData);
	numData = sizeof(_midiType11);
	reg.QueryValue("MidiType11", &type, (BYTE*)&_midiType11, &numData);
	numData = sizeof(_midiCommand11);
	reg.QueryValue("MidiCommand11", &type, (BYTE*)&_midiCommand11, &numData);
	numData = sizeof(_midiFrom11);
	reg.QueryValue("MidiFrom11", &type, (BYTE*)&_midiFrom11, &numData);
	numData = sizeof(_midiTo11);
	reg.QueryValue("MidiTo11", &type, (BYTE*)&_midiTo11, &numData);

	numData = sizeof(_midiRecord12);
	reg.QueryValue("MidiRecord12", &type, (BYTE*)&_midiRecord12, &numData);
	numData = sizeof(_midiMessage12);
	reg.QueryValue("MidiMessage12", &type, (BYTE*)&_midiMessage12, &numData);
	numData = sizeof(_midiType12);
	reg.QueryValue("MidiType12", &type, (BYTE*)&_midiType12, &numData);
	numData = sizeof(_midiCommand12);
	reg.QueryValue("MidiCommand12", &type, (BYTE*)&_midiCommand12, &numData);
	numData = sizeof(_midiFrom12);
	reg.QueryValue("MidiFrom12", &type, (BYTE*)&_midiFrom12, &numData);
	numData = sizeof(_midiTo12);
	reg.QueryValue("MidiTo12", &type, (BYTE*)&_midiTo12, &numData);

	numData = sizeof(_midiRecord13);
	reg.QueryValue("MidiRecord13", &type, (BYTE*)&_midiRecord13, &numData);
	numData = sizeof(_midiMessage13);
	reg.QueryValue("MidiMessage13", &type, (BYTE*)&_midiMessage13, &numData);
	numData = sizeof(_midiType13);
	reg.QueryValue("MidiType13", &type, (BYTE*)&_midiType13, &numData);
	numData = sizeof(_midiCommand13);
	reg.QueryValue("MidiCommand13", &type, (BYTE*)&_midiCommand13, &numData);
	numData = sizeof(_midiFrom13);
	reg.QueryValue("MidiFrom13", &type, (BYTE*)&_midiFrom13, &numData);
	numData = sizeof(_midiTo13);
	reg.QueryValue("MidiTo13", &type, (BYTE*)&_midiTo13, &numData);

	numData = sizeof(_midiRecord14);
	reg.QueryValue("MidiRecord14", &type, (BYTE*)&_midiRecord14, &numData);
	numData = sizeof(_midiMessage14);
	reg.QueryValue("MidiMessage14", &type, (BYTE*)&_midiMessage14, &numData);
	numData = sizeof(_midiType14);
	reg.QueryValue("MidiType14", &type, (BYTE*)&_midiType14, &numData);
	numData = sizeof(_midiCommand14);
	reg.QueryValue("MidiCommand14", &type, (BYTE*)&_midiCommand14, &numData);
	numData = sizeof(_midiFrom14);
	reg.QueryValue("MidiFrom14", &type, (BYTE*)&_midiFrom14, &numData);
	numData = sizeof(_midiTo14);
	reg.QueryValue("MidiTo14", &type, (BYTE*)&_midiTo14, &numData);

	numData = sizeof(_midiRecord15);
	reg.QueryValue("MidiRecord15", &type, (BYTE*)&_midiRecord15, &numData);
	numData = sizeof(_midiMessage15);
	reg.QueryValue("MidiMessage15", &type, (BYTE*)&_midiMessage15, &numData);
	numData = sizeof(_midiType15);
	reg.QueryValue("MidiType15", &type, (BYTE*)&_midiType15, &numData);
	numData = sizeof(_midiCommand15);
	reg.QueryValue("MidiCommand15", &type, (BYTE*)&_midiCommand15, &numData);
	numData = sizeof(_midiFrom15);
	reg.QueryValue("MidiFrom15", &type, (BYTE*)&_midiFrom15, &numData);
	numData = sizeof(_midiTo15);
	reg.QueryValue("MidiTo15", &type, (BYTE*)&_midiTo15, &numData);

	numData = sizeof(defaultPatLines);
	reg.QueryValue("defaultPatLines", &type, (BYTE*)&defaultPatLines, &numData);

	for (int c=0; c<MAX_PATTERNS; c++)
	{
		// All pattern reset
		Global::_pSong->patternLines[c]=defaultPatLines;
	}

	numData = sizeof(bShowSongInfoOnLoad);
	reg.QueryValue("bShowSongInfoOnLoad", &type, (BYTE*)&bShowSongInfoOnLoad, &numData);
	numData = sizeof(bFileSaveReminders);
	reg.QueryValue("bFileSaveReminders", &type, (BYTE*)&bFileSaveReminders, &numData);
	

	numData = sizeof(autosaveSong);
	reg.QueryValue("autosaveSong", &type, (BYTE*)&autosaveSong, &numData);
	numData = sizeof(autosaveSongTime);
	reg.QueryValue("autosaveSongTime", &type, (BYTE*)&autosaveSongTime, &numData);
	
	
	numData = sizeof(mv_colour);
	reg.QueryValue("mv_colour", &type, (BYTE*)&mv_colour, &numData);
	numData = sizeof(mv_wirecolour);
	reg.QueryValue("mv_wirecolour", &type, (BYTE*)&mv_wirecolour, &numData);
	numData = sizeof(mv_polycolour);
	reg.QueryValue("mv_polycolour", &type, (BYTE*)&mv_polycolour, &numData);
	numData = sizeof(mv_wireaa);
	reg.QueryValue("mv_wireaa", &type, (BYTE*)&mv_wireaa, &numData);
	numData = sizeof(mv_wirewidth);
	reg.QueryValue("mv_wirewidth", &type, (BYTE*)&mv_wirewidth, &numData);
	numData = sizeof(mv_generator_fontcolour);
	reg.QueryValue("mv_generator_fontcolour", &type, (BYTE*)&mv_generator_fontcolour, &numData);
	numData = sizeof(mv_effect_fontcolour);
	reg.QueryValue("mv_effect_fontcolour", &type, (BYTE*)&mv_effect_fontcolour, &numData);
	numData = sizeof(mv_triangle_size);
	reg.QueryValue("mv_triangle_size", &type, (BYTE*)&mv_triangle_size, &numData);

	numData = sizeof(bBmpBkg);
	reg.QueryValue("bBmpBkg", &type, (BYTE*)&bBmpBkg, &numData);

	numData = sizeof(szBmpBkgFilename);
	reg.QueryValue("szBmpBkgFilename", &type, (BYTE*)&szBmpBkgFilename, &numData);

	numData = sizeof(pvc_background);
	reg.QueryValue("pvc_background", &type, (BYTE*)&pvc_background, &numData);
	numData = sizeof(pvc_background2);
	reg.QueryValue("pvc_background2", &type, (BYTE*)&pvc_background2, &numData);
	numData = sizeof(pvc_separator);
	reg.QueryValue("pvc_separator", &type, (BYTE*)&pvc_separator, &numData);
	numData = sizeof(pvc_separator2);
	reg.QueryValue("pvc_separator2", &type, (BYTE*)&pvc_separator2, &numData);
	numData = sizeof(pvc_row4beat);
	reg.QueryValue("pvc_row4beat", &type, (BYTE*)&pvc_row4beat, &numData);
	numData = sizeof(pvc_row4beat2);
	reg.QueryValue("pvc_row4beat2", &type, (BYTE*)&pvc_row4beat2, &numData);
	numData = sizeof(pvc_rowbeat);
	reg.QueryValue("pvc_rowbeat", &type, (BYTE*)&pvc_rowbeat, &numData);
	numData = sizeof(pvc_rowbeat2);
	reg.QueryValue("pvc_rowbeat2", &type, (BYTE*)&pvc_rowbeat2, &numData);
	numData = sizeof(pvc_row);
	reg.QueryValue("pvc_row", &type, (BYTE*)&pvc_row, &numData);
	numData = sizeof(pvc_row2);
	reg.QueryValue("pvc_row2", &type, (BYTE*)&pvc_row2, &numData);
	numData = sizeof(pvc_font);
	reg.QueryValue("pvc_font", &type, (BYTE*)&pvc_font, &numData);
	numData = sizeof(pvc_font2);
	reg.QueryValue("pvc_font2", &type, (BYTE*)&pvc_font2, &numData);
	numData = sizeof(pvc_fontPlay);
	reg.QueryValue("pvc_fontPlay", &type, (BYTE*)&pvc_fontPlay, &numData);
	numData = sizeof(pvc_fontPlay2);
	reg.QueryValue("pvc_fontPlay2", &type, (BYTE*)&pvc_fontPlay2, &numData);
	numData = sizeof(pvc_fontCur);
	reg.QueryValue("pvc_fontCur", &type, (BYTE*)&pvc_fontCur, &numData);
	numData = sizeof(pvc_fontCur2);
	reg.QueryValue("pvc_fontCur2", &type, (BYTE*)&pvc_fontCur2, &numData);
	numData = sizeof(pvc_fontSel);
	reg.QueryValue("pvc_fontSel", &type, (BYTE*)&pvc_fontSel, &numData);
	numData = sizeof(pvc_fontSel2);
	reg.QueryValue("pvc_fontSel2", &type, (BYTE*)&pvc_fontSel2, &numData);
	numData = sizeof(pvc_selection);
	reg.QueryValue("pvc_selection", &type, (BYTE*)&pvc_selection, &numData);
	numData = sizeof(pvc_selection2);
	reg.QueryValue("pvc_selection2", &type, (BYTE*)&pvc_selection2, &numData);
	numData = sizeof(pvc_playbar);
	reg.QueryValue("pvc_playbar", &type, (BYTE*)&pvc_playbar, &numData);
	numData = sizeof(pvc_playbar2);
	reg.QueryValue("pvc_playbar2", &type, (BYTE*)&pvc_playbar2, &numData);
	numData = sizeof(pvc_cursor);
	reg.QueryValue("pvc_cursor", &type, (BYTE*)&pvc_cursor, &numData);
	numData = sizeof(pvc_cursor2);
	reg.QueryValue("pvc_cursor2", &type, (BYTE*)&pvc_cursor2, &numData);

	numData = sizeof(vu1);
	reg.QueryValue("vu1", &type, (BYTE*)&vu1, &numData);
	numData = sizeof(vu2);
	reg.QueryValue("vu2", &type, (BYTE*)&vu2, &numData);
	numData = sizeof(vu3);
	reg.QueryValue("vu3", &type, (BYTE*)&vu3, &numData);

	mv_wireaacolour = ((((mv_wirecolour&0x00ff0000) + ((mv_colour&0x00ff0000)*4))/5)&0x00ff0000) +
					  ((((mv_wirecolour&0x00ff00) + ((mv_colour&0x00ff00)*4))/5)&0x00ff00) +
					  ((((mv_wirecolour&0x00ff) + ((mv_colour&0x00ff)*4))/5)&0x00ff);

	mv_wireaacolour2 = (((((mv_wirecolour&0x00ff0000)) + ((mv_colour&0x00ff0000)))/2)&0x00ff0000) +
			(((((mv_wirecolour&0x00ff00)) + ((mv_colour&0x00ff00)))/2)&0x00ff00) +
			(((((mv_wirecolour&0x00ff)) + ((mv_colour&0x00ff)))/2)&0x00ff);

	numData = sizeof(pattern_fontface);
	reg.QueryValue("pattern_fontface", &type, (BYTE*)&pattern_fontface, &numData);
	numData = sizeof(pattern_header_skin);
	reg.QueryValue("pattern_header_skin", &type, (BYTE*)&pattern_header_skin, &numData);
	numData = sizeof(pattern_font_point);
	reg.QueryValue("pattern_font_point", &type, (BYTE*)&pattern_font_point, &numData);
	numData = sizeof(pattern_font_x);
	reg.QueryValue("pattern_font_x", &type, (BYTE*)&pattern_font_x, &numData);
	numData = sizeof(pattern_font_y);
	reg.QueryValue("pattern_font_y", &type, (BYTE*)&pattern_font_y, &numData);
	numData = sizeof(pattern_font_flags);
	reg.QueryValue("pattern_font_flags", &type, (BYTE*)&pattern_font_flags, &numData);

	numData = sizeof(pattern_draw_empty_data);
	reg.QueryValue("pattern_draw_empty_data", &type, (BYTE*)&pattern_draw_empty_data, &numData);
	numData = sizeof(draw_mac_index);
	reg.QueryValue("draw_mac_index", &type, (BYTE*)&draw_mac_index, &numData);

	numData = sizeof(generator_fontface);
	reg.QueryValue("generator_fontface", &type, (BYTE*)&generator_fontface, &numData);
	numData = sizeof(generator_font_point);
	reg.QueryValue("generator_font_point", &type, (BYTE*)&generator_font_point, &numData);
	numData = sizeof(generator_font_flags);
	reg.QueryValue("generator_font_flags", &type, (BYTE*)&generator_font_flags, &numData);

	numData = sizeof(effect_fontface);
	reg.QueryValue("effect_fontface", &type, (BYTE*)&effect_fontface, &numData);
	numData = sizeof(effect_font_point);
	reg.QueryValue("effect_font_point", &type, (BYTE*)&effect_font_point, &numData);
	numData = sizeof(effect_font_flags);
	reg.QueryValue("effect_font_flags", &type, (BYTE*)&effect_font_flags, &numData);

	numData = sizeof(machine_skin);
	reg.QueryValue("machine_skin", &type, (BYTE*)&machine_skin, &numData);

	CreateFonts();	
	
	numData = sizeof(output);
	if (reg.QueryValue("OutputDriver", &type, (BYTE*)&output, &numData) == ERROR_SUCCESS)
	{
		if ((output >= 0) && (output < _numOutputDrivers))
		{
			_outputDriverIndex = output;
		}
		_pOutputDriver = _ppOutputDrivers[_outputDriverIndex];
	}
	// MIDI IMPLEMENTATION
	numData = sizeof(midiInput);
	if (reg.QueryValue("MidiInputDriver", &type, (BYTE*)&midiInput, &numData) == ERROR_SUCCESS)
	{
		int _numMidiDrivers = CMidiInput::Instance()->GetNumDevices();
		if( midiInput >= 0 && (midiInput <= _numMidiDrivers ))
		{
			_midiDriverIndex = midiInput;
			CMidiInput::Instance()->SetDeviceId( DRIVER_MIDI, _midiDriverIndex-1 );
		}
	}
	numData = sizeof(syncDriver);
	if (reg.QueryValue("MidiSyncDriver", &type, (BYTE*)&syncDriver, &numData) == ERROR_SUCCESS)
	{
		int _numMidiDrivers = CMidiInput::Instance()->GetNumDevices();
		if( syncDriver >= 0 && (syncDriver <= _numMidiDrivers ))
		{
			_syncDriverIndex = syncDriver;
			CMidiInput::Instance()->SetDeviceId( DRIVER_SYNC, _syncDriverIndex-1 );
		}
	}
	numData = sizeof(midiHeadroom);
	if (reg.QueryValue("MidiInputHeadroom", &type, (BYTE*)&midiHeadroom, &numData) == ERROR_SUCCESS)
	{
		_midiHeadroom = midiHeadroom;
		CMidiInput::Instance()->GetConfigPtr()->midiHeadroom = midiHeadroom;
	}

	if (_psInitialInstrumentDir != NULL)
	{
		delete _psInitialInstrumentDir;
		_psInitialInstrumentDir = NULL;
	}
	numData = sizeof(string);
	if (reg.QueryValue("InstrumentDir", &type, (BYTE*)&string, &numData) == ERROR_SUCCESS)
	{
		_psInitialInstrumentDir = new char[numData+1];
		strcpy(_psInitialInstrumentDir, &string[0]);
	}
	else
	{
		_psInitialInstrumentDir = new char[strlen(appPath)+sizeof(DEFAULT_INSTRUMENT_DIR)];
		strcpy(_psInitialInstrumentDir, &appPath[0]);
		strcat(_psInitialInstrumentDir, DEFAULT_INSTRUMENT_DIR);
	}
	if (_psInstrumentDir == NULL)
	{
		SetInstrumentDir(_psInitialInstrumentDir);
	}

	if (_psInitialSongDir != NULL)
	{
		delete _psInitialSongDir;
		_psInitialSongDir = NULL;
	}
	numData = sizeof(string);
	if (reg.QueryValue("SongDir", &type, (BYTE*)&string, &numData) == ERROR_SUCCESS)
	{
		_psInitialSongDir = new char[numData+1];
		strcpy(_psInitialSongDir, &string[0]);
	}
	else
	{
		_psInitialSongDir = new char[strlen(appPath)+sizeof(DEFAULT_SONG_DIR)];
		strcpy(_psInitialSongDir, &appPath[0]);
		strcat(_psInitialSongDir, DEFAULT_SONG_DIR);
	}
	if (_psSongDir == NULL)
	{
		SetSongDir(_psInitialSongDir);
	}

	if (_psInitialSkinDir != NULL)
	{
		delete _psInitialSkinDir;
		_psInitialSkinDir = NULL;
	}
	numData = sizeof(string);
	if (reg.QueryValue("SkinDir", &type, (BYTE*)&string, &numData) == ERROR_SUCCESS)
	{
		_psInitialSkinDir = new char[numData+1];
		strcpy(_psInitialSkinDir, &string[0]);
	}
	else
	{
		_psInitialSkinDir = new char[strlen(appPath)+sizeof(DEFAULT_SKIN_DIR)];
		strcpy(_psInitialSkinDir, &appPath[0]);
		strcat(_psInitialSkinDir, DEFAULT_SKIN_DIR);
	}
	if (_psSkinDir == NULL)
	{
		SetSkinDir(_psInitialSkinDir);
	}

#endif // ndef _WINAMP_PLUGIN_

	if (_psInitialPluginDir != NULL)
	{
		delete _psInitialPluginDir;
		_psInitialPluginDir = NULL;
	}
	numData = sizeof(string);
	if (reg.QueryValue("PluginDir", &type, (BYTE*)&string, &numData) == ERROR_SUCCESS)
	{
		_psInitialPluginDir = new char[numData+1];
		strcpy(_psInitialPluginDir, &string[0]);
	}
	else
	{
		_psInitialPluginDir = new char[strlen(appPath)+sizeof(DEFAULT_PLUGIN_DIR)];
		strcpy(_psInitialPluginDir, &appPath[0]);
		strcat(_psInitialPluginDir, DEFAULT_PLUGIN_DIR);
	}
	if (_psPluginDir == NULL)
	{
		SetPluginDir(_psInitialPluginDir);
	}

	if (_psInitialVstDir != NULL)
	{
		delete _psInitialVstDir;
		_psInitialVstDir = NULL;
	}
	numData = sizeof(string);
	if (reg.QueryValue("VstDir", &type, (BYTE*)&string, &numData) == ERROR_SUCCESS)
	{
		_psInitialVstDir = new char[numData+1];
		strcpy(_psInitialVstDir, &string[0]);
	}
	else
	{
		_psInitialVstDir = new char[strlen(appPath)+sizeof(DEFAULT_VST_DIR)];
		strcpy(_psInitialVstDir, &appPath[0]);
		strcat(_psInitialVstDir, DEFAULT_VST_DIR);
	}
	if (_psVstDir == NULL)
	{
		SetVstDir(_psInitialVstDir);
	}


	reg.CloseKey();
	reg.CloseRootKey();
	return existed;
}

#if !defined(_WINAMP_PLUGIN_)

void
Configuration::Write()
{
	Registry reg;
	if (reg.OpenRootKey(HKEY_CURRENT_USER, SOFTWARE_ROOT_KEY) != ERROR_SUCCESS)
	{
		Error("Unable to write configuration to the registry");
		return;
	}
	if (reg.OpenKey(CONFIG_KEY) != ERROR_SUCCESS)
	{
		if (reg.CreateKey(CONFIG_KEY) != ERROR_SUCCESS)
		{
			Error("Unable to write configuration to the registry");
			return;
		}
	}

	reg.SetValue("NewMacDlgpluginOrder", REG_BINARY, (BYTE*)&CNewMachine::pluginOrder, sizeof(CNewMachine::pluginOrder));
	reg.SetValue("NewMacDlgpluginName", REG_BINARY, (BYTE*)&CNewMachine::pluginName, sizeof(CNewMachine::pluginName));
	reg.SetValue("WrapAround", REG_BINARY, (BYTE*)&_wrapAround, sizeof(_wrapAround));
	reg.SetValue("CenterCursor", REG_BINARY, (BYTE*)&_centerCursor, sizeof(_centerCursor));
	reg.SetValue("FollowSong", REG_BINARY, (BYTE*)&_followSong, sizeof(_followSong));
	reg.SetValue("CursorAlwaysDown", REG_BINARY, (BYTE*)&_cursorAlwaysDown, sizeof(_cursorAlwaysDown));
	reg.SetValue("useDoubleBuffer", REG_BINARY, (BYTE*)&useDoubleBuffer, sizeof(useDoubleBuffer));
	reg.SetValue("DisplayLineNumbers", REG_BINARY, (BYTE*)&_linenumbers, sizeof(_linenumbers));
	reg.SetValue("DisplayLineNumbersHex", REG_BINARY, (BYTE*)&_linenumbersHex, sizeof(_linenumbersHex));
	reg.SetValue("DisplayLineNumbersCursor", REG_BINARY, (BYTE*)&_linenumbersCursor, sizeof(_linenumbersCursor));
	reg.SetValue("showAboutAtStart", REG_BINARY, (BYTE*)&_showAboutAtStart, sizeof(_showAboutAtStart));
	reg.SetValue("RecordNoteoff", REG_BINARY, (BYTE*)&_RecordNoteoff, sizeof(_RecordNoteoff));
	reg.SetValue("RecordTweaks", REG_BINARY, (BYTE*)&_RecordTweaks, sizeof(_RecordTweaks));
	reg.SetValue("RecordMouseTweaksSmooth", REG_BINARY, (BYTE*)&_RecordMouseTweaksSmooth, sizeof(_RecordMouseTweaksSmooth));
	reg.SetValue("notesToEffects", REG_BINARY, (BYTE*)&_notesToEffects, sizeof(_notesToEffects));
	reg.SetValue("RecordUnarmed", REG_BINARY, (BYTE*)&_RecordUnarmed, sizeof(_RecordUnarmed));
	reg.SetValue("MidiMachineViewSeqMode", REG_BINARY, (BYTE*)&_midiMachineViewSeqMode, sizeof(_midiMachineViewSeqMode));
	reg.SetValue("OutputDriver", REG_DWORD, (BYTE*)&_outputDriverIndex, sizeof(_outputDriverIndex));
	reg.SetValue("MidiInputDriver", REG_DWORD, (BYTE*)&_midiDriverIndex, sizeof(_midiDriverIndex));	// MIDI IMPLEMENTATION
	reg.SetValue("MidiSyncDriver", REG_DWORD, (BYTE*)&_syncDriverIndex, sizeof(_syncDriverIndex));	// MIDI IMPLEMENTATION
	reg.SetValue("MidiInputHeadroom", REG_DWORD, (BYTE*)&_midiHeadroom, sizeof(_midiHeadroom));	

	reg.SetValue("MidiRecordVel", REG_BINARY, (BYTE*)&_midiRecordVel, sizeof(_midiRecordVel));
	reg.SetValue("MidiTypeVel", REG_BINARY, (BYTE*)&_midiTypeVel, sizeof(_midiTypeVel));
	reg.SetValue("MidiCommandVel", REG_DWORD, (BYTE*)&_midiCommandVel, sizeof(_midiCommandVel));	
	reg.SetValue("MidiFromVel", REG_DWORD, (BYTE*)&_midiFromVel, sizeof(_midiFromVel));	
	reg.SetValue("MidiToVel", REG_DWORD, (BYTE*)&_midiToVel, sizeof(_midiToVel));	

	reg.SetValue("MidiRecordPit", REG_BINARY, (BYTE*)&_midiRecordPit, sizeof(_midiRecordPit));
	reg.SetValue("MidiTypePit", REG_BINARY, (BYTE*)&_midiTypePit, sizeof(_midiTypePit));
	reg.SetValue("MidiCommandPit", REG_DWORD, (BYTE*)&_midiCommandPit, sizeof(_midiCommandPit));	
	reg.SetValue("MidiFromPit", REG_DWORD, (BYTE*)&_midiFromPit, sizeof(_midiFromPit));	
	reg.SetValue("MidiToPit", REG_DWORD, (BYTE*)&_midiToPit, sizeof(_midiToPit));	
	
	reg.SetValue("MidiRecord0", REG_BINARY, (BYTE*)&_midiRecord0, sizeof(_midiRecord0));
	reg.SetValue("MidiMessage0", REG_BINARY, (BYTE*)&_midiMessage0, sizeof(_midiMessage0));
	reg.SetValue("MidiType0", REG_BINARY, (BYTE*)&_midiType0, sizeof(_midiType0));
	reg.SetValue("MidiCommand0", REG_DWORD, (BYTE*)&_midiCommand0, sizeof(_midiCommand0));	
	reg.SetValue("MidiFrom0", REG_DWORD, (BYTE*)&_midiFrom0, sizeof(_midiFrom0));	
	reg.SetValue("MidiTo0", REG_DWORD, (BYTE*)&_midiTo0, sizeof(_midiTo0));	

	reg.SetValue("MidiRecord1", REG_BINARY, (BYTE*)&_midiRecord1, sizeof(_midiRecord1));
	reg.SetValue("MidiMessage1", REG_BINARY, (BYTE*)&_midiMessage1, sizeof(_midiMessage1));
	reg.SetValue("MidiType1", REG_BINARY, (BYTE*)&_midiType1, sizeof(_midiType1));
	reg.SetValue("MidiCommand1", REG_DWORD, (BYTE*)&_midiCommand1, sizeof(_midiCommand1));	
	reg.SetValue("MidiFrom1", REG_DWORD, (BYTE*)&_midiFrom1, sizeof(_midiFrom1));	
	reg.SetValue("MidiTo1", REG_DWORD, (BYTE*)&_midiTo1, sizeof(_midiTo1));	

	reg.SetValue("MidiRecord2", REG_BINARY, (BYTE*)&_midiRecord2, sizeof(_midiRecord2));
	reg.SetValue("MidiMessage2", REG_BINARY, (BYTE*)&_midiMessage2, sizeof(_midiMessage2));
	reg.SetValue("MidiType2", REG_BINARY, (BYTE*)&_midiType2, sizeof(_midiType2));
	reg.SetValue("MidiCommand2", REG_DWORD, (BYTE*)&_midiCommand2, sizeof(_midiCommand2));	
	reg.SetValue("MidiFrom2", REG_DWORD, (BYTE*)&_midiFrom2, sizeof(_midiFrom2));	
	reg.SetValue("MidiTo2", REG_DWORD, (BYTE*)&_midiTo2, sizeof(_midiTo2));	

	reg.SetValue("MidiRecord3", REG_BINARY, (BYTE*)&_midiRecord3, sizeof(_midiRecord3));
	reg.SetValue("MidiMessage3", REG_BINARY, (BYTE*)&_midiMessage3, sizeof(_midiMessage3));
	reg.SetValue("MidiType3", REG_BINARY, (BYTE*)&_midiType3, sizeof(_midiType3));
	reg.SetValue("MidiCommand3", REG_DWORD, (BYTE*)&_midiCommand3, sizeof(_midiCommand3));	
	reg.SetValue("MidiFrom3", REG_DWORD, (BYTE*)&_midiFrom3, sizeof(_midiFrom3));	
	reg.SetValue("MidiTo3", REG_DWORD, (BYTE*)&_midiTo3, sizeof(_midiTo3));	

	reg.SetValue("MidiRecord4", REG_BINARY, (BYTE*)&_midiRecord4, sizeof(_midiRecord4));
	reg.SetValue("MidiMessage4", REG_BINARY, (BYTE*)&_midiMessage4, sizeof(_midiMessage4));
	reg.SetValue("MidiType4", REG_BINARY, (BYTE*)&_midiType4, sizeof(_midiType4));
	reg.SetValue("MidiCommand4", REG_DWORD, (BYTE*)&_midiCommand4, sizeof(_midiCommand4));	
	reg.SetValue("MidiFrom4", REG_DWORD, (BYTE*)&_midiFrom4, sizeof(_midiFrom4));	
	reg.SetValue("MidiTo4", REG_DWORD, (BYTE*)&_midiTo4, sizeof(_midiTo4));	

	reg.SetValue("MidiRecord5", REG_BINARY, (BYTE*)&_midiRecord5, sizeof(_midiRecord5));
	reg.SetValue("MidiMessage5", REG_BINARY, (BYTE*)&_midiMessage5, sizeof(_midiMessage5));
	reg.SetValue("MidiType5", REG_BINARY, (BYTE*)&_midiType5, sizeof(_midiType5));
	reg.SetValue("MidiCommand5", REG_DWORD, (BYTE*)&_midiCommand5, sizeof(_midiCommand5));	
	reg.SetValue("MidiFrom5", REG_DWORD, (BYTE*)&_midiFrom5, sizeof(_midiFrom5));	
	reg.SetValue("MidiTo5", REG_DWORD, (BYTE*)&_midiTo5, sizeof(_midiTo5));	

	reg.SetValue("MidiRecord6", REG_BINARY, (BYTE*)&_midiRecord6, sizeof(_midiRecord6));
	reg.SetValue("MidiMessage6", REG_BINARY, (BYTE*)&_midiMessage6, sizeof(_midiMessage6));
	reg.SetValue("MidiType6", REG_BINARY, (BYTE*)&_midiType6, sizeof(_midiType6));
	reg.SetValue("MidiCommand6", REG_DWORD, (BYTE*)&_midiCommand6, sizeof(_midiCommand6));	
	reg.SetValue("MidiFrom6", REG_DWORD, (BYTE*)&_midiFrom6, sizeof(_midiFrom6));	
	reg.SetValue("MidiTo6", REG_DWORD, (BYTE*)&_midiTo6, sizeof(_midiTo6));	

	reg.SetValue("MidiRecord7", REG_BINARY, (BYTE*)&_midiRecord7, sizeof(_midiRecord7));
	reg.SetValue("MidiMessage7", REG_BINARY, (BYTE*)&_midiMessage7, sizeof(_midiMessage7));
	reg.SetValue("MidiType7", REG_BINARY, (BYTE*)&_midiType7, sizeof(_midiType7));
	reg.SetValue("MidiCommand7", REG_DWORD, (BYTE*)&_midiCommand7, sizeof(_midiCommand7));	
	reg.SetValue("MidiFrom7", REG_DWORD, (BYTE*)&_midiFrom7, sizeof(_midiFrom7));	
	reg.SetValue("MidiTo7", REG_DWORD, (BYTE*)&_midiTo7, sizeof(_midiTo7));	

	reg.SetValue("MidiRecord8", REG_BINARY, (BYTE*)&_midiRecord8, sizeof(_midiRecord8));
	reg.SetValue("MidiMessage8", REG_BINARY, (BYTE*)&_midiMessage8, sizeof(_midiMessage8));
	reg.SetValue("MidiType8", REG_BINARY, (BYTE*)&_midiType8, sizeof(_midiType8));
	reg.SetValue("MidiCommand8", REG_DWORD, (BYTE*)&_midiCommand8, sizeof(_midiCommand8));	
	reg.SetValue("MidiFrom8", REG_DWORD, (BYTE*)&_midiFrom8, sizeof(_midiFrom8));	
	reg.SetValue("MidiTo8", REG_DWORD, (BYTE*)&_midiTo8, sizeof(_midiTo8));	

	reg.SetValue("MidiRecord9", REG_BINARY, (BYTE*)&_midiRecord9, sizeof(_midiRecord9));
	reg.SetValue("MidiMessage9", REG_BINARY, (BYTE*)&_midiMessage9, sizeof(_midiMessage9));
	reg.SetValue("MidiType9", REG_BINARY, (BYTE*)&_midiType9, sizeof(_midiType9));
	reg.SetValue("MidiCommand9", REG_DWORD, (BYTE*)&_midiCommand9, sizeof(_midiCommand9));	
	reg.SetValue("MidiFrom9", REG_DWORD, (BYTE*)&_midiFrom9, sizeof(_midiFrom9));	
	reg.SetValue("MidiTo9", REG_DWORD, (BYTE*)&_midiTo9, sizeof(_midiTo9));	

	reg.SetValue("MidiRecord10", REG_BINARY, (BYTE*)&_midiRecord10, sizeof(_midiRecord10));
	reg.SetValue("MidiMessage10", REG_BINARY, (BYTE*)&_midiMessage10, sizeof(_midiMessage10));
	reg.SetValue("MidiType10", REG_BINARY, (BYTE*)&_midiType10, sizeof(_midiType10));
	reg.SetValue("MidiCommand10", REG_DWORD, (BYTE*)&_midiCommand10, sizeof(_midiCommand10));	
	reg.SetValue("MidiFrom10", REG_DWORD, (BYTE*)&_midiFrom10, sizeof(_midiFrom10));	
	reg.SetValue("MidiTo10", REG_DWORD, (BYTE*)&_midiTo10, sizeof(_midiTo10));	

	reg.SetValue("MidiRecord11", REG_BINARY, (BYTE*)&_midiRecord11, sizeof(_midiRecord11));
	reg.SetValue("MidiMessage11", REG_BINARY, (BYTE*)&_midiMessage11, sizeof(_midiMessage11));
	reg.SetValue("MidiType11", REG_BINARY, (BYTE*)&_midiType11, sizeof(_midiType11));
	reg.SetValue("MidiCommand11", REG_DWORD, (BYTE*)&_midiCommand11, sizeof(_midiCommand11));	
	reg.SetValue("MidiFrom11", REG_DWORD, (BYTE*)&_midiFrom11, sizeof(_midiFrom11));	
	reg.SetValue("MidiTo11", REG_DWORD, (BYTE*)&_midiTo11, sizeof(_midiTo11));	

	reg.SetValue("MidiRecord12", REG_BINARY, (BYTE*)&_midiRecord12, sizeof(_midiRecord12));
	reg.SetValue("MidiMessage12", REG_BINARY, (BYTE*)&_midiMessage12, sizeof(_midiMessage12));
	reg.SetValue("MidiType12", REG_BINARY, (BYTE*)&_midiType12, sizeof(_midiType12));
	reg.SetValue("MidiCommand12", REG_DWORD, (BYTE*)&_midiCommand12, sizeof(_midiCommand12));	
	reg.SetValue("MidiFrom12", REG_DWORD, (BYTE*)&_midiFrom12, sizeof(_midiFrom12));	
	reg.SetValue("MidiTo12", REG_DWORD, (BYTE*)&_midiTo12, sizeof(_midiTo12));	

	reg.SetValue("MidiRecord13", REG_BINARY, (BYTE*)&_midiRecord13, sizeof(_midiRecord13));
	reg.SetValue("MidiMessage13", REG_BINARY, (BYTE*)&_midiMessage13, sizeof(_midiMessage13));
	reg.SetValue("MidiType13", REG_BINARY, (BYTE*)&_midiType13, sizeof(_midiType13));
	reg.SetValue("MidiCommand13", REG_DWORD, (BYTE*)&_midiCommand13, sizeof(_midiCommand13));	
	reg.SetValue("MidiFrom13", REG_DWORD, (BYTE*)&_midiFrom13, sizeof(_midiFrom13));	
	reg.SetValue("MidiTo13", REG_DWORD, (BYTE*)&_midiTo13, sizeof(_midiTo13));	

	reg.SetValue("MidiRecord14", REG_BINARY, (BYTE*)&_midiRecord14, sizeof(_midiRecord14));
	reg.SetValue("MidiMessage14", REG_BINARY, (BYTE*)&_midiMessage14, sizeof(_midiMessage14));
	reg.SetValue("MidiType14", REG_BINARY, (BYTE*)&_midiType14, sizeof(_midiType14));
	reg.SetValue("MidiCommand14", REG_DWORD, (BYTE*)&_midiCommand14, sizeof(_midiCommand14));	
	reg.SetValue("MidiFrom14", REG_DWORD, (BYTE*)&_midiFrom14, sizeof(_midiFrom14));	
	reg.SetValue("MidiTo14", REG_DWORD, (BYTE*)&_midiTo14, sizeof(_midiTo14));	

	reg.SetValue("MidiRecord15", REG_BINARY, (BYTE*)&_midiRecord15, sizeof(_midiRecord15));
	reg.SetValue("MidiMessage15", REG_BINARY, (BYTE*)&_midiMessage15, sizeof(_midiMessage15));
	reg.SetValue("MidiType15", REG_BINARY, (BYTE*)&_midiType15, sizeof(_midiType15));
	reg.SetValue("MidiCommand15", REG_DWORD, (BYTE*)&_midiCommand15, sizeof(_midiCommand15));	
	reg.SetValue("MidiFrom15", REG_DWORD, (BYTE*)&_midiFrom15, sizeof(_midiFrom15));	
	reg.SetValue("MidiTo15", REG_DWORD, (BYTE*)&_midiTo15, sizeof(_midiTo15));	

	reg.SetValue("defaultPatLines", REG_DWORD, (BYTE*)&defaultPatLines, sizeof(defaultPatLines));	

	reg.SetValue("bShowSongInfoOnLoad", REG_DWORD, (BYTE*)&bShowSongInfoOnLoad, sizeof(bShowSongInfoOnLoad));	
	reg.SetValue("bFileSaveReminders", REG_DWORD, (BYTE*)&bFileSaveReminders, sizeof(bFileSaveReminders));	

	reg.SetValue("autosaveSong", REG_DWORD, (BYTE*)&autosaveSong, sizeof(autosaveSong));
	reg.SetValue("autosaveSongTime", REG_DWORD, (BYTE*)&autosaveSongTime, sizeof(autosaveSong));

	reg.SetValue("mv_colour", REG_DWORD, (BYTE*)&mv_colour, sizeof(mv_colour));	
	reg.SetValue("mv_wirecolour", REG_DWORD, (BYTE*)&mv_wirecolour, sizeof(mv_wirecolour));	
	reg.SetValue("mv_polycolour", REG_DWORD, (BYTE*)&mv_polycolour, sizeof(mv_polycolour));	
	reg.SetValue("mv_wireaa", REG_BINARY, (BYTE*)&mv_wireaa, sizeof(mv_wireaa));	

	reg.SetValue("mv_wirewidth", REG_DWORD, (BYTE*)&mv_wirewidth, sizeof(mv_wirewidth));	
	reg.SetValue("mv_generator_fontcolour", REG_DWORD, (BYTE*)&mv_generator_fontcolour, sizeof(mv_generator_fontcolour));	
	reg.SetValue("mv_effect_fontcolour", REG_DWORD, (BYTE*)&mv_effect_fontcolour, sizeof(mv_effect_fontcolour));	

	reg.SetValue("mv_triangle_size", REG_BINARY, (BYTE*)&mv_triangle_size, sizeof(mv_triangle_size));	

	reg.SetValue("bBmpBkg", REG_DWORD, (BYTE*)&bBmpBkg, sizeof(bBmpBkg));	
	reg.SetValue("szBmpBkgFilename", REG_DWORD, (BYTE*)&szBmpBkgFilename, sizeof(szBmpBkgFilename));

	reg.SetValue("pvc_separator", REG_DWORD, (BYTE*)&pvc_separator, sizeof(pvc_separator));	
	reg.SetValue("pvc_separator2", REG_DWORD, (BYTE*)&pvc_separator2, sizeof(pvc_separator2));	
	reg.SetValue("pvc_background", REG_DWORD, (BYTE*)&pvc_background, sizeof(pvc_background));	
	reg.SetValue("pvc_background2", REG_DWORD, (BYTE*)&pvc_background2, sizeof(pvc_background2));	
	reg.SetValue("pvc_row4beat", REG_DWORD, (BYTE*)&pvc_row4beat, sizeof(pvc_row4beat));	
	reg.SetValue("pvc_row4beat2", REG_DWORD, (BYTE*)&pvc_row4beat2, sizeof(pvc_row4beat2));	
	reg.SetValue("pvc_rowbeat", REG_DWORD, (BYTE*)&pvc_rowbeat, sizeof(pvc_rowbeat));	
	reg.SetValue("pvc_rowbeat2", REG_DWORD, (BYTE*)&pvc_rowbeat2, sizeof(pvc_rowbeat2));	
	reg.SetValue("pvc_row", REG_DWORD, (BYTE*)&pvc_row, sizeof(pvc_row));	
	reg.SetValue("pvc_row2", REG_DWORD, (BYTE*)&pvc_row2, sizeof(pvc_row2));	
	reg.SetValue("pvc_font", REG_DWORD, (BYTE*)&pvc_font, sizeof(pvc_font));	
	reg.SetValue("pvc_font2", REG_DWORD, (BYTE*)&pvc_font2, sizeof(pvc_font2));	
	reg.SetValue("pvc_fontPlay", REG_DWORD, (BYTE*)&pvc_fontPlay, sizeof(pvc_fontPlay));	
	reg.SetValue("pvc_fontPlay2", REG_DWORD, (BYTE*)&pvc_fontPlay2, sizeof(pvc_fontPlay2));	
	reg.SetValue("pvc_fontCur", REG_DWORD, (BYTE*)&pvc_fontCur, sizeof(pvc_fontCur));	
	reg.SetValue("pvc_fontCur2", REG_DWORD, (BYTE*)&pvc_fontCur2, sizeof(pvc_fontCur2));	
	reg.SetValue("pvc_fontSel", REG_DWORD, (BYTE*)&pvc_fontSel, sizeof(pvc_fontSel));	
	reg.SetValue("pvc_fontSel2", REG_DWORD, (BYTE*)&pvc_fontSel2, sizeof(pvc_fontSel2));	
	reg.SetValue("pvc_selection", REG_DWORD, (BYTE*)&pvc_selection, sizeof(pvc_selection));	
	reg.SetValue("pvc_selection2", REG_DWORD, (BYTE*)&pvc_selection2, sizeof(pvc_selection2));	
	reg.SetValue("pvc_playbar", REG_DWORD, (BYTE*)&pvc_playbar, sizeof(pvc_playbar));	
	reg.SetValue("pvc_playbar2", REG_DWORD, (BYTE*)&pvc_playbar2, sizeof(pvc_playbar2));	
	reg.SetValue("pvc_cursor", REG_DWORD, (BYTE*)&pvc_cursor, sizeof(pvc_cursor));	
	reg.SetValue("pvc_cursor2", REG_DWORD, (BYTE*)&pvc_cursor2, sizeof(pvc_cursor2));	

	reg.SetValue("vu1", REG_DWORD, (BYTE*)&vu1, sizeof(vu1));	
	reg.SetValue("vu2", REG_DWORD, (BYTE*)&vu2, sizeof(vu2));	
	reg.SetValue("vu3", REG_DWORD, (BYTE*)&vu3, sizeof(vu3));	

	reg.SetValue("pattern_fontface", REG_SZ, (BYTE*)pattern_fontface, strlen(pattern_fontface));
	reg.SetValue("pattern_font_point", REG_DWORD, (BYTE*)&pattern_font_point, sizeof(pattern_font_point));	
	reg.SetValue("pattern_font_flags", REG_DWORD, (BYTE*)&pattern_font_flags, sizeof(pattern_font_flags));	
	reg.SetValue("pattern_font_x", REG_DWORD, (BYTE*)&pattern_font_x, sizeof(pattern_font_x));	
	reg.SetValue("pattern_font_y", REG_DWORD, (BYTE*)&pattern_font_y, sizeof(pattern_font_y));	
	reg.SetValue("pattern_draw_empty_data", REG_DWORD, (BYTE*)&pattern_draw_empty_data, sizeof(pattern_draw_empty_data));	
	reg.SetValue("draw_mac_index", REG_DWORD, (BYTE*)&draw_mac_index, sizeof(draw_mac_index));	

	reg.SetValue("pattern_header_skin", REG_SZ, (BYTE*)pattern_header_skin, strlen(pattern_header_skin));

	reg.SetValue("generator_fontface", REG_SZ, (BYTE*)generator_fontface, strlen(generator_fontface));
	reg.SetValue("generator_font_point", REG_DWORD, (BYTE*)&generator_font_point, sizeof(generator_font_point));	
	reg.SetValue("generator_font_flags", REG_DWORD, (BYTE*)&generator_font_flags, sizeof(generator_font_flags));	

	reg.SetValue("effect_fontface", REG_SZ, (BYTE*)effect_fontface, strlen(effect_fontface));
	reg.SetValue("effect_font_point", REG_DWORD, (BYTE*)&effect_font_point, sizeof(effect_font_point));	
	reg.SetValue("effect_font_flags", REG_DWORD, (BYTE*)&effect_font_flags, sizeof(effect_font_flags));	

	reg.SetValue("machine_skin", REG_SZ, (BYTE*)machine_skin, strlen(machine_skin));
	
	if (_psInitialInstrumentDir != NULL)
	{
		reg.SetValue("InstrumentDir", REG_SZ, (BYTE*)_psInitialInstrumentDir, strlen(_psInitialInstrumentDir));
	}
	if (_psInitialSongDir != NULL)
	{
		reg.SetValue("SongDir", REG_SZ, (BYTE*)_psInitialSongDir, strlen(_psInitialSongDir));
	}
	if (_psInitialPluginDir != NULL)
	{
		reg.SetValue("PluginDir", REG_SZ, (BYTE*)_psInitialPluginDir, strlen(_psInitialPluginDir));
	}
	if (_psInitialVstDir != NULL)
	{
		reg.SetValue("VstDir", REG_SZ, (BYTE*)_psInitialVstDir, strlen(_psInitialVstDir));
	}
	if (_psInitialSkinDir != NULL)
	{
		reg.SetValue("SkinDir", REG_SZ, (BYTE*)_psInitialSkinDir, strlen(_psInitialSkinDir));
	}
	reg.CloseKey();
	reg.CloseRootKey();
}

void Configuration::SetInstrumentDir(const char* psDir)
{
	if (_psInstrumentDir != NULL)
	{
		delete _psInstrumentDir;
	}
	_psInstrumentDir = new char[strlen(psDir)+1];
	strcpy(_psInstrumentDir, psDir);
}

void Configuration::SetInitialInstrumentDir(const char* psDir)
{
	if (_psInitialInstrumentDir != NULL)
	{
		delete _psInitialInstrumentDir;
	}
	_psInitialInstrumentDir = new char[strlen(psDir)+1];
	strcpy(_psInitialInstrumentDir, psDir);
	SetInstrumentDir(psDir);
}

void Configuration::SetSongDir(const char* psDir)
{
	if (_psSongDir != NULL)
	{
		delete _psSongDir;
	}
	_psSongDir = new char[strlen(psDir)+1];
	strcpy(_psSongDir, psDir);
}

void Configuration::SetInitialSongDir(const char* psDir)
{
	if (_psInitialSongDir != NULL)
	{
		delete _psInitialSongDir;
	}
	_psInitialSongDir = new char[strlen(psDir)+1];
	strcpy(_psInitialSongDir, psDir);
	SetSongDir(psDir);
}

void Configuration::SetSkinDir(const char* psDir)
{
	if (_psSkinDir != NULL)
	{
		delete _psSkinDir;
	}
	_psSkinDir = new char[strlen(psDir)+1];
	strcpy(_psSkinDir, psDir);
}

void Configuration::SetInitialSkinDir(const char* psDir)
{
	if (_psInitialSkinDir != NULL)
	{
		delete _psInitialSkinDir;
	}
	_psInitialSkinDir = new char[strlen(psDir)+1];
	strcpy(_psInitialSkinDir, psDir);
	SetSkinDir(psDir);
}

#endif // ndef _WINAMP_PLUGIN_

void Configuration::SetPluginDir(const char* psDir)
{
	if (_psPluginDir != NULL)
	{
		delete _psPluginDir;
	}
	_psPluginDir = new char[strlen(psDir)+1];
	strcpy(_psPluginDir, psDir);
}

void Configuration::SetInitialPluginDir(const char* psDir)
{
	if (_psInitialPluginDir != NULL)
	{
		delete _psInitialPluginDir;
	}
	_psInitialPluginDir = new char[strlen(psDir)+1];
	strcpy(_psInitialPluginDir, psDir);
	SetPluginDir(psDir);
}

void Configuration::SetVstDir(const char* psDir)
{
	if (_psVstDir != NULL)
	{
		delete _psVstDir;
	}
	_psVstDir = new char[strlen(psDir)+1];
	strcpy(_psVstDir, psDir);
}

void Configuration::SetInitialVstDir(const char* psDir)
{
	if (_psInitialVstDir != NULL)
	{
		delete _psInitialVstDir;
	}
	_psInitialVstDir = new char[strlen(psDir)+1];
	strcpy(_psInitialVstDir, psDir);
	SetVstDir(psDir);
}

void Configuration::Error(const char* psMsg)
{
	MessageBox(NULL,psMsg, "Psycle", MB_OK);
}
#if !defined(_WINAMP_PLUGIN_)
bool Configuration::CreatePsyFont(CFont &f, char * sFontFace, int HeightPx, bool bBold, bool bItalic)
{
	f.DeleteObject();
	CString sFace(sFontFace);
				
	LOGFONT lf;
	memset(&lf, 0, sizeof(LOGFONT));       // clear out structure.

	if(bBold)
		lf.lfWeight = FW_BOLD;
	if(bItalic)
		lf.lfItalic = true;
	lf.lfHeight = HeightPx;
	lf.lfQuality = NONANTIALIASED_QUALITY;
	strncpy(lf.lfFaceName,(LPCTSTR)sFace,32);

	if(!f.CreatePointFontIndirect(&lf))
	{			
		CString sFaceLowerCase = sFace;
		sFaceLowerCase.MakeLower();
		strncpy(lf.lfFaceName,(LPCTSTR)sFaceLowerCase,32);
		
		if(!f.CreatePointFontIndirect(&lf))
			return false;
	}

	return true;
}


void Configuration::CreateFonts()
{	
	bool bBold = pattern_font_flags&1?true:false;
	bool bItalic = pattern_font_flags&2?true:false;
	if (!CreatePsyFont(seqFont,pattern_fontface,pattern_font_point,bBold,bItalic))
	{
		MessageBox(NULL,pattern_fontface,"Could not find this font!",0);
		if (!CreatePsyFont(seqFont,"Tahoma",pattern_font_point,bBold,bItalic))
			if (!CreatePsyFont(seqFont,"Verdana",pattern_font_point,bBold,bItalic))
				CreatePsyFont(seqFont,"Arial",pattern_font_point,bBold,bItalic);
	}

	bBold = generator_font_flags&1;
	bItalic = generator_font_flags&2?true:false;
	if (!CreatePsyFont(generatorFont,generator_fontface,generator_font_point,bBold,bItalic))
	{
		MessageBox(NULL,generator_fontface,"Could not find this font!",0);
		if (!CreatePsyFont(seqFont,"Tahoma",generator_font_point,bBold,bItalic))
			if (!CreatePsyFont(seqFont,"Verdana",generator_font_point,bBold,bItalic))
				CreatePsyFont(seqFont,"Arial",generator_font_point,bBold,bItalic);
	}


	bBold = effect_font_flags&1;
	bItalic = effect_font_flags&2?true:false;
	if (!CreatePsyFont(effectFont,generator_fontface,effect_font_point,bBold,bItalic))
	{
		MessageBox(NULL,effect_fontface,"Could not find this font!",0);
		if (!CreatePsyFont(seqFont,"Tahoma",effect_font_point,bBold,bItalic))
			if (!CreatePsyFont(seqFont,"Verdana",effect_font_point,bBold,bItalic))
				CreatePsyFont(seqFont,"Arial",effect_font_point,bBold,bItalic);
	}

}
#endif // #if !defined(_WINAMP_PLUGIN_)
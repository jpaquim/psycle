#include "stdafx.h"

#if defined(_WINAMP_PLUGIN_)
	#include <windows.h>
	#include "Configuration.h"
	#include "Registry.h"
#else
	#include "Configuration.h"
	#include "Registry.h"
	#include "WaveOut.h"
	#include "DirectSound.h"
	#include "MidiInput.h"
#endif //_WINAMP_PLUGIN_


Configuration::Configuration()
{
	_initialized = false;

#if defined(_WINAMP_PLUGIN_)
	_psPluginDir = NULL;
	_psVstDir = NULL;
	_samplesPerSec = 44100;
#else

	_keyboardMode = 0; // Set to Standard
	_wrapAround = true;
	_centerCursor = false;
	_cursorAlwaysDown = false;
	_RecordNoteoff = false;
	_midiMachineViewSeqMode = false;
	autoStopMachines = false;
	useDoubleBuffer = true;
	_showAboutAtStart = true;
	mv_colour =	0x009a887c;
	pvc_background = 0x009a887c;
	pvc_row4beat = 0x00d5ccc6;
	pvc_rowbeat = 0x00c9beb8;
	pvc_row = 0x00c1b5aa;
	pvc_font = 0x00000000;
	// If you change the initial colour values, change it also in "CSkinDlg::OnResetcolours()"
	vu1 = 0x00f1c992;
	vu2 = 0x00403731;
	vu3 = 0x00262bd7;

	_numOutputDrivers = 3;
	_outputDriverIndex = 0;
	_midiDriverIndex = 0;			// MIDI IMPLEMENTATION
	_syncDriverIndex = 0;
	_midiHeadroom = 100;
	_ppOutputDrivers = new AudioDriver*[3];
	_ppOutputDrivers[0] = new AudioDriver;
	_ppOutputDrivers[1] = new WaveOut;
	_ppOutputDrivers[2] = new DirectSound;
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

	_midiRecordVel = FALSE;
	_midiCommandVel = 0x0c;
	_midiFromVel = 0x00;
	_midiToVel = 0xff;

	_midiRecord0 = FALSE;
	_midiType0 = 0;
	_midiMessage0 = 11;
	_midiCommand0 = 1;
	_midiFrom0 = 0;
	_midiTo0 = 0xff;

	_midiRecord1 = FALSE;
	_midiType1 = 0;
	_midiMessage1 = 14;
	_midiCommand1 = 1;
	_midiFrom1 = 0;
	_midiTo1 = 0xff;

	_midiRecord2= FALSE;
	_midiType2 = 0;
	_midiMessage2 = 0;
	_midiCommand2 = 1;
	_midiFrom2 = 0;
	_midiTo2 = 0xff;

	_midiRecord3 = FALSE;
	_midiType3 = 0;
	_midiMessage3 = 0;
	_midiCommand3 = 1;
	_midiFrom3 = 0;
	_midiTo3 = 0xff;

	_midiRecord4 = FALSE;
	_midiType4 = 0;
	_midiMessage4 = 0;
	_midiCommand4 = 1;
	_midiFrom4 = 0;
	_midiTo4 = 0xff;

	_midiRecord5 = FALSE;
	_midiType5 = 0;
	_midiMessage5 = 0;
	_midiCommand5 = 1;
	_midiFrom5 = 0;
	_midiTo5 = 0xff;

	_midiRecord6 = FALSE;
	_midiType6 = 0;
	_midiMessage6 = 0;
	_midiCommand6 = 1;
	_midiFrom6 = 0;
	_midiTo6 = 0xff;

	_midiRecord7 = FALSE;
	_midiType7 = 0;
	_midiMessage7 = 0;
	_midiCommand7 = 1;
	_midiFrom7 = 0;
	_midiTo7 = 0xff;

	_midiRecord8 = FALSE;
	_midiType8 = 0;
	_midiMessage8 = 0;
	_midiCommand8 = 1;
	_midiFrom8 = 0;
	_midiTo8 = 0xff;

	_midiRecord9 = FALSE;
	_midiType9 = 0;
	_midiMessage9 = 0;
	_midiCommand9 = 1;
	_midiFrom9 = 0;
	_midiTo9 = 0xff;

	_midiRecord10 = FALSE;
	_midiType10 = 0;
	_midiMessage10 = 0;
	_midiCommand10 = 1;
	_midiFrom10 = 0;
	_midiTo10 = 0xff;

	_midiRecord11 = FALSE;
	_midiType11 = 0;
	_midiMessage11 = 0;
	_midiCommand11 = 1;
	_midiFrom11 = 0;
	_midiTo11 = 0xff;

	_midiRecord12 = FALSE;
	_midiType12 = 0;
	_midiMessage12 = 0;
	_midiCommand12 = 1;
	_midiFrom12 = 0;
	_midiTo12 = 0xff;

	_midiRecord13 = FALSE;
	_midiType13 = 0;
	_midiMessage13 = 0;
	_midiCommand13 = 1;
	_midiFrom13 = 0;
	_midiTo13 = 0xff;

	_midiRecord14 = FALSE;
	_midiType14 = 0;
	_midiMessage14 = 0;
	_midiCommand14 = 1;
	_midiFrom14 = 0;
	_midiTo14 = 0xff;

	_midiRecord15 = FALSE;
	_midiType15 = 0;
	_midiMessage15 = 0;
	_midiCommand15 = 1;
	_midiFrom15 = 0;
	_midiTo15 = 0xff;

#endif // _WINAMP_PLUGIN
}

Configuration::~Configuration()
{

#if !defined(_WINAMP_PLUGIN_)

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
Configuration::Read(
	void)
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

	char appPath[_MAX_PATH];
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
	numData = sizeof(_keyboardMode);
	reg.QueryValue("KeyboardMode", &type, (BYTE*)&_keyboardMode, &numData);
	numData = sizeof(_wrapAround);
	reg.QueryValue("WrapAround", &type, (BYTE*)&_wrapAround, &numData);
	numData = sizeof(_centerCursor);
	reg.QueryValue("CenterCursor", &type, (BYTE*)&_centerCursor, &numData);
	numData = sizeof(_cursorAlwaysDown);
	reg.QueryValue("CursorAlwaysDown", &type, (BYTE*)&_cursorAlwaysDown, &numData);
	numData = sizeof(useDoubleBuffer);
	reg.QueryValue("useDoubleBuffer", &type, (BYTE*)&useDoubleBuffer, &numData);
	numData = sizeof(_showAboutAtStart);
	reg.QueryValue("showAboutAtStart", &type, (BYTE*)&_showAboutAtStart, &numData);
	numData = sizeof(_RecordNoteoff);
	reg.QueryValue("RecordNoteoff", &type, (BYTE*)&_RecordNoteoff, &numData);
	numData = sizeof(_midiMachineViewSeqMode);
	reg.QueryValue("MidiMachineViewSeqMode", &type, (BYTE*)&_midiMachineViewSeqMode, &numData);

	numData = sizeof(_midiRecordVel);
	reg.QueryValue("MidiRecordVel", &type, (BYTE*)&_midiRecordVel, &numData);
	numData = sizeof(_midiCommandVel);
	reg.QueryValue("MidiCommandVel", &type, (BYTE*)&_midiCommandVel, &numData);
	numData = sizeof(_midiFromVel);
	reg.QueryValue("MidiFromVel", &type, (BYTE*)&_midiFromVel, &numData);
	numData = sizeof(_midiToVel);
	reg.QueryValue("MidiToVel", &type, (BYTE*)&_midiToVel, &numData);

	numData = sizeof(_midiRecord0);
	reg.QueryValue("MidiRecord0", &type, (BYTE*)&_midiRecord0, &numData);
	numData = sizeof(_midiCommand0);
	reg.QueryValue("MidiMessage0", &type, (BYTE*)&_midiMessage0, &numData);
	numData = sizeof(_midiMessage0);
	reg.QueryValue("MidiType0", &type, (BYTE*)&_midiType0, &numData);
	numData = sizeof(_midiType0);
	reg.QueryValue("MidiCommand0", &type, (BYTE*)&_midiCommand0, &numData);
	numData = sizeof(_midiFrom0);
	reg.QueryValue("MidiFrom0", &type, (BYTE*)&_midiFrom0, &numData);
	numData = sizeof(_midiTo0);
	reg.QueryValue("MidiTo0", &type, (BYTE*)&_midiTo0, &numData);

	numData = sizeof(_midiRecord1);
	reg.QueryValue("MidiRecord1", &type, (BYTE*)&_midiRecord1, &numData);
	numData = sizeof(_midiCommand1);
	reg.QueryValue("MidiMessage1", &type, (BYTE*)&_midiMessage1, &numData);
	numData = sizeof(_midiMessage1);
	reg.QueryValue("MidiType1", &type, (BYTE*)&_midiType1, &numData);
	numData = sizeof(_midiType1);
	reg.QueryValue("MidiCommand1", &type, (BYTE*)&_midiCommand1, &numData);
	numData = sizeof(_midiFrom1);
	reg.QueryValue("MidiFrom1", &type, (BYTE*)&_midiFrom1, &numData);
	numData = sizeof(_midiTo1);
	reg.QueryValue("MidiTo1", &type, (BYTE*)&_midiTo1, &numData);

	numData = sizeof(_midiRecord2);
	reg.QueryValue("MidiRecord2", &type, (BYTE*)&_midiRecord2, &numData);
	numData = sizeof(_midiCommand2);
	reg.QueryValue("MidiMessage2", &type, (BYTE*)&_midiMessage2, &numData);
	numData = sizeof(_midiMessage2);
	reg.QueryValue("MidiType2", &type, (BYTE*)&_midiType2, &numData);
	numData = sizeof(_midiType2);
	reg.QueryValue("MidiCommand2", &type, (BYTE*)&_midiCommand2, &numData);
	numData = sizeof(_midiFrom2);
	reg.QueryValue("MidiFrom2", &type, (BYTE*)&_midiFrom2, &numData);
	numData = sizeof(_midiTo2);
	reg.QueryValue("MidiTo2", &type, (BYTE*)&_midiTo2, &numData);

	numData = sizeof(_midiRecord3);
	reg.QueryValue("MidiRecord3", &type, (BYTE*)&_midiRecord3, &numData);
	numData = sizeof(_midiCommand3);
	reg.QueryValue("MidiMessage3", &type, (BYTE*)&_midiMessage3, &numData);
	numData = sizeof(_midiMessage3);
	reg.QueryValue("MidiType3", &type, (BYTE*)&_midiType3, &numData);
	numData = sizeof(_midiType3);
	reg.QueryValue("MidiCommand3", &type, (BYTE*)&_midiCommand3, &numData);
	numData = sizeof(_midiFrom3);
	reg.QueryValue("MidiFrom3", &type, (BYTE*)&_midiFrom3, &numData);
	numData = sizeof(_midiTo3);
	reg.QueryValue("MidiTo3", &type, (BYTE*)&_midiTo3, &numData);

	numData = sizeof(_midiRecord4);
	reg.QueryValue("MidiRecord4", &type, (BYTE*)&_midiRecord4, &numData);
	numData = sizeof(_midiCommand4);
	reg.QueryValue("MidiMessage4", &type, (BYTE*)&_midiMessage4, &numData);
	numData = sizeof(_midiMessage4);
	reg.QueryValue("MidiType4", &type, (BYTE*)&_midiType4, &numData);
	numData = sizeof(_midiType4);
	reg.QueryValue("MidiCommand4", &type, (BYTE*)&_midiCommand4, &numData);
	numData = sizeof(_midiFrom4);
	reg.QueryValue("MidiFrom4", &type, (BYTE*)&_midiFrom4, &numData);
	numData = sizeof(_midiTo4);
	reg.QueryValue("MidiTo4", &type, (BYTE*)&_midiTo4, &numData);

	numData = sizeof(_midiRecord5);
	reg.QueryValue("MidiRecord5", &type, (BYTE*)&_midiRecord5, &numData);
	numData = sizeof(_midiCommand5);
	reg.QueryValue("MidiMessage5", &type, (BYTE*)&_midiMessage5, &numData);
	numData = sizeof(_midiMessage5);
	reg.QueryValue("MidiType5", &type, (BYTE*)&_midiType5, &numData);
	numData = sizeof(_midiType5);
	reg.QueryValue("MidiCommand5", &type, (BYTE*)&_midiCommand5, &numData);
	numData = sizeof(_midiFrom5);
	reg.QueryValue("MidiFrom5", &type, (BYTE*)&_midiFrom5, &numData);
	numData = sizeof(_midiTo5);
	reg.QueryValue("MidiTo5", &type, (BYTE*)&_midiTo5, &numData);

	numData = sizeof(_midiRecord6);
	reg.QueryValue("MidiRecord6", &type, (BYTE*)&_midiRecord6, &numData);
	numData = sizeof(_midiCommand6);
	reg.QueryValue("MidiMessage6", &type, (BYTE*)&_midiMessage6, &numData);
	numData = sizeof(_midiMessage6);
	reg.QueryValue("MidiType6", &type, (BYTE*)&_midiType6, &numData);
	numData = sizeof(_midiType6);
	reg.QueryValue("MidiCommand6", &type, (BYTE*)&_midiCommand6, &numData);
	numData = sizeof(_midiFrom6);
	reg.QueryValue("MidiFrom6", &type, (BYTE*)&_midiFrom6, &numData);
	numData = sizeof(_midiTo6);
	reg.QueryValue("MidiTo6", &type, (BYTE*)&_midiTo6, &numData);

	numData = sizeof(_midiRecord7);
	reg.QueryValue("MidiRecord7", &type, (BYTE*)&_midiRecord7, &numData);
	numData = sizeof(_midiCommand7);
	reg.QueryValue("MidiMessage7", &type, (BYTE*)&_midiMessage7, &numData);
	numData = sizeof(_midiMessage7);
	reg.QueryValue("MidiType7", &type, (BYTE*)&_midiType7, &numData);
	numData = sizeof(_midiType7);
	reg.QueryValue("MidiCommand7", &type, (BYTE*)&_midiCommand7, &numData);
	numData = sizeof(_midiFrom7);
	reg.QueryValue("MidiFrom7", &type, (BYTE*)&_midiFrom7, &numData);
	numData = sizeof(_midiTo7);
	reg.QueryValue("MidiTo7", &type, (BYTE*)&_midiTo7, &numData);

	numData = sizeof(_midiRecord8);
	reg.QueryValue("MidiRecord8", &type, (BYTE*)&_midiRecord8, &numData);
	numData = sizeof(_midiCommand8);
	reg.QueryValue("MidiMessage8", &type, (BYTE*)&_midiMessage8, &numData);
	numData = sizeof(_midiMessage8);
	reg.QueryValue("MidiType8", &type, (BYTE*)&_midiType8, &numData);
	numData = sizeof(_midiType8);
	reg.QueryValue("MidiCommand8", &type, (BYTE*)&_midiCommand8, &numData);
	numData = sizeof(_midiFrom8);
	reg.QueryValue("MidiFrom8", &type, (BYTE*)&_midiFrom8, &numData);
	numData = sizeof(_midiTo8);
	reg.QueryValue("MidiTo8", &type, (BYTE*)&_midiTo8, &numData);

	numData = sizeof(_midiRecord9);
	reg.QueryValue("MidiRecord9", &type, (BYTE*)&_midiRecord9, &numData);
	numData = sizeof(_midiCommand9);
	reg.QueryValue("MidiMessage9", &type, (BYTE*)&_midiMessage9, &numData);
	numData = sizeof(_midiMessage9);
	reg.QueryValue("MidiType9", &type, (BYTE*)&_midiType9, &numData);
	numData = sizeof(_midiType9);
	reg.QueryValue("MidiCommand9", &type, (BYTE*)&_midiCommand9, &numData);
	numData = sizeof(_midiFrom9);
	reg.QueryValue("MidiFrom9", &type, (BYTE*)&_midiFrom9, &numData);
	numData = sizeof(_midiTo9);
	reg.QueryValue("MidiTo9", &type, (BYTE*)&_midiTo9, &numData);

	numData = sizeof(_midiRecord10);
	reg.QueryValue("MidiRecord10", &type, (BYTE*)&_midiRecord10, &numData);
	numData = sizeof(_midiCommand10);
	reg.QueryValue("MidiMessage10", &type, (BYTE*)&_midiMessage10, &numData);
	numData = sizeof(_midiMessage10);
	reg.QueryValue("MidiType10", &type, (BYTE*)&_midiType10, &numData);
	numData = sizeof(_midiType10);
	reg.QueryValue("MidiCommand10", &type, (BYTE*)&_midiCommand10, &numData);
	numData = sizeof(_midiFrom10);
	reg.QueryValue("MidiFrom10", &type, (BYTE*)&_midiFrom10, &numData);
	numData = sizeof(_midiTo10);
	reg.QueryValue("MidiTo10", &type, (BYTE*)&_midiTo10, &numData);

	numData = sizeof(_midiRecord11);
	reg.QueryValue("MidiRecord11", &type, (BYTE*)&_midiRecord11, &numData);
	numData = sizeof(_midiCommand11);
	reg.QueryValue("MidiMessage11", &type, (BYTE*)&_midiMessage11, &numData);
	numData = sizeof(_midiMessage11);
	reg.QueryValue("MidiType11", &type, (BYTE*)&_midiType11, &numData);
	numData = sizeof(_midiType11);
	reg.QueryValue("MidiCommand11", &type, (BYTE*)&_midiCommand11, &numData);
	numData = sizeof(_midiFrom11);
	reg.QueryValue("MidiFrom11", &type, (BYTE*)&_midiFrom11, &numData);
	numData = sizeof(_midiTo11);
	reg.QueryValue("MidiTo11", &type, (BYTE*)&_midiTo11, &numData);

	numData = sizeof(_midiRecord12);
	reg.QueryValue("MidiRecord12", &type, (BYTE*)&_midiRecord12, &numData);
	numData = sizeof(_midiCommand12);
	reg.QueryValue("MidiMessage12", &type, (BYTE*)&_midiMessage12, &numData);
	numData = sizeof(_midiMessage12);
	reg.QueryValue("MidiType12", &type, (BYTE*)&_midiType12, &numData);
	numData = sizeof(_midiType12);
	reg.QueryValue("MidiCommand12", &type, (BYTE*)&_midiCommand12, &numData);
	numData = sizeof(_midiFrom12);
	reg.QueryValue("MidiFrom12", &type, (BYTE*)&_midiFrom12, &numData);
	numData = sizeof(_midiTo12);
	reg.QueryValue("MidiTo12", &type, (BYTE*)&_midiTo12, &numData);

	numData = sizeof(_midiRecord13);
	reg.QueryValue("MidiRecord13", &type, (BYTE*)&_midiRecord13, &numData);
	numData = sizeof(_midiCommand13);
	reg.QueryValue("MidiMessage13", &type, (BYTE*)&_midiMessage13, &numData);
	numData = sizeof(_midiMessage13);
	reg.QueryValue("MidiType13", &type, (BYTE*)&_midiType13, &numData);
	numData = sizeof(_midiType13);
	reg.QueryValue("MidiCommand13", &type, (BYTE*)&_midiCommand13, &numData);
	numData = sizeof(_midiFrom13);
	reg.QueryValue("MidiFrom13", &type, (BYTE*)&_midiFrom13, &numData);
	numData = sizeof(_midiTo13);
	reg.QueryValue("MidiTo13", &type, (BYTE*)&_midiTo13, &numData);

	numData = sizeof(_midiRecord14);
	reg.QueryValue("MidiRecord14", &type, (BYTE*)&_midiRecord14, &numData);
	numData = sizeof(_midiCommand14);
	reg.QueryValue("MidiMessage14", &type, (BYTE*)&_midiMessage14, &numData);
	numData = sizeof(_midiMessage14);
	reg.QueryValue("MidiType14", &type, (BYTE*)&_midiType14, &numData);
	numData = sizeof(_midiType14);
	reg.QueryValue("MidiCommand14", &type, (BYTE*)&_midiCommand14, &numData);
	numData = sizeof(_midiFrom14);
	reg.QueryValue("MidiFrom14", &type, (BYTE*)&_midiFrom14, &numData);
	numData = sizeof(_midiTo14);
	reg.QueryValue("MidiTo14", &type, (BYTE*)&_midiTo14, &numData);

	numData = sizeof(_midiRecord15);
	reg.QueryValue("MidiRecord15", &type, (BYTE*)&_midiRecord15, &numData);
	numData = sizeof(_midiCommand15);
	reg.QueryValue("MidiMessage15", &type, (BYTE*)&_midiMessage15, &numData);
	numData = sizeof(_midiMessage15);
	reg.QueryValue("MidiType15", &type, (BYTE*)&_midiType15, &numData);
	numData = sizeof(_midiType15);
	reg.QueryValue("MidiCommand15", &type, (BYTE*)&_midiCommand15, &numData);
	numData = sizeof(_midiFrom15);
	reg.QueryValue("MidiFrom15", &type, (BYTE*)&_midiFrom15, &numData);
	numData = sizeof(_midiTo15);
	reg.QueryValue("MidiTo15", &type, (BYTE*)&_midiTo15, &numData);

	numData = sizeof(mv_colour);
	reg.QueryValue("mv_colour", &type, (BYTE*)&mv_colour, &numData);
	numData = sizeof(pvc_background);
	reg.QueryValue("pvc_background", &type, (BYTE*)&pvc_background, &numData);
	numData = sizeof(pvc_row4beat);
	reg.QueryValue("pvc_row4beat", &type, (BYTE*)&pvc_row4beat, &numData);
	numData = sizeof(pvc_rowbeat);
	reg.QueryValue("pvc_rowbeat", &type, (BYTE*)&pvc_rowbeat, &numData);
	numData = sizeof(pvc_row);
	reg.QueryValue("pvc_row", &type, (BYTE*)&pvc_row, &numData);
	numData = sizeof(pvc_font);
	reg.QueryValue("pvc_font", &type, (BYTE*)&pvc_font, &numData);
	numData = sizeof(vu1);
	reg.QueryValue("vu1", &type, (BYTE*)&vu1, &numData);
	numData = sizeof(vu2);
	reg.QueryValue("vu2", &type, (BYTE*)&vu2, &numData);
	numData = sizeof(vu3);
	reg.QueryValue("vu3", &type, (BYTE*)&vu3, &numData);

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
		_psInitialInstrumentDir = new char[numData];
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
		_psInitialSongDir = new char[numData];
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
#endif // ndef _WINAMP_PLUGIN_

	if (_psInitialPluginDir != NULL)
	{
		delete _psInitialPluginDir;
		_psInitialPluginDir = NULL;
	}
	numData = sizeof(string);
	if (reg.QueryValue("PluginDir", &type, (BYTE*)&string, &numData) == ERROR_SUCCESS)
	{
		_psInitialPluginDir = new char[numData];
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
		_psInitialVstDir = new char[numData];
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
Configuration::Write(
	void)
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
	reg.SetValue("KeyboardMode", REG_DWORD, (BYTE*)&_keyboardMode, sizeof(_keyboardMode));
	reg.SetValue("WrapAround", REG_BINARY, (BYTE*)&_wrapAround, sizeof(_wrapAround));
	reg.SetValue("CenterCursor", REG_BINARY, (BYTE*)&_centerCursor, sizeof(_centerCursor));
	reg.SetValue("CursorAlwaysDown", REG_BINARY, (BYTE*)&_cursorAlwaysDown, sizeof(_cursorAlwaysDown));
	reg.SetValue("useDoubleBuffer", REG_BINARY, (BYTE*)&useDoubleBuffer, sizeof(useDoubleBuffer));
	reg.SetValue("showAboutAtStart", REG_BINARY, (BYTE*)&_showAboutAtStart, sizeof(_showAboutAtStart));
	reg.SetValue("RecordNoteoff", REG_BINARY, (BYTE*)&_RecordNoteoff, sizeof(_RecordNoteoff));
	reg.SetValue("MidiMachineViewSeqMode", REG_BINARY, (BYTE*)&_midiMachineViewSeqMode, sizeof(_midiMachineViewSeqMode));
	reg.SetValue("OutputDriver", REG_DWORD, (BYTE*)&_outputDriverIndex, sizeof(_outputDriverIndex));
	reg.SetValue("MidiInputDriver", REG_DWORD, (BYTE*)&_midiDriverIndex, sizeof(_midiDriverIndex));	// MIDI IMPLEMENTATION
	reg.SetValue("MidiSyncDriver", REG_DWORD, (BYTE*)&_syncDriverIndex, sizeof(_syncDriverIndex));	// MIDI IMPLEMENTATION
	reg.SetValue("MidiInputHeadroom", REG_DWORD, (BYTE*)&_midiHeadroom, sizeof(_midiHeadroom));	

	reg.SetValue("MidiRecordVel", REG_BINARY, (BYTE*)&_midiRecordVel, sizeof(_midiRecordVel));
	reg.SetValue("MidiCommandVel", REG_DWORD, (BYTE*)&_midiCommandVel, sizeof(_midiCommandVel));	
	reg.SetValue("MidiFromVel", REG_DWORD, (BYTE*)&_midiFromVel, sizeof(_midiFromVel));	
	reg.SetValue("MidiToVel", REG_DWORD, (BYTE*)&_midiToVel, sizeof(_midiToVel));	
	
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

	reg.SetValue("mv_colour", REG_DWORD, (BYTE*)&mv_colour, sizeof(mv_colour));	
	reg.SetValue("pvc_background", REG_DWORD, (BYTE*)&pvc_background, sizeof(pvc_background));	
	reg.SetValue("pvc_row4beat", REG_DWORD, (BYTE*)&pvc_row4beat, sizeof(pvc_row4beat));	
	reg.SetValue("pvc_rowbeat", REG_DWORD, (BYTE*)&pvc_rowbeat, sizeof(pvc_rowbeat));	
	reg.SetValue("pvc_row", REG_DWORD, (BYTE*)&pvc_row, sizeof(pvc_row));	
	reg.SetValue("pvc_font", REG_DWORD, (BYTE*)&pvc_font, sizeof(pvc_font));	
	reg.SetValue("vu1", REG_DWORD, (BYTE*)&vu1, sizeof(vu1));	
	reg.SetValue("vu2", REG_DWORD, (BYTE*)&vu2, sizeof(vu2));	
	reg.SetValue("vu3", REG_DWORD, (BYTE*)&vu3, sizeof(vu3));	
	
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
	reg.CloseKey();
	reg.CloseRootKey();
}

void
Configuration::SetInstrumentDir(
	const char* psDir)
{
	if (_psInstrumentDir != NULL)
	{
		delete _psInstrumentDir;
	}
	_psInstrumentDir = new char[strlen(psDir)+1];
	strcpy(_psInstrumentDir, psDir);
}

void
Configuration::SetInitialInstrumentDir(
	const char* psDir)
{
	if (_psInitialInstrumentDir != NULL)
	{
		delete _psInitialInstrumentDir;
	}
	_psInitialInstrumentDir = new char[strlen(psDir)+1];
	strcpy(_psInitialInstrumentDir, psDir);
	SetInstrumentDir(psDir);
}

void
Configuration::SetSongDir(
	const char* psDir)
{
	if (_psSongDir != NULL)
	{
		delete _psSongDir;
	}
	_psSongDir = new char[strlen(psDir)+1];
	strcpy(_psSongDir, psDir);
}

void
Configuration::SetInitialSongDir(
	const char* psDir)
{
	if (_psInitialSongDir != NULL)
	{
		delete _psInitialSongDir;
	}
	_psInitialSongDir = new char[strlen(psDir)+1];
	strcpy(_psInitialSongDir, psDir);
	SetSongDir(psDir);
}

#endif // ndef _WINAMP_PLUGIN_

void
Configuration::SetPluginDir(
	const char* psDir)
{
	if (_psPluginDir != NULL)
	{
		delete _psPluginDir;
	}
	_psPluginDir = new char[strlen(psDir)+1];
	strcpy(_psPluginDir, psDir);
}

void
Configuration::SetInitialPluginDir(
	const char* psDir)
{
	if (_psInitialPluginDir != NULL)
	{
		delete _psInitialPluginDir;
	}
	_psInitialPluginDir = new char[strlen(psDir)+1];
	strcpy(_psInitialPluginDir, psDir);
	SetPluginDir(psDir);
}

void
Configuration::SetVstDir(
	const char* psDir)
{
	if (_psVstDir != NULL)
	{
		delete _psVstDir;
	}
	_psVstDir = new char[strlen(psDir)+1];
	strcpy(_psVstDir, psDir);
}

void
Configuration::SetInitialVstDir(
	const char* psDir)
{
	if (_psInitialVstDir != NULL)
	{
		delete _psInitialVstDir;
	}
	_psInitialVstDir = new char[strlen(psDir)+1];
	strcpy(_psInitialVstDir, psDir);
	SetVstDir(psDir);
}

void
Configuration::Error(
	const char* psMsg)
{
	MessageBox(NULL,psMsg, "Psycle", MB_OK);
}

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
	_midiRecordNoteoff = false;
	_midiMachineViewSeqMode = false;
	autoStopMachines = false;
	useDoubleBuffer = true;
	_showAboutAtStart = true;
	mv_colour =	0x00bfa880;
	pvc_background = 0x00decaab;
	pvc_row4beat = 0x00dec9ab;
	pvc_rowbeat = 0x00ebddcb;
	pvc_row = 0x00f3ebe0;
	pvc_font = 0x00000000;
	// If you change the initial colour values, change it also in "CSkinDlg::OnResetcolours()"
	vu1 = 0x00d6c6a9;
	vu2 = 0x00000000;
	vu3 = 0x00d6c6a9;

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
	numData = sizeof(_midiRecordNoteoff);
	reg.QueryValue("MidiRecordNoteoff", &type, (BYTE*)&_midiRecordNoteoff, &numData);
	numData = sizeof(_midiMachineViewSeqMode);
	reg.QueryValue("MidiMachineViewSeqMode", &type, (BYTE*)&_midiMachineViewSeqMode, &numData);

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
	reg.SetValue("MidiRecordNoteoff", REG_BINARY, (BYTE*)&_midiRecordNoteoff, sizeof(_midiRecordNoteoff));
	reg.SetValue("MidiMachineViewSeqMode", REG_BINARY, (BYTE*)&_midiMachineViewSeqMode, sizeof(_midiMachineViewSeqMode));
	reg.SetValue("OutputDriver", REG_DWORD, (BYTE*)&_outputDriverIndex, sizeof(_outputDriverIndex));
	reg.SetValue("MidiInputDriver", REG_DWORD, (BYTE*)&_midiDriverIndex, sizeof(_midiDriverIndex));	// MIDI IMPLEMENTATION
	reg.SetValue("MidiSyncDriver", REG_DWORD, (BYTE*)&_syncDriverIndex, sizeof(_syncDriverIndex));	// MIDI IMPLEMENTATION
	reg.SetValue("MidiInputHeadroom", REG_DWORD, (BYTE*)&_midiHeadroom, sizeof(_midiHeadroom));	

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

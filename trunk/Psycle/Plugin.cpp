#include "stdafx.h"

#if defined(_WINAMP_PLUGIN_)
	#include "global.h"
	#include "Plugin.h"
	#include "FileIO.h"
#else
	#include "psycle2.h"
	#include "Plugin.h"
	#include "FileIO.h"
	#include "NewMachine.h"
#endif // _WINAMP_PLUGIN_

typedef CMachineInfo * (*GETINFO)(void);
typedef CMachineInterface * (*CREATEMACHINE)(void);

PluginFxCallback Plugin::_callback;

Plugin::Plugin()
{
	_type = MACH_PLUGIN;
	_mode = MACHMODE_PLUGIN;
	sprintf(_editName, "Plugin");
	_dll = NULL;
	_pInterface = NULL;
	_psAuthor=NULL;
	_psDllName=NULL;
	_psName=NULL;
}

Plugin::~Plugin()
{
	Free();
	delete _psAuthor;	// delete NULL has no effect, so It's safe.
	delete _psDllName;	//
	delete _psName;		//
	delete _pInterface;	//

}

bool Plugin::Instance(char* psFileName)
{
	_dll = LoadLibrary(psFileName);

	if (_dll == NULL) return false;

	GETINFO GetInfo =(GETINFO)GetProcAddress(_dll, "GetInfo");
	if (GetInfo == NULL)
	{
		FreeLibrary(_dll);
		_dll=NULL;
		return false;
	}

	_pInfo = GetInfo();
	
	if ( _pInfo->Version < MI_VERSION )
	{
		FreeLibrary(_dll);
		_dll=NULL;
		return false;
	}

	_isSynth = (_pInfo->Flags == 3 );
	if (_isSynth )	_mode = MACHMODE_GENERATOR;

	strncpy(_psShortName,_pInfo->ShortName,15);
	_psShortName[15]='\0';
	strncpy(_editName, _pInfo->ShortName,15);
	_editName[15]='\0';

	_psAuthor = new char[strlen(_pInfo->Author)+1];
	strcpy(_psAuthor,_pInfo->Author);

	_psName = new char[strlen(_pInfo->Name)+1];
	strcpy(_psName,_pInfo->Name);

	_psDllName = new char[strlen(psFileName)+1];
	strcpy(_psDllName, psFileName);

	CREATEMACHINE GetInterface =(CREATEMACHINE)GetProcAddress(_dll, "CreateMachine");
	if (GetInterface == NULL)
	{
		FreeLibrary(_dll);
		_dll=NULL;
		return false;
	}

	_pInterface = GetInterface();
	_pInterface->pCB = &_callback;

	return true;
}
void Plugin::Free(void)
{
	if (_dll != NULL)
	{
		delete _pInterface;
		_pInterface = NULL;
		FreeLibrary(_dll);
		_dll=NULL;
	}
}
/*bool Plugin::Create(Plugin *plug)
{
	_dll=plug->_dll;
	_pInfo = plug->_pInfo;
	_isSynth = plug->_isSynth;

	strcpy(_psShortName,plug->_psShortName);

	_psAuthor = new char[strlen(plug->_psAuthor)+1];
	strcpy(_psAuthor,plug->_psAuthor);

	_psName = new char[strlen(plug->_psName)+1];
	strcpy(_psName,plug->_psName);

	_psDllName = new char[strlen(plug->_psDllName)+1];
	strcpy(_psDllName, plug->_psDllName);

	_pInterface = plug->_pInterface;
	
	_isSynth = plug->_isSynth;

	strcpy(_editName, plug->_editName);

	plug->_dll=NULL;	// This is to avoid the "Free()" call when "plug" is destroyed.
	Init();
	return true;
}*/
void Plugin::Init(void)
{
	Machine::Init();

	if (_pInterface!= NULL ) 
	{
		_pInterface->Init();
		for (int gbp = 0; gbp<_pInfo->numParameters; gbp++)
		{
			_pInterface->ParameterTweak(gbp, _pInfo->Parameters[gbp]->DefValue);
		}
	}
}

void Plugin::Work(
	int numSamples)
{
	if (_mode != MACHMODE_GENERATOR)
	{
		Machine::Work(numSamples);
	}

#if defined(_WINAMP_PLUGIN_)
	_pInterface->Work(_pSamplesL, _pSamplesR, numSamples, Global::_pSong->SONGTRACKS);
#else
	CPUCOST_INIT(cost);
	if ((!_mute) && (_mode == MACHMODE_GENERATOR || (!_stopped && !_bypass)))
	{
		_pInterface->Work(_pSamplesL, _pSamplesR, numSamples, Global::_pSong->SONGTRACKS);
		if ( Global::pConfig->autoStopMachines )
		{
			Machine::SetVolumeCounterAccurate(numSamples);
			if (_volumeCounter < 8)	{
				_volumeCounter = 0;
				_stopped = true;
			}
			else _stopped = false;
		}
		else Machine::SetVolumeCounter(numSamples);
	}
	CPUCOST_CALC(cost, numSamples);
	_cpuCost += cost;
#endif // ndef _WINAMP_PLUGIN_

	_worked = true;
}

void Plugin::Stop(void)
{
	_pInterface->Stop();
}

void Plugin::Tick(void)
{
	_pInterface->SequencerTick();
}

void Plugin::Tick(
	int channel,
	PatternEntry* pData)
{
	if (_mode == MACHMODE_GENERATOR)
	{
		_pInterface->SeqTick(channel ,pData->_note, pData->_inst, pData->_cmd, pData->_parameter);
	}

	if (pData->_note == 121 || pData->_note == 122 )
	{
		if (pData->_inst < _pInfo->numParameters)
		{

			int nv = (pData->_cmd<<8)+pData->_parameter;
			int const min = _pInfo->Parameters[pData->_inst]->MinValue;
			int const max = _pInfo->Parameters[pData->_inst]->MaxValue;

			nv += min;
			if (nv > max)
			{
				nv = max;
			}

			_pInterface->ParameterTweak(pData->_inst, nv);
#if !defined(_WINAMP_PLUGIN_)
			Global::_pSong->Tweaker = true;
#endif // ndef _WINAMP_PLUGIN_
		}
	}
}

bool Plugin::Load(
	RiffFile* pFile)
{
	bool result = true;
	char junk[256];
	memset(&junk, 0, sizeof(junk));

	char sDllName[256];
	int numParameters;

		CString sPath;
		char sPath2[_MAX_PATH];

		pFile->Read(sDllName, sizeof(sDllName)); // Plugin dll name
		_strlwr(sDllName);

		//Patch: Automatically replace old AS's by AS2F.
		bool wasAB=false;
		bool wasAS1=false;
		if (strcmp(sDllName,"arguru bass.dll" ) == 0)
		{
			strcpy(sDllName,"arguru synth 2f.dll");
			wasAB=true;
		}
		if (strcmp(sDllName,"arguru synth.dll" ) == 0)
		{
			strcpy(sDllName,"arguru synth 2f.dll");
			wasAS1=true;
		}
		if (strcmp(sDllName,"arguru synth 2.dll" ) == 0)
			strcpy(sDllName,"arguru synth 2f.dll");
		if (strcmp(sDllName,"synth21.dll" ) == 0)
			strcpy(sDllName,"arguru synth 2f.dll");

#if defined(_WINAMP_PLUGIN_
		sPath = Global::pConfig->GetPluginDir();

		if ( FindFileinDir(sDllName,sPath) )
		{
			strcpy(sPath2,sPath);
			if (!Instance(sPath2)) result=false;
		}
		else result = false;
#else
		if ( !CNewMachine::dllNames.Lookup(sDllName,sPath) ) 
		{
//			Check Compatibility Table.
//			Probably could be done with the dllNames lockup.
//
//			GetCompatible(sDllName,sPath2) // If no one found, it will return a null string.
			strcpy(sPath2,sDllName);
		}
		else { strcpy(sPath2,sPath); }

		if (!Instance(sPath2))
		{
			char sError[_MAX_PATH];
			sprintf(sError,"Missing or corrupted native Plug-in \"%s\"",sDllName);
			::MessageBox(NULL,sError, "Error", MB_OK);
			result = false;
		}
#endif // _WINAMP_PLUGIN_
	Init();
	pFile->Read(&_editName, sizeof(_editName));

		pFile->Read(&numParameters, sizeof(numParameters));
		if (result)
		{
			int *Vals = new int[numParameters];
			pFile->Read(Vals, numParameters*sizeof(int));

			if ( wasAB ) // Patch to replace Arguru Bass by Arguru Synth 2f
			{
				_pInterface->ParameterTweak(0,Vals[0]);
				for (int i=1;i<15;i++)
				{
					_pInterface->ParameterTweak(i+4,Vals[i]);
				}
				_pInterface->ParameterTweak(19,0);
				_pInterface->ParameterTweak(20,Vals[15]);

				if (numParameters>16)
				{
					_pInterface->ParameterTweak(24,Vals[16]);
					_pInterface->ParameterTweak(25,Vals[17]);
				}
			}
			else for (int i=0; i<numParameters; i++)
			{
				_pInterface->ParameterTweak(i,Vals[i]);
			}
			int size = _pInterface->GetDataSize();
			//pFile->Read(&size,sizeof(int));	// This SHOULD be the right thing to do
			if (size)
			{
				byte* pData = new byte[size];
				pFile->Read(pData, size); // Number of parameters
				_pInterface->PutData(pData); // Internal load
				delete pData;
			}
			
			if ( wasAS1 )	// Patch to replace Synth1 by Arguru Synth 2f
			{
				_pInterface->ParameterTweak(17,Vals[17]+10);
			}

		}
		else
		{
			for (int i=0; i<numParameters; i++)
			{
				pFile->Read(&junk[0], sizeof(int));			
			}
			/*int size;		// This SHOULD be done, but it breaks the fileformat.
			pFile->Read(&size,sizeof(int));
			if (size)
			{
				byte* pData = new byte[size];
				pFile->Read(pData, size); // Number of parameters
				delete pData;
			}*/
		}

	pFile->Read(&_inputMachines[0], sizeof(_inputMachines));
	pFile->Read(&_outputMachines[0], sizeof(_outputMachines));
	pFile->Read(&_inputConVol[0], sizeof(_inputConVol));
	pFile->Read(&_connection[0], sizeof(_connection));
	pFile->Read(&_inputCon[0], sizeof(_inputCon));
	pFile->Read(&_connectionPoint[0], sizeof(_connectionPoint));
	pFile->Read(&_numInputs, sizeof(_numInputs));
	pFile->Read(&_numOutputs, sizeof(_numOutputs));

	pFile->Read(&_panning, sizeof(_panning));
	Machine::SetPan(_panning);
	pFile->Read(&junk[0], 8*sizeof(int)); // SubTrack[]
	pFile->Read(&junk[0], sizeof(int)); // numSubtracks
	pFile->Read(&junk[0], sizeof(int)); // interpol

	pFile->Read(&_outDry, sizeof(_outDry));
	pFile->Read(&_outWet, sizeof(_outWet));

	pFile->Read(&junk[0], sizeof(int)); // distPosThreshold
	pFile->Read(&junk[0], sizeof(int)); // distPosClamp
	pFile->Read(&junk[0], sizeof(int)); // distNegThreshold
	pFile->Read(&junk[0], sizeof(int)); // distNegClamp

	pFile->Read(&junk[0], sizeof(char)); // sinespeed
	pFile->Read(&junk[0], sizeof(char)); // sineglide
	pFile->Read(&junk[0], sizeof(char)); // sinevolume
	pFile->Read(&junk[0], sizeof(char)); // sinelfospeed
	pFile->Read(&junk[0], sizeof(char)); // sinelfoamp

	pFile->Read(&junk[0], sizeof(int)); // delayTimeL
	pFile->Read(&junk[0], sizeof(int)); // delayTimeR
	pFile->Read(&junk[0], sizeof(int)); // delayFeedbackL
	pFile->Read(&junk[0], sizeof(int)); // delayFeedbackR

	pFile->Read(&junk[0], sizeof(int)); // filterCutoff
	pFile->Read(&junk[0], sizeof(int)); // filterResonance
	pFile->Read(&junk[0], sizeof(int)); // filterLfospeed
	pFile->Read(&junk[0], sizeof(int)); // filterLfoamp
	pFile->Read(&junk[0], sizeof(int)); // filterLfophase
	pFile->Read(&junk[0], sizeof(int)); // filterMode

	return result;
}
#if !defined(_WINAMP_PLUGIN_)
bool Plugin::Save(
	RiffFile* pFile)
{
	char junk[256];
	memset(&junk, 0, sizeof(junk));

	pFile->Write(&_x, sizeof(_x));
	pFile->Write(&_y, sizeof(_y));
	pFile->Write(&_type, sizeof(_type));

		CString str = _psDllName;
		char str2[256];
		strcpy(str2,str.Mid(str.ReverseFind('\\')+1));
		pFile->Write(&str2,sizeof(str2));

	pFile->Write(&_editName, sizeof(_editName));

		int j=_pInfo->numParameters;
		pFile->Write(&j, sizeof(j)); // Number of parameters
		pFile->Write(_pInterface->Vals, j*sizeof(int)); // Parameters
		int size = _pInterface->GetDataSize();
		//pFile->Write(&size,sizeof(int));	// This SHOULD be the right thing to do
		if (size)
		{
			byte* pData = new byte[size];
			_pInterface->GetData(pData); // Internal save
			pFile->Write(pData, size); // Number of parameters
			delete pData;
		}

	pFile->Write(&_inputMachines[0], sizeof(_inputMachines));
	pFile->Write(&_outputMachines[0], sizeof(_outputMachines));

	float tmpvol[MAX_CONNECTIONS];
	memcpy(tmpvol,_inputConVol,MAX_CONNECTIONS*sizeof(float));
	for (int i=0; i<MAX_CONNECTIONS;i++) // Just a conversion to the new Values used.
	{
		if ( _inputCon[i] )
		{
			MachineType type =Global::_pSong->_pMachines[_inputMachines[i]]->_type;

			if ( type != MACH_VST || type != MACH_VSTFX ) tmpvol[i]*=0.000030517578125f;
		}
	}
	pFile->Write(&tmpvol[0], sizeof(_inputConVol));

	pFile->Write(&_connection[0], sizeof(_connection));
	pFile->Write(&_inputCon[0], sizeof(_inputCon));
	pFile->Write(&_connectionPoint[0], sizeof(_connectionPoint));
	pFile->Write(&_numInputs, sizeof(_numInputs));
	pFile->Write(&_numOutputs, sizeof(_numOutputs));

	pFile->Write(&_panning, sizeof(_panning));
	pFile->Write(&junk[0], 8*sizeof(int)); // SubTrack[]
	pFile->Write(&junk[0], sizeof(int)); // numSubtracks
	pFile->Write(&junk[0], sizeof(int)); // interpol

	pFile->Write(&_outDry, sizeof(_outDry));
	pFile->Write(&_outWet, sizeof(_outWet));

	pFile->Write(&junk[0], sizeof(int)); // distPosThreshold
	pFile->Write(&junk[0], sizeof(int)); // distPosClamp
	pFile->Write(&junk[0], sizeof(int)); // distNegThreshold
	pFile->Write(&junk[0], sizeof(int)); // distNegClamp

	pFile->Write(&junk[0], sizeof(char)); // sinespeed
	pFile->Write(&junk[0], sizeof(char)); // sineglide
	pFile->Write(&junk[0], sizeof(char)); // sinevolume
	pFile->Write(&junk[0], sizeof(char)); // sinelfospeed
	pFile->Write(&junk[0], sizeof(char)); // sinelfoamp

	pFile->Write(&junk[0], sizeof(int)); // delayTimeL
	pFile->Write(&junk[0], sizeof(int)); // delayTimeR
	pFile->Write(&junk[0], sizeof(int)); // delayFeedbackL
	pFile->Write(&junk[0], sizeof(int)); // delayFeedbackR

	pFile->Write(&junk[0], sizeof(int)); // filterCutoff
	pFile->Write(&junk[0], sizeof(int)); // filterResonance
	pFile->Write(&junk[0], sizeof(int)); // filterLfospeed
	pFile->Write(&junk[0], sizeof(int)); // filterLfoamp
	pFile->Write(&junk[0], sizeof(int)); // filterLfophase
	pFile->Write(&junk[0], sizeof(int)); // filterMode

	return true;
}
#endif // ndef _WINAMP_PLUGIN_
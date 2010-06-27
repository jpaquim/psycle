#include "stdafx.h"

#if defined(_WINAMP_PLUGIN_)
//	#include "global.h"
	#include "Plugin.h"
	#include "FileIO.h"
#else
	#include "psycle.h"
	#include "Plugin.h"
	#include "FileIO.h"
	#include "NewMachine.h"
#endif // _WINAMP_PLUGIN_
#include "InputHandler.h"

typedef CMachineInfo * (*GETINFO)(void);
typedef CMachineInterface * (*CREATEMACHINE)(void);

PluginFxCallback Plugin::_callback;

Plugin::Plugin(int index)
{
	_macIndex = index;
	_type = MACH_PLUGIN;
	_mode = MACHMODE_FX;
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
//		MessageBox(NULL,"Could not Get address of Getinfo","",MB_OK);
		FreeLibrary(_dll);
		_dll=NULL;
		return false;
	}

	_pInfo = GetInfo();
	
	if ( _pInfo->Version < MI_VERSION )
	{
//		MessageBox(NULL,"Invalid MI_VERSION","",MB_OK);
		FreeLibrary(_dll);
		_dll=NULL;
		return false;
	}

	_isSynth = (_pInfo->Flags == 3 );
	if (_isSynth )	
	{
		_mode = MACHMODE_GENERATOR;
	}

	strncpy(_psShortName,_pInfo->ShortName,15);
	_psShortName[15]='\0';
	strncpy(_editName, _pInfo->ShortName,31);
	_editName[31]='\0';

	_psAuthor = new char[strlen(_pInfo->Author)+1];
	strcpy(_psAuthor,_pInfo->Author);

	_psName = new char[strlen(_pInfo->Name)+1];
	strcpy(_psName,_pInfo->Name);

	_psDllName = new char[strlen(psFileName)+1];
	strcpy(_psDllName, psFileName);

	if ( GetProcAddress(_dll, "CreateMachine") == NULL)
	{
//		MessageBox(NULL,"createmachine address failed","",MB_OK);
		
	}
	CREATEMACHINE GetInterface =(CREATEMACHINE)GetProcAddress(_dll, "CreateMachine");
	if (GetInterface == NULL)
	{
//		MessageBox(NULL,"Createmachine get interface failed","",MB_OK);
	
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

void Plugin::Work(int numSamples)
{
	if (_mode != MACHMODE_GENERATOR)
	{
		Machine::Work(numSamples);
	}

#if !defined(_WINAMP_PLUGIN_)
	CPUCOST_INIT(cost);
#endif
	if (!_mute) 
	{
		if ((_mode == MACHMODE_GENERATOR) || (!_bypass && !_stopped))
		{
			int ns = numSamples;
			int us = 0;
			while (ns)
			{
				int nextevent;
				nextevent = ns+1;
				if (TWSActive)
				{
					nextevent = TWSSamples;
				}
				else
				{
					nextevent = ns+1;
				}
				for (int i=0; i < Global::_pSong->SONGTRACKS; i++)
				{
					if (TriggerDelay[i]._cmd)
					{
						if (TriggerDelayCounter[i] < nextevent)
						{
							nextevent = TriggerDelayCounter[i];
						}
					}
				}
				if (nextevent > ns)
				{
					if (TWSActive)
					{
						TWSSamples -= ns;
					}
					for (int i=0; i < Global::_pSong->SONGTRACKS; i++)
					{
						// come back to this
						if (TriggerDelay[i]._cmd)
						{
							TriggerDelayCounter[i] -= ns;
						}
					}
					_pInterface->Work(_pSamplesL+us, _pSamplesR+us, ns, Global::_pSong->SONGTRACKS);

					ns = 0;
				}
				else
				{
					if (nextevent)
					{
						ns -= nextevent;
						_pInterface->Work(_pSamplesL+us, _pSamplesR+us, nextevent, Global::_pSong->SONGTRACKS);
						us += nextevent;
					}
					if (TWSActive)
					{
						if (TWSSamples == nextevent)
						{
							int activecount = 0;
							TWSSamples = TWEAK_SLIDE_SAMPLES;
							for (int i = 0; i < MAX_TWS; i++)
							{
								if (TWSDelta[i] != 0)
								{
									TWSCurrent[i] += TWSDelta[i];

									if (((TWSDelta[i] > 0) && (TWSCurrent[i] >= TWSDestination[i]))
										|| ((TWSDelta[i] < 0) && (TWSCurrent[i] <= TWSDestination[i])))
									{
										TWSCurrent[i] = TWSDestination[i];
										TWSDelta[i] = 0;
									}
									else
									{
										activecount++;
									}
									_pInterface->ParameterTweak(TWSInst[i],int(TWSCurrent[i]));
								}
							}
							if (activecount == 0)
							{
								TWSActive = FALSE;
							}
						}
					}
					for (int i=0; i < Global::_pSong->SONGTRACKS; i++)
					{
						// come back to this
						if (TriggerDelay[i]._cmd == 0xfd)
						{
							if (TriggerDelayCounter[i] == nextevent)
							{
								// do event
								_pInterface->SeqTick(i ,TriggerDelay[i]._note, TriggerDelay[i]._inst, 0, 0);
								TriggerDelay[i]._cmd = 0;
							}
							else
							{
								TriggerDelayCounter[i] -= nextevent;
							}
						}
						else if (TriggerDelay[i]._cmd == 0xfb)
						{
							if (TriggerDelayCounter[i] == nextevent)
							{
								// do event
								_pInterface->SeqTick(i ,TriggerDelay[i]._note, TriggerDelay[i]._inst, 0, 0);
								TriggerDelayCounter[i] = (RetriggerRate[i]*Global::_pSong->SamplesPerTick)/256;
							}
							else
							{
								TriggerDelayCounter[i] -= nextevent;
							}
						}
						else if (TriggerDelay[i]._cmd == 0xfa)
						{
							if (TriggerDelayCounter[i] == nextevent)
							{
								// do event
								_pInterface->SeqTick(i ,TriggerDelay[i]._note, TriggerDelay[i]._inst, 0, 0);
								TriggerDelayCounter[i] = (RetriggerRate[i]*Global::_pSong->SamplesPerTick)/256;
								int parameter = TriggerDelay[i]._parameter&0x0f;
								if (parameter < 9)
								{
									RetriggerRate[i]+= 4*parameter;
								}
								else
								{
									RetriggerRate[i]-= 2*(16-parameter);
									if (RetriggerRate[i] < 16)
									{
										RetriggerRate[i] = 16;
									}
								}
							}
							else
							{
								TriggerDelayCounter[i] -= nextevent;
							}
						}
					}
				}
			}
#ifndef _WINAMP_PLUGIN_
			Machine::SetVolumeCounter(numSamples);
			if ( Global::pConfig->autoStopMachines )
			{
				if (_volumeCounter < 8.0f)
				{
					_volumeCounter = 0.0f;
					_volumeDisplay = 0;
					_stopped = true;
				}
				else _stopped = false;
			}
#endif
		}
	}
#ifndef _WINAMP_PLUGIN_
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
//	if (_mode == MACHMODE_GENERATOR) <- effects want command data too please
//	{
		_pInterface->SeqTick(channel ,pData->_note, pData->_inst, pData->_cmd, pData->_parameter);
//	}

	if (pData->_note == cdefTweakM || pData->_note == cdefTweakE)
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
	else if (pData->_note == cdefTweakS)
	{
		if (pData->_inst < _pInfo->numParameters)
		{
			int i;
			if (TWSActive)
			{
				// see if a tweak slide for this parameter is already happening
				for (i = 0; i < MAX_TWS; i++)
				{
					if ((TWSInst[i] == pData->_inst) && (TWSDelta[i] != 0))
					{
						// yes
						break;
					}
				}
				if (i == MAX_TWS)
				{
					// nope, find an empty slot
					for (i = 0; i < MAX_TWS; i++)
					{
						if (TWSDelta[i] == 0)
						{
							break;
						}
					}
				}
			}
			else
			{
				// wipe our array for safety
				for (i = MAX_TWS-1; i > 0; i--)
				{
					TWSDelta[i] = 0;
				}
			}
			if (i < MAX_TWS)
			{
				TWSDestination[i] = float(pData->_cmd<<8)+pData->_parameter;
				float min = float(_pInfo->Parameters[pData->_inst]->MinValue);
				float max = float(_pInfo->Parameters[pData->_inst]->MaxValue);

				TWSDestination[i] += min;
				if (TWSDestination[i] > max)
				{
					TWSDestination[i] = max;
				}
				TWSInst[i] = pData->_inst;
				TWSCurrent[i] = float(_pInterface->Vals[TWSInst[i]]);
				TWSDelta[i] = float((TWSDestination[i]-TWSCurrent[i])*TWEAK_SLIDE_SAMPLES)/Global::_pSong->SamplesPerTick;
				TWSSamples = 0;
				TWSActive = TRUE;
			}
			else
			{
				// we have used all our slots, just send a twk
				int nv = (pData->_cmd<<8)+pData->_parameter;
				int const min = _pInfo->Parameters[pData->_inst]->MinValue;
				int const max = _pInfo->Parameters[pData->_inst]->MaxValue;

				nv += min;
				if (nv > max)
				{
					nv = max;
				}

				_pInterface->ParameterTweak(pData->_inst, nv);
			}
		}
#if !defined(_WINAMP_PLUGIN_)
			Global::_pSong->Tweaker = true;
#endif // ndef _WINAMP_PLUGIN_
	}
}

bool Plugin::Load(RiffFile* pFile)
{
	bool result = true;
	char junk[256];
	memset(&junk, 0, sizeof(junk));

	char sDllName[256];
	int numParameters;


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

		char sPath2[_MAX_PATH];
		CString sPath;
#if defined(_WINAMP_PLUGIN_)
		sPath = Global::pConfig->GetPluginDir();

		if ( FindFileinDir(sDllName,sPath) )
		{
			strcpy(sPath2,sPath);
			if (!Instance(sPath2)) 
			{
				result=false;
			}
		}
		else 
		{
			result = false;
		}
#else
		if ( !CNewMachine::dllNames.Lookup(sDllName,sPath) ) 
		{
//			Check Compatibility Table.
//			Probably could be done with the dllNames lockup.
//
//			GetCompatible(sDllName,sPath2) // If no one found, it will return a null string.
			strcpy(sPath2,sDllName);
		}
		else 
		{ 
			strcpy(sPath2,sPath); 
		}
		
		if ( !CNewMachine::TestFilename(sPath2) ) 
		{
			result = false;
		}
		else 
		{
			if (!Instance(sPath2))
			{
				char sError[_MAX_PATH];
				sprintf(sError,"Missing or corrupted native Plug-in \"%s\" - replacing with Dummy.",sDllName);
				::MessageBox(NULL,sError, "Error", MB_OK);
				result = false;
			}
		}
#endif // _WINAMP_PLUGIN_
	Init();
	pFile->Read(&_editName,16);
	_editName[15] = 0;

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
		delete Vals;
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
#if defined (_WINAMP_PLUGIN_)
	pFile->Skip(96) ; // sizeof(CPoint) = 8.
#else
	pFile->Read(&_connectionPoint[0], sizeof(_connectionPoint));
#endif
	pFile->Read(&_numInputs, sizeof(_numInputs));
	pFile->Read(&_numOutputs, sizeof(_numOutputs));

	pFile->Read(&_panning, sizeof(_panning));
	Machine::SetPan(_panning);
	pFile->Read(&junk[0], 8*sizeof(int)); // SubTrack[]
	pFile->Read(&junk[0], sizeof(int)); // numSubtracks
	pFile->Read(&junk[0], sizeof(int)); // interpol

	pFile->Read(&junk[0], sizeof(int)); // outwet
	pFile->Read(&junk[0], sizeof(int)); // outdry

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
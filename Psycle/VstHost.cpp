#include "stdafx.h"

#if defined(_WINAMP_PLUGIN_)
	#include "global.h"
#endif // _WINAMP_PLUGIN_

#include "song.h"
#include "Configuration.h"
#include "FileIO.h"
#include "VstHost.h"
#include "Player.h"
#include "Song.h"

VstTimeInfo VSTPlugin::_timeInfo;

////////////////////
//
/////////////////////
VSTPlugin::VSTPlugin()
{
	_volumeMultiplier = 1.0f;
	memset(junk,0,STREAM_SIZE*sizeof(float));
	for (int i=0;i<MAX_INOUTS;i++)
	{
		inputs[i]=junk;
		outputs[i]=junk;
	}
	outputs[0]=_pSamplesL;
	outputs[1]=_pSamplesR;

	queue_size=0;
	wantidle = false;
	_sDllName = NULL;
	_pEffect=NULL;
	h_dll=NULL;
	instantiated=false;		// Constructin' with no instance

	requiresProcess=false;
	requiresRepl=false;
}

VSTPlugin::~VSTPlugin()
{
	Free();				// Call free
	delete _sDllName;	// if _sDllName = NULL , the operation does nothing -> it's safe.

}

int VSTPlugin::Instance(char *dllname,bool overwriteName)
{
	h_dll=LoadLibrary(dllname);

	if(h_dll==NULL)	return VSTINSTANCE_ERR_NO_VALID_FILE;


	PVSTMAIN main = (PVSTMAIN)GetProcAddress(h_dll,"main");
	if(!main)
	{	
		FreeLibrary(h_dll);
		_pEffect=NULL;
		instantiated=false;
		return VSTINSTANCE_ERR_NO_VST_PLUGIN;
	}

	//This calls the "main" function and receives the pointer to the AEffect structure.
	_pEffect=main((audioMasterCallback)&Master);
	
	if(!_pEffect || _pEffect->magic!=kEffectMagic)
	{
		TRACE("VST plugin : Instance query rejected by 0x%.8X\n",(int)_pEffect);
		FreeLibrary(h_dll);
		_pEffect=NULL;
		instantiated=false;
		return VSTINSTANCE_ERR_REJECTED;
	}

	TRACE("VST plugin : Instanced at (Effect*): %.8X\n",(int)_pEffect);

	//init plugin (probably a call to "Init()" function should be done here)
	_pEffect->user = this;
	Dispatch( effOpen        ,  0, 0, NULL, 0.0f);
	Dispatch( effSetProgram  ,  0, 0, NULL, 0.0f);
	Dispatch( effMainsChanged,  0, 1, NULL, 0.0f);

#if defined(_WINAMP_PLUGIN_)
	Dispatch( effSetSampleRate, 0, 0, NULL, (float)Global::pConfig->_samplesPerSec);
#else
	Dispatch( effSetSampleRate, 0, 0, NULL, (float)Global::pConfig->_pOutputDriver->_samplesPerSec);
#endif // _WINAMP_PLUGIN_

	Dispatch( effSetBlockSize,  0, STREAM_SIZE, NULL, 0.0f);
	if (!Dispatch( effGetEffectName, 0, 0, &_sProductName, 0.0f))
	{
		CString str1(dllname);
		CString str2 = str1.Mid(str1.ReverseFind('\\')+1);
		str1 = str2.Left(str2.Find('.'));
		strcpy(_sProductName,str1);

	}
	if ( overwriteName ) memcpy(_editName,_sProductName,15);
	_editName[15]='\0';

// Compatibility hacks
	if ( strcmp(_sProductName,"sc-101") == 0 ) requiresRepl=true;

//

	if (!_pEffect->dispatcher(_pEffect, effGetVendorString, 0, 0, &_sVendorName, 0.0f))
	{
		strcpy(_sVendorName, "Unknown vendor");
	}
	_version = _pEffect->dispatcher(_pEffect,effGetVendorVersion , 0 , 0 , NULL, 0.0f);

	_isSynth = (_pEffect->flags & effFlagsIsSynth)?true:false;

	if ( _sDllName != NULL ) delete _sDllName;
	_sDllName = new char[strlen(dllname)+1];
	sprintf(_sDllName,dllname);
	TRACE("VST plugin dll filename : %s\n",_sDllName);

	instantiated=true;

	TRACE("VST plugin : Inputs = %d, Outputs = %d\n",_pEffect->numInputs,_pEffect->numOutputs);

	return VSTINSTANCE_NO_ERROR;
}
/*
void VSTPlugin::Create(VSTPlugin *plug)  // Old way of loading VST's from songs
{
	h_dll=plug->h_dll;
	_pEffect=plug->_pEffect;
	_pEffect->user=this;
	Dispatch( effMainsChanged,  0, 1, NULL, 0.0f);
//	strcpy(_editName,plug->_editName); On current implementation, this replaces the right one. 
	strcpy(_sProductName,plug->_sProductName);
	strcpy(_sVendorName,plug->_sVendorName);
	
	_sDllName = new char[strlen(plug->_sDllName)+1];
	strcpy(_sDllName,plug->_sDllName);

	_isSynth=plug->_isSynth;
	_version = plug->_version;

	plug->instantiated=false;	// We are "stoling" the plugin from the "plug" object so this
								// is just a "trick" so that when destructing the "plug", it
								// doesn't unload the Dll.
	instantiated=true;
	requiresProcess=plug->requiresProcess;
	requiresRepl=plug->requiresRepl;
}
*/
void VSTPlugin::Free() // Called also in destruction
{
	if(instantiated)
	{
		instantiated=false;
		TRACE("VST plugin : Free query 0x%.8X\n",(int)_pEffect);
		_pEffect->user = NULL;
		Dispatch( effMainsChanged, 0, 0, NULL, 0.0f);
		Dispatch( effClose,        0, 0, NULL, 0.0f);
		//delete _pEffect; // <-  Should check for the necessity of this command.
		_pEffect=NULL;
		FreeLibrary(h_dll);
	}
}

void VSTPlugin::Init(void) // Currently this function is unused!!! Some changes in the creation and
{							// loading from song need to be done.
	Machine::Init();

	for (int i=0; i<MAX_CONNECTIONS; i++)
	{
		_inputConVol[i] = 0.000030517578125f; // 1/32767 -> VST Plugins use the range -1.0..1.0
	}

	Dispatch(effOpen        ,  0, 0, NULL, 0.f);
	Dispatch(effSetProgram  ,  0, 0, NULL, 0.f);
	Dispatch(effMainsChanged,  0, 1, NULL, 0.f);

#if defined(_WINAMP_PLUGIN_)
	Dispatch(effSetSampleRate, 0, 0, 0, (float)Global::pConfig->_samplesPerSec);
#else
	Dispatch(effSetSampleRate, 0, 0, 0, (float)Global::pConfig->_pOutputDriver->_samplesPerSec);
#endif // _WINAMP_PLUGIN_

	Dispatch(effSetBlockSize,  0, STREAM_SIZE, NULL, 0.f);
}

bool VSTPlugin::Load(RiffFile* pFile)
{
	char junkdata[256];
	memset(&junkdata, 0, sizeof(junkdata));

	Machine::Init();
/*  This part is read when loading the song to detect the machine type.
	Might change in the new fileformat (i.e. Moving this to Machine::Load(RiffFile* pFile).

	pFile->Read(&_x, sizeof(_x));
	pFile->Read(&_y, sizeof(_y));
	pFile->Read(&_type, sizeof(_type));
*/

/*  Enable this when changing the File Format.

	CString sPath;
	char sPath2[_MAX_PATH];
	char dllName[128];
	pFile->Read(dllName,sizeof(dllName));
	_strlwr(dllName);

	if ( CNewMachine::dllNames.Lookup(dllName,sPath) ) 
	{
		strcpy(sPath2,sPath);
		if ( Instance(sPath2) != VSTINSTANCE_NOERROR )
		{
			CString error;
			sprintf(error,"Error. '%s' is Missing or Corrupted,tempName);
			MessageBox(error,"VST Plugin Loading Error",MB_OK | MB_ICONERROR);
		}
	}
	else
	{
		CString error;
		sprintf(error,"Error. '%s' is Missing or Corrupted,tempName);
		MessageBox(error,"VST Plugin Loading Error",MB_OK | MB_ICONERROR);
	}

	Init();
	pFile->Read(&_editName, sizeof(_editName));

	int num;
	pFile->Read(&num,sizeof(int));

	if ( !instantiated ) 
	{
		for (int p=0;p<num;p++ ) pFile->Read(&junkdata,sizeof(float));
		pFile->Read(&junkdata,sizeof(int));

		if(_pEffect->flags & effFlagsProgramChunks)
		{
			long chunk_size;
			pFile->Read(&chunk_size,sizeof(long));
			
			// Read chunk
			char *chunk=new char[chunk_size];
			pFile->Read(chunk,chunk_size);
			delete chunk;
		}
	}
	else 
	{
		float value;
		for(int p=0;p<num;p++)
		{
			pFile->Write(&value,sizeof(float));
			_pEffect->setParameter(_pEffect,p,value);
		}
		int cprog;
		pFile->Read(&cprog,sizeof(int));
		Dispatch(effSetProgram,0,cprog,NULL,0.0f);

		if(_pEffect->flags & effFlagsProgramChunks)
		{
			long chunk_size;
			pFile->Read(&chunk_size,sizeof(long));
			
			// Read chunk
			char *chunk=new char[chunk_size];	
			pFile->Read(chunk,chunk_size);
			Dispatch(effSetChunk,0,chunk_size, chunk ,0.0f);
			delete chunk;
		}

	}

*/

/**/	pFile->Read(&_editName, sizeof(_editName));	//Remove when changing the fileformat.
	_editName[15]='\0';
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
	pFile->Read(&junkdata[0], 8*sizeof(int)); // SubTrack[]
	pFile->Read(&junkdata[0], sizeof(int)); // numSubtracks
	pFile->Read(&junkdata[0], sizeof(int)); // interpol

	pFile->Read(&_outDry, sizeof(_outDry));
	pFile->Read(&_outWet, sizeof(_outWet));

	pFile->Read(&junkdata[0], sizeof(int)); // distPosThreshold
	pFile->Read(&junkdata[0], sizeof(int)); // distPosClamp
	pFile->Read(&junkdata[0], sizeof(int)); // distNegThreshold
	pFile->Read(&junkdata[0], sizeof(int)); // distNegClamp

	pFile->Read(&junkdata[0], sizeof(char)); // sinespeed
	pFile->Read(&junkdata[0], sizeof(char)); // sineglide
	pFile->Read(&junkdata[0], sizeof(char)); // sinevolume
	pFile->Read(&junkdata[0], sizeof(char)); // sinelfospeed
	pFile->Read(&junkdata[0], sizeof(char)); // sinelfoamp

	pFile->Read(&junkdata[0], sizeof(int)); // delayTimeL
	pFile->Read(&junkdata[0], sizeof(int)); // delayTimeR
	pFile->Read(&junkdata[0], sizeof(int)); // delayFeedbackL
	pFile->Read(&junkdata[0], sizeof(int)); // delayFeedbackR

	pFile->Read(&junkdata[0], sizeof(int)); // filterCutoff
	pFile->Read(&junkdata[0], sizeof(int)); // filterResonance
	pFile->Read(&junkdata[0], sizeof(int)); // filterLfospeed
	pFile->Read(&junkdata[0], sizeof(int)); // filterLfoamp
	pFile->Read(&junkdata[0], sizeof(int)); // filterLfophase
	pFile->Read(&junkdata[0], sizeof(int)); // filterMode

		bool old;
		pFile->Read(&old, sizeof(old)); // old format
		pFile->Read(&_instance, sizeof(_instance)); // ovst.instance
		if ( old )
		{
			char mch;
			pFile->Read(&mch,sizeof(mch));
			_program=0;
		}
		else
		{
			pFile->Read(&_program, sizeof(_program));
		}

	return true;
}
#if !defined(_WINAMP_PLUGIN_)
bool VSTPlugin::Save(RiffFile* pFile)
{
	char junkdata[256];
	memset(&junkdata, 0, sizeof(junkdata));

/*  This part is read when loading the song to detect the machine type.
	Might change in the new fileformat (i.e. Moving this to Machine::Save(RiffFile* pFile).*/

/**/	pFile->Write(&_x, sizeof(_x));
/**/	pFile->Write(&_y, sizeof(_y));
/**/	pFile->Write(&_type, sizeof(_type));

	
/*  Enable this when changing the File Format.

	CString str = sDllName;
	char str2[128];
	strcpy(str2,str.Mid(str.ReverseFind('\\')+1));// if not found, -1+1 = 0 -> Starting letter
	pFile->Write(&str2,sizeof(str2));
*/
	pFile->Write(&_editName, sizeof(_editName));

/*	  Enable this when changing the File Format.
	
	const int num = _pEffect->numParams;
	pFile->Write(&num,sizeof(int));
	for(int p=0;p<num;p++)
	{
		const float value=_pEffect->getParameter(_pEffect,p);
		pFile->Write(&value,sizeof(float));
	}

	const int cprog=Dispatch(effGetProgram,0,0,NULL,0.0f)
	pFile->Write(&cprog,sizeof(int));

	// Detect & Write additional VST cHunk data
	if(_pEffect->flags & effFlagsProgramChunks)
	{
		char *chunk=NULL;
		long chunk_size=Dispatch( effGetChunk,0,0, &chunk ,0.0f);
		pFile->Write(&chunk_size,sizeof(long));
		pFile->Write(chunk,chunk_size);
	}
*/
	pFile->Write(&_inputMachines[0], sizeof(_inputMachines));
	pFile->Write(&_outputMachines[0], sizeof(_outputMachines));

	float tmpvol[MAX_CONNECTIONS];
	memcpy(tmpvol,_inputConVol,MAX_CONNECTIONS*sizeof(float));
	for (int i=0; i<MAX_CONNECTIONS;i++) // Just a conversion to the new Values used.
	{
		if (_inputCon[i])
		{
			MachineType type =Global::_pSong->_pMachines[_inputMachines[i]]->_type;

			if ( type != MACH_VST && type != MACH_VSTFX ) tmpvol[i]*=32768;
		}
	}
	pFile->Write(&tmpvol[0], sizeof(_inputConVol));

	pFile->Write(&_connection[0], sizeof(_connection));
	pFile->Write(&_inputCon[0], sizeof(_inputCon));
	pFile->Write(&_connectionPoint[0], sizeof(_connectionPoint));
	pFile->Write(&_numInputs, sizeof(_numInputs));
	pFile->Write(&_numOutputs, sizeof(_numOutputs));

	pFile->Write(&_panning, sizeof(_panning));
	pFile->Write(&junkdata[0], 8*sizeof(int)); // SubTrack[]
	pFile->Write(&junkdata[0], sizeof(int)); // numSubtracks
	pFile->Write(&junkdata[0], sizeof(int)); // interpol

	pFile->Write(&_outDry, sizeof(_outDry));
	pFile->Write(&_outWet, sizeof(_outWet));

	pFile->Write(&junkdata[0], sizeof(int)); // distPosThreshold
	pFile->Write(&junkdata[0], sizeof(int)); // distPosClamp
	pFile->Write(&junkdata[0], sizeof(int)); // distNegThreshold
	pFile->Write(&junkdata[0], sizeof(int)); // distNegClamp

	pFile->Write(&junkdata[0], sizeof(char)); // sinespeed
	pFile->Write(&junkdata[0], sizeof(char)); // sineglide
	pFile->Write(&junkdata[0], sizeof(char)); // sinevolume
	pFile->Write(&junkdata[0], sizeof(char)); // sinelfospeed
	pFile->Write(&junkdata[0], sizeof(char)); // sinelfoamp

	pFile->Write(&junkdata[0], sizeof(int)); // delayTimeL
	pFile->Write(&junkdata[0], sizeof(int)); // delayTimeR
	pFile->Write(&junkdata[0], sizeof(int)); // delayFeedbackL
	pFile->Write(&junkdata[0], sizeof(int)); // delayFeedbackR

	pFile->Write(&junkdata[0], sizeof(int)); // filterCutoff
	pFile->Write(&junkdata[0], sizeof(int)); // filterResonance
	pFile->Write(&junkdata[0], sizeof(int)); // filterLfospeed
	pFile->Write(&junkdata[0], sizeof(int)); // filterLfoamp
	pFile->Write(&junkdata[0], sizeof(int)); // filterLfophase
	pFile->Write(&junkdata[0], sizeof(int)); // filterMode

		bool old = false;
		pFile->Write(&old, sizeof(old)); // Is old format?
		pFile->Write(&_instance, sizeof(_instance)); // ovst.instance
		int p = GetCurrentProgram();
		_program = p;
		pFile->Write(&_program, sizeof(_program));

	return true;

}
#endif // ndef _WINAMP_PLUGIN_

bool VSTPlugin::LoadChunk(RiffFile *pFile)
{
	if(_pEffect->flags & effFlagsProgramChunks)
	{
		long chunk_size;
		pFile->Read(&chunk_size,sizeof(long));
		
		char *chunk=NULL;

//		This was to be used as a "safe chunk loader", but some
//		plugins change its size depending on the settings.
//
//		long ck_sz=Dispatch( effGetChunk,0,0, &chunk ,0.0f);
//		while ( chunk_size != ck_sz && !pFile->Eof())
//		{
//			pFile->Skip(chunk_size);
//			pFile->Read(&chunk_size,sizeof(long));
//		}

		// Read chunk
		chunk=new char[chunk_size];	
		pFile->Read(chunk,chunk_size);
		Dispatch(effSetChunk,0, chunk_size, chunk ,0.0f);
		delete chunk;
		return true;
	}
	return false;
}

#if !defined(_WINAMP_PLUGIN_)
bool VSTPlugin::SaveChunk(RiffFile *pFile,bool &isfirst)
{
	// Detect & Write additional VST cHunk data
	if(_pEffect->flags & effFlagsProgramChunks)
	{
		char *chunk=NULL;
		long chunk_size=Dispatch( effGetChunk,0,0, &chunk ,0.0f);
		if ( isfirst )
		{
			pFile->Write(&isfirst,sizeof(bool));
			isfirst=false;
		}
		pFile->Write(&chunk_size,sizeof(long));
		pFile->Write(chunk,chunk_size);
		return true;
	}
	return false;
}
#endif // ndef _WINAMP_PLUGIN_
bool VSTPlugin::DescribeValue(int parameter,char* psTxt)
{
	if(instantiated)
	{
		if(parameter<_pEffect->numParams)
		{
//			char par_name[64];
			char par_display[64];
			char par_label[64];

//			Dispatch(effGetParamName,parameter,0,par_name,0.0f);
			Dispatch(effGetParamDisplay,parameter,0,par_display,0.0f);
			Dispatch(effGetParamLabel,parameter,0,par_label,0.0f);
//			sprintf(psTxt,"%s:%s%s",par_name,par_display,par_label);
			sprintf(psTxt,"%s%s",par_display,par_label);
			return true;
		}
		else	sprintf(psTxt,"NumParams Exeeded");
	}
	else		sprintf(psTxt,"Not loaded");

	return false;
}

bool VSTPlugin::SetParameter(int parameter, float value)
{
	if(instantiated)
	{
		if(parameter<_pEffect->numParams)
		{
			_pEffect->setParameter(_pEffect,parameter,value);
			return true;
		}
	}

	return false;
}

bool VSTPlugin::SetParameter(int parameter, int value)
{
	return SetParameter(parameter,value/65535.0f);
}

int VSTPlugin::GetCurrentProgram()
{
	if(instantiated)
		return Dispatch(effGetProgram,0,0,NULL,0.0f);
	else
		return 0;
}

void VSTPlugin::SetCurrentProgram(int prg)
{
	if(instantiated)
		Dispatch(effSetProgram,0,prg,NULL,0.0f);
}

bool VSTPlugin::AddMIDI(unsigned char data0,unsigned char data1,unsigned char data2)
{
	if (instantiated)
	{
		VstMidiEvent* pevent=&midievent[queue_size];

		pevent->type = kVstMidiType;
		pevent->byteSize = 24;
		pevent->deltaFrames = 0;
		pevent->flags = 0;
		pevent->detune = 0;
		pevent->noteLength = 0;
		pevent->noteOffset = 0;
		pevent->reserved1 = 0;
		pevent->reserved2 = 0;
		pevent->noteOffVelocity = 0;
		pevent->midiData[0] = data0;
		pevent->midiData[1] = data1;
		pevent->midiData[2] = data2;
		pevent->midiData[3] = 0;

		if ( queue_size < MAX_EVENTS ) queue_size++;

		return true;
	}
	else return false;
}


void VSTPlugin::SendMidi()
{
	if(/*instantiated &&*/ queue_size>0)
	{
		// Prepare MIDI events and free queue dispatching all events
		events.numEvents = queue_size;
		events.reserved  = 0;
		for(int q=0;q<queue_size;q++) events.events[q] = (VstEvent*)&midievent[q];

		//Finally Send the events.

		Dispatch(effProcessEvents, 0, 0, &events, 0.0f);
		queue_size=0;
	}
}

// Host callback dispatcher
long VSTPlugin::Master(AEffect *effect, long opcode, long index, long value, void *ptr, float opt)
{
	TRACE("VST plugin call to host dispatcher: Eff: 0x%.8X, Opcode = %d, Index = %d, Value = %d, PTR = %.8X, OPT = %.3f\n",(int)effect, opcode,index,value,(int)ptr,opt);

	// Support opcodes
	switch(opcode)
	{
	case audioMasterAutomate:

		return 0;		// index, value, returns 0
		
	case audioMasterVersion:			return 7;		// vst version, currently 7 (0 for older)
		
	case audioMasterCurrentId:			return 'AASH';	// returns the unique id of a plug that's currently loading

	case audioMasterIdle:
		effect->dispatcher(effect, effEditIdle, 0, 0, NULL, 0.0f);
		return 0;		// call application idle routine (this will call effEditIdle for all open editors too) 
		
	case audioMasterPinConnected:
		if (value == 0) //input
		{
			if ( index < 2) return 0;
			else return 1;
		}
		else //output
		{
			if ( index < 2) return 0;
			else return 1;
		}
		return 0;	// inquire if an input or output is beeing connected;

	case audioMasterWantMidi:			return 0;

	case audioMasterProcessEvents:		return 0; 	// Support of vst events to host is not available

	case audioMasterGetTime:
		memset(&_timeInfo, 0, sizeof(_timeInfo));
		_timeInfo.samplePos = 0;

#if defined(_WINAMP_PLUGIN_)
		_timeInfo.sampleRate = Global::pConfig->_samplesPerSec;
#else
		_timeInfo.sampleRate = Global::pConfig->_pOutputDriver->_samplesPerSec;
#endif // _WINAMP_PLUGIN_

/*		if (value & kVstNanosValid)
		{
			_timeInfo.flags |= kVstNanosValid;
			_timeInfo.nanoSeconds = ::timeGetTime();
		}*/
		if (value & kVstPpqPosValid)
		{
			_timeInfo.flags |= kVstPpqPosValid;

			const float currentline = (float)(Global::pPlayer->_lineCounter%Global::pPlayer->tpb);
			const float linestep = ((float)(Global::_pSong->SamplesPerTick-Global::pPlayer->_ticksRemaining)*4.0f)/(Global::_pSong->SamplesPerTick*Global::pPlayer->tpb);
			_timeInfo.ppqPos = currentline+linestep;
			
		}
		if (value & kVstTempoValid)
		{
			_timeInfo.flags |= kVstTempoValid;
			_timeInfo.tempo = Global::_pSong->BeatsPerMin;
		}
		if (value & kVstTimeSigValid)
		{
			_timeInfo.flags |= 	kVstTimeSigValid;
			_timeInfo.timeSigNumerator = 4;
			_timeInfo.timeSigDenominator = 4;
		}
		return (long)&_timeInfo;
		
	case audioMasterTempoAt:			return Global::_pSong->BeatsPerMin*10000;

	case audioMasterNeedIdle:
		if ( effect->user ) {
			((VSTPlugin*)effect->user)->wantidle = true;
		}
		return 1;
#if defined(_WINAMP_PLUGIN_)
	case audioMasterGetSampleRate:		return Global::pConfig->_samplesPerSec;	
#else
	case audioMasterGetSampleRate:		return Global::pConfig->_pOutputDriver->_samplesPerSec;	
#endif // _WINAMP_PLUGIN_
	case audioMasterGetVendorString:	// Just fooling version string
		strcpy((char*)ptr,"Steinberg");
		return 0;
	
	case audioMasterGetVendorVersion:	return 5000;	// HOST version 5000

	case audioMasterGetProductString:	// Just fooling product string
		strcpy((char*)ptr,"Cubase VST");
		return 0;

	case audioMasterVendorSpecific:		return 0;

	case audioMasterGetLanguage:		return kVstLangEnglish;
	
	case audioMasterUpdateDisplay:
		effect->dispatcher(effect, effEditIdle, 0, 0, NULL, 0.0f);
		return 0;

	case 	audioMasterSizeWindow:
//		((VSTPlugin*)effect->user)->
		
		break;
	case 	audioMasterGetParameterQuantization:	
		return NUMTICKS+1; // because its from 0 to NUMTICKS
		
	case 	audioMasterSetTime:						TRACE("VST master dispatcher: Set Time\n");break;
	case 	audioMasterGetNumAutomatableParameters:	TRACE("VST master dispatcher: GetNumAutPar\n");break;
//	case 	audioMasterGetParameterQuantization:	TRACE("VST master dispatcher: ParamQuant\n");break;
	case 	audioMasterIOChanged:					TRACE("VST master dispatcher: IOchanged\n");break;
//	case 	audioMasterSizeWindow:					TRACE("VST master dispatcher: Size Window\n");break;
	case 	audioMasterGetBlockSize:				TRACE("VST master dispatcher: GetBlockSize\n");break;
	case 	audioMasterGetInputLatency:				TRACE("VST master dispatcher: GetInLatency\n");break;
	case 	audioMasterGetOutputLatency:			TRACE("VST master dispatcher: GetOutLatency\n");break;
	case 	audioMasterGetPreviousPlug:				TRACE("VST master dispatcher: PrevPlug\n");break;
	case 	audioMasterGetNextPlug:					TRACE("VST master dispatcher: NextPlug\n");break;
	case 	audioMasterWillReplaceOrAccumulate:		TRACE("VST master dispatcher: WillReplace\n");break;
	case 	audioMasterGetCurrentProcessLevel:		TRACE("VST master dispatcher: GetProcessLevel\n");break;
	case 	audioMasterGetAutomationState:			TRACE("VST master dispatcher: GetAutState\n");break;
	case 	audioMasterOfflineStart:				TRACE("VST master dispatcher: Offlinestart\n");break;
	case 	audioMasterOfflineRead:					TRACE("VST master dispatcher: Offlineread\n");break;
	case 	audioMasterOfflineWrite:				TRACE("VST master dispatcher: Offlinewrite\n");break;
	case 	audioMasterOfflineGetCurrentPass:		TRACE("VST master dispatcher: OfflineGetcurrentpass\n");break;
	case 	audioMasterOfflineGetCurrentMetaPass:	TRACE("VST master dispatcher: OfflineGetCurrentMetapass\n");break;
	case 	audioMasterSetOutputSampleRate:			TRACE("VST master dispatcher: SetOutputsamplerate\n");break;
	case 	audioMasterGetSpeakerArrangement:		TRACE("VST master dispatcher: Getspeaker\n");break;
	case 	audioMasterSetIcon:						TRACE("VST master dispatcher: SetIcon\n");break;
	case 	audioMasterCanDo:						TRACE("VST master dispatcher: Can Do\n");break;
	case 	audioMasterOpenWindow:					TRACE("VST master dispatcher: OpenWindow\n");break;
	case 	audioMasterCloseWindow:					TRACE("VST master dispatcher: CloseWindow\n");break;
	case 	audioMasterGetDirectory:				TRACE("VST master dispatcher: GetDirectory\n");break;
	default: TRACE("VST master dispatcher: undefed: %d\n",opcode)	;break;
	}	

	
	return 0;
}
///////////////////
// VST Instrument
///////////////////
VSTInstrument::VSTInstrument()
{
	_type = MACH_VST;
	_mode = MACHMODE_GENERATOR;
	sprintf(_editName, "Vst2 Instr.");
	_program = 0;
	for (int i=0;i<MAX_TRACKS;i++)
	{
		trackNote[i].note=255; // No Note.
		trackNote[i].midichan=0;
	}
}

void VSTInstrument::Tick(int channel,PatternEntry* pData)
{
	if(instantiated)
	{
		const int note = pData->_note;

		if ( pData->_cmd == 0x10 ) // _OLD_ MIDI Command
		{
			if ( (pData->_inst & 0xF0) == 0x80 || (pData->_inst & 0xF0) == 0x90)
			{
				AddMIDI(pData->_inst,note,pData->_parameter);
			}
			else AddMIDI(pData->_inst,pData->_parameter);
		}
		else if (pData->_note == 123) // Mcm (MIDI CC) Command
		{
			AddMIDI(pData->_inst,pData->_cmd,pData->_parameter);
		}
		else if (note < 120)
		{
			if (pData->_cmd == 0x0C) 
			{
				if ( pData->_inst== 0xFF) AddNoteOn(channel,note,pData->_parameter/2);
				else AddNoteOn(channel,note,pData->_parameter/2,pData->_inst&0x0F);
			}
			else 
			{
				if ( pData->_inst== 0xFF) AddNoteOn(channel,note,127); // should be 100, but previous host used 127
				else AddNoteOn(channel,note,127,pData->_inst&0x0F);
			}
		}
		else if (note == 120) // Note Off.
		{
			if ( pData->_inst== 0xFF) AddNoteOff(channel);
			else AddNoteOff(channel,pData->_inst&0x0F);
		}
		else if ((note == 121) || ( note == 122 )) // Tweak Command
		{
			const float value = ((pData->_cmd*256)+pData->_parameter)/65535.0f;
			SetParameter(pData->_inst, value);
		}
	}
}

bool VSTInstrument::AddNoteOn(unsigned char channel, unsigned char note,unsigned char speed,unsigned char midichannel)
{
	if(instantiated)
	{
		if (trackNote[channel].note != 255) AddNoteOff(channel,trackNote[channel].note,true);

		VstMidiEvent* pevent=&midievent[queue_size];

		pevent->type = kVstMidiType;
		pevent->byteSize = 24;
		pevent->deltaFrames = 0;
		pevent->flags = 0;
		pevent->detune = 0;
		pevent->noteLength = 0;
		pevent->noteOffset = 0;
		pevent->reserved1 = 0;
		pevent->reserved2 = 0;
		pevent->noteOffVelocity = 0;
		pevent->midiData[0] = (char)0x90 | midichannel; // Midi On
		pevent->midiData[1] = note;
		pevent->midiData[2] = speed;
		pevent->midiData[3] = 0;

		if ( queue_size < MAX_EVENTS ) queue_size++;

		VSTinote thisnote;
		thisnote.note = note;
		thisnote.midichan=midichannel;

		trackNote[channel]=thisnote;
		return true;
	}
	else	return false;


}

bool VSTInstrument::AddNoteOff(unsigned char channel,unsigned char midichannel,bool addatStart)
{
	if (instantiated)
	{
		if (trackNote[channel].note != 255)
		{
			VstMidiEvent* pevent;
			if ( addatStart )
			{
				//PATCH: When a new note enters, it adds a note-off for the previous note playing in
				//		 the track (this is ok). But if you have like:  A-4 C-5 and in the next line
				//		 C-5 E-5 , you will only hear E-5. Solution: Move the NoteOffs at the beginning.
				for (int i=MAX_EVENTS-1;i>0;i--)
				{
					midievent[i]=midievent[i-1];
				}
				pevent=&midievent[0];
			}
			else pevent=&midievent[queue_size];

			pevent->type = kVstMidiType;
			pevent->byteSize = 24;
			pevent->deltaFrames = 0;
			pevent->flags = 0;
			pevent->detune = 0;
			pevent->noteLength = 0;
			pevent->noteOffset = 0;
			pevent->reserved1 = 0;
			pevent->reserved2 = 0;
			pevent->noteOffVelocity = 0;
			pevent->midiData[0] = (char)0x80 | trackNote[channel].midichan; //midichannel; // Midi Off
			pevent->midiData[1] = trackNote[channel].note;
			pevent->midiData[2] = 0;
			pevent->midiData[3] = 0;

			if ( queue_size < MAX_EVENTS ) queue_size++;
			VSTinote thisnote;
			thisnote.note = 255;
			thisnote.midichan=0;

			trackNote[channel]=thisnote;

			return true;
		}
		else return false;
	}
	else	return false;
}

void VSTInstrument::Stop()
{
	if (instantiated)
	{
		for (int i=0;i<MAX_TRACKS;i++)
		{
			AddNoteOff(i);
		}
		// Reset all controllers
		for (i=0;i<16;i++)
		{
			AddMIDI(0xb0+i,0x7e);
			AddMIDI(0xb0+i,0x7b);
		}
	}
}


void VSTInstrument::Work(int numSamples)
{

#if defined(_WINAMP_PLUGIN_)

	if (instantiated)
	{
		if ( wantidle ) Dispatch(effIdle, 0, 0, NULL, 0.0f);
		SendMidi();

		if (!requiresRepl ) _pEffect->process(_pEffect,inputs,outputs,numSamples);
		else _pEffect->processReplacing(_pEffect,inputs,outputs,numSamples);

		if ( _pEffect->numOutputs == 1)
		{
			Dsp::Add(outputs[0],outputs[1],numSamples,1);
		}
	}
	_worked = true;

#else
	
	CPUCOST_INIT(cost);
	if (!_mute && instantiated)
	{
//		Dispatch(effSetBlockSize, 0, numSamples, NULL, 0.f); // Should this be done?

		if ( wantidle ) Dispatch(effIdle, 0, 0, NULL, 0.0f);

		SendMidi();

		if (!requiresRepl ) _pEffect->process(_pEffect,inputs,outputs,numSamples);
		else _pEffect->processReplacing(_pEffect,inputs,outputs,numSamples);
		if ( _pEffect->numOutputs == 1)
		{
			Dsp::Add(outputs[0],outputs[1],numSamples,1);
		}

		if ( Global::pConfig->autoStopMachines )
		{
			_volumeCounter = Dsp::F2I(Dsp::GetMaxVSTVolAccurate(_pSamplesL,_pSamplesR,numSamples) * 32768.0f);
			if (_volumeCounter < 8)	{
				_volumeCounter = 0;
				_stopped = true;
			}
			else _stopped = false;
		}
		else 
		{
			int newVolume = Dsp::F2I(Dsp::GetMaxVSTVol(_pSamplesL,_pSamplesR,numSamples)*32768.0f);
			if (newVolume > _volumeCounter)
			{
				_volumeCounter = newVolume;
			}
			_volumeCounter -= numSamples;
			if (_volumeCounter < 0)
			{
				_volumeCounter = 0;
			}
		}
	}

	CPUCOST_CALC(cost, numSamples);
	_cpuCost += cost;
	_worked = true;
#endif // _WINAMP_PLUGIN_
}


///////////////////
// VST Effect
///////////////////

VSTFX::VSTFX()
{
	for (int i=0; i<MAX_CONNECTIONS; i++)
	{
		_inputConVol[i] = 0.000030517578125f; // 1/32767 -> VST Plugins use the range -1.0..1.0
	}
	VSTPlugin::VSTPlugin();
	_type = MACH_VSTFX;
	_mode = MACHMODE_PLUGIN;
	sprintf(_editName, "Vst2 Fx");

	_pOutSamplesL = new float[STREAM_SIZE];
	_pOutSamplesR = new float[STREAM_SIZE];
	inputs[0]=_pSamplesL;
	inputs[1]=_pSamplesR;
	outputs[0]=_pOutSamplesL;
	outputs[1]=_pOutSamplesR;
	_program = 0;
}
VSTFX::~VSTFX()
{
	delete _pOutSamplesL;
	delete _pOutSamplesR;
}
void VSTFX::Tick(int channel,PatternEntry* pData)
{
	if(instantiated)
	{
		if ( pData->_cmd == 0x10 ) // _OLD_ MIDI Command
		{
			if ( (pData->_inst & 0xF0) == 0x80 || (pData->_inst & 0xF0) == 0x90)
			{
				AddMIDI(pData->_inst,pData->_note,pData->_parameter);
			}
			else AddMIDI(pData->_inst,pData->_parameter);
		}
		else if (pData->_note == 123) // Mcm (MIDI CC) Command
		{
			AddMIDI(pData->_inst,pData->_cmd,pData->_parameter);
		}
		else if ((pData->_note == 121) || ( pData->_note == 122 )) // Tweak command
		{
			const float value = ((pData->_cmd*256)+pData->_parameter)/65535.0f;
			SetParameter(pData->_inst, value);
		}
	}
}
void VSTFX::Work(int numSamples)
{
	Machine::Work(numSamples);
#if !defined(_WINAMP_PLUGIN_)
	CPUCOST_INIT(cost);
#endif // ndef _WINAMP_PLUGIN_
	if ((!_mute) && (!_stopped) && (!_bypass))
	{
		if (instantiated)
		{
			if ( wantidle ) Dispatch(effIdle, 0, 0, NULL, 0.0f);
		
			SendMidi();

			Dsp::Undenormalize(_pSamplesL,_pSamplesR,numSamples);
			if ( _pEffect->numInputs == 1)
			{
			// MIX input0 and input1!
			}
			if ((_pEffect->flags & effFlagsCanReplacing) || requiresRepl)
			{
				_pEffect->processReplacing(_pEffect, inputs, outputs, numSamples);
			}
			else
			{
				Dsp::Clear(_pOutSamplesL,numSamples);
				Dsp::Clear(_pOutSamplesR,numSamples);

				_pEffect->process(_pEffect,inputs,outputs,numSamples);
			}
			if ( _pEffect->numOutputs == 1 )
			{
				memcpy(outputs[1],outputs[0],numSamples*sizeof(float));
			}

			float* const tempSamplesL=inputs[0];	// Just an inversion of the pointers
			float* const tempSamplesR=inputs[1];	// so that i don't need to copy the
			_pSamplesL=_pOutSamplesL;				// whole _pOutSamples to _pSamples
			_pSamplesR=_pOutSamplesR;
			 inputs[0]=_pOutSamplesL;
			 inputs[1]=_pOutSamplesR;
			_pOutSamplesL=tempSamplesL;
			_pOutSamplesR=tempSamplesR;
			outputs[0]=tempSamplesL;
			outputs[1]=tempSamplesR;

		}
#if !defined(_WINAMP_PLUGIN_)
		if ( Global::pConfig->autoStopMachines )
		{
			_volumeCounter = Dsp::F2I(Dsp::GetMaxVSTVolAccurate(_pSamplesL,_pSamplesR,numSamples) * 32768.0f);
			if (_volumeCounter < 8)	{
				_volumeCounter = 0;
				_stopped = true;
			}
			else _stopped = false;
		}
		else 
		{
			int newVolume = Dsp::F2I(Dsp::GetMaxVSTVol(_pSamplesL,_pSamplesR,numSamples)*32768.0f);
			if (newVolume > _volumeCounter)
			{
				_volumeCounter = newVolume;
			}
			_volumeCounter -= numSamples;
			if (_volumeCounter < 0)
			{
				_volumeCounter = 0;
			}
		}
#endif // ndef _WINAMP_PLUGIN_
	}
#if !defined(_WINAMP_PLUGIN_)
	CPUCOST_CALC(cost, numSamples);
	_cpuCost += cost;
#endif // ndef _WINAMP_PLUGIN_
	_worked = true;
}

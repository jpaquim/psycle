// PCVstPlug.cpp: implementation of the PCVstPlug class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "PsyClean.h"
#include "PCVstPlug.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

/////////////////////
long PCVstHost::Master(AEffect *effect, long opcode, long index, long value, void *ptr, float opt)
{
	if ( effect->user == NULL ) return 0; // If this happens, we don't know this plugin.

/*	case audioMasterAutomate:	return 0;		// index, value, returns 0
	case audioMasterVersion:			return 7;		// vst version, currently 7 (0 for older)
	case audioMasterCurrentId:			return 'AASH';	// returns the unique id of a plug that's currently loading
	case audioMasterIdle:
		effect->dispatcher(effect, effEditIdle, 0, 0, NULL, 0.0f);
		return 0;		// call application idle routine (this will call effEditIdle for all open editors too) 
		
	case audioMasterPinConnected:	
		return 0;	// inquire if an input or output is beeing connected;
*/
	case audioMasterWantMidi:	((PCVstPlug*)(effect->user))->wantMidi=true;	return 0;

}

PCSignalProcessor* PCVstHost::Create(CString &dllpath,int& error)
{
	HMODULE h_mod=LoadLibrary(dllname);

	if(h_mod==NULL)	
	{
		error = VSTINSTANCE_ERR_NO_VALID_FILE;
		return NULL;
	}


	PVSTMAIN main = (PVSTMAIN)GetProcAddress(h_mod,"main");
	if(!main)
	{	
		FreeLibrary(h_mod);
		error = VSTINSTANCE_ERR_NO_VST_PLUGIN;
		return NULL;
	}

	//This calls the "main" function and receives the pointer to the AEffect structure.
	Aeffect* aeffect=main((audioMasterCallback)&Master);
	
	if(!affect || aEffect->magic!=kEffectMagic)
	{
		TRACE("\"%s\ is not a VST Plugin. (@: %.8X\n)",dllname,(int)aeffect);
		FreeLibrary(h_mod);
		error = VSTINSTANCE_ERR_REJECTED;
		return NULL;
	}
	
	TRACE("VST plugin \"%s\" Created.Getting info... (@: %.8X\n)",dllname,(int)_pEffect);
	error = VSTINSTANCE_NO_ERROR;

	PCSignalProcessor* pvst;
	if ( (aEffect->flags & effFlagsIsSynth) )  // VSTInstrument.
	{
		pvst = new PCVSTi
		((PCVSTi*)pvst)->Create(h_mod,aEffect);
	}
	else
	{
		pvst = new PCVSTFx(h_mod,aEffect);
		((PCVSTFx*)pvst)->Create(h_mod,aEffect);
	}

	return pvst;
}


//////////////////////////////////////////////////////////////////////
// PCVstPlug Class
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

PCVstPlug::PCVstPlug()
{
	h_dll=NULL;
	_pEffect=NULL;
	wantMidi = false;
	initialized = false;

	for (int i=0; i< MAX_EVENTS ; i++ )
	{
		vstmidievent[i].type = kVstMidiType;
		vstmidievent[i].byteSize = 24;
		vstmidievent[i].deltaFrames = 0;
		vstmidievent[i].flags = 0;
		vstmidievent[i].detune = 0;
		vstmidievent[i].noteLength = 0;
		vstmidievent[i].noteOffset = 0;
		vstmidievent[i].reserved1 = 0;
		vstmidievent[i].reserved2 = 0;
		vstmidievent[i].noteOffVelocity = 0;
		vstmidievent[i].midiData[0] = 0;
		vstmidievent[i].midiData[1] = 0;
		vstmidievent[i].midiData[2] = 0;
		vstmidievent[i].midiData[3] = 0;
		
		events.events[i] = (VstEvent*)&vstmidievent[i];
		events.reserved  = 0;
	}
}

PCVstPlug::~PCVstPlug()
{
	Destroy();
}

void PCVstPlug::Create(HMODULE hdll, AEffect *aeffect)
{
	h_dll = hdll;
	_pEffect= aeffect;

	_pEffect->user = this;

	Dispatch( effOpen        ,  0, 0, NULL, 0.f);	// Initialization (like what?
									// None of the examples on the SDK implement this)
	numInputs = _pEffect->numInputs;
	inputs = new soundBuffer[numInputs];

	numOutputs= _pEffect->numOutputs;
	outputs = new soundBuffer[numOutputs];


}

void PCVstPlug::Destroy()
{
	if(_pEffect)
	{
		TRACE("VST plugin : Destroy query 0x%.8X\n",(int)_pEffect);

		if ( initialized )
		{	Dispatch( effMainsChanged, 0, 0, NULL, 0.0f);
			Dispatch( effClose,        0, 0, NULL, 0.0f);
			initialized=false;
		}
		//delete _pEffect;		// This should never be done. It is a member of the AudioEffect Class.
		_pEffect=NULL;
	}
	if ( h_dll )
	{	FreeLibrary(h_dll);
		h_dll=NULL;
	}
}

void PCVstPlug::Init()
{
	Dispatch( effSetSampleRate, 0, 0, NULL, (float)Global::pConfig->_samplesPerSec);
	Dispatch( effSetBlockSize,  0, Global::pConfig->_blocksize, NULL, 0.0f);

	Dispatch( effSetProgram  ,  0, 0, NULL, 0.f);	// Required notification by the VST standard.
	Dispatch( effMainsChanged,  0, 1, NULL, 0.f);	// Tell the plugin that it is On.
	
	initialized=true;
}

void AddMidiEvent(PCMidiEvent& midievent)
{
	if ( queue_size+1 < MAX_EVENTS ) 
	{
		vstmidievent[queue_size].deltaFrames=midievent.deltaFrames;
		vstmidievent[queue_size].detune=midievent.detune;
		vstmidievent[queue_size].noteOffset=midievent.noteOffset;
		memcpy(vstmidievent[queuesize].midiData,midievent.midiData,4);
		queue_size++;
	}
}


bool PCVstPlug::ProcessMidi()
{
	if(initialized)
	{
		events.numEvents = queue_size;
		long v=Dispatch(effProcessEvents, 0, 0, &events, 0.0f);

		queue_size=0;
		return v?true:false;
	}
	return false;
}


//////////////////////////////////////////////////////////////////////
// PCVSTi Class
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

PCVSTi::PCVSTi()
{

}

PCVSTi::~PCVSTi()
{

}

bool PCVSTi::ProcessSoundReplacing(bool )
{
	if ( wantMidi ) wantMidi = ProcessMidi();

	if (_pEffect->flags & effFlagsCanReplacing)
	{
		_pEffect->processReplacing(_pEffect, inputs, outputs, buffer_size);
	}
	else
	{
		Dsp::Clear(_pOutSamplesL,numSamples);
		Dsp::Clear(_pOutSamplesR,numSamples);

		_pEffect->process(_pEffect,inputs,outputs,numSamples);
	}

}

bool PCVSTi::ProcessSoundAdding(sBufferArray inputbuffers, sBufferArray outputbuffers)
{
	if ( wantMidi ) wantMidi = ProcessMidi();

	_pEffect->process(_pEffect,inputs,outputs,numSamples);
}

//////////////////////////////////////////////////////////////////////
// PCVSTFx Class
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

PCVSTFx::PCVSTFx()
{

}

PCVSTFx::~PCVSTFx()
{

}

void PCVSTFx::Create(HMODULE hdll, AEffect *aeffect)
{
	PCVstPlug::Create(hdd,aeffect);

	// Can bypass the working function if no input audio data.
	canBypass =	(_pEffect->flags & effFlagsNoSoundInStop)?true:false;

}

bool PCVSTFx::ProcessSoundReplacing(sBufferArray inputbuffers, sBufferArray outputbuffers)
{
	if ( wantMidi ) wantMidi = ProcessMidi();
}

bool PCVSTFx::ProcessSoundAdding(sBufferArray inputbuffers, sBufferArray outputbuffers)
{

}

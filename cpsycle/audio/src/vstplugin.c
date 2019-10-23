// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "vstplugin.h"
#include "aeffectx.h"
#include <stdlib.h>
#include <excpt.h>
#include <operations.h>

#if !defined(FALSE)
#define FALSE 0
#endif

#if !defined(TRUE)
#define TRUE 1
#endif

#if !defined(BOOL)
#define BOOL int
#endif

typedef CMachineInfo * (*GETINFO)(void);
typedef CMachineInterface * (*CREATEMACHINE)(void);

static void work(VstPlugin* self, BufferContext*);
static int hostevent(VstPlugin* self, int const eventNr, int const val1, float const val2);
static void seqtick(VstPlugin* self, int channel, const PatternEvent* event);
static void sequencerlinetick(VstPlugin* self);
static const CMachineInfo* info(VstPlugin* self);
static void parametertweak(VstPlugin* self, int par, int val);
static int parameterlabel(VstPlugin*, char* txt, int param);
static int parametername(VstPlugin*, char* txt, int param);
static int describevalue(VstPlugin*, char* txt, int param, int value);
static int value(VstPlugin*, int param);
static void setvalue(VstPlugin*, int param, int value);
static unsigned int numparameters(VstPlugin*);
static unsigned int numcols(VstPlugin*);
static const CMachineParameter* parameter(VstPlugin*, unsigned int par);

static void dispose(VstPlugin* self);
static unsigned int numinputs(VstPlugin*);
static unsigned int numoutputs(VstPlugin*);
static const VstInt32 kBlockSize = 512;
static const float kSampleRate = 48000.f;
static const VstInt32 kNumProcessCycles = 5;
static void checkEffectProperties (AEffect* effect);
static void checkEffectProcessing (AEffect* effect);
static int DispatchMachineInfo(AEffect* effect, CMachineInfo* info);
typedef AEffect* (*PluginEntryProc) (audioMasterCallback audioMaster);
static VstIntPtr VSTCALLBACK HostCallback (AEffect* effect, VstInt32 opcode, VstInt32 index, VstIntPtr value, void* ptr, float opt);
static PluginEntryProc getMainEntry(Library* library);

static CMachineParameter const paraVst = 
{ 
	"VST",	
	"VST",									// description
	0,										// MinValue	
	65535,									// MaxValue
	MPF_STATE,								// Flags
	0
};

void vstplugin_init(VstPlugin* self, MachineCallback callback, const char* path)
{	
	PluginEntryProc mainProc;	

	machine_init(&self->machine, callback);
	self->machine.work = work;
	self->machine.hostevent = hostevent;
	self->machine.seqtick = seqtick;
	self->machine.sequencerlinetick = sequencerlinetick;
	self->machine.info = info;
	self->machine.parametertweak = parametertweak;
	self->machine.parameterlabel = parameterlabel;
	self->machine.parametername = parametername;
	self->machine.describevalue = describevalue;
	self->machine.setvalue = setvalue;
	self->machine.value = value;
	self->machine.numparameters = numparameters;
	self->machine.numcols = numcols;
	self->machine.parameter = parameter;
	self->machine.dispose = dispose;
	self->machine.numinputs = numinputs;
	self->machine.numoutputs = numoutputs;
	self->machine.setcallback(self, callback);
	self->info = 0;
	library_init(&self->library);
	library_load(&self->library, path);		
	mainProc = getMainEntry(&self->library);
	if (mainProc) {
		self->effect = mainProc (HostCallback);
		if (self->effect) {
			VstInt32 numInputs;
			VstInt32 numOutputs;
			numInputs = self->effect->numInputs;
			numOutputs = self->effect->numOutputs;
			self->effect->user = self;
			self->effect->dispatcher (self->effect, effOpen, 0, 0, 0, 0);
			self->effect->dispatcher (self->effect, effSetSampleRate, 0, 0, 0, kSampleRate);
			self->effect->dispatcher (self->effect, effSetProcessPrecision, 0, kVstProcessPrecision32, 0, 0);
			self->effect->dispatcher (self->effect, effSetBlockSize, 0, kBlockSize, 0, 0);
			self->effect->dispatcher (self->effect, effMainsChanged, 0, 1, 0, 0);
			self->effect->dispatcher (self->effect, effStartProcess, 0, 0, 0, 0);			
		}
	}		
} 

void dispose(VstPlugin* self)
{	
	if (self->library.module) {
		if (self->effect) {
			self->effect->dispatcher (self->effect, effClose, 0, 0, 0, 0);
			self->effect = 0;			
		}
		library_dispose(&self->library);		
		if (self->info) {
			free((char*)self->info->Author);
			free((char*)self->info->Name);
			free((char*)self->info->ShortName);
			free((char*)self->info->Command);
		}
		self->info = 0;
	}	
	machine_dispose(&self->machine);
}

PluginEntryProc getMainEntry(Library* library)
{
	PluginEntryProc mainProc = 0;	
	mainProc = (PluginEntryProc)library_functionpointer(library, "VSTPluginMain");
	if(!mainProc)
		mainProc = (PluginEntryProc)library_functionpointer(library,"main");	
	return mainProc;
}

int plugin_vst_test(const char* path, CMachineInfo* rv)
{
	int vst = 0;

	if (path && strcmp(path, "") != 0) {
		Library library;
		PluginEntryProc mainEntry;	
		
		library_init(&library);		
		library_load(&library, path);		
		mainEntry = getMainEntry(&library);
		if (mainEntry) {
			AEffect* effect = mainEntry (HostCallback);
			if (effect) {				
				int err;

				err = DispatchMachineInfo(effect, rv);
				if (err == 0) {
					vst = 1;
				}
			}
		}	
		library_dispose(&library);	
	}
	return vst;
}

void work(VstPlugin* self, BufferContext* bc)
{	
	self->effect->processReplacing(self->effect, bc->input->samples,
		bc->output->samples, bc->numsamples);	
}

void seqtick(VstPlugin* self, int channel, const PatternEvent* event)
{
	struct VstMidiEvent e;
	struct VstEvents ve;
	int n;

	n = 1;	
	ve.numEvents = 1;
	ve.reserved = 0;	

	memset(&e, 0, sizeof(struct VstMidiEvent));
	e.type          = kVstMidiType;
	e.byteSize      = sizeof(e);
	e.flags         = kVstMidiEventIsRealtime;
	e.midiData[0]   = (char)(channel + (1 ? 0x90 : 0x80));
	e.midiData[1]   = (char)(event->note);
	e.midiData[2]   = (char)(127);

	ve.events[0] = (VstEvent*)&e;
//	if(auto l = vstMidi.lock()) {
//		vstMidi.events.push_back(e);
//	}
	self->effect->dispatcher(self->effect, effProcessEvents, 0, 0, &ve, 0);	
}

int hostevent(VstPlugin* self, int const eventNr, int const val1, float const val2)
{
	return 0;
}

void sequencerlinetick(VstPlugin* self)
{
	
}

static int FilterException(int code, struct _EXCEPTION_POINTERS *ep) 
{
	return EXCEPTION_EXECUTE_HANDLER;
}

int DispatchMachineInfo(AEffect* effect, CMachineInfo* info)
{
	char effectName[256] = {0};
	char vendorString[256] = {0};
	char productString[256] = {0};
	int err = 0;

	__try {
		effect->dispatcher (effect, effGetEffectName, 0, 0, effectName, 0);
		effect->dispatcher (effect, effGetVendorString, 0, 0, vendorString, 0);
		effect->dispatcher (effect, effGetProductString, 0, 0, productString, 0);
		
		info->Author = _strdup(vendorString);
		info->Command = _strdup(""); // 0; // strdup(pInfo->Command);
		info->Flags = 16; // pInfo->Flags;
		info->Name = _strdup(effectName);
		info->numCols = 6; // pInfo->numCols;
		info->numParameters = 0; //pInfo->numParameters;
		info->ShortName = _strdup(effectName);
		info->APIVersion = 0; //pInfo->Version;
		info->PlugVersion = 0;
	}
	__except(FilterException(GetExceptionCode(), GetExceptionInformation())) {		
		err = GetExceptionCode();
	}
	return err;
}

const CMachineInfo* info(VstPlugin* self)
{	
	if (self->info == 0 && self->effect) {
		self->info = (CMachineInfo*) malloc(sizeof(CMachineInfo));		
		if (DispatchMachineInfo(self->effect, self->info)) {
			free(self->info);
			self->info = 0;
		}
	}
	return self->info;
}

void parametertweak(VstPlugin* self, int par, int val)
{
	self->effect->setParameter(self->effect, par, val / 65535.f);
}

int parameterlabel(VstPlugin* self, char* txt, int param)
{
	txt[0] = '\0';
	self->effect->dispatcher(self->effect, effGetParamLabel, param, 0, txt, 0);
	return *txt != '\0';
}

int parametername(VstPlugin* self, char* txt, int param)
{
	txt[0] = '\0';
	self->effect->dispatcher(self->effect, effGetParamName, param, 0, txt, 0);
	return *txt != '\0';
}

int describevalue(VstPlugin* self, char* txt, int param, int value)
{ 		
	txt[0] = '\0';
	self->effect->dispatcher(self->effect, effGetParamDisplay, param, 0, txt, 0);
	return *txt != '\0';
}

int value(VstPlugin* self, int param)
{
	return (int)(self->effect->getParameter(self->effect, param) * 65535.f);
}

void setvalue(VstPlugin* self, int param, int value)
{

}

VstIntPtr VSTCALLBACK HostCallback (AEffect* effect, VstInt32 opcode, VstInt32 index, VstIntPtr value, void* ptr, float opt)
{
	VstIntPtr result = 0;

	// Filter idle calls...
	BOOL filtered = FALSE;
	if(opcode == audioMasterIdle)
	{
		static BOOL wasIdle = FALSE;
		if(wasIdle)
			filtered = TRUE;
		else
		{
			printf ("(Future idle calls will not be displayed!)\n");
			wasIdle = TRUE;
		}
	}

	if(!filtered)
		printf ("PLUG> HostCallback (opcode %d)\n index = %d, value = %p, ptr = %p, opt = %f\n", opcode, index,
		(void*) (value), ptr, opt);

	switch(opcode)
	{
	case audioMasterVersion :
		result = kVstVersion;
		break;		

	}

	return result;
}

void checkEffectProperties (AEffect* effect)
{	
	char effectName[256] = {0};
	char vendorString[256] = {0};
	char productString[256] = {0};
	VstInt32 progIndex;
	VstInt32 paramIndex;

	printf ("HOST> Gathering properties...\n");

	effect->dispatcher (effect, effGetEffectName, 0, 0, effectName, 0);
	effect->dispatcher (effect, effGetVendorString, 0, 0, vendorString, 0);
	effect->dispatcher (effect, effGetProductString, 0, 0, productString, 0);

	printf ("Name = %s\nVendor = %s\nProduct = %s\n\n", effectName, vendorString, productString);

	printf ("numPrograms = %d\nnumParams = %d\nnumInputs = %d\nnumOutputs = %d\n\n", 
			effect->numPrograms, effect->numParams, effect->numInputs, effect->numOutputs);

	// Iterate programs...
	for(progIndex = 0; progIndex < effect->numPrograms; progIndex++)
	{
		char progName[256] = {0};
		if(!effect->dispatcher (effect, effGetProgramNameIndexed, progIndex, 0, progName, 0))
		{
			effect->dispatcher (effect, effSetProgram, 0, progIndex, 0, 0); // Note: old program not restored here!
			effect->dispatcher (effect, effGetProgramName, 0, 0, progName, 0);
		}
		printf ("Program %03d: %s\n", progIndex, progName);
	}

	printf ("\n");

	// Iterate parameters...
	for(paramIndex = 0; paramIndex < effect->numParams; paramIndex++)
	{
		char paramName[256] = {0};
		char paramLabel[256] = {0};
		char paramDisplay[256] = {0};
		float value;

		effect->dispatcher (effect, effGetParamName, paramIndex, 0, paramName, 0);
		effect->dispatcher (effect, effGetParamLabel, paramIndex, 0, paramLabel, 0);
		effect->dispatcher (effect, effGetParamDisplay, paramIndex, 0, paramDisplay, 0);
		value = effect->getParameter (effect, paramIndex);

		printf ("Param %03d: %s [%s %s] (normalized = %f)\n", paramIndex, paramName, paramDisplay, paramLabel, value);
	}

	printf ("\n");

	// Can-do nonsense...
	{
		VstInt32 canDoIndex;
		static const char* canDos[] =
		{
			"receiveVstEvents",
			"receiveVstMidiEvent",
			"midiProgramNames"
		};

		for(canDoIndex = 0; canDoIndex < sizeof(canDos)/sizeof(canDos[0]); canDoIndex++)
		{
			VstInt32 result;
			printf ("Can do %s... ", canDos[canDoIndex]);
			result = (VstInt32)effect->dispatcher (effect, effCanDo, 0, 0, (void*)canDos[canDoIndex], 0);
			switch(result)
			{
				case 0  : printf ("don't know"); break;
				case 1  : printf ("yes"); break;
				case -1 : printf ("definitely not!"); break;
				default : printf ("?????");
			}
			printf ("\n");
		}
	}

	printf ("\n");
}

//-------------------------------------------------------------------------------------------------------
void checkEffectProcessing (AEffect* effect)
{
	float** inputs = 0;
	float** outputs = 0;
	VstInt32 numInputs = effect->numInputs;
	VstInt32 numOutputs = effect->numOutputs;
	
	if(numInputs > 0)
	{
		VstInt32 i;
		inputs = (float**)malloc(sizeof(float*)*numInputs);
		for(i = 0; i < numInputs; i++)
		{
			inputs[i] = (float*)malloc(sizeof(float)*kBlockSize);
			memset (inputs[i], 0, kBlockSize * sizeof(float));
		}
	}

	if(numOutputs > 0)
	{
		VstInt32 i;
		outputs = (float**)malloc(sizeof(float*)*numOutputs);
		for(i = 0; i < numOutputs; i++)
		{
			outputs[i] = (float*)malloc(sizeof(float)*kBlockSize);
			memset (outputs[i], 0, kBlockSize * sizeof(float));
		}
	}

	printf ("HOST> Resume effect...\n");
	effect->dispatcher (effect, effMainsChanged, 0, 1, 0, 0);

	{
		VstInt32 processCount;
		for(processCount = 0; processCount < kNumProcessCycles; processCount++)
		{
			printf ("HOST> Process Replacing...\n");
			effect->processReplacing (effect, inputs, outputs, kBlockSize);
		}
	}

	printf ("HOST> Suspend effect...\n");
	effect->dispatcher (effect, effMainsChanged, 0, 0, 0, 0);

	if(numInputs > 0)
	{
		VstInt32 i;
		for(i = 0; i < numInputs; i++)
			free(inputs[i]);
		free(inputs);
	}

	if(numOutputs > 0)
	{
		VstInt32 i;
		for(i = 0; i < numOutputs; i++)
			free(outputs[i]);
		free(outputs);
	}
}

unsigned int numinputs(VstPlugin* self)
{
	return self->effect->numInputs;
}

unsigned int numoutputs(VstPlugin* self)
{
	return self->effect->numOutputs;
}

unsigned int numparameters(VstPlugin* self)
{
	return self->effect->numParams;	
}

unsigned int numcols(VstPlugin* self)
{
	return 6;
}

const CMachineParameter* parameter(VstPlugin* self, unsigned int par)
{
	return &paraVst;
}

/*
bool Plugin::LoadSpecificChunk(RiffFile * pFile, int version)
{
	try {
		UINT size;
		unsigned char _program;
		pFile->Read(&size, sizeof size );
		if(size)
		{
			UINT count;
			pFile->Read(&_program, sizeof _program);
			pFile->Read(&count, sizeof count);
			size -= sizeof _program + sizeof count + sizeof(float) * count;
			if(!size)
			{
				BeginSetProgram();
				SetProgram(_program);
				for(UINT i(0) ; i < count ; ++i)
				{
					float temp;
					pFile->Read(&temp, sizeof temp);
					SetParameter(i, temp);
				}
				EndSetProgram();
			}
			else
			{
				BeginSetProgram();
				SetProgram(_program);
				EndSetProgram();
				pFile->Skip(sizeof(float) *count);
				if(ProgramIsChunk())
				{
					char * data(new char[size]);
					pFile->Read(data, size); // Number of parameters
					SetChunk(data,size);
					zapArray(data);
				}
				else
				{
					// there is a data chunk, but this machine does not want one.
					pFile->Skip(size);
					return false;
				}
			}
		}
		return true;
	}
	catch(...){return false;}
}
*/

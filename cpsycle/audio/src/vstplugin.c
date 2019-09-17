// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net
#include "vstplugin.h"
#include "aeffectx.h"

typedef CMachineInfo * (*GETINFO)(void);
typedef CMachineInterface * (*CREATEMACHINE)(void);

static void work(VstPlugin* self, BufferContext*);
static int hostevent(VstPlugin* self, int const eventNr, int const val1, float const val2);
static void seqtick(VstPlugin* self, int channel, const PatternEvent* event);
static void sequencertick(VstPlugin* self);
static const CMachineInfo* info(VstPlugin* self);
static void parametertweak(VstPlugin* self, int par, int val);
static int describevalue(VstPlugin*, char* txt, int const param, int const value);
static int value(VstPlugin*, int const param);
static void setvalue(VstPlugin*, int const param, int const value);
static void dispose(VstPlugin* self);
static int mode(VstPlugin* self);
static const VstInt32 kBlockSize = 512;
static const float kSampleRate = 48000.f;
static const VstInt32 kNumProcessCycles = 5;
static void checkEffectProperties (AEffect* effect);
static void checkEffectProcessing (AEffect* effect);
static void DispatchMachineInfo(AEffect* effect, CMachineInfo* info);
typedef AEffect* (*PluginEntryProc) (audioMasterCallback audioMaster);
static VstIntPtr VSTCALLBACK HostCallback (AEffect* effect, VstInt32 opcode, VstInt32 index, VstIntPtr value, void* ptr, float opt);
static PluginEntryProc getMainEntry (HINSTANCE module);


void vstplugin_init(VstPlugin* self, const char* path)
{
	int err;
	PluginEntryProc mainProc;	

	machine_init(&self->machine);
	self->machine.work = work;
	self->machine.hostevent = hostevent;
	self->machine.seqtick = seqtick;
	self->machine.sequencertick = sequencertick;
	self->machine.info = info;
	self->machine.parametertweak = parametertweak;
	self->machine.describevalue = describevalue;
	self->machine.setvalue = setvalue;
	self->machine.value = value;
	self->machine.dispose = dispose;
	self->machine.mode = mode;
	self->info = 0;
	self->module = LoadLibrary(path);
	err = GetLastError();
	mainProc = getMainEntry(self->module);
	if (mainProc) {

// dispatcher(int32_t opcode, int32_t index = 0, intptr_t value = 0, void *ptr = nullptr, float opt = 0.0f) const {
// return aEffect->dispatcher(aEffect, opcode, index, value, ptr, opt);


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
	if (self->module) {
		if (self->effect) {
			self->effect->dispatcher (self->effect, effClose, 0, 0, 0, 0);
			self->effect = 0;			
		}
		FreeLibrary(self->module);		
		self->module = 0;
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

PluginEntryProc getMainEntry (HINSTANCE module)
{
	PluginEntryProc mainProc = 0;	
	mainProc = (PluginEntryProc)GetProcAddress ((HMODULE)module, "VSTPluginMain");
	if(!mainProc)
		mainProc = (PluginEntryProc)GetProcAddress ((HMODULE)module, "main");	
	return mainProc;
}


CMachineInfo* plugin_vst_test(const char* path)
{		
	CMachineInfo* rv;
	HINSTANCE dll;	
	PluginEntryProc mainEntry;
	int err;

	rv = 0;
	dll = LoadLibrary(path);
	err = GetLastError();	
	mainEntry = getMainEntry(dll);
	if (mainEntry) {
		AEffect* effect = mainEntry (HostCallback);
		if (effect) {			
			rv = (CMachineInfo*) malloc(sizeof(CMachineInfo));
			DispatchMachineInfo(effect, rv);									
		}
	}	
	FreeLibrary(dll);
	return rv;
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

void sequencertick(VstPlugin* self)
{
	
}

void DispatchMachineInfo(AEffect* effect, CMachineInfo* info)
{
	char effectName[256] = {0};
	char vendorString[256] = {0};
	char productString[256] = {0};

	effect->dispatcher (effect, effGetEffectName, 0, 0, effectName, 0);
	effect->dispatcher (effect, effGetVendorString, 0, 0, vendorString, 0);
	effect->dispatcher (effect, effGetProductString, 0, 0, productString, 0);
	
	info->Author = strdup(vendorString);
	info->Command = strdup(""); // 0; // strdup(pInfo->Command);
	info->Flags = 16; // pInfo->Flags;
	info->Name = strdup(effectName);
	info->numCols = 6; // pInfo->numCols;
	info->numParameters = 0; //pInfo->numParameters;
	info->ShortName = strdup(effectName);
	info->APIVersion = 0; //pInfo->Version;
	info->PlugVersion = 0;
}

const CMachineInfo* info(VstPlugin* self)
{	
	if (self->info == 0 && self->effect) {
		self->info = (CMachineInfo*) malloc(sizeof(CMachineInfo));	
		DispatchMachineInfo(self->effect, self->info);
	}
	return self->info;
}

void parametertweak(VstPlugin* self, int par, int val)
{

}

int describevalue(VstPlugin* self, char* txt, int const param, int const value)
{ 
	return 0;
}

int value(VstPlugin* self, int const param)
{
	return 0;
}

void setvalue(VstPlugin* self, int const param, int const value)
{

}

int mode(VstPlugin* self)
{
	if (self->machine.info(self) && (self->machine.info(self)->Flags == 3 )) {
		return MACHMODE_GENERATOR;
	} else {
		return MACHMODE_FX;
	}	
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





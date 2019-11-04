// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "vstplugin.h"
#include "aeffectx.h"
#include <stdlib.h>
#include <excpt.h>
#include <operations.h>
#include "pattern.h"

static int mode(VstPlugin*);
static void work(VstPlugin* self, BufferContext*);
static int hostevent(VstPlugin* self, int const eventNr, int const val1, float const val2);
static const MachineInfo* info(VstPlugin* self);
static void parametertweak(VstPlugin* self, int par, int val);
static int parameterlabel(VstPlugin*, char* txt, int param);
static int parametername(VstPlugin*, char* txt, int param);
static int describevalue(VstPlugin*, char* txt, int param, int value);
static int parametervalue(VstPlugin*, int param);
static int parametertype(VstPlugin*, int param);
static void parameterrange(VstPlugin*, int param, int* minval, int* maxval);
static unsigned int numparameters(VstPlugin*);
static unsigned int numparametercols(VstPlugin*);
static void dispose(VstPlugin* self);
static unsigned int numinputs(VstPlugin*);
static unsigned int numoutputs(VstPlugin*);
static const VstInt32 kBlockSize = 512;
static const float kSampleRate = 48000.f;
static const VstInt32 kNumProcessCycles = 5;
static void checkEffectProperties (AEffect* effect);
static void checkEffectProcessing (AEffect* effect);
static int machineinfo(AEffect* effect, MachineInfo* info, const char* path, int shellidx);
typedef AEffect* (*PluginEntryProc)(audioMasterCallback audioMaster);
static VstIntPtr VSTCALLBACK hostcallback(AEffect* effect, VstInt32 opcode, VstInt32 index, VstIntPtr value, void* ptr, float opt);
static PluginEntryProc getmainentry(Library* library);
static int haseditor(VstPlugin*);
static void seteditorhandle(VstPlugin*, void* handle);
static void editorsize(VstPlugin*, int* width, int* height);
static void editoridle(VstPlugin*);
static void processevents(VstPlugin*, BufferContext*);
struct VstMidiEvent* allocinitmidievent(VstPlugin*, const PatternEntry*);

void vstplugin_init(VstPlugin* self, MachineCallback callback, const char* path)
{	
	PluginEntryProc mainproc;	

	machine_init(&self->machine, callback);
	self->machine.mode = mode;
	self->machine.work = work;
	self->machine.hostevent = hostevent;	
	self->machine.info = info;
	self->machine.parametertype = parametertype;
	self->machine.parameterrange = parameterrange;
	self->machine.parametertweak = parametertweak;
	self->machine.parameterlabel = parameterlabel;
	self->machine.parametername = parametername;
	self->machine.describevalue = describevalue;	
	self->machine.parametervalue = parametervalue;
	self->machine.numparameters = numparameters;
	self->machine.numparametercols = numparametercols;	
	self->machine.dispose = dispose;
	self->machine.numinputs = numinputs;
	self->machine.numoutputs = numoutputs;
	self->machine.setcallback(self, callback);
	self->machine.haseditor = haseditor;
	self->machine.seteditorhandle = seteditorhandle;
	self->machine.editorsize = editorsize;
	self->machine.editoridle = editoridle;
	self->info = 0;
	self->editorhandle = 0;
	self->events = 0;	
	library_init(&self->library);
	library_load(&self->library, path);		
	mainproc = getmainentry(&self->library);
	if (mainproc) {
		self->effect = mainproc(hostcallback);
		if (self->effect) {			
			VstInt32 numInputs;
			VstInt32 numOutputs;
			unsigned int size;

			self->effect->user = self;
			self->eventcap = 1024;
			size = sizeof(struct VstEvents) + 
				sizeof(VstEvent*) * self->eventcap;
			self->events = (struct VstEvents*) malloc(size);
			table_init(&self->noteons);			
			numInputs = self->effect->numInputs;
			numOutputs = self->effect->numOutputs;			
			self->effect->user = self;
			self->effect->dispatcher (self->effect, effOpen, 0, 0, 0, 0);
			self->effect->dispatcher (self->effect, effSetSampleRate, 0, 0, 0, kSampleRate);
			self->effect->dispatcher (self->effect, effSetProcessPrecision, 0, kVstProcessPrecision32, 0, 0);
			self->effect->dispatcher (self->effect, effSetBlockSize, 0, kBlockSize, 0, 0);
			self->effect->dispatcher (self->effect, effMainsChanged, 0, 1, 0, 0);
			self->effect->dispatcher (self->effect, effStartProcess, 0, 0, 0, 0);
			self->plugininfo = machineinfo_allocinit();
			machineinfo(self->effect, self->plugininfo, self->library.path, 0);
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
		free(self->events);
		table_dispose(&self->noteons);
	}	
	if (self->plugininfo) {
		machineinfo_dispose(self->plugininfo);
		free(self->plugininfo);
		self->plugininfo = 0;
	}
	machine_dispose(&self->machine);
}

PluginEntryProc getmainentry(Library* library)
{
	PluginEntryProc rv = 0;

	rv = (PluginEntryProc)library_functionpointer(library, "VSTPluginMain");
	if(!rv) {
		rv = (PluginEntryProc)library_functionpointer(library,"main");
	}
	return rv;
}

int plugin_vst_test(const char* path, MachineInfo* rv)
{
	int vst = 0;
	
	if (path && strcmp(path, "") != 0) {
		Library library;
		PluginEntryProc mainentry;	
		
		library_init(&library);		
		library_load(&library, path);
		if (!library_empty(&library)) {
			mainentry = getmainentry(&library);
			if (mainentry) {
				AEffect* effect;
				
				effect = mainentry(hostcallback);
				vst = effect && machineinfo(effect, rv, path, 0) == 0;
			}	
		}
		library_dispose(&library);	
	}
	return vst;
}

void work(VstPlugin* self, BufferContext* bc)
{
	unsigned int c;	
	for (c = 0; c < bc->output->numchannels; ++c) {
		dsp_mul(bc->output->samples[c], bc->numsamples, 1/32768.f);
	}
	processevents(self, bc);
	self->effect->processReplacing(self->effect, bc->output->samples,
		bc->output->samples, bc->numsamples);
	for (c = 0; c < bc->output->numchannels; ++c) {
		dsp_mul(bc->output->samples[c], bc->numsamples, 32768.f);
	}
}

void processevents(VstPlugin* self, BufferContext* bc)
{
	List* p;
	int count = 0;
	int i;
	
	for (p = bc->events; p != 0 && count < self->eventcap;
			p = p->next) {
		PatternEntry* entry = (PatternEntry*)p->entry;		
		if (entry->event.cmd == SET_PANNING) {
			// todo split work
			self->machine.setpanning(&self->machine, 
					entry->event.parameter / 255.f);
		} else
		if (entry->event.note == NOTECOMMANDS_TWEAK) {
			// todo translate to midi events 
//			self->parametertweak(self, entry->event.inst,
//				entry->event.parameter);
		} else {
			
			self->events->events[count] = (VstEvent*)
				allocinitmidievent(self, entry);
			++count;			
		}			
	}	
	self->events->numEvents = count;
	self->events->reserved = 0;
	self->effect->dispatcher(self->effect, effProcessEvents, 0, 0, self->events, 0);
	for (i = 0; i < count; ++i) {		
		free(self->events->events[i]);
	}
}

struct VstMidiEvent* allocinitmidievent(VstPlugin* self, const PatternEntry* entry)
{
	struct VstMidiEvent* rv;
	char note;
	int noteon;

	rv = malloc(sizeof(struct VstMidiEvent));
	memset(rv, 0, sizeof(struct VstMidiEvent));
	noteon = entry->event.note < NOTECOMMANDS_RELEASE;
	if (noteon) {
		table_insert(&self->noteons, entry->track, (void*)entry->event.note);
		note = (char) entry->event.note;
	} else {
		note = (char) table_at(&self->noteons, entry->track);
	}
	rv->type          = kVstMidiType;
	rv->byteSize      = sizeof(struct VstMidiEvent);
	rv->flags         = kVstMidiEventIsRealtime;
	rv->midiData[0]   = (char) (entry->track + noteon ? 0x90 : 0x80);
	rv->midiData[1]   = (char)note;
	rv->midiData[2]   = (char)(127);
	return rv;
}

int hostevent(VstPlugin* self, int const eventNr, int const val1, float const val2)
{
	return 0;
}

static int FilterException(int code, struct _EXCEPTION_POINTERS *ep) 
{
	return EXCEPTION_EXECUTE_HANDLER;
}

int machineinfo(AEffect* effect, MachineInfo* info, const char* path,
	int shellidx)
{
	char effectName[256] = {0};
	char vendorString[256] = {0};
	char productString[256] = {0};
	int err = 0;

	__try {
		effect->dispatcher (effect, effGetEffectName, 0, 0, effectName, 0);
		effect->dispatcher (effect, effGetVendorString, 0, 0, vendorString, 0);
		effect->dispatcher (effect, effGetProductString, 0, 0, productString, 0);
		
		machineinfo_set(info,
			vendorString,
			"",
			(effect->flags & effFlagsIsSynth) == effFlagsIsSynth,
			effectName,
			effectName,
			0,
			0,
			MACH_VST,
			path,
			shellidx);		
	}
	__except(FilterException(GetExceptionCode(), GetExceptionInformation())) {		
		err = GetExceptionCode();
	}
	return err;
}

const MachineInfo* info(VstPlugin* self)
{	
	return self->plugininfo;
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

int parametervalue(VstPlugin* self, int param)
{
	return (int)(self->effect->getParameter(self->effect, param) * 65535.f);
}

void setvalue(VstPlugin* self, int param, int value)
{

}

VstIntPtr VSTCALLBACK hostcallback (AEffect* effect, VstInt32 opcode, VstInt32 index, VstIntPtr value, void* ptr, float opt)
{
	VstIntPtr result = 0;
	VstPlugin* self;
	
	if (effect) {
		self = (VstPlugin*) effect->user;
	} else
	if (opcode == audioMasterVersion) {
		return kVstVersion;
	} else {
		return 0;
	}
	
	switch(opcode)
	{
		case audioMasterVersion :
			result = kVstVersion;
		break;		
		case audioMasterIdle:            
        break;
		case audioMasterGetSampleRate:
			result = self->machine.samplerate(&self->machine);
		default:
		break;
	}

	return result;
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

unsigned int numparametercols(VstPlugin* self)
{
	return 6;
}

int parametertype(VstPlugin* self, int param)
{
	return MPF_STATE;
}

void parameterrange(VstPlugin* self, int param, int* minval, int* maxval)
{
	*minval = 0;
	*maxval = 65535;
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

int haseditor(VstPlugin* self)
{
	return (self->effect->flags & effFlagsHasEditor) == effFlagsHasEditor;
}

void seteditorhandle(VstPlugin* self, void* handle)
{		
	self->editorhandle = handle;
	self->effect->dispatcher(self->effect, effEditOpen, 0, 0, handle, 0);	
}

void editorsize(VstPlugin* self, int* width, int* height)
{
	struct ERect* r = 0;

	self->effect->dispatcher(self->effect, effEditGetRect, 0, 0,  &r, 0);
	if (r != 0) {
		*width = r->right - r->left;
		*height = r->bottom - r->top;
	} else {
		*width = 0;
		*height = 0;
	}
}

void editoridle(VstPlugin* self)
{
	if(self->editorhandle) {
		self->effect->dispatcher(self->effect, effEditIdle, 0, 0, 0, 0);
	}
}

int mode(VstPlugin* self)
{ 
	return (self->effect->flags & effFlagsIsSynth) == effFlagsIsSynth
		? MACHMODE_GENERATOR
		: MACHMODE_FX;
}

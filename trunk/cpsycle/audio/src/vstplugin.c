// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"
#include "../../detail/os.h"

#include "vstplugin.h"
#if defined(__GNUC__)
#define _inline inline
#endif
#include "aeffectx.h"
#include <stdlib.h>
#if defined DIVERSALIS__OS__MICROSOFT
#include <excpt.h>
#endif
#include <operations.h>
#include "pattern.h"
#include "songio.h"

static const VstInt32 kBlockSize = 512;
static const VstInt32 kNumProcessCycles = 5;

// vtable prototypes
static int mode(psy_audio_VstPlugin*);
static void work(psy_audio_VstPlugin* self, psy_audio_BufferContext*);
static const psy_audio_MachineInfo* info(psy_audio_VstPlugin*);
static void parametertweak(psy_audio_VstPlugin*, uintptr_t param, int val);
static int parameterlabel(psy_audio_VstPlugin*, char* txt, uintptr_t param);
static int parametername(psy_audio_VstPlugin*, char* txt, uintptr_t param);
static int describevalue(psy_audio_VstPlugin*, char* txt, uintptr_t param, int value);
static int parametervalue(psy_audio_VstPlugin*, uintptr_t param);
static int parametertype(psy_audio_VstPlugin*, uintptr_t param);
static void parameterrange(psy_audio_VstPlugin*, uintptr_t param, int* minval, int* maxval);
static uintptr_t numparameters(psy_audio_VstPlugin*);
static unsigned int numparametercols(psy_audio_VstPlugin*);
static void dispose(psy_audio_VstPlugin* self);
static uintptr_t numinputs(psy_audio_VstPlugin*);
static uintptr_t numoutputs(psy_audio_VstPlugin*);
static void loadspecific(psy_audio_VstPlugin*, psy_audio_SongFile*,
	uintptr_t slot);
static void savespecific(psy_audio_VstPlugin*, psy_audio_SongFile*,
	uintptr_t slot);
static int haseditor(psy_audio_VstPlugin*);
static void seteditorhandle(psy_audio_VstPlugin*, void* handle);
static void editorsize(psy_audio_VstPlugin*, int* width, int* height);
static void editoridle(psy_audio_VstPlugin*);
// private
static void checkEffectProperties (AEffect* effect);
static void checkEffectProcessing (AEffect* effect);
static int machineinfo(AEffect* effect, psy_audio_MachineInfo* info,
	const char* path, int shellidx);
typedef AEffect* (*PluginEntryProc)(audioMasterCallback audioMaster);
static VstIntPtr VSTCALLBACK hostcallback(AEffect* effect, VstInt32 opcode,
	VstInt32 index, VstIntPtr value, void* ptr, float opt);
static PluginEntryProc getmainentry(psy_Library* library);
static void processevents(psy_audio_VstPlugin*, psy_audio_BufferContext*);
static void generateaudio(psy_audio_VstPlugin*, psy_audio_BufferContext*);
struct VstMidiEvent* allocinitmidievent(psy_audio_VstPlugin*,
	const psy_audio_PatternEntry*);

struct VstMidiEvent* allocmidi(psy_audio_VstPlugin*, unsigned char data0,
	unsigned char data1, unsigned char data2);
struct VstMidiEvent* allocnoteon(psy_audio_VstPlugin*,
	const psy_audio_PatternEntry*, int channel);
struct VstMidiEvent* allocnoteoff(psy_audio_VstPlugin*, int note, int channel);
struct VstMidiEvent* allocmidientry(psy_audio_VstPlugin* self,
	const psy_audio_PatternEntry*);
static psy_dsp_amp_range_t amprange(psy_audio_VstPlugin* self)
{
	return PSY_DSP_AMP_RANGE_VST;
}

static void vstplugin_onfileselect(psy_audio_VstPlugin*, struct VstFileSelect*);

// init vstplugin class vtable
static MachineVtable vtable;
static int vtable_initialized = 0;

static void vtable_init(psy_audio_VstPlugin* self)
{
	if (!vtable_initialized) {
		vtable = *(psy_audio_vstplugin_base(self)->vtable);
		vtable.mode = (fp_machine_mode) mode;
		vtable.work = (fp_machine_work) work;
		vtable.info = (fp_machine_info) info;
		vtable.parametertype = (fp_machine_parametertype) parametertype;
		vtable.parameterrange = (fp_machine_parameterrange) parameterrange;
		vtable.parametertweak = (fp_machine_parametertweak) parametertweak;
		vtable.parameterlabel = (fp_machine_parameterlabel) parameterlabel;
		vtable.parametername = (fp_machine_parametername) parametername;
		vtable.describevalue = (fp_machine_describevalue) describevalue;	
		vtable.parametervalue = (fp_machine_parametervalue) parametervalue;
		vtable.numparameters = (fp_machine_numparameters) numparameters;
		vtable.numparametercols = (fp_machine_numparametercols)
			numparametercols;
		vtable.dispose = (fp_machine_dispose) dispose;
		vtable.numinputs = (fp_machine_numinputs) numinputs;
		vtable.numoutputs = (fp_machine_numoutputs) numoutputs;
		vtable.loadspecific = (fp_machine_loadspecific) loadspecific;
		vtable.savespecific = (fp_machine_savespecific) savespecific;
		vtable.haseditor = (fp_machine_haseditor) haseditor;
		vtable.seteditorhandle = (fp_machine_seteditorhandle) seteditorhandle;
		vtable.editorsize = (fp_machine_editorsize) editorsize;
		vtable.editoridle = (fp_machine_editoridle) editoridle;
		vtable.amprange = (fp_machine_amprange) amprange;
		vtable_initialized = 1;
	}
}

void psy_audio_vstplugin_init(psy_audio_VstPlugin* self, MachineCallback callback,
	const char* path)
{		
	PluginEntryProc mainproc;
	
	custommachine_init(&self->custommachine, callback);	
	vtable_init(self);
	psy_audio_vstplugin_base(self)->vtable = &vtable;
	psy_audio_machine_setcallback(psy_audio_vstplugin_base(self), callback);	
	self->info = 0;
	self->editorhandle = 0;
	self->events = 0;
	self->plugininfo = 0;
	psy_table_init(&self->tracknote);
	psy_library_init(&self->library);
	psy_library_load(&self->library, path);		
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
			numInputs = self->effect->numInputs;
			numOutputs = self->effect->numOutputs;			
			self->effect->user = self;
			self->effect->dispatcher (self->effect, effOpen, 0, 0, 0, 0);
			self->effect->dispatcher (self->effect, effSetSampleRate, 0, 0, 0,
				(float) psy_audio_machine_samplerate(psy_audio_vstplugin_base(self)));
			self->effect->dispatcher (self->effect, effSetProcessPrecision, 0,
				kVstProcessPrecision32, 0, 0);
			self->effect->dispatcher (self->effect, effSetBlockSize, 0, kBlockSize, 0, 0);
			self->effect->dispatcher (self->effect, effMainsChanged, 0, 1, 0, 0);
			self->effect->dispatcher (self->effect, effStartProcess, 0, 0, 0, 0);
			self->plugininfo = machineinfo_allocinit();
			machineinfo(self->effect, self->plugininfo, self->library.path, 0);
			psy_audio_machine_seteditname(psy_audio_vstplugin_base(self),
				self->plugininfo->ShortName);
		}
	}
	if (!psy_audio_machine_editname(psy_audio_vstplugin_base(self))) {
		psy_audio_machine_seteditname(psy_audio_vstplugin_base(self), "VstPlugin");
	}
} 

void dispose(psy_audio_VstPlugin* self)
{		
	if (self->library.module) {
		if (self->effect) {
			self->effect->dispatcher (self->effect, effClose, 0, 0, 0, 0);
			self->effect = 0;			
		}
		psy_library_dispose(&self->library);		
		if (self->info) {
			free((char*) self->info->Author);
			free((char*) self->info->Name);
			free((char*) self->info->ShortName);
			free((char*) self->info->Command);
		}
		self->info = 0;
		free(self->events);		
	}	
	if (self->plugininfo) {
		machineinfo_dispose(self->plugininfo);
		free(self->plugininfo);
		self->plugininfo = 0;
	}
	psy_table_dispose(&self->tracknote);
	custommachine_dispose(&self->custommachine);
}

PluginEntryProc getmainentry(psy_Library* library)
{
	PluginEntryProc rv = 0;

	rv = (PluginEntryProc)psy_library_functionpointer(library, "VSTPluginMain");
	if(!rv) {
		rv = (PluginEntryProc)psy_library_functionpointer(library,"main");
	}
	return rv;
}

int psy_audio_plugin_vst_test(const char* path, psy_audio_MachineInfo* rv)
{
	int vst = 0;
	
	if (path && strcmp(path, "") != 0) {
		psy_Library library;
		PluginEntryProc mainentry;	
		
		psy_library_init(&library);		
		psy_library_load(&library, path);
		if (!psy_library_empty(&library)) {
			mainentry = getmainentry(&library);
			if (mainentry) {
				AEffect* effect;
				
				effect = mainentry(hostcallback);
				vst = effect && machineinfo(effect, rv, path, 0) == 0;
			}	
		}
		psy_library_dispose(&library);	
	}
	return vst;
}

void work(psy_audio_VstPlugin* self, psy_audio_BufferContext* bc)
{	
	if (!psy_audio_machine_bypassed(psy_audio_vstplugin_base(self))) {		
		processevents(self, bc);					
		// add to buffer memory
		if (bc->output && bc->output->numchannels > 0) {
			if (psy_audio_machine_buffermemory(psy_audio_vstplugin_base(self))) {
				psy_audio_buffer_insertsamples(
					psy_audio_machine_buffermemory(psy_audio_vstplugin_base(self)),
						bc->output,
						psy_audio_machine_buffermemorysize(psy_audio_vstplugin_base(self)),
						bc->numsamples);
			}
		}
	}
}

void processevents(psy_audio_VstPlugin* self, psy_audio_BufferContext* bc)
{	
	psy_List* p = 0;
	int count = 0;
	int i;
	uintptr_t amount = bc->numsamples;
	uintptr_t pos = 0;
		
	for (p = bc->events; p != 0 && count < self->eventcap; p = p->next) {
		int numworksamples;
		int midichannel;
		psy_audio_PatternEntry* entry = (psy_audio_PatternEntry*)p->entry;

		numworksamples = (unsigned int)entry->delta - pos;		
		if (patternentry_front(entry)->inst == NOTECOMMANDS_INST_EMPTY) {
			midichannel = 0;
		} else {
			midichannel = patternentry_front(entry)->inst & 0x0F;
		}
		if (patternentry_front(entry)->cmd == SET_PANNING) {
			// todo split work
			psy_audio_machine_setpanning(psy_audio_vstplugin_base(self),
				patternentry_front(entry)->parameter / 255.f);
		} else
		if (patternentry_front(entry)->note == NOTECOMMANDS_MIDICC) {
			if (patternentry_front(entry)->inst >= 0x80 &&
				patternentry_front(entry)->inst < 0xFF) {
					self->events->events[count] = (VstEvent*)
					allocmidientry(self, entry);
				++count;
			} else {						
				// Panning
				if (patternentry_front(entry)->cmd == 0xC2) {
					self->events->events[count] = (VstEvent*) allocmidi(self,
						(unsigned char)(0xB0 | midichannel), 0x0A,
						(unsigned char)(
							patternentry_front(entry)->parameter >> 1));
					++count;	
				}
			}
		} else
		if (patternentry_front(entry)->note == NOTECOMMANDS_TWEAK) {			
			int value;
			
			if (numworksamples > 0) {				
				int restorenumsamples = bc->numsamples;
		
				if (bc->input) {
					psy_audio_buffer_setoffset(bc->input, pos);
				}
				if (bc->output) {
					psy_audio_buffer_setoffset(bc->output, pos);
				}
				bc->numsamples = numworksamples;
				self->events->numEvents = count;
				self->events->reserved = 0;
				generateaudio(self, bc);
				amount -= numworksamples;
				bc->numsamples = restorenumsamples;
			}
			value = (patternentry_front(entry)->cmd << 8) +
				patternentry_front(entry)->parameter;
			psy_audio_machine_patterntweak(psy_audio_vstplugin_base(self),
				patternentry_front(entry)->inst, value);
			for (i = 0; i < count; ++i) {		
				free(self->events->events[i]);
			}
			self->events->numEvents = 0;
			count = 0;
		} else 
		if (patternentry_front(entry)->note < NOTECOMMANDS_RELEASE) {
			VstNote* note = 0;

			if (psy_table_exists(&self->tracknote, entry->track)) {
				note = (VstNote*) psy_table_at(&self->tracknote, entry->track);
				self->events->events[count] = (VstEvent*)
					allocnoteoff(self, note->key, entry->track);
				++count;
			}
			// Panning
			if (patternentry_front(entry)->cmd == 0xC2) {
				self->events->events[count] = (VstEvent*) allocmidi(self,
					(unsigned char)(0xB0 | midichannel), 0x0A,
					(unsigned char)(patternentry_front(entry)->parameter >> 1));
				++count;	
			}
			self->events->events[count] = (VstEvent*)
				allocnoteon(self, entry, midichannel);
			if (!note) {
				note = malloc(sizeof(VstNote));
				psy_table_insert(&self->tracknote, entry->track, (void*) note);
			}
			note->key = patternentry_front(entry)->note;
			note->midichan = midichannel;
			++count;			
		} else
		if (patternentry_front(entry)->note == NOTECOMMANDS_RELEASE) {
			if (psy_table_exists(&self->tracknote, entry->track)) {
				VstNote* note;
				
				note = psy_table_at(&self->tracknote, entry->track);
				self->events->events[count] = (VstEvent*)
					allocnoteoff(self, note->key, note->midichan);
				++count;
				psy_table_remove(&self->tracknote, entry->track);				
			}
		}
	}	
	self->events->numEvents = count;
	self->events->reserved = 0;
	if (amount > 0) {
		int restorenumsamples = bc->numsamples;
		if (bc->input) {
			psy_audio_buffer_setoffset(bc->input, pos);
		}
		if (bc->output) {
			psy_audio_buffer_setoffset(bc->output, pos);
		}
		bc->numsamples = amount;
		self->events->numEvents = count;
		self->events->reserved = 0;
		generateaudio(self, bc);		
		bc->numsamples = restorenumsamples;
	}
	if (bc->input) {
		psy_audio_buffer_setoffset(bc->input, 0);
	}
	if (bc->output) {
		psy_audio_buffer_setoffset(bc->output, 0);	
	}
	for (i = 0; i < count; ++i) {		
		free(self->events->events[i]);
	}
	self->events->numEvents = 0;
}

void generateaudio(psy_audio_VstPlugin* self, psy_audio_BufferContext* bc)
{
	if (!psy_audio_machine_bypassed(psy_audio_vstplugin_base(self))) {
		uintptr_t c;				
				
		if (bc->output->offset > 0) {
			for (c = 0; c < bc->output->numchannels; ++c) {
				bc->output->samples[c] = bc->output->samples[c] + bc->output->offset;
			}
		}
		self->effect->dispatcher(self->effect, effProcessEvents, 0, 0,
			self->events, 0);
		if (bc->output->numchannels > 0) {
			self->effect->processReplacing(self->effect,
				bc->output->samples, bc->output->samples,			
				bc->numsamples);
		}
		if (bc->output->offset > 0) {
			for (c = 0; c < bc->output->numchannels; ++c) {
				bc->output->samples[c] = bc->output->samples[c] - bc->output->offset;
			}
		}
	}
}

struct VstMidiEvent* allocnoteon(psy_audio_VstPlugin* self, const psy_audio_PatternEntry* entry, int channel)
{
	struct VstMidiEvent* rv;	

	rv = malloc(sizeof(struct VstMidiEvent));
	if (rv) {
		char note;

		memset(rv, 0, sizeof(struct VstMidiEvent));
		note = (char) patternentry_front_const(entry)->note;
		rv->type = kVstMidiType;
		rv->byteSize = sizeof(struct VstMidiEvent);
		rv->flags = kVstMidiEventIsRealtime;
		rv->midiData[0] = (char)(channel + 0x90);
		rv->midiData[1] = (char)note;
		rv->midiData[2] = (char)(127);
	}
	return rv;
}

struct VstMidiEvent* allocnoteoff(psy_audio_VstPlugin* self, int note, int channel)
{
	struct VstMidiEvent* rv;	

	rv = malloc(sizeof(struct VstMidiEvent));
	if (rv) {
		memset(rv, 0, sizeof(struct VstMidiEvent));
		rv->type = kVstMidiType;
		rv->byteSize = sizeof(struct VstMidiEvent);
		rv->flags = kVstMidiEventIsRealtime;
		rv->midiData[0] = (char)(channel | 0x80);
		rv->midiData[1] = (char) note;
		rv->midiData[2] = (char) 0;
	}
	return rv;
}

struct VstMidiEvent* allocmidientry(psy_audio_VstPlugin* self,
	const psy_audio_PatternEntry* entry)
{
	struct VstMidiEvent* rv;	

	rv = malloc(sizeof(struct VstMidiEvent));
	if (rv) {
		char note;

		memset(rv, 0, sizeof(struct VstMidiEvent));
		note = (char) patternentry_front_const(entry)->note;
		rv->type = kVstMidiType;
		rv->byteSize = sizeof(struct VstMidiEvent);
		rv->flags = kVstMidiEventIsRealtime;
		rv->midiData[0] = (char) patternentry_front_const(entry)->inst;
		rv->midiData[1] = (char) patternentry_front_const(entry)->cmd;
		rv->midiData[2] = (char) patternentry_front_const(entry)->parameter;
	}
	return rv;
}

struct VstMidiEvent* allocmidi(psy_audio_VstPlugin* self,
	unsigned char data0, unsigned char data1, unsigned char data2)
{
	struct VstMidiEvent* rv;	

	rv = malloc(sizeof(struct VstMidiEvent));
	if (rv) {
		memset(rv, 0, sizeof(struct VstMidiEvent));		
		rv->type = kVstMidiType;
		rv->byteSize = sizeof(struct VstMidiEvent);
		rv->flags = kVstMidiEventIsRealtime;
		rv->midiData[0] = (char) data0;
		rv->midiData[1] = (char) data1;
		rv->midiData[2] = (char) data2;
	}
	return rv;
}

#if defined DIVERSALIS__OS__MICROSOFT
static int FilterException(int code, struct _EXCEPTION_POINTERS *ep) 
{
	return EXCEPTION_EXECUTE_HANDLER;
}
#endif

int machineinfo(AEffect* effect, psy_audio_MachineInfo* info, const char* path,
	int shellidx)
{
	char effectName[256] = {0};
	char vendorString[256] = {0};
	char productString[256] = {0};
	int err = 0;

#if defined DIVERSALIS__OS__MICROSOFT    
	__try
#endif   
    {
		psy_audio_MachineMode mode;

		effect->dispatcher (effect, effGetEffectName, 0, 0, effectName, 0);
		effect->dispatcher (effect, effGetVendorString, 0, 0, vendorString, 0);
		effect->dispatcher (effect, effGetProductString, 0, 0, productString, 0);
		mode = ((effect->flags & effFlagsIsSynth) == effFlagsIsSynth)
			? MACHMODE_GENERATOR
			: MACHMODE_FX;
		machineinfo_set(
			info,
			vendorString,
			"",
			0,
			mode,
			effectName,
			effectName,
			(short) 0, 
			(short) 0,
			(mode == MACHMODE_GENERATOR) ? MACH_VST : MACH_VSTFX,
			path,
			shellidx);		
	}
#if defined DIVERSALIS__OS__MICROSOFT        	
	__except(FilterException(GetExceptionCode(), GetExceptionInformation())) {		
		err = GetExceptionCode();
	}
#endif	
	return err;
}

const psy_audio_MachineInfo* info(psy_audio_VstPlugin* self)
{	
	return self->plugininfo;
}

void parametertweak(psy_audio_VstPlugin* self, uintptr_t param, int value)
{
	self->effect->setParameter(self->effect, param, value / 65535.f);
}

int parameterlabel(psy_audio_VstPlugin* self, char* txt, uintptr_t param)
{
	txt[0] = '\0';
	self->effect->dispatcher(self->effect, effGetParamLabel, param, 0, txt, 0);
	return *txt != '\0';
}

int parametername(psy_audio_VstPlugin* self, char* txt, uintptr_t param)
{
	txt[0] = '\0';
	self->effect->dispatcher(self->effect, effGetParamName, param, 0, txt, 0);
	return *txt != '\0';
}

int describevalue(psy_audio_VstPlugin* self, char* txt, uintptr_t param, int value)
{ 		
	txt[0] = '\0';
	self->effect->dispatcher(self->effect, effGetParamDisplay, param, 0, txt, 0);
	return *txt != '\0';
}

int parametervalue(psy_audio_VstPlugin* self, uintptr_t param)
{
	return (int)(self->effect->getParameter(self->effect, param) * 65535.f);
}

uintptr_t numinputs(psy_audio_VstPlugin* self)
{
	return (uintptr_t) self->effect->numInputs;
}

uintptr_t numoutputs(psy_audio_VstPlugin* self)
{
	return (uintptr_t) self->effect->numOutputs;
}

uintptr_t numparameters(psy_audio_VstPlugin* self)
{
	return self->effect->numParams;	
}

unsigned int numparametercols(psy_audio_VstPlugin* self)
{
	return 6;
}

int parametertype(psy_audio_VstPlugin* self, uintptr_t param)
{
	return MPF_STATE;
}

void parameterrange(psy_audio_VstPlugin* self, uintptr_t param, int* minval, int* maxval)
{
	*minval = 0;
	*maxval = 65535;
}

void loadspecific(psy_audio_VstPlugin* self, psy_audio_SongFile* songfile,
	uintptr_t slot)
{
	uint32_t size;
	unsigned char program;

	psyfile_read(songfile->file, &size, sizeof(size));
	if(size) {
		uint32_t count;

		psyfile_read(songfile->file, &program, sizeof program);
		psyfile_read(songfile->file, &count, sizeof count);
		size -= sizeof(program) + sizeof(count) + sizeof(float) * count;
		if(!size) {
			if (program < self->effect->numPrograms) {
				uint32_t i;				

				self->effect->dispatcher(self->effect,
					effBeginSetProgram, 0, 0, 0, 0);
				self->effect->dispatcher(self->effect,
					effSetProgram, 0, (VstIntPtr) program, 0, 0);
				for(i = 0; i < count; ++i) {
					float temp;
				
					psyfile_read(songfile->file, &temp, sizeof(temp));
					self->effect->setParameter(self->effect, (VstInt32) i,
						temp);
				}
				self->effect->dispatcher(self->effect,
					effEndSetProgram, 0, 0, 0, 0);
			}
		} else {
			self->effect->dispatcher(self->effect,
				effBeginSetProgram, 0, 0, 0, 0);
			self->effect->dispatcher(self->effect,
					effSetProgram, 0, (VstIntPtr) program, 0, 0);
			self->effect->dispatcher(self->effect,
				effEndSetProgram, 0, 0, 0, 0);
			psyfile_skip(songfile->file, sizeof(float) * count);
			if(self->effect->flags & effFlagsProgramChunks) {
				char * data;
				
				data = (char*) malloc(size);
				psyfile_read(songfile->file, data, size); // Number of parameters
				self->effect->dispatcher(self->effect,
					effSetChunk, 0, size, data, 0);					
				free(data);
				data = 0;					
			} else {
				// there is a data chunk, but this machine does not want one.
				psyfile_skip(songfile->file, size);
				return;
			}
		}	
	}
}

void savespecific(psy_audio_VstPlugin* self, psy_audio_SongFile* songfile,
	uintptr_t slot)
{	
	uint32_t count;
	unsigned char program=0;
	uint32_t size;
	uint32_t chunksize = 0;
	char * data = 0;

	count = self->effect->numParams;
	size = sizeof(program) + sizeof(count);
	if(self->effect->flags & effFlagsProgramChunks) {
		count = 0;
		chunksize = self->effect->dispatcher(self->effect,
				effGetChunk, 0, 0, &data, 0);				
		size += chunksize;
	} else {
		size += (sizeof(float) * count);
	}
	psyfile_write(songfile->file, &size, sizeof(size));
	program = (unsigned char) self->effect->dispatcher(self->effect,
		effGetProgram, 0, 0, 0, 0);	
	psyfile_write(songfile->file, &program, sizeof(program));
	psyfile_write(songfile->file, &count, sizeof count);

	if(self->effect->flags & effFlagsProgramChunks) {
		psyfile_write(songfile->file, data, chunksize);
	} else {
		uint32_t i;

		for (i = 0; i < count; ++i) {
			float temp;
			
			temp = self->effect->getParameter(self->effect, (VstInt32) i);								
			psyfile_write(songfile->file, &temp, sizeof(temp));
		}
	}
}

int haseditor(psy_audio_VstPlugin* self)
{
	return (self->effect->flags & effFlagsHasEditor) == effFlagsHasEditor;
}

void seteditorhandle(psy_audio_VstPlugin* self, void* handle)
{		
	self->editorhandle = handle;
	self->effect->dispatcher(self->effect, effEditOpen, 0, 0, handle, 0);	
}

void editorsize(psy_audio_VstPlugin* self, int* width, int* height)
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

void editoridle(psy_audio_VstPlugin* self)
{
	if(self->editorhandle) {
		self->effect->dispatcher(self->effect, effEditIdle, 0, 0, 0, 0);
	}
}

int mode(psy_audio_VstPlugin* self)
{ 
	return (self->effect->flags & effFlagsIsSynth) == effFlagsIsSynth
		? MACHMODE_GENERATOR
		: MACHMODE_FX;
}

VstIntPtr VSTCALLBACK hostcallback (AEffect* effect, VstInt32 opcode, VstInt32 index, VstIntPtr value, void* ptr, float opt)
{
	VstIntPtr result = 0;
	psy_audio_VstPlugin* self;	
	
	if (effect) {
		self = (psy_audio_VstPlugin*) effect->user;
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
			result = psy_audio_machine_samplerate(psy_audio_vstplugin_base(
				self));
		break;
		case audioMasterOpenFileSelector :
			vstplugin_onfileselect(self, (struct VstFileSelect*) ptr);
		break;		
		default:
		break;
	}

	return result;
}

void vstplugin_onfileselect(psy_audio_VstPlugin* self,
	struct VstFileSelect* select)
{
	switch (select->command) {
		case kVstFileLoad:
			self->custommachine.machine.callback.fileselect_load(
				self->custommachine.machine.callback.context);
		break;
		case kVstFileSave:
			self->custommachine.machine.callback.fileselect_save(
				self->custommachine.machine.callback.context);
		break;
		case kVstDirectorySelect:
			self->custommachine.machine.callback.fileselect_directory(
				self->custommachine.machine.callback.context);
		break;
		default:
		break;
	}
}

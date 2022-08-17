/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "vstplugin.h"

#ifdef PSYCLE_USE_VST2

#include "../../detail/os.h"
#if defined(DIVERSALIS__OS__UNIX)
#define _inline static inline
#endif
#include "aeffectx.h"
#include <stdlib.h>
#if defined DIVERSALIS__OS__MICROSOFT
#include <excpt.h>
#endif
#include <operations.h>
#include "pattern.h"
#include "sequencer.h"
#include "plugin_interface.h"
#include "songio.h"
#include "preset.h"

#include "../../driver/eventdriver.h"

#include <dir.h>

#include "../../detail/portable.h"
#include "../../detail/trace.h"

static const VstInt32 kBlockSize = 512;
static const VstInt32 kNumProcessCycles = 5;

#define MIDI_CC_NOTEOFF_ALL 0x7B
#define MIDI_CC_SOUNDOFF_ALL 0x78

// VstNote
typedef struct {
	unsigned char key;
	unsigned char midichan;
} VstNote;

// VstTimeInfo
static void vsttimeinfo_init_default(struct VstTimeInfo* rv)
{	
	rv->samplePos = 0.0;
	rv->sampleRate = 44100.0;
	rv->nanoSeconds = 0.0;
	rv->ppqPos = 0.0;
	rv->tempo = 120.0;
	rv->barStartPos = 0.0;
	rv->cycleStartPos = 0.0;
	rv->cycleEndPos = 0.0;
	rv->timeSigNumerator = 4;
	rv->timeSigDenominator = 4;
	rv->smpteOffset = 0;
	rv->smpteFrameRate = 1;
	rv->samplesToNextClock = 0;
	rv->flags = 0;
}

static struct VstTimeInfo* vsttimeinfo_alloc(void)
{
	return (struct VstTimeInfo*)malloc(sizeof(struct VstTimeInfo));
}

static void vsttimeinfo_deallocate(struct VstTimeInfo* self)
{
	free(self);	
}

// vtable prototypes
static int mode(psy_audio_VstPlugin*);
static void work(psy_audio_VstPlugin* self, psy_audio_BufferContext*);
static const psy_audio_MachineInfo* info(psy_audio_VstPlugin*);
// Parameter
static psy_audio_MachineParam* parameter(psy_audio_VstPlugin*,
	uintptr_t param);
static uintptr_t numparameters(psy_audio_VstPlugin*);
static unsigned int numparametercols(psy_audio_VstPlugin*);
static void dispose(psy_audio_VstPlugin* self);
static uintptr_t numinputs(psy_audio_VstPlugin*);
static uintptr_t numoutputs(psy_audio_VstPlugin*);
static int loadspecific(psy_audio_VstPlugin*, psy_audio_SongFile*,
	uintptr_t slot);
static int savespecific(psy_audio_VstPlugin*, psy_audio_SongFile*,
	uintptr_t slot);
static int haseditor(psy_audio_VstPlugin*);
static void seteditorhandle(psy_audio_VstPlugin*, void* handle);
static void editorsize(psy_audio_VstPlugin*, double* width, double* height);
static void editoridle(psy_audio_VstPlugin*);
static int makemachineinfo(AEffect* effect, psy_audio_MachineInfo*,
	const char* path, int shellidx);
typedef AEffect* (*PluginEntryProc)(audioMasterCallback audioMaster);
static VstIntPtr VSTCALLBACK hostcallback(AEffect* effect, VstInt32 opcode,
	VstInt32 index, VstIntPtr value, void* ptr, float opt);
static PluginEntryProc getmainentry(psy_Library* library);
static void processevents(psy_audio_VstPlugin*, psy_audio_BufferContext*);
static void generateaudio(psy_audio_VstPlugin*, psy_audio_BufferContext*);
static void stop(psy_audio_VstPlugin*);
// auxcolumns
static const char* auxcolumnname(psy_audio_VstPlugin*, uintptr_t index);
static uintptr_t numauxcolumns(psy_audio_VstPlugin*);

static psy_dsp_amp_range_t amprange(psy_audio_VstPlugin* self)
{
	return PSY_DSP_AMP_RANGE_VST;
}

// programs
static void programname(psy_audio_VstPlugin*, uintptr_t bnkidx,
	uintptr_t prgidx, char* val);
static uintptr_t numprograms(psy_audio_VstPlugin*);
static void setcurrprogram(psy_audio_VstPlugin*, uintptr_t prgidx);
static uintptr_t currprogram(psy_audio_VstPlugin*);
static void bankname(psy_audio_VstPlugin*, uintptr_t bnkidx, char* val);
static uintptr_t numbanks(psy_audio_VstPlugin*);
static void setcurrbank(psy_audio_VstPlugin*, uintptr_t bnkidx);
static uintptr_t currbank(psy_audio_VstPlugin*);
static void currentpreset(psy_audio_VstPlugin*, psy_audio_Preset*);
static void vstplugin_onfileselect(psy_audio_VstPlugin*,
	struct VstFileSelect*);
static void initparameters(psy_audio_VstPlugin*);
static void disposeparameters(psy_audio_VstPlugin*);
static void update_vsttimeinfo(psy_audio_VstPlugin*);

static const char* MIDI_CHAN_NAMES[16] = {
	"MIDI Channel 01", "MIDI Channel 02","MIDI Channel 03","MIDI Channel 04",
	"MIDI Channel 05","MIDI Channel 06","MIDI Channel 07","MIDI Channel 08",
	"MIDI Channel 09","MIDI Channel 10","MIDI Channel 11","MIDI Channel 12",
	"MIDI Channel 13","MIDI Channel 14","MIDI Channel 15","MIDI Channel 16"
};

/* vtable */
static MachineVtable vtable;
static int vtable_initialized = FALSE;

static void vtable_init(psy_audio_VstPlugin* self)
{
	if (!vtable_initialized) {
		vtable = *(psy_audio_vstplugin_base(self)->vtable);
		vtable.mode = (fp_machine_mode)mode;
		vtable.work = (fp_machine_work)work;
		vtable.info = (fp_machine_info)info;
		vtable.parameter = (fp_machine_parameter)parameter;
		vtable.numparameters = (fp_machine_numparameters)numparameters;
		vtable.numparametercols = (fp_machine_numparametercols)
			numparametercols;
		vtable.dispose = (fp_machine_dispose)dispose;
		vtable.numinputs = (fp_machine_numinputs)numinputs;
		vtable.numoutputs = (fp_machine_numoutputs)numoutputs;
		vtable.loadspecific = (fp_machine_loadspecific)loadspecific;
		vtable.savespecific = (fp_machine_savespecific)savespecific;
		vtable.haseditor = (fp_machine_haseditor)haseditor;
		vtable.seteditorhandle = (fp_machine_seteditorhandle)seteditorhandle;
		vtable.editorsize = (fp_machine_editorsize)editorsize;
		vtable.editoridle = (fp_machine_editoridle)editoridle;
		vtable.amprange = (fp_machine_amprange)amprange;
		vtable.programname = (fp_machine_programname)programname;
		vtable.numprograms = (fp_machine_numprograms)numprograms;
		vtable.setcurrprogram = (fp_machine_setcurrprogram)setcurrprogram;
		vtable.currprogram = (fp_machine_currprogram)currprogram;
		vtable.bankname = (fp_machine_bankname)bankname;
		vtable.numbanks = (fp_machine_numbanks)numbanks;
		vtable.setcurrbank = (fp_machine_setcurrbank)setcurrbank;
		vtable.currbank = (fp_machine_currbank)currbank;
		vtable.currentpreset = (fp_machine_currentpreset)currentpreset;
		vtable.stop = (fp_machine_stop)stop;
		vtable.auxcolumnname = (fp_machine_auxcolumnname)auxcolumnname;
		vtable.numauxcolumns = (fp_machine_numauxcolumns)numauxcolumns;
		vtable_initialized = TRUE;
	}
}

/* implementation */
int psy_audio_vstplugin_init(psy_audio_VstPlugin* self,
	psy_audio_MachineCallback* callback, const char* path)
{	
	int status;	
	PluginEntryProc mainproc;
	
	assert(self);

	status = PSY_OK;
	psy_audio_custommachine_init(&self->custommachine, callback);
	vtable_init(self);
	psy_audio_vstplugin_base(self)->vtable = &vtable;
	psy_audio_machine_setcallback(psy_audio_vstplugin_base(self), callback);
	psy_table_init(&self->parameters);	
	self->editorhandle = NULL;
	self->plugininfo = NULL;
	psy_audio_vstevents_init(&self->vstevents, 0);
	psy_audio_vstevents_init(&self->vstoutevents, 0);
	self->vsttimeinfo = vsttimeinfo_alloc();
	vsttimeinfo_init_default(self->vsttimeinfo);
	psy_audio_vstinterface_init(&self->mi, NULL, NULL);
	psy_table_init(&self->tracknote);
	psy_library_init(&self->library);	
	psy_library_load(&self->library, path);	
	mainproc = getmainentry(&self->library);
	if (mainproc) {
		AEffect* effect;

		effect = mainproc(hostcallback);
		if (effect) {						
			psy_audio_vstevents_dispose(&self->vstevents);
			psy_audio_vstevents_dispose(&self->vstoutevents);
			psy_audio_vstevents_init(&self->vstevents, 1024);
			psy_audio_vstevents_init(&self->vstoutevents, 1024);
			psy_audio_vstinterface_init(&self->mi, effect, self);
			psy_audio_vstinterface_open(&self->mi);
			psy_audio_vstinterface_setsamplerate(&self->mi,	(float)
				psy_audio_machine_samplerate(psy_audio_vstplugin_base(self)));
			psy_audio_vstinterface_setprocessprecision32(&self->mi);			
			psy_audio_vstinterface_setblocksize(&self->mi, kBlockSize);
			psy_audio_vstinterface_mainschanged(&self->mi);			
			psy_audio_vstinterface_startprocess(&self->mi);			
			self->plugininfo = machineinfo_allocinit();
			makemachineinfo(effect, self->plugininfo, self->library.path,
				0);
			psy_audio_machine_seteditname(psy_audio_vstplugin_base(self),
				self->plugininfo->shortname);			
			initparameters(self);
		} else {
			status = PSY_ERRRUN;
		}
	} else {
		status = PSY_ERRRUN;
	}
	if (!psy_audio_machine_editname(psy_audio_vstplugin_base(self))) {
		psy_audio_machine_seteditname(psy_audio_vstplugin_base(self),
			"VstPlugin");
	}
	return status;
} 

void dispose(psy_audio_VstPlugin* self)
{
	assert(self);

	if (self->library.module) {		
		psy_audio_vstinterface_close(&self->mi);		
		psy_library_dispose(&self->library);		
		psy_audio_vstevents_dispose(&self->vstevents);
		psy_audio_vstevents_dispose(&self->vstoutevents);
	}	
	if (self->plugininfo) {
		machineinfo_dispose(self->plugininfo);
		free(self->plugininfo);
		self->plugininfo = NULL;
	}
	psy_table_dispose(&self->tracknote);
	disposeparameters(self);
	vsttimeinfo_deallocate(self->vsttimeinfo);	
	psy_audio_custommachine_dispose(&self->custommachine);
}

void initparameters(psy_audio_VstPlugin* self)
{
	int32_t gbp;

	assert(self);
	if (self->mi.effect) {
		for (gbp = 0; gbp < self->mi.effect->numParams; ++gbp) {
			psy_table_insert(&self->parameters, gbp, (void*)
				psy_audio_vstpluginparam_allocinit(self->mi.effect, gbp));
		}
	}
}

void disposeparameters(psy_audio_VstPlugin* self)
{
	assert(self);

	psy_table_dispose_all(&self->parameters, (psy_fp_disposefunc)
		psy_audio_vstpluginparam_dispose);	
}

PluginEntryProc getmainentry(psy_Library* library)
{
	PluginEntryProc rv;

	rv = (PluginEntryProc)psy_library_functionpointer(library,
		"VSTPluginMain");
	if(!rv) {
		rv = (PluginEntryProc)psy_library_functionpointer(library, "main");
	}
	return rv;
}

bool psy_audio_vstplugin_test(const char* path, psy_audio_MachineInfo* rv)
{
	bool success = FALSE;
	
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
				if (effect) {
					psy_audio_VstInterface mi;
					
					psy_audio_vstinterface_init(&mi, effect, NULL);
					psy_audio_vstinterface_open(&mi);					
					success = (makemachineinfo(effect, rv, path, 0) == PSY_OK);
					psy_audio_vstinterface_close(&mi);					
				} else {
					success = FALSE;
				}
			}	
		}
		psy_library_dispose(&library);	
	}
	return success;
}

void work(psy_audio_VstPlugin* self, psy_audio_BufferContext* bc)
{
	uintptr_t n;

	assert(self);	
		
	if (bc->numsamples > 0 &&
			!psy_audio_machine_muted(psy_audio_vstplugin_base(self)) &&
			((psy_audio_vstinterface_mode(&self->mi) == psy_audio_MACHMODE_GENERATOR) ||
			 !psy_audio_machine_bypassed(psy_audio_vstplugin_base(self)))) {
		processevents(self, bc);		
	}
	// copy midi out events generated by the vst plugin (collected in the
	// HostCallback) within this process interval to the bc. The player
	// will delegate them to the midiinput of sequencer
	for (n = 0; n < self->vstoutevents.counter; ++n) {
		struct VstMidiEvent* buffer;
		struct psy_EventDriverMidiData* ev;

		buffer = (struct VstMidiEvent*)self->vstoutevents.events->events[n];
		ev = (psy_EventDriverMidiData*)malloc(sizeof(psy_EventDriverMidiData));
		ev->byte0 = buffer->midiData[0];
		ev->byte1 = buffer->midiData[1];
		ev->byte2 = buffer->midiData[2];
		psy_list_append(&bc->outevents, ev);
	}
	psy_audio_vstevents_clear(&self->vstoutevents);
}

void processevents(psy_audio_VstPlugin* self, psy_audio_BufferContext* bc)
{		
	psy_List* p = 0;		
	uintptr_t amount = bc->numsamples;
	uintptr_t pos = 0;

	assert(self);
		
	for (p = bc->events; p != NULL; psy_list_next(&p)) {		
		psy_audio_PatternEntry* entry;		
		psy_audio_PatternEvent* ev;
		intptr_t numworksamples;
		int midichannel;

		entry = psy_audio_patternnode_entry(p);
		ev = psy_audio_patternentry_front(entry);
		numworksamples = (intptr_t)entry->delta - pos;
		if (ev->note == psy_audio_NOTECOMMANDS_EMPTY &&
				ev->cmd == psy_audio_PATTERNCMD_EXTENDED) {
			if ((ev->parameter & 0xF0) == psy_audio_PATTERNCMD_SET_BYPASS) {
				if ((ev->parameter & 0x0F) == 0) {
					psy_audio_machine_unbypass(psy_audio_vstplugin_base(self));
				} else {
					psy_audio_machine_bypass(psy_audio_vstplugin_base(self));
				}
			} else if ((ev->parameter & 0xF0) == psy_audio_PATTERNCMD_SET_MUTE) {
				if ((ev->parameter & 0x0F) == 0) {
					psy_audio_machine_unmute(psy_audio_vstplugin_base(self));
				} else {
					psy_audio_machine_mute(psy_audio_vstplugin_base(self));
				}
			}
		} else if (psy_audio_patternentry_front(entry)->inst == psy_audio_NOTECOMMANDS_INST_EMPTY) {
			midichannel = 0;
		} else {
			midichannel = psy_audio_patternentry_front(entry)->inst & 0x0F;
		}
		if (psy_audio_patternentry_front(entry)->cmd == psy_audio_PATTERNCMD_SET_PANNING) {
			// todo split work
			psy_audio_machine_setpanning(psy_audio_vstplugin_base(self),
				psy_audio_patternentry_front(entry)->parameter / 255.f);
		} else if (psy_audio_patternentry_front(entry)->note == psy_audio_NOTECOMMANDS_MIDICC) {
			if (psy_audio_patternentry_front(entry)->inst >= 0x80 &&
				psy_audio_patternentry_front(entry)->inst < 0xFF) {
				psy_audio_vstevents_append_midi(&self->vstevents,
					(char)psy_audio_patternentry_front_const(entry)->inst,
					(char)psy_audio_patternentry_front_const(entry)->cmd,
					(char)psy_audio_patternentry_front_const(entry)->parameter);
			} else {
				// Panning
				if (psy_audio_patternentry_front(entry)->cmd == 0xC2) {
					psy_audio_vstevents_append_midi_control(&self->vstevents,
						midichannel, 0x0A, (unsigned char)
						(psy_audio_patternentry_front(entry)->parameter >> 1));
				}
			}
		} else if (psy_audio_patternentry_front(entry)->note == psy_audio_NOTECOMMANDS_TWEAK) {
			psy_audio_MachineParam* param;
						
			if (numworksamples > 0) {				
				intptr_t restorenumsamples = bc->numsamples;
		
				psy_audio_buffercontext_setoffset(bc, pos);				
				bc->numsamples = numworksamples;				
				generateaudio(self, bc);				
				amount -= numworksamples;
				pos = (unsigned int)entry->delta;
				bc->numsamples = restorenumsamples;
				psy_audio_buffercontext_setoffset(bc, 0);
			}
			param = psy_audio_machine_tweakparameter(psy_audio_vstplugin_base(self),
				psy_audio_patternentry_front(entry)->inst);			
			if (param) {
				uint16_t v;

				v = psy_audio_patternevent_tweakvalue(psy_audio_patternentry_front(entry));
				if (psy_audio_patternentry_front(entry)->vol > 0) {
					int32_t curr;
					int32_t step;
					int32_t nv;

					curr = (int32_t)psy_audio_machine_parameter_patternvalue(
						psy_audio_vstplugin_base(self), param);
					step = (v - curr) / psy_audio_patternentry_front(entry)->vol;
					nv = curr + step;
					psy_audio_machine_parameter_tweak_pattern(psy_audio_vstplugin_base(self), param, nv);
				} else {
					psy_audio_machine_parameter_tweak_pattern(psy_audio_vstplugin_base(self), param, v);
				}
			}			
			psy_audio_vstevents_clear(&self->vstevents);			
		} else if (psy_audio_patternentry_front(entry)->note < psy_audio_NOTECOMMANDS_RELEASE) {
			VstNote* note = 0;

			if (psy_table_exists(&self->tracknote, entry->track)) {
				note = (VstNote*) psy_table_at(&self->tracknote, entry->track);
				psy_audio_vstevents_append_noteoff(&self->vstevents,
					(uint8_t)entry->track, note->key);
			}
			// Panning
			if (psy_audio_patternentry_front(entry)->cmd == 0xC2) {				
				psy_audio_vstevents_append_midi_control(&self->vstevents,
					0x0A, midichannel, (unsigned char)
					(psy_audio_patternentry_front(entry)->parameter >> 1));
			}			
			psy_audio_vstevents_append_noteon(&self->vstevents,
				midichannel, psy_audio_patternentry_front(entry)->note);
			if (!note) {
				note = malloc(sizeof(VstNote));
				psy_table_insert(&self->tracknote, entry->track, (void*) note);
			}
			note->key = psy_audio_patternentry_front(entry)->note;
			note->midichan = midichannel;			
		} else if (psy_audio_patternentry_front(entry)->note ==
				psy_audio_NOTECOMMANDS_RELEASE) {
			if (psy_table_exists(&self->tracknote, entry->track)) {
				VstNote* note;
				
				note = psy_table_at(&self->tracknote, entry->track);
				psy_audio_vstevents_append_noteoff(&self->vstevents,
					note->midichan, note->key);
				psy_table_remove(&self->tracknote, entry->track);				
			}
		}
	}	
	if (amount > 0) {
		intptr_t restorenumsamples = bc->numsamples;
		psy_audio_buffercontext_setoffset(bc, pos);		
		bc->numsamples = amount;		
		generateaudio(self, bc);		
		bc->numsamples = restorenumsamples;
	}
	psy_audio_buffercontext_setoffset(bc, 0);
	psy_audio_vstevents_clear(&self->vstevents);
}

void generateaudio(psy_audio_VstPlugin* self, psy_audio_BufferContext* bc)
{
	uintptr_t c;

	assert(self);	
				
	if (bc->output->offset > 0) {
		for (c = 0; c < bc->output->numchannels; ++c) {
			bc->output->samples[c] = bc->output->samples[c] +
				bc->output->offset;
		}
	}				
	psy_audio_vstinterface_tick(&self->mi, self->vstevents.events);		
	if (bc->output->numchannels > 0) {
		psy_audio_vstinterface_work(&self->mi, bc);
	}
	if (bc->output->offset > 0) {
		for (c = 0; c < bc->output->numchannels; ++c) {
			bc->output->samples[c] = bc->output->samples[c] -
				bc->output->offset;
		}
	}	
}

void stop(psy_audio_VstPlugin* self)
{	
	psy_TableIterator it;		
	uint8_t midichannel;

	assert(self);

	psy_audio_vstevents_clear(&self->vstevents);
	// send note off to all tracknotes	
	for (it = psy_table_begin(&self->tracknote);			
			!psy_tableiterator_equal(&it, psy_table_end());
			psy_tableiterator_inc(&it)) {
		psy_audio_vstevents_append_noteoff(&self->vstevents,
			((VstNote*)psy_tableiterator_value(&it))->midichan,
			((VstNote*)psy_tableiterator_value(&it))->key);
	}
	for (midichannel = 0; midichannel < 16; ++midichannel) {		
		psy_audio_vstevents_append_midi_control(&self->vstevents,
			midichannel, MIDI_CC_NOTEOFF_ALL, 0);
		psy_audio_vstevents_append_midi_control(&self->vstevents,		
			midichannel, MIDI_CC_SOUNDOFF_ALL, 0);
	}	
	psy_audio_vstinterface_tick(&self->mi, self->vstevents.events);
	psy_audio_vstevents_clear(&self->vstevents);
}

#if defined DIVERSALIS__OS__MICROSOFT
static int FilterException(int code, struct _EXCEPTION_POINTERS *ep) 
{
	return EXCEPTION_EXECUTE_HANDLER;
}
#endif

static int makemachineinfo(AEffect* effect, psy_audio_MachineInfo* info,
	const char* filename, int shellidx)
{
	char effectName[256] = {0};
	char vendorString[256] = {0};
	char productString[256] = {0};
	char productName[256] = { 0 };
	char productNameShort[256] = { 0 };
	int err = 0;

#if defined DIVERSALIS__OS__MICROSOFT    
	__try
#endif   
    {
		psy_audio_MachineMode mode;

		// GetEffectName is the better option to GetProductString.
		// To the few that they show different values in these,
		// synthedit plugins show only "SyntheditVST" in GetProductString()
		// and others like battery 1 or psp-nitro, don't have GetProductString(),
		// so it's almost a no-go.
		effect->dispatcher(effect, effGetEffectName, 0, 0, effectName, 0);
		effect->dispatcher(effect, effGetVendorString, 0, 0, vendorString, 0);
		effect->dispatcher(effect, effGetProductString, 0, 0, productString, 0);
		// No effectName but productString, use productstring
		if (effectName[0] == '\0' && productString[0] != '\0') {			
			psy_snprintf(productName, 256, "%s", productString);
			psy_snprintf(productNameShort, 256, "%s", productString);
		} else
		// use effectName and productString if different
		if (effectName[0] != '\0' && productString[0] != '\0' &&
			strcmp(effectName, productString) != 0) {			
			psy_snprintf(productName, 256, "%s (%s)", effectName, productString);
			psy_snprintf(productNameShort, 256, "%s", effectName);
		} else
		// use only effectName
		if (effectName[0] != '\0') {			
			psy_snprintf(productName, 256, "%s", effectName);
			psy_snprintf(productNameShort, 256, "%s", effectName);
		} else {
			// neither effect nor productString, extract name from dll path
			psy_Path path;			

			psy_path_init(&path, filename);
			psy_snprintf(productName, 256, psy_path_name(&path));
			psy_strlwr(productName);
			psy_replacechar(productName, ' ', '-');
			psy_replacechar(productName, '_', '-');
			psy_snprintf(productNameShort, 256, "%s", productName);
			psy_path_dispose(&path);
		}				
		mode = ((effect->flags & effFlagsIsSynth) == effFlagsIsSynth)
			? psy_audio_MACHMODE_GENERATOR
			: psy_audio_MACHMODE_FX;
		machineinfo_set(
			info,
			vendorString,
			"",
			0,
			mode,
			productName,
			productNameShort,
			(int16_t) 0, 
			(int16_t) 0,
			(mode == psy_audio_MACHMODE_GENERATOR) ? psy_audio_VST : psy_audio_VSTFX,
			filename,
			shellidx,
			"",
			"VST",
			"");		
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
	assert(self);

	return self->plugininfo;
}

uintptr_t numinputs(psy_audio_VstPlugin* self)
{
	assert(self);

	return psy_audio_vstinterface_numinputs(&self->mi);
}

uintptr_t numoutputs(psy_audio_VstPlugin* self)
{
	assert(self);

	return psy_audio_vstinterface_numoutputs(&self->mi);
}

psy_audio_MachineParam* parameter(psy_audio_VstPlugin* self, uintptr_t param)
{
	assert(self);

	return (psy_audio_MachineParam*)psy_table_at(&self->parameters, param);
}

uintptr_t numparameters(psy_audio_VstPlugin* self)
{
	assert(self);

	return psy_audio_vstinterface_numparameters(&self->mi);	
}

unsigned int numparametercols(psy_audio_VstPlugin* self)
{
	assert(self);

	return 6;
}

int loadspecific(psy_audio_VstPlugin* self, psy_audio_SongFile* songfile,
	uintptr_t slot)
{
	uint32_t size;
	unsigned char program;
	int status;

	assert(self);

	if (status = psyfile_read(songfile->file, &size, sizeof(size))) {
		return status;
	}
	if(size) {
		uint32_t count;

		if (status = psyfile_read(songfile->file, &program, sizeof program)) {
			return status;
		}
		if (status = psyfile_read(songfile->file, &count, sizeof count)) {
			return status;
		}
		size -= sizeof(program) + sizeof(count) + sizeof(float) * count;
		if (!size) {
			if (program < psy_audio_vstinterface_numprograms(&self->mi)) {
				uint32_t i;				
				
				psy_audio_vstinterface_beginprogram(&self->mi);
				psy_audio_vstinterface_setprogram(&self->mi, program);
				for(i = 0; i < count; ++i) {
					float temp;
				
					if (status = psyfile_read(songfile->file, &temp,
							sizeof(temp))) {
						return status;
					}
					psy_audio_vstinterface_setparametervalue(&self->mi, i,
						temp);
				}
				psy_audio_vstinterface_endprogram(&self->mi);				
			}
		} else {			
			psy_audio_vstinterface_beginprogram(&self->mi);
			psy_audio_vstinterface_setprogram(&self->mi, program);
			psy_audio_vstinterface_endprogram(&self->mi);
			if (psyfile_skip(songfile->file, sizeof(float) * count) == -1) {
				return PSY_ERRFILE;
			}
			if (psy_audio_vstinterface_hasprogramchunk(&self->mi)) {
				char * data;
				
				data = (char*)malloc(size);
				// Number of parameters
				if (status = psyfile_read(songfile->file, data, size)) {
					free(data);
					return status;
				}
				psy_audio_vstinterface_setchunkdata(&self->mi, FALSE, data,
					size);
				free(data);				
			} else {
				// there is a data chunk, but this machine does not want one.
				if (psyfile_skip(songfile->file, size) == -1) {
					return PSY_ERRFILE;
				}
				return PSY_OK;
			}
		}	
	}
	disposeparameters(self);
	psy_table_init(&self->parameters);
	initparameters(self);
	return PSY_OK;
}

int savespecific(psy_audio_VstPlugin* self, psy_audio_SongFile* songfile,
	uintptr_t slot)
{	
	uint32_t count;
	uint8_t program;
	uint32_t size;
	uintptr_t chunksize;
	char* data;
	int status;

	assert(self);

	program = 0;
	chunksize = 0;	
	count = (uint32_t)numparameters(self);
	size = sizeof(program) + sizeof(count);
	data = psy_audio_vstinterface_chunkdata(&self->mi, FALSE, &chunksize);
	if (data) {
		count = 0;		
		size += (uint32_t)chunksize;
	} else {
		size += sizeof(float) * count;
	}
	if (status = psyfile_write(songfile->file, &size, sizeof(size))) {
		return status;
	}
	program = psy_audio_vstinterface_program(&self->mi);
	if (status = psyfile_write(songfile->file, &program, sizeof(program))) {
		return status;
	}
	if (status = psyfile_write(songfile->file, &count, sizeof count)) {
		return status;
	}

	if (data) {
		if (status = psyfile_write(songfile->file, data, (uint32_t)chunksize)) {
			return status;
		}
	} else {
		uint32_t i;

		for (i = 0; i < count; ++i) {
			float temp;
			
			temp = psy_audio_vstinterface_parametervalue(&self->mi, i);
			if (status = psyfile_write(songfile->file, &temp, sizeof(float))) {
				return status;
			}
		}
	}
	return PSY_OK;
}

int haseditor(psy_audio_VstPlugin* self)
{
	assert(self);

	return psy_audio_vstinterface_haseditor(&self->mi);	
}

void seteditorhandle(psy_audio_VstPlugin* self, void* handle)
{		
	assert(self);

	if (self->editorhandle && handle == NULL) {
		self->editorhandle = NULL;
		psy_audio_vstinterface_closeeditor(&self->mi, handle);		
	} else {
		self->editorhandle = handle;
		psy_audio_vstinterface_openeditor(&self->mi, handle);		
	}
}

void editorsize(psy_audio_VstPlugin* self, double* width, double* height)
{
	struct ERect* r = 0;

	assert(self);

	if (self->mi.effect) {

		self->mi.effect->dispatcher(self->mi.effect, effEditGetRect, 0, 0, &r, 0);
		if (r != 0) {
			*width = (double)(r->right - r->left);
			*height = (double)(r->bottom - r->top);
			return;
		}
	}
	*width = 0.0;
	*height = 0.0;	
}

void editoridle(psy_audio_VstPlugin* self)
{
	assert(self);

	if(self->editorhandle) {
		psy_audio_vstinterface_editoridle(&self->mi);		
	}
}

int mode(psy_audio_VstPlugin* self)
{ 
	assert(self);

	return psy_audio_vstinterface_mode(&self->mi);	
}

void programname(psy_audio_VstPlugin* self, uintptr_t bnkidx, uintptr_t prgidx,
	char* val)
{
	assert(self);

	if (prgidx != psy_INDEX_INVALID) {
		self->mi.effect->dispatcher(self->mi.effect, effGetProgramNameIndexed,
			(VstInt32)(bnkidx * 128 + prgidx), -1, val, 0);
	} else {
		val[0] = '\0';
	}
}

uintptr_t numprograms(psy_audio_VstPlugin* self)
{
	assert(self);

	return psy_audio_vstinterface_numprograms(&self->mi);	
}

void setcurrprogram(psy_audio_VstPlugin* self, uintptr_t prgidx)
{
	assert(self);

	psy_audio_vstinterface_setprogram(&self->mi, prgidx);	
}

uintptr_t currprogram(psy_audio_VstPlugin* self)
{
	assert(self);

	return psy_audio_vstinterface_program(&self->mi);	
}

void bankname(psy_audio_VstPlugin* self, uintptr_t bnkidx, char* val)
{
	assert(self);

	if (bnkidx < numbanks(self)) {
		psy_snprintf(val, 256, "Internal %d", bnkidx + 1);
	} else {
		val[0] = '\0';
	}
}

uintptr_t numbanks(psy_audio_VstPlugin* self)
{
	assert(self);

	return (numprograms(self) / 128) + 1;
}

void setcurrbank(psy_audio_VstPlugin* self, uintptr_t bnkidx)
{
	assert(self);

	setcurrprogram(self, bnkidx * 128 + currprogram(self));
}

uintptr_t currbank(psy_audio_VstPlugin* self)
{
	assert(self);

	return currprogram(self) / 128;
}

void currentpreset(psy_audio_VstPlugin* self, psy_audio_Preset* preset)
{	
	uintptr_t gbp;
	uintptr_t chunksize;
	void* ptr;

	assert(self);

	for (gbp = 0; gbp < numparameters(self); ++gbp) {
		psy_audio_MachineParam* param;

		param = parameter(self, gbp);
		if (param) {
			float value;
			
			value = psy_audio_machineparam_normvalue(param);
			psy_audio_preset_setvalue(preset, gbp, (intptr_t)(value * 0xFFFF));
			preset->isfloat = TRUE;
		}		
	}
	ptr = psy_audio_vstinterface_chunkdata(&self->mi, TRUE, &chunksize);
	if (ptr) {
		psy_audio_preset_putdata(preset, (int)chunksize, ptr);
	}
	if (self->mi.effect) {
		preset->id = self->mi.effect->uniqueID;
		preset->magic = self->mi.effect->magic;
		preset->version = self->mi.effect->version;
	}
}

void vstplugin_onfileselect(psy_audio_VstPlugin* self,
	struct VstFileSelect* select)
{
	assert(self);

	if (!self->custommachine.machine.callback) {
		return;
	}
	switch (select->command) {
	case kVstFileLoad:
		self->custommachine.machine.callback->vtable->fileselect_load(
			self->custommachine.machine.callback, NULL, NULL);
		break;
	case kVstFileSave:
		self->custommachine.machine.callback->vtable->fileselect_save(
			self->custommachine.machine.callback, NULL, NULL);
		break;
	case kVstDirectorySelect:
		self->custommachine.machine.callback->vtable->fileselect_directory(
			self->custommachine.machine.callback);
		break;
	default:
		break;
	}
}

void update_vsttimeinfo(psy_audio_VstPlugin* self)
{	
	psy_audio_SequencerTime* sequencertime;	

	assert(self);

	sequencertime = psy_audio_machine_sequencertime(
		psy_audio_vstplugin_base(self));
	self->vsttimeinfo->sampleRate = sequencertime->samplerate;
	self->vsttimeinfo->samplePos = (double)sequencertime->playcounter;
	self->vsttimeinfo->ppqPos = sequencertime->position;
	self->vsttimeinfo->tempo = sequencertime->bpm;
	self->vsttimeinfo->barStartPos = sequencertime->lastbarposition;
	// SMPTE offset(in SMPTE subframes(bits; 1 / 80 of a frame))
	self->vsttimeinfo->smpteOffset = 0; // (sequencertime->playcounter / sequencertime->samplerate)*	
	self->vsttimeinfo->samplesToNextClock = (VstInt32)sequencertime->samplestonextclock;
	self->vsttimeinfo->flags =
		kVstPpqPosValid | kVstBarsValid | kVstTempoValid | kVstTimeSigValid;
	if (sequencertime->playing) {
		self->vsttimeinfo->flags |= kVstTransportPlaying;
	}
	if (sequencertime->playstarting || sequencertime->playstopping) {
		self->vsttimeinfo->flags |= kVstTransportChanged;
	}
	self->vsttimeinfo->timeSigNumerator = (VstInt32)
		sequencertime->timesig_numerator;
	self->vsttimeinfo->timeSigDenominator = (VstInt32)
		sequencertime->timesig_denominator;
}

/* vstcallback */
VstIntPtr VSTCALLBACK hostcallback(AEffect* effect, VstInt32 opcode,
	VstInt32 index, VstIntPtr value, void* ptr, float opt)
{
	VstIntPtr result = 0;
	psy_audio_VstPlugin* self = 0;

#ifdef PSYCLE_DEBUG_VST2_OPCODES
	if (opcode != audioMasterGetTime) {
		TRACE("vst-opcode: ");
		TRACE_INT(opcode);
		TRACE("\n");
	}
#endif	
	if (effect) {
		self = (psy_audio_VstPlugin*)effect->user;
	} else if (opcode == audioMasterVersion) {
		return kVstVersion;
	} else {
		return 0;
	}
	switch (opcode)
	{
	case audioMasterVersion:
		result = kVstVersion;
		break;
	case audioMasterProcessEvents: {
		if (self) {
			// VstEvents* in <ptr>		
			struct VstEvents* v = (struct VstEvents*)ptr;
			VstInt32 n;

			for (n = 0; n < v->numEvents; ++n) {
				VstEvent* vme = (VstEvent*)(v->events[n]);
				if (vme->type == kVstMidiType) {
					VstEvent* copy;

					copy = (VstEvent*)malloc(sizeof(VstEvent));
					*copy = *vme;
					psy_audio_vstevents_append(&self->vstoutevents, copy);
				}
			}
		}
		break; }
	case audioMasterIdle:
		break;	
	case audioMasterGetCurrentProcessLevel:
		result = kVstProcessLevelUnknown;
		break;
	case audioMasterGetVendorString:
		strcpy((char*)(ptr), "Psycledelics");
		result = TRUE;
		break;
	case audioMasterGetProductString:
		strcpy((char*)(ptr), "Default Psycle VSTHost");
		result = TRUE;
		break;
	case audioMasterGetSampleRate:
		if (self) {
			result = (VstIntPtr)psy_audio_machine_samplerate(
				psy_audio_vstplugin_base(self));
		} else {
			result = 44100;
		}
		break;
	case audioMasterGetTime:
		if (self && self->vsttimeinfo) {
			update_vsttimeinfo(self);
			result = (VstIntPtr)self->vsttimeinfo;
		}
		break;
	case audioMasterOpenFileSelector:
		if (self) {
			vstplugin_onfileselect(self, (struct VstFileSelect*)ptr);
		}
	case audioMasterSizeWindow:
		if (self) {
			result = psy_audio_machine_editresize(
				psy_audio_vstplugin_base(self),
				(intptr_t)index, (intptr_t)value);
		}
		break;
	case audioMasterCanDo:
		if (result = (strcmp((char*)ptr, "sizeWindow") == 0)) {
			break;
		}		
		if (result = (strcmp((char*)ptr, "sendVstTimeInfo") == 0)) {
			break;
		}
		if (result = (strcmp((char*)ptr, "sendVstMidiEvent") == 0)) {
			break;
		}
		if (result = (strcmp((char*)ptr, "sendVstEvents") == 0)) {
			break;
		}
		break;
	case audioMasterGetAutomationState:		
		/// difference kVstAutomationOff and kVstAutomationUnsupported?
		result = kVstAutomationUnsupported;
		break;
	case audioMasterGetLanguage: {
		if (self) {
			const char* lang;

			lang = psy_audio_machine_language(psy_audio_vstplugin_base(self));
			if (strcmp(lang, "de") == 0) {
				result = kVstLangGerman;
			} else if (strcmp(lang, "en") == 0) {
				result = kVstLangEnglish;				
			} else if (strcmp(lang, "es") == 0) {
				result = kVstLangSpanish;
			} else if (strcmp(lang, "fr") == 0) {
				result = kVstLangItalian;
			} else if (strcmp(lang, "it") == 0) {
				result = kVstLangItalian;
			} else if (strcmp(lang, "jp") == 0) {
				result = kVstLangJapanese;
			} else {
				result = kVstLangSpanish;
			}
		} else {
			result = kVstLangSpanish;
		}
		break; }
	default:
		break;
	}
	return result;
}

const char* auxcolumnname(psy_audio_VstPlugin* self, uintptr_t index)
{
	if (index < 16) {
		return MIDI_CHAN_NAMES[index];
	}
	return "";
}

uintptr_t numauxcolumns(psy_audio_VstPlugin* self)
{
	return 16;
}

#endif /* PSYCLE_USE_VST2 */


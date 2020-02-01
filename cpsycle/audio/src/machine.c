// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "machine.h"
#include "machines.h"
#include "pattern.h"
#include "songio.h"
#include <string.h>
#include <operations.h>
#include "plugin_interface.h"

static psy_audio_MachineInfo const macinfo = {	
	MI_VERSION,
	0x0250,
	EFFECT | 32 | 64,
	MACHMODE_FX,
	"Machine"
		#ifndef NDEBUG
		" (debug build)"
		#endif
		,
	"Machine",
	"Psycledelics",
	"help",
	MACH_UNDEFINED
};

static psy_audio_Machine* clone(psy_audio_Machine* self) { return 0; }
static psy_audio_Buffer* mix(psy_audio_Machine*, size_t slot, uintptr_t amount,
	psy_audio_MachineSockets*, psy_audio_Machines*);
static void work(psy_audio_Machine*, psy_audio_BufferContext*);
static void work_entries(psy_audio_Machine*, psy_audio_PatternEntry*);
static void generateaudio(psy_audio_Machine* self, psy_audio_BufferContext* bc) { }
static int hostevent(psy_audio_Machine* self, int const eventNr, int const val1, float const val2) { return 0; }
static void seqtick(psy_audio_Machine* self, uintptr_t channel,
	const psy_audio_PatternEvent* event) { }
static void stop(psy_audio_Machine* self) { }
static void sequencertick(psy_audio_Machine* self) { }
static psy_List* sequencerinsert(psy_audio_Machine* self, psy_List* events) { return 0; }
static void sequencerlinetick(psy_audio_Machine* self) { }
static const psy_audio_MachineInfo* info(psy_audio_Machine* self) { return &macinfo; }
static int parametertype(psy_audio_Machine* self, uintptr_t param) { return MPF_STATE; }
static void parameterrange(psy_audio_Machine* self, uintptr_t numparam, int* minval, int* maxval)
{
	*minval = 0;
	*maxval = 0;
}
static void parametertweak(psy_audio_Machine* self, uintptr_t param, float val) { }
static void patterntweak(psy_audio_Machine* self, uintptr_t param, float val)
{
	psy_audio_machine_parametertweak(self, param, val);
}
static int parameterlabel(psy_audio_Machine* self, char* txt, uintptr_t param) { txt[0] = '\0'; return 0; }
static int parametername(psy_audio_Machine* self, char* txt, uintptr_t param) { txt[0] = '\0'; return 0; }
static int describevalue(psy_audio_Machine* self, char* txt, uintptr_t param, int value) { return 0; }
static float parametervalue(psy_audio_Machine* self, uintptr_t param) { return 0.f; }
static void setpanning(psy_audio_Machine* self, psy_dsp_amp_t panning) { }
static psy_dsp_amp_t panning(psy_audio_Machine* self) { return (psy_dsp_amp_t) 0.5f; }
static void mute(psy_audio_Machine* self) { }
static void unmute(psy_audio_Machine* self) { }
static int muted(psy_audio_Machine* self) { return 0; }
static void bypass(psy_audio_Machine* self) { }
static void unbypass(psy_audio_Machine* self) { }
static int bypassed(psy_audio_Machine* self) { return 0; }
static void dispose(psy_audio_Machine*);
static int mode(psy_audio_Machine*);
static uintptr_t numinputs(psy_audio_Machine* self) { return 0; }
static uintptr_t numoutputs(psy_audio_Machine* self) { return 0; }	
static void setcallback(psy_audio_Machine* self, MachineCallback callback) { self->callback = callback; }
static void updatesamplerate(psy_audio_Machine* self, unsigned int samplerate) { }
static void loadspecific(psy_audio_Machine*, struct psy_audio_SongFile*,
	uintptr_t slot);
static void savespecific(psy_audio_Machine*, struct psy_audio_SongFile*,
	uintptr_t slot);
static void addsamples(psy_audio_Buffer* dst, psy_audio_Buffer* source,
	uintptr_t numsamples, psy_dsp_amp_t vol);
static uintptr_t numparameters(psy_audio_Machine* self) { return 0; }
static unsigned int numparametercols(psy_audio_Machine* self) { return 0; }
static int paramviewoptions(psy_audio_Machine* self) { return 0; }
static uintptr_t slot(psy_audio_Machine* self) { return NOMACHINE_INDEX; }
static void setslot(psy_audio_Machine* self, uintptr_t slot) { }
static int haseditor(psy_audio_Machine* self) { return 0; }
static void seteditorhandle(psy_audio_Machine* self, void* handle) { }
static void editorsize(psy_audio_Machine* self, int* width, int* height)
{ 
	*width = 0;
	*height = 0;
}
static void editoridle(psy_audio_Machine* self) { }
static const char* editname(psy_audio_Machine* self) { return ""; }
static void seteditname(psy_audio_Machine* self, const char* name) { }
static psy_audio_Buffer* buffermemory(psy_audio_Machine* self) { return 0; }
static uintptr_t buffermemorysize(psy_audio_Machine* self) { return 0; }
static void setbuffermemorysize(psy_audio_Machine* self, uintptr_t size) { }
static psy_dsp_amp_range_t amprange(psy_audio_Machine* self)
{
	return PSY_DSP_AMP_RANGE_NATIVE;
}

/// machinecallback
static unsigned int samplerate(psy_audio_Machine* self) { return self->callback.samplerate(self->callback.context); }
static unsigned int bpm(psy_audio_Machine* self) { return self->callback.bpm(self->callback.context); }
static psy_dsp_beat_t beatspersample(psy_audio_Machine* self) { return self->callback.beatspersample(self->callback.context); }
static psy_dsp_beat_t currbeatsperline(psy_audio_Machine* self) { return self->callback.currbeatsperline(self->callback.context); }
static struct psy_audio_Samples* samples(psy_audio_Machine* self) { return self->callback.samples(self->callback.context); }
static struct psy_audio_Machines* machines(psy_audio_Machine* self) { return self->callback.machines(self->callback.context); }
static struct psy_audio_Instruments* instruments(psy_audio_Machine* self) { return self->callback.instruments(self->callback.context); }
static void output(psy_audio_Machine* self, const char* text) { self->callback.output(self->callback.context, text); }

static MachineVtable vtable;
static int vtable_initialized = 0;

static void vtable_init(void)
{
	if (!vtable_initialized) {
		vtable.clone = clone;
		vtable.dispose = machine_dispose;
		vtable.mix = mix;
		vtable.work = work;
		vtable.mode = mode;
		vtable.hostevent = hostevent;
		vtable.seqtick = seqtick;
		vtable.stop = stop;
		vtable.sequencertick = sequencertick;
		vtable.sequencerlinetick = sequencerlinetick;
		vtable.sequencerinsert = sequencerinsert;
		vtable.info = info;
		vtable.parametertweak = parametertweak;
		vtable.patterntweak = patterntweak;
		vtable.describevalue = describevalue;
		vtable.parametervalue = parametervalue;
		vtable.setpanning = setpanning;
		vtable.panning = panning;
		vtable.mute = mute;
		vtable.unmute = unmute;
		vtable.muted = muted;
		vtable.bypass = bypass;
		vtable.unbypass = unbypass;
		vtable.bypassed = bypassed;
		vtable.generateaudio = generateaudio;
		vtable.numinputs = numinputs;
		vtable.numoutputs = numoutputs;	
		vtable.parameterrange = parameterrange;
		vtable.parametertype = parametertype;
		vtable.numparameters = numparameters;
		vtable.numparametercols = numparametercols;
		vtable.paramviewoptions = paramviewoptions;
		vtable.parameterlabel = parameterlabel;
		vtable.parametername = parametername;
		vtable.setcallback = setcallback;
		vtable.updatesamplerate = updatesamplerate;
		vtable.loadspecific = loadspecific;
		vtable.savespecific = savespecific;
		vtable.bpm = bpm;
		vtable.beatspersample = beatspersample;
		vtable.currbeatsperline = currbeatsperline;
		vtable.samplerate = samplerate;
		vtable.instruments = instruments;	
		vtable.samples = samples;
		vtable.machines = machines;
		vtable.output = output;
		vtable.slot = slot;
		vtable.setslot = setslot;		
		vtable.haseditor = haseditor;
		vtable.seteditorhandle = seteditorhandle;
		vtable.editorsize = editorsize;
		vtable.editoridle = editoridle;
		vtable.editname = editname;
		vtable.seteditname = seteditname;
		vtable.buffermemory = buffermemory;
		vtable.buffermemorysize = buffermemorysize;
		vtable.setbuffermemorysize = setbuffermemorysize;
		vtable.amprange = amprange;
		vtable_initialized = 1;
	}
}

void machine_init(psy_audio_Machine* self, MachineCallback callback)
{		
	memset(self, 0, sizeof(psy_audio_Machine));
	vtable_init();
	self->vtable = &vtable;
	self->callback = callback;
	psy_signal_init(&self->signal_worked);
}

void machine_dispose(psy_audio_Machine* self)
{
	psy_signal_dispose(&self->signal_worked);
}

void work(psy_audio_Machine* self, psy_audio_BufferContext* bc)
{			
	psy_List* p;
	uintptr_t amount = bc->numsamples;
	uintptr_t pos = 0;
	uintptr_t last = 0;
	
	for (p = bc->events; p != 0; p = p->next) {					
		int numworksamples;

		psy_audio_PatternEntry* entry = (psy_audio_PatternEntry*)p->entry;
		assert((uintptr_t)(entry->delta) >= last);
		numworksamples = (uintptr_t) entry->delta - pos;
		last = (uintptr_t) entry->delta;
		if (numworksamples > 0) {				
			int restorenumsamples = bc->numsamples;
		
			if (bc->input) {
				psy_audio_buffer_setoffset(bc->input, pos);
			}
			if (bc->output) {
				psy_audio_buffer_setoffset(bc->output, pos);
			}
			bc->numsamples = numworksamples;
			if (!psy_audio_machine_bypassed(self)) {
				psy_audio_machine_generateaudio(self, bc);
			}
			amount -= numworksamples;
			bc->numsamples = restorenumsamples;
		}
		work_entries(self, entry);
		pos = (uintptr_t) entry->delta;	
	}
	if (amount > 0) {
		int restorenumsamples = bc->numsamples;
		if (bc->input) {
			psy_audio_buffer_setoffset(bc->input, pos);
		}
		if (bc->output) {
			psy_audio_buffer_setoffset(bc->output, pos);
		}
		bc->numsamples = amount;
		if (!psy_audio_machine_bypassed(self)) {
			psy_audio_machine_generateaudio(self, bc);
		}
		bc->numsamples = restorenumsamples;
	}
	if (bc->input) {
		psy_audio_buffer_setoffset(bc->input, 0);
	}
	if (bc->output) {
		psy_audio_Buffer* memory;

		psy_audio_buffer_setoffset(bc->output, 0);		
		memory = psy_audio_machine_buffermemory(self);
		if (memory) {			
			psy_audio_buffer_insertsamples(memory, bc->output,
				psy_audio_machine_buffermemorysize(self), bc->numsamples);
		}
	}
}

void work_entries(psy_audio_Machine* self, psy_audio_PatternEntry* entry)
{
	psy_List* p;

	for (p = entry->events; p != 0; p = p->next) {
		psy_audio_PatternEvent* ev;

		ev = (psy_audio_PatternEvent*) p->entry;		
		if (ev->note == NOTECOMMANDS_MIDICC) {
			// only native plugins, vst plugins handle tweaks in vstplugin_work
			psy_audio_Machine* dst;			

			dst = machines_at(psy_audio_machine_machines(self), ev->mach);
			if (dst) {				
				psy_audio_machine_patterntweak(self, ev->inst, 
					machine_patternvalue_normed(self, ev->inst,
						psy_audio_patternevent_tweakvalue(ev)));
			}
		} else
		if (ev->note == NOTECOMMANDS_TWEAK) {
			if (ev->inst < psy_audio_machine_numparameters(self)) {				
				psy_audio_machine_patterntweak(self, ev->inst, 
					machine_patternvalue_normed(self, ev->inst,
						psy_audio_patternevent_tweakvalue(ev)));
			}
		} else {
			if (ev->cmd == SET_PANNING) {
				psy_audio_machine_setpanning(self, ev->parameter / 255.f);
			}
			psy_audio_machine_seqtick(self, entry->track, ev);
		}
	}
}

static int mode(psy_audio_Machine* self)
{ 
	const psy_audio_MachineInfo* info;

	info = psy_audio_machine_info(self);
	return (!info) ? MACHMODE_FX : info->mode;
}

int machine_supports(psy_audio_Machine* self, int option)
{
	if (psy_audio_machine_info(self)) {
		return (psy_audio_machine_info(self)->Flags & option) == option;
	}
	return 0;
}

psy_audio_Buffer* mix(psy_audio_Machine* self,
	size_t slot,
	uintptr_t amount,
	psy_audio_MachineSockets* connected_machine_sockets,
	psy_audio_Machines* machines)
{			
	psy_audio_Buffer* output;

	output = machines_outputs(machines, slot);
	if (output) {
		psy_audio_buffer_clearsamples(output, amount);
		if (connected_machine_sockets) {
			WireSocket* input_socket;
			
			for (input_socket = connected_machine_sockets->inputs;
					input_socket != 0; input_socket = input_socket->next) {
				psy_audio_WireSocketEntry* source = (psy_audio_WireSocketEntry*)
					input_socket->entry;
				addsamples(output, machines_outputs(machines, source->slot),
					amount, source->volume);				
			}							
		}
	}
	return output;
}

void addsamples(psy_audio_Buffer* dst, psy_audio_Buffer* source, uintptr_t numsamples,
	psy_dsp_amp_t vol)
{
	unsigned int channel;

	if (source) {
		psy_audio_buffer_scale(dst, source->range, numsamples);
		for (channel = 0; channel < source->numchannels && 
			channel < dst->numchannels; ++channel) {
				dsp.add(
					source->samples[channel],
					dst->samples[channel],
					numsamples,
					vol);
				dsp.erase_all_nans_infinities_and_denormals(
					dst->samples[channel], numsamples);					
		}
	}
}

void loadspecific(psy_audio_Machine* self, struct psy_audio_SongFile* songfile,
	uintptr_t slot)
{
	uint32_t size;
	uint32_t numparams;
	uint32_t i;
	
	psyfile_read(songfile->file, &size, sizeof(size));	
	psyfile_read(songfile->file, &numparams, sizeof(numparams));
	for (i = 0; i < numparams; ++i) {
		uint32_t temp;

		psyfile_read(songfile->file, &temp, sizeof(temp));
		psy_audio_machine_parametertweak(self, i,
			machine_parametervalue_normed(self, i, temp));
	}
	psyfile_skip(songfile->file, size - sizeof(numparams) - (numparams * sizeof(uint32_t)));
}

void savespecific(psy_audio_Machine* self, struct psy_audio_SongFile* songfile,
	uintptr_t slot)
{		
	uint32_t numparams;
	uint32_t size;
	uint32_t i;
		
	numparams = (uint32_t) psy_audio_machine_numparameters(self);
	size = sizeof(numparams) + (numparams * sizeof(numparams));		
	psyfile_write(songfile->file, &size, sizeof(size));
	psyfile_write(songfile->file, &numparams, sizeof(numparams));
	for (i = 0; i < numparams; ++i) {		
		psyfile_write_int32(songfile->file, 
			(int32_t) machine_parametervalue_scaled(self, i,
				psy_audio_machine_parametervalue(self, i)));
	}
}

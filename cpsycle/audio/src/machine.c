// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "machine.h"
#include "machines.h"
#include "pattern.h"
#include "songio.h"
#include <string.h>
#include <operations.h>

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
static void parametertweak(psy_audio_Machine* self, uintptr_t param, int val) { }
static void patterntweak(psy_audio_Machine* self, uintptr_t param, int val)
{
	machine_parametertweak(self, param, val);
}
static int parameterlabel(psy_audio_Machine* self, char* txt, uintptr_t param) { txt[0] = '\0'; return 0; }
static int parametername(psy_audio_Machine* self, char* txt, uintptr_t param) { txt[0] = '\0'; return 0; }
static int describevalue(psy_audio_Machine* self, char* txt, uintptr_t param, int value) { return 0; }
static int parametervalue(psy_audio_Machine* self, uintptr_t param) { return 0; }
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
	
	for (p = bc->events; p != 0; p = p->next) {					
		int numworksamples;

		psy_audio_PatternEntry* entry = (psy_audio_PatternEntry*)p->entry;		
		numworksamples = (uintptr_t) entry->delta - pos;
		if (numworksamples > 0) {				
			int restorenumsamples = bc->numsamples;
		
			if (bc->input) {
				psy_audio_buffer_setoffset(bc->input, pos);
			}
			if (bc->output) {
				psy_audio_buffer_setoffset(bc->output, pos);
			}
			bc->numsamples = numworksamples;
			if (!machine_bypassed(self)) {
				machine_generateaudio(self, bc);
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
		if (!machine_bypassed(self)) {
			machine_generateaudio(self, bc);
		}
		bc->numsamples = restorenumsamples;
	}
	if (bc->input) {
		psy_audio_buffer_setoffset(bc->input, 0);
	}
	if (bc->output) {
		psy_audio_Buffer* memory;

		psy_audio_buffer_setoffset(bc->output, 0);		
		memory = machine_buffermemory(self);
		if (memory) {			
			psy_audio_buffer_insertsamples(memory, bc->output,
				machine_buffermemorysize(self), bc->numsamples);
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
			// only native plugins, vst handle it in vstplugin_work
			psy_audio_Machine* dst;
			int value;

			dst = machines_at(machine_machines(self), ev->mach);
			if (dst) {
				value = (ev->cmd << 8) + ev->parameter;
				machine_patterntweak(dst, ev->inst, value);
			}
		} else
		if (ev->note == NOTECOMMANDS_TWEAK) {
			int value;
			
			value = (ev->cmd << 8) + ev->parameter;
			machine_patterntweak(self, ev->inst, value);
		} else {
			if (ev->cmd == SET_PANNING) {
				machine_setpanning(self, ev->parameter / 255.f);
			}
			machine_seqtick(self, entry->track, ev);
		}
	}
}

static int mode(psy_audio_Machine* self)
{ 
	const psy_audio_MachineInfo* info;

	info = machine_info(self);
	return (!info) ? MACHMODE_FX : info->mode;
}

int machine_supports(psy_audio_Machine* self, int option)
{
	if (machine_info(self)) {
		return (machine_info(self)->Flags & option) == option;
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
	uint32_t count;
	uint32_t i;

	psyfile_read(songfile->file, &size, sizeof(size)); // size of this part params to load	
	psyfile_read(songfile->file, &count, sizeof(count)); // num params to load
	for (i = 0; i < count; i++) {
		uint32_t temp;
		psyfile_read(songfile->file, &temp, sizeof(temp));
		machine_parametertweak(self, i, temp);
	}
	psyfile_skip(songfile->file, size - sizeof(count) - (count * sizeof(int)));
}

void savespecific(psy_audio_Machine* self, struct psy_audio_SongFile* songfile,
	uintptr_t slot)
{
	uint32_t i;
	uint32_t count;
	uint32_t size;
	
	count = (uint32_t) machine_numparameters(self);
	size = sizeof(count) + (count * sizeof(int32_t));
	psyfile_write(songfile->file, &size, sizeof(size));
	psyfile_write(songfile->file, &count, sizeof(count));
	for (i = 0; i < count; ++i) {		
		psyfile_write_int32(songfile->file, 
			(int32_t) machine_parametervalue(self, i));
	}
}

// virtual calls
const psy_audio_MachineInfo* machine_info(psy_audio_Machine* self)
{
	return self->vtable->info(self);
}

psy_audio_Buffer* machine_mix(psy_audio_Machine* self, uintptr_t slot,
	uintptr_t amount, psy_audio_MachineSockets* sockets,
	struct psy_audio_Machines* machines)
{
	return self->vtable->mix(self, slot, amount, sockets, machines);
}

void machine_work(psy_audio_Machine* self, psy_audio_BufferContext* bc)
{
	self->vtable->work(self, bc);
}

void machine_generateaudio(psy_audio_Machine* self,
	psy_audio_BufferContext* bc)
{
	self->vtable->generateaudio(self, bc);
}

void machine_seqtick(psy_audio_Machine* self, uintptr_t channel,
	const psy_audio_PatternEvent* event)
{
	self->vtable->seqtick(self, channel, event);
}

void machine_sequencertick(psy_audio_Machine* self)
{
	self->vtable->sequencertick(self);
}

void machine_sequencerlinetick(psy_audio_Machine* self)
{
	self->vtable->sequencerlinetick(self);
}

psy_List* machine_sequencerinsert(psy_audio_Machine* self, psy_List* events)
{
	return self->vtable->sequencerinsert(self, events);
}

int machine_mode(psy_audio_Machine* self)
{
	return self->vtable->mode(self);
}

int machine_describevalue(psy_audio_Machine* self, char* txt, uintptr_t param,
	int value)
{
	return self->vtable->describevalue(self, txt, param, value);
}

void machine_parametertweak(psy_audio_Machine* self, uintptr_t param, int value)
{
	self->vtable->parametertweak(self, param, value);
}

void machine_patterntweak(psy_audio_Machine* self, uintptr_t param, int value)
{
	self->vtable->patterntweak(self, param, value);
}

int machine_parametervalue(psy_audio_Machine* self, uintptr_t param)
{
	return self->vtable->parametervalue(self, param);
}

void machine_parameterrange(struct psy_audio_Machine* self,
	uintptr_t param, int* minval, int* maxval)
{
	self->vtable->parameterrange(self, param, minval, maxval);
}

uintptr_t machine_numparameters(psy_audio_Machine* self)
{
	return self->vtable->numparameters(self);
}

int machine_parameterlabel(psy_audio_Machine* self, char* txt, uintptr_t param)
{
	return self->vtable->parameterlabel(self, txt, param);
}

int machine_parametername(psy_audio_Machine* self, char* txt, uintptr_t param)
{
	return self->vtable->parametername(self, txt, param);
}

uintptr_t machine_numinputs(psy_audio_Machine* self)
{
	return self->vtable->numinputs(self);
}

uintptr_t machine_numoutputs(psy_audio_Machine* self)
{
	return self->vtable->numoutputs(self);
}

uintptr_t machine_slot(psy_audio_Machine* self)
{
	return self->vtable->slot(self);
}

void machine_setcallback(psy_audio_Machine* self, MachineCallback callback)
{
	self->vtable->setcallback(self, callback);
}

void machine_setpanning(psy_audio_Machine* self, psy_dsp_amp_t pan)
{
	self->vtable->setpanning(self, pan);
}

psy_dsp_amp_t machine_panning(psy_audio_Machine* self)
{
	return self->vtable->panning(self);
}

void machine_mute(psy_audio_Machine* self)
{
	self->vtable->mute(self);
}

void machine_unmute(psy_audio_Machine* self)
{
	self->vtable->unmute(self);
}

int machine_muted(psy_audio_Machine* self)
{
	return self->vtable->muted(self);
}

void machine_bypass(psy_audio_Machine* self)
{
	self->vtable->bypass(self);
}

void machine_unbypass(psy_audio_Machine* self)
{
	self->vtable->unbypass(self);
}

int machine_bypassed(psy_audio_Machine* self)
{
	return self->vtable->bypassed(self);
}

int machine_haseditor(psy_audio_Machine* self)
{
	return self->vtable->haseditor(self);
}

void machine_seteditorhandle(psy_audio_Machine* self, void* handle)
{
	self->vtable->seteditorhandle(self, handle);
}

void machine_editorsize(psy_audio_Machine* self, int* width, int* height)
{
	self->vtable->editorsize(self, width, height);
}

void machine_editoridle(psy_audio_Machine* self)
{
	self->vtable->editoridle(self);
}

void machine_seteditname(psy_audio_Machine* self, const char* name)
{
	self->vtable->seteditname(self, name);
}

const char* machine_editname(psy_audio_Machine* self)
{
	return self->vtable->editname(self);
}

unsigned int machine_bpm(psy_audio_Machine* self)
{
	return self->vtable->bpm(self);
}

psy_dsp_beat_t machine_beatspersample(psy_audio_Machine* self)
{
	return self->vtable->beatspersample(self);
}

psy_dsp_beat_t machine_currbeatsperline(psy_audio_Machine* self)
{
	return self->vtable->currbeatsperline(self);
}

unsigned int machine_samplerate(psy_audio_Machine* self)
{
	return self->vtable->samplerate(self);
}

psy_audio_Samples* machine_samples(psy_audio_Machine* self)
{
	return self->vtable->samples(self);
}

psy_audio_Machines* machine_machines(psy_audio_Machine* self)
{
	return self->vtable->machines(self);
}

psy_audio_Instruments* machine_instruments(psy_audio_Machine* self)
{
	return self->vtable->instruments(self);
}

void machine_output(psy_audio_Machine* self, const char* text)
{
	self->vtable->output(self, text);
}

psy_audio_Buffer* machine_buffermemory(psy_audio_Machine* self)
{
	return self->vtable->buffermemory(self);
}

uintptr_t machine_buffermemorysize(psy_audio_Machine* self)
{
	return self->vtable->buffermemorysize(self);
}

psy_dsp_amp_range_t machine_amprange(psy_audio_Machine* self)
{
	return self->vtable->amprange(self);
}

void machine_loadspecific(psy_audio_Machine* self,
	struct psy_audio_SongFile* songfile, uintptr_t slot)
{
	self->vtable->loadspecific(self, songfile, slot);
}

void machine_savespecific(psy_audio_Machine* self,
	struct psy_audio_SongFile* songfile, uintptr_t slot)
{
	self->vtable->savespecific(self, songfile, slot);
}
// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

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
static psy_audio_Buffer* mix(psy_audio_Machine*, size_t slot, unsigned int amount, psy_audio_MachineSockets*, psy_audio_Machines*);
static void work(psy_audio_Machine*, psy_audio_BufferContext*);
static void generateaudio(psy_audio_Machine* self, psy_audio_BufferContext* bc) { }
static int hostevent(psy_audio_Machine* self, int const eventNr, int const val1, float const val2) { return 0; }
static void seqtick(psy_audio_Machine* self, uintptr_t channel,
	const psy_audio_PatternEvent* event) { }
static void stop(psy_audio_Machine* self) { }
static void sequencertick(psy_audio_Machine* self) { }
static psy_List* sequencerinsert(psy_audio_Machine* self, psy_List* events) { return 0; }
static void sequencerlinetick(psy_audio_Machine* self) { }
static const psy_audio_MachineInfo* info(psy_audio_Machine* self) { return &macinfo; }
static int parametertype(psy_audio_Machine* self, int param) { return MPF_STATE; }
static void parameterrange(psy_audio_Machine* self, int numparam, int* minval, int* maxval)
{
	*minval = 0;
	*maxval = 0;
}
static void parametertweak(psy_audio_Machine* self, int par, int val) { }
static void patterntweak(psy_audio_Machine* self, int par, int val)
{
	self->vtable->parametertweak(self, par, val);
}
static int parameterlabel(psy_audio_Machine* self, char* txt, int param) { txt[0] = '\0'; return 0; }
static int parametername(psy_audio_Machine* self, char* txt, int param) { txt[0] = '\0'; return 0; }
static int describevalue(psy_audio_Machine* self, char* txt, int param, int value) { return 0; }
static int parametervalue(psy_audio_Machine* self, int const param) { return 0; }
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
static void loadspecific(psy_audio_Machine*, struct psy_audio_SongFile*, unsigned int slot);
static void savespecific(psy_audio_Machine*, struct psy_audio_SongFile*, unsigned int slot);
static void addsamples(psy_audio_Buffer* dst, psy_audio_Buffer* source, unsigned int numsamples,
	psy_dsp_amp_t vol);
static unsigned int numparameters(psy_audio_Machine* self) { return 0; }
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

/// machinecallback
static unsigned int samplerate(psy_audio_Machine* self) { return self->callback.samplerate(self->callback.context); }
static unsigned int bpm(psy_audio_Machine* self) { return self->callback.bpm(self->callback.context); }
static struct psy_audio_Samples* samples(psy_audio_Machine* self) { return self->callback.samples(self->callback.context); }
static struct psy_audio_Machines* machines(psy_audio_Machine* self) { return self->callback.machines(self->callback.context); }
static struct psy_audio_Instruments* instruments(psy_audio_Machine* self) { return self->callback.instruments(self->callback.context); }

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
		vtable.samplerate = samplerate;
		vtable.instruments = instruments;	
		vtable.samples = samples;
		vtable.machines = machines;		
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
	unsigned int amount = bc->numsamples;
	unsigned int pos = 0;

	for (p = bc->events; p != 0; p = p->next) {					
		int numworksamples;

		psy_audio_PatternEntry* entry = (psy_audio_PatternEntry*)p->entry;		
		numworksamples = (unsigned int)entry->delta - pos;		
		if (numworksamples > 0) {				
			int restorenumsamples = bc->numsamples;
		
			if (bc->input) {
				buffer_setoffset(bc->input, pos);
			}
			if (bc->output) {
				buffer_setoffset(bc->output, pos);
			}
			bc->numsamples = numworksamples;
			if (!self->vtable->bypassed(self)) {
				self->vtable->generateaudio(self, bc);
			}
			amount -= numworksamples;
			bc->numsamples = restorenumsamples;
		}
		if (entry->event.cmd == SET_PANNING) {
			self->vtable->setpanning(self, 
				entry->event.parameter / 255.f);
		} else
		if (entry->event.note == NOTECOMMANDS_MIDICC) {
			psy_audio_Machine* dst;
			int value;

			dst = machines_at(self->vtable->machines(self), entry->event.mach);
			if (dst) {
				value = (entry->event.cmd << 8) + entry->event.parameter;
				dst->vtable->patterntweak(dst, entry->event.inst, value);
			}
		} else
		if (entry->event.note == NOTECOMMANDS_TWEAK) {
			int value;
			
			value = (entry->event.cmd << 8) + entry->event.parameter;
			self->vtable->patterntweak(self, entry->event.inst, value);
		} else {
			self->vtable->seqtick(self, entry->track, &entry->event);
		}
		pos = (unsigned int)entry->delta;	
	}
	if (amount > 0 && self->vtable->generateaudio) {
		int restorenumsamples = bc->numsamples;
		if (bc->input) {
			buffer_setoffset(bc->input, pos);
		}
		if (bc->output) {
			buffer_setoffset(bc->output, pos);
		}
		bc->numsamples = amount;
		if (!self->vtable->bypassed(self)) {
			self->vtable->generateaudio(self, bc);
		}
		bc->numsamples = restorenumsamples;
	}
	if (bc->input) {
		buffer_setoffset(bc->input, 0);
	}
	if (bc->output) {
		psy_audio_Buffer* memory;

		buffer_setoffset(bc->output, 0);		
		memory = self->vtable->buffermemory(self);
		if (memory) {			
			buffer_insertsamples(memory, bc->output, 
				self->vtable->buffermemorysize(self),
				bc->numsamples);
		}
	}
}

static int mode(psy_audio_Machine* self)
{ 
	const psy_audio_MachineInfo* info;

	info = self->vtable->info(self);
	return (!info) ? MACHMODE_FX : info->mode;
}

int machine_supports(psy_audio_Machine* self, int option)
{
	if (self->vtable->info(self)) {
		return (self->vtable->info(self)->Flags & option) == option;
	}
	return 0;
}

psy_audio_Buffer* mix(psy_audio_Machine* self, size_t slot, unsigned int amount, psy_audio_MachineSockets* connected_machine_sockets, psy_audio_Machines* machines)
{			
	psy_audio_Buffer* output;

	output = machines_outputs(machines, slot);
	if (output) {
		buffer_clearsamples(output, amount);
		if (connected_machine_sockets) {
			WireSocket* input_socket;
			
			for (input_socket = connected_machine_sockets->inputs; input_socket != 0; input_socket = input_socket->next) {
				psy_audio_WireSocketEntry* source = (psy_audio_WireSocketEntry*) input_socket->entry;
				addsamples(output, machines_outputs(machines, source->slot), amount,
					source->volume);				
			}							
		}
	}
	return output;
}

void addsamples(psy_audio_Buffer* dst, psy_audio_Buffer* source, unsigned int numsamples,
	psy_dsp_amp_t vol)
{
	unsigned int channel;

	if (source) {
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

void loadspecific(psy_audio_Machine* self, struct psy_audio_SongFile* songfile, unsigned int slot)
{
	unsigned int size;
	unsigned int count;
	unsigned int i;

	psyfile_read(songfile->file, &size, sizeof(size)); // size of this part params to load	
	psyfile_read(songfile->file, &count, sizeof(count)); // num params to load
	for (i = 0; i < count; i++) {
		int temp;
		psyfile_read(songfile->file, &temp, sizeof(temp));
		self->vtable->parametertweak(self, i, temp);
	}
	psyfile_skip(songfile->file, size - sizeof(count) - (count * sizeof(int)));
}

void savespecific(psy_audio_Machine* self, struct psy_audio_SongFile* songfile, unsigned int slot)
{
	unsigned int i;
	unsigned count = self->vtable->numparameters(self);
	unsigned  size = sizeof(count)+(count*sizeof(int));
	psyfile_write(songfile->file, &size, sizeof(size));
	psyfile_write(songfile->file, &count, sizeof(count));
	for(i = 0; i < count; ++i) {
		int temp;
		
		temp = self->vtable->parametervalue(self, i);
		psyfile_write(songfile->file, &temp, sizeof(temp));
	}
}

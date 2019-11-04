// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "machine.h"
#include "machines.h"
#include "pattern.h"
#include "songio.h"
#include <string.h>
#include <operations.h>

static MachineInfo const macinfo = {	
	MI_VERSION,
	0x0250,
	EFFECT | 32 | 64,
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

static Machine* clone(Machine* self) { return 0; }
static Buffer* mix(Machine*, size_t slot, unsigned int amount, MachineSockets*, Machines*);
static void work(Machine*, BufferContext*);
static void generateaudio(Machine* self, BufferContext* bc) { }
static int hostevent(Machine* self, int const eventNr, int const val1, float const val2) { return 0; }
static void seqtick(Machine* self, int channel, const PatternEvent* event) { }
static void sequencertick(Machine* self) { }
static List* sequencerinsert(Machine* self, List* events) { return 0; }
static void sequencerlinetick(Machine* self) { }
static const MachineInfo* info(Machine* self) { return &macinfo; }
static int parametertype(Machine* self, int param) { return MPF_STATE; }
static void parameterrange(Machine* self, int numparam, int* minval, int* maxval)
{
	*minval = 0;
	*maxval = 0;
}
static void parametertweak(Machine* self, int par, int val) { }
static void patterntweak(Machine* self, int par, int val)
{
	self->parametertweak(self, par, val);
}
static int parameterlabel(Machine* self, char* txt, int param) { txt[0] = '\0'; return 0; }
static int parametername(Machine* self, char* txt, int param) { txt[0] = '\0'; return 0; }
static int describevalue(Machine* self, char* txt, int param, int value) { return 0; }
static int parametervalue(Machine* self, int const param) { return 0; }
static void setpanning(Machine*, amp_t);
static amp_t panning(Machine*);
static void dispose(Machine*);
static int mode(Machine*);
static unsigned int numinputs(Machine* self) { return 0; }
static unsigned int numoutputs(Machine* self) { return 0; }	
static void setcallback(Machine* self, MachineCallback callback) { self->callback = callback; }
static void updatesamplerate(Machine* self, unsigned int samplerate) { }
static void loadspecific(Machine*, struct SongFile*, unsigned int slot);
static void savespecific(Machine*, struct SongFile*, unsigned int slot);
static void addsamples(Buffer* dst, Buffer* source, unsigned int numsamples, amp_t vol);
static unsigned int numparameters(Machine* self) { return 0; }
static unsigned int numparametercols(Machine* self) { return 0; }
static int paramviewoptions(Machine* self) { return 0; }
static unsigned int slot(Machine* self) { return -1; }
static void setslot(Machine* self, int slot) { }
static int haseditor(Machine* self) { return 0; }
static void seteditorhandle(Machine* self, void* handle) { }
static void editorsize(Machine* self, int* width, int* height)
{ 
	*width = 0;
	*height = 0;
}
void editoridle(Machine* self) { }

/// machinecallback
static unsigned int samplerate(Machine* self) { return self->callback.samplerate(self->callback.context); }
static unsigned int bpm(Machine* self) { return self->callback.bpm(self->callback.context); }
static struct Samples* samples(Machine* self) { return self->callback.samples(self->callback.context); }
static struct Machines* machines(Machine* self) { return self->callback.machines(self->callback.context); }
static struct Instruments* instruments(Machine* self) { return self->callback.instruments(self->callback.context); }


void machine_init(Machine* self, MachineCallback callback)
{		
	memset(self, 0, sizeof(Machine));
	self->clone = clone;
	self->dispose = machine_dispose;
	self->mix = mix;
	self->work = work;
	self->mode = mode;
	self->hostevent = hostevent;
	self->seqtick = seqtick;
	self->sequencertick = sequencertick;
	self->sequencerlinetick = sequencerlinetick;
	self->sequencerinsert = sequencerinsert;
	self->info = info;
	self->parametertweak = parametertweak;
	self->patterntweak = patterntweak;
	self->describevalue = describevalue;
	self->parametervalue = parametervalue;
	self->setpanning = setpanning;
	self->panning = panning;
	self->generateaudio = generateaudio;
	self->numinputs = numinputs;
	self->numoutputs = numoutputs;	
	self->parameterrange = parameterrange;
	self->parametertype = parametertype;
	self->numparameters = numparameters;
	self->numparametercols = numparametercols;
	self->paramviewoptions = paramviewoptions;
	self->parameterlabel = parameterlabel;
	self->parametername = parametername;
	self->setcallback = setcallback;
	self->updatesamplerate = updatesamplerate;
	self->loadspecific = loadspecific;
	self->savespecific = savespecific;
	self->bpm = bpm;
	self->samplerate = samplerate;
	self->instruments = instruments;	
	self->samples = samples;
	self->machines = machines;
	self->callback = callback;
	self->slot = slot;
	self->setslot = setslot;
	self->bypass = 0;
	self->mute = 0;
	self->pan = 0.5f;
	self->ismixersend = 0;	
	self->haseditor = haseditor;
	self->seteditorhandle = seteditorhandle;
	self->editorsize = editorsize;
	self->editoridle = editoridle;
	signal_init(&self->signal_worked);
}

void machine_dispose(Machine* self)
{
	signal_dispose(&self->signal_worked);
}

void work(Machine* self, BufferContext* bc)
{			
	List* p;
	unsigned int amount = bc->numsamples;
	unsigned int pos = 0;

	for (p = bc->events; p != 0; p = p->next) {					
		int numworksamples;

		PatternEntry* entry = (PatternEntry*)p->entry;		
		numworksamples = (unsigned int)entry->delta - pos;		
		if (numworksamples > 0) {				
			int restorenumsamples = bc->numsamples;
			
			buffer_setoffset(bc->input, pos);
			buffer_setoffset(bc->output, pos);			
			bc->numsamples = numworksamples;
			self->generateaudio(self, bc);
			amount -= numworksamples;
			bc->numsamples = restorenumsamples;
		}
		if (entry->event.cmd == SET_PANNING) {
			self->setpanning(self, 
				entry->event.parameter / 255.f);
		} else
		if (entry->event.note == NOTECOMMANDS_TWEAK) {
			int value;
			
			value = (entry->event.cmd << 8) + entry->event.parameter;
			self->patterntweak(self, entry->event.inst, value);
		} else {
			self->seqtick(self, entry->track, &entry->event);
		}
		pos = (unsigned int)entry->delta;	
	}
	if (amount > 0 && self->generateaudio) {
		int restorenumsamples = bc->numsamples;
		buffer_setoffset(bc->input, pos);
		buffer_setoffset(bc->output, pos);			
		bc->numsamples = amount;
		self->generateaudio(self, bc);
		bc->numsamples = restorenumsamples;
	}
	buffer_setoffset(bc->input, 0);
	buffer_setoffset(bc->output, 0);			
}

static int mode(Machine* self)
{ 
	const MachineInfo* info;

	info = self->info(self);
	return (!info) ? MACHMODE_GENERATOR : (info->Flags & 3) == 3
			? MACHMODE_GENERATOR 
		    : MACHMODE_FX;
}

int machine_supports(Machine* self, int option)
{
	if (self->info(self)) {
		return (self->info(self)->Flags & option) == option;
	}
	return 0;
}

int machine_bypassed(Machine* self)
{
	return self->bypass;
}

void machine_bypass(Machine* self)
{
	self->bypass = 1;
}

void machine_unbypass(Machine* self)
{
	self->bypass = 0;
}

int machine_muted(Machine* self)
{
	return self->mute;
}

void machine_mute(Machine* self)
{
	self->mute = 1;
}

void machine_unmute(Machine* self)
{
	self->mute = 0;
}

void setpanning(Machine* self, amp_t val)
{
	self->pan = val < 0.f ? 0.f : val > 1.f ? 1.f : val;
}

amp_t panning(Machine* self)
{
	return self->pan;
}

Buffer* mix(Machine* self, size_t slot, unsigned int amount, MachineSockets* connected_machine_sockets, Machines* machines)
{			
	Buffer* output;

	output = machines_outputs(machines, slot);
	if (output) {
		buffer_clearsamples(output, amount);
		if (connected_machine_sockets) {
			WireSocket* input_socket;
			
			for (input_socket = connected_machine_sockets->inputs; input_socket != 0; input_socket = input_socket->next) {
				WireSocketEntry* source = (WireSocketEntry*) input_socket->entry;
				addsamples(output, machines_outputs(machines, source->slot), amount,
					source->volume);				
			}							
		}
	}
	return output;
}

void addsamples(Buffer* dst, Buffer* source, unsigned int numsamples, amp_t vol)
{
	unsigned int channel;

	if (source) {
		for (channel = 0; channel < source->numchannels && 
			channel < dst->numchannels; ++channel) {
				dsp_add(
					source->samples[channel],
					dst->samples[channel],
					numsamples,
					vol);
				dsp_erase_all_nans_infinities_and_denormals(
					dst->samples[channel], numsamples);					
		}
	}
}

void loadspecific(Machine* self, struct SongFile* songfile, unsigned int slot)
{
	unsigned int size;
	unsigned int count;
	unsigned int i;

	psyfile_read(songfile->file, &size, sizeof(size)); // size of this part params to load	
	psyfile_read(songfile->file, &count, sizeof(count)); // num params to load
	for (i = 0; i < count; i++) {
		int temp;
		psyfile_read(songfile->file, &temp, sizeof(temp));
		self->parametertweak(self, i, temp);
	}
	psyfile_skip(songfile->file, size - sizeof(count) - (count * sizeof(int)));
}

void savespecific(Machine* self, struct SongFile* songfile, unsigned int slot)
{
	unsigned int i;
	unsigned count = self->numparameters(self);
	unsigned  size = sizeof(count)+(count*sizeof(int));
	psyfile_write(songfile->file, &size, sizeof(size));
	psyfile_write(songfile->file, &count, sizeof(count));
	for(i = 0; i < count; ++i) {
		int temp;
		
		temp = self->parametervalue(self, i);
		psyfile_write(songfile->file, &temp, sizeof(temp));
	}
}

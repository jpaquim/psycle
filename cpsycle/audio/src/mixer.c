// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "mixer.h"
#include "machines.h"
#include <operations.h>

#define TRUE 1
#define FALSE 0

const CMachineInfo* mixer_info(void)
{
	static CMachineInfo const macinfo = {
		MI_VERSION,
		0x0250,
		GENERATOR | 32 | 64,
		0,
		0,
		"Mixer"
			#ifndef NDEBUG
			" (debug build)"
			#endif
			,
		"Mixer",
		"Psycledelics",
		"help",
		0
	};
	return &macinfo;
}

static const CMachineInfo* info(Mixer* self) { return mixer_info(); }
static void mixer_dispose(Mixer* self);
static int mixer_mode(Mixer* self) { return MACHMODE_FX; }
static void mixer_seqtick(Mixer* self, int channel, const PatternEvent* event);
static unsigned int numinputs(Mixer*);
static unsigned int numoutputs(Mixer*);
static Buffer* mix(Mixer*, int slot, unsigned int amount, MachineSockets*, Machines*);
static void addsamples(Buffer* dst, Buffer* source, unsigned int numsamples, float vol);

static char* _psName;

void mixerinputchannel_init(MixerInputChannel* self)
{
	self->volume_=1.0f;
	self->panning_=0.5f;
	self->drymix_=1.0f;
	self->mute_= FALSE;
	self->dryonly_= FALSE;
	self->wetonly_= FALSE;		
}

void mixerreturnchannel_init(MixerReturnChannel* self)
{
	self->mastersend_= TRUE;
	self->volume_ = 1.0f;
	self->panning_ = 0.5f;
	self->mute_ = FALSE;	
}

void mixermasterchannel_init(MixerMasterChannel* self)
{
	self->volume_ = 1.0f;
	self->drywetmix_ = 0.5f;
	self->gain_ = 1.0f;
}

void mixer_init(Mixer* self, MachineCallback callback)
{
	machine_init(&self->machine, callback);
	self->machine.numinputs = numinputs;
	self->machine.numoutputs = numoutputs;
	self->machine.dispose = mixer_dispose;
	self->machine.seqtick = mixer_seqtick;
	self->machine.mode = mixer_mode;
	self->machine.mix = mix;
	self->solocolumn_ = -1;	
}

void mixer_dispose(Mixer* self)
{		
	machine_dispose(&self->machine);
}

void mixer_seqtick(Mixer* self, int channel, const PatternEvent* event)
{	
	if(event->note == NOTECOMMANDS_TWEAK)
	{
		int nv = (event->cmd<<8)+event->parameter;
		int param =  event->inst; // translate_param(event->inst);

		// if(param < GetNumParams()) {
		//	SetParameter(param,nv);
		// }
	}
	else if(event->note == NOTECOMMANDS_TWEAKSLIDE)
	{
		//\todo: Tweaks and tweak slides should not be a per-machine thing, but rather be player centric.
		// doing simply "tweak" for now..
		int nv = (event->cmd<<8)+event->parameter;
		int param = event->inst; // translate_param(event->inst);

		//if(param < GetNumParams()) {
		//	SetParameter(param,nv);
		//}
	}
}

unsigned int numinputs(Mixer* self)
{
	return 2;
}

unsigned int numoutputs(Mixer* self)
{
	return 2;
}

Buffer* mix(Mixer* self, int slot, unsigned int amount, MachineSockets* connected_machine_sockets, Machines* machines)
{			
	Buffer* output;

	output = machines_outputs(machines, slot);
	if (output) {
		buffer_clearsamples(output, amount);
		if (connected_machine_sockets) {
			WireSocket* WireSocket;
			
			for (WireSocket = connected_machine_sockets->inputs;
				WireSocket != 0; WireSocket = WireSocket->next) {
				WireSocketEntry* source = 
					(WireSocketEntry*)WireSocket->entry;
				if (source->slot != -1) {
					addsamples(
						output, 
						machines_outputs(machines, source->slot),
						amount,
						source->volume);
				}						
			}							
		}
	}
	return output;
}

void addsamples(Buffer* dst, Buffer* source, unsigned int numsamples, float vol)
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

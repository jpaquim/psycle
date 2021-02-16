// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "virtualgenerator.h"
#include "machines.h"
#include "plugin_interface.h"

const psy_audio_MachineInfo* psy_audio_virtualgenerator_info(void)
{
	static psy_audio_MachineInfo const macinfo = {
		MI_VERSION,
		0x0250,
		EFFECT | 32 | 64,
		MACHMODE_GENERATOR,
		"Virtual Generator"
			#ifndef NDEBUG
			" (debug build)"
			#endif
			,		
		"Virtual Generator",
		"Psycledelics",
		"help",		
		MACH_VIRTUALGENERATOR,
		0,
		0,		
		""
	};
	return &macinfo;
}

static const psy_audio_MachineInfo* info(psy_audio_VirtualGenerator* self) {
	return psy_audio_virtualgenerator_info();
}

static void dispose(psy_audio_VirtualGenerator*);
static int mode(psy_audio_VirtualGenerator* self) { return MACHMODE_GENERATOR; }
static void seqtick(psy_audio_VirtualGenerator*, uintptr_t channel,
	const psy_audio_PatternEvent* ev);
static uintptr_t numinputs(psy_audio_VirtualGenerator* self) { return 0; }
static uintptr_t numoutputs(psy_audio_VirtualGenerator* self) { return 0; }
static psy_audio_MachineParam* parameter(psy_audio_VirtualGenerator*,
	uintptr_t param);
static uintptr_t numparameters(psy_audio_VirtualGenerator*);
static unsigned int numparametercols(psy_audio_VirtualGenerator*);
static psy_audio_Buffer* buffermemory(psy_audio_VirtualGenerator*);
static uintptr_t buffermemorysize(psy_audio_VirtualGenerator*);
static void setbuffermemorysize(psy_audio_VirtualGenerator*, uintptr_t size);

static MachineVtable super_vtable;

static MachineVtable vtable;
static bool vtable_initialized = FALSE;

static void vtable_init(psy_audio_VirtualGenerator* self)
{
	if (!vtable_initialized) {		
		vtable = *self->custommachine.machine.vtable;
		super_vtable = vtable;
		vtable.dispose = (fp_machine_dispose)dispose;
		vtable.info = (fp_machine_info)info;
		vtable.mode = (fp_machine_mode)mode;
		vtable.seqtick = (fp_machine_seqtick)seqtick;
		vtable.numinputs = (fp_machine_numinputs)numinputs;
		vtable.numoutputs = (fp_machine_numoutputs)numoutputs;
		vtable.parameter = (fp_machine_parameter)parameter;		
		vtable.numparameters = (fp_machine_numparameters)numparameters;
		vtable.numparametercols = (fp_machine_numparametercols)numparametercols;
		vtable.buffermemory = (fp_machine_buffermemory)
			buffermemory;
		vtable.buffermemorysize = (fp_machine_buffermemorysize)
			buffermemorysize;
		vtable.setbuffermemorysize = (fp_machine_setbuffermemorysize)
			setbuffermemorysize;
		vtable_initialized = TRUE;
	}
}

void psy_audio_virtualgenerator_init(psy_audio_VirtualGenerator* self,
	psy_audio_MachineCallback* callback, uintptr_t macindex, uintptr_t instindex)
{
	assert(self);
	
	psy_audio_custommachine_init(&self->custommachine, callback);
	vtable_init(self);
	self->custommachine.machine.vtable = &vtable;
	self->machine_index = macindex;	
	self->instrument_index = psy_audio_instrumentindex_make(0, instindex);
	psy_audio_uintptrmachineparam_init(&self->param_inst,
		"Instrument", "Inst", MPF_STATE, &self->instrument_index.subslot,
		0, 0xFF);
	psy_audio_uintptrmachineparam_init(&self->param_sampler,
		"Sampler", "Sampler", MPF_STATE, &self->machine_index, 0, 0xFF);
}

void dispose(psy_audio_VirtualGenerator* self)
{
	assert(self);

	psy_audio_uintptrmachineparam_dispose(&self->param_inst);
	psy_audio_uintptrmachineparam_dispose(&self->param_sampler);
}

void psy_audio_virtualgenerator_setinstrument(psy_audio_VirtualGenerator* self,
	uintptr_t index)
{
	assert(self);

	self->instrument_index.subslot = index;
}

uintptr_t psy_audio_virtualgenerator_instrumentindex(const
	psy_audio_VirtualGenerator* self)
{
	assert(self);

	return self->instrument_index.subslot;
}

void seqtick(psy_audio_VirtualGenerator* self, uintptr_t channel,
	const psy_audio_PatternEvent* ev)
{
	psy_audio_Machines* machines;

	machines = psy_audio_machine_machines(&self->custommachine.machine);
	if (machines) {
		psy_audio_Machine* sampler;

		sampler = psy_audio_machines_at(machines, self->machine_index);
		if (sampler) {			
			psy_audio_PatternEvent realevent;			
									
			realevent = *ev;
			// translate event to real machine
			// instrument_index.subslot: instrument index of the real sampler
			// (instrument group is set in the sampler machine)
			// self->machine_index:      machine index of the real sampler
			if (ev->note <= psy_audio_NOTECOMMANDS_RELEASE) {
				// set instrument only at notecommand else
				// xmsampler::tick will reset the channel
				// todo: check if sampulse should be changed instead here
				// In XMSampler::Tick
				// if (currentVoice != NULL && !bNoteOn && pData->note != psy_audio_NOTECOMMANDS_RELEASE) {
				//	  //Whenever an instrument appears alone in a channel, the values are reset.
				//	  //todo: It should be reset to the values of the instrument set.
				//    psy_audio_xmsamplervoice_resetvolandpan(currentVoice, -1, TRUE);
				realevent.inst = (uint16_t)self->instrument_index.subslot;
			} else {
				realevent.inst = psy_audio_NOTECOMMANDS_INST_EMPTY;
			}
			realevent.mach = (uint8_t)self->machine_index;
			// 1. tick with cmd and parameter
			psy_audio_machine_seqtick(sampler, channel, &realevent);			
			if (ev->inst != psy_audio_NOTECOMMANDS_INST_EMPTY) {
				// 
				if (psy_audio_machine_type(sampler) == MACH_XMSAMPLER) {
					realevent.cmd = 0x1E;
				} else {
					realevent.cmd = 0xFC;
				}
				realevent.parameter = (uint8_t)ev->inst;
				psy_audio_machine_seqtick(sampler, channel, &realevent);
			}			
		}
	}
}

psy_audio_MachineParam* parameter(psy_audio_VirtualGenerator* self, uintptr_t param)
{
	if (param == 0) {
		return &self->param_inst.machineparam;
	} else if (param == 1) {
		return &self->param_sampler.machineparam;
	}
	return NULL;
}

uintptr_t numparameters(psy_audio_VirtualGenerator* self)
{
	return 2;
}

unsigned int numparametercols(psy_audio_VirtualGenerator* self)
{
	return 1;
}

psy_audio_Buffer* buffermemory(psy_audio_VirtualGenerator* self)
{
	psy_audio_Machines* machines;

	// delegate the buffermemory from the real machine that the scopes
	// display it
	// todo delegate channel voice buffer of inst selected
	machines = psy_audio_machine_machines(&self->custommachine.machine);
	if (machines) {
		psy_audio_Machine* sampler;

		sampler = psy_audio_machines_at(machines, self->machine_index);
		if (sampler) {
			return psy_audio_machine_buffermemory(sampler);
		}
	}
	// fallback to own buffermemory if sampler machine is not valid
	return super_vtable.buffermemory(&self->custommachine.machine);
}

uintptr_t buffermemorysize(psy_audio_VirtualGenerator* self)
{
	psy_audio_Machines* machines;

	machines = psy_audio_machine_machines(&self->custommachine.machine);
	if (machines) {
		psy_audio_Machine* sampler;

		sampler = psy_audio_machines_at(machines, self->machine_index);
		if (sampler) {
			return psy_audio_machine_buffermemorysize(sampler);
		}
	}
	// fallback to own buffermemory if sampler machine is not valid
	return super_vtable.buffermemorysize(&self->custommachine.machine);
}

void setbuffermemorysize(psy_audio_VirtualGenerator* self, uintptr_t size)
{
	psy_audio_Machines* machines;

	machines = psy_audio_machine_machines(&self->custommachine.machine);
	if (machines) {
		psy_audio_Machine* sampler;

		sampler = psy_audio_machines_at(machines, self->machine_index);
		if (sampler) {
			psy_audio_machine_setbuffermemorysize(sampler, size);
		}
	}
	// fallback to own buffermemory if sampler machine is not valid
	super_vtable.setbuffermemorysize(&self->custommachine.machine, size);
}

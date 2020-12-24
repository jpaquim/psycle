// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

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

static int mode(psy_audio_VirtualGenerator* self) { return MACHMODE_GENERATOR; }
static void seqtick(psy_audio_VirtualGenerator*, uintptr_t channel,
	const psy_audio_PatternEvent* ev);
static uintptr_t numinputs(psy_audio_VirtualGenerator* self) { return 0; }
static uintptr_t numoutputs(psy_audio_VirtualGenerator* self) { return 0; }
static psy_audio_MachineParam* parameter(psy_audio_VirtualGenerator*,
	uintptr_t param);
static uintptr_t numparameters(psy_audio_VirtualGenerator*);
static unsigned int numparametercols(psy_audio_VirtualGenerator* self);

static MachineVtable vtable;
static bool vtable_initialized = FALSE;

static void vtable_init(psy_audio_VirtualGenerator* self)
{
	if (!vtable_initialized) {
		vtable = *self->custommachine.machine.vtable;
		vtable.info = (fp_machine_info)info;
		vtable.mode = (fp_machine_mode)mode;
		vtable.seqtick = (fp_machine_seqtick)seqtick;
		vtable.numinputs = (fp_machine_numinputs)numinputs;
		vtable.numoutputs = (fp_machine_numoutputs)numoutputs;
		vtable.parameter = (fp_machine_parameter)parameter;		
		vtable.numparameters = (fp_machine_numparameters)numparameters;
		vtable.numparametercols = (fp_machine_numparametercols)numparametercols;
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
	psy_audio_intmachineparam_init(&self->param_sampler,
		"Sampler", "Sampler", MPF_STATE, &self->machine_index, 0, 0xFF);
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
			realevent.inst = self->instrument_index.subslot;
			if (ev->inst != psy_audio_NOTECOMMANDS_INST_EMPTY) {				
				if (psy_audio_machine_type(sampler) == MACH_XMSAMPLER) {
					realevent.cmd = 0x1E;
				} else {
					realevent.cmd = 0xFC;
				}
				realevent.parameter = ev->inst;
			}
			psy_audio_machine_seqtick(sampler, channel, &realevent);
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
// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "dummy.h"
#include "plugin_interface.h"

const psy_audio_MachineInfo* dummymachine_info(void)
{
	static psy_audio_MachineInfo const macinfo = {
		MI_VERSION,
		0x0250,
		EFFECT | 32 | 64,
		MACHMODE_FX,
		"Dummy plug"
			#ifndef NDEBUG
			" (debug build)"
			#endif
			,		
		"Dummy Plug",
		"Psycledelics",
		"help",		
		MACH_DUMMY,
		0,
		0,		
		""
	};
	return &macinfo;
}

static const psy_audio_MachineInfo* info(psy_audio_DummyMachine* self) {
	return dummymachine_info();
}
static int mode(psy_audio_DummyMachine* self) { return self->mode; }
static uintptr_t numinputs(psy_audio_DummyMachine* self) { return 2; }
static uintptr_t numoutputs(psy_audio_DummyMachine* self) { return 2; }
static psy_dsp_amp_range_t amprange(psy_audio_DummyMachine* self)
{
	return PSY_DSP_AMP_RANGE_IGNORE;
}

static MachineVtable vtable;
static int vtable_initialized = 0;

static void vtable_init(psy_audio_DummyMachine* self)
{
	if (!vtable_initialized) {
		vtable = *self->custommachine.machine.vtable;
		vtable.mode = (fp_machine_mode) mode;
		vtable.info = (fp_machine_info) info;
		vtable.numinputs = (fp_machine_numinputs) numinputs;
		vtable.numoutputs = (fp_machine_numoutputs) numoutputs;
		vtable.amprange = (fp_machine_amprange) amprange;
		vtable_initialized = 1;
	}
}

void dummymachine_init(psy_audio_DummyMachine* self, psy_audio_MachineCallback callback)
{	
	self->mode = MACHMODE_FX;
	custommachine_init(&self->custommachine, callback);
	vtable_init(self);
	self->custommachine.machine.vtable = &vtable;
}

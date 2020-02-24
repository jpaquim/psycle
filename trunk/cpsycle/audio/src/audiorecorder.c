// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "audiorecorder.h"
#include "plugin_interface.h"

const psy_audio_MachineInfo* psy_audio_audiorecorder_info(void)
{
	static psy_audio_MachineInfo const macinfo = {
		MI_VERSION,
		0x0250,
		0,
		MACHMODE_GENERATOR,
		"AudioRecorder"
			#ifndef NDEBUG
			" (debug build)"
			#endif
			,
		"AudioRecorder",
		"Psycledelics",
		"help",
		MACH_RECORDER
	};
	return &macinfo;
}

static const psy_audio_MachineInfo* info(psy_audio_AudioRecorder* self) {
	return psy_audio_audiorecorder_info();
}
static int mode(psy_audio_AudioRecorder* self) { return MACHMODE_GENERATOR; }
static uintptr_t numinputs(psy_audio_AudioRecorder* self) { return 0; }
static uintptr_t numoutputs(psy_audio_AudioRecorder* self) { return 2; }
static psy_dsp_amp_range_t amprange(psy_audio_AudioRecorder* self)
{
	return PSY_DSP_AMP_RANGE_NATIVE;
}

static MachineVtable vtable;
static int vtable_initialized = 0;

static void vtable_init(psy_audio_AudioRecorder* self)
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

void psy_audio_audiorecorder_init(psy_audio_AudioRecorder* self, MachineCallback callback)
{		
	custommachine_init(&self->custommachine, callback);
	vtable_init(self);
	self->custommachine.machine.vtable = &vtable;
	psy_audio_machine_seteditname(psy_audio_audiorecorder_base(self), "Recorder");
}

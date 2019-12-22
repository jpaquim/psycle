// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "custommachine.h"
#include <stdlib.h>
#include <string.h>
#include <operations.h>

static const char* custommachine_editname(psy_audio_CustomMachine*);
static void custommachine_seteditname(psy_audio_CustomMachine*, const char* name);
static void setpanning(psy_audio_CustomMachine*, psy_dsp_amp_t);
static psy_dsp_amp_t panning(psy_audio_CustomMachine*);
static void mute(psy_audio_CustomMachine* self) { self->ismuted = 1;  }
static void unmute(psy_audio_CustomMachine* self) { self->ismuted = 0;  }
static int muted(psy_audio_CustomMachine* self) { return self->ismuted; }
static void bypass(psy_audio_CustomMachine* self) { self->isbypassed = 1; }
static void unbypass(psy_audio_CustomMachine* self) { self->isbypassed = 0; }
static int bypassed(psy_audio_CustomMachine* self) { return self->isbypassed; }
static psy_audio_Buffer* custommachine_buffermemory(psy_audio_CustomMachine*);
static uintptr_t custommachine_buffermemorysize(psy_audio_CustomMachine*);
static void custommachine_setbuffermemorysize(psy_audio_CustomMachine*, uintptr_t size);
static uintptr_t custommachine_slot(psy_audio_CustomMachine*);
static void custommachine_setslot(psy_audio_CustomMachine*, uintptr_t slot);

static MachineVtable vtable;
static int vtable_initialized = 0;

static void vtable_init(psy_audio_CustomMachine* self)
{
	if (!vtable_initialized) {
		vtable = *self->machine.vtable;
		vtable.setpanning = (fp_machine_setpanning) setpanning;
		vtable.panning = (fp_machine_panning) panning;
		vtable.mute = (fp_machine_mute) mute;
		vtable.unmute = (fp_machine_unmute) unmute;
		vtable.muted = (fp_machine_muted) muted;
		vtable.bypass = (fp_machine_bypass) bypass;
		vtable.unbypass = (fp_machine_unbypass) unbypass;
		vtable.bypassed = (fp_machine_bypassed) bypassed;
		vtable.editname = (fp_machine_editname) custommachine_editname;
		vtable.seteditname = (fp_machine_seteditname)
			custommachine_seteditname;
		vtable.buffermemory = (fp_machine_buffermemory)
			custommachine_buffermemory;
		vtable.buffermemorysize = (fp_machine_buffermemorysize)
			custommachine_buffermemorysize;
		vtable.setbuffermemorysize = (fp_machine_setbuffermemorysize)
			custommachine_setbuffermemorysize;
		vtable.setslot = (fp_machine_setslot) custommachine_setslot;
		vtable.slot = (fp_machine_slot) custommachine_slot;
		vtable_initialized = 1;
	}
}

void custommachine_init(psy_audio_CustomMachine* self, MachineCallback callback)
{
	uintptr_t c;

	machine_init(&self->machine, callback);
	vtable_init(self);
	self->machine.vtable = &vtable;
	self->editname = 0;
	self->ismuted = 0;
	self->isbypassed = 0;
	self->pan = (psy_dsp_amp_t) 0.5f;
	self->slot = NOMACHINE_INDEX;
	buffer_init(&self->memorybuffer, 2);
	self->memorybuffersize = 256;
	for (c = 0; c < self->memorybuffer.numchannels; ++c) {
		self->memorybuffer.samples[c] = dsp.memory_alloc(
			self->memorybuffersize, sizeof(psy_dsp_amp_t));
	}
}

void custommachine_dispose(psy_audio_CustomMachine* self)
{
	uintptr_t c;

	free(self->editname);
	self->editname = 0;
	for (c = 0; c < self->memorybuffer.numchannels; ++c) {
		dsp.memory_dealloc(self->memorybuffer.samples[c]);
	}
	buffer_dispose(&self->memorybuffer);
	machine_dispose(&self->machine);
}

void setpanning(psy_audio_CustomMachine* self, psy_dsp_amp_t val)
{
	self->pan = val < 0.f ? 0.f : val > 1.f ? 1.f : val;
}

psy_dsp_amp_t panning(psy_audio_CustomMachine* self)
{
	return self->pan;
}

const char* custommachine_editname(psy_audio_CustomMachine* self)
{
	return self->editname;
}

void custommachine_seteditname(psy_audio_CustomMachine* self, const char* name)
{
	free(self->editname);
	self->editname = strdup(name);
}

psy_audio_Buffer* custommachine_buffermemory(psy_audio_CustomMachine* self)
{
	return &self->memorybuffer;	
}

uintptr_t custommachine_buffermemorysize(psy_audio_CustomMachine* self)
{
	return self->memorybuffersize;	
}

void custommachine_setbuffermemorysize(psy_audio_CustomMachine* self, uintptr_t size)
{
	// self->memorybuffersize = size;
}

uintptr_t custommachine_slot(psy_audio_CustomMachine* self)
{
	return self->slot;
}

void custommachine_setslot(psy_audio_CustomMachine* self, uintptr_t slot)
{
	self->slot = slot;
}

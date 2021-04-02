// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "custommachine.h"
// dsp
#include <operations.h>
// std
#include <stdlib.h>
#include <string.h>
// platform
#include "../../detail/portable.h"

static void custommachine_init_memory(psy_audio_CustomMachine*, uintptr_t numframes);
static void custommachine_dispose_memory(psy_audio_CustomMachine*);
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
static void standby(psy_audio_CustomMachine* self)
{
	if (psy_audio_machine_mode(&self->machine) == psy_audio_MACHMODE_GENERATOR) {
		psy_audio_machine_mute(&self->machine);
	} else {
		psy_audio_machine_bypass(&self->machine);
	}	
}
static void deactivatestandby(psy_audio_CustomMachine* self) {
	if (psy_audio_machine_mode(&self->machine) == psy_audio_MACHMODE_GENERATOR) {
		psy_audio_machine_unmute(&self->machine);
	} else {
		psy_audio_machine_unbypass(&self->machine);
	}
}
static int hasstandby(psy_audio_CustomMachine* self)
{
	if (psy_audio_machine_mode(&self->machine) == psy_audio_MACHMODE_GENERATOR) {
		return psy_audio_machine_muted(&self->machine);
	} else {
		return psy_audio_machine_bypassed(&self->machine);
	}
}

static void setbus(psy_audio_CustomMachine* self)
{
	self->isbus = 1;
	psy_audio_machine_buschanged(&self->machine);
}

static void unsetbus(psy_audio_CustomMachine* self)
{
	self->isbus = 0;
	psy_audio_machine_buschanged(&self->machine);
}

static int isbus(psy_audio_CustomMachine* self) { return self->isbus; }
static psy_audio_Buffer* custommachine_buffermemory(psy_audio_CustomMachine*);
static uintptr_t custommachine_buffermemorysize(psy_audio_CustomMachine*);
static void custommachine_setbuffermemorysize(psy_audio_CustomMachine*, uintptr_t size);
static uintptr_t custommachine_slot(psy_audio_CustomMachine*);
static void custommachine_setslot(psy_audio_CustomMachine*, uintptr_t slot);
static void setposition(psy_audio_CustomMachine* self, double x, double y)
{
	self->x = x;
	self->y = y;
}
static void position(psy_audio_CustomMachine* self, double* x, double* y)
{
	*x = self->x;
	*y = self->y;
}
// parameters
static void selectparam(psy_audio_CustomMachine* self, uintptr_t index)
{
	self->paramselected = index;
}

static uintptr_t paramselected(psy_audio_CustomMachine* self)
{
	return self->paramselected;
}
// auxcolumn
static const char* auxcolumnname(psy_audio_CustomMachine* self,
	uintptr_t index)
{
	return "";
}
static uintptr_t numauxcolumns(psy_audio_CustomMachine* self) { return 0; }
static uintptr_t auxcolumnselected(psy_audio_CustomMachine* self)
{
	return self->auxcolumnselected;
}

static void selectauxcolumn(psy_audio_CustomMachine* self, uintptr_t index)
{
	self->auxcolumnselected = index;
}

// vtable
static MachineVtable vtable;
static bool vtable_initialized = FALSE;

static void vtable_init(psy_audio_CustomMachine* self)
{
	if (!vtable_initialized) {
		vtable = *self->machine.vtable;
		vtable.setpanning = (fp_machine_setpanning)setpanning;
		vtable.panning = (fp_machine_panning)panning;
		vtable.mute = (fp_machine_mute)mute;
		vtable.unmute = (fp_machine_unmute)unmute;
		vtable.muted = (fp_machine_muted)muted;
		vtable.bypass = (fp_machine_bypass)bypass;
		vtable.unbypass = (fp_machine_unbypass)unbypass;
		vtable.bypassed = (fp_machine_bypassed)bypassed;
		vtable.standby = (fp_machine_standby)standby;
		vtable.deactivatestandby = (fp_machine_deactivatestandby)
			deactivatestandby;
		vtable.hasstandby = (fp_machine_hasstandby)hasstandby;
		vtable.setbus = (fp_machine_setbus)setbus;
		vtable.unsetbus = (fp_machine_unsetbus)unsetbus;
		vtable.isbus = (fp_machine_isbus)isbus;
		vtable.editname = (fp_machine_editname)custommachine_editname;
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
		vtable.setposition = (fp_machine_setposition)setposition;
		vtable.position = (fp_machine_position)position;
		vtable.numauxcolumns = (fp_machine_numauxcolumns)numauxcolumns;
		vtable.selectauxcolumn = (fp_machine_selectauxcolumn)selectauxcolumn;
		vtable.auxcolumnselected = (fp_machine_auxcolumnselected)auxcolumnselected;
		vtable.paramselected = (fp_machine_paramselected)paramselected;
		vtable.selectparam = (fp_machine_selectparam)selectparam;
		vtable_initialized = TRUE;
	}
}

void psy_audio_custommachine_init(psy_audio_CustomMachine* self,
	psy_audio_MachineCallback* callback)
{	
	psy_audio_machine_init(&self->machine, callback);
	vtable_init(self);
	self->machine.vtable = &vtable;
	self->editname = 0;
	self->ismuted = 0;
	self->isbypassed = 0;
	self->isbus = 0;
	self->pan = (psy_dsp_amp_t) 0.5f;
	self->slot = psy_INDEX_INVALID;
	self->x = 0;
	self->y = 0;
	self->auxcolumnselected = 0;
	self->paramselected = 0;
	custommachine_init_memory(self, psy_audio_MAX_STREAM_SIZE);
}

void custommachine_init_memory(psy_audio_CustomMachine* self, uintptr_t numframes)
{
	psy_audio_buffer_init(&self->memorybuffer, 2);
	self->memorybuffersize = numframes;
	psy_audio_buffer_allocsamples(&self->memorybuffer, self->memorybuffersize);
	psy_audio_buffer_clearsamples(&self->memorybuffer, self->memorybuffersize);
	psy_audio_buffer_enablerms(&self->memorybuffer);
}

void psy_audio_custommachine_dispose(psy_audio_CustomMachine* self)
{	
	free(self->editname);
	self->editname = 0;
	custommachine_dispose_memory(self);
	machine_base_dispose(&self->machine);
}

void custommachine_dispose_memory(psy_audio_CustomMachine* self)
{	
	psy_audio_buffer_dispose(&self->memorybuffer);
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
	psy_strreset(&self->editname, name);
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
	custommachine_dispose_memory(self);
	custommachine_init_memory(self, size);
}

uintptr_t custommachine_slot(psy_audio_CustomMachine* self)
{
	return self->slot;
}

void custommachine_setslot(psy_audio_CustomMachine* self, uintptr_t slot)
{
	self->slot = slot;
}

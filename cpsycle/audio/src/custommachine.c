// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "custommachine.h"
#include <stdlib.h>
#include <string.h>

static const char* custommachine_editname(CustomMachine*);
static void custommachine_seteditname(CustomMachine*, const char* name);
static void setpanning(CustomMachine*, psy_dsp_amp_t);
static psy_dsp_amp_t panning(CustomMachine*);
static void mute(CustomMachine* self) { self->ismuted = 1;  }
static void unmute(CustomMachine* self) { self->ismuted = 0;  }
static int muted(CustomMachine* self) { return self->ismuted; }
static void bypass(CustomMachine* self) { self->isbypassed = 1; }
static void unbypass(CustomMachine* self) { self->isbypassed = 0; }
static int bypassed(CustomMachine* self) { return self->isbypassed; }

static MachineVtable vtable;
static int vtable_initialized = 0;

static void vtable_init(CustomMachine* self)
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
		vtable.seteditname = (fp_machine_seteditname) custommachine_seteditname;
		vtable_initialized = 1;
	}
}

void custommachine_init(CustomMachine* self, MachineCallback callback)
{
	machine_init(&self->machine, callback);
	vtable_init(self);
	self->machine.vtable = &vtable;
	self->editname = 0;
	self->ismuted = 0;
	self->isbypassed = 0;
	self->pan = (psy_dsp_amp_t) 0.5f;
}

void setpanning(CustomMachine* self, psy_dsp_amp_t val)
{
	self->pan = val < 0.f ? 0.f : val > 1.f ? 1.f : val;
}

psy_dsp_amp_t panning(CustomMachine* self)
{
	return self->pan;
}

void custommachine_dispose(CustomMachine* self)
{
	free(self->editname);
	machine_dispose(&self->machine);
}

const char* custommachine_editname(CustomMachine* self)
{
	return self->editname;
}

void custommachine_seteditname(CustomMachine* self, const char* name)
{
	free(self->editname);
	self->editname = strdup(name);
}

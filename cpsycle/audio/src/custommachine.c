// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "custommachine.h"
#include <stdlib.h>
#include <string.h>

static const char* custommachine_editname(CustomMachine*);
static void custommachine_seteditname(CustomMachine*, const char* name);
static void setpanning(CustomMachine*, amp_t);
static amp_t panning(CustomMachine*);
static void mute(CustomMachine* self) { self->ismuted = 1;  }
static void unmute(CustomMachine* self) { self->ismuted = 0;  }
static int muted(CustomMachine* self) { return self->ismuted; }
static void bypass(CustomMachine* self) { self->isbypassed = 1; }
static void unbypass(CustomMachine* self) { self->isbypassed = 0; }
static int bypassed(CustomMachine* self) { return self->isbypassed; }

void custommachine_init(CustomMachine* self, MachineCallback callback)
{
	machine_init(&self->machine, callback);
	self->machine.setpanning = setpanning;
	self->machine.panning = panning;
	self->machine.mute = mute;
	self->machine.unmute = unmute;
	self->machine.muted = muted;
	self->machine.bypass = bypass;
	self->machine.unbypass = unbypass;
	self->machine.bypassed = bypassed;
	self->machine.editname = custommachine_editname;
	self->machine.seteditname = custommachine_seteditname;
	self->editname = 0;
	self->ismuted = 0;
	self->isbypassed = 0;
	self->pan = (amp_t) 0.5f;
}

void setpanning(CustomMachine* self, amp_t val)
{
	self->pan = val < 0.f ? 0.f : val > 1.f ? 1.f : val;
}

amp_t panning(CustomMachine* self)
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

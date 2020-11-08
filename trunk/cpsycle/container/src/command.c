// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "command.h"

static void dispose(psy_Command* self) { }
static void execute(psy_Command* self) { }
static void revert(psy_Command* self) { }

static psy_CommandVtable vtable;
static int vtable_initialized = 0;

static void vtable_init(void)
{
	if (!vtable_initialized) {
		vtable.dispose = dispose;
		vtable.execute = execute;
		vtable.revert = revert;
		vtable_initialized = 1;
	}
}

void psy_command_init(psy_Command* self)
{
	assert(self);
	vtable_init();
	self->vtable = &vtable;
}

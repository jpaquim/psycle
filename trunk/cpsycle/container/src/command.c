/*
** This source is free software; you can redistribute itand /or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "command.h"
#include "properties.h"

static void dispose(psy_Command* self) { }
static void execute(psy_Command* self, const psy_Property* params) { }
static void revert(psy_Command* self) { }

static psy_CommandVtable vtable;
static bool vtable_initialized = FALSE;

static void vtable_init(psy_Command* self)
{
	if (!vtable_initialized) {
		vtable.dispose = dispose;
		vtable.execute = execute;
		vtable.revert = revert;
		vtable_initialized = TRUE;
	}
	self->vtable = &vtable;
}

void psy_command_init(psy_Command* self)
{
	assert(self);
	vtable_init(self);	
}

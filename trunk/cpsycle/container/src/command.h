// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#if !defined(psy_COMMAND_H)
#define psy_COMMAND_H

#include "../../detail/psydef.h"

// aim: Interface that implements the Command Pattern used
//      for undo/redo operations

struct psy_Command;

typedef void (*psy_fp_command)(struct psy_Command*);

typedef struct psy_CommandVtable {
	psy_fp_command dispose;
	psy_fp_command execute;
	psy_fp_command revert;	
} psy_CommandVtable;

typedef struct psy_Command {
	psy_CommandVtable* vtable;
} psy_Command;

void psy_command_init(psy_Command* self);

INLINE void psy_command_dispose(psy_Command* self)
{
	self->vtable->dispose(self);
}

INLINE void psy_command_execute(psy_Command* self)
{
	self->vtable->execute(self);
}

INLINE void psy_command_revert(psy_Command* self)
{
	self->vtable->revert(self);
}

#endif /* psy_Command */

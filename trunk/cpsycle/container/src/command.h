/*
** This source is free software; you can redistribute itand /or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#if !defined(psy_COMMAND_H)
#define psy_COMMAND_H

#include "../../detail/psydef.h"


#include <assert.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
** Interface that implements the Command Pattern used
** for undo/redo operations
*/

struct psy_Command;
struct psy_Property;

typedef void (*psy_fp_command)(struct psy_Command*);
typedef void (*psy_fp_command_params)(struct psy_Command*,
	const struct psy_Property*);

typedef struct psy_CommandVtable {
	psy_fp_command dispose;
	psy_fp_command_params execute;
	psy_fp_command revert;	
} psy_CommandVtable;

typedef struct psy_Command {
	psy_CommandVtable* vtable;
} psy_Command;

void psy_command_init(psy_Command* self);


INLINE void psy_command_dispose(psy_Command* self)
{
	assert(self);
	self->vtable->dispose(self);
}

INLINE void psy_command_execute(psy_Command* self, struct psy_Property* params)
{
	assert(self);

	self->vtable->execute(self, params);
}

INLINE void psy_command_revert(psy_Command* self)
{
	assert(self);
	self->vtable->revert(self);
}

#ifdef __cplusplus
}
#endif

#endif /* psy_Command */

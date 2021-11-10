/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
**  copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "pianogridstate.h"

/* PianoGridState */
void pianogridstate_init(PianoGridState* self, PatternViewState* pvstate)
{
	assert(self);

	self->pv = pvstate;	
	self->defaultbeatwidth = 90;
	self->pxperbeat = self->defaultbeatwidth;	
}

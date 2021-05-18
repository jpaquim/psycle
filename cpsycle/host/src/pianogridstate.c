/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
**  copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "pianogridstate.h"

/* PianoGridState */
void pianogridstate_init(PianoGridState* self, PatternViewSkin* skin)
{
	assert(self);

	self->pattern = NULL;
	self->defaultbeatwidth = 90;
	self->pxperbeat = self->defaultbeatwidth;
	self->lpb = 4;	
	self->skin = skin;		
	psy_audio_patterncursor_init(&self->cursor);
}

/*
** This source is free software; you can redistribute itand /or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2, or (at your option) any later version.
** copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "patternviewstate.h"
/* audio */
#include <pattern.h>
/* std */
#include <math.h>
#include <assert.h>
/* platform */
#include "../../detail/trace.h"
#include "../../detail/portable.h"

/* implementation */
void patternviewstate_init(PatternViewState* self, psy_audio_Song* song)
{
	psy_audio_sequencecursor_init(&self->cursor);
	psy_audio_blockselection_init(&self->selection);	
	self->pgupdownstepmode = PATTERNCURSOR_STEP_BEAT;
	self->pgupdownstep = 4;	
	self->singlemode = TRUE;
	self->song = song;
	self->pattern = NULL;
	self->skin = NULL;	
	self->display = PROPERTY_ID_PATTERN_DISPLAYMODE_TRACKER;
}

void patternviewstate_dispose(PatternViewState* self)
{	
}

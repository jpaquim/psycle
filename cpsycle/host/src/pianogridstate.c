/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
**  copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "pianogridstate.h"

void pianogridstate_init(PianoGridState* self, PatternViewState* pvstate)
{
	assert(self);

	self->pv = pvstate;	
	self->defaultbeatwidth = 90;
	self->pxperbeat = self->defaultbeatwidth;
	psy_property_init_type(&self->track_display, "track-display",
		PSY_PROPERTY_TYPE_CHOICE);
	psy_property_set_item_int(&self->track_display,
		PIANOROLL_TRACK_DISPLAY_ALL);
	self->track_all = psy_property_set_text(psy_property_append_int(
		&self->track_display, "all",  PIANOROLL_TRACK_DISPLAY_ALL, 0, 0),
		"patternview.all");
	self->track_current = psy_property_set_text(psy_property_append_int(
		&self->track_display, "current", PIANOROLL_TRACK_DISPLAY_CURRENT, 0, 0),
		"patternview.current");
	self->track_active = psy_property_set_text(psy_property_append_int(
		&self->track_display, "active",  PIANOROLL_TRACK_DISPLAY_ACTIVE, 0, 0),
		"patternview.active");
}

void pianogridstate_dispose(PianoGridState* self)
{
	assert(self);
	
	psy_property_dispose(&self->track_display);
}

PianoTrackDisplay pianogridstate_track_display(const PianoGridState* self)
{
	psy_Property* p;
	
	assert(self);
	
	p = psy_property_at_choice(&((PianoGridState*)self)->track_display);
	if (p) {
		return (PianoTrackDisplay)psy_property_item_int(p);
	}
	return PIANOROLL_TRACK_DISPLAY_ALL;
}

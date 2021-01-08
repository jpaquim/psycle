// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "sequencetrackbox.h"
// platform
#include "../../detail/portable.h"

// prototypes
static void sequencetrackbox_drawtext(SequenceTrackBox*,
	psy_ui_Graphics*, intptr_t x, intptr_t y, const char* text);
// implementation
void sequencetrackbox_init(SequenceTrackBox* self,
	psy_ui_Rectangle position, psy_ui_TextMetric tm,
	psy_audio_SequenceTrack* track,
	psy_audio_Sequence* sequence,
	uintptr_t trackindex, bool selected)
{
	self->position = position;
	self->tm = tm;
	self->track = track;			
	self->track = track;
	self->sequence = sequence;
	self->trackindex = trackindex;
	self->selected = selected;
	self->showname = FALSE;
	self->colour = psy_ui_colour_make(0x00303030);
	self->colour_highlight = psy_ui_colour_make(0x00545454);
	self->colour_font = psy_ui_colour_make(0x00B2B2B2);
	self->colour_fonthighlight = psy_ui_colour_make(0x00B1C8B0);	
}

void sequencetrackbox_draw(SequenceTrackBox* self, psy_ui_Graphics* g)
{	
	psy_ui_Rectangle r;
	char text[64];		
	intptr_t centery;	
	
	centery = (self->position.bottom - self->position.top - self->tm.tmHeight) / 2;
	psy_ui_settextcolour(g, self->colour_font);
	if (self->track) {
		r = self->position;
		if (self->selected) {
			psy_ui_setcolour(g, self->colour_highlight);			
		} else {
			psy_ui_setcolour(g, self->colour);
		}
		psy_ui_drawrectangle(g, r);
		psy_snprintf(text, 64, "%.2X", (int)self->trackindex);
		r.top += centery;
		r.bottom = self->tm.tmHeight;
		sequencetrackbox_drawtext(self, g, r.left +
			(intptr_t)(self->tm.tmAveCharWidth * 0.2), r.top, text);
		if (psy_audio_sequence_istracksoloed(self->sequence, self->trackindex)) {
			psy_ui_settextcolour(g, self->colour_fonthighlight);
		} else {
			psy_ui_settextcolour(g, self->colour_font);
		}
		sequencetrackbox_drawtext(self, g, r.left +
			(intptr_t)(self->tm.tmAveCharWidth * 5), r.top, "S");
		if (self->trackindex != 0) {
			sequencetrackbox_drawtext(self, g, self->position.right -
				(int)(self->tm.tmAveCharWidth * 3.5), r.top, "X");
		}
		if (psy_audio_sequence_istrackmuted(self->sequence, self->trackindex)) {		
			psy_ui_settextcolour(g, self->colour_fonthighlight);
		} else {
			psy_ui_settextcolour(g, self->colour_font);
		}
		sequencetrackbox_drawtext(self, g,
			r.left + (int)(self->tm.tmAveCharWidth * 8), r.top, "M");
		psy_ui_settextcolour(g, self->colour_font);
		if (self->showname) {
			sequencetrackbox_drawtext(self, g,
				r.left + (int)(self->tm.tmAveCharWidth * 11), r.top,
				self->track->name);
		}
	} else {		
		psy_ui_setcolour(g, self->colour);
		psy_ui_drawrectangle(g, self->position);
		psy_ui_settextcolour(g, self->colour);
		sequencetrackbox_drawtext(self, g,
			self->position.left +
				(int)(((self->position.right - self->position.left) -
				self->tm.tmAveCharWidth * 14) / 2),
			self->position.top + centery,
			"+ New Track");
	}
}

void sequencetrackbox_drawtext(SequenceTrackBox* self,
	psy_ui_Graphics* g, intptr_t x, intptr_t y, const char* text)
{
	psy_ui_textout(g, x + 3, y, text, psy_strlen(text));
}

SequenceTrackBoxEvent sequencetrackbox_hittest(const SequenceTrackBox* self,
	intptr_t x, intptr_t y)
{		
	if (self->trackindex != 0 && x >= self->position.right -
			(int)(self->tm.tmAveCharWidth * 3.5)) {
		return SEQUENCETRACKBOXEVENT_DEL;
	} else if (x >= (int)(self->tm.tmAveCharWidth * 11)) {
		return SEQUENCETRACKBOXEVENT_SELECT;
	} else if (x >= (int)(self->tm.tmAveCharWidth * 8)) {
		return SEQUENCETRACKBOXEVENT_MUTE;
	} else if (x >= (int)(self->tm.tmAveCharWidth * 5)) {
		return SEQUENCETRACKBOXEVENT_SOLO;
	}
	return SEQUENCETRACKBOXEVENT_SELECT;
}
// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "sequencetrackbox.h"
// host
#include "styles.h"
// platform
#include "../../detail/portable.h"

// prototypes
static void sequencetrackbox_drawtext(SequenceTrackBox*,
	psy_ui_Graphics*, double x, double y, const char* text);
// implementation
void sequencetrackbox_init(SequenceTrackBox* self,
	psy_ui_RealRectangle position, const psy_ui_TextMetric* tm,
	psy_audio_SequenceTrack* track,
	psy_audio_Sequence* sequence,
	uintptr_t trackindex, bool selected, bool hover)
{
	self->position = position;
	self->tm = tm;
	self->track = track;			
	self->track = track;
	self->sequence = sequence;
	self->trackindex = trackindex;
	self->selected = selected;
	self->hover = hover;
	self->showname = FALSE;
	self->colour = psy_ui_colour_make(0x00303030);
	self->colour_highlight = psy_ui_colour_make(0x00545454);
	self->colour_font = psy_ui_colour_make(0x00B2B2B2);
	self->colour_fonthighlight = psy_ui_colour_make(0x00B1C8B0);	
}

void sequencetrackbox_draw(SequenceTrackBox* self, psy_ui_Graphics* g)
{	
	psy_ui_RealRectangle r;
	char text[64];		
	double centery;
	
	centery = (self->position.bottom - self->position.top - self->tm->tmHeight) / 2;
	r = self->position;
	if (self->track) {
		int styletype;
		
		if (self->selected) {
			styletype = STYLE_SEQ_TAB_SELECT;
		} else if (self->hover) {
			styletype = STYLE_SEQ_TAB_HOVER;
		} else {
			styletype = STYLE_SEQ_TAB;
		}			
		if (psy_ui_style(styletype)->backgroundcolour.mode.set) {
			psy_ui_drawsolidrectangle(g, r,
				psy_ui_style(styletype)->backgroundcolour);
		}		
		psy_ui_drawborder(g, r, &psy_ui_style(styletype)->border);
		psy_snprintf(text, 64, "%.2X", (int)self->trackindex);
		r.top += centery;
		r.bottom = self->tm->tmHeight;
		psy_ui_settextcolour(g, psy_ui_style(styletype)->colour);
		sequencetrackbox_drawtext(self, g, r.left +
			(intptr_t)(self->tm->tmAveCharWidth * 0.2), r.top, text);
		if (psy_audio_sequence_istracksoloed(self->sequence, self->trackindex)) {
			psy_ui_settextcolour(g, self->colour_fonthighlight);
		} else {
			psy_ui_settextcolour(g, self->colour_font);
		}
		sequencetrackbox_drawtext(self, g, r.left +
			(intptr_t)(self->tm->tmAveCharWidth * 5), r.top, "S");
		if (self->trackindex != 0) {
			sequencetrackbox_drawtext(self, g, self->position.right -
				(int)(self->tm->tmAveCharWidth * 3.5), r.top, "X");
		}
		if (psy_audio_sequence_istrackmuted(self->sequence, self->trackindex)) {		
			psy_ui_settextcolour(g, self->colour_fonthighlight);
		} else {
			psy_ui_settextcolour(g, self->colour_font);
		}
		sequencetrackbox_drawtext(self, g,
			r.left + (int)(self->tm->tmAveCharWidth * 8), r.top, "M");
		psy_ui_settextcolour(g, self->colour_font);
		if (self->showname) {
			sequencetrackbox_drawtext(self, g,
				r.left + (int)(self->tm->tmAveCharWidth * 11), r.top,
				self->track->name);
		}
	} else {		
		if (psy_ui_style(STYLE_SEQ_TAB_NEW)->backgroundcolour.mode.set) {
			psy_ui_drawsolidrectangle(g, r,
				psy_ui_style(STYLE_SEQ_TAB_NEW)->backgroundcolour);
		}		
		psy_ui_settextcolour(g, psy_ui_style(STYLE_SEQ_TAB_NEW)->colour);
		sequencetrackbox_drawtext(self, g,
			self->position.left +
				(int)(((self->position.right - self->position.left) -
				self->tm->tmAveCharWidth * 14) / 2),
			self->position.top + centery,
			psy_ui_translate("sequencerview.new-trk"));
	}
}

void sequencetrackbox_drawtext(SequenceTrackBox* self,
	psy_ui_Graphics* g, double x, double y, const char* text)
{
	psy_ui_textout(g, x + 3, y, text, psy_strlen(text));
}

SequenceTrackBoxEvent sequencetrackbox_hittest(const SequenceTrackBox* self,
	psy_ui_RealPoint pt)
{		
	if (self->trackindex != 0 && pt.x >= self->position.right -
			(int)(self->tm->tmAveCharWidth * 3.5)) {
		return SEQUENCETRACKBOXEVENT_DEL;
	} else if (pt.x >= (int)(self->tm->tmAveCharWidth * 11)) {
		return SEQUENCETRACKBOXEVENT_SELECT;
	} else if (pt.x >= (int)(self->tm->tmAveCharWidth * 8)) {
		return SEQUENCETRACKBOXEVENT_MUTE;
	} else if (pt.x >= (int)(self->tm->tmAveCharWidth * 5)) {
		return SEQUENCETRACKBOXEVENT_SOLO;
	}
	return SEQUENCETRACKBOXEVENT_SELECT;
}
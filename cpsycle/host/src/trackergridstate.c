// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

// prefix file for each .c file
#include "../../detail/prefix.h"

#include "trackerview.h"
// audio
#include <pattern.h>
// std
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <assert.h>
// platform
#include "../../detail/trace.h"
#include "../../detail/portable.h"

// TrackerConfig
// implementation
void trackconfig_init(TrackConfig* self, bool wideinst)
{	
	self->textwidth = 9;
	self->textleftedge = 2;
	self->textleftedge = 2;
	self->patterntrackident = 0;
	self->headertrackident = 0;
	psy_table_init(&self->trackconfigs);
	trackconfig_initcolumns(self, wideinst);
}

void trackconfig_dispose(TrackConfig* self)
{
	psy_table_disposeall(&self->trackconfigs, (psy_fp_disposefunc)NULL);
}

void trackconfig_initcolumns(TrackConfig* self, bool wideinst)
{
	trackdef_init(&self->trackdef);
	if (wideinst) {
		self->trackdef.inst.numdigits = 4;
		self->trackdef.inst.numchars = 4;
	}
}

// TrackerGridState
// implementation
void trackerpianogridstate_init(TrackerGridState* self, TrackConfig* trackconfig)
{
	// init signals
	psy_signal_init(&self->signal_cursorchanged);
	// set references
	self->trackconfig = trackconfig;
	self->pattern = NULL;
	self->patterns = NULL;
	self->sequence = NULL;
	self->skin = NULL;	
	// init internal data
	psy_audio_patterncursor_init(&self->cursor);	
	self->numtracks = 16;
	self->singlemode = TRUE;
}

void trackergridstate_dispose(TrackerGridState* self)
{
	psy_signal_dispose(&self->signal_cursorchanged);
}

uintptr_t trackergridstate_paramcol(TrackerGridState* self, uintptr_t track,
	double x)
{	
	uintptr_t rv;
	double trackx;
	double maincolumnwidth;
	TrackDef* trackdef;

	trackdef = trackergridstate_trackdef(self, track);
	trackx = trackergridstate_tracktopx(self, track);
	maincolumnwidth = trackergridstate_basewidth(self, track);
	rv = (uintptr_t)((x - (trackx + maincolumnwidth)) / (4 * self->trackconfig->textwidth));
	return rv;
}


// TrackColumnDef
// implementation
void trackcolumndef_init(TrackColumnDef* self, int numdigits, int numchars,
	int marginright, int wrapeditcolumn, int wrapclearcolumn, int emptyvalue)
{
	self->numdigits = numdigits;
	self->numchars = numchars;
	self->marginright = marginright;
	self->wrapeditcolumn = wrapeditcolumn;
	self->wrapclearcolumn = wrapclearcolumn;
	self->emptyvalue = emptyvalue;
}

// TrackDef
void trackdef_init(TrackDef* self)
{
	trackcolumndef_init(&self->note, 1, 3, 1,
		TRACKER_COLUMN_NOTE, TRACKER_COLUMN_NOTE, 0xFF);
	trackcolumndef_init(&self->inst, 2, 2, 1,
		TRACKER_COLUMN_INST, TRACKER_COLUMN_INST, psy_audio_NOTECOMMANDS_INST_EMPTY);
	trackcolumndef_init(&self->mach, 2, 2, 1,
		TRACKER_COLUMN_MACH, TRACKER_COLUMN_MACH, psy_audio_NOTECOMMANDS_MACH_EMPTY);
	trackcolumndef_init(&self->vol, 2, 2, 1,
		TRACKER_COLUMN_VOL, TRACKER_COLUMN_VOL, psy_audio_NOTECOMMANDS_VOL_EMPTY);
	trackcolumndef_init(&self->cmd, 2, 2, 0,
		TRACKER_COLUMN_NONE, TRACKER_COLUMN_CMD, 0x00);
	trackcolumndef_init(&self->param, 2, 2, 1,
		TRACKER_COLUMN_CMD, TRACKER_COLUMN_PARAM, 0x00);
	self->numfx = 1;
}


double trackergridstate_trackwidth(TrackerGridState* self, uintptr_t track)
{
	TrackDef* trackdef;
	
	trackdef = trackergridstate_trackdef(self, track);
	if (trackdef) {
		return trackdef_width(trackdef,
			(self->trackconfig)
				? self->trackconfig->textwidth
				: 12) + 1;
	}
	return trackergridstate_preferredtrackwidth(self);	
}


double trackergridstate_tracktopx(TrackerGridState* self, uintptr_t track)
{
	double rv = 0;
	uintptr_t t;

	for (t = 0; t < track; ++t) {
		rv += trackergridstate_trackwidth(self, t);
	}
	return rv;
}

uintptr_t trackergridstate_pxtotrack(TrackerGridState* self, double x, uintptr_t numsongtracks)
{
	double currx = 0;
	uintptr_t rv = 0;

	while (rv < numsongtracks) {
		currx += trackergridstate_trackwidth(self, rv);
		if (currx > x) {
			break;
		}
		++rv;
	}
	return rv;
}

double trackergridstate_basewidth(TrackerGridState* self, uintptr_t track)
{
	return trackdef_basewidth(trackergridstate_trackdef(self, track),
		self->trackconfig->textwidth) + 1;
}

TrackDef* trackergridstate_trackdef(TrackerGridState* self, uintptr_t track)
{
	if (self->trackconfig) {
		TrackDef* rv;

		rv = psy_table_at(&self->trackconfig->trackconfigs, track);
		if (!rv) {
			rv = &self->trackconfig->trackdef;
		}
		return rv;
	}
	return NULL;
}

uintptr_t trackdef_numdigits(TrackDef* self, uintptr_t column)
{
	TrackColumnDef* coldef;

	coldef = trackdef_columndef(self, column);
	return (coldef)
		? coldef->numdigits
		: 0;
}

uintptr_t trackdef_numcolumns(TrackDef* self)
{
	return 4 + self->numfx * 2;
}

void trackdef_setvalue(TrackDef* self, uintptr_t column,
	psy_audio_PatternEntry* entry, uintptr_t value)
{
	if (column < TRACKER_COLUMN_CMD) {
		switch (column) {
		case TRACKER_COLUMN_NOTE:
			psy_audio_patternentry_front(entry)->note = (uint8_t)value;
			break;
		case TRACKER_COLUMN_INST:
			if ((self->inst.numchars == 2) && value == 0xFF) {
				// this also clears the high byte of the 16bit instrument
				// value, if only two digits are visible
				// (settings wideinstrumentcolum: off)
				psy_audio_patternentry_front(entry)->inst = 0xFFFF;
			} else {
				psy_audio_patternentry_front(entry)->inst = (uint16_t)value;
			}
			break;
		case TRACKER_COLUMN_MACH:
			psy_audio_patternentry_front(entry)->mach = (uint8_t)value;
			break;
		case TRACKER_COLUMN_VOL:
			psy_audio_patternentry_front(entry)->vol = (uint16_t)value;
			break;
		default:
			break;
		}
	} else {
		uintptr_t c;
		uintptr_t num;
		psy_List* p;

		column = column - 4;
		num = column / 2;
		c = 0;
		p = entry->events;
		while (c <= num) {
			if (!p) {
				psy_audio_PatternEvent ev;

				psy_audio_patternevent_clear(&ev);
				psy_audio_patternentry_addevent(entry, &ev);
				p = (entry->events)
					? entry->events->tail
					: NULL;
			}
			if (c == num) {
				break;
			}
			if (p) {
				p = p->next;
			}
			++c;
		}
		if (p) {
			psy_audio_PatternEvent* event;

			event = (psy_audio_PatternEvent*)p->entry;
			assert(event);
			if ((column % 2) == 0) {
				event->cmd = (uint8_t)value;
			} else {
				event->parameter = (uint8_t)value;
			}
		}
	}
}

uintptr_t trackdef_value(TrackDef* self, uintptr_t column,
	const psy_audio_PatternEntry* entry)
{
	uintptr_t rv;

	if (column < TRACKER_COLUMN_CMD) {
		switch (column) {
		case TRACKER_COLUMN_NOTE:
			rv = psy_audio_patternentry_front_const(entry)->note;
			break;
		case TRACKER_COLUMN_INST:
			rv = psy_audio_patternentry_front_const(entry)->inst;
			break;
		case TRACKER_COLUMN_MACH:
			rv = psy_audio_patternentry_front_const(entry)->mach;
			break;
		case TRACKER_COLUMN_VOL:
			rv = psy_audio_patternentry_front_const(entry)->vol;
			break;
		default:
			rv = 0;
			break;
		}
	} else {
		uintptr_t c;
		uintptr_t num;
		psy_List* p;

		column = column - 4;
		num = column / 2;
		c = 0;
		p = entry->events;
		while (p && c < num) {
			p = p->next;
			++c;
		}
		if (p) {
			psy_audio_PatternEvent* ev;

			ev = (psy_audio_PatternEvent*)p->entry;
			assert(ev);
			if ((column % 2) == 0) {
				rv = ev->cmd;
			} else {
				rv = ev->parameter;
			}
		} else {
			rv = 0;
		}
	}
	return rv;
}

uintptr_t trackdef_emptyvalue(TrackDef* self, uintptr_t column)
{
	TrackColumnDef* coldef;

	coldef = trackdef_columndef(self, column);
	return coldef ? coldef->emptyvalue : 0;
}

double trackdef_basewidth(TrackDef* self, double textwidth)
{
	double rv = 0;
	uintptr_t column;

	for (column = 0; column < 4; ++column) {
		rv += trackdef_columnwidth(self, column, textwidth);
	}
	return rv;
}

double trackdef_width(TrackDef* self, double textwidth)
{
	double rv = 0;
	uintptr_t column;

	for (column = 0; column < trackdef_numcolumns(self); ++column) {
		rv += trackdef_columnwidth(self, column, textwidth);
	}
	return rv;
}

double trackdef_columnwidth(TrackDef* self, intptr_t column, double textwidth)
{
	TrackColumnDef* coldef;

	coldef = trackdef_columndef(self, column);
	return coldef ? coldef->numchars * textwidth + coldef->marginright : 0;
}

TrackColumnDef* trackdef_columndef(TrackDef* self, intptr_t column)
{
	TrackColumnDef* rv;

	if (column >= TRACKER_COLUMN_CMD) {
		if (column % 2 == 0) {
			column = TRACKER_COLUMN_CMD;
		} else {
			column = TRACKER_COLUMN_PARAM;
		}
	}
	switch (column) {
	case TRACKER_COLUMN_NOTE:
		rv = &self->note;
		break;
	case TRACKER_COLUMN_INST:
		rv = &self->inst;
		break;
	case TRACKER_COLUMN_MACH:
		rv = &self->mach;
		break;
	case TRACKER_COLUMN_VOL:
		rv = &self->vol;
		break;
	case TRACKER_COLUMN_CMD:
		rv = &self->cmd;
		break;
	case TRACKER_COLUMN_PARAM:
		rv = &self->param;
		break;
	default:
		rv = 0;
		break;
	}
	return rv;
}

void trackergridstate_synccursor(TrackerGridState* self)
{
	assert(self);

	psy_signal_emit(&self->signal_cursorchanged, self, 0);
}

void trackergridstate_setcursor(TrackerGridState* self,
	psy_audio_PatternCursor cursor)
{
	self->cursor = cursor;
	trackergridstate_synccursor(self);
}

void trackergridstate_clip(TrackerGridState* self, const psy_ui_RealRectangle* clip,
	psy_audio_PatternSelection* rv)
{
	assert(self);

	rv->topleft.track = trackergridstate_pxtotrack(self, clip->left, self->numtracks);
	rv->topleft.column = 0;
	rv->topleft.digit = 0;		
	rv->bottomright.track = trackergridstate_pxtotrack(self, clip->right, self->numtracks) + 1;
	if (rv->bottomright.track > self->numtracks) {
		rv->bottomright.track = self->numtracks;
	}
	rv->bottomright.column = 0;
	rv->bottomright.digit = 0;	
}
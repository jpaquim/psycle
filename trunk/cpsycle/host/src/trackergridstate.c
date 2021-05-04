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
	self->colresize = FALSE;
	self->noteresize = FALSE;
	self->resizetrack = psy_INDEX_INVALID;
	psy_ui_realsize_init(&self->resizesize);
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

// TrackerEventTable
void trackereventtable_init(TrackerEventTable* self)
{
	psy_table_init(&self->tracks);
	self->currcursorline = 0;
	self->currplaybarline = 0;
}

void trackereventtable_dispose(TrackerEventTable* self)
{
	psy_TableIterator it;

	assert(self);

	for (it = psy_table_begin(&self->tracks);
			!psy_tableiterator_equal(&it, psy_table_end());
			psy_tableiterator_inc(&it)) {
		psy_List** track;

		track = psy_tableiterator_value(&it);
		psy_list_free(*track);
		free(psy_tableiterator_value(&it));
	}
	psy_table_dispose(&self->tracks);
}

void trackereventtable_clearevents(TrackerEventTable* self)
{
	psy_TableIterator it;

	assert(self);	
	
	for (it = psy_table_begin(&self->tracks);
			!psy_tableiterator_equal(&it, psy_table_end());
			psy_tableiterator_inc(&it)) {
		psy_List** track;

		track = psy_tableiterator_value(&it);
		psy_list_free(*track);
		*track = NULL;		
	}	
}

psy_List** trackereventtable_track(TrackerEventTable* self, uintptr_t index)
{
	psy_List** rv;

	rv = (psy_List**)psy_table_at(&self->tracks, index);
	if (!rv) {
		rv = (psy_List**)malloc(sizeof(psy_List**));
		assert(rv);
		*rv = NULL;
		psy_table_insert(&self->tracks, index, (void*)rv);
	}
	return rv;
}

// TrackerGridState
// implementation
void trackergridstate_init(TrackerGridState* self, TrackConfig* trackconfig,
	psy_audio_Patterns* patterns, psy_audio_Sequence* sequence)
{
	// init signals
	psy_signal_init(&self->signal_cursorchanged);
	// set references
	self->trackconfig = trackconfig;
	self->pattern = NULL;
	self->patterns = patterns;
	self->sequence = sequence;
	self->skin = NULL;	
	self->drawbeathighlights = TRUE;
	self->synccursor = TRUE;
	self->showresizecursor = TRUE;
	// init internal data
	psy_audio_patterncursor_init(&self->cursor);
	psy_audio_patterncursor_init(&self->dragcursor);
	self->singlemode = TRUE;
	self->showemptydata = FALSE;
	self->midline = FALSE;
	self->playbar = TRUE;
	trackereventtable_init(&self->trackevents);
	psy_audio_patternentry_init(&self->empty);
}

void trackergridstate_dispose(TrackerGridState* self)
{
	psy_signal_dispose(&self->signal_cursorchanged);
	trackereventtable_dispose(&self->trackevents);
	psy_audio_patternentry_dispose(&self->empty);
}

// TrackColumnDef
// implementation
void trackcolumndef_init(TrackColumnDef* self, int numdigits, int numchars,
	double marginright, int wrapeditcolumn, int wrapclearcolumn, int emptyvalue)
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
		TRACKER_COLUMN_MACH, TRACKER_COLUMN_MACH, psy_audio_NOTECOMMANDS_psy_audio_EMPTY);
	trackcolumndef_init(&self->vol, 2, 2, 1,
		TRACKER_COLUMN_VOL, TRACKER_COLUMN_VOL, psy_audio_NOTECOMMANDS_VOL_EMPTY);
	trackcolumndef_init(&self->cmd, 2, 2, 0,
		TRACKER_COLUMN_NONE, TRACKER_COLUMN_CMD, 0x00);
	trackcolumndef_init(&self->param, 2, 2, 1,
		TRACKER_COLUMN_CMD, TRACKER_COLUMN_PARAM, 0x00);
	self->numfx = 1;
	self->numnotes = 1;
}


double trackergridstate_trackwidth(const TrackerGridState* self, uintptr_t track)
{
	TrackDef* trackdef;
	
	trackdef = trackergridstate_trackdef((TrackerGridState*)self, track);
	if (trackdef) {
		return trackdef_width(trackdef,
			(self->trackconfig)
				? self->trackconfig->textwidth
				: 12) + 1;
	}
	return trackergridstate_preferredtrackwidth(self);	
}

double trackergridstate_defaulttrackwidth(const TrackerGridState* self)
{
	TrackDef* trackdef;

	if (self->trackconfig) {
		trackdef = &self->trackconfig->trackdef;		
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

uintptr_t trackergridstate_pxtotrack(const TrackerGridState* self, double x)
{
	double currx = 0;
	uintptr_t rv = 0;

	while (rv < trackergridstate_numsongtracks(self)) {
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
	rv = self->numnotes * rv;
	return rv;
}

double trackdef_columnwidth(TrackDef* self, intptr_t column, double textwidth)
{
	TrackColumnDef* coldef;

	coldef = trackdef_columndef(self, column);
	return coldef ? coldef->numchars * textwidth + coldef->marginright : 0;
}

double trackdef_marginright(TrackDef* self, intptr_t column)
{
	TrackColumnDef* coldef;

	coldef = trackdef_columndef(self, column);
	return coldef ? coldef->marginright : 0;
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

	rv->topleft.track = trackergridstate_pxtotrack(self, clip->left);
	rv->topleft.column = 0;
	rv->topleft.digit = 0;		
	rv->bottomright.track = trackergridstate_pxtotrack(self, clip->right) + 1;
	if (rv->bottomright.track > trackergridstate_numsongtracks(self)) {
		rv->bottomright.track = trackergridstate_numsongtracks(self);
	}
	rv->bottomright.column = 0;
	rv->bottomright.digit = 0;	
}

void trackergridstate_startdragselection(TrackerGridState* self,
	psy_audio_PatternCursor cursor, double bpl)
{
	assert(self);

	psy_audio_patternselection_enable(&self->selection);
	self->selection.topleft = cursor;
	self->selection.bottomright = cursor;
	if (cursor.track >= self->dragselectionbase.track) {
		self->selection.topleft.track = self->dragselectionbase.track;
		self->selection.bottomright.track = cursor.track;
	} else {
		self->selection.topleft.track = cursor.track;
		self->selection.bottomright.track = self->dragselectionbase.track;
	}
	if (cursor.offset >= self->dragselectionbase.offset) {
		self->selection.topleft.offset = self->dragselectionbase.offset;
		self->selection.bottomright.offset = cursor.offset + bpl;
	} else {
		self->selection.topleft.offset = cursor.offset;
		self->selection.bottomright.offset = self->dragselectionbase.offset +
			bpl;
	}
	self->selection.bottomright.track += 1;
}

void trackergridstate_dragselection(TrackerGridState* self, psy_audio_PatternCursor cursor,
	double bpl)
{
	assert(self);

	if (cursor.track >= self->dragselectionbase.track) {
		self->selection.topleft.track = self->dragselectionbase.track;
		self->selection.bottomright.track = cursor.track + 1;
	} else {
		self->selection.topleft.track = cursor.track;
		self->selection.bottomright.track = self->dragselectionbase.track + 1;
	}
	if (cursor.offset >= self->dragselectionbase.offset) {
		self->selection.topleft.offset = self->dragselectionbase.offset;
		self->selection.bottomright.offset = cursor.offset + bpl;
	} else {
		self->selection.topleft.offset = cursor.offset;
		self->selection.bottomright.offset = self->dragselectionbase.offset + bpl;
	}	
}

psy_audio_PatternCursor trackergridstate_checkcursorbounds(TrackerGridState* self,
	psy_audio_PatternCursor cursor)
{
	psy_audio_PatternCursor rv;

	rv = cursor;
	if (rv.offset < 0) {
		rv.offset = 0;
	} else {
		if (self->pattern) {
			if (rv.offset >= psy_audio_pattern_length(self->pattern)) {
				rv.offset = psy_audio_pattern_length(self->pattern);
			}
		} else {
			rv.offset = 0;
		}
	}
	if (rv.track < 0) {
		rv.track = 0;
	} else if (rv.track >= trackergridstate_numsongtracks(self)) {
		rv.track = trackergridstate_numsongtracks(self);
	}
	return rv;
}

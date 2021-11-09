/*
** This source is free software; you can redistribute itand /or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2, or (at your option) any later version.
** copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "trackerview.h"
/* audio */
#include <pattern.h>
/* std */
#include <math.h>
#include <assert.h>
/* platform */
#include "../../detail/trace.h"
#include "../../detail/portable.h"

/*
** TrackerConfig
** implementation
*/
void trackconfig_init(TrackConfig* self, bool wideinst)
{	
	self->textwidth = 9;
	self->textleftedge = 2;
	self->textleftedge = 2;	
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

/* TrackColumnDef */
/* implementation */
void trackcolumndef_init(TrackColumnDef* self, uintptr_t numdigits,
	uintptr_t numchars, double marginright, int wrapeditcolumn,
	int wrapclearcolumn, int emptyvalue)
{
	self->numdigits = numdigits;
	self->numchars = numchars;
	self->marginright = marginright;
	self->wrapeditcolumn = wrapeditcolumn;
	self->wrapclearcolumn = wrapclearcolumn;
	self->emptyvalue = emptyvalue;
}

/* TrackDef */
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


/* TrackerEventTable */
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

		track = (psy_List**)psy_tableiterator_value(&it);
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

		track = (psy_List**)psy_tableiterator_value(&it);
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

/* TrackerState */
/* implementation */
void trackerstate_init(TrackerState* self, TrackConfig* trackconfig,
	psy_audio_Song* song)
{		
	patternviewstate_init(&self->pv, song);	
	self->trackconfig = trackconfig;	
	self->drawbeathighlights = TRUE;
	self->synccursor = TRUE;
	self->showresizecursor = TRUE;		
	self->showemptydata = FALSE;
	self->midline = FALSE;	
	trackereventtable_init(&self->trackevents);
	psy_audio_patternentry_init(&self->empty);
	self->defaultlineheight = psy_ui_value_make_eh(1.0);
	self->lineheight = self->defaultlineheight;
	self->lineheightpx = 19.0;
	self->flatsize = 8;			
	self->drawcursor = TRUE;
	self->visilines = 25;
	self->cursorchanging = FALSE;
	self->gridfont = NULL;	
}

void trackerstate_dispose(TrackerState* self)
{
	trackereventtable_dispose(&self->trackevents);
	psy_audio_patternentry_dispose(&self->empty);
}

double trackerstate_trackwidth(const TrackerState* self, uintptr_t track)
{
	TrackDef* trackdef;
	
	trackdef = trackerstate_trackdef((TrackerState*)self, track);
	if (trackdef) {
		return trackdef_width(trackdef,
			(self->trackconfig)
				? self->trackconfig->textwidth
				: 12) + 1;
	}
	return trackerstate_preferredtrackwidth(self);	
}

double trackerstate_defaulttrackwidth(const TrackerState* self)
{
	TrackDef* trackdef;

	if (self->trackconfig) {
		trackdef = &self->trackconfig->trackdef;		
		return trackdef_width(trackdef,
			(self->trackconfig)
			? self->trackconfig->textwidth
			: 12) + 1;		
	}
	return trackerstate_preferredtrackwidth(self);
}

uintptr_t trackerstate_pxtotrack(const TrackerState* self, double x)
{
	double currx = 0;
	uintptr_t rv = 0;

	while (rv < patternviewstate_numsongtracks(&self->pv)) {
		currx += trackerstate_trackwidth(self, rv);
		if (currx > x) {
			break;
		}
		++rv;
	}
	return rv;
}

double trackerstate_basewidth(TrackerState* self, uintptr_t track)
{
	return trackdef_basewidth(trackerstate_trackdef(self, track),
		self->trackconfig->textwidth) + 1;
}

TrackDef* trackerstate_trackdef(TrackerState* self, uintptr_t track)
{
	if (self->trackconfig) {
		TrackDef* rv;

		rv = (TrackDef*)psy_table_at(&self->trackconfig->trackconfigs, track);
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
				/*
				** this also clears the high byte of the 16bit instrument
				** value, if only two digits are visible
				** (settings wideinstrumentcolum: off)
				*/
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

void trackerstate_clip(TrackerState* self, const psy_ui_RealRectangle* clip,
	psy_audio_BlockSelection* rv)
{
	assert(self);

	rv->topleft.track = trackerstate_pxtotrack(self, clip->left);
	rv->topleft.column = 0;
	rv->topleft.digit = 0;
	rv->topleft.lpb = (uintptr_t)(1.0 / trackerstate_bpl(self));
	rv->bottomright.track = trackerstate_pxtotrack(self, clip->right) + 1;
	if (rv->bottomright.track > patternviewstate_numsongtracks(&self->pv)) {
		rv->bottomright.track = patternviewstate_numsongtracks(&self->pv);
	}
	rv->bottomright.column = 0;
	rv->bottomright.digit = 0;
	rv->bottomright.lpb = trackerstate_lpb(self);
}

void trackerstate_startdragselection(TrackerState* self,
	psy_audio_SequenceCursor cursor, double bpl)
{
	assert(self);

	psy_audio_blockselection_startdrag(&self->pv.selection,
		self->pv.dragselectionbase, cursor, bpl);
}

void trackerstate_dragselection(TrackerState* self, psy_audio_SequenceCursor cursor,
	double bpl)
{
	assert(self);

	psy_audio_blockselection_drag(&self->pv.selection,
		self->pv.dragselectionbase, cursor, bpl);
}

psy_audio_SequenceCursor trackerstate_checkcursorbounds(TrackerState* self,
	psy_audio_SequenceCursor cursor)
{
	psy_audio_SequenceCursor rv;

	rv = cursor;
	if (rv.offset < 0) {
		rv.offset = 0;
	} else {
		if (self->pv.pattern) {
			if (rv.offset >= psy_audio_pattern_length(self->pv.pattern)) {
				rv.offset = psy_audio_pattern_length(self->pv.pattern);
			}
		} else {
			rv.offset = 0;
		}
	}
	if (rv.track < 0) {
		rv.track = 0;
	} else if (rv.track >= patternviewstate_numsongtracks(&self->pv)) {
		rv.track = patternviewstate_numsongtracks(&self->pv);
	}
	return rv;
}

void trackerstate_selectcol(TrackerState* self)
{
	assert(self);

	if (patternviewstate_pattern(&self->pv)) {
		self->pv.selection.topleft.offset = 0;
		self->pv.selection.topleft.track = self->pv.cursor.track;
		self->pv.selection.bottomright.offset = patternviewstate_pattern(&self->pv)->length;
		self->pv.selection.bottomright.track = self->pv.cursor.track + 1;
		psy_audio_blockselection_enable(&self->pv.selection);
	}
}

void trackerstate_selectbar(TrackerState* self)
{
	assert(self);

	if (patternviewstate_pattern(&self->pv)) {
		self->pv.selection.topleft.offset = self->pv.cursor.offset;
		self->pv.selection.topleft.track = self->pv.cursor.track;
		self->pv.selection.bottomright.offset = self->pv.cursor.offset + 4.0;
		if (self->pv.cursor.offset > patternviewstate_pattern(&self->pv)->length) {
			self->pv.cursor.offset = patternviewstate_pattern(&self->pv)->length;
		}
		self->pv.selection.bottomright.track = self->pv.cursor.track + 1;
		psy_audio_blockselection_enable(&self->pv.selection);
	}
}

void trackerstate_selectall(TrackerState* self)
{
	assert(self);

	if (patternviewstate_pattern(&self->pv)) {
		psy_audio_blockselection_init(&self->pv.selection);
		self->pv.selection.topleft.key = psy_audio_NOTECOMMANDS_B9;
		self->pv.selection.bottomright.offset = psy_audio_pattern_length(
			patternviewstate_pattern(&self->pv));
		self->pv.selection.bottomright.track =
			patternviewstate_numsongtracks(&self->pv);
		psy_audio_blockselection_enable(&self->pv.selection);
	}
}

ScrollDir trackerstate_nextcol(TrackerState* self, bool wrap)
{
	if (self->pv.pattern) {
		TrackDef* trackdef;
		int invalidate = 1;

		assert(self);

		trackdef = trackerstate_trackdef(self, self->pv.cursor.track);
		if (self->pv.cursor.column == trackdef_numcolumns(trackdef) - 1 &&
			self->pv.cursor.digit == trackdef_numdigits(trackdef,
				self->pv.cursor.column) - 1) {
			if (self->pv.cursor.track < patternviewstate_numsongtracks(&self->pv) - 1) {
				self->pv.cursor.column = 0;
				self->pv.cursor.digit = 0;
				++self->pv.cursor.track;
				return SCROLL_DIR_RIGHT;				
			} else if (wrap) {
				self->pv.cursor.column = 0;
				self->pv.cursor.digit = 0;
				self->pv.cursor.track = 0;
				return SCROLL_DIR_LEFT;				
			}
		} else {
			++self->pv.cursor.digit;
			if (self->pv.cursor.digit >=
				trackdef_numdigits(trackdef, self->pv.cursor.column)) {
				++self->pv.cursor.column;
				self->pv.cursor.digit = 0;
			}
		}		
	}
	return SCROLL_DIR_NONE;
}

ScrollDir trackerstate_prevcol(TrackerState* self, bool wrap)
{
	int invalidate = 1;

	assert(self);

	if (self->pv.cursor.column == 0 && self->pv.cursor.digit == 0) {
		if (self->pv.cursor.track > 0) {
			TrackDef* trackdef;

			--self->pv.cursor.track;
			trackdef = trackerstate_trackdef(self, self->pv.cursor.track);
			self->pv.cursor.column = trackdef_numcolumns(trackdef) - 1;
			self->pv.cursor.digit = trackdef_numdigits(trackdef,
				self->pv.cursor.column) - 1;
			return SCROLL_DIR_LEFT;			
		} else if (wrap) {
			TrackDef* trackdef;

			self->pv.cursor.track = patternviewstate_numsongtracks(&self->pv) - 1;
			trackdef = trackerstate_trackdef(self, self->pv.cursor.track);
			self->pv.cursor.column = trackdef_numcolumns(trackdef) - 1;
			self->pv.cursor.digit = trackdef_numdigits(trackdef,
				self->pv.cursor.column) - 1;
			return SCROLL_DIR_RIGHT;			
		}
	} else {
		if (self->pv.cursor.digit > 0) {
			--self->pv.cursor.digit;
		} else {
			TrackDef* trackdef;

			trackdef = trackerstate_trackdef(self,
				self->pv.cursor.track);
			--self->pv.cursor.column;
			self->pv.cursor.digit = trackdef_numdigits(trackdef,
				self->pv.cursor.column) - 1;
		}
	}
	return SCROLL_DIR_NONE;
}

static int testcursor(psy_audio_SequenceCursor cursor, uintptr_t track,
	psy_dsp_big_beat_t offset, uintptr_t lpb)
{
	return cursor.track == track && psy_dsp_testrange(cursor.offset, offset, 1.0 / lpb);
}

bool trackerstate_testplaybar(TrackerState* self,
	psy_dsp_big_beat_t playposition, psy_dsp_big_beat_t offset)
{
	assert(self);

	return psy_dsp_testrange(
		playposition -
			((self->pv.singlemode)
			? self->pv.cursor.seqoffset
			: 0.0),
		offset, trackerstate_bpl(self));
}

uintptr_t trackerstate_numlines(const TrackerState* self)
{
	assert(self);

	return trackerstate_beattoline(self,
		patternviewstate_length(&self->pv));
}

void trackerstate_lineclip(TrackerState* self, const psy_ui_RealRectangle* clip,
	psy_audio_BlockSelection* rv)
{
	assert(self);

	rv->topleft.column = 0;
	rv->topleft.digit = 0;
	rv->topleft.offset = trackerstate_pxtobeat(self, psy_max(0.0, clip->top));
	rv->topleft.lpb = trackerstate_lpb(self);
	rv->bottomright.column = 0;
	rv->bottomright.digit = 0;
	rv->bottomright.offset = trackerstate_pxtobeatnotquantized(self,
		psy_max(0.0, clip->bottom));	
	rv->bottomright.lpb = rv->topleft.lpb;
}

void trackerstate_setfont(TrackerState* self, const psy_ui_Font* font,
	const psy_ui_TextMetric* tm)
{	
	self->gridfont = font;
	self->lineheightpx = psy_max(1.0, psy_ui_value_px(
		&self->lineheight, tm, NULL));	
	self->trackconfig->textwidth = (int)(tm->tmAveCharWidth * 1.5) +
		2;	
	self->flatsize = (double)(tm->tmAveCharWidth) + 2.0;
}

psy_audio_SequenceCursor trackerstate_makecursor(TrackerState* self,
	psy_ui_RealPoint pt, uintptr_t index)
{
	psy_audio_SequenceCursor rv;
	TrackDef* trackdef;
	double cpx;
	psy_audio_Sequence* sequence;

	rv = self->pv.cursor;
	sequence = patternviewstate_sequence(&self->pv);
	if (sequence) {
		psy_audio_sequencecursor_updateseqoffset(&rv,
			sequence);
	}
	rv.absolute = !self->pv.singlemode;
	rv.offset = trackerstate_pxtobeat(self, pt.y);
	rv.lpb = trackerstate_lpb(self);
	if (patternviewstate_pattern(&self->pv) &&
		rv.offset >= psy_audio_pattern_length(patternviewstate_pattern(&self->pv))) {
		if (self->pv.singlemode) {
			rv.offset = psy_audio_pattern_length(patternviewstate_pattern(&self->pv)) -
				trackerstate_bpl(self);
		}
	}
	rv.track = index;
	rv.column = 0;
	rv.digit = 0;
	rv.key = sequence->cursor.key;
	trackdef = trackerstate_trackdef(self, rv.track);
	cpx = 0;
	while (rv.column < trackdef_numcolumns(trackdef) &&
		cpx + trackdef_columnwidth(trackdef, rv.column,
			self->trackconfig->textwidth) < pt.x) {
		cpx += trackdef_columnwidth(trackdef, rv.column,
			self->trackconfig->textwidth);
		++rv.column;
	}
	rv.digit = (uintptr_t)((pt.x - cpx) /
		self->trackconfig->textwidth);
	if (rv.digit >= trackdef_numdigits(trackdef, rv.column)) {
		rv.digit = trackdef_numdigits(trackdef, rv.column) - 1;
	}
	self->pv.cursor.patternid = sequence->cursor.patternid;
	return rv;
}

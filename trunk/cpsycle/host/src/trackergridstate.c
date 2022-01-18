/*
** This source is free software; you can redistribute itand /or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "trackerview.h"
/* audio */
#include <pattern.h>
/* platform */
#include "../../detail/portable.h"

/* TrackerConfig */
/* implementation */
void trackdrag_init(TrackDrag* self)
{	
	self->active = FALSE;	
	self->track = psy_INDEX_INVALID;
	psy_ui_realsize_init(&self->size);
}

void trackdrag_start(TrackDrag* self, uintptr_t track, double width)
{
	self->track = self->track;
	self->size.width = width;
	self->active = TRUE;
}

void trackdrag_stop(TrackDrag* self)
{
	self->active = FALSE;	
}

bool trackdrag_active(const TrackDrag* self)
{
	return (self->active);
}

bool trackdrag_trackactive(const TrackDrag* self, uintptr_t track)
{
	return (self->active && self->track == track);
}

/* TrackerConfig */
/* implementation */
void trackconfig_init(TrackConfig* self, bool wideinst)
{	
	self->textwidth = 9;
	self->textleftedge = 2;
	self->multicolumn = FALSE;
	trackdrag_init(&self->resize);	
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

double trackconfig_width(const TrackConfig* self, uintptr_t track)
{
	const TrackDef* trackdef;
	double rv;
	double cmdparam;
	uintptr_t column;	

	trackdef = trackerconfig_trackdef_const(self, track);
	cmdparam = 0.0;
	for (column = 0, rv = 0.0; column < trackdef_numcolumns(trackdef);
			++column) {
		double width;

		width = trackdef_columnwidth(trackdef, column, self->textwidth);
		if (column >= PATTERNEVENT_COLUMN_CMD) {
			cmdparam += width;
		}
		rv += width;
	}
	if (self->multicolumn) {
		return rv + (trackdef->visinotes - 1) * cmdparam + 1.0;
	}
	return trackdef->visinotes * rv + 1.0;
}

double trackconfig_width_cmdparam(const TrackConfig* self)
{
	const TrackDef* trackdef;
	double rv;	
	uintptr_t column;

	trackdef = trackerconfig_trackdef_const(self, psy_INDEX_INVALID);	
	for (column = PATTERNEVENT_COLUMN_CMD, rv = 0.0;
		column < trackdef_numcolumns(trackdef);
		++column) {
		double width;

		width = trackdef_columnwidth(trackdef, column, self->textwidth);		
		rv += width;
	}	
	return rv;
}

TrackDef* trackconfig_insert_trackdef(TrackConfig* self, uintptr_t track,
	uintptr_t numnotes)
{
	TrackDef* rv;
	
	rv = trackdef_allocinit();
	if (rv) {
		trackdef_init(rv);		
		rv->visinotes = numnotes;
		psy_table_insert(&self->trackconfigs, track, rv);		
	}
	return rv;
}

void trackconfig_settrack(TrackConfig* self, uintptr_t track,
	uintptr_t numnotes)
{
	TrackDef* trackdef;

	numnotes = psy_max(1, numnotes);	
	trackdef = trackerconfig_trackdef(self, track);
	if (trackdef != &self->trackdef) {		
		trackdef->visinotes = numnotes;
	} else {
		trackconfig_insert_trackdef(self, track, numnotes);
	}
}

TrackDef* trackerconfig_trackdef(TrackConfig* self, uintptr_t track)
{	
	TrackDef* rv;

	rv = (TrackDef*)psy_table_at(&self->trackconfigs, track);
	if (!rv) {
		return &self->trackdef;
	}
	return rv;	
}

TrackDef* trackerconfig_trackdef_const(const TrackConfig* self, uintptr_t track)
{
	return trackerconfig_trackdef((TrackConfig*)self, track);
}

void trackconfig_resize(TrackConfig* self, uintptr_t track, double width)
{	
	if (trackdrag_active(&self->resize)) {
		TrackDef* trackdef;
		double basewidth;
		uintptr_t numnotes;

		trackdef = trackerconfig_trackdef(self, track);
		basewidth = trackdef_defaulttrackwidth(trackdef, self->textwidth);
		width = psy_max(0.0, width);
		self->resize.size.width = width;
		self->resize.track = track;
		if (self->multicolumn) {
			numnotes = (uintptr_t)psy_max(1.0,
				psy_max(0.0, (width - (basewidth - trackconfig_width_cmdparam(self)))) /
				trackconfig_width_cmdparam(self));
		} else {
			numnotes = (uintptr_t)(psy_max(1.0,
				(psy_max(0.0, self->resize.size.width)) /
				basewidth) + 0.9);
		}
		trackconfig_settrack(self, track, numnotes);
	}
}

/* TrackColumnDef */
static void enterdigit(int digit, int newval, unsigned char* val);

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
	self->visinotes = 1;
	trackcolumndef_init(&self->note, 1, 3, 1,
		PATTERNEVENT_COLUMN_NOTE, PATTERNEVENT_COLUMN_NOTE, 0xFF);
	trackcolumndef_init(&self->inst, 2, 2, 1,
		PATTERNEVENT_COLUMN_INST, PATTERNEVENT_COLUMN_INST,
		psy_audio_NOTECOMMANDS_INST_EMPTY);
	trackcolumndef_init(&self->mach, 2, 2, 1,
		PATTERNEVENT_COLUMN_MACH, PATTERNEVENT_COLUMN_MACH,
		psy_audio_NOTECOMMANDS_psy_audio_EMPTY);
	trackcolumndef_init(&self->vol, 2, 2, 1,
		PATTERNEVENT_COLUMN_VOL, PATTERNEVENT_COLUMN_VOL,
		psy_audio_NOTECOMMANDS_VOL_EMPTY);
	trackcolumndef_init(&self->cmd, 2, 2, 0,
		PATTERNEVENT_COLUMN_NONE, PATTERNEVENT_COLUMN_CMD, 0x00);
	trackcolumndef_init(&self->param, 2, 2, 1,
		PATTERNEVENT_COLUMN_CMD, PATTERNEVENT_COLUMN_PARAM, 0x00);	
}

TrackDef* trackdef_alloc(void)
{
	return (TrackDef*)malloc(sizeof(TrackDef));
}

TrackDef* trackdef_allocinit(void)
{
	TrackDef* rv;

	rv = trackdef_alloc();
	if (rv) {
		trackdef_init(rv);
	}
	return rv;
}

void trackdef_deallocate(TrackDef* self)
{
	free(self);
}

/* TrackerEventTable */
void trackereventtable_init(TrackerEventTable* self)
{
	psy_table_init(&self->tracks);
	self->currcursorline = 0;	
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
	PatternViewState* pvstate)
{			
	self->trackconfig = trackconfig;
	self->pv = pvstate;
	self->drawbeathighlights = TRUE;
	self->synccursor = TRUE;	
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
}

void trackerstate_dispose(TrackerState* self)
{
	trackereventtable_dispose(&self->trackevents);
	psy_audio_patternentry_dispose(&self->empty);
}

double trackerstate_trackwidth(const TrackerState* self, uintptr_t track)
{
	assert(self);
	assert(self->trackconfig);

	if (self->trackconfig->resize.active &&
			self->trackconfig->resize.track == track) {
		return self->trackconfig->resize.size.width;
	}
	return trackconfig_width(self->trackconfig, track);
}

uintptr_t trackerstate_pxtotrack(const TrackerState* self, double x)
{
	double currx = 0;
	uintptr_t rv = 0;

	while (rv < patternviewstate_numsongtracks(self->pv)) {
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

uintptr_t trackdef_numcolumns(const TrackDef* self)
{
	return PATTERNEVENT_COLUMN_END;
}

psy_audio_PatternEvent trackdef_setevent_digit(TrackDef* self,
	uintptr_t column, uintptr_t digit, const psy_audio_PatternEvent* ev,
	uintptr_t digitvalue)
{
	uintptr_t value;
	uintptr_t num;
	ptrdiff_t pos;
	uint8_t* data;
	psy_audio_PatternEvent rv;
	TrackColumnDef* columndef;

	assert(ev);

	rv = *ev;
	columndef = trackdef_columndef(self, column);
	if (!columndef) {
		return *ev;
	}
	if (digitvalue != 0xF && digitvalue == trackdef_emptyvalue(self, column)) {
		value = 0;
	} else {
		value = psy_audio_patternevent_value(ev, column);
	}
	if (value == trackdef_emptyvalue(self, column)) {
		value = 0;
	}
	num = trackdef_numdigits(self, column);
	pos = num / 2 - digit / 2 - 1;
	data = (uint8_t*)&value + pos;
	enterdigit(digit % 2, (uint8_t)digitvalue, data);
	psy_audio_patternevent_setvalue(&rv, column, value);
	return rv;
}

void enterdigit(int digit, int newval, unsigned char* val)
{
	if (digit == 0) {
		*val = (*val & 0x0F) | ((newval & 0x0F) << 4);
	} else if (digit == 1) {
		*val = (*val & 0xF0) | (newval & 0x0F);
	}
}

uintptr_t trackdef_event_value(TrackDef* self, uintptr_t column,
	const psy_audio_PatternEvent* ev)
{
	if (column < PATTERNEVENT_COLUMN_END) {
		switch (column) {
		case PATTERNEVENT_COLUMN_NOTE:
			return ev->note;			
		case PATTERNEVENT_COLUMN_INST:
			return ev->inst;			
		case PATTERNEVENT_COLUMN_MACH:
			return ev->mach;			
		case PATTERNEVENT_COLUMN_VOL:
			return ev->vol;	
		case PATTERNEVENT_COLUMN_CMD:
			return ev->cmd;
		case PATTERNEVENT_COLUMN_PARAM:
			return ev->parameter;
		default:			
			break;
		}
	}
	return 0;
}

uintptr_t trackdef_value(TrackDef* self, uintptr_t column,
	const psy_audio_PatternEntry* entry)
{
	uintptr_t rv;

	if (column < PATTERNEVENT_COLUMN_CMD) {
		rv = trackdef_event_value(self, column,
			psy_audio_patternentry_front_const(entry));		
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

double trackdef_basewidth(const TrackDef* self, double textwidth)
{
	double rv = 0;
	uintptr_t column;

	for (column = 0; column < 4; ++column) {
		rv += trackdef_columnwidth(self, column, textwidth);
	}
	return rv;
}

double trackdef_defaulttrackwidth(const TrackDef* self, double textwidth)
{
	double rv = 0;
	uintptr_t column;

	for (column = 0; column < PATTERNEVENT_COLUMN_END; ++column) {
		rv += trackdef_columnwidth(self, column, textwidth);
	}
	return rv;
}

double trackdef_columnwidth(const TrackDef* self, intptr_t column, double textwidth)
{
	const TrackColumnDef* coldef;

	coldef = trackdef_columndef((TrackDef*)self, column);
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
	switch (column) {
	case PATTERNEVENT_COLUMN_NOTE:
		return &self->note;		
	case PATTERNEVENT_COLUMN_INST:
		return &self->inst;		
	case PATTERNEVENT_COLUMN_MACH:
		return &self->mach;		
	case PATTERNEVENT_COLUMN_VOL:
		return &self->vol;		
	case PATTERNEVENT_COLUMN_CMD:
		return &self->cmd;		
	case PATTERNEVENT_COLUMN_PARAM:
		return &self->param;		
	default:
		return NULL;		
	}	
}

void trackerstate_clip(TrackerState* self, const psy_ui_RealRectangle* clip,
	psy_audio_BlockSelection* rv)
{
	assert(self);

	rv->topleft.track = trackerstate_pxtotrack(self, clip->left);
	rv->topleft.column = 0;
	rv->topleft.digit = 0;
	rv->topleft.lpb = patternviewstate_lpb(self->pv);
	rv->bottomright.track = trackerstate_pxtotrack(self, clip->right) + 1;
	if (rv->bottomright.track > patternviewstate_numsongtracks(self->pv)) {
		rv->bottomright.track = patternviewstate_numsongtracks(self->pv);
	}
	rv->bottomright.column = 0;
	rv->bottomright.digit = 0;
	rv->bottomright.lpb = patternviewstate_lpb(self->pv);
}

void trackerstate_startdragselection(TrackerState* self,
	psy_audio_SequenceCursor cursor, double bpl)
{
	assert(self);

	psy_audio_blockselection_startdrag(&self->pv->selection,
		self->pv->dragselectionbase, cursor, bpl);
}

void trackerstate_dragselection(TrackerState* self, psy_audio_SequenceCursor cursor,
	double bpl)
{
	assert(self);

	psy_audio_blockselection_drag(&self->pv->selection,
		self->pv->dragselectionbase, cursor, bpl);
}

psy_audio_SequenceCursor trackerstate_checkcursorbounds(TrackerState* self,
	psy_audio_SequenceCursor cursor)
{
	psy_audio_SequenceCursor rv;

	rv = cursor;
	if (rv.offset < 0) {
		rv.offset = 0;
	} else {
		if (self->pv->pattern) {
			if (rv.offset >= psy_audio_pattern_length(self->pv->pattern)) {
				rv.offset = psy_audio_pattern_length(self->pv->pattern);
			}
		} else {
			rv.offset = 0;
		}
	}
	if (rv.track < 0) {
		rv.track = 0;
	} else if (rv.track >= patternviewstate_numsongtracks(self->pv)) {
		rv.track = patternviewstate_numsongtracks(self->pv);
	}
	return rv;
}

ScrollDir trackerstate_nextcol(TrackerState* self, bool wrap)
{
	assert(self);

	if (self->pv->pattern) {
		TrackDef* trackdef;
		int invalidate = 1;		

		trackdef = trackerstate_trackdef(self, self->pv->cursor.track);
		if (self->pv->cursor.column == trackdef_numcolumns(trackdef) - 1 &&
			self->pv->cursor.digit == trackdef_numdigits(trackdef,
				self->pv->cursor.column) - 1) {
			if (self->pv->cursor.noteindex + 1 < trackdef_visinotes(trackdef)) {
				++self->pv->cursor.noteindex;
				if (!self->trackconfig->multicolumn) {
					self->pv->cursor.column = 0;
				} else {
					self->pv->cursor.column = PATTERNEVENT_COLUMN_CMD;
				}
				self->pv->cursor.digit = 0;
			} else {
				if (self->pv->cursor.track < patternviewstate_numsongtracks(self->pv) - 1) {
					self->pv->cursor.column = 0;
					self->pv->cursor.digit = 0;
					self->pv->cursor.noteindex = 0;
					++self->pv->cursor.track;
					return SCROLL_DIR_RIGHT;
				} else if (wrap) {
					self->pv->cursor.column = 0;
					self->pv->cursor.digit = 0;
					self->pv->cursor.track = 0;
					self->pv->cursor.noteindex = 0;
					return SCROLL_DIR_LEFT;
				}
			}
		} else {			
			++self->pv->cursor.digit;
			if (self->pv->cursor.digit >=
				trackdef_numdigits(trackdef, self->pv->cursor.column)) {
				++self->pv->cursor.column;
				self->pv->cursor.digit = 0;
			}			
		}		
	}
	return SCROLL_DIR_NONE;
}

ScrollDir trackerstate_prevcol(TrackerState* self, bool wrap)
{
	int invalidate = 1;

	assert(self);

	if ((self->pv->cursor.column == 0) || (self->trackconfig->multicolumn &&
				self->pv->cursor.noteindex > 0 &&
				self->pv->cursor.column == PATTERNEVENT_COLUMN_CMD) &&
			self->pv->cursor.digit == 0) {
		if (self->pv->cursor.noteindex > 0) {
			TrackDef* trackdef;

			--self->pv->cursor.noteindex;											
			trackdef = trackerstate_trackdef(self, self->pv->cursor.track);			
			self->pv->cursor.column = trackdef_numcolumns(trackdef) - 1;
			self->pv->cursor.digit = trackdef_numdigits(trackdef,
			self->pv->cursor.column) - 1;			
		} else if (self->pv->cursor.track > 0) {
			TrackDef* trackdef;

			--self->pv->cursor.track;
			trackdef = trackerstate_trackdef(self, self->pv->cursor.track);
			self->pv->cursor.noteindex = trackdef_visinotes(trackdef) - 1;
			self->pv->cursor.column = trackdef_numcolumns(trackdef) - 1;
			self->pv->cursor.digit = trackdef_numdigits(trackdef,
				self->pv->cursor.column) - 1;
			return SCROLL_DIR_LEFT;			
		} else if (wrap) {
			TrackDef* trackdef;

			self->pv->cursor.track = patternviewstate_numsongtracks(self->pv) - 1;
			trackdef = trackerstate_trackdef(self, self->pv->cursor.track);
			self->pv->cursor.column = trackdef_numcolumns(trackdef) - 1;
			self->pv->cursor.digit = trackdef_numdigits(trackdef,
				self->pv->cursor.column) - 1;
			return SCROLL_DIR_RIGHT;			
		}
	} else if (self->pv->cursor.digit > 0) {
		--self->pv->cursor.digit;
	} else {
		TrackDef* trackdef;

		trackdef = trackerstate_trackdef(self,
			self->pv->cursor.track);
		--self->pv->cursor.column;
		self->pv->cursor.digit = trackdef_numdigits(trackdef,
			self->pv->cursor.column) - 1;		
	}
	return SCROLL_DIR_NONE;
}

bool trackerstate_testplaybar(TrackerState* self,
	psy_dsp_big_beat_t playposition, psy_dsp_big_beat_t offset)
{
	assert(self);

	return psy_dsp_testrange(
		playposition -
			((self->pv->singlemode)
			? self->pv->cursor.seqoffset
			: 0.0),
		offset, patternviewstate_bpl(self->pv));
}

void trackerstate_lineclip(TrackerState* self, const psy_ui_RealRectangle* clip,
	psy_audio_BlockSelection* rv)
{
	assert(self);

	rv->topleft.column = 0;
	rv->topleft.digit = 0;
	rv->topleft.offset = trackerstate_pxtobeat(self, psy_max(0.0, clip->top));
	rv->topleft.lpb = patternviewstate_lpb(self->pv);
	rv->bottomright.column = 0;
	rv->bottomright.digit = 0;
	rv->bottomright.offset = trackerstate_pxtobeatnotquantized(self,
		psy_max(0.0, clip->bottom));	
	rv->bottomright.lpb = rv->topleft.lpb;
}

void trackerstate_updatemetric(TrackerState* self, const psy_ui_TextMetric* tm,
	double lineheight)
{		
	self->lineheight = psy_ui_value_make_eh(lineheight);
	self->lineheightpx = psy_max(1.0, psy_ui_value_px(&self->lineheight, tm,
		NULL));	
	self->trackconfig->textwidth = (int)((double)tm->tmAveCharWidth * 1.5) + 2;
	self->flatsize = (double)(tm->tmAveCharWidth) + 2.0;
}

psy_audio_SequenceCursor trackerstate_makecursor(TrackerState* self,
	psy_ui_RealPoint pt, uintptr_t index)
{
	psy_audio_SequenceCursor rv;
	TrackDef* trackdef;
	double cpx;
	psy_audio_Sequence* sequence;

	rv = self->pv->cursor;
	sequence = patternviewstate_sequence(self->pv);
	if (sequence) {
		psy_audio_sequencecursor_updateseqoffset(&rv,
			sequence);
	}
	rv.absolute = !self->pv->singlemode;
	rv.offset = trackerstate_pxtobeat(self, pt.y);
	rv.lpb = patternviewstate_lpb(self->pv);
	if (patternviewstate_pattern(self->pv) &&
		rv.offset >= psy_audio_pattern_length(patternviewstate_pattern(self->pv))) {
		if (self->pv->singlemode) {
			rv.offset = psy_audio_pattern_length(patternviewstate_pattern(self->pv)) -
				patternviewstate_bpl(self->pv);
		}
	}
	rv.track = index;
	rv.column = 0;
	rv.digit = 0;
	if (sequence) {
		rv.key = sequence->cursor.key;
	} else {
		rv.key = 0;
	}
	trackdef = trackerstate_trackdef(self, rv.track);
	if (trackdef->visinotes == 1) {		
		cpx = 0.0;
		rv.noteindex = 0;
	} else {
		rv.noteindex = (uintptr_t)(pt.x / trackdef_defaulttrackwidth(
			trackdef, self->trackconfig->textwidth));
		cpx = trackdef_defaulttrackwidth(trackdef,
			self->trackconfig->textwidth) * rv.noteindex;		
	}
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
	self->pv->cursor.patternid = sequence->cursor.patternid;
	psy_audio_sequencecursor_updatecache(&rv);
	return rv;
}

void trackerstate_columncolours(TrackerState* self,
	TrackerColumnFlags flags, uintptr_t track,
	psy_ui_Colour* bg, psy_ui_Colour* fore)
{		
	PatternViewSkin* skin;
	uintptr_t numtracks;

	assert(bg);
	assert(fore);

	skin = &self->pv->patconfig->skin;
	numtracks = patternviewstate_numsongtracks(self->pv);
	if (flags.cursor != FALSE) {		
		*bg = skin->cursor;
		*fore = patternviewskin_fontcurcolour(skin, track, numtracks);
	} else if (flags.playbar) {
		*bg = patternviewskin_playbarcolour(skin, track, numtracks);
		*fore = patternviewskin_fontplaycolour(skin, track, numtracks);
	} else if (flags.selection) {
		if (flags.beat4) {
			*bg = patternviewskin_selection4beatcolour(skin, track, numtracks);
		} else if (flags.beat) {
			*bg = patternviewskin_selectionbeatcolour(skin, track, numtracks);
		} else {
			*bg = patternviewskin_selectioncolour(skin, track, numtracks);
		}
		*fore = patternviewskin_fontselcolour(skin, track, numtracks);
	} else if (flags.mid) {
		*bg = patternviewskin_midlinecolour(skin, track, numtracks);
		if (flags.cursor != FALSE) {
			*fore = patternviewskin_fontcurcolour(skin, track, numtracks);
		} else {
			*fore = patternviewskin_fontcolour(skin, track, numtracks);
		}
	} else {
		if (flags.beat4) {
			*bg = patternviewskin_row4beatcolour(skin, track, numtracks);
			*fore = skin->font;
		} else if (flags.beat) {
			*bg = patternviewskin_rowbeatcolour(skin, track, numtracks);
			*fore = patternviewskin_fontcolour(skin, track, numtracks);
		} else {
			*bg = patternviewskin_rowcolour(skin, track, numtracks);
			*fore = patternviewskin_fontcolour(skin, track, numtracks);
		}
	}	
}

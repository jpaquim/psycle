/*
** This source is free software; you can redistribute itand /or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "trackerview.h"
/* host */
#include "styles.h"
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
	self->textleftedge = 2;
	self->multicolumn = FALSE;
	self->flatsize = psy_ui_value_make_ew(1.6);
	trackdrag_init(&self->resize);
	psy_table_init(&self->trackconfigs);
	trackconfig_init_columns(self, wideinst);
}

void trackconfig_dispose(TrackConfig* self)
{
	psy_table_disposeall(&self->trackconfigs, (psy_fp_disposefunc)NULL);
}

void trackconfig_init_columns(TrackConfig* self, bool wideinst)
{
	trackdef_init(&self->trackdef);
	if (wideinst) {
		self->trackdef.inst.numdigits = 4;
		self->trackdef.inst.numchars = 4;
	}
}

double trackconfig_width(const TrackConfig* self, uintptr_t track,
	const psy_ui_TextMetric* tm)
{
	const TrackDef* trackdef;
	double rv;
	double cmdparam;
	uintptr_t column;	

	trackdef = trackerconfig_trackdef_const(self, track);
	cmdparam = 0.0;
	for (column = 0, rv = 0.0; column < trackdef_num_columns(trackdef);
			++column) {
		double width;

		width = trackdef_column_width(trackdef, column, 
			psy_ui_value_px(&self->flatsize, tm, NULL));
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

double trackconfig_width_cmdparam(const TrackConfig* self,
	const psy_ui_TextMetric* tm)
{
	const TrackDef* trackdef;
	double rv;	
	uintptr_t column;

	trackdef = trackerconfig_trackdef_const(self, psy_INDEX_INVALID);	
	for (column = PATTERNEVENT_COLUMN_CMD, rv = 0.0;
			column < trackdef_num_columns(trackdef);
			++column) {		
		rv += trackdef_column_width(trackdef, column,
			psy_ui_value_px(&self->flatsize, tm, NULL));
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

void trackconfig_resize(TrackConfig* self, uintptr_t track, double width,
	const psy_ui_TextMetric* tm)
{	
	if (trackdrag_active(&self->resize)) {
		TrackDef* trackdef;
		double basewidth;
		uintptr_t numnotes;

		trackdef = trackerconfig_trackdef(self, track);
		basewidth = trackdef_defaulttrackwidth(trackdef, 
			psy_ui_value_px(&self->flatsize, tm, NULL));
		width = psy_max(0.0, width);
		self->resize.size.width = width;
		self->resize.track = track;
		if (self->multicolumn) {
			numnotes = (uintptr_t)psy_max(1.0,
				psy_max(0.0, (width - (basewidth -
					trackconfig_width_cmdparam(self, tm)))) /
				trackconfig_width_cmdparam(self, tm));
		} else {
			numnotes = (uintptr_t)(psy_max(1.0,
				(psy_max(0.0, self->resize.size.width)) /
				basewidth) + 0.9);
		}
		trackconfig_settrack(self, track, numnotes);
	}
}

void trackconfig_track_position(TrackConfig* self, uintptr_t track,
	double x, const psy_ui_TextMetric* tm,
	uintptr_t* rv_column, uintptr_t* rv_digit,
	uintptr_t* rv_noteindex)
{
	TrackDef* trackdef;
	double cpx;
	double flatsize;

	*rv_column = *rv_digit = 0;
	flatsize = psy_ui_value_px(&self->flatsize, tm, NULL);
	trackdef = trackerconfig_trackdef(self, track);
	if (trackdef->visinotes == 1) {
		cpx = 0.0;
		*rv_noteindex = 0;
	} else {		
		*rv_noteindex = (uintptr_t)(x / trackdef_defaulttrackwidth(
			trackdef, flatsize));
		cpx = trackdef_defaulttrackwidth(trackdef, flatsize) * (*rv_noteindex);
	}
	*rv_column = 0;
	while (*rv_column < trackdef_num_columns(trackdef) &&
		cpx + trackdef_column_width(trackdef, *rv_column,
			flatsize) < x) {
		cpx += trackdef_column_width(trackdef, *rv_column, flatsize);
		++(*rv_column);
	}
	*rv_digit = (uintptr_t)((x - cpx) / flatsize);
	if (*rv_digit >= trackdef_num_digits(trackdef, *rv_column)) {
		*rv_digit = trackdef_num_digits(trackdef, *rv_column) - 1;
	}
}

/* TrackColumnDef */
static void enter_digit(int digit, int newval, unsigned char* val);

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

void trackereventtable_clear_events(TrackerEventTable* self)
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
void trackerstate_init(TrackerState* self, TrackConfig* track_config,
	PatternViewState* pvstate)
{			
	self->track_config = track_config;
	self->pv = pvstate;
	self->draw_beat_highlights = TRUE;	
	self->draw_playbar = TRUE;
	self->show_empty_data = FALSE;
	self->midline = FALSE;	
	trackereventtable_init(&self->track_events);
	psy_audio_patternentry_init(&self->empty);	
	self->line_height = psy_ui_value_make_eh(1.0);	
}

void trackerstate_dispose(TrackerState* self)
{
	trackereventtable_dispose(&self->track_events);
	psy_audio_patternentry_dispose(&self->empty);
}

double trackerstate_trackwidth(const TrackerState* self, uintptr_t track,
	const psy_ui_TextMetric* tm)
{
	assert(self);
	assert(self->track_config);

	if (self->track_config->resize.active &&
			self->track_config->resize.track == track) {
		return self->track_config->resize.size.width;
	}
	return trackconfig_width(self->track_config, track, tm);
}

uintptr_t trackerstate_px_to_track(const TrackerState* self, double x,
	const psy_ui_TextMetric* tm)
{
	double currx = 0;
	uintptr_t rv = 0;

	while (rv < patternviewstate_num_song_tracks(self->pv)) {
		currx += trackerstate_trackwidth(self, rv, tm);
		if (currx > x) {
			break;
		}
		++rv;
	}
	return rv;
}

double trackerstate_base_width(TrackerState* self, uintptr_t track,
	const psy_ui_TextMetric* tm)
{
	return trackdef_basewidth(trackerconfig_trackdef(self->track_config,
		track), psy_ui_value_px(&self->track_config->flatsize, tm, NULL)) + 1;
}

uintptr_t trackdef_num_digits(TrackDef* self, uintptr_t column)
{
	TrackColumnDef* coldef;

	coldef = trackdef_column_def(self, column);
	return (coldef)
		? coldef->numdigits
		: 0;
}

uintptr_t trackdef_num_columns(const TrackDef* self)
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
	columndef = trackdef_column_def(self, column);
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
	num = trackdef_num_digits(self, column);
	pos = num / 2 - digit / 2 - 1;
	data = (uint8_t*)&value + pos;
	enter_digit(digit % 2, (uint8_t)digitvalue, data);
	psy_audio_patternevent_setvalue(&rv, column, value);
	return rv;
}

void enter_digit(int digit, int newval, unsigned char* val)
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

	coldef = trackdef_column_def(self, column);
	return coldef ? coldef->emptyvalue : 0;
}

double trackdef_basewidth(const TrackDef* self, double textwidth)
{
	double rv = 0;
	uintptr_t column;

	for (column = 0; column < 4; ++column) {
		rv += trackdef_column_width(self, column, textwidth);
	}
	return rv;
}

double trackdef_defaulttrackwidth(const TrackDef* self, double textwidth)
{
	double rv = 0;
	uintptr_t column;

	for (column = 0; column < PATTERNEVENT_COLUMN_END; ++column) {
		rv += trackdef_column_width(self, column, textwidth);
	}
	return rv;
}

double trackdef_column_width(const TrackDef* self, intptr_t column, double textwidth)
{
	const TrackColumnDef* coldef;

	coldef = trackdef_column_def((TrackDef*)self, column);
	return ((coldef)
		? coldef->numchars * textwidth + coldef->marginright
		: 0.0);
}

double trackdef_marginright(TrackDef* self, intptr_t column)
{
	TrackColumnDef* coldef;

	coldef = trackdef_column_def(self, column);
	return ((coldef) ? coldef->marginright : 0.0);
}

TrackColumnDef* trackdef_column_def(TrackDef* self, intptr_t column)
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
	psy_audio_BlockSelection* rv, const psy_ui_TextMetric* tm)
{
	assert(self);

	rv->topleft.track = trackerstate_px_to_track(self, clip->left, tm);
	rv->topleft.column = 0;
	rv->topleft.digit = 0;
	rv->topleft.lpb = patternviewstate_lpb(self->pv);
	rv->bottomright.track = trackerstate_px_to_track(self, clip->right, tm) + 1;
	if (rv->bottomright.track > patternviewstate_num_song_tracks(self->pv)) {
		rv->bottomright.track = patternviewstate_num_song_tracks(self->pv);
	}
	rv->bottomright.column = 0;
	rv->bottomright.digit = 0;
	rv->bottomright.lpb = patternviewstate_lpb(self->pv);
}

void trackerstate_start_drag_selection(TrackerState* self,
	psy_audio_SequenceCursor cursor)
{
	assert(self);

	psy_audio_blockselection_startdrag(&self->pv->selection,
		self->pv->dragselectionbase, cursor);
}

void trackerstate_drag_selection(TrackerState* self, psy_audio_SequenceCursor
	cursor)
{
	assert(self);

	psy_audio_blockselection_drag(&self->pv->selection,
		self->pv->dragselectionbase, cursor);
}

psy_audio_SequenceCursor trackerstate_checkcursorbounds(TrackerState* self,
	psy_audio_SequenceCursor cursor)
{
	psy_audio_SequenceCursor rv;

	rv = cursor;
	if (rv.absoffset < 0) {
		rv.absoffset = 0;
	} else {
		if (patternviewstate_pattern(self->pv)) {
			if (rv.absoffset >= patternviewstate_length(self->pv)) {
				rv.absoffset = patternviewstate_length(self->pv);
			}
		} else {
			rv.absoffset = 0;
		}
	}
	if (rv.track < 0) {
		rv.track = 0;
	} else if (rv.track >= patternviewstate_num_song_tracks(self->pv)) {
		rv.track = patternviewstate_num_song_tracks(self->pv);
	}
	return rv;
}

void trackerstate_line_clip(TrackerState* self, const psy_ui_RealRectangle* clip,
	psy_audio_BlockSelection* rv, double line_height)
{	
	double offset;

	assert(self);
	
	offset = trackerstate_px_to_beat(self, psy_max(0.0, clip->top), line_height);
	if (patternviewstate_single_mode(self->pv)) {
		offset += self->pv->cursor.seqoffset;
		rv->topleft.seqoffset = self->pv->cursor.seqoffset;		
	} else {
		rv->topleft.seqoffset = 0;
	}
	rv->topleft.absoffset = offset;
	rv->topleft.column = 0;
	rv->topleft.digit = 0;		
	rv->topleft.lpb = patternviewstate_lpb(self->pv);
	psy_audio_sequencecursor_updatecache(&rv->topleft);
	rv->bottomright.column = 0;
	rv->bottomright.digit = 0;
	rv->bottomright.seqoffset = 0;	
	offset = trackerstate_px_to_beat(self,
		psy_max(0.0, clip->bottom), line_height);	 
	if (patternviewstate_single_mode(self->pv)) {
		offset += self->pv->cursor.seqoffset;
		rv->bottomright.absoffset = offset;		
		rv->bottomright.seqoffset = self->pv->cursor.seqoffset;		
		rv->bottomright.absoffset =
			psy_min(rv->bottomright.absoffset,
				rv->bottomright.seqoffset + 
				patternviewstate_length(self->pv));
	} else {
		rv->bottomright.absoffset = offset;
		rv->bottomright.seqoffset = 0;
		rv->bottomright.absoffset =
			psy_min(rv->bottomright.absoffset,				
				patternviewstate_length(self->pv));
	}	
	rv->bottomright.lpb = rv->topleft.lpb;
	psy_audio_sequencecursor_updatecache(&rv->bottomright);
}

psy_audio_SequenceCursor trackerstate_make_cursor(TrackerState* self,
	psy_ui_RealPoint pt, uintptr_t index, double line_height,
	const psy_ui_TextMetric* tm)
{
	psy_audio_SequenceCursor rv;	
	psy_audio_Sequence* sequence;	

	rv = self->pv->cursor;
	if (!patternviewstate_single_mode(self->pv)) {
		rv.absoffset = trackerstate_px_to_beat(self, pt.y, line_height);
	} else {
		rv.absoffset = trackerstate_px_to_beat(self, pt.y, line_height) +
			rv.seqoffset;
	}
	rv.lpb = patternviewstate_lpb(self->pv);	
	sequence = patternviewstate_sequence(self->pv);
	if (!sequence) {
		return rv;
	}
	if (!patternviewstate_single_mode(self->pv)) {
		psy_audio_sequencecursor_update_order(&rv, sequence);
	}	
	psy_audio_sequencecursor_updatecache(&rv);
	self->pv->cursor.patternid = sequence->cursor.patternid;
	rv.track = index;
	trackconfig_track_position(self->track_config, rv.track, pt.x, tm,
		&rv.column, &rv.digit, &rv.noteindex);	
	rv.key = sequence->cursor.key;	
	return rv; // trackerstate_checkcursorbounds(self, rv);
}

void trackerstate_columncolours(TrackerState* self,
	TrackerColumnFlags flags, uintptr_t track,
	psy_ui_Colour* bg, psy_ui_Colour* fore)
{		
	/* uintptr_t numtracks; */
	psy_ui_Style* style;

	assert(bg);
	assert(fore);
	
	// numtracks = patternviewstate_num_song_tracks(self->pv);
	style = NULL;
	if (flags.cursor != FALSE) {		
		style = psy_ui_style(STYLE_PV_CURSOR);
	} else if (flags.playbar) {
		style = psy_ui_style(STYLE_PV_PLAYBAR);		
	} else if (flags.selection) {
		if (flags.beat4) {
			style = psy_ui_style(STYLE_PV_ROW4BEAT_SELECT);			
		} else if (flags.beat) {
			style = psy_ui_style(STYLE_PV_ROWBEAT_SELECT);			
		} else {
			style = psy_ui_style(STYLE_PV_ROW_SELECT);			
		}		
	} else if (flags.mid) {				
		if (flags.cursor != FALSE) {
			style = psy_ui_style(STYLE_PV_CURSOR);			
		} else {
			style = psy_ui_style(STYLE_PV_PLAYBAR);			
		}
	} else {
		if (flags.beat4) {			
			style = psy_ui_style(STYLE_PV_ROW4BEAT);			
		} else if (flags.beat) {
			style = psy_ui_style(STYLE_PV_ROWBEAT);			
		} else {
			style = psy_ui_style(STYLE_PV_ROW);				
		}
	}	
	if (style) {
		*bg = style->background.colour;
		*fore = style->colour;
	} else {
		*bg = psy_ui_colour_black();
		*fore = psy_ui_colour_white();
	}
}

void trackerstate_update_clip_events(TrackerState* self,
	const psy_audio_BlockSelection* clip)
{
	uintptr_t track;
	uintptr_t maxlines;
	double offset;
	double seqoffset;
	double length;
	psy_audio_SequenceTrackIterator ite;
	uintptr_t line;

	assert(self);

	if (!patternviewstate_pattern(self->pv)) {
		return;
	}
	trackereventtable_clear_events(&self->track_events);
	offset = clip->topleft.absoffset;
	if (patternviewstate_single_mode(self->pv)) {
		offset -= self->pv->cursor.seqoffset;
	}
	psy_audio_sequencetrackiterator_init(&ite);
	patternviewstate_sequencestart(self->pv, offset, &ite);
	length = 0.0;
	if (ite.pattern) {
		length = ite.pattern->length;
	}
	seqoffset = psy_audio_sequencetrackiterator_seqoffset(&ite);
	line = patternviewstate_beat_to_line(self->pv, offset);
	maxlines = patternviewstate_numlines(self->pv);
	while (offset <= clip->bottomright.absoffset && line < maxlines) {
		bool fill;

		fill = !(offset >= seqoffset && offset < seqoffset + length) || !ite.patternnode;
		/* draw trackline */
		for (track = clip->topleft.track; track < clip->bottomright.track;
			++track) {
			bool hasevent = FALSE;

			while (!fill && ite.patternnode &&
				psy_audio_sequencetrackiterator_patternentry(&ite)->track <= track &&
				psy_dsp_testrange_e(
					psy_audio_sequencetrackiterator_offset(&ite),
					offset,
					patternviewstate_bpl(self->pv))) {
				psy_audio_PatternEntry* entry;

				entry = psy_audio_sequencetrackiterator_patternentry(&ite);
				if (entry->track == track) {
					psy_List** track_events;

					track_events = trackereventtable_track(&self->track_events, entry->track);
					psy_list_append(track_events, entry);
					psy_list_next(&ite.patternnode);
					hasevent = TRUE;
					break;
				}
				psy_list_next(&ite.patternnode);
			}
			if (!hasevent) {
				psy_List** track_events;

				track_events = trackereventtable_track(&self->track_events, track);
				psy_list_append(track_events, NULL);
			} else if (ite.patternnode && ((psy_audio_PatternEntry*)(ite.patternnode->entry))->track <= track) {
				fill = TRUE;
			}
		}
		/* skip remaining events of the line */
		while (ite.patternnode && (offset < seqoffset + length) &&
			(psy_audio_sequencetrackiterator_offset(&ite) + psy_dsp_epsilon * 2 <
				offset + patternviewstate_bpl(self->pv))) {
			psy_list_next(&ite.patternnode);
		}
		offset += patternviewstate_bpl(self->pv);
		if (offset >= seqoffset + length) {
			/* go to next seqentry or end draw */
			if (ite.sequencentrynode && ite.sequencentrynode->next) {
				psy_audio_sequencetrackiterator_inc_entry(&ite);
				seqoffset = psy_audio_sequencetrackiterator_seqoffset(&ite);
				offset = seqoffset;
				if (ite.pattern) {
					length = ite.pattern->length;
				} else {
					break;
				}
			} else {
				break;
			}
		}
	}
	self->track_events.clip = *clip;
	self->track_events.currcursorline =
		patternviewstate_beat_to_line(self->pv,
			(patternviewstate_single_mode(self->pv))
			? psy_audio_sequencecursor_pattern_offset(&self->pv->cursor)
			: psy_audio_sequencecursor_offset_abs(&self->pv->cursor));
	psy_audio_sequencetrackiterator_dispose(&ite);
}

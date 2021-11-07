/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net
*/

#if !defined(TRACKERGRIDSTATE_H)
#define TRACKERGRIDSTATE_H

/* host */
#include "patternviewskin.h"
#include "patternviewstate.h"
#include "trackercmds.h"
#include "workspace.h"
/* audio */
#include <pattern.h>
#include <sequence.h>

/*
** The TrackerView is where you enter notes. It displays a Pattern selected by
** the SequenceView as a tracker grid.
*/

typedef enum {
	SCROLL_DIR_NONE,
	SCROLL_DIR_LEFT,
	SCROLL_DIR_RIGHT
} ScrollDir;

enum {
	TRACKER_COLUMN_NONE = -1,
	TRACKER_COLUMN_NOTE = 0,
	TRACKER_COLUMN_INST = 1,
	TRACKER_COLUMN_MACH = 2,
	TRACKER_COLUMN_VOL = 3,
	TRACKER_COLUMN_CMD = 4,
	TRACKER_COLUMN_PARAM = 5,
	TRACKER_COLUMN_END = 6
};

typedef struct TrackColumnDef {
	uintptr_t numdigits;
	uintptr_t numchars;
	double marginright;
	int wrapeditcolumn;
	int wrapclearcolumn;
	uintptr_t emptyvalue;
} TrackColumnDef;

void trackcolumndef_init(TrackColumnDef* self, uintptr_t numdigits,
	uintptr_t numchars, double marginright, int wrapeditcolumn,
	int wrapclearcolumn,
	int emptyvalue);

typedef struct TrackDef {
	TrackColumnDef note;
	TrackColumnDef inst;
	TrackColumnDef mach;
	TrackColumnDef vol;
	TrackColumnDef cmd;
	TrackColumnDef param;
	uintptr_t numfx;
	uintptr_t numnotes;
} TrackDef;

void trackdef_init(TrackDef*);
uintptr_t trackdef_numdigits(TrackDef*, uintptr_t column);
uintptr_t trackdef_numcolumns(TrackDef*);
uintptr_t trackdef_value(TrackDef*, uintptr_t column,
	const psy_audio_PatternEntry*);
uintptr_t trackdef_emptyvalue(TrackDef*, uintptr_t column);
void trackdef_setvalue(TrackDef*, uintptr_t column,
	psy_audio_PatternEntry*, uintptr_t value);
double trackdef_width(TrackDef*, double textwidth);
double trackdef_basewidth(TrackDef* self, double textwidth);
TrackColumnDef* trackdef_columndef(TrackDef* self, intptr_t column);
double trackdef_columnwidth(TrackDef* self, intptr_t column, double textwidth);
double trackdef_marginright(TrackDef* self, intptr_t column);

typedef struct TrackConfig {
	psy_Table trackconfigs;
	TrackDef trackdef;
	double textwidth;
	double textleftedge;	
	bool colresize;
	bool noteresize;
	uintptr_t resizetrack;
	psy_ui_RealSize resizesize;	
} TrackConfig;

void trackconfig_init(TrackConfig*, bool wideinst);
void trackconfig_dispose(TrackConfig*);
void trackconfig_initcolumns(TrackConfig*, bool wideinst);

/* TrackerEventTable */
typedef struct TrackerEventTable {
	psy_Table tracks;
	double seqoffset;
	psy_audio_BlockSelection clip;
	uintptr_t currcursorline;
	uintptr_t currplaybarline;	
} TrackerEventTable;

void trackereventtable_init(TrackerEventTable*);
void trackereventtable_dispose(TrackerEventTable*);

void trackereventtable_clearevents(TrackerEventTable*);
psy_List** trackereventtable_track(TrackerEventTable*, uintptr_t index);

/* TrackerState */
typedef struct TrackerState {		
	/* internal */
	PatternViewState pv;	
	TrackConfig* trackconfig;	
	TrackerEventTable trackevents;	
	bool showemptydata;
	bool midline;
	bool drawbeathighlights;
	bool synccursor;	
	bool showresizecursor;	
	psy_audio_PatternEntry empty;
	psy_ui_Value lineheight;
	psy_ui_Value defaultlineheight;
	double lineheightpx;
	double flatsize;	
	bool drawcursor;		
	/* precomputed */
	intptr_t visilines;
	bool cursorchanging;	
	/* references */
	const psy_ui_Font* gridfont;	
} TrackerState;

void trackerstate_init(TrackerState*, TrackConfig*, psy_audio_Song* song);
void trackerstate_dispose(TrackerState*);
double trackerstate_trackwidth(const TrackerState*, uintptr_t track);
double trackerstate_defaulttrackwidth(const TrackerState*);
TrackDef* trackerstate_trackdef(TrackerState*, uintptr_t track);
uintptr_t trackerstate_pxtotrack(const TrackerState*, double x);
double trackerstate_basewidth(TrackerState*, uintptr_t track);

INLINE double trackerstate_preferredtrackwidth(const
	TrackerState* self)
{
	if (self->pv.skin) {
		return self->pv.skin->headercoords.background.dest.right -
			self->pv.skin->headercoords.background.dest.left;
	}
	return 0;
}

INLINE bool trackerstate_cursorposition_valid(TrackerState* self)
{
	if (patternviewstate_pattern(&self->pv)) {
		return psy_audio_sequencecursor_offset(&self->pv.cursor) <
			psy_audio_pattern_length(patternviewstate_pattern(&self->pv));
	}
	return psy_audio_sequencecursor_offset(&self->pv.cursor) != 0.0;
}

void trackerstate_clip(TrackerState*, const psy_ui_RealRectangle* clip,
	psy_audio_BlockSelection* rv);

void trackerstate_startdragselection(TrackerState*,
	psy_audio_SequenceCursor, double bpl);
void trackerstate_dragselection(TrackerState*,
	psy_audio_SequenceCursor, double bpl);
psy_audio_SequenceCursor trackerstate_checkcursorbounds(TrackerState*,
	psy_audio_SequenceCursor);
void trackerstate_selectcol(TrackerState*);
void trackerstate_selectbar(TrackerState*);
void trackerstate_selectall(TrackerState*);
ScrollDir trackerstate_nextcol(TrackerState*, bool wrap);
ScrollDir trackerstate_prevcol(TrackerState*, bool wrap);
uintptr_t trackerstate_numlines(const TrackerState*);
bool trackerstate_testplaybar(TrackerState*,
	psy_dsp_big_beat_t playposition,
	psy_dsp_big_beat_t offset);

INLINE uintptr_t trackerstate_lpb(const TrackerState* self)
{
	return self->pv.cursor.cursor.lpb;
}

INLINE psy_dsp_big_beat_t trackerstate_bpl(const TrackerState* self)
{
	return (psy_dsp_big_beat_t)1.0 / self->pv.cursor.cursor.lpb;
}

INLINE intptr_t trackerstate_beattoline(const TrackerState* self,
	psy_dsp_big_beat_t offset)
{
	assert(self);

	return cast_decimal(offset * trackerstate_lpb(self));
}

INLINE psy_dsp_big_beat_t trackerstate_quantize(const TrackerState*
	self, psy_dsp_big_beat_t position)
{
	assert(self);

	return trackerstate_beattoline(self, position) *
		trackerstate_bpl(self);
}

/* quantized */
INLINE double trackerstate_beattopx(TrackerState* self,
	psy_dsp_big_beat_t position)
{
	assert(self);

	return self->lineheightpx * trackerstate_beattoline(self, position);
}

INLINE double trackerstate_linetopx(TrackerState* self,
	uintptr_t line)
{
	assert(self);

	return self->lineheightpx * line;
}

INLINE double trackerstate_lineheight(TrackerState* self)
{
	assert(self);

	return self->lineheightpx;
}

/* quantized */
INLINE psy_dsp_big_beat_t trackerstate_pxtobeat(
	const TrackerState* self, double px)
{
	assert(self);

	return trackerstate_quantize(self,
		(px / (psy_dsp_big_beat_t)self->lineheightpx) *
		trackerstate_bpl(self));
}

INLINE psy_dsp_big_beat_t trackerstate_pxtobeatnotquantized(
	TrackerState* self, double px)
{
	assert(self);

	return (px / self->lineheightpx) * trackerstate_bpl(self);
}

INLINE bool trackerstate_testplayposition(TrackerState* self,
	psy_dsp_big_beat_t position)
{
	assert(self);

	if (self->pv.pattern) {
		return psy_dsp_testrange(position, self->pv.cursor.seqoffset,
			psy_audio_pattern_length(self->pv.pattern));
	}
	return FALSE;
}

void trackerstate_lineclip(TrackerState*, const psy_ui_RealRectangle* clip,
	psy_audio_BlockSelection* rv);

INLINE uintptr_t trackerstate_midline(const TrackerState* self,
	double scrolltop_px)
{
	return trackerstate_beattoline(self, trackerstate_pxtobeat(
		self, scrolltop_px) + self->visilines / 2);
}

void trackerstate_setfont(TrackerState*, const psy_ui_Font*,
	const psy_ui_TextMetric*);

#endif /* TRACKERGRIDSTATE_H */

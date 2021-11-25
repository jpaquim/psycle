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

/* TrackResize */
typedef struct TrackResize {
	bool column;
	bool note;
	uintptr_t track;
	psy_ui_RealSize size;
} TrackResize;

void trackresize_init(TrackResize*);

void trackresize_stop(TrackResize*);
bool trackresize_column_dragging(const TrackResize*);
bool trackresize_note_dragging(const TrackResize*);

/* TrackDef */
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
	TrackResize resize;
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
} TrackerEventTable;

void trackereventtable_init(TrackerEventTable*);
void trackereventtable_dispose(TrackerEventTable*);

void trackereventtable_clearevents(TrackerEventTable*);
psy_List** trackereventtable_track(TrackerEventTable*, uintptr_t index);

/* TrackerState */
typedef struct TrackerState {		
	/* internal */
	PatternViewState* pv;	
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
} TrackerState;

void trackerstate_init(TrackerState*, TrackConfig*, PatternViewState* pvstate);
void trackerstate_dispose(TrackerState*);

double trackerstate_trackwidth(const TrackerState*, uintptr_t track);
double trackerstate_defaulttrackwidth(const TrackerState*);
TrackDef* trackerstate_trackdef(TrackerState*, uintptr_t track);
uintptr_t trackerstate_pxtotrack(const TrackerState*, double x);
double trackerstate_basewidth(TrackerState*, uintptr_t track);

void trackerstate_clip(TrackerState*, const psy_ui_RealRectangle* clip,
	psy_audio_BlockSelection* rv);

void trackerstate_startdragselection(TrackerState*,
	psy_audio_SequenceCursor, double bpl);
void trackerstate_dragselection(TrackerState*,
	psy_audio_SequenceCursor, double bpl);
psy_audio_SequenceCursor trackerstate_checkcursorbounds(TrackerState*,
	psy_audio_SequenceCursor);
ScrollDir trackerstate_nextcol(TrackerState*, bool wrap);
ScrollDir trackerstate_prevcol(TrackerState*, bool wrap);
bool trackerstate_testplaybar(TrackerState*,
	psy_dsp_big_beat_t playposition,
	psy_dsp_big_beat_t offset);

/* quantized */
INLINE double trackerstate_beattopx(TrackerState* self,
	psy_dsp_big_beat_t position)
{
	assert(self);

	return self->lineheightpx * patternviewstate_beattoline(self->pv,
		position);
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

	return patternviewstate_quantize(self->pv,
		(px / (psy_dsp_big_beat_t)self->lineheightpx) *
		patternviewstate_bpl(self->pv));
}

INLINE psy_dsp_big_beat_t trackerstate_pxtobeatnotquantized(
	TrackerState* self, double px)
{
	assert(self);

	return (px / self->lineheightpx) * patternviewstate_bpl(self->pv);
}

void trackerstate_lineclip(TrackerState*, const psy_ui_RealRectangle* clip,
	psy_audio_BlockSelection* rv);

INLINE uintptr_t trackerstate_midline(const TrackerState* self,
	double scrolltop_px)
{
	return patternviewstate_beattoline(self->pv, trackerstate_pxtobeat(
		self, scrolltop_px) + self->visilines / 2);
}

void trackerstate_updatemetric(TrackerState*, const psy_ui_TextMetric*,
	double lineheight);

psy_audio_SequenceCursor trackerstate_makecursor(TrackerState*,
	psy_ui_RealPoint pt, uintptr_t index);

#endif /* TRACKERGRIDSTATE_H */

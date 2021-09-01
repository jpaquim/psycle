/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net
*/

#if !defined(TRACKERGRIDSTATE_H)
#define TRACKERGRIDSTATE_H

/* host */
#include "patternviewskin.h"
#include "trackercmds.h"
/* audio */
#include <pattern.h>
#include <sequence.h>

/*
** The TrackerView is where you enter notes. It displays a Pattern selected by
** the SeqView as a tracker grid.
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

void trackcolumndef_init(TrackColumnDef* self, int numdigits, int numchars,
	double marginright, int wrapeditcolumn, int wrapclearcolumn, int emptyvalue);

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
	psy_audio_PatternSelection clip;
	uintptr_t currcursorline;
	uintptr_t currplaybarline;	
} TrackerEventTable;

void trackereventtable_init(TrackerEventTable*);
void trackereventtable_dispose(TrackerEventTable*);

void trackereventtable_clearevents(TrackerEventTable*);
psy_List** trackereventtable_track(TrackerEventTable*, uintptr_t index);

/* TrackerGridState */
typedef struct TrackerGridState {
	/* signals */
	psy_Signal signal_cursorchanged;
	/* internal */
	psy_audio_PatternCursor cursor;
	/* references */
	psy_audio_Pattern* pattern;
	psy_audio_Patterns* patterns;
	psy_audio_Sequence* sequence;
	PatternViewSkin* skin;
	TrackConfig* trackconfig;
	bool singlemode;
	TrackerEventTable trackevents;
	psy_audio_PatternSelection selection;
	bool showemptydata;
	bool midline;
	bool playbar;
	bool drawbeathighlights;
	bool synccursor;	
	bool showresizecursor;
	psy_audio_PatternCursor dragselectionbase;
	psy_audio_PatternCursor dragcursor;
	psy_audio_PatternEntry empty;
	psy_ui_Value lineheight;
	psy_ui_Value defaultlineheight;
	double lineheightpx;
	double flatsize;
	uintptr_t lpb;
	psy_dsp_big_beat_t bpl;
	bool drawcursor;
	psy_dsp_big_beat_t lastplayposition;
	psy_dsp_big_beat_t sequenceentryoffset;
	uintptr_t trackidx;
	// precomputed
	intptr_t visilines;
	bool cursorchanging;
	// references	
	const psy_ui_Font* gridfont;	
	uintptr_t maxlines;
} TrackerGridState;

void trackergridstate_init(TrackerGridState*, TrackConfig*,
	psy_audio_Patterns*, psy_audio_Sequence*);
void trackergridstate_dispose(TrackerGridState*);
double trackergridstate_trackwidth(const TrackerGridState*, uintptr_t track);
double trackergridstate_defaulttrackwidth(const TrackerGridState*);
TrackDef* trackergridstate_trackdef(TrackerGridState*, uintptr_t track);
uintptr_t trackergridstate_pxtotrack(const TrackerGridState*, double x);
double trackergridstate_basewidth(TrackerGridState*, uintptr_t track);

INLINE void trackergridstate_setsequence(TrackerGridState* self,
	psy_audio_Sequence* sequence)
{
	assert(self);

	self->sequence = sequence;
}

INLINE void trackergridstate_setpattern(TrackerGridState* self,
	psy_audio_Pattern* pattern)
{
	assert(self);

	self->pattern = pattern;
}

INLINE void trackergridstate_setpatterns(TrackerGridState* self,
	psy_audio_Patterns* patterns)
{
	assert(self);
	
	self->patterns = patterns;
}

INLINE psy_audio_Patterns* trackergridstate_patterns(TrackerGridState* self)
{
	assert(self);

	return self->patterns;
}

INLINE psy_audio_Pattern* trackergridstate_pattern(TrackerGridState* self)
{
	assert(self);

	return self->pattern;
}

INLINE double trackergridstate_preferredtrackwidth(const
	TrackerGridState* self)
{
	if (self->skin) {
		return self->skin->headercoords.background.dest.right - 
			self->skin->headercoords.background.dest.left;
	}
	return 0;
}

INLINE uintptr_t trackergridstate_numsongtracks(const TrackerGridState* self)
{
	if (self->patterns) {
		return psy_audio_patterns_numtracks(self->patterns);
	}
	return 0;
}

INLINE psy_audio_PatternCursor trackergridstate_cursor(TrackerGridState* self)
{
	return self->cursor;
}

INLINE uintptr_t trackergridstate_cursorposition_valid(TrackerGridState* self)
{
	if (self->pattern) {
		return self->cursor.offset < psy_audio_pattern_length(self->pattern);
	}
	return self->cursor.offset != 0.0;
}

void trackergridstate_synccursor(TrackerGridState*);
void trackergridstate_setcursor(TrackerGridState*,psy_audio_PatternCursor);
void trackergridstate_clip(TrackerGridState*, const psy_ui_RealRectangle* clip,
	psy_audio_PatternSelection* rv);

INLINE void trackergridstate_enableplaybar(TrackerGridState* self)
{
	self->playbar = TRUE;
}

INLINE void trackergridstate_preventplaybar(TrackerGridState* self)
{
	self->playbar = FALSE;
}

INLINE bool trackergridstate_hasplaybar(const TrackerGridState* self)
{
	return self->playbar;
}

void trackergridstate_startdragselection(TrackerGridState*,
	psy_audio_PatternCursor, double bpl);
void trackergridstate_dragselection(TrackerGridState*,
	psy_audio_PatternCursor, double bpl);
psy_audio_PatternCursor trackergridstate_checkcursorbounds(TrackerGridState*,
	psy_audio_PatternCursor);
void trackergridstate_selectcol(TrackerGridState*);
void trackergridstate_selectbar(TrackerGridState*);
void trackergridstate_selectall(TrackerGridState*);
ScrollDir trackergridstate_nextcol(TrackerGridState*, bool wrap);
ScrollDir trackergridstate_prevcol(TrackerGridState*, bool wrap);

uintptr_t trackergridstate_numlines(const TrackerGridState*);
psy_dsp_big_beat_t trackergridstate_length(const TrackerGridState*);
bool trackergridstate_testplaybar(TrackerGridState*, psy_dsp_big_beat_t
	offset);

INLINE intptr_t trackergridstate_beattoline(const TrackerGridState* self,
	psy_dsp_big_beat_t offset)
{
	assert(self);

	return cast_decimal(offset * self->lpb);
}

INLINE void trackergridstate_setlpb(TrackerGridState* self, uintptr_t lpb)
{
	self->lpb = lpb;
	self->bpl = 1.0 / lpb;
}

INLINE uintptr_t trackergridstate_lpb(const TrackerGridState* self)
{
	return self->lpb;
}

INLINE psy_dsp_big_beat_t trackergridstate_bpl(const TrackerGridState* self)
{
	return self->bpl;
}

INLINE psy_audio_Sequence* trackergridstate_sequence(TrackerGridState* self)
{
	assert(self);

	return self->sequence;
}

INLINE psy_dsp_big_beat_t trackergridstate_quantize(const TrackerGridState*
	self, psy_dsp_big_beat_t position)
{
	assert(self);

	return trackergridstate_beattoline(self, position) *
		self->bpl;
}

/* quantized */
INLINE double trackergridstate_beattopx(TrackerGridState* self,
	psy_dsp_big_beat_t position)
{
	assert(self);

	return self->lineheightpx * trackergridstate_beattoline(self, position);
}

INLINE double trackergridstate_linetopx(TrackerGridState* self,
	uintptr_t line)
{
	assert(self);

	return self->lineheightpx * line;
}

INLINE double trackergridstate_lineheight(TrackerGridState* self)
{
	assert(self);

	return self->lineheightpx;
}

// quantized
INLINE psy_dsp_big_beat_t trackergridstate_pxtobeat(
	const TrackerGridState* self, double px)
{
	assert(self);

	return trackergridstate_quantize(self,
		(px / (psy_dsp_big_beat_t)self->lineheightpx) * self->bpl);
}

INLINE psy_dsp_big_beat_t trackergridstate_pxtobeatnotquantized(
	TrackerGridState* self, double px)
{
	assert(self);

	return (px / self->lineheightpx) * self->bpl;
}

INLINE bool trackergridstate_testplayposition(TrackerGridState* self,
	psy_dsp_big_beat_t position)
{
	assert(self);

	if (self->pattern) {
		return psy_dsp_testrange(position, self->sequenceentryoffset,
			psy_audio_pattern_length(self->pattern));
	}
	return FALSE;
}

void trackergridstate_lineclip(TrackerGridState*, const psy_ui_RealRectangle* clip,
	psy_audio_PatternSelection* rv);

INLINE uintptr_t trackergridstate_midline(const TrackerGridState* self,
	double scrolltop_px)
{
	return trackergridstate_beattoline(self, trackergridstate_pxtobeat(
		self, scrolltop_px) + self->visilines / 2);
}

#endif /* TRACKERGRIDSTATE_H */

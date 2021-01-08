// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#if !defined(TRACKERGRIDSTATE)
#define TRACKERGRIDSTATE

// host
#include "patternviewskin.h"
// audio
#include <pattern.h>

// The TrackerView is where you enter notes. It displays a Pattern selected by
// the SequenceView as a tracker grid.

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

enum {
	CMD_NAVUP = 1024,
	CMD_NAVDOWN,
	CMD_NAVLEFT,
	CMD_NAVRIGHT,
	CMD_NAVPAGEUP,	///< pgup
	CMD_NAVPAGEDOWN,///< pgdn	
	CMD_NAVTOP,		///< home
	CMD_NAVBOTTOM,	///< end

	CMD_COLUMNPREV,	///< tab
	CMD_COLUMNNEXT,	///< s-tab

	CMD_BLOCKSTART,
	CMD_BLOCKEND,
	CMD_BLOCKUNMARK,
	CMD_BLOCKCUT,
	CMD_BLOCKCOPY,
	CMD_BLOCKPASTE,
	CMD_BLOCKMIX,

	CMD_ROWINSERT,
	CMD_ROWDELETE,
	CMD_ROWCLEAR,

	CMD_TRANSPOSEBLOCKINC,
	CMD_TRANSPOSEBLOCKDEC,
	CMD_TRANSPOSEBLOCKINC12,
	CMD_TRANSPOSEBLOCKDEC12,

	CMD_SELECTALL,
	CMD_SELECTCOL,
	CMD_SELECTBAR,

	CMD_SELECTMACHINE,	///< Enter
	CMD_UNDO,
	CMD_REDO,

	CMD_BLOCKDELETE,
	CMD_NAVPAGEUPKEYBOARD, ///< pgup keyboard
	CMD_NAVPAGEDOWNKEYBOARD, ///< pgdn keyboard

	CMD_DIGIT0,
	CMD_DIGIT1,
	CMD_DIGIT2,
	CMD_DIGIT3,
	CMD_DIGIT4,
	CMD_DIGIT5,
	CMD_DIGIT6,
	CMD_DIGIT7,
	CMD_DIGIT8,
	CMD_DIGIT9,
	CMD_DIGITA,
	CMD_DIGITB,
	CMD_DIGITC,
	CMD_DIGITD,
	CMD_DIGITE,
	CMD_DIGITF
};

typedef struct TrackColumnDef {
	uintptr_t numdigits;
	uintptr_t numchars;
	uintptr_t marginright;
	int wrapeditcolumn;
	int wrapclearcolumn;
	uintptr_t emptyvalue;
} TrackColumnDef;

void trackcolumndef_init(TrackColumnDef* self, int numdigits, int numchars,
	int marginright, int wrapeditcolumn, int wrapclearcolumn, int emptyvalue);

typedef struct TrackDef {
	TrackColumnDef note;
	TrackColumnDef inst;
	TrackColumnDef mach;
	TrackColumnDef vol;
	TrackColumnDef cmd;
	TrackColumnDef param;
	intptr_t numfx;
} TrackDef;

void trackdef_init(TrackDef*);
uintptr_t trackdef_numdigits(TrackDef*, uintptr_t column);
uintptr_t trackdef_numcolumns(TrackDef*);
uintptr_t trackdef_value(TrackDef*, uintptr_t column,
	const psy_audio_PatternEntry*);
uintptr_t trackdef_emptyvalue(TrackDef*, uintptr_t column);
void trackdef_setvalue(TrackDef*, uintptr_t column,
	psy_audio_PatternEntry*, uintptr_t value);
intptr_t trackdef_width(TrackDef*, intptr_t textwidth);
intptr_t trackdef_basewidth(TrackDef* self, intptr_t textwidth);
TrackColumnDef* trackdef_columndef(TrackDef* self, intptr_t column);
intptr_t trackdef_columnwidth(TrackDef* self, intptr_t column, intptr_t textwidth);

typedef struct TrackConfig {
	psy_Table trackconfigs;
	TrackDef trackdef;
	intptr_t textwidth;
	intptr_t textleftedge;
	intptr_t patterntrackident;
	intptr_t headertrackident;
} TrackConfig;

void trackconfig_init(TrackConfig*, bool wideinst);
void trackconfig_dispose(TrackConfig*);
void trackconfig_initcolumns(TrackConfig*, bool wideinst);

typedef struct TrackerGridState {
	// signals
	psy_Signal signal_cursorchanged;
	// internal data
	psy_audio_PatternCursor cursor;	
	uintptr_t numtracks;	
	// references
	psy_audio_Pattern* pattern;
	PatternViewSkin* skin;
	TrackConfig* trackconfig;
} TrackerGridState;

void trackerpianogridstate_init(TrackerGridState*, TrackConfig*);
void trackergridstate_dispose(TrackerGridState*);
intptr_t trackergridstate_trackwidth(TrackerGridState*, uintptr_t track);
TrackDef* trackergridstate_trackdef(TrackerGridState*, uintptr_t track);
intptr_t trackergridstate_tracktopx(TrackerGridState*, uintptr_t track);
uintptr_t trackergridstate_pxtotrack(TrackerGridState*, intptr_t x,
	uintptr_t numsongtracks);
uintptr_t trackergridstate_basewidth(TrackerGridState*, uintptr_t track);
uintptr_t trackergridstate_paramcol(TrackerGridState*, uintptr_t track, intptr_t x);

INLINE void trackergridstate_setpattern(TrackerGridState* self, psy_audio_Pattern* pattern)
{
	assert(self);

	self->pattern = pattern;
}

INLINE psy_audio_Pattern* trackergridstate_pattern(TrackerGridState* self)
{
	assert(self);

	return self->pattern;
}

INLINE intptr_t trackergridstate_preferredtrackwidth(TrackerGridState* self)
{
	if (self->skin) {
		return self->skin->headercoords.background.destwidth;
	}
	return 0;
}

INLINE uintptr_t trackergridstate_numsongtracks(TrackerGridState* self)
{
	return self->numtracks;
}

INLINE void trackergridstate_setnumsongtracks(TrackerGridState* self,
	uintptr_t numtracks)
{
	self->numtracks = numtracks;
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
void trackergridstate_setcursor(TrackerGridState*,psy_audio_PatternCursor cursor);

#endif /* TRACKERGRIDSTATE */

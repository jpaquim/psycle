// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#if !defined(TRACKERGRIDSTATE)
#define TRACKERGRIDSTATE

// host
#include "patternviewskin.h"
// audio
#include <pattern.h>
#include <sequence.h>

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
	CMD_DIGITF,

	CMD_COLUMN_0, // Jump To Track (0..7)
	CMD_COLUMN_1, // FT2: ALT + Q .. I
	CMD_COLUMN_2,
	CMD_COLUMN_3,
	CMD_COLUMN_4,
	CMD_COLUMN_5,
	CMD_COLUMN_6,
	CMD_COLUMN_7,
	CMD_COLUMN_8, // Jump To Track (8..F)
	CMD_COLUMN_9, // have no key default settings atm
	CMD_COLUMN_A, // because FT2 key defaults conflict
	CMD_COLUMN_B, // with other psycle cmd-key mappings
	CMD_COLUMN_C, // (FT2 is: ALT + A .. K)
	CMD_COLUMN_D,
	CMD_COLUMN_E,
	CMD_COLUMN_F
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
	double patterntrackident;
	double headertrackident;
} TrackConfig;

void trackconfig_init(TrackConfig*, bool wideinst);
void trackconfig_dispose(TrackConfig*);
void trackconfig_initcolumns(TrackConfig*, bool wideinst);

// TrackerEventTable
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

// TrackerGridState
typedef struct TrackerGridState {
	// signals
	psy_Signal signal_cursorchanged;
	// internal data
	psy_audio_PatternCursor cursor;
	// references
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
	bool colresize;
	uintptr_t resizetrack;
	psy_audio_PatternEntry empty;
} TrackerGridState;

void trackergridstate_init(TrackerGridState*, TrackConfig*,
	psy_audio_Patterns*, psy_audio_Sequence*);
void trackergridstate_dispose(TrackerGridState*);
double trackergridstate_trackwidth(const TrackerGridState*, uintptr_t track);
double trackergridstate_defaulttrackwidth(const TrackerGridState* self);
TrackDef* trackergridstate_trackdef(TrackerGridState*, uintptr_t track);
double trackergridstate_tracktopx(TrackerGridState*, uintptr_t track);
uintptr_t trackergridstate_pxtotrack(const TrackerGridState*, double x);
double trackergridstate_basewidth(TrackerGridState*, uintptr_t track);
uintptr_t trackergridstate_paramcol(TrackerGridState*, uintptr_t track,
	double x);


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

#endif /* TRACKERGRIDSTATE */

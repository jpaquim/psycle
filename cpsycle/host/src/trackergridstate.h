// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

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

	CMD_FOLLOWSONG,
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


typedef struct {
	uintptr_t numdigits;
	uintptr_t numchars;
	uintptr_t marginright;
	int wrapeditcolumn;
	int wrapclearcolumn;
	uintptr_t emptyvalue;
} TrackColumnDef;

void trackcolumndef_init(TrackColumnDef* self, int numdigits, int numchars,
	int marginright, int wrapeditcolumn, int wrapclearcolumn, int emptyvalue);

typedef struct {
	TrackColumnDef note;
	TrackColumnDef inst;
	TrackColumnDef mach;
	TrackColumnDef vol;
	TrackColumnDef cmd;
	TrackColumnDef param;
	int numfx;
} TrackDef;

void trackdef_init(TrackDef*);
uintptr_t trackdef_numdigits(TrackDef*, uintptr_t column);
int trackdef_columnwidth(TrackDef*, int column, int textwidth);
TrackColumnDef* trackdef_columndef(TrackDef*, int column);
uintptr_t trackdef_numcolumns(TrackDef*);
uintptr_t trackdef_value(TrackDef*, uintptr_t column,
	const psy_audio_PatternEntry*);
uintptr_t trackdef_emptyvalue(TrackDef*, uintptr_t column);
void trackdef_setvalue(TrackDef*, uintptr_t column,
	psy_audio_PatternEntry*, uintptr_t value);
int trackdef_width(TrackDef*, int textwidth);
int trackdef_basewidth(TrackDef* self, int textwidth);
TrackColumnDef* trackdef_columndef(TrackDef* self, int column);
int trackdef_columnwidth(TrackDef* self, int column, int textwidth);
int trackdef_width(TrackDef* self, int textwidth);
int trackdef_basewidth(TrackDef* self, int textwidth);
uintptr_t trackdef_emptyvalue(TrackDef* self, uintptr_t column);
uintptr_t trackdef_value(TrackDef* self, uintptr_t column,
	const psy_audio_PatternEntry* entry);
void trackdef_setvalue(TrackDef* self, uintptr_t column,
	psy_audio_PatternEntry* entry, uintptr_t value);
uintptr_t trackdef_numcolumns(TrackDef* self);
uintptr_t trackdef_numdigits(TrackDef* self, uintptr_t column);


typedef struct {
	psy_Table trackconfigs;
	TrackDef trackdef;
	int textwidth;
	int textleftedge;
	int patterntrackident;
	int headertrackident;
} TrackConfig;

void trackconfig_init(TrackConfig*, bool wideinst);
void trackconfig_dispose(TrackConfig*);
void trackconfig_initcolumns(TrackConfig*, bool wideinst);

typedef struct {
	psy_audio_Pattern* pattern;
	psy_audio_PatternCursor cursor;
	PatternViewSkin* skin;	
	TrackConfig* trackconfig;	
	uintptr_t numtracks;
	double zoom;
} TrackerGridState;

void trackergridstate_init(TrackerGridState*, TrackConfig* config);
void trackergridstate_dispose(TrackerGridState*);
int trackergridstate_trackwidth(TrackerGridState*, uintptr_t track);
TrackDef* trackergridstate_trackdef(TrackerGridState*, uintptr_t track);
int trackergridstate_track_x(TrackerGridState*, uintptr_t track);
uintptr_t trackergridstate_screentotrack(TrackerGridState*, int x,
	uintptr_t numsongtracks);
uintptr_t trackergridstate_basewidth(TrackerGridState*, uintptr_t track);
uintptr_t trackergridstate_paramcol(TrackerGridState*, uintptr_t track, int x);

INLINE int trackergridstate_preferredtrackwidth(TrackerGridState* self)
{
	if (self->skin) {
		return self->skin->headercoords.background.destwidth;
	}
	return 0;
}

#endif /* TRACKERGRIDSTATE */

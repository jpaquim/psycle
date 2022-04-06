/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#if !defined(TRACKERGRIDSTATE_H)
#define TRACKERGRIDSTATE_H

/* host */
#include "patternviewstate.h"
#include "trackercmds.h"
#include "workspace.h"
/* audio */
#include <pattern.h>
#include <sequence.h>


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

/* TrackDrag */
typedef struct TrackDrag {
	bool active;	
	uintptr_t track;
	psy_ui_RealSize size;
} TrackDrag;

void trackdrag_init(TrackDrag*);

void trackdrag_start(TrackDrag*, uintptr_t track, double width);
void trackdrag_stop(TrackDrag*);
bool trackdrag_active(const TrackDrag*);
bool trackdrag_trackactive(const TrackDrag*, uintptr_t track);

/* TrackDef */
typedef struct TrackDef {
	TrackColumnDef note;
	TrackColumnDef inst;
	TrackColumnDef mach;
	TrackColumnDef vol;
	TrackColumnDef cmd;
	TrackColumnDef param;	
	uintptr_t visinotes;
} TrackDef;

void trackdef_init(TrackDef*);

TrackDef* trackdef_alloc(void);
TrackDef* trackdef_allocinit(void);
void trackdef_deallocate(TrackDef*);

uintptr_t trackdef_numdigits(TrackDef*, uintptr_t column);
uintptr_t trackdef_numcolumns(const TrackDef*);

INLINE uintptr_t trackdef_visinotes(const TrackDef* self)
{	
	return self->visinotes;
}

uintptr_t trackdef_value(TrackDef*, uintptr_t column,
	const psy_audio_PatternEntry*);
uintptr_t trackdef_event_value(TrackDef*, uintptr_t column,
	const psy_audio_PatternEvent*);
uintptr_t trackdef_emptyvalue(TrackDef*, uintptr_t column);
double trackdef_basewidth(const TrackDef* self, double textwidth);
double trackdef_defaulttrackwidth(const TrackDef*, double textwidth);
TrackColumnDef* trackdef_columndef(TrackDef*, intptr_t column);
double trackdef_columnwidth(const TrackDef*, intptr_t column,
	double textwidth);
double trackdef_marginright(TrackDef*, intptr_t column);
psy_audio_PatternEvent trackdef_setevent_digit(TrackDef*,
	uintptr_t column, uintptr_t digit, const psy_audio_PatternEvent*,
	uintptr_t digitvalue);

typedef struct TrackConfig {
	psy_Table trackconfigs;
	TrackDef trackdef;
	double textwidth;
	double textleftedge;
	TrackDrag resize;
	bool multicolumn;
} TrackConfig;

void trackconfig_init(TrackConfig*, bool wideinst);
void trackconfig_dispose(TrackConfig*);

void trackconfig_init_columns(TrackConfig*, bool wideinst);
double trackconfig_width(const TrackConfig*, uintptr_t track);
double trackconfig_width_cmdparam(const TrackConfig* self);
TrackDef* trackconfig_insert_trackdef(TrackConfig*, uintptr_t track,
	uintptr_t numnotes);
TrackDef* trackerconfig_trackdef(TrackConfig*, uintptr_t track);
TrackDef* trackerconfig_trackdef_const(const TrackConfig*, uintptr_t track);
void trackconfig_settrack(TrackConfig*, uintptr_t track,
	uintptr_t numnotes);
void trackconfig_resize(TrackConfig*, uintptr_t track, double width);
void trackconfig_track_position(TrackConfig*, uintptr_t track,
	double x, uintptr_t* rv_column, uintptr_t* rv_digit,
	uintptr_t* rv_noteindex);


/* TrackerEventTable */
typedef struct TrackerEventTable {
	psy_Table tracks;	
	psy_audio_BlockSelection clip;
	uintptr_t currcursorline;	
} TrackerEventTable;

void trackereventtable_init(TrackerEventTable*);
void trackereventtable_dispose(TrackerEventTable*);

void trackereventtable_clearevents(TrackerEventTable*);
psy_List** trackereventtable_track(TrackerEventTable*, uintptr_t index);

typedef struct TrackerColumnFlags {
	int playbar;
	int cursor;
	int selection;
	int beat;
	int beat4;
	int mid;
	int focus;
} TrackerColumnFlags;

/* TrackerState */
typedef struct TrackerState {		
	/* internal */
	PatternViewState* pv;	
	TrackConfig* trackconfig;	
	TrackerEventTable trackevents;	
	bool showemptydata;
	bool midline;
	bool drawbeathighlights;	
	bool draw_playbar;	
	psy_audio_PatternEntry empty;
	psy_ui_Value lineheight;
	psy_ui_Value defaultlineheight;
	double lineheightpx;
	double flatsize;	
	/* precomputed */
	intptr_t visilines;	
} TrackerState;

void trackerstate_init(TrackerState*, TrackConfig*, PatternViewState* pvstate);
void trackerstate_dispose(TrackerState*);

double trackerstate_trackwidth(const TrackerState*, uintptr_t track);
uintptr_t trackerstate_px_to_track(const TrackerState*, double x);
double trackerstate_basewidth(TrackerState*, uintptr_t track);

void trackerstate_clip(TrackerState*, const psy_ui_RealRectangle* clip,
	psy_audio_BlockSelection* rv);
void trackerstate_update_clip_events(TrackerState*,
	const psy_audio_BlockSelection* clip);

void trackerstate_start_drag_selection(TrackerState*, psy_audio_SequenceCursor);
void trackerstate_drag_selection(TrackerState*, psy_audio_SequenceCursor);
psy_audio_SequenceCursor trackerstate_checkcursorbounds(TrackerState*,
	psy_audio_SequenceCursor);

/* quantized */
INLINE double trackerstate_beat_to_px(TrackerState* self,
	psy_dsp_big_beat_t position)
{
	assert(self);

	return self->lineheightpx * patternviewstate_beat_to_line(self->pv,
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

void trackerstate_line_clip(TrackerState*, const psy_ui_RealRectangle* clip,
	psy_audio_BlockSelection* rv);

INLINE uintptr_t trackerstate_midline(const TrackerState* self,
	double scrolltop_px)
{
	return patternviewstate_beat_to_line(self->pv, trackerstate_pxtobeat(
		self, scrolltop_px) + self->visilines / 2);
}

void trackerstate_update_metric(TrackerState*, const psy_ui_TextMetric*,
	double lineheight);

psy_audio_SequenceCursor trackerstate_make_cursor(TrackerState*,
	psy_ui_RealPoint pt, uintptr_t index);
void trackerstate_columncolours(TrackerState*, TrackerColumnFlags,
	uintptr_t track, psy_ui_Colour* bg, psy_ui_Colour* fore);

#endif /* TRACKERGRIDSTATE_H */

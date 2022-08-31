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

uintptr_t trackdef_num_digits(TrackDef*, uintptr_t column);
uintptr_t trackdef_num_columns(const TrackDef*);

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
TrackColumnDef* trackdef_column_def(TrackDef*, intptr_t column);
double trackdef_column_width(const TrackDef*, intptr_t column,
	double textwidth);
double trackdef_marginright(TrackDef*, intptr_t column);
psy_audio_PatternEvent trackdef_setevent_digit(TrackDef*,
	uintptr_t column, uintptr_t digit, const psy_audio_PatternEvent*,
	uintptr_t digitvalue);

typedef struct TrackConfig {
	psy_Table trackconfigs;
	TrackDef trackdef;	
	double textleftedge;
	psy_ui_Value flatsize;
	TrackDrag resize;
	bool multicolumn;
} TrackConfig;

void trackconfig_init(TrackConfig*, bool wideinst);
void trackconfig_dispose(TrackConfig*);

void trackconfig_init_columns(TrackConfig*, bool wideinst);
double trackconfig_width(const TrackConfig*, uintptr_t track,
	const psy_ui_TextMetric*);
double trackconfig_width_cmdparam(const TrackConfig*, const psy_ui_TextMetric*);
TrackDef* trackconfig_insert_trackdef(TrackConfig*, uintptr_t track,
	uintptr_t numnotes);
TrackDef* trackerconfig_trackdef(TrackConfig*, uintptr_t track);
TrackDef* trackerconfig_trackdef_const(const TrackConfig*, uintptr_t track);
void trackconfig_settrack(TrackConfig*, uintptr_t track,
	uintptr_t numnotes);
void trackconfig_resize(TrackConfig*, uintptr_t track, double width,
	const psy_ui_TextMetric*);
void trackconfig_track_position(TrackConfig*, uintptr_t track, double x,
	const psy_ui_TextMetric* tm,
	uintptr_t* rv_column, uintptr_t* rv_digit, uintptr_t* rv_noteindex);


/* TrackerEventTable */

typedef struct TrackerEventPair {
	double offset;
	double seqoffset;
	psy_audio_PatternEntry* entry;
} TrackerEventPair;

TrackerEventPair* trackereventpair_allocinit(double offset,
	double seqoffset, psy_audio_PatternEntry*);

typedef struct TrackerEventTable {
	psy_Table tracks;	
	double top;
	double seqoffset;
	uintptr_t left;
	uintptr_t right;
	psy_dsp_big_beat_t selection_top_abs;
	psy_dsp_big_beat_t selection_bottom_abs;
} TrackerEventTable;

void trackereventtable_init(TrackerEventTable*);
void trackereventtable_dispose(TrackerEventTable*);

void trackereventtable_clear_events(TrackerEventTable*);
psy_List** trackereventtable_track(TrackerEventTable*, uintptr_t index);
void trackereventtable_prepare_selection(TrackerEventTable*,
	psy_audio_Sequence*, const psy_audio_BlockSelection*);

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
	BeatConvert beat_convert;
	TrackConfig* track_config;	
	TrackerEventTable track_events;	
	bool show_empty_data;
	bool midline;
	bool draw_beat_highlights;	
	bool draw_playbar;	
	psy_audio_PatternEntry empty;
	psy_ui_Value line_height;	
} TrackerState;

void trackerstate_init(TrackerState*, TrackConfig*, PatternViewState* pvstate);
void trackerstate_dispose(TrackerState*);

double trackerstate_trackwidth(const TrackerState*, uintptr_t track,
	const psy_ui_TextMetric*);
uintptr_t trackerstate_px_to_track(const TrackerState*, double x,
	const psy_ui_TextMetric*);
double trackerstate_base_width(TrackerState*, uintptr_t track,
	const psy_ui_TextMetric*);

void trackerstate_track_clip(TrackerState*, const psy_ui_RealRectangle*,
	const psy_ui_TextMetric*, uintptr_t* rv_left, uintptr_t* rv_right);
void trackerstate_update_clip_events(TrackerState*,
	psy_ui_RealRectangle* g_clip, const psy_ui_TextMetric*);
void trackerstate_update_abs_positions(TrackerState*, const psy_audio_Player*);	

psy_audio_SequenceCursor trackerstate_checkcursorbounds(TrackerState*,
	psy_audio_SequenceCursor);

/* quantized */
INLINE double trackerstate_beat_to_px(TrackerState* self,
	psy_dsp_big_beat_t position)
{	
	assert(self);
		
	return beatconvert_beat_to_px(&self->beat_convert, position);
}

/* quantized */
INLINE psy_dsp_big_beat_t trackerstate_px_to_beat(
	const TrackerState* self, double px)
{	
	assert(self);
		
	return beatconvert_px_to_beat(&self->beat_convert, px);	
}

INLINE uintptr_t trackerstate_mid_line(const TrackerState* self,
	double scrolltop_px, double client_height)
{	
	intptr_t visilines;
		
	visilines = (intptr_t)(client_height / self->beat_convert.line_px);	
	return beatline_beat_to_line(self->beat_convert.beat_line,
		trackerstate_px_to_beat(self, scrolltop_px) + visilines / 2);
}

psy_audio_SequenceCursor trackerstate_make_cursor(TrackerState*,
	psy_ui_RealPoint pt, uintptr_t index, double line_height,
	const psy_ui_TextMetric*);
psy_ui_Style* trackerstate_column_style(TrackerState*, TrackerColumnFlags,
	uintptr_t track);
	
void trackerstate_update_textmetric(TrackerState*, const psy_ui_TextMetric*);

#endif /* TRACKERGRIDSTATE_H */

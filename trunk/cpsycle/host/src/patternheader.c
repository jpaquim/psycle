// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "patternheader.h"
// local
#include "patterncmds.h"
#include "skingraphics.h"
// std
#include <math.h>
// platform
#include "../../detail/portable.h"
#include "../../detail/trace.h"

// TrackerHeader
// prototypes
static void trackerheader_ondestroy(TrackerHeader*);
static TrackerHeaderTrackState* trackerheader_trackstate(TrackerHeader*,
	uintptr_t track);
static TrackerHeaderTrackState* trackerheader_updatetrackstate(TrackerHeader*,
	uintptr_t track);
static void trackerheader_ondraw(TrackerHeader*, psy_ui_Graphics*);
static void trackerheader_drawtrackseparator(TrackerHeader*, psy_ui_Graphics*,
	double x, uintptr_t track);
static void trackerheader_drawtrackbackground(TrackerHeader*, psy_ui_Graphics*,
	psy_ui_RealPoint dest);
static void trackerheader_drawtrackplayon(TrackerHeader*, psy_ui_Graphics*,
	psy_ui_RealPoint dest, uintptr_t track);
static void trackerheader_drawtracknumber(TrackerHeader*, psy_ui_Graphics*,
	psy_ui_RealPoint dest, uintptr_t track);
static void trackerheader_drawtrackleds(TrackerHeader*, psy_ui_Graphics*,
	psy_ui_RealPoint dest, uintptr_t track);
static void trackerheader_drawtrackledmute(TrackerHeader*, psy_ui_Graphics*,
	psy_ui_RealPoint dest, uintptr_t track);
static void trackerheader_drawtrackledsoloed(TrackerHeader* self, psy_ui_Graphics*,
	psy_ui_RealPoint dest, uintptr_t track);
static void trackerheader_drawtrackledarmed(TrackerHeader* self, psy_ui_Graphics*,
	psy_ui_RealPoint dest, uintptr_t track);
static void trackerheader_drawtrackselection(TrackerHeader*, psy_ui_Graphics*,
	psy_ui_RealPoint dest, uintptr_t track);
static void trackerheader_onmousedown(TrackerHeader*, psy_ui_MouseEvent*);
double trackerheader_trackposition(const TrackerHeader* self, uintptr_t track);
static double trackerheader_centerx(const TrackerHeader* self,
	uintptr_t track);
static bool trackerheader_hittest_mute(const TrackerHeader*, double x, double y);
static bool trackerheader_hittest_solo(const TrackerHeader*, double x, double y);
static bool trackerheader_hittest_record(const TrackerHeader*, double x, double y);
static void trackerheader_onpreferredsize(TrackerHeader*,
	const psy_ui_Size* limit, psy_ui_Size* rv);
static void trackerheader_onpatterncursorchanged(TrackerHeader*, Workspace*);
static void trackerheader_ontimer(TrackerHeader*, uintptr_t timerid);
static bool trackerheader_hasredraw(const TrackerHeader*);
static bool trackerheader_hastrackredraw(const TrackerHeader*,
	uintptr_t track);
// vtable
static psy_ui_ComponentVtable trackerheader_vtable;
static bool trackerheader_vtable_initialized = FALSE;

static void trackerheader_vtable_init(TrackerHeader* self)
{
	if (!trackerheader_vtable_initialized) {
		trackerheader_vtable = *(self->component.vtable);
		trackerheader_vtable.ondestroy = (psy_ui_fp_component_ondestroy)
			trackerheader_ondestroy;
		trackerheader_vtable.ontimer = (psy_ui_fp_component_ontimer)
			trackerheader_ontimer;
		trackerheader_vtable.ondraw = (psy_ui_fp_component_ondraw)
			trackerheader_ondraw;
		trackerheader_vtable.onmousedown = (psy_ui_fp_component_onmousedown)
			trackerheader_onmousedown;
		trackerheader_vtable.onpreferredsize =
			(psy_ui_fp_component_onpreferredsize)
			trackerheader_onpreferredsize;
		trackerheader_vtable_initialized = TRUE;
	}
}
// implementation
void trackerheader_init(TrackerHeader* self, psy_ui_Component* parent,
	TrackConfig* trackconfig, TrackerGridState* gridstate,
	Workspace* workspace)
{
	psy_ui_component_init(&self->component, parent);
	trackerheader_vtable_init(self);
	self->component.vtable = &trackerheader_vtable;
	trackerheader_setsharedgridstate(self, gridstate, trackconfig);	
	psy_ui_component_doublebuffer(&self->component);
	self->classic = TRUE;
	self->workspace = workspace;
	self->currtrack = 0;
	psy_table_init(&self->trackstates);
	self->playing = FALSE;
	psy_signal_connect(&self->workspace->signal_patterncursorchanged, self,
		trackerheader_onpatterncursorchanged);	
	psy_ui_component_starttimer(&self->component, 0, 50);
}

void trackerheader_ondestroy(TrackerHeader* self)
{
	psy_table_disposeall(&self->trackstates, (psy_fp_disposefunc)NULL);
}

void trackerheader_setsharedgridstate(TrackerHeader* self, TrackerGridState*
	gridstate, TrackConfig* trackconfig)
{
	if (gridstate) {
		self->gridstate = gridstate;
	} else {
		trackergridstate_init(&self->defaultgridstate, trackconfig);
		self->gridstate = &self->defaultgridstate;
	}
}

TrackerHeaderTrackState* trackerheader_trackstate(TrackerHeader* self,
	uintptr_t track)
{
	TrackerHeaderTrackState* trackstate;

	trackstate = (TrackerHeaderTrackState*)
		psy_table_at(&self->trackstates, track);
	if (!trackstate) {
		trackstate = (TrackerHeaderTrackState*)malloc(sizeof(
			TrackerHeaderTrackState));
		if (trackstate) {
			trackstate->playon = FALSE;
		}
	}
	return trackstate;
}

TrackerHeaderTrackState* trackerheader_updatetrackstate(TrackerHeader* self,
	uintptr_t track)
{
	TrackerHeaderTrackState* trackstate;

	trackstate = trackerheader_trackstate(self, track);
	trackstate->playon = psy_audio_activechannels_playon(
		&workspace_player(self->workspace)->playon, track);
	return trackstate;
}

void trackerheader_ondraw(TrackerHeader* self, psy_ui_Graphics* g)
{
	psy_ui_RealSize size;	
	double cpx;	
	uintptr_t track;
	double scrollleft;

	size = psy_ui_component_sizepx(&self->component);		
	scrollleft = psy_ui_component_scrollleftpx(&self->component);
	psy_ui_drawsolidrectangle(g,
		psy_ui_realrectangle_make(psy_ui_realpoint_make(scrollleft, 0.0),
			size),
		self->gridstate->skin->background);
	for (track = 0, cpx = 0; track < trackergridstate_numsongtracks(
			self->gridstate); ++track) {		
		psy_ui_RealPoint dest;
				
		dest = psy_ui_realpoint_make(cpx + trackerheader_centerx(self, track),
			0.0);
		trackerheader_drawtrackseparator(self, g, cpx, track);
		trackerheader_drawtrackbackground(self, g, dest);
		trackerheader_drawtrackplayon(self, g, dest, track);
		trackerheader_drawtracknumber(self, g, dest, track);
		trackerheader_drawtrackleds(self, g, dest, track);
		trackerheader_drawtrackselection(self, g, dest, track);
		cpx += trackergridstate_trackwidth(self->gridstate, track);
	}
}

void trackerheader_drawtrackseparator(TrackerHeader* self, psy_ui_Graphics* g,
	double x, uintptr_t track)
{
	double trackwidth;
	psy_ui_RealSize size;	

	size = psy_ui_component_sizepx(&self->component);
	trackwidth = trackergridstate_trackwidth(self->gridstate, track);
	psy_ui_setcolour(g, self->gridstate->skin->separator);
	psy_ui_drawline(g, psy_ui_realpoint_make(x + trackwidth - 1, 0),
		psy_ui_realpoint_make(x + trackwidth - 1, size.height));
}

void trackerheader_drawtrackbackground(TrackerHeader* self, psy_ui_Graphics* g,
	psy_ui_RealPoint dest)
{
	skin_blitpart(g, &self->gridstate->skin->bitmap, dest,
		&self->gridstate->skin->headercoords.background);
}

void trackerheader_drawtrackplayon(TrackerHeader* self, psy_ui_Graphics* g,
	psy_ui_RealPoint dest, uintptr_t track)
{
	TrackerHeaderTrackState* trackstate;

	trackstate = trackerheader_updatetrackstate(self, track);
	if (trackstate->playon) {
		skin_blitpart(g, &self->gridstate->skin->bitmap,
			psy_ui_realpoint_make(dest.x, 0),
			&self->gridstate->skin->headercoords.dplayon);
	}
}

void trackerheader_drawtracknumber(TrackerHeader* self, psy_ui_Graphics* g,
	psy_ui_RealPoint dest, uintptr_t track)
{
	intptr_t trackx0 = track / 10;
	intptr_t track0x = track % 10;
	SkinCoord digitx0 = self->gridstate->skin->headercoords.digitx0;
	SkinCoord digit0x = self->gridstate->skin->headercoords.digit0x;
	digitx0.src.left += trackx0 * psy_ui_realrectangle_width(&digitx0.src);
	digit0x.src.left += track0x * psy_ui_realrectangle_width(&digit0x.src);
	skin_blitpart(g, &self->gridstate->skin->bitmap, dest, &digitx0);
	skin_blitpart(g, &self->gridstate->skin->bitmap, dest, &digit0x);
}

void trackerheader_drawtrackleds(TrackerHeader* self, psy_ui_Graphics* g,
	psy_ui_RealPoint dest, uintptr_t track)
{
	if (trackergridstate_patterns(self->gridstate)) {
		trackerheader_drawtrackledmute(self, g, dest, track);
		trackerheader_drawtrackledsoloed(self, g, dest, track);
		trackerheader_drawtrackledarmed(self, g, dest, track);		
	}
}

void trackerheader_drawtrackledmute(TrackerHeader* self, psy_ui_Graphics* g,
	psy_ui_RealPoint dest, uintptr_t track)
{	
	if (psy_audio_patterns_istrackmuted(
		trackergridstate_patterns(self->gridstate), track)) {
		skin_blitpart(g, &self->gridstate->skin->bitmap,
			dest, &self->gridstate->skin->headercoords.mute);
	}
}

void trackerheader_drawtrackledsoloed(TrackerHeader* self,
	psy_ui_Graphics* g, psy_ui_RealPoint dest, uintptr_t track)
{
	if (psy_audio_patterns_istracksoloed(
		trackergridstate_patterns(self->gridstate), track)) {
		skin_blitpart(g, &self->gridstate->skin->bitmap,
			dest, &self->gridstate->skin->headercoords.solo);
	}
}

void trackerheader_drawtrackledarmed(TrackerHeader* self, psy_ui_Graphics* g,
	psy_ui_RealPoint dest, uintptr_t track)
{	
	if (psy_audio_patterns_istrackarmed(
		trackergridstate_patterns(self->gridstate), track)) {
		skin_blitpart(g, &self->gridstate->skin->bitmap,
			dest, &self->gridstate->skin->headercoords.record);
	}
}

void trackerheader_drawtrackselection(TrackerHeader* self, psy_ui_Graphics* g,
	psy_ui_RealPoint dest, uintptr_t track)
{		
	if (track == self->gridstate->cursor.track) {
		psy_ui_setcolour(g, self->gridstate->skin->font);
		psy_ui_drawline(g, dest,
			psy_ui_realpoint_make(
				psy_min(
					dest.x + self->gridstate->skin->headercoords.background.dest.right -
						self->gridstate->skin->headercoords.background.dest.left,
					dest.x + trackergridstate_trackwidth(self->gridstate, track)),
				0));
	}
}

void trackerheader_onmousedown(TrackerHeader* self, psy_ui_MouseEvent* ev)
{
	if (trackergridstate_patterns(self->gridstate)) {
		uintptr_t track;
		double track_x;
		double ev_track_x;
		psy_audio_Patterns* patterns;		

		patterns = trackergridstate_patterns(self->gridstate);
		track = trackergridstate_pxtotrack(self->gridstate, ev->x);	
		track_x = trackerheader_trackposition(self, track);
		ev_track_x = ev->x - track_x;		
		 if (trackerheader_hittest_solo(self, ev_track_x, ev->y)) {
			if (psy_audio_patterns_istracksoloed(patterns, track)) {
				psy_audio_patterns_deactivatesolotrack(patterns);
			} else {
				psy_audio_patterns_activatesolotrack(patterns, track);
			}
			psy_ui_component_invalidate(&self->component);
		} else if (trackerheader_hittest_mute(self, ev_track_x, ev->y)) {
			if (psy_audio_patterns_istrackmuted(patterns, track)) {
				psy_audio_patterns_unmutetrack(patterns, track);
			} else {
				psy_audio_patterns_mutetrack(patterns, track);
			}
			psy_ui_component_invalidate(&self->component);			
		} else if (trackerheader_hittest_record(self, ev_track_x, ev->y)) {
			if (psy_audio_patterns_istrackarmed(patterns, track)) {
				psy_audio_patterns_unarmtrack(patterns, track);
			} else {
				psy_audio_patterns_armtrack(patterns, track);
			}
			psy_ui_component_invalidate(&self->component);			
		} else {
			psy_audio_PatternCursor cursor;

			cursor = workspace_patterncursor(self->workspace);
			cursor.track = track;
			workspace_setpatterncursor(self->workspace, cursor);
			++self->gridstate->pattern->opcount;
		}
	}
}

bool trackerheader_hittest_mute(const TrackerHeader* self, double x, double y)
{
	return skincoord_hittest(&self->gridstate->skin->headercoords.mute, x, y);
}

bool trackerheader_hittest_solo(const TrackerHeader* self, double x, double y)
{
	return skincoord_hittest(&self->gridstate->skin->headercoords.solo, x, y);
}

bool trackerheader_hittest_record(const TrackerHeader* self, double x, double y)
{
	return skincoord_hittest(&self->gridstate->skin->headercoords.record, x, y);
}

double trackerheader_trackposition(const TrackerHeader* self, uintptr_t track)
{
	return trackergridstate_tracktopx(self->gridstate, track) +
		trackerheader_centerx(self, track);
}

double trackerheader_centerx(const TrackerHeader* self, uintptr_t track)
{
	return psy_max(0.0,
		((trackergridstate_trackwidth(self->gridstate, track) -
		self->gridstate->skin->headercoords.background.dest.right -
		self->gridstate->skin->headercoords.background.dest.left)) / 2);
}

void trackerheader_onpreferredsize(TrackerHeader* self,
	const psy_ui_Size* limit, psy_ui_Size* rv)
{
	rv->width = psy_ui_value_makepx(trackergridstate_tracktopx(self->gridstate,
		trackergridstate_numsongtracks(self->gridstate)));
	rv->height = psy_ui_value_makepx(30);
}

void trackerheader_onpatterncursorchanged(TrackerHeader* self,
	Workspace* sender)
{
	psy_audio_PatternCursor cursor;

	cursor = workspace_patterncursor(sender);
	if (self->currtrack != cursor.track) {
		self->currtrack = cursor.track;
		psy_ui_component_invalidate(&self->component);
	}
}

void trackerheader_ontimer(TrackerHeader* self, uintptr_t timerid)
{	
	if (psy_ui_component_drawvisible(&self->component) &&
			trackerheader_hasredraw(self)) {
		psy_ui_component_invalidate(&self->component);
	}
}

bool trackerheader_hasredraw(const TrackerHeader* self)
{
	bool rv;
	uintptr_t track;

	rv = FALSE;
	for (track = 0; track < trackergridstate_numsongtracks(self->gridstate);
			++track) {
		if (trackerheader_hastrackredraw(self, track)) {
			rv = TRUE;
			break;
		}
	}
	return rv;
}

bool trackerheader_hastrackredraw(const TrackerHeader* self, uintptr_t track)
{
	TrackerHeaderTrackState* trackstate;

	trackstate = trackerheader_trackstate((TrackerHeader*)self, track);
	return (psy_audio_activechannels_playon(
		&workspace_player(self->workspace)->playon, track) !=
		trackstate->playon);
}

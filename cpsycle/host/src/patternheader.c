// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "patternheader.h"
// local
#include "patterncmds.h"
#include "skingraphics.h"
// std
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <assert.h>
// platform
#include "../../detail/trace.h"
#include "../../detail/portable.h"

// TrackerHeader
// prototypes
static void trackerheader_ondestroy(TrackerHeader*);
static TrackerHeaderTrackState* trackerheader_trackstate(TrackerHeader*, uintptr_t track);
static TrackerHeaderTrackState* trackerheader_updatetrackstate(TrackerHeader*, uintptr_t track);
static void trackerheader_ondraw(TrackerHeader*, psy_ui_Graphics*);
static void trackerheader_drawtrackseparator(TrackerHeader* self, psy_ui_Graphics* g, intptr_t x, uintptr_t track);
static void trackerheader_drawtrackbackground(TrackerHeader*, psy_ui_Graphics*, intptr_t x);
static void trackerheader_drawtrackplayon(TrackerHeader* self, psy_ui_Graphics* g, intptr_t x, uintptr_t track);
static void trackerheader_drawtracknumber(TrackerHeader*, psy_ui_Graphics*, intptr_t x, uintptr_t track);
static void trackerheader_drawtrackleds(TrackerHeader*, psy_ui_Graphics*, intptr_t x, uintptr_t track);
static void trackerheader_drawtrackselection(TrackerHeader*, psy_ui_Graphics*, intptr_t x, uintptr_t track);
static void trackerheader_onmousedown(TrackerHeader*, psy_ui_MouseEvent*);
static void trackerheader_onpreferredsize(TrackerHeader*, psy_ui_Size* limit, psy_ui_Size* rv);
static void trackerheader_onpatterncursorchanged(TrackerHeader*, Workspace*);
static void trackerheader_ontimer(TrackerHeader*, uintptr_t timerid);
static bool trackerheader_hasredraw(TrackerHeader*);
static bool trackerheader_hastrackredraw(TrackerHeader*, uintptr_t track);
static void trackerheader_numtrackschanged(TrackerHeader* self, psy_audio_Player* player,
	uintptr_t numsongtracks);
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
	TrackConfig* trackconfig, TrackerGridState* gridstate, Workspace* workspace)
{
	psy_ui_component_init(&self->component, parent);
	trackerheader_vtable_init(self);
	self->component.vtable = &trackerheader_vtable;
	trackerheader_setsharedgridstate(self, gridstate, trackconfig);	
	psy_ui_component_doublebuffer(&self->component);
	self->classic = 1;
	self->workspace = workspace;
	self->currtrack = 0;
	psy_table_init(&self->trackstates);
	self->playing = FALSE;
	psy_signal_connect(&self->workspace->signal_patterncursorchanged, self,
		trackerheader_onpatterncursorchanged);
	psy_signal_connect(&workspace_player(self->workspace)->signal_numsongtrackschanged, self,
		trackerheader_numtrackschanged);
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

TrackerHeaderTrackState* trackerheader_trackstate(TrackerHeader* self, uintptr_t track)
{
	TrackerHeaderTrackState* trackstate;

	trackstate = (TrackerHeaderTrackState*)psy_table_at(&self->trackstates, track);
	if (!trackstate) {
		trackstate = (TrackerHeaderTrackState*)malloc(sizeof(
			TrackerHeaderTrackState));
		if (trackstate) {
			trackstate->playon = FALSE;
		}
	}
	return trackstate;
}

TrackerHeaderTrackState* trackerheader_updatetrackstate(TrackerHeader* self, uintptr_t track)
{
	TrackerHeaderTrackState* trackstate;

	trackstate = trackerheader_trackstate(self, track);
	trackstate->playon = psy_audio_activechannels_playon(
		&workspace_player(self->workspace)->playon, track);
	return trackstate;
}

void trackerheader_ondraw(TrackerHeader* self, psy_ui_Graphics* g)
{
	psy_ui_Size size;
	psy_ui_TextMetric tm;
	intptr_t cpx;
	intptr_t centerx;
	uintptr_t track;

	size = psy_ui_component_size(&self->component);
	tm = psy_ui_component_textmetric(&self->component);
	psy_ui_drawsolidrectangle(g,
		psy_ui_rectangle_make(psy_ui_component_scrollleft(&self->component), 0,
			psy_ui_value_px(&size.width, &tm), psy_ui_value_px(&size.height, &tm)),
		self->gridstate->skin->background);
	for (track = 0, cpx = 0; track < self->gridstate->numtracks; ++track) {
		centerx = psy_max(0, (trackergridstate_trackwidth(self->gridstate, track) -
			self->gridstate->skin->headercoords.background.destwidth) / 2);
		trackerheader_drawtrackseparator(self, g, cpx, track);
		trackerheader_drawtrackbackground(self, g, cpx + centerx);
		trackerheader_drawtrackplayon(self, g, cpx + centerx, track);
		trackerheader_drawtracknumber(self, g, cpx + centerx, track);
		trackerheader_drawtrackleds(self, g, cpx + centerx, track);
		trackerheader_drawtrackselection(self, g, cpx + centerx, track);
		cpx += trackergridstate_trackwidth(self->gridstate, track);
	}
}

void trackerheader_drawtrackseparator(TrackerHeader* self, psy_ui_Graphics* g, intptr_t x, uintptr_t track)
{
	intptr_t trackwidth;
	psy_ui_Size size;
	psy_ui_TextMetric tm;

	size = psy_ui_component_size(&self->component);
	tm = psy_ui_component_textmetric(&self->component);
	trackwidth = trackergridstate_trackwidth(self->gridstate, track);
	psy_ui_setcolour(g, self->gridstate->skin->separator);
	psy_ui_drawline(g, x + trackwidth - 1, 0, x + trackwidth - 1,
		psy_ui_value_px(&size.height, &tm));
}

void trackerheader_drawtrackbackground(TrackerHeader* self, psy_ui_Graphics* g, intptr_t x)
{
	skin_blitpart(g, &self->gridstate->skin->bitmap, x, 0,
		&self->gridstate->skin->headercoords.background);
}

void trackerheader_drawtrackplayon(TrackerHeader* self, psy_ui_Graphics* g, intptr_t x, uintptr_t track)
{
	TrackerHeaderTrackState* trackstate;

	trackstate = trackerheader_updatetrackstate(self, track);
	if (trackstate->playon) {
		skin_blitpart(g, &self->gridstate->skin->bitmap, x, 0,
			&self->gridstate->skin->headercoords.dplayon);
	}
}

void trackerheader_drawtracknumber(TrackerHeader* self, psy_ui_Graphics* g, intptr_t x, uintptr_t track)
{
	intptr_t trackx0 = track / 10;
	intptr_t track0x = track % 10;
	SkinCoord digitx0 = self->gridstate->skin->headercoords.digitx0;
	SkinCoord digit0x = self->gridstate->skin->headercoords.digit0x;
	digitx0.srcx += trackx0 * digitx0.srcwidth;
	digit0x.srcx += track0x * digit0x.srcwidth;
	skin_blitpart(g, &self->gridstate->skin->bitmap, x, 0, &digitx0);
	skin_blitpart(g, &self->gridstate->skin->bitmap, x, 0, &digit0x);
}

void trackerheader_drawtrackleds(TrackerHeader* self, psy_ui_Graphics* g, intptr_t x, uintptr_t track)
{
	if (workspace_song(self->workspace)) {
		if (psy_audio_patterns_istrackmuted(&workspace_song(self->workspace)->patterns,
			track)) {
			skin_blitpart(g, &self->gridstate->skin->bitmap, x, 0,
				&self->gridstate->skin->headercoords.mute);
		}
		if (psy_audio_patterns_istracksoloed(&workspace_song(self->workspace)->patterns,
			track)) {
			skin_blitpart(g, &self->gridstate->skin->bitmap, x, 0,
				&self->gridstate->skin->headercoords.solo);
		}
	}
}

void trackerheader_drawtrackselection(TrackerHeader* self, psy_ui_Graphics* g, intptr_t x, uintptr_t track)
{
	psy_audio_PatternCursor editposition;

	editposition = workspace_patterncursor(self->workspace);
	if (track == editposition.track) {
		psy_ui_setcolour(g, self->gridstate->skin->font);
		psy_ui_drawline(g, x, 0,
			psy_min(x + self->gridstate->skin->headercoords.background.destwidth,
				x + trackergridstate_trackwidth(self->gridstate, track)), 0);
	}
}

void trackerheader_onmousedown(TrackerHeader* self, psy_ui_MouseEvent* ev)
{
	if (workspace_song(self->workspace)) {
		psy_ui_Rectangle r;
		uintptr_t track;
		intptr_t track_x;
		intptr_t centerx;

		track = trackergridstate_pxtotrack(self->gridstate,
			ev->x + psy_ui_component_scrollleft(&self->component),
			psy_audio_player_numsongtracks(workspace_player(self->workspace)));
		centerx = psy_max(0, (trackergridstate_trackwidth(self->gridstate, track) -
			self->gridstate->skin->headercoords.background.destwidth) / 2);
		track_x = trackergridstate_tracktopx(self->gridstate, track) + centerx;
		psy_ui_setrectangle(&r,
			self->gridstate->skin->headercoords.mute.destx + track_x,
			self->gridstate->skin->headercoords.mute.desty,
			self->gridstate->skin->headercoords.mute.destwidth,
			self->gridstate->skin->headercoords.mute.destheight);
		if (psy_ui_rectangle_intersect(&r, ev->x +
			psy_ui_component_scrollleft(&self->component), ev->y)) {
			if (psy_audio_patterns_istrackmuted(&workspace_song(self->workspace)->patterns,
				track)) {
				psy_audio_patterns_unmutetrack(&workspace_song(self->workspace)->patterns,
					track);
			} else {
				psy_audio_patterns_mutetrack(&workspace_song(self->workspace)->patterns,
					track);
			}
			psy_ui_component_invalidate(&self->component);
		}
		psy_ui_setrectangle(&r,
			self->gridstate->skin->headercoords.solo.destx + track_x,
			self->gridstate->skin->headercoords.solo.desty,
			self->gridstate->skin->headercoords.solo.destwidth,
			self->gridstate->skin->headercoords.solo.destheight);
		if (psy_ui_rectangle_intersect(&r, ev->x +
			psy_ui_component_scrollleft(&self->component), ev->y)) {
			if (psy_audio_patterns_istracksoloed(&workspace_song(self->workspace)->patterns,
				track)) {
				psy_audio_patterns_deactivatesolotrack(
					&workspace_song(self->workspace)->patterns);
			} else {
				psy_audio_patterns_activatesolotrack(
					&workspace_song(self->workspace)->patterns, track);
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

void trackerheader_onpreferredsize(TrackerHeader* self, psy_ui_Size* limit,
	psy_ui_Size* rv)
{
	rv->width = psy_ui_value_makepx(trackergridstate_tracktopx(self->gridstate,
		self->gridstate->numtracks));
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
	if (workspace_currview(self->workspace) == VIEW_ID_PATTERNVIEW &&
		trackerheader_hasredraw(self)) {
		psy_ui_component_invalidate(&self->component);
	}
}

bool trackerheader_hasredraw(TrackerHeader* self)
{
	bool rv;
	uintptr_t track;

	rv = FALSE;
	for (track = 0; track < self->gridstate->numtracks; ++track) {
		if (trackerheader_hastrackredraw(self, track)) {
			rv = TRUE;
			break;
		}
	}
	return rv;
}

bool trackerheader_hastrackredraw(TrackerHeader* self, uintptr_t track)
{
	TrackerHeaderTrackState* trackstate;

	trackstate = trackerheader_trackstate(self, track);
	return (psy_audio_activechannels_playon(&workspace_player(self->workspace)->playon, track)
		!= trackstate->playon);
}

void trackerheader_numtrackschanged(TrackerHeader* self, psy_audio_Player* player,
	uintptr_t numsongtracks)
{
	self->gridstate->numtracks = numsongtracks;
	psy_ui_component_invalidate(&self->component);
}

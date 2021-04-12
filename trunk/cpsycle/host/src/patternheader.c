// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "patternheader.h"
// local
#include "patterncmds.h"
#include "skingraphics.h"
#include "trackbox.h"
// std
#include <math.h>
// platform
#include "../../detail/portable.h"
#include "../../detail/trace.h"

static void trackdraw_drawseparator(TrackDraw*, psy_ui_Graphics*,
	double x);
static void trackdraw_drawbackground(TrackDraw*, psy_ui_Graphics*);
static void trackdraw_drawplayon(TrackDraw*, psy_ui_Graphics*);
static void trackdraw_drawnumber(TrackDraw*, psy_ui_Graphics*);
static void trackdraw_drawleds(TrackDraw*, psy_ui_Graphics*);
static void trackdraw_drawledmute(TrackDraw*, psy_ui_Graphics*);
static void trackdraw_drawledsoloed(TrackDraw*, psy_ui_Graphics*);
static void trackdraw_drawledarmed(TrackDraw*, psy_ui_Graphics*);
static void trackdraw_drawselection(TrackDraw*, psy_ui_Graphics*);

void trackdraw_init(TrackDraw* self, TrackerGridState* state, uintptr_t index,
	double height, bool playon)
{
	self->gridstate = state;
	self->track = index;
	self->height = height;
	self->playon = playon;
}

void trackdraw_draw(TrackDraw* self, psy_ui_Graphics* g, double cpx, psy_ui_RealPoint dest)
{
	trackdraw_drawseparator(self, g, cpx);
	psy_ui_setorigin(g, psy_ui_realpoint_make(-dest.x, -dest.y));
	trackdraw_drawbackground(self, g);
	trackdraw_drawplayon(self, g);
	trackdraw_drawnumber(self, g);
	trackdraw_drawleds(self, g);
	trackdraw_drawselection(self, g);
	psy_ui_resetorigin(g);
}

void trackdraw_drawseparator(TrackDraw* self, psy_ui_Graphics* g,
	double x)
{
	double trackwidth;

	trackwidth = trackergridstate_trackwidth(self->gridstate, self->track);
	psy_ui_setcolour(g, self->gridstate->skin->separator);
	psy_ui_drawline(g, psy_ui_realpoint_make(x + trackwidth - 1, 0),
		psy_ui_realpoint_make(x + trackwidth - 1, self->height));
}

void trackdraw_drawbackground(TrackDraw* self, psy_ui_Graphics* g)
{
	skin_blitcoord(g, &self->gridstate->skin->bitmap, psy_ui_realpoint_zero(),
		&self->gridstate->skin->headercoords.background);
}

void trackdraw_drawplayon(TrackDraw* self, psy_ui_Graphics* g)
{
	if (self->playon) {
		skin_blitcoord(g, &self->gridstate->skin->bitmap,
			psy_ui_realpoint_make(0, 0),
			&self->gridstate->skin->headercoords.dplayon);
	}
}

void trackdraw_drawnumber(TrackDraw* self, psy_ui_Graphics* g)
{
	intptr_t trackx0 = self->track / 10;
	intptr_t track0x = self->track % 10;
	SkinCoord digitx0 = self->gridstate->skin->headercoords.digitx0;
	SkinCoord digit0x = self->gridstate->skin->headercoords.digit0x;
	digitx0.src.left += trackx0 * psy_ui_realrectangle_width(&digitx0.src);
	digit0x.src.left += track0x * psy_ui_realrectangle_width(&digit0x.src);
	skin_blitcoord(g, &self->gridstate->skin->bitmap, psy_ui_realpoint_zero(), &digitx0);
	skin_blitcoord(g, &self->gridstate->skin->bitmap, psy_ui_realpoint_zero(), &digit0x);
}

void trackdraw_drawleds(TrackDraw* self, psy_ui_Graphics* g)
{
	if (trackergridstate_patterns(self->gridstate)) {
		trackdraw_drawledmute(self, g);
		trackdraw_drawledsoloed(self, g);
		trackdraw_drawledarmed(self, g);
	}
}

void trackdraw_drawledmute(TrackDraw* self, psy_ui_Graphics* g)
{
	if (psy_audio_patterns_istrackmuted(
		trackergridstate_patterns(self->gridstate), self->track)) {
		skin_blitcoord(g, &self->gridstate->skin->bitmap,
			psy_ui_realpoint_zero(),
			&self->gridstate->skin->headercoords.mute);
	}
}

void trackdraw_drawledsoloed(TrackDraw* self,
	psy_ui_Graphics* g)
{
	if (psy_audio_patterns_istracksoloed(
		trackergridstate_patterns(self->gridstate), self->track)) {
		skin_blitcoord(g, &self->gridstate->skin->bitmap,
			psy_ui_realpoint_zero(),
			&self->gridstate->skin->headercoords.solo);
	}
}

void trackdraw_drawledarmed(TrackDraw* self, psy_ui_Graphics* g)
{
	if (psy_audio_patterns_istrackarmed(
		trackergridstate_patterns(self->gridstate), self->track)) {
		skin_blitcoord(g, &self->gridstate->skin->bitmap,
			psy_ui_realpoint_zero(),
			&self->gridstate->skin->headercoords.record);
	}
}

void trackdraw_drawselection(TrackDraw* self, psy_ui_Graphics* g)
{
	if (self->track == self->gridstate->cursor.track) {
		psy_ui_setcolour(g, self->gridstate->skin->font);
		psy_ui_drawline(g, psy_ui_realpoint_zero(),
			psy_ui_realpoint_make(
				psy_min(
					self->gridstate->skin->headercoords.background.dest.right -
					self->gridstate->skin->headercoords.background.dest.left,
					trackergridstate_trackwidth(self->gridstate, self->track)),
				0.0));
	}
}

// plain trackdraw
static void trackplaindraw_drawseparator(TrackPlainDraw*, psy_ui_Graphics*,
	double x);
static void trackplaindraw_drawbackground(TrackPlainDraw*, psy_ui_Graphics*);
static void trackplaindraw_drawplayon(TrackPlainDraw*, psy_ui_Graphics*);
static void trackplaindraw_drawnumber(TrackPlainDraw*, psy_ui_Graphics*);
static void trackplaindraw_drawleds(TrackPlainDraw*, psy_ui_Graphics*);
static void trackplaindraw_drawledmute(TrackPlainDraw*, psy_ui_Graphics*);
static void trackplaindraw_drawledsoloed(TrackPlainDraw*, psy_ui_Graphics*);
static void trackplaindraw_drawledarmed(TrackPlainDraw*, psy_ui_Graphics*);
static void trackplaindraw_drawselection(TrackPlainDraw*, psy_ui_Graphics*);
static psy_ui_RealRectangle realrectangle_zoom(psy_ui_RealRectangle src,
	psy_ui_RealPoint zoom);

void trackplaindraw_init(TrackPlainDraw* self, TrackerGridState* state,
	psy_ui_RealPoint zoom, uintptr_t index,
	double height, bool playon)
{
	self->zoom = zoom;
	self->gridstate = state;
	self->track = index;
	self->height = height;
	self->playon = playon;
	self->coords = trackerheadercoords_default_classic();
}

void trackplaindraw_draw(TrackPlainDraw* self, psy_ui_Graphics* g, double cpx, psy_ui_RealPoint dest)
{
	trackplaindraw_drawseparator(self, g, cpx);
	psy_ui_setorigin(g, psy_ui_realpoint_make(-dest.x, -dest.y));
	trackplaindraw_drawbackground(self, g);
	trackplaindraw_drawplayon(self, g);
	trackplaindraw_drawnumber(self, g);
	trackplaindraw_drawleds(self, g);
	trackplaindraw_drawselection(self, g);
	psy_ui_resetorigin(g);
}

void trackplaindraw_drawseparator(TrackPlainDraw* self, psy_ui_Graphics* g,
	double x)
{
	double trackwidth;

	trackwidth = trackergridstate_trackwidth(self->gridstate, self->track);
	psy_ui_setcolour(g, self->gridstate->skin->separator);
	psy_ui_drawline(g, psy_ui_realpoint_make(x + trackwidth - 1, 0),
		psy_ui_realpoint_make(x + trackwidth - 1, self->height));
}

void trackplaindraw_drawbackground(TrackPlainDraw* self, psy_ui_Graphics* g)
{

	psy_ui_RealRectangle r;
	
	r = self->coords->background.dest;
	psy_ui_realrectangle_expand(&r, 0, -1, -1, 0);
	r = realrectangle_zoom(r, self->zoom);
	psy_ui_drawsolidrectangle(g, r, self->gridstate->skin->background);
	psy_ui_setcolour(g, self->gridstate->skin->row4beat);		
	psy_ui_drawrectangle(g, r);
}

psy_ui_RealRectangle realrectangle_zoom(psy_ui_RealRectangle src, psy_ui_RealPoint zoom)
{
	double width;
	double height;
	
	width = psy_ui_realrectangle_width(&src);
	height = psy_ui_realrectangle_height(&src);
	return psy_ui_realrectangle_make(
		psy_ui_realpoint_make(src.left * zoom.x, src.top * zoom.y),
		psy_ui_realsize_make(width * zoom.x, height * zoom.y));	
}

void trackplaindraw_drawplayon(TrackPlainDraw* self, psy_ui_Graphics* g)
{
	psy_ui_RealRectangle r;

	r = self->coords->dplayon.dest;
	psy_ui_realrectangle_expand(&r, 0, -1, -1, 0);
	r = realrectangle_zoom(r, self->zoom);
	psy_ui_setcolour(g, self->gridstate->skin->row4beat);	
	if (self->playon) {
		psy_ui_drawsolidrectangle(g, r, self->gridstate->skin->selection);		
	} else {
		psy_ui_drawsolidrectangle(g, r, self->gridstate->skin->row);
	}		
}

void trackplaindraw_drawnumber(TrackPlainDraw* self, psy_ui_Graphics* g)
{
	char str[2];

	intptr_t trackx0 = self->track / 10;
	intptr_t track0x = self->track % 10;
	SkinCoord digitx0 = self->coords->digitx0;
	SkinCoord digit0x = self->coords->digit0x;

	digitx0.dest = realrectangle_zoom(digitx0.dest, self->zoom);
	digit0x.dest = realrectangle_zoom(digit0x.dest, self->zoom);
	psy_ui_settextcolour(g, self->gridstate->skin->font);
	psy_snprintf(str, 2, "%X", (int)trackx0);
	psy_ui_textout(g, digitx0.dest.left, digitx0.dest.top, str, strlen(str));
	psy_snprintf(str, 2, "%X", (int)track0x);
	psy_ui_textout(g, digit0x.dest.left + 2, digit0x.dest.top, str, strlen(str));
}

void trackplaindraw_drawleds(TrackPlainDraw* self, psy_ui_Graphics* g)
{
	if (trackergridstate_patterns(self->gridstate)) {
		trackplaindraw_drawledmute(self, g);
		trackplaindraw_drawledsoloed(self, g);
		trackplaindraw_drawledarmed(self, g);
	}
}

void trackplaindraw_drawledmute(TrackPlainDraw* self, psy_ui_Graphics* g)
{
	psy_ui_RealRectangle r;

	r = self->coords->mute.dest;
	psy_ui_realrectangle_expand(&r, 0, -1, -1, 0);
	r = realrectangle_zoom(r, self->zoom);
	psy_ui_setcolour(g, self->gridstate->skin->row4beat);
	psy_ui_drawrectangle(g, r);
	if (psy_audio_patterns_istrackmuted(
			trackergridstate_patterns(self->gridstate), self->track)) {
		psy_ui_settextcolour(g, self->gridstate->skin->fontcur);
	} else {
		psy_ui_settextcolour(g, self->gridstate->skin->font);
	}
	psy_ui_textout(g, r.left + 2, r.top, "M", 1);
}

void trackplaindraw_drawledsoloed(TrackPlainDraw* self,
	psy_ui_Graphics* g)
{
	psy_ui_RealRectangle r;

	r = self->coords->solo.dest;
	psy_ui_realrectangle_expand(&r, 0, -1, -1, 0);
	r = realrectangle_zoom(r, self->zoom);
	psy_ui_setcolour(g, self->gridstate->skin->row4beat);
	psy_ui_drawrectangle(g, r);
	if (psy_audio_patterns_istracksoloed(
			trackergridstate_patterns(self->gridstate), self->track)) {		
		psy_ui_settextcolour(g, self->gridstate->skin->fontcur);
	} else {
		psy_ui_settextcolour(g, self->gridstate->skin->font);
	}
	psy_ui_textout(g, r.left + 2, r.top, "S", 1);
}

void trackplaindraw_drawledarmed(TrackPlainDraw* self, psy_ui_Graphics* g)
{
	psy_ui_RealRectangle r;

	r = self->coords->record.dest;
	psy_ui_realrectangle_expand(&r, 0, -1, -1, 0);
	r = realrectangle_zoom(r, self->zoom);
	psy_ui_setcolour(g, self->gridstate->skin->row4beat);
	psy_ui_drawrectangle(g, r);
	if (psy_audio_patterns_istrackarmed(
		trackergridstate_patterns(self->gridstate), self->track)) {
		psy_ui_settextcolour(g, self->gridstate->skin->fontcur);
	} else {
		psy_ui_settextcolour(g, self->gridstate->skin->font);
	}	
	psy_ui_textout(g, r.left + 2, r.top, "R", 1);
}

void trackplaindraw_drawselection(TrackPlainDraw* self, psy_ui_Graphics* g)
{
	if (self->track == self->gridstate->cursor.track) {
		psy_ui_RealRectangle r;		

		r = self->coords->background.dest;
		psy_ui_realrectangle_expand(&r, 0, -1, -1, 0);
		r = realrectangle_zoom(r, self->zoom);
		psy_ui_realrectangle_setheight(&r, 1.0);
		psy_ui_drawsolidrectangle(g, r, self->gridstate->skin->font);		
	}
}


// TrackerHeader
// prototypes
static void trackerheader_ondestroy(TrackerHeader*);
static TrackerHeaderTrackState* trackerheader_trackstate(TrackerHeader*,
	uintptr_t track);
static TrackerHeaderTrackState* trackerheader_updatetrackstate(TrackerHeader*,
	uintptr_t track);
static void trackerheader_ondraw(TrackerHeader*, psy_ui_Graphics*);
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
		trackerheader_vtable.onmousedown = (psy_ui_fp_component_onmouseevent)
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
	psy_ui_component_init(&self->component, parent, NULL);
	trackerheader_vtable_init(self);
	self->component.vtable = &trackerheader_vtable;
	trackerheader_setsharedgridstate(self, gridstate, trackconfig);	
	psy_ui_component_doublebuffer(&self->component);
	self->classic = TRUE;
	self->workspace = workspace;
	self->currtrack = 0;
	self->usebitmapskin = TRUE;
	trackerheader_updatecoords(self);
	psy_table_init(&self->trackstates);
	self->playing = FALSE;
	psy_signal_connect(&self->workspace->signal_patterncursorchanged, self,
		trackerheader_onpatterncursorchanged);	
	trackerheader_build(self);	
	psy_ui_component_starttimer(&self->component, 0, 50);
}

void trackerheader_ondestroy(TrackerHeader* self)
{
	psy_table_disposeall(&self->trackstates, (psy_fp_disposefunc)NULL);
}

void trackerheader_build(TrackerHeader* self)
{
	/*uintptr_t trackidx;

	psy_ui_component_clear(&self->component);
	for (trackidx = 0; trackidx < trackergridstate_numsongtracks(self->gridstate);
			++trackidx) {
		TrackBox* trackbox;

		trackbox = trackbox_allocinit(&self->component, NULL);
			//&self->component);
		trackbox_setindex(trackbox, trackidx);
		psy_ui_component_setalign(trackbox_base(trackbox), psy_ui_ALIGN_LEFT);
	}*/
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

void trackerheader_updatecoords(TrackerHeader* self)
{
	if (self->usebitmapskin) {
		self->coords = &self->gridstate->skin->headercoords;
	} else {
		self->coords = trackerheadercoords_default_classic();
	}
}

TrackerHeaderTrackState* trackerheader_trackstate(TrackerHeader* self,
	uintptr_t track)
{
	TrackerHeaderTrackState* state;

	state = (TrackerHeaderTrackState*)
		psy_table_at(&self->trackstates, track);
	if (!state) {
		state = (TrackerHeaderTrackState*)malloc(sizeof(
			TrackerHeaderTrackState));
		if (state) {
			state->playon = FALSE;
		}
	}
	return state;
}

TrackerHeaderTrackState* trackerheader_updatetrackstate(TrackerHeader* self,
	uintptr_t track)
{
	TrackerHeaderTrackState* state;

	state = trackerheader_trackstate(self, track);
	state->playon = psy_audio_activechannels_playon(
		&workspace_player(self->workspace)->playon, track);
	return state;
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
	if (self->usebitmapskin) {
		for (track = 0, cpx = 0; track < trackergridstate_numsongtracks(
			self->gridstate); ++track) {
			TrackDraw trackdraw;			
			TrackerHeaderTrackState* state;

			state = trackerheader_updatetrackstate(self, track);
			trackdraw_init(&trackdraw, self->gridstate, track, size.height,
				state->playon);
			trackdraw_draw(&trackdraw, g, cpx,
				psy_ui_realpoint_make(
					cpx + trackerheader_centerx(self, track), 0.0));
			cpx += trackergridstate_trackwidth(self->gridstate, track);
		}
	} else {
		for (track = 0, cpx = 0; track < trackergridstate_numsongtracks(
			self->gridstate); ++track) {			
			TrackPlainDraw trackdraw;
			TrackerHeaderTrackState* state;

			state = trackerheader_updatetrackstate(self, track);
			trackplaindraw_init(&trackdraw, self->gridstate,
				psy_ui_realpoint_make(
					trackergridstate_defaulttrackwidth(self->gridstate) /
					psy_ui_realrectangle_width(&self->coords->background.dest),
					size.height /
					psy_ui_realrectangle_height(&self->coords->background.dest)),
				track, size.height,
				state->playon);
			trackplaindraw_draw(&trackdraw, g, cpx,
				psy_ui_realpoint_make(cpx,  0.0));
			cpx += trackergridstate_trackwidth(self->gridstate, track);
		}
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
		track = trackergridstate_pxtotrack(self->gridstate, ev->pt.x);	
		if (!self->usebitmapskin) {
			double zoomx;

			zoomx = trackergridstate_defaulttrackwidth(self->gridstate) /
				psy_ui_realrectangle_width(&self->coords->background.dest);
			track_x = trackergridstate_tracktopx(self->gridstate, track);
			ev_track_x = (ev->pt.x - track_x) * 1.0 / zoomx;
		} else {
			track_x = trackerheader_trackposition(self, track);
			ev_track_x = ev->pt.x - track_x;
		}				
		if (trackerheader_hittest_solo(self, ev_track_x, ev->pt.y)) {
			if (psy_audio_patterns_istracksoloed(patterns, track)) {
				psy_audio_patterns_deactivatesolotrack(patterns);
			} else {
				psy_audio_patterns_activatesolotrack(patterns, track);
			}
			psy_ui_component_invalidate(&self->component);
		} else if (trackerheader_hittest_mute(self, ev_track_x, ev->pt.y)) {
			if (psy_audio_patterns_istrackmuted(patterns, track)) {
				psy_audio_patterns_unmutetrack(patterns, track);
			} else {
				psy_audio_patterns_mutetrack(patterns, track);
			}
			psy_ui_component_invalidate(&self->component);			
		} else if (trackerheader_hittest_record(self, ev_track_x, ev->pt.y)) {
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
	return skincoord_hittest(&self->coords->mute, x, y);
}

bool trackerheader_hittest_solo(const TrackerHeader* self, double x, double y)
{
	return skincoord_hittest(&self->coords->solo, x, y);
}

bool trackerheader_hittest_record(const TrackerHeader* self, double x, double y)
{
	return skincoord_hittest(&self->coords->record, x, y);
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
			self->coords->background.dest.right -
			self->coords->background.dest.left)) / 2);
}

void trackerheader_onpreferredsize(TrackerHeader* self,
	const psy_ui_Size* limit, psy_ui_Size* rv)
{
	rv->width = psy_ui_value_makepx(
		trackergridstate_tracktopx(self->gridstate,
			trackergridstate_numsongtracks(self->gridstate)));
	if (self->usebitmapskin) {
		rv->height = psy_ui_value_makepx(
			psy_ui_realrectangle_height(
				&self->coords->background.dest));
	} else {
		rv->height = psy_ui_value_makeeh(1.5);
	}
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
	TrackerHeaderTrackState* state;

	state = trackerheader_trackstate((TrackerHeader*)self, track);
	return (psy_audio_activechannels_playon(
		&workspace_player(self->workspace)->playon, track) !=
		state->playon);
}

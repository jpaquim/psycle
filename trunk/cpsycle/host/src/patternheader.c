/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "patternheader.h"
/* local */
#include "patterncmds.h"
#include "skingraphics.h"
/* platform */
#include "../../detail/portable.h"

/* prototypes */
static void patterntrackbox_ondraw(PatternTrackBox*, psy_ui_Graphics*);
static void patterntrackbox_drawbackground(PatternTrackBox*, psy_ui_Graphics*);
static void patterntrackbox_drawplayon(PatternTrackBox*, psy_ui_Graphics*);
static void patterntrackbox_drawnumber(PatternTrackBox*, psy_ui_Graphics*);
static void patterntrackbox_drawleds(PatternTrackBox*, psy_ui_Graphics*);
static void patterntrackbox_drawledmute(PatternTrackBox*, psy_ui_Graphics*);
static void patterntrackbox_drawledsoloed(PatternTrackBox*, psy_ui_Graphics*);
static void patterntrackbox_drawledarmed(PatternTrackBox*, psy_ui_Graphics*);
static void patterntrackbox_drawselection(PatternTrackBox*, psy_ui_Graphics*);
static void patterntrackbox_onmousedown(PatternTrackBox*, psy_ui_MouseEvent*);
static void patterntrackbox_onmousedown(PatternTrackBox*, psy_ui_MouseEvent*);
static void patterntrackbox_onpreferredsize(PatternTrackBox*,
	const psy_ui_Size* limit, psy_ui_Size* rv);
/* vtable */
static psy_ui_ComponentVtable patterntrackbox_vtable;
static bool patterntrackbox_vtable_initialized = FALSE;

static void patterntrackbox_vtable_init(PatternTrackBox* self)
{
	if (!patterntrackbox_vtable_initialized) {
		patterntrackbox_vtable = *(self->component.vtable);
		patterntrackbox_vtable.ondraw =
			(psy_ui_fp_component_ondraw)
			patterntrackbox_ondraw;
		patterntrackbox_vtable.onmousedown =
			(psy_ui_fp_component_onmouseevent)
			patterntrackbox_onmousedown;
		patterntrackbox_vtable.onpreferredsize =
			(psy_ui_fp_component_onpreferredsize)
			patterntrackbox_onpreferredsize;
		patterntrackbox_vtable_initialized = TRUE;
	}
	self->component.vtable = &patterntrackbox_vtable;
}

/* implementation */
void patterntrackbox_init(PatternTrackBox* self, psy_ui_Component* parent,
	uintptr_t index, TrackerState* state)
{
	psy_ui_component_init(&self->component, parent, NULL);
	patterntrackbox_vtable_init(self);
	self->state = state;
	self->index = index;
	self->playon = FALSE;
}

PatternTrackBox* patterntrackbox_alloc(void)
{
	return (PatternTrackBox*)malloc(sizeof(PatternTrackBox));
}

PatternTrackBox* patterntrackbox_allocinit(psy_ui_Component* parent,
	uintptr_t index, TrackerState* state)
{
	PatternTrackBox* rv;

	rv = patterntrackbox_alloc();
	if (rv) {
		patterntrackbox_init(rv, parent, index, state);
		psy_ui_component_deallocateafterdestroyed(patterntrackbox_base(rv));
	}
	return rv;
}

void patterntrackbox_playon(PatternTrackBox* self)
{
	if (!self->playon) {
		self->playon = TRUE;
		psy_ui_component_invalidate(&self->component);
	}
}

void patterntrackbox_playoff(PatternTrackBox* self)
{
	if (self->playon) {
		self->playon = FALSE;
		psy_ui_component_invalidate(&self->component);
	}
}

void patterntrackbox_ondraw(PatternTrackBox* self, psy_ui_Graphics* g)
{
	psy_ui_RealSize size;
	psy_ui_RealPoint origin;
	psy_ui_RealPoint restoreorigin;
	double centerx;

	size = psy_ui_component_size_px(&self->component);
	centerx = patternviewstate_skin(self->state->pv)->headercoords.background.dest.right -
		patternviewstate_skin(self->state->pv)->headercoords.background.dest.left;
	centerx = floor((size.width - centerx) / 2.0);
	restoreorigin = psy_ui_origin(g);
	origin = restoreorigin;
	origin.x -= centerx;
	psy_ui_setorigin(g, origin);	
	patterntrackbox_drawbackground(self, g);
	patterntrackbox_drawplayon(self, g);
	patterntrackbox_drawnumber(self, g);
	patterntrackbox_drawleds(self, g);
	patterntrackbox_drawselection(self, g);
	psy_ui_setorigin(g, restoreorigin);
}

void patterntrackbox_drawbackground(PatternTrackBox* self, psy_ui_Graphics* g)
{
	skin_blitcoord(g, &patternviewstate_skin(self->state->pv)->bitmap, psy_ui_realpoint_zero(),
		&patternviewstate_skin(self->state->pv)->headercoords.background);
}

void patterntrackbox_drawplayon(PatternTrackBox* self, psy_ui_Graphics* g)
{
	if (self->playon) {
	 	skin_blitcoord(g, &patternviewstate_skin(self->state->pv)->bitmap,
			psy_ui_realpoint_make(0, 0),
			&patternviewstate_skin(self->state->pv)->headercoords.dplayon);
	}
}

void patterntrackbox_drawnumber(PatternTrackBox* self, psy_ui_Graphics* g)
{
	intptr_t trackx0 = self->index / 10;
	intptr_t track0x = self->index % 10;
	SkinCoord digitx0 = patternviewstate_skin(self->state->pv)->headercoords.digitx0;
	SkinCoord digit0x = patternviewstate_skin(self->state->pv)->headercoords.digit0x;
	digitx0.src.left += trackx0 * psy_ui_realrectangle_width(&digitx0.src);
	digit0x.src.left += track0x * psy_ui_realrectangle_width(&digit0x.src);
	skin_blitcoord(g, &patternviewstate_skin(self->state->pv)->bitmap, psy_ui_realpoint_zero(), &digitx0);
	skin_blitcoord(g, &patternviewstate_skin(self->state->pv)->bitmap, psy_ui_realpoint_zero(), &digit0x);
}

void patterntrackbox_drawleds(PatternTrackBox* self, psy_ui_Graphics* g)
{
	if (patternviewstate_patterns(self->state->pv)) {
		patterntrackbox_drawledmute(self, g);
		patterntrackbox_drawledsoloed(self, g);
		patterntrackbox_drawledarmed(self, g);
	}
}

void patterntrackbox_drawledmute(PatternTrackBox* self, psy_ui_Graphics* g)
{
	if (psy_audio_patterns_istrackmuted(
		patternviewstate_patterns(self->state->pv), self->index)) {
		skin_blitcoord(g, &patternviewstate_skin(self->state->pv)->bitmap,
			psy_ui_realpoint_zero(),
			&patternviewstate_skin(self->state->pv)->headercoords.mute);
	}
}

void patterntrackbox_drawledsoloed(PatternTrackBox* self,
	psy_ui_Graphics* g)
{
	if (psy_audio_patterns_istracksoloed(
		patternviewstate_patterns(self->state->pv), self->index)) {
		skin_blitcoord(g, &patternviewstate_skin(self->state->pv)->bitmap,
			psy_ui_realpoint_zero(),
			&patternviewstate_skin(self->state->pv)->headercoords.solo);
	}
}

void patterntrackbox_drawledarmed(PatternTrackBox* self, psy_ui_Graphics* g)
{
	if (psy_audio_patterns_istrackarmed(
		patternviewstate_patterns(self->state->pv), self->index)) {
		skin_blitcoord(g, &patternviewstate_skin(self->state->pv)->bitmap,
			psy_ui_realpoint_zero(),
			&patternviewstate_skin(self->state->pv)->headercoords.record);
	}
}


void patterntrackbox_drawselection(PatternTrackBox* self, psy_ui_Graphics* g)
{
	if (self->index == self->state->pv->cursor.track) {
		psy_ui_setcolour(g, patternviewstate_skin(self->state->pv)->font);
		psy_ui_drawline(g, psy_ui_realpoint_zero(),
			psy_ui_realpoint_make(
				psy_min(
					patternviewstate_skin(self->state->pv)->headercoords.background.dest.right -
					patternviewstate_skin(self->state->pv)->headercoords.background.dest.left,
					trackerstate_trackwidth(self->state, self->index)),
				0.0));
	}
}

void patterntrackbox_onmousedown(PatternTrackBox* self,
	psy_ui_MouseEvent* ev)
{
	if (patternviewstate_patterns(self->state->pv)) {
		psy_audio_Patterns* patterns;
		psy_ui_RealSize size;		
		double centerx;
		psy_ui_RealPoint pt;
		bool repaint;

		size = psy_ui_component_size_px(&self->component);
		centerx = patternviewstate_skin(self->state->pv)->headercoords.background.dest.right -
			patternviewstate_skin(self->state->pv)->headercoords.background.dest.left;
		centerx = floor((size.width - centerx) / 2.0);
		pt = ev->pt;
		pt.x -= centerx;
		patterns = patternviewstate_patterns(self->state->pv);
		repaint = TRUE;
		if (skincoord_hittest(&patternviewstate_skin(self->state->pv)->headercoords.solo, pt.x, pt.y)) {
			if (psy_audio_patterns_istracksoloed(patterns, self->index)) {
				psy_audio_patterns_deactivatesolotrack(patterns);
			} else {
				psy_audio_patterns_activatesolotrack(patterns, self->index);
			}			
		} else if (skincoord_hittest(&patternviewstate_skin(self->state->pv)->headercoords.mute, pt.x, pt.y)) {
			if (psy_audio_patterns_istrackmuted(patterns, self->index)) {
				psy_audio_patterns_unmutetrack(patterns, self->index);
			} else {
				psy_audio_patterns_mutetrack(patterns, self->index);
			}			
		} else if (skincoord_hittest(&patternviewstate_skin(self->state->pv)->headercoords.record, pt.x, pt.y)) {
			if (psy_audio_patterns_istrackarmed(patterns, self->index)) {
				psy_audio_patterns_unarmtrack(patterns, self->index);
			} else {
				psy_audio_patterns_armtrack(patterns, self->index);
			}
			
		} else {
			psy_audio_SequenceCursor cursor;

			cursor = self->state->pv->cursor;
			cursor.track = self->index;
			patternviewstate_setcursor(self->state->pv, cursor);
			repaint = FALSE;
		}
		if (repaint) {
			psy_ui_component_invalidate(&self->component);
		}
	}
}

void patterntrackbox_onpreferredsize(PatternTrackBox* self,
	const psy_ui_Size* limit, psy_ui_Size* rv)
{
	if (self->state->trackconfig->resize.active &&
			self->state->trackconfig->resize.track == self->index) {
		rv->width = psy_ui_value_make_px(
			self->state->trackconfig->resize.size.width);
	} else {
		rv->width = psy_ui_value_make_px(trackerstate_trackwidth(
			self->state, self->index));
	}
	rv->height = psy_ui_value_make_px(
		patternviewstate_skin(self->state->pv)->headercoords.background.dest.bottom -
		patternviewstate_skin(self->state->pv)->headercoords.background.dest.top);
}

/* TrackerHeader */
/* prototypes */
static void trackerheader_ondestroy(TrackerHeader*);
static void trackerheader_onsongchanged(TrackerHeader*, Workspace* sender,
	int flag);
static void trackerheader_connectsong(TrackerHeader*);
static void trackerheader_oncursorchanged(TrackerHeader*, psy_audio_Sequence*);
static void trackerheader_ontimer(TrackerHeader*, uintptr_t timerid);
static void trackerheader_updateplayons(TrackerHeader*);
static void trackerheader_onmousewheel(TrackerHeader*, psy_ui_MouseEvent*);
static void trackerheader_ontrackstatechanged(TrackerHeader*,
	psy_audio_TrackState* sender);
static void trackerheader_onplaylinechanged(TrackerHeader*, Workspace* sender);

/* vtable */
static psy_ui_ComponentVtable trackerheader_vtable;
static bool trackerheader_vtable_initialized = FALSE;

static void trackerheader_vtable_init(TrackerHeader* self)
{
	if (!trackerheader_vtable_initialized) {
		trackerheader_vtable = *(self->component.vtable);
		trackerheader_vtable.ondestroy =
			(psy_ui_fp_component_event)
			trackerheader_ondestroy;
		trackerheader_vtable.ontimer =
			(psy_ui_fp_component_ontimer)
			trackerheader_ontimer;
		trackerheader_vtable.onmousewheel =
			(psy_ui_fp_component_onmouseevent)
			trackerheader_onmousewheel;

		trackerheader_vtable_initialized = TRUE;
	}
	self->component.vtable = &trackerheader_vtable;
}

/* implementation */
void trackerheader_init(TrackerHeader* self, psy_ui_Component* parent,
	TrackConfig* trackconfig, TrackerState* state,
	Workspace* workspace)
{
	psy_ui_component_init(&self->component, parent, NULL);
	psy_ui_component_setbackgroundmode(&self->component, psy_ui_NOBACKGROUND);
	trackerheader_vtable_init(self);
	psy_ui_component_setalignexpand(&self->component,
		psy_ui_HEXPAND);	
	self->state = state;	
	self->workspace = workspace;
	self->currtrack = 0;
	psy_table_init(&self->boxes);	
	psy_signal_connect(&workspace->signal_songchanged, self,
		trackerheader_onsongchanged);
	trackerheader_connectsong(self);	
	trackerheader_build(self);	
	psy_ui_component_starttimer(&self->component, 0, 50);
	psy_signal_connect(&self->workspace->signal_playlinechanged, self,
		trackerheader_onplaylinechanged);
}

void trackerheader_ondestroy(TrackerHeader* self)
{
	psy_table_dispose(&self->boxes);
}

void trackerheader_build(TrackerHeader* self)
{
	uintptr_t track;

	psy_ui_component_clear(&self->component);
	psy_table_clear(&self->boxes);
	for (track = 0; track < patternviewstate_numsongtracks(self->state->pv); ++track) {
		PatternTrackBox* trackbox;

		trackbox = patterntrackbox_allocinit(&self->component, track,
			self->state);
		psy_ui_component_setalign(patterntrackbox_base(trackbox),
			psy_ui_ALIGN_LEFT);
		psy_table_insert(&self->boxes, track, (void*)trackbox);
	}
	psy_ui_component_align(&self->component);
}

void trackerheader_onsongchanged(TrackerHeader* self, Workspace* workspace,
	int flag)
{
	trackerheader_connectsong(self);
}

void trackerheader_connectsong(TrackerHeader* self)
{
	if (workspace_song(self->workspace)) {
		psy_signal_connect(&psy_audio_song_sequence(workspace_song(
			self->workspace))->signal_cursorchanged,
			self, trackerheader_oncursorchanged);
		psy_signal_connect(&psy_audio_song_patterns(workspace_song(
			self->workspace))->trackstate.signal_changed,
			self, trackerheader_ontrackstatechanged);
	}
}

void trackerheader_oncursorchanged(TrackerHeader* self,
	psy_audio_Sequence* sender)
{	
	psy_audio_SequenceCursor cursor;

	cursor = sender->cursor;		
	if (self->currtrack != cursor.track) {
		self->currtrack = cursor.track;
		psy_ui_component_invalidate(&self->component);	
	}		
}

void trackerheader_ontimer(TrackerHeader* self, uintptr_t timerid)
{	
	trackerheader_updateplayons(self);
}

void trackerheader_updateplayons(TrackerHeader* self)
{
	psy_TableIterator it;

	assert(self);

	for (it = psy_table_begin(&self->boxes);
			!psy_tableiterator_equal(&it, psy_table_end());
			psy_tableiterator_inc(&it)) {
		PatternTrackBox* trackbox;

		trackbox = (PatternTrackBox*)psy_tableiterator_value(&it);
		if ((psy_audio_activechannels_playon(&self->workspace->player.playon,
				psy_tableiterator_key(&it)))) {
			patterntrackbox_playon(trackbox);
		} else {
			patterntrackbox_playoff(trackbox);
		}
	}
}

void trackerheader_onmousewheel(TrackerHeader* self, psy_ui_MouseEvent* ev)
{
	if (ev->delta > 0) {
		psy_audio_player_sendcmd(workspace_player(self->workspace),
			"tracker", psy_eventdrivercmd_makeid(CMD_COLUMNNEXT));		
	} else {		
		psy_audio_player_sendcmd(workspace_player(self->workspace),
			"tracker", psy_eventdrivercmd_makeid(CMD_COLUMNPREV));		
	}
}

void trackerheader_ontrackstatechanged(TrackerHeader* self,
	psy_audio_TrackState* sender)
{
	psy_ui_component_invalidate(&self->component);
}

void trackerheader_onplaylinechanged(TrackerHeader* self, Workspace* sender)
{	
	psy_ui_component_invalidate(&self->component);
}

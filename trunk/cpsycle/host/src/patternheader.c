/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "patternheader.h"
/* local */
#include "patterncmds.h"
#include "skingraphics.h"


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
static void patterntrackbox_blit(PatternTrackBox*, psy_ui_Graphics*,
	const SkinCoord*);
static void patterntrackbox_onmousedown(PatternTrackBox*, psy_ui_MouseEvent*);
static void patterntrackbox_onmousedown(PatternTrackBox*, psy_ui_MouseEvent*);
static psy_ui_RealPoint patterntrackbox_center(const PatternTrackBox*);
static double patterntrackbox_skinwidth(const PatternTrackBox*);
static const TrackerHeaderCoords* patterntrackbox_coords(
	const PatternTrackBox*);
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
	psy_ui_RealPoint origin;	
	
	origin = psy_ui_origin(g);	
	origin.x -= patterntrackbox_center(self).x;
	psy_ui_setorigin(g, origin);	
	patterntrackbox_drawbackground(self, g);
	patterntrackbox_drawplayon(self, g);
	patterntrackbox_drawnumber(self, g);
	patterntrackbox_drawleds(self, g);
	patterntrackbox_drawselection(self, g);	
}

void patterntrackbox_drawbackground(PatternTrackBox* self, psy_ui_Graphics* g)
{
	patterntrackbox_blit(self, g, &patterntrackbox_coords(self)->background);
}

void patterntrackbox_drawplayon(PatternTrackBox* self, psy_ui_Graphics* g)
{
	if (self->playon) {
		patterntrackbox_blit(self, g, &patterntrackbox_coords(self)->dplayon);	 	
	}
}

void patterntrackbox_drawnumber(PatternTrackBox* self, psy_ui_Graphics* g)
{	
	SkinCoord digitx0;
	SkinCoord digit0x;

	digitx0 = patterntrackbox_coords(self)->digitx0;
	digit0x = patterntrackbox_coords(self)->digit0x;
	digitx0.src.left += (self->index / 10) * psy_ui_realrectangle_width(&digitx0.src);
	digit0x.src.left += (self->index % 10) * psy_ui_realrectangle_width(&digit0x.src);
	patterntrackbox_blit(self, g, &digitx0);
	patterntrackbox_blit(self, g, &digit0x);	
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
		patterntrackbox_blit(self, g, &patterntrackbox_coords(self)->mute);
	}
}

void patterntrackbox_drawledsoloed(PatternTrackBox* self,
	psy_ui_Graphics* g)
{
	if (psy_audio_patterns_istracksoloed(
		patternviewstate_patterns(self->state->pv), self->index)) {
		patterntrackbox_blit(self, g, &patterntrackbox_coords(self)->solo);		
	}
}

void patterntrackbox_drawledarmed(PatternTrackBox* self, psy_ui_Graphics* g)
{
	if (psy_audio_patterns_istrackarmed(
			patternviewstate_patterns(self->state->pv), self->index)) {
		patterntrackbox_blit(self, g, &patterntrackbox_coords(self)->record);
	}
}

void patterntrackbox_drawselection(PatternTrackBox* self, psy_ui_Graphics* g)
{
	if (self->index == psy_audio_sequencecursor_track(
			&self->state->pv->cursor)) {
		psy_ui_setcolour(g, patternviewstate_skin(self->state->pv)->font);
		psy_ui_drawline(g, psy_ui_realpoint_zero(),
			psy_ui_realpoint_make(psy_min(patterntrackbox_skinwidth(self),
				trackerstate_trackwidth(self->state, self->index)), 0.0));
	}
}

void patterntrackbox_blit(PatternTrackBox* self, psy_ui_Graphics* g,
	const SkinCoord* coord)
{
	skin_blitcoord(g, &patternviewstate_skin(self->state->pv)->bitmap,
		psy_ui_realpoint_zero(), coord);
}

void patterntrackbox_onmousedown(PatternTrackBox* self, psy_ui_MouseEvent* ev)
{
	if (patternviewstate_patterns(self->state->pv)) {
		psy_audio_Patterns* patterns;		
		psy_ui_RealPoint pt;		
		
		pt = ev->pt;
		pt.x -= patterntrackbox_center(self).x;
		patterns = patternviewstate_patterns(self->state->pv);		
		if (skincoord_hittest(&patterntrackbox_coords(self)->solo, pt)) {
			if (psy_audio_patterns_istracksoloed(patterns, self->index)) {
				psy_audio_patterns_deactivatesolotrack(patterns);
			} else {
				psy_audio_patterns_activatesolotrack(patterns, self->index);
			}			
		} else if (skincoord_hittest(&patterntrackbox_coords(self)->mute, pt)) {
			if (psy_audio_patterns_istrackmuted(patterns, self->index)) {
				psy_audio_patterns_unmutetrack(patterns, self->index);
			} else {
				psy_audio_patterns_mutetrack(patterns, self->index);
			}			
		} else if (skincoord_hittest(&patterntrackbox_coords(self)->record, pt)) {
			if (psy_audio_patterns_istrackarmed(patterns, self->index)) {
				psy_audio_patterns_unarmtrack(patterns, self->index);
			} else {
				psy_audio_patterns_armtrack(patterns, self->index);
			}			
		}
	}
}

psy_ui_RealPoint patterntrackbox_center(const PatternTrackBox* self)
{
	psy_ui_RealSize size;

	size = psy_ui_component_size_px(&self->component);
	return psy_ui_realpoint_make(
		floor((size.width - patterntrackbox_skinwidth(self)) / 2.0), 0.0);
}

double patterntrackbox_skinwidth(const PatternTrackBox* self)
{
	return psy_ui_realrectangle_width(
		&patterntrackbox_coords(self)->background.dest);
}

const TrackerHeaderCoords* patterntrackbox_coords(const PatternTrackBox* self)
{
	return &patternviewstate_skin(self->state->pv)->headercoords;
}

void patterntrackbox_onpreferredsize(PatternTrackBox* self,
	const psy_ui_Size* limit, psy_ui_Size* rv)
{	
	psy_ui_size_init_px(rv, trackerstate_trackwidth(self->state, self->index),
		psy_ui_realrectangle_height(
			&patterntrackbox_coords(self)->background.dest));
}

/* TrackerHeader */

/* prototypes */
static void trackerheader_ondestroy(TrackerHeader*);
static void trackerheader_onsongchanged(TrackerHeader*, Workspace* sender);
static void trackerheader_connectsong(TrackerHeader*);
static void trackerheader_oncursorchanged(TrackerHeader*, psy_audio_Sequence*);
static void trackerheader_ontimer(TrackerHeader*, uintptr_t timerid);
static void trackerheader_updateplayons(TrackerHeader*);
static void trackerheader_onmousewheel(TrackerHeader*, psy_ui_MouseEvent*);
static void trackerheader_ontrackstatechanged(TrackerHeader*,
	psy_audio_TrackState* sender);

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
	psy_ui_component_setvtable(&self->component, &trackerheader_vtable);	
}

/* implementation */
void trackerheader_init(TrackerHeader* self, psy_ui_Component* parent,
	TrackConfig* trackconfig, TrackerState* state, Workspace* workspace)
{
	psy_ui_component_init(&self->component, parent, NULL);
	trackerheader_vtable_init(self);
	psy_ui_component_setbackgroundmode(&self->component, psy_ui_NOBACKGROUND);
	psy_ui_component_setalignexpand(&self->component, psy_ui_HEXPAND);
	self->state = state;	
	self->workspace = workspace;	
	psy_table_init(&self->boxes);	
	psy_signal_connect(&workspace->signal_songchanged, self,
		trackerheader_onsongchanged);
	trackerheader_connectsong(self);	
	trackerheader_build(self);	
	psy_ui_component_starttimer(&self->component, 0, 50);
}

void trackerheader_ondestroy(TrackerHeader* self)
{
	psy_table_dispose(&self->boxes);
}

void trackerheader_build(TrackerHeader* self)
{
	uintptr_t track;
	uintptr_t numtracks;

	psy_ui_component_clear(&self->component);
	psy_table_clear(&self->boxes);
	numtracks = patternviewstate_numsongtracks(self->state->pv);
	for (track = 0; track < numtracks; ++track) {		
		psy_table_insert(&self->boxes, track,
			psy_ui_component_setalign(
				patterntrackbox_base(patterntrackbox_allocinit(
					trackerheader_base(self), track, self->state)),
				psy_ui_ALIGN_LEFT));
	}
	psy_ui_component_align(trackerheader_base(self));
}

void trackerheader_onsongchanged(TrackerHeader* self, Workspace* sender)
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
	if (psy_audio_sequencecursor_track(&sender->lastcursor) !=
			psy_audio_sequencecursor_track(&sender->cursor)) {		
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
	psy_audio_player_sendcmd(workspace_player(self->workspace), "tracker",
		psy_eventdrivercmd_makeid(
			(ev->delta > 0)
			? CMD_COLUMNNEXT
			: CMD_COLUMNPREV));
}

void trackerheader_ontrackstatechanged(TrackerHeader* self,
	psy_audio_TrackState* sender)
{
	psy_ui_component_invalidate(&self->component);
}

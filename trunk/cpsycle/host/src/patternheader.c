/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "patternheader.h"
/* local */
#include "styles.h"
/* portable */
#include "../../detail/portable.h"

/* prototypes */
static void patterntrackbox_ondraw(PatternTrackBox*, psy_ui_Graphics*);
static void patterntrackbox_drawnumber(PatternTrackBox*, psy_ui_Graphics*);
static void patterntrackbox_drawselection(PatternTrackBox*, psy_ui_Graphics*);
static void patterntrackbox_drawtext(PatternTrackBox*, psy_ui_Graphics*);
static void patterntrackbox_on_mouse_down(PatternTrackBox*, psy_ui_MouseEvent*);
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
		patterntrackbox_vtable.on_mouse_down =
			(psy_ui_fp_component_on_mouse_event)
			patterntrackbox_on_mouse_down;		
		patterntrackbox_vtable.onpreferredsize =
			(psy_ui_fp_component_onpreferredsize)
			patterntrackbox_onpreferredsize;
		patterntrackbox_vtable_initialized = TRUE;
	}
	psy_ui_component_set_vtable(&self->component, &patterntrackbox_vtable);
}

/* implementation */
void patterntrackbox_init(PatternTrackBox* self, psy_ui_Component* parent,
	uintptr_t index, TrackerState* state)
{
	psy_ui_component_init(&self->component, parent, NULL);
	patterntrackbox_vtable_init(self);
	psy_ui_component_set_style_type(&self->component,
		STYLE_PV_TRACK_HEADER);
	self->state = state;
	self->index = index;	
	psy_ui_component_init(&self->solo, &self->component, NULL);
	psy_ui_component_set_style_type(&self->solo,
		STYLE_PV_TRACK_HEADER_SOLO);
	psy_ui_component_set_style_type_select(&self->solo,
		STYLE_PV_TRACK_HEADER_SOLO_SELECT);
	psy_ui_component_init(&self->mute, &self->component, NULL);
	psy_ui_component_set_style_type(&self->mute,
		STYLE_PV_TRACK_HEADER_MUTE);
	psy_ui_component_set_style_type_select(&self->mute,
		STYLE_PV_TRACK_HEADER_MUTE_SELECT);
	psy_ui_component_init(&self->record, &self->component, NULL);
	psy_ui_component_set_style_type(&self->record,
		STYLE_PV_TRACK_HEADER_RECORD);
	psy_ui_component_set_style_type_select(&self->record,
		STYLE_PV_TRACK_HEADER_RECORD_SELECT);
	psy_ui_component_init(&self->play, &self->component, NULL);
	psy_ui_component_set_style_type(&self->play,
		STYLE_PV_TRACK_HEADER_PLAY);
	psy_ui_component_set_style_type_select(&self->play,
		STYLE_PV_TRACK_HEADER_PLAY_SELECT);
	patterntrackbox_update(self);
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
		psy_ui_component_deallocate_after_destroyed(patterntrackbox_base(rv));
	}
	return rv;
}

void patterntrackbox_playon(PatternTrackBox* self)
{
	psy_ui_component_addstylestate(&self->play, psy_ui_STYLESTATE_SELECT);
}

void patterntrackbox_playoff(PatternTrackBox* self)
{
	psy_ui_component_removestylestate(&self->play, psy_ui_STYLESTATE_SELECT);
}

void patterntrackbox_ondraw(PatternTrackBox* self, psy_ui_Graphics* g)
{
	patterntrackbox_drawnumber(self, g);
	patterntrackbox_drawselection(self, g);
	patterntrackbox_drawtext(self, g);
}

void patterntrackbox_drawnumber(PatternTrackBox* self, psy_ui_Graphics* g)
{
	psy_ui_Style* style_x0;
	psy_ui_Style* style_0x;
	psy_ui_RealRectangle r;
	const psy_ui_TextMetric* tm;
	double src_x0;
	double src_0x;

	tm = psy_ui_component_textmetric(&self->component);
	style_x0 = psy_ui_style(STYLE_PV_TRACK_HEADER_DIGITX0);
	style_0x = psy_ui_style(STYLE_PV_TRACK_HEADER_DIGIT0X);
	src_x0 = (self->index / 10) * style_x0->background.size.width;
	src_0x = (self->index % 10) * style_0x->background.size.width;
	r = psy_ui_realrectangle_make(
		psy_ui_realpoint_make(
			psy_ui_value_px(&style_x0->padding.left, tm, 0),
			psy_ui_value_px(&style_x0->padding.top, tm, 0)),
		style_x0->background.size);
	psy_ui_drawbitmap(g, &style_x0->background.bitmap, r,
		psy_ui_realpoint_make(
			-style_x0->background.position.x + src_x0,
			-style_x0->background.position.y));
	r = psy_ui_realrectangle_make(
		psy_ui_realpoint_make(
			psy_ui_value_px(&style_0x->padding.left, tm, 0),
			psy_ui_value_px(&style_0x->padding.top, tm, 0)),
		style_0x->background.size);
	psy_ui_drawbitmap(g, &style_0x->background.bitmap, r,
		psy_ui_realpoint_make(
			-style_0x->background.position.x + src_0x,
			-style_0x->background.position.y));
}

void patterntrackbox_drawselection(PatternTrackBox* self, psy_ui_Graphics* g)
{
	if (self->index == psy_audio_sequencecursor_track(
		&self->state->pv->cursor)) {
		psy_ui_RealSize size;

		size = psy_ui_component_size_px(&self->component);
		psy_ui_drawline(g, psy_ui_realpoint_zero(),
			psy_ui_realpoint_make(size.width, 0.0));
	}
}

void patterntrackbox_drawtext(PatternTrackBox* self, psy_ui_Graphics* g)
{
	psy_ui_Style* style_text;

	style_text = psy_ui_style(STYLE_PV_TRACK_HEADER_TEXT);
	if (psy_ui_position_is_active(&style_text->position)) {
		psy_ui_Rectangle r;
		const psy_ui_TextMetric* tm;
		static const char* text = "";

		r = psy_ui_style_position(style_text);		
		tm = psy_ui_component_textmetric(&self->component);		
		psy_ui_setfont(g, &style_text->font);
		psy_ui_textoutrectangle(g,
			psy_ui_realpoint_make(
				psy_ui_value_px(&r.topleft.x, tm, NULL),
				psy_ui_value_px(&r.topleft.y, tm, NULL)),
			psy_ui_ETO_CLIPPED,
			psy_ui_realrectangle_make(
				psy_ui_realpoint_make(
					psy_ui_value_px(&r.topleft.x, tm, NULL),
					psy_ui_value_px(&r.topleft.y, tm, NULL)),
				psy_ui_realsize_make(
					psy_ui_value_px(&r.size.width, tm, NULL),
					psy_ui_value_px(&r.size.height, tm, NULL))),
			text,
			psy_strlen(text));		
	}
}

void patterntrackbox_on_mouse_down(PatternTrackBox* self, psy_ui_MouseEvent* ev)
{
	if (patternviewstate_patterns(self->state->pv)) {
		psy_audio_Patterns* patterns;		
		psy_ui_RealPoint pt;		
		
		pt = psy_ui_mouseevent_pt(ev);		
		patterns = patternviewstate_patterns(self->state->pv);		
		if (psy_ui_mouseevent_target(ev) == &self->solo) {
			if (psy_audio_patterns_istracksoloed(patterns, self->index)) {
				psy_audio_patterns_deactivatesolotrack(patterns);
			} else {
				psy_audio_patterns_activatesolotrack(patterns, self->index);
			}			
		} else if (psy_ui_mouseevent_target(ev) == &self->mute) {
			if (psy_audio_patterns_istrackmuted(patterns, self->index)) {
				psy_audio_patterns_unmutetrack(patterns, self->index);
			} else {
				psy_audio_patterns_mutetrack(patterns, self->index);
			}			
		} else if (psy_ui_mouseevent_target(ev) == &self->record) {
			if (psy_audio_patterns_istrackarmed(patterns, self->index)) {
				psy_audio_patterns_unarmtrack(patterns, self->index);
			} else {
				psy_audio_patterns_armtrack(patterns, self->index);
			}			
		}
	}
}

void patterntrackbox_onpreferredsize(PatternTrackBox* self,
	const psy_ui_Size* limit, psy_ui_Size* rv)
{	
	psy_ui_Style* style;

	style = psy_ui_style(STYLE_PV_TRACK_HEADER);
	psy_ui_size_init_px(rv,
		style->background.size.width,
		style->background.size.height);
}

void patterntrackbox_update(PatternTrackBox* self)
{
	psy_audio_Patterns* patterns;
	
	patterns = patternviewstate_patterns(self->state->pv);
	if (psy_audio_patterns_istracksoloed(patterns, self->index)) {
		psy_ui_component_addstylestate(&self->solo,
			psy_ui_STYLESTATE_SELECT);
	} else {
		psy_ui_component_removestylestate(&self->solo,
			psy_ui_STYLESTATE_SELECT);
	}
	if (psy_audio_patterns_istrackmuted(patterns, self->index)) {
		psy_ui_component_addstylestate(&self->mute,
			psy_ui_STYLESTATE_SELECT);
	} else {
		psy_ui_component_removestylestate(&self->mute,
			psy_ui_STYLESTATE_SELECT);
	}
	if (psy_audio_patterns_istrackarmed(patterns, self->index)) {
		psy_ui_component_addstylestate(&self->record,
			psy_ui_STYLESTATE_SELECT);
	} else {
		psy_ui_component_removestylestate(&self->record,
			psy_ui_STYLESTATE_SELECT);
	}
}

/* PatternTrack */

/* prototypes */
static void patterntrack_onpreferredsize(PatternTrack*,
	const psy_ui_Size* limit, psy_ui_Size* rv);

/* vtable */
static psy_ui_ComponentVtable patterntrack_vtable;
static bool patterntrack_vtable_initialized = FALSE;

static void patterntrack_vtable_init(PatternTrack* self)
{
	if (!patterntrack_vtable_initialized) {
		patterntrack_vtable = *(self->component.vtable);		
		patterntrack_vtable.onpreferredsize =
			(psy_ui_fp_component_onpreferredsize)
			patterntrack_onpreferredsize;
		patterntrack_vtable_initialized = TRUE;
	}
	self->component.vtable = &patterntrack_vtable;
}

/* implementation */
void patterntrack_init(PatternTrack* self, psy_ui_Component* parent,
	uintptr_t index, TrackerState* state)
{
	psy_ui_component_init(&self->component, parent, NULL);
	patterntrack_vtable_init(self);
	patterntrackbox_init(&self->trackbox, &self->component, index, state);
	psy_ui_component_set_align(&self->trackbox.component, psy_ui_ALIGN_CENTER);
}

PatternTrack* patterntrack_alloc(void)
{
	return (PatternTrack*)malloc(sizeof(PatternTrack));
}

PatternTrack* patterntrack_allocinit(psy_ui_Component* parent,
	uintptr_t index, TrackerState* state)
{
	PatternTrack* rv;

	rv = patterntrack_alloc();
	if (rv) {
		patterntrack_init(rv, parent, index, state);
		psy_ui_component_deallocate_after_destroyed(patterntrack_base(rv));
	}
	return rv;
}

void patterntrack_onpreferredsize(PatternTrack* self,
	const psy_ui_Size* limit, psy_ui_Size* rv)
{
	psy_ui_Style* style;

	style = psy_ui_style(STYLE_PV_TRACK_HEADER);
		psy_ui_size_init_px(rv,
			trackerstate_trackwidth(self->trackbox.state,
				self->trackbox.index),
			style->background.size.height);
}

/* TrackerHeader */

/* prototypes */
static void trackerheader_on_destroy(TrackerHeader*);
static void trackerheader_onsongchanged(TrackerHeader*, Workspace* sender);
static void trackerheader_connectsong(TrackerHeader*);
static void trackerheader_oncursorchanged(TrackerHeader*, psy_audio_Sequence*);
static void trackerheader_on_timer(TrackerHeader*, uintptr_t timerid);
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
		trackerheader_vtable.on_destroy =
			(psy_ui_fp_component_event)
			trackerheader_on_destroy;
		trackerheader_vtable.on_timer =
			(psy_ui_fp_component_on_timer)
			trackerheader_on_timer;
		trackerheader_vtable.onmousewheel =
			(psy_ui_fp_component_on_mouse_event)
			trackerheader_onmousewheel;
		trackerheader_vtable_initialized = TRUE;
	}
	psy_ui_component_set_vtable(&self->component, &trackerheader_vtable);	
}

/* implementation */
void trackerheader_init(TrackerHeader* self, psy_ui_Component* parent,
	TrackConfig* trackconfig, TrackerState* state, Workspace* workspace)
{
	psy_ui_component_init(&self->component, parent, NULL);
	trackerheader_vtable_init(self);	
	psy_ui_component_setalignexpand(&self->component, psy_ui_HEXPAND);
	self->state = state;	
	self->workspace = workspace;	
	psy_table_init(&self->boxes);	
	psy_signal_connect(&workspace->signal_songchanged, self,
		trackerheader_onsongchanged);
	trackerheader_connectsong(self);	
	trackerheader_build(self);	
	psy_ui_component_start_timer(&self->component, 0, 50);
}

void trackerheader_on_destroy(TrackerHeader* self)
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
			psy_ui_component_set_align(
				patterntrack_base(patterntrack_allocinit(
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

void trackerheader_on_timer(TrackerHeader* self, uintptr_t timerid)
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
		PatternTrack* track;

		track = (PatternTrack*)psy_tableiterator_value(&it);
		if ((psy_audio_activechannels_playon(&self->workspace->player.playon,
				psy_tableiterator_key(&it)))) {
			patterntrackbox_playon(&track->trackbox);
		} else {
			patterntrackbox_playoff(&track->trackbox);
		}
	}
}

void trackerheader_onmousewheel(TrackerHeader* self, psy_ui_MouseEvent* ev)
{	
	psy_audio_player_sendcmd(workspace_player(self->workspace), "tracker",
		psy_eventdrivercmd_makeid(
			(psy_ui_mouseevent_delta(ev) > 0)
			? CMD_COLUMNNEXT
			: CMD_COLUMNPREV));
}

void trackerheader_ontrackstatechanged(TrackerHeader* self,
	psy_audio_TrackState* sender)
{
	psy_TableIterator it;

	assert(self);

	for (it = psy_table_begin(&self->boxes);
		!psy_tableiterator_equal(&it, psy_table_end());
		psy_tableiterator_inc(&it)) {
		PatternTrack* track;

		track = (PatternTrack*)psy_tableiterator_value(&it);		
		patterntrackbox_update(&track->trackbox);		
	}
	psy_ui_component_invalidate(&self->component);
}

/* TrackerHeaderView */

/* prototypes*/
void trackerheaderview_on_configure(TrackerHeaderView*, PatternViewConfig*,
	psy_Property*);
void trackerheaderview_on_header_line(TrackerHeaderView*, psy_ui_Component* sender);

/* implementation */
void trackerheaderview_init(TrackerHeaderView* self, psy_ui_Component* parent, TrackConfig* config,
	TrackerState* state, Workspace* workspace)
{
	psy_ui_component_init(&self->component, parent, NULL);
	/* desc */
	psy_ui_button_init_connect(&self->desc, &self->component,
		self, trackerheaderview_on_header_line);
	psy_ui_component_set_style_types(&self->desc.component,
		psy_ui_STYLE_LABEL, psy_INDEX_INVALID, psy_INDEX_INVALID,
		psy_INDEX_INVALID);
	psy_ui_button_set_text(&self->desc, "Line");
	psy_ui_component_set_align(&self->desc.component, psy_ui_ALIGN_LEFT);
	psy_ui_component_set_preferred_size(&self->desc.component,
		psy_ui_size_make_em(8.0, 1.0));
	/* tracks */
	psy_ui_component_init(&self->pane, &self->component, NULL);
	psy_ui_component_set_align(&self->pane, psy_ui_ALIGN_CLIENT);
	trackerheader_init(&self->header, &self->pane, config, state, workspace);
	psy_ui_component_set_align(&self->header.component, psy_ui_ALIGN_FIXED);
	/* configuration */
	psy_signal_connect(&workspace->config.patview.signal_changed, self,
		trackerheaderview_on_configure);
}

void trackerheaderview_on_configure(TrackerHeaderView* self, PatternViewConfig* config,
	psy_Property* property)
{	
	if (patternviewconfig_linenumber_width(config) == 0.0) {
		psy_ui_component_hide(&self->desc.component);
	} else {
		psy_ui_component_set_preferred_size(&self->desc.component,
			psy_ui_size_make_em(
				patternviewconfig_linenumber_width(config) * self->header.state->flatsize,
				1.0));
		psy_ui_component_show(&self->desc.component);
	}	
}

void trackerheaderview_on_header_line(TrackerHeaderView* self, psy_ui_Component* sender)
{
	patternviewconfig_switch_header(&self->header.workspace->config.patview);
	psy_ui_component_align(&self->header.component);
	psy_ui_component_invalidate(&self->header.component);
}

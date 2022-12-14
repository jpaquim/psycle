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
static void patterntrackbox_on_draw(PatternTrackBox*, psy_ui_Graphics*);
static void patterntrackbox_draw_number(PatternTrackBox*, psy_ui_Graphics*);
static void patterntrackbox_draw_selection(PatternTrackBox*, psy_ui_Graphics*);
static void patterntrackbox_draw_text(PatternTrackBox*, psy_ui_Graphics*);
static void patterntrackbox_on_mouse_down(PatternTrackBox*, psy_ui_MouseEvent*);
static void patterntrackbox_on_preferred_size(PatternTrackBox*,
	const psy_ui_Size* limit, psy_ui_Size* rv);
static void patterndefaultline_update_preferred_size(TrackerHeaderView*);

/* vtable */
static psy_ui_ComponentVtable patterntrackbox_vtable;
static bool patterntrackbox_vtable_initialized = FALSE;

static void patterntrackbox_vtable_init(PatternTrackBox* self)
{
	if (!patterntrackbox_vtable_initialized) {
		patterntrackbox_vtable = *(self->component.vtable);
		patterntrackbox_vtable.ondraw =
			(psy_ui_fp_component_ondraw)
			patterntrackbox_on_draw;
		patterntrackbox_vtable.on_mouse_down =
			(psy_ui_fp_component_on_mouse_event)
			patterntrackbox_on_mouse_down;		
		patterntrackbox_vtable.onpreferredsize =
			(psy_ui_fp_component_on_preferred_size)
			patterntrackbox_on_preferred_size;
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

void patterntrackbox_play_on(PatternTrackBox* self)
{
	psy_ui_component_add_style_state(&self->play, psy_ui_STYLESTATE_SELECT);
}

void patterntrackbox_play_off(PatternTrackBox* self)
{
	psy_ui_component_remove_style_state(&self->play, psy_ui_STYLESTATE_SELECT);
}

void patterntrackbox_on_draw(PatternTrackBox* self, psy_ui_Graphics* g)
{
	patterntrackbox_draw_number(self, g);
	patterntrackbox_draw_selection(self, g);
	patterntrackbox_draw_text(self, g);
}

void patterntrackbox_draw_number(PatternTrackBox* self, psy_ui_Graphics* g)
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

void patterntrackbox_draw_selection(PatternTrackBox* self, psy_ui_Graphics* g)
{
	if (self->index == psy_audio_sequencecursor_track(
		&self->state->pv->cursor)) {
		psy_ui_RealSize size;

		size = psy_ui_component_size_px(&self->component);
		psy_ui_drawline(g, psy_ui_realpoint_zero(),
			psy_ui_realpoint_make(size.width, 0.0));
	}
}

void patterntrackbox_draw_text(PatternTrackBox* self, psy_ui_Graphics* g)
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
		} else if (self->index != psy_audio_sequencecursor_track(
				patternviewstate_cursor(self->state->pv))) {
			psy_audio_SequenceCursor cursor;
			
			cursor = *patternviewstate_cursor(self->state->pv);
			cursor.track = self->index;
			psy_audio_sequence_set_cursor(self->state->pv->sequence, cursor);
		}
	}
}

void patterntrackbox_on_preferred_size(PatternTrackBox* self,
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
		psy_ui_component_add_style_state(&self->solo,
			psy_ui_STYLESTATE_SELECT);
	} else {
		psy_ui_component_remove_style_state(&self->solo,
			psy_ui_STYLESTATE_SELECT);
	}
	if (psy_audio_patterns_istrackmuted(patterns, self->index)) {
		psy_ui_component_add_style_state(&self->mute,
			psy_ui_STYLESTATE_SELECT);
	} else {
		psy_ui_component_remove_style_state(&self->mute,
			psy_ui_STYLESTATE_SELECT);
	}
	if (psy_audio_patterns_istrackarmed(patterns, self->index)) {
		psy_ui_component_add_style_state(&self->record,
			psy_ui_STYLESTATE_SELECT);
	} else {
		psy_ui_component_remove_style_state(&self->record,
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
			(psy_ui_fp_component_on_preferred_size)
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
			trackerstate_trackwidth(self->trackbox.state, self->trackbox.index,
				psy_ui_component_textmetric(&self->component)),
			style->background.size.height);
}

/* TrackerHeader */

/* prototypes */
static void trackerheader_on_destroyed(TrackerHeader*);
static void trackerheader_on_song_changed(TrackerHeader*,
	psy_audio_Player* sender);
static void trackerheader_connect_song(TrackerHeader*);
static void trackerheader_on_cursor_changed(TrackerHeader*,
	psy_audio_Sequence*);
static void trackerheader_on_timer(TrackerHeader*, uintptr_t timer_id);
static void trackerheader_update_play_ons(TrackerHeader*);
static void trackerheader_on_mouse_wheel(TrackerHeader*, psy_ui_MouseEvent*);
static void trackerheader_on_track_state_changed(TrackerHeader*,
	psy_audio_TrackState* sender);

/* vtable */
static psy_ui_ComponentVtable trackerheader_vtable;
static bool trackerheader_vtable_initialized = FALSE;

static void trackerheader_vtable_init(TrackerHeader* self)
{
	if (!trackerheader_vtable_initialized) {
		trackerheader_vtable = *(self->component.vtable);
		trackerheader_vtable.on_destroyed =
			(psy_ui_fp_component)
			trackerheader_on_destroyed;
		trackerheader_vtable.on_timer =
			(psy_ui_fp_component_on_timer)
			trackerheader_on_timer;
		trackerheader_vtable.on_mouse_wheel =
			(psy_ui_fp_component_on_mouse_event)
			trackerheader_on_mouse_wheel;
		trackerheader_vtable_initialized = TRUE;
	}
	psy_ui_component_set_vtable(&self->component, &trackerheader_vtable);	
}

/* implementation */
void trackerheader_init(TrackerHeader* self, psy_ui_Component* parent,
	TrackConfig* track_config, TrackerState* state, Workspace* workspace)
{
	psy_ui_component_init(&self->component, parent, NULL);
	trackerheader_vtable_init(self);	
	psy_ui_component_set_align_expand(&self->component, psy_ui_HEXPAND);
	self->component.blitscroll = TRUE;
	self->state = state;	
	self->workspace = workspace;	
	psy_table_init(&self->boxes);	
	psy_signal_connect(&workspace->player.signal_song_changed, self,
		trackerheader_on_song_changed);
	trackerheader_connect_song(self);	
	trackerheader_build(self);	
	psy_ui_component_start_timer(&self->component, 0, 50);
}

void trackerheader_on_destroyed(TrackerHeader* self)
{
	psy_table_dispose(&self->boxes);
}

void trackerheader_build(TrackerHeader* self)
{
	uintptr_t track;
	uintptr_t numtracks;

	psy_ui_component_clear(&self->component);
	psy_table_clear(&self->boxes);
	numtracks = patternviewstate_num_song_tracks(self->state->pv);
	for (track = 0; track < numtracks; ++track) {		
		psy_table_insert(&self->boxes, track,
			psy_ui_component_set_align(
				patterntrack_base(patterntrack_allocinit(
					trackerheader_base(self), track, self->state)),
				psy_ui_ALIGN_LEFT));
	}	
	psy_ui_component_align(trackerheader_base(self));
}

void trackerheader_on_song_changed(TrackerHeader* self,
	psy_audio_Player* sender)
{
	trackerheader_connect_song(self);
}

void trackerheader_connect_song(TrackerHeader* self)
{
	if (workspace_song(self->workspace)) {
		psy_signal_connect(&psy_audio_song_sequence(workspace_song(
			self->workspace))->signal_cursorchanged,
			self, trackerheader_on_cursor_changed);
		psy_signal_connect(&psy_audio_song_patterns(workspace_song(
			self->workspace))->trackstate.signal_changed,
			self, trackerheader_on_track_state_changed);
	}
}

void trackerheader_on_cursor_changed(TrackerHeader* self,
	psy_audio_Sequence* sender)
{		
	if (psy_audio_sequencecursor_track(&sender->lastcursor) !=
			psy_audio_sequencecursor_track(&sender->cursor)) {		
		psy_ui_component_invalidate(&self->component);	
	}		
}

void trackerheader_on_timer(TrackerHeader* self, uintptr_t timer_id)
{	
	trackerheader_update_play_ons(self);
}

void trackerheader_update_play_ons(TrackerHeader* self)
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
			patterntrackbox_play_on(&track->trackbox);
		} else {
			patterntrackbox_play_off(&track->trackbox);
		}
	}
}

void trackerheader_on_mouse_wheel(TrackerHeader* self, psy_ui_MouseEvent* ev)
{	
	psy_audio_player_sendcmd(workspace_player(self->workspace), "tracker",
		psy_eventdrivercmd_makeid(
			(psy_ui_mouseevent_delta(ev) > 0)
			? CMD_COLUMNNEXT
			: CMD_COLUMNPREV));
}

void trackerheader_on_track_state_changed(TrackerHeader* self,
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
static void trackerheaderview_on_switch_header_line(TrackerHeaderView*, psy_ui_Component* sender);
static void trackerheaderview_on_update_size(TrackerHeaderView*,
	psy_Property* sender);
static void trackerheaderview_update_preferred_size(TrackerHeaderView*);

/* implementation */
void trackerheaderview_init(TrackerHeaderView* self, psy_ui_Component* parent, TrackConfig* config,
	TrackerState* state, Workspace* workspace)
{
	PatternViewConfig* pvconfig;
	
	psy_ui_component_init(&self->component, parent, NULL);
	/* desc */
	psy_ui_button_init_connect(&self->desc, &self->component,
		self, trackerheaderview_on_switch_header_line);
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
	trackerheaderview_update_preferred_size(self);
	/* configuration */
	pvconfig = &self->header.workspace->config.visual.patview;	
	patternviewconfig_connect(pvconfig, "linenumbers",
		self, trackerheaderview_on_update_size);
	patternviewconfig_connect(pvconfig, "beatoffset",
		self, trackerheaderview_on_update_size);
	patternviewconfig_connect(pvconfig, "displaysinglepattern",
		self, trackerheaderview_on_update_size);	
}

void trackerheaderview_on_switch_header_line(TrackerHeaderView* self, psy_ui_Component* sender)
{
	patternviewtheme_switch_header(&self->header.workspace->config.visual.patview.theme);
	trackerheaderview_update_preferred_size(self);
}

void trackerheaderview_on_update_size(TrackerHeaderView* self,
	psy_Property* sender)
{
	trackerheaderview_update_preferred_size(self);
}

void trackerheaderview_update_preferred_size(TrackerHeaderView* self)
{
	PatternViewConfig* config;

	config = &self->header.workspace->config.visual.patview;
	if (patternviewconfig_linenumber_num_digits(config) == 0.0) {
		psy_ui_component_hide(&self->desc.component);
	}
	else {
		psy_ui_component_set_preferred_size(&self->desc.component,
			psy_ui_size_make(
				psy_ui_mul_values(
					psy_ui_value_make_ew(
						patternviewconfig_linenumber_num_digits(
							config)),
					self->header.state->track_config->flatsize,
					psy_ui_component_textmetric(&self->component),
					NULL),
				psy_ui_value_make_eh(1.0)));
		psy_ui_component_show(&self->desc.component);
	}
	psy_ui_component_align(&self->component);
	psy_ui_component_invalidate(&self->component);
}


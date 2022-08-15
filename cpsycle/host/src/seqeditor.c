/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "seqeditor.h"
/* host */
#include "styles.h"

/* prototypes */
static void seqeditor_on_destroyed(SeqEditor*);
static void seqeditor_on_song_changed(SeqEditor*, Workspace* sender);
static void seqeditor_update_song(SeqEditor*);
static void seqeditor_build(SeqEditor*);
static void seqeditor_on_tracks_scroll(SeqEditor*, psy_ui_Component* sender);
static void seqeditor_on_configure(SeqEditor*, GeneralConfig* sender,
	psy_Property*);
static void seqeditor_on_hzoom(SeqEditor*, ZoomBox* sender);
static void seqeditor_update_scroll_step(SeqEditor*);
static void seqeditor_on_clear(SeqEditor*, psy_audio_Sequence*);
static void seqeditor_on_track_insert(SeqEditor*, psy_audio_Sequence*,
	uintptr_t trackidx);
static void seqeditor_on_track_remove(SeqEditor*, psy_audio_Sequence*,
	uintptr_t trackidx);
static void seqeditor_on_track_swap(SeqEditor*,
	psy_audio_Sequence* sender, uintptr_t first, uintptr_t second);
static void seqeditor_on_mouse_move(SeqEditor*, psy_ui_MouseEvent*);
static void seqeditor_on_mouse_up(SeqEditor*, psy_ui_MouseEvent*);
static void seqeditor_on_toggle_expand(SeqEditor*, psy_ui_Button* sender);
static void seqeditor_on_track_resize(SeqEditor*, psy_ui_Component* sender,
	uintptr_t trackid, double* height);
static void seqeditor_on_toggle_timesig(SeqEditor*, psy_ui_Button* sender);
static void seqeditor_on_toggle_loop(SeqEditor*, psy_ui_Button* sender);
static void seqeditor_on_cursor_changed(SeqEditor*, psy_audio_Sequence* sender);

/* vtable */
static psy_ui_ComponentVtable seqeditor_vtable;
static bool seqeditor_vtable_initialized = FALSE;

static void seqeditor_vtable_init(SeqEditor* self)
{
	if (!seqeditor_vtable_initialized) {
		seqeditor_vtable = *(self->component.vtable);
		seqeditor_vtable.on_destroyed =
			(psy_ui_fp_component_event)
			seqeditor_on_destroyed;
		seqeditor_vtable.on_mouse_move =
			(psy_ui_fp_component_on_mouse_event)
			seqeditor_on_mouse_move;
		seqeditor_vtable.on_mouse_up =
			(psy_ui_fp_component_on_mouse_event)
			seqeditor_on_mouse_up;

		seqeditor_vtable_initialized = TRUE;
	}
	self->component.vtable = &seqeditor_vtable;
}
/* implementation */
void seqeditor_init(SeqEditor* self, psy_ui_Component* parent,
	Workspace* workspace)
{	
	psy_ui_component_init(&self->component, parent, NULL);
	seqeditor_vtable_init(self);
	psy_ui_component_set_title(&self->component, "seqedit.editor");	
	psy_ui_component_set_style_type(&self->component, STYLE_SEQEDT);
	psy_ui_component_set_preferred_height(&self->component,
		psy_ui_value_make_ph(0.20));	
	sequencecmds_init(&self->cmds, workspace);
	seqeditstate_init(&self->state, &self->cmds, &self->component);	
	/* toolbar */
	seqedittoolbar_init(&self->toolbar, &self->component, &self->state);
	psy_ui_component_set_align(&self->toolbar.component, psy_ui_ALIGN_TOP);	
	/* spacer */
	psy_ui_component_init_align(&self->spacer, &self->component, NULL,
		psy_ui_ALIGN_TOP);
	psy_ui_component_set_style_type(&self->spacer, STYLE_SEQEDT_SPACER);
	/* track_header */
	psy_ui_component_init(&self->tracks, &self->component, NULL);	
	psy_ui_component_set_align(&self->tracks, psy_ui_ALIGN_CLIENT);
	/* track_header */
	psy_ui_component_init(&self->tracks_header, &self->tracks, NULL);	
	psy_ui_component_set_align(&self->tracks_header, psy_ui_ALIGN_TOP);
	/* SeqEditorHeaderBar */
	seqeditorheaderdescbar_init(&self->headerdescbar, &self->tracks_header,
		&self->state);
	psy_ui_component_set_align(&self->headerdescbar.component,
		psy_ui_ALIGN_LEFT);	
	psy_signal_connect(&self->headerdescbar.hzoom.signal_changed, self,
		seqeditor_on_hzoom);
	/* items header */
	seqeditheader_init(&self->header, &self->tracks_header, &self->state);
	psy_ui_component_set_align(&self->header.component, psy_ui_ALIGN_CLIENT);
	/* tracks pane */
	psy_ui_component_init(&self->tracks_pane, &self->tracks, NULL);	
	psy_ui_component_set_style_type(&self->tracks_pane, STYLE_SEQEDT_TRACKS);
	psy_ui_component_set_align(&self->tracks_pane, psy_ui_ALIGN_CLIENT);	
	/* track description */
	psy_ui_component_init(&self->trackdescpane, &self->tracks_pane, NULL);
	psy_ui_component_set_align(&self->trackdescpane, psy_ui_ALIGN_LEFT);
	psy_ui_component_set_preferred_width(&self->trackdescpane,
		psy_ui_value_make_ew(40.0));
	seqedittrackdesc_init(&self->trackdesc, &self->trackdescpane,
		&self->state, workspace);		
	psy_signal_connect(&self->trackdesc.signal_resize, self,
		seqeditor_on_track_resize);
	psy_ui_component_set_align(&self->trackdesc.component,
		psy_ui_ALIGN_HCLIENT);	
	/* connect expand */
	psy_signal_connect(&self->toolbar.expand.signal_clicked, self,
		seqeditor_on_toggle_expand);
	self->expanded = FALSE;
	/* track items */
	seqeditortracks_init(&self->track_items, &self->tracks_pane, &self->state,
		workspace);
	psy_ui_scroller_init(&self->scroller, &self->tracks_pane, NULL, NULL);
	psy_ui_scroller_set_client(&self->scroller, &self->track_items.component);
	psy_ui_component_set_align(&self->track_items.component,
		psy_ui_ALIGN_FIXED);
	psy_ui_component_set_align(&self->scroller.component, psy_ui_ALIGN_CLIENT);	
	/* properties */
	seqeditproperties_init(&self->properties, &self->component, &self->state);
	psy_ui_component_set_align(&self->properties.component, psy_ui_ALIGN_RIGHT);
	psy_ui_splitter_init(&self->splitbar_properties, &self->component);
	psy_ui_component_set_align(psy_ui_splitter_base(&self->splitbar_properties),
		psy_ui_ALIGN_RIGHT);	
	/* update */	
	seqeditor_update_song(self);	
	/* connect signals */
	psy_signal_connect(&self->state.workspace->signal_songchanged, self,
		seqeditor_on_song_changed);
	psy_signal_connect(&self->track_items.component.signal_scrolled, self,
		seqeditor_on_tracks_scroll);	
	psy_signal_connect(
		&psycleconfig_general(workspace_conf(workspace))->signal_changed,
		self, seqeditor_on_configure);	
	psy_signal_connect(&self->toolbar.timesig.signal_clicked, self,
		seqeditor_on_toggle_timesig);
	psy_signal_connect(&self->toolbar.loop.signal_clicked, self,
		seqeditor_on_toggle_loop);
	seqeditor_update_scroll_step(self);
}

void seqeditor_on_destroyed(SeqEditor* self)
{
	seqeditstate_dispose(&self->state);
}

void seqeditor_on_song_changed(SeqEditor* self, Workspace* sender)
{	
	seqeditor_update_song(self);
}

void seqeditor_update_song(SeqEditor* self)
{
	psy_audio_Song* song;

	song = self->state.workspace->song;
	if (song) {
		psy_ui_component_move(&self->trackdesc.component, psy_ui_point_zero());
		psy_ui_component_move(&self->track_items.component,
			psy_ui_point_zero());
		seqeditor_build(self);
		seqeditortoolbar_update_track_name(&self->toolbar);
		psy_ui_component_invalidate(&self->trackdesc.component);
		psy_ui_component_invalidate(&self->header.component);
		psy_ui_component_align(&self->track_items.component);
		psy_ui_component_invalidate(&self->track_items.component);
		psy_signal_connect(&song->sequence.signal_clear, self,
			seqeditor_on_clear);
		psy_signal_connect(&song->sequence.signal_trackinsert, self,
			seqeditor_on_track_insert);
		psy_signal_connect(&song->sequence.signal_trackswap, self,
			seqeditor_on_track_swap);
		psy_signal_connect(&song->sequence.signal_trackremove, self,
			seqeditor_on_track_remove);
		psy_signal_connect(&song->sequence.signal_cursorchanged, self,
			seqeditor_on_cursor_changed);
	}
}

void seqeditor_update_scroll_step(SeqEditor* self)
{		
	psy_ui_component_set_scroll_step_height(&self->track_items.component,
		seqeditstate_line_height(&self->state));
	psy_ui_component_set_scroll_step_height(&self->trackdesc.component,
		seqeditstate_line_height(&self->state));
}

void seqeditor_on_clear(SeqEditor* self, psy_audio_Sequence* sender)
{
	psy_ui_component_move(&self->trackdesc.component, psy_ui_point_zero());
	psy_ui_component_move(&self->track_items.component, psy_ui_point_zero());
	seqeditor_build(self);
	psy_ui_component_invalidate(&self->trackdesc.component);
	psy_ui_component_align(&self->track_items.component);
	psy_ui_component_invalidate(&self->track_items.component);
}

void seqeditor_on_track_insert(SeqEditor* self, psy_audio_Sequence* sender,
	uintptr_t trackidx)
{
	seqeditor_build(self);	
}

void seqeditor_on_track_swap(SeqEditor* self, psy_audio_Sequence* sender,
	uintptr_t first, uintptr_t second)
{
	seqeditor_build(self);
	psy_ui_component_invalidate(&self->trackdesc.component);
	psy_ui_component_align(&self->track_items.component);
	psy_ui_component_invalidate(&self->track_items.component);
}

void seqeditor_on_track_remove(SeqEditor* self, psy_audio_Sequence* sender,
	uintptr_t trackidx)
{
	seqeditor_build(self);	
}

void seqeditor_build(SeqEditor* self)
{	
	seqeditortracks_build(&self->track_items);
	seqedittrackdesc_build(&self->trackdesc);	
	psy_ui_component_align(&self->scroller.pane);	
	psy_ui_component_align(&self->trackdescpane);
	psy_ui_component_invalidate(&self->component);
}

void seqeditor_on_tracks_scroll(SeqEditor* self, psy_ui_Component* sender)
{	
	psy_ui_component_set_scroll_top(&self->trackdesc.component,
		psy_ui_component_scroll_top(&self->track_items.component));
	psy_ui_component_set_scroll_left(&self->header.pane,
		psy_ui_component_scroll_left(&self->track_items.component));
}

void seqeditor_on_configure(SeqEditor* self, GeneralConfig* sender,
	psy_Property* property)
{	
	self->state.showpatternnames = generalconfig_showing_pattern_names(sender);
	psy_ui_component_invalidate(&self->track_items.component);
}

void seqeditor_on_hzoom(SeqEditor* self, ZoomBox* sender)
{
	self->state.pxperbeat = sender->zoomrate * self->state.defaultpxperbeat;
	psy_ui_component_align(&self->tracks);
	psy_ui_component_invalidate(&self->tracks);
}

void seqeditor_on_mouse_move(SeqEditor* self, psy_ui_MouseEvent* ev)
{
	if (self->state.updatecursorposition) {
		psy_ui_component_invalidate(&self->header.component);
		self->state.updatecursorposition = FALSE;		
	}
}

void seqeditor_on_mouse_up(SeqEditor* self, psy_ui_MouseEvent* ev)
{	
	if (self->state.cmd == SEQEDTCMD_NEWTRACK) {
		sequencecmds_append_track(self->state.cmds);
	} else if (self->state.cmd == SEQEDTCMD_DELTRACK) {
		sequencecmds_remove_track(self->state.cmds,
			self->state.dragseqpos.track);
	}	
	self->state.cmd = SEQEDTCMD_NONE;
}

void seqeditor_on_toggle_expand(SeqEditor* self, psy_ui_Button* sender)
{	
	self->expanded = !self->expanded;	
	psy_ui_component_set_preferred_size(&self->component,
		psy_ui_size_make(psy_ui_value_zero(), psy_ui_value_make_ph(
			(self->expanded) ? 0.75 : 0.2)));
	psy_ui_component_align(psy_ui_component_parent(&self->component));
	psy_ui_component_invalidate(psy_ui_component_parent(&self->component));
}

void seqeditor_on_track_resize(SeqEditor* self, psy_ui_Component* sender,
	uintptr_t track_id, double* height)
{
	psy_ui_Component* track;
	
	assert(height);

	track = psy_ui_component_at(&self->track_items.component, track_id);
	if (track) {		
		psy_ui_component_set_preferred_height(track,
			psy_ui_value_make_eh(*height));
		psy_ui_component_align(&self->tracks_pane);				
		psy_ui_component_invalidate(&self->tracks_pane);
	}
}

void seqeditor_on_toggle_timesig(SeqEditor* self, psy_ui_Button* sender)
{	
	psy_ui_Size size;

	if (psy_ui_component_visible(&self->header.timesig.component)) {
		seqeditheader_hide_timesig(&self->header);		
	} else {
		seqeditheader_show_timesig(&self->header);
	}
	size = psy_ui_component_preferred_size(&self->header.component, NULL);
	if (psy_ui_component_visible(&self->header.timesig.component)) {
		psy_ui_button_highlight(sender);
		psy_ui_button_set_icon(sender, psy_ui_ICON_LESS);
		psy_ui_component_align(psy_ui_component_parent(
			psy_ui_button_base(sender)));		
	} else {		
		psy_ui_button_disable_highlight(sender);
		psy_ui_button_set_icon(sender, psy_ui_ICON_MORE);
		psy_ui_component_align(psy_ui_component_parent(
			psy_ui_button_base(sender)));
	}
	psy_ui_component_set_preferred_height(&self->headerdescbar.component,
		size.height);
	psy_ui_component_align(&self->tracks_pane);
	psy_ui_component_align(&self->component);
	psy_ui_component_invalidate(&self->component);
}

void seqeditor_on_toggle_loop(SeqEditor* self, psy_ui_Button* sender)
{
	psy_ui_Size size;

	if (psy_ui_component_visible(&self->header.loops.component)) {
		seqeditheader_hide_loops(&self->header);
	} else {
		seqeditheader_show_loops(&self->header);
	}
	size = psy_ui_component_preferred_size(&self->header.component, NULL);
	if (psy_ui_component_visible(&self->header.loops.component)) {
		psy_ui_button_highlight(sender);
		psy_ui_component_align(psy_ui_component_parent(
			psy_ui_button_base(sender)));		
	} else {		
		psy_ui_button_disable_highlight(sender);
		psy_ui_component_align(psy_ui_component_parent(
			psy_ui_button_base(sender)));		
	}
	psy_ui_component_set_preferred_height(&self->headerdescbar.component,
		size.height);
	psy_ui_component_align(&self->tracks_pane);
	psy_ui_component_align(&self->component);
	psy_ui_component_invalidate(&self->component);
}

void seqeditor_on_cursor_changed(SeqEditor* self, psy_audio_Sequence* sender)
{
	if (!psy_audio_orderindex_equal(&sender->cursor.order_index,
			sender->lastcursor.order_index)) {		
	}
}

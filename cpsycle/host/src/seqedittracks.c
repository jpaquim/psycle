/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "seqedittracks.h"
/* host */
#include "styles.h"


/* prototypes */
static void seqeditortracks_on_mouse_down(SeqEditorTracks*, psy_ui_MouseEvent*);
static void seqeditortracks_on_mouse_move(SeqEditorTracks*, psy_ui_MouseEvent*);
static void seqeditortracks_on_mouse_up(SeqEditorTracks*, psy_ui_MouseEvent*);
static void seqeditortracks_on_mouse_enter(SeqEditorTracks*);
static void seqeditortracks_on_mouse_leave(SeqEditorTracks*);
static void seqeditortracks_on_sequence_selection_select(SeqEditorTracks*,
	psy_audio_SequenceSelection*, psy_audio_OrderIndex*);
static void seqeditortracks_on_sequence_selection_deselect(SeqEditorTracks*,
	psy_audio_SequenceSelection*, psy_audio_OrderIndex*);
static void seqeditortracks_on_align(SeqEditorTracks*);
static void seqeditortracks_on_timer(SeqEditorTracks*, uintptr_t timerid);
static void seqeditortracks_on_cursor_changed(SeqEditorTracks*, SeqEditState*);
static void seqeditortracks_update_cursor_line_position(SeqEditorTracks*);
static void seqeditortracks_update_seq_edit_line_position(SeqEditorTracks*);

/* vtable */
static psy_ui_ComponentVtable seqeditortracks_vtable;
static bool seqeditortracks_vtable_initialized = FALSE;

static void seqeditortracks_vtable_init(SeqEditorTracks* self)
{
	if (!seqeditortracks_vtable_initialized) {
		seqeditortracks_vtable = *(self->component.vtable);		
		seqeditortracks_vtable.onalign =
			(psy_ui_fp_component)
			seqeditortracks_on_align;
		seqeditortracks_vtable.on_timer =
			(psy_ui_fp_component_on_timer)
			seqeditortracks_on_timer;
		seqeditortracks_vtable.on_mouse_down =
			(psy_ui_fp_component_on_mouse_event)
			seqeditortracks_on_mouse_down;
		seqeditortracks_vtable.on_mouse_up =
			(psy_ui_fp_component_on_mouse_event)
			seqeditortracks_on_mouse_up;
		seqeditortracks_vtable.on_mouse_move =
			(psy_ui_fp_component_on_mouse_event)
			seqeditortracks_on_mouse_move;
		seqeditortracks_vtable.onmouseenter =
			(psy_ui_fp_component)
			seqeditortracks_on_mouse_enter;
		seqeditortracks_vtable.onmouseleave =
			(psy_ui_fp_component)
			seqeditortracks_on_mouse_leave;		
		seqeditortracks_vtable_initialized = TRUE;
	}
	self->component.vtable = &seqeditortracks_vtable;
}

/* implementation */
void seqeditortracks_init(SeqEditorTracks* self, psy_ui_Component* parent,
	SeqEditState* state, Workspace* workspace)
{		
	psy_ui_component_init(&self->component, parent, NULL);
	seqeditortracks_vtable_init(self);
	self->workspace = workspace;
	self->state = state;
	self->playline = NULL;
	self->cursorline = NULL;
	self->seqeditposline = NULL;
	psy_ui_component_set_style_type(&self->component, STYLE_SEQEDT_TRACKS);
	psy_ui_component_set_scroll_step(&self->component,
		psy_ui_size_make_em(8.0, 0.0));	
	psy_ui_component_set_wheel_scroll(&self->component, 1);	
	psy_ui_component_set_overflow(&self->component, psy_ui_OVERFLOW_SCROLL);	
	psy_ui_component_set_default_align(&self->component,
		psy_ui_ALIGN_TOP, psy_ui_margin_zero());
	psy_signal_connect(&workspace->song->sequence.selection.signal_select,
		self, seqeditortracks_on_sequence_selection_select);
	psy_signal_connect(&workspace->song->sequence.selection.signal_deselect,
		self, seqeditortracks_on_sequence_selection_deselect);
	seqeditortracks_build(self);
	psy_signal_connect(&self->state->signal_cursorchanged, self,
		seqeditortracks_on_cursor_changed);
	psy_ui_component_start_timer(&self->component, 0, 50);
}

void seqeditortracks_build(SeqEditorTracks* self)
{
	psy_audio_Sequence* sequence;	

	self->playline = NULL;
	psy_ui_component_clear(&self->component);
	sequence = seqeditstate_sequence(self->state);	
	if (sequence) {
		uintptr_t t;		

		for (t = 0; t < psy_audio_sequence_width(sequence); ++t) {
			psy_audio_SequenceTrack* seqtrack;
			SeqEditTrack* seqedittrack;
						
			seqtrack = psy_audio_sequence_track_at(sequence, t);			
			seqedittrack = seqedittrack_allocinit(&self->component,
				self->state, seqtrack, t);			
		}		
	}
	self->playline = seqeditorplayline_allocinit(&self->component,
		self->state);
	if (self->playline) {
		psy_ui_component_set_align(&self->playline->seqeditorline.component,
			psy_ui_ALIGN_NONE);
		psy_ui_component_set_margin(&self->playline->seqeditorline.component,
			psy_ui_margin_zero());		
		seqeditorplayline_update(self->playline);
	}
	self->cursorline = seqeditorline_allocinit(&self->component, self->state);
	if (self->cursorline) {
		psy_ui_component_set_align(&self->cursorline->component,
			psy_ui_ALIGN_NONE);
		psy_ui_component_set_margin(&self->cursorline->component,
			psy_ui_margin_zero());
		psy_ui_component_hide(&self->cursorline->component);		
	}
	self->seqeditposline = seqeditorline_allocinit(&self->component,
		self->state);
	if (self->seqeditposline) {
		psy_ui_component_set_align(&self->seqeditposline->component,
			psy_ui_ALIGN_NONE);
		psy_ui_component_set_margin(&self->seqeditposline->component,
			psy_ui_margin_zero());		
		seqeditortracks_update_seq_edit_line_position(self);
	}
}

void seqeditortracks_on_sequence_selection_select(SeqEditorTracks* self,
	psy_audio_SequenceSelection* sender, psy_audio_OrderIndex* index)
{
	seqeditortracks_update_seq_edit_line_position(self);	
}

void seqeditortracks_on_sequence_selection_deselect(SeqEditorTracks* self,
	psy_audio_SequenceSelection* sender, psy_audio_OrderIndex* index)
{
	if (self->seqeditposline) {
		psy_audio_SequenceEntry* seqentry;
				
		seqentry = psy_audio_sequence_entry(seqeditstate_sequence(self->state),
			psy_audio_sequenceselection_first(sender));
		if (seqentry) {
			seqeditorline_updateposition(self->seqeditposline,
				seqentry->offset);
		}
	}
	psy_ui_component_invalidate(&self->component);
}

void seqeditortracks_on_align(SeqEditorTracks* self)
{	
	if (self->playline) {
		seqeditorplayline_update(self->playline);
	}
	seqeditortracks_update_cursor_line_position(self);
	seqeditortracks_update_seq_edit_line_position(self);
}

void seqeditortracks_on_cursor_changed(SeqEditorTracks* self,
	SeqEditState* sender)
{
	seqeditortracks_update_cursor_line_position(self);
}

void seqeditortracks_update_cursor_line_position(SeqEditorTracks* self)
{
	if (self->cursorline) {
		if (self->state->cursoractive && !psy_ui_component_visible(
				&self->cursorline->component)) {
			psy_ui_component_show(&self->cursorline->component);
		} else if (!self->state->cursoractive &&
				psy_ui_component_visible(&self->cursorline->component)) {
			psy_ui_component_hide(&self->cursorline->component);
			psy_ui_component_invalidate(&self->component);
			return;
		}
		seqeditorline_updateposition(self->cursorline,
			self->state->cursorposition);		
	}
}

void seqeditortracks_update_seq_edit_line_position(SeqEditorTracks* self)
{
	if (self->seqeditposline) {
		psy_audio_SequenceEntry* seqentry;

		seqentry = psy_audio_sequence_entry(
			seqeditstate_sequence(self->state),
			psy_audio_sequenceselection_first(
				&self->workspace->song->sequence.selection));
		if (seqentry) {
			seqeditorline_updateposition(self->seqeditposline,
				seqentry->offset);
		}
	}
}

void seqeditortracks_on_timer(SeqEditorTracks* self, uintptr_t timerid)
{		
	if (psy_audio_player_playing(workspace_player(self->workspace))) {
		seqeditorplayline_update(self->playline);
	}
}

void seqeditortracks_on_mouse_down(SeqEditorTracks* self,
	psy_ui_MouseEvent* ev)
{
	if (self->state->dragstatus == SEQEDIT_DRAG_REMOVE) {
		sequencecmds_delentry(self->state->cmds);
		self->state->dragstatus = SEQEDIT_DRAG_NONE;
		psy_ui_mouseevent_stop_propagation(ev);
	} else if ((self->state->dragtype & SEQEDIT_DRAGTYPE_REORDER)
			== SEQEDIT_DRAGTYPE_REORDER) {
		if (self->cursorline) {
			psy_ui_component_hide(&self->cursorline->component);
			psy_ui_component_invalidate(&self->component);
		}
	}
}

void seqeditortracks_on_mouse_move(SeqEditorTracks* self,
	psy_ui_MouseEvent* ev)
{	
	seqeditstate_setcursor(self->state, seqeditstate_quantize(self->state,
		seqeditstate_pxtobeat(self->state, psy_ui_mouseevent_pt(ev).x)));
	if (self->state->dragstatus &&
			(self->state->dragtype & SEQEDIT_DRAGTYPE_MOVE) ==
			SEQEDIT_DRAGTYPE_MOVE) {
		seqeditortracks_update_seq_edit_line_position(self);
	}
}

void seqeditortracks_on_mouse_up(SeqEditorTracks* self,
	psy_ui_MouseEvent* ev)
{
	psy_ui_component_release_capture(&self->component);	
	self->state->cmd = SEQEDTCMD_NONE;
	self->state->seqentry = NULL;
	self->state->dragseqpos = psy_audio_orderindex_make_invalid();
	self->state->dragstatus = SEQEDIT_DRAG_NONE;
	psy_ui_mouseevent_stop_propagation(ev);
}

void seqeditortracks_on_mouse_enter(SeqEditorTracks* self)
{
	self->state->cursoractive = TRUE;	
	psy_signal_emit(&self->state->signal_cursorchanged, self->state, 0);
}

void seqeditortracks_on_mouse_leave(SeqEditorTracks* self)
{
	self->state->cursoractive = FALSE;
	if (self->cursorline) {
		psy_ui_component_hide(&self->cursorline->component);
		psy_ui_component_invalidate(&self->component);
	}
	psy_signal_emit(&self->state->signal_cursorchanged, self->state, 0);
}

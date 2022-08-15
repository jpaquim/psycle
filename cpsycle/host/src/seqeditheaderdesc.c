/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "seqeditheaderdesc.h"
/* host */
#include "resources/resource.h"
#include "styles.h"
/* ui */
#include "uiapp.h"

/* prototypes */
static void seqeditorheaderdescbar_oninserttrack(SeqEditorHeaderDescBar*,
	psy_ui_Button* sender);
static void seqeditorheaderdescbar_ondeletetrack(SeqEditorHeaderDescBar*,
	psy_ui_Button* sender);
static void seqeditorheaderdescbar_onmovetrackup(SeqEditorHeaderDescBar*,
	psy_ui_Button* sender);
static void seqeditorheaderdescbar_onmovetrackdown(SeqEditorHeaderDescBar*,
	psy_ui_Button* sender);

/* implementation */
void seqeditorheaderdescbar_init(SeqEditorHeaderDescBar* self,
	psy_ui_Component* parent, SeqEditState* state)
{
	psy_ui_Margin margin;

	psy_ui_component_init(&self->component, parent, NULL);
	psy_ui_component_set_style_type(&self->component, STYLE_SEQEDT_TRACKS);
	self->state = state;
	psy_ui_component_set_preferred_size(&self->component,
		psy_ui_size_make_em(40.0, 2.0));	
	psy_ui_component_init(&self->top, &self->component, NULL);
	psy_ui_component_set_align(&self->top, psy_ui_ALIGN_TOP);
	psy_ui_component_set_align_expand(&self->top, psy_ui_HEXPAND);
	psy_ui_component_set_default_align(&self->top, psy_ui_ALIGN_LEFT,
		psy_ui_defaults_hmargin(psy_ui_defaults()));		
	psy_ui_button_init_connect(&self->insert, &self->top, self,
		seqeditorheaderdescbar_oninserttrack);
	psy_ui_button_load_resource(&self->insert, IDB_SEQ_ADD_TRACK,
		IDB_SEQ_ADD_TRACK, psy_ui_colour_white());
	psy_ui_button_init_connect(&self->del, &self->top, self,
		seqeditorheaderdescbar_ondeletetrack);
	psy_ui_button_load_resource(&self->del, IDB_SEQ_DEL_TRACK,
		IDB_SEQ_DEL_TRACK, psy_ui_colour_white());	
	margin = psy_ui_defaults_hmargin(psy_ui_defaults());
	psy_ui_margin_setright(&margin, psy_ui_value_make_ew(3.0));
	psy_ui_component_set_margin(psy_ui_button_base(&self->del), margin);
	psy_ui_button_init_connect(&self->up, &self->top, self,
		seqeditorheaderdescbar_onmovetrackup);
	psy_ui_button_load_resource(&self->up, IDB_ARROW_UP_DARK,
		IDB_ARROW_UP_DARK, psy_ui_colour_white());	
	psy_ui_button_init_connect(&self->down, &self->top, self,
		seqeditorheaderdescbar_onmovetrackdown);
	psy_ui_button_load_resource(&self->down, IDB_ARROW_DOWN_DARK,
		IDB_ARROW_DOWN_DARK, psy_ui_colour_white());
	psy_ui_component_set_margin(psy_ui_button_base(&self->down), margin);
	zoombox_init(&self->hzoom, &self->top);
}

void seqeditorheaderdescbar_oninserttrack(SeqEditorHeaderDescBar* self,
	psy_ui_Button* sender)
{	
	sequencecmds_insert_track(self->state->cmds);	
}

void seqeditorheaderdescbar_ondeletetrack(SeqEditorHeaderDescBar* self,
	psy_ui_Button* sender)
{
	psy_audio_OrderIndex editpos;

	editpos = seqeditstate_editposition(self->state);
	if (editpos.track > 0) {
		sequencecmds_remove_track(self->state->cmds, editpos.track);
	}
}

void seqeditorheaderdescbar_onmovetrackup(SeqEditorHeaderDescBar* self,
	psy_ui_Button* sender)
{
	psy_audio_Sequence* sequence;

	sequence = seqeditstate_sequence(self->state);
	if (sequence) {
		psy_audio_OrderIndex editpos;

		editpos = seqeditstate_editposition(self->state);
		if (editpos.track > 0) {
			psy_audio_SequenceCursor cursor;

			psy_audio_sequence_swaptracks(sequence, editpos.track - 1,
				editpos.track);
			--editpos.track;
			psy_audio_sequencecursor_init(&cursor);
			psy_audio_sequencecursor_set_order_index(&cursor, editpos);
			if (self->state->workspace && workspace_song(self->state->workspace)) {
				psy_audio_sequence_set_cursor(
					psy_audio_song_sequence(workspace_song(self->state->workspace)),
					cursor);
			}			
		}
	}
}

void seqeditorheaderdescbar_onmovetrackdown(SeqEditorHeaderDescBar* self,
	psy_ui_Button* sender)
{
	psy_audio_Sequence* sequence;

	sequence = seqeditstate_sequence(self->state);
	if (sequence) {
		psy_audio_OrderIndex editpos;
		psy_audio_SequenceCursor cursor;

		editpos = seqeditstate_editposition(self->state);
		if (editpos.track + 1 < psy_audio_sequence_width(sequence)) {
			psy_audio_sequence_swaptracks(sequence, editpos.track,
				editpos.track + 1);
			++editpos.track;			
			psy_audio_sequencecursor_init(&cursor);
			psy_audio_sequencecursor_set_order_index(&cursor, editpos);
			if (self->state->workspace && workspace_song(self->state->workspace)) {
				psy_audio_sequence_set_cursor(
					psy_audio_song_sequence(workspace_song(self->state->workspace)),
					cursor);
			}
		}
	}
}

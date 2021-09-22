/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
**  copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net
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
	self->state = state;
	psy_ui_component_setpreferredsize(&self->component,
		psy_ui_size_make_em(40.0, 2.0));	
	psy_ui_component_init(&self->top, &self->component, NULL);
	psy_ui_component_setalign(&self->top, psy_ui_ALIGN_TOP);
	psy_ui_component_setalignexpand(&self->top, psy_ui_HEXPAND);
	psy_ui_component_setdefaultalign(&self->top, psy_ui_ALIGN_LEFT,
		psy_ui_defaults_hmargin(psy_ui_defaults()));		
	psy_ui_button_init_connect(&self->insert, &self->top, NULL, self,
		seqeditorheaderdescbar_oninserttrack);
	psy_ui_button_loadresource(&self->insert, IDB_SEQ_ADD_TRACK,
		IDB_SEQ_ADD_TRACK, psy_ui_colour_white());
	psy_ui_button_init_connect(&self->del, &self->top, NULL, self,
		seqeditorheaderdescbar_ondeletetrack);
	psy_ui_button_loadresource(&self->del, IDB_SEQ_DEL_TRACK,
		IDB_SEQ_DEL_TRACK, psy_ui_colour_white());	
	margin = psy_ui_defaults_hmargin(psy_ui_defaults());
	psy_ui_margin_setright(&margin, psy_ui_value_make_ew(3.0));
	psy_ui_component_setmargin(psy_ui_button_base(&self->del), margin);
	psy_ui_button_init_connect(&self->up, &self->top, NULL, self,
		seqeditorheaderdescbar_onmovetrackup);
	psy_ui_button_loadresource(&self->up, IDB_ARROW_UP_DARK,
		IDB_ARROW_UP_DARK, psy_ui_colour_white());	
	psy_ui_button_init_connect(&self->down, &self->top, NULL, self,
		seqeditorheaderdescbar_onmovetrackdown);
	psy_ui_button_loadresource(&self->down, IDB_ARROW_DOWN_DARK,
		IDB_ARROW_DOWN_DARK, psy_ui_colour_white());
	psy_ui_component_setmargin(psy_ui_button_base(&self->down), margin);
	zoombox_init(&self->hzoom, &self->top);
}

void seqeditorheaderdescbar_oninserttrack(SeqEditorHeaderDescBar* self,
	psy_ui_Button* sender)
{	
	sequencecmds_inserttrack(self->state->cmds);	
}

void seqeditorheaderdescbar_ondeletetrack(SeqEditorHeaderDescBar* self,
	psy_ui_Button* sender)
{
	psy_audio_OrderIndex editpos;

	editpos = seqeditstate_editposition(self->state);
	if (editpos.track > 0) {
		sequencecmds_deltrack(self->state->cmds, editpos.track);
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
			cursor.orderindex = editpos;
			workspace_setcursor(self->state->workspace, cursor);
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
			cursor.orderindex = editpos;
			workspace_setcursor(self->state->workspace, cursor);
		}
	}
}
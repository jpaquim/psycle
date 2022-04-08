/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
**  copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "seqviewtrackheaders.h"

/* SeqviewTrackHeaders */

/* prototypes */
static void seqviewtrackheaders_on_destroy(SeqviewTrackHeaders*);
static void seqviewtrackheaders_on_mouse_up(SeqviewTrackHeaders*,
	psy_ui_MouseEvent*);
static void seqviewtrackheaders_onnewtrack(SeqviewTrackHeaders*,
	psy_ui_Button* sender);
static void seqviewtrackheaders_ondeltrack(SeqviewTrackHeaders*,
	TrackBox* sender);

/* vtable */
static psy_ui_ComponentVtable trackheaderviews_vtable;
static bool trackheaderviews_vtable_initialized = FALSE;

static void trackheaderview_vtable_init(SeqviewTrackHeaders* self)
{
	if (!trackheaderviews_vtable_initialized) {
		trackheaderviews_vtable = *(self->component.vtable);
		trackheaderviews_vtable.on_destroy =
			(psy_ui_fp_component_event)
			seqviewtrackheaders_on_destroy;		
		trackheaderviews_vtable.on_mouse_up =
			(psy_ui_fp_component_on_mouse_event)
			seqviewtrackheaders_on_mouse_up;
		trackheaderviews_vtable_initialized = TRUE;
	}
}

/* implemenetation */
void seqviewtrackheaders_init(SeqviewTrackHeaders* self,
	psy_ui_Component* parent, SeqViewState* state)
{
	psy_ui_component_init(&self->component, parent, NULL);
	trackheaderview_vtable_init(self);
	self->component.vtable = &trackheaderviews_vtable;
	self->state = state;	
	psy_ui_component_setminimumsize(&self->component,
		psy_ui_size_make_em(0.0, 2.0));	
	psy_signal_init(&self->signal_trackselected);
	psy_ui_component_init(&self->client, &self->component, NULL);
	psy_ui_component_set_align(&self->client, psy_ui_ALIGN_FIXED);
	psy_ui_component_set_defaultalign(&self->client,
		psy_ui_ALIGN_LEFT, psy_ui_margin_zero());
	psy_ui_component_set_align_expand(&self->client, psy_ui_HEXPAND);
	psy_ui_component_setscrollstep(&self->client,
		psy_ui_size_make(self->state->trackwidth, self->state->line_height));
	psy_ui_component_set_overflow(&self->client, psy_ui_OVERFLOW_HSCROLL);
	seqviewtrackheaders_build(self);
}

void seqviewtrackheaders_on_destroy(SeqviewTrackHeaders* self)
{	
	psy_signal_dispose(&self->signal_trackselected);
}

void seqviewtrackheaders_build(SeqviewTrackHeaders* self)
{
	psy_ui_component_clear(&self->client);		
	if (self->state->cmds->sequence) {
		psy_audio_SequenceTrackNode* t;
		uintptr_t c;
		psy_ui_Button* newtrack;

		for (t = self->state->cmds->sequence->tracks, c = 0; t != NULL;
			psy_list_next(&t), ++c) {
			SequenceTrackBox* sequencetrackbox;
			
			sequencetrackbox = sequencetrackbox_allocinit(&self->client,
				self->state->cmds->sequence, c, self->state->cmds->workspace);
			if (sequencetrackbox) {
				psy_ui_component_setminimumsize(
					sequencetrackbox_base(sequencetrackbox),
					psy_ui_size_make(
						self->state->trackwidth,
						psy_ui_value_zero()));
				psy_signal_connect(&sequencetrackbox->trackbox.signal_close,
					self, seqviewtrackheaders_ondeltrack);
			}
		}
		newtrack = psy_ui_button_allocinit(&self->client);
		if (newtrack) {
			psy_ui_button_set_text(newtrack, "seqview.new-trk");
			newtrack->stoppropagation = FALSE;
			psy_signal_connect(&newtrack->signal_clicked, self,
				seqviewtrackheaders_onnewtrack);
		}
	}
	psy_ui_component_align(&self->client);
	psy_ui_component_align(&self->component);	
}

void seqviewtrackheaders_on_mouse_up(SeqviewTrackHeaders* self,
	psy_ui_MouseEvent* ev)
{	
	if (self->state->cmd == SEQLVCMD_NEWTRACK) {
		sequencecmds_appendtrack(self->state->cmds);		
	} else if (self->state->cmd == SEQLVCMD_DELTRACK) {
		sequencecmds_deltrack(self->state->cmds,
			self->state->cmd_orderindex.track);
	}
	self->state->cmd = SEQLVCMD_NONE;
}

void seqviewtrackheaders_onnewtrack(SeqviewTrackHeaders* self,
	psy_ui_Button* sender)
{
	self->state->cmd = SEQLVCMD_NEWTRACK;
}

void seqviewtrackheaders_ondeltrack(SeqviewTrackHeaders* self,
	TrackBox* sender)
{
	self->state->cmd = SEQLVCMD_DELTRACK;
	self->state->cmd_orderindex.track = trackbox_trackindex(sender);
}

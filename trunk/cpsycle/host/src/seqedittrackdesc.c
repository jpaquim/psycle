/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "seqedittrackdesc.h"
/* host */
#include "sequencetrackbox.h"
#include "styles.h"
/* ui */
#include <uibutton.h>

/* SeqEditTrackDesc */
/* prototypes */
static void seqedittrackdesc_on_destroyed(SeqEditTrackDesc*);
static void seqedittrackdesc_onsequenceselectionselect(SeqEditTrackDesc*,
	psy_audio_SequenceSelection*, psy_audio_OrderIndex*);
static void seqedittrackdesc_onsequenceselectiondeselect(SeqEditTrackDesc*,
	psy_audio_SequenceSelection*, psy_audio_OrderIndex*);
static void seqedittrackdesc_onnewtrack(SeqEditTrackDesc*,
	psy_ui_Button* sender);
static void seqedittrackdesc_ondeltrack(SeqEditTrackDesc*,
	TrackBox* sender);
static void seqedittrackdesc_ondragstart(SeqEditTrackDesc*,
	psy_ui_Component* sender, psy_ui_DragEvent*);
static void seqedittrackdesc_ondragover(SeqEditTrackDesc*,
	psy_ui_DragEvent*);
static void seqedittrackdesc_ondragdrop(SeqEditTrackDesc*,
	psy_ui_DragEvent*);
static void seqedittrackdesc_onresize(SeqEditTrackDesc*,
	psy_ui_Component* sender, double* offset);
/* vtable */
static psy_ui_ComponentVtable seqedittrackdesc_vtable;
static bool seqedittrackdesc_vtable_initialized = FALSE;

static void seqedittrackdesc_vtable_init(SeqEditTrackDesc* self)
{
	if (!seqedittrackdesc_vtable_initialized) {
		seqedittrackdesc_vtable = *(self->component.vtable);		
		seqedittrackdesc_vtable.on_destroyed =
			(psy_ui_fp_component)
			seqedittrackdesc_on_destroyed;		
		seqedittrackdesc_vtable.ondragover =
			(psy_ui_fp_component_on_drag_event)
			seqedittrackdesc_ondragover;
		seqedittrackdesc_vtable.ondrop =
			(psy_ui_fp_component_on_drag_event)
			seqedittrackdesc_ondragdrop;
		seqedittrackdesc_vtable_initialized = TRUE;
	}
	self->component.vtable = &seqedittrackdesc_vtable;
}
/*  implementation */
void seqedittrackdesc_init(SeqEditTrackDesc* self, psy_ui_Component* parent,
	SeqEditState* state, Workspace* workspace)
{	
	psy_ui_component_init(&self->component, parent, NULL);	
	seqedittrackdesc_vtable_init(self);	
	psy_signal_init(&self->signal_resize);
	self->workspace = workspace;
	self->state = state;		
	psy_ui_component_set_default_align(&self->component,
		psy_ui_ALIGN_TOP, psy_ui_margin_zero());	
	seqedittrackdesc_build(self);	
	psy_signal_connect(&workspace->song->sequence.selection.signal_select, self,
		seqedittrackdesc_onsequenceselectionselect);
	psy_signal_connect(&workspace->song->sequence.selection.signal_deselect, self,
		seqedittrackdesc_onsequenceselectiondeselect);	
}

void seqedittrackdesc_on_destroyed(SeqEditTrackDesc* self)
{
	psy_signal_dispose(&self->signal_resize);
}

void seqedittrackdesc_onsequenceselectionselect(SeqEditTrackDesc* self,
	psy_audio_SequenceSelection* selection,
	psy_audio_OrderIndex* index)
{
	psy_ui_Component* track;

	track = psy_ui_component_at(&self->component, index->track);
	if (track) {
		psy_ui_Component* trackbox;
		
		trackbox = psy_ui_component_at(track, 0);
		if (trackbox) {
			psy_ui_component_add_style_state(trackbox, psy_ui_STYLESTATE_SELECT);
		}
	}
}

void seqedittrackdesc_onsequenceselectiondeselect(SeqEditTrackDesc* self,
	psy_audio_SequenceSelection* selection,
	psy_audio_OrderIndex* index)
{
	psy_ui_Component* track;

	track = psy_ui_component_at(&self->component, index->track);
	if (track) {
		psy_ui_Component* trackbox;

		trackbox = psy_ui_component_at(track, 0);
		if (trackbox) {
			psy_ui_component_remove_style_state(trackbox,
				psy_ui_STYLESTATE_SELECT);
		}
	}
}

void seqedittrackdesc_onnewtrack(SeqEditTrackDesc* self,
	psy_ui_Button* sender)
{
	self->state->cmd = SEQEDTCMD_NEWTRACK;
}

void seqedittrackdesc_ondeltrack(SeqEditTrackDesc* self,
	TrackBox* sender)
{
	self->state->cmd = SEQEDTCMD_DELTRACK;
	self->state->dragseqpos.track = trackbox_trackindex(sender);
}

void seqedittrackdesc_build(SeqEditTrackDesc* self)
{
	psy_audio_Sequence* sequence;

	psy_ui_component_clear(&self->component);	
	sequence = seqeditstate_sequence(self->state);
	if (sequence) {
		uintptr_t t;		
		psy_ui_Button* newtrack;
		psy_audio_OrderIndex editposition;

		editposition = psy_audio_sequenceselection_first(
			&self->state->cmds->workspace->song->sequence.selection);
		for (t = 0; t < psy_audio_sequence_width(sequence); ++t) {
			SequenceTrackBox* trackbox;
			psy_audio_SequenceTrack* track;
			
			track = psy_audio_sequence_track_at(sequence, t);			
			trackbox = sequencetrackbox_allocinit(&self->component,
				seqeditstate_sequence(self->state), t, self->workspace);
			if (trackbox) {				
				psy_ui_component_show(&trackbox->trackbox.resize);
				psy_ui_component_set_minimum_size(
					sequencetrackbox_base(trackbox),
					psy_ui_size_make_em(0.0, 2.0));
				if (track && track->height != 0.0) {
					psy_ui_component_set_preferred_height(&trackbox->component,
						psy_ui_value_make_eh(track->height));
				}
				sequencetrackbox_showtrackname(trackbox);
			}
			if (trackbox) {				
				/* psy_ui_component_set_style_type_hover(
					&trackbox->trackbox.component,
					STYLE_SEQEDT_DESCITEM_HOVER); */
				psy_ui_component_set_style_type_select(
					&trackbox->trackbox.component,
					STYLE_SEQEDT_DESCITEM_SELECTED);
				psy_ui_component_set_style_type(
					&trackbox->trackbox.track.component,
					STYLE_SEQEDT_TRACK_NUMBER);				
				psy_signal_connect(&trackbox->trackbox.signal_close, self,
					seqedittrackdesc_ondeltrack);				
				trackbox->trackbox.track.component.draggable = TRUE;
				psy_signal_connect(
					&trackbox->trackbox.track.component.signal_dragstart,
					self, seqedittrackdesc_ondragstart);
				if (t == editposition.track) {					
					psy_ui_component_add_style_state(
						&trackbox->trackbox.component,
						psy_ui_STYLESTATE_SELECT);
				}
				psy_signal_connect(&trackbox->signal_resize, self,
					seqedittrackdesc_onresize);
			}
		}
		newtrack = psy_ui_button_allocinit(&self->component);
		if (newtrack) {
			psy_ui_button_set_text(newtrack, "seqview.new-trk");
			psy_ui_component_set_minimum_size(psy_ui_button_base(newtrack),
				psy_ui_size_make_em(0.0, 2.0));
			newtrack->stoppropagation = FALSE;
			psy_signal_connect(&newtrack->signal_clicked, self,
				seqedittrackdesc_onnewtrack);
		}		
	}
	psy_ui_component_align(&self->component);
}

void seqedittrackdesc_onresize(SeqEditTrackDesc* self,
	psy_ui_Component* sender, double* offset)
{		
	psy_ui_RealSize size;
	double height;
	const psy_ui_TextMetric* tm;

	size = psy_ui_component_size_px(sender);
	tm = psy_ui_component_textmetric(&self->component);
	height = (size.height + *offset) / tm->tmHeight;	
	sequencecmds_update(self->state->cmds);
	psy_audio_sequence_settrackheight(self->state->cmds->sequence,
		(uintptr_t)sender->id, height);
	psy_ui_component_set_preferred_height(sender,
		psy_ui_value_make_eh(height));	
	psy_signal_emit(&self->signal_resize, self, 2, (void*)sender->id,
		&height);
}

void seqedittrackdesc_ondragstart(SeqEditTrackDesc* self,
	psy_ui_Component* sender, psy_ui_DragEvent* ev)
{
	ev->dataTransfer = psy_property_allocinit_key(NULL);	
	psy_property_append_int(ev->dataTransfer,
		"seqedittrack", sender->id, 0, 0);
	psy_ui_dragevent_prevent_default(ev);
}

void seqedittrackdesc_ondragover(SeqEditTrackDesc* self, psy_ui_DragEvent* ev)
{
	psy_ui_dragevent_prevent_default(ev);
}

void seqedittrackdesc_ondragdrop(SeqEditTrackDesc* self, psy_ui_DragEvent* ev)
{
	psy_ui_dragevent_prevent_default(ev);
	if (ev->dataTransfer && self->state->workspace->song) {
		intptr_t trackid;

		trackid = psy_property_at_int(ev->dataTransfer, "seqedittrack", -1);
		if (trackid != -1) {
			uintptr_t index;

			psy_ui_component_intersect(&self->component, psy_ui_mouseevent_pt(&ev->mouse), &index);
			if (index != trackid) {
				psy_audio_sequence_swaptracks(
					&self->state->workspace->song->sequence, trackid, index);
			}
		}
	}
}

/*
** This source is free software; you can redistribute itand /or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "sequencetrackbox.h"
/* ui */
#include <uiapp.h>


/* prototypes */
static void sequencetrackbox_on_destroyed(SequenceTrackBox*);
static void sequencetrackbox_on_solo_track(SequenceTrackBox*, TrackBox* sender);
static void sequencetrackbox_on_mutetrack(SequenceTrackBox*, TrackBox* sender);
static void sequencetrackbox_on_solochanged(SequenceTrackBox*,
	psy_audio_Sequence* sender, uintptr_t track);
static void sequencetrackbox_on_mutechanged(SequenceTrackBox*,
	psy_audio_Sequence* sender, uintptr_t track);
static void sequencetrackbox_on_label_click(SequenceTrackBox*, psy_ui_Label* sender,
	psy_ui_MouseEvent*);
static void sequencetrackbox_on_resize(SequenceTrackBox*, psy_ui_Component* sender,
	double* offset);

/* vtable */
static psy_ui_ComponentVtable sequencetrackbox_vtable;
static bool sequencetrackbox_vtable_initialized = FALSE;

static void sequencetrackbox_vtable_init(SequenceTrackBox* self)
{
	if (!sequencetrackbox_vtable_initialized) {
		sequencetrackbox_vtable = *(sequencetrackbox_base(self)->vtable);		
		sequencetrackbox_vtable.on_destroyed =
			(psy_ui_fp_component_event)
			sequencetrackbox_on_destroyed;		
		sequencetrackbox_vtable_initialized = TRUE;
	}
	psy_ui_component_set_vtable(sequencetrackbox_base(self),
		&sequencetrackbox_vtable);
}

/* implementation */
void sequencetrackbox_init(SequenceTrackBox* self, psy_ui_Component* parent,
	psy_audio_Sequence* sequence,
	uintptr_t trackidx, Workspace* workspace)
{
	assert(self);

	psy_ui_component_init(&self->component, parent, NULL);	
	psy_signal_init(&self->signal_resize);
	trackbox_init(&self->trackbox, &self->component);	
	psy_ui_component_set_align(&self->trackbox.component,
		psy_ui_ALIGN_CLIENT);	
	sequencetrackbox_vtable_init(self);
	psy_signal_connect(&self->trackbox.signal_resize, self,
		sequencetrackbox_on_resize);
	self->sequence = sequence;
	self->trackidx = trackidx;
	self->preventedit = TRUE;
	self->workspace = workspace;
	self->component.id = (intptr_t)self->trackidx;
	self->trackbox.track.component.id = (intptr_t)self->trackidx;
	trackbox_set_index(&self->trackbox, trackidx);	
	if (self->sequence) {
		psy_signal_connect(&self->sequence->signal_solochanged, self,
			sequencetrackbox_on_solochanged);
		psy_signal_connect(&self->sequence->signal_mutechanged, self,
			sequencetrackbox_on_mutechanged);
	}
	psy_signal_connect(&self->trackbox.signal_solo, self,
		sequencetrackbox_on_solo_track);
	psy_signal_connect(&self->trackbox.signal_mute, self,
		sequencetrackbox_on_mutetrack);	
	psy_signal_connect(&self->trackbox.desc.component.signal_mousedown, self,
		sequencetrackbox_on_label_click);	
}

void sequencetrackbox_on_destroyed(SequenceTrackBox* self)
{
	assert(self);

	if (self->sequence) {
		psy_signal_disconnect(&self->sequence->signal_solochanged, self,
			sequencetrackbox_on_solochanged);
		psy_signal_disconnect(&self->sequence->signal_mutechanged, self,
			sequencetrackbox_on_mutechanged);
	}
	psy_signal_dispose(&self->signal_resize);	
}

SequenceTrackBox* sequencetrackbox_alloc(void)
{
	return (SequenceTrackBox*)malloc(sizeof(SequenceTrackBox));
}

SequenceTrackBox* sequencetrackbox_allocinit(psy_ui_Component* parent,
	psy_audio_Sequence* sequence, uintptr_t trackidx, Workspace* workspace)
{
	SequenceTrackBox* rv;	

	rv = sequencetrackbox_alloc();
	if (rv) {
		sequencetrackbox_init(rv, parent, sequence, trackidx, workspace);
		psy_ui_component_deallocate_after_destroyed(sequencetrackbox_base(rv));
	}
	return rv;
}

void sequencetrackbox_on_solo_track(SequenceTrackBox* self, TrackBox* sender)
{
	assert(self);

	if (self->sequence) {		
		if (psy_audio_sequence_istracksoloed(self->sequence, self->trackidx)) {
			psy_audio_sequence_deactivatesolotrack(self->sequence);
		} else {
			psy_audio_sequence_activatesolotrack(self->sequence, self->trackidx);
		}
	}
}

void sequencetrackbox_on_mutetrack(SequenceTrackBox* self, TrackBox* sender)
{
	assert(self);

	if (self->sequence) {				
		if (psy_audio_sequence_istrackmuted(self->sequence, self->trackidx)) {
			psy_audio_sequence_unmutetrack(self->sequence, self->trackidx);
		} else {
			psy_audio_sequence_mutetrack(self->sequence, self->trackidx);
		}
	}
}

void sequencetrackbox_on_solochanged(SequenceTrackBox* self,
	psy_audio_Sequence* sender, uintptr_t trackidx)
{	
	assert(self);

	trackbox_unsolo(&self->trackbox);
	if (self->trackidx == trackidx) {
		if (psy_audio_sequence_istracksoloed(sender, trackidx)) {
			trackbox_solo(&self->trackbox);
		}
	}
	if (psy_audio_sequence_istrackmuted(sender, trackidx)) {
		trackbox_mute(&self->trackbox);
	} else {
		trackbox_unmute(&self->trackbox);
	}
}

void sequencetrackbox_on_mutechanged(SequenceTrackBox* self,
	psy_audio_Sequence* sender, uintptr_t trackidx)
{
	assert(self);

	if (self->trackidx == trackidx) {
		if (psy_audio_sequence_istrackmuted(sender, trackidx)) {
			trackbox_mute(&self->trackbox);
		} else {
			trackbox_unmute(&self->trackbox);
		}
	}
}

void sequencetrackbox_showtrackname(SequenceTrackBox* self)
{
	assert(self);

	if (self->sequence) {
		psy_audio_SequenceTrack* track;

		track = psy_audio_sequence_track_at(self->sequence, self->trackidx);
		if (track) {
			trackbox_set_description(&self->trackbox, track->name);
		}
	}
}

void sequencetrackbox_on_label_click(SequenceTrackBox* self, psy_ui_Label* sender,
	psy_ui_MouseEvent* ev)
{
	psy_audio_OrderIndex editposition;	
	psy_audio_SequenceCursor cursor;

	editposition = psy_audio_sequenceselection_first(&self->workspace->song->sequence.selection);
	editposition.track = self->trackidx;	
	psy_audio_sequencecursor_init(&cursor);	
	psy_audio_sequencecursor_set_order_index(&cursor, editposition);	
	if (self->workspace && workspace_song(self->workspace)) {
		psy_audio_sequence_set_cursor(
			psy_audio_song_sequence(workspace_song(self->workspace)),
			cursor);
	}	
}

void sequencetrackbox_on_resize(SequenceTrackBox* self, psy_ui_Component* sender,
	double* offset)
{	
	psy_signal_emit(&self->signal_resize, self, 1, (void*)offset);
}

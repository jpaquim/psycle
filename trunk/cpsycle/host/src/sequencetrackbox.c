// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "sequencetrackbox.h"

// SequenceTrackBox
// prototypes
static void sequencetrackbox_ondestroy(SequenceTrackBox*);
static void sequencetrackbox_onsolotrack(SequenceTrackBox*,
	psy_ui_Button* sender);
static void sequencetrackbox_onmutetrack(SequenceTrackBox*,
	psy_ui_Button* sender);
static void sequencetrackbox_onsolochanged(SequenceTrackBox*,
	psy_audio_Sequence* sender, uintptr_t track);
static void sequencetrackbox_onmutechanged(SequenceTrackBox*,
	psy_audio_Sequence* sender, uintptr_t track);
// vtable
static psy_ui_ComponentVtable sequencetrackbox_vtable;
static bool sequencetrackbox_vtable_initialized = FALSE;

static void sequencetrackbox_vtable_init(SequenceTrackBox* self)
{
	if (!sequencetrackbox_vtable_initialized) {
		sequencetrackbox_vtable = *(sequencetrackbox_base(self)->vtable);
		sequencetrackbox_vtable.ondestroy =
			(psy_ui_fp_component_ondestroy)
			sequencetrackbox_ondestroy;		
		sequencetrackbox_vtable_initialized = TRUE;
	}
	psy_ui_component_setvtable(sequencetrackbox_base(self),
		&sequencetrackbox_vtable);

}

// implementation
void sequencetrackbox_init(SequenceTrackBox* self, psy_ui_Component* parent,
	psy_ui_Component* view, psy_audio_Sequence* sequence,
	uintptr_t trackidx)
{	
	trackbox_init(&self->trackbox, parent, view);
	sequencetrackbox_vtable_init(self);
	self->sequence = sequence;
	self->trackidx = trackidx;
	trackbox_setindex(&self->trackbox, trackidx);	
	if (self->sequence) {
		psy_signal_connect(&self->sequence->signal_solochanged, self,
			sequencetrackbox_onsolochanged);
		psy_signal_connect(&self->sequence->signal_mutechanged, self,
			sequencetrackbox_onmutechanged);
	}
	psy_signal_connect(&self->trackbox.solo.signal_clicked, self,
		sequencetrackbox_onsolotrack);
	psy_signal_connect(&self->trackbox.mute.signal_clicked, self,
		sequencetrackbox_onmutetrack);
}

void sequencetrackbox_ondestroy(SequenceTrackBox* self)
{
	if (self->sequence) {
		psy_signal_disconnect(&self->sequence->signal_solochanged, self,
			sequencetrackbox_onsolochanged);
		psy_signal_disconnect(&self->sequence->signal_mutechanged, self,
			sequencetrackbox_onmutechanged);
	}
}

SequenceTrackBox* sequencetrackbox_alloc(void)
{
	return (SequenceTrackBox*)malloc(sizeof(SequenceTrackBox));
}

SequenceTrackBox* sequencetrackbox_allocinit(psy_ui_Component* parent,
	psy_ui_Component* view, psy_audio_Sequence* sequence,
	uintptr_t trackidx)
{
	SequenceTrackBox* rv;

	rv = sequencetrackbox_alloc();
	if (rv) {
		sequencetrackbox_init(rv, parent, view, sequence, trackidx);
		psy_ui_component_deallocateafterdestroyed(sequencetrackbox_base(rv));
	}
	return rv;
}

void sequencetrackbox_onsolotrack(SequenceTrackBox* self,
	psy_ui_Button* sender)
{
	if (self->sequence) {		
		if (psy_audio_sequence_istracksoloed(self->sequence, self->trackidx)) {
			psy_audio_sequence_deactivatesolotrack(self->sequence);
		} else {
			psy_audio_sequence_activatesolotrack(self->sequence, self->trackidx);
		}
	}
}

void sequencetrackbox_onmutetrack(SequenceTrackBox* self,
	psy_ui_Button* sender)
{
	if (self->sequence) {				
		if (psy_audio_sequence_istrackmuted(self->sequence, self->trackidx)) {
			psy_audio_sequence_unmutetrack(self->sequence, self->trackidx);
		} else {
			psy_audio_sequence_mutetrack(self->sequence, self->trackidx);
		}
	}
}

void sequencetrackbox_onsolochanged(SequenceTrackBox* self,
	psy_audio_Sequence* sender, uintptr_t trackidx)
{	
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

void sequencetrackbox_onmutechanged(SequenceTrackBox* self,
	psy_audio_Sequence* sender, uintptr_t trackidx)
{
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
	if (self->sequence) {
		psy_audio_SequenceTrack* track;

		track = psy_audio_sequence_track_at(self->sequence, self->trackidx);
		if (track) {
			trackbox_setdescription(&self->trackbox, track->name);
		}
	}
}
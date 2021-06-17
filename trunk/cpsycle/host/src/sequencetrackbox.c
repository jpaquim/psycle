// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "sequencetrackbox.h"
#include "uiapp.h"

// SequenceTrackBox
// prototypes
static void sequencetrackbox_ondestroy(SequenceTrackBox*);
static void sequencetrackbox_onsolotrack(SequenceTrackBox*, TrackBox* sender);
static void sequencetrackbox_onmutetrack(SequenceTrackBox*, TrackBox* sender);
static void sequencetrackbox_onsolochanged(SequenceTrackBox*,
	psy_audio_Sequence* sender, uintptr_t track);
static void sequencetrackbox_onmutechanged(SequenceTrackBox*,
	psy_audio_Sequence* sender, uintptr_t track);
static void sequencetrackbox_onlabelclick(SequenceTrackBox*, psy_ui_Label* sender,
	psy_ui_MouseEvent*);
static void sequencetrackbox_onresize(SequenceTrackBox*, psy_ui_Component* sender,
	double* offset);
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

/* implementation */
void sequencetrackbox_init(SequenceTrackBox* self, psy_ui_Component* parent,
	psy_ui_Component* view, psy_audio_Sequence* sequence,
	uintptr_t trackidx, Workspace* workspace)
{
	assert(self);

	psy_ui_component_init(&self->component, parent, view);	
	psy_signal_init(&self->signal_resize);
	trackbox_init(&self->trackbox, &self->component, view);	
	psy_ui_component_setalign(&self->trackbox.component,
		psy_ui_ALIGN_CLIENT);	
	sequencetrackbox_vtable_init(self);
	psy_signal_connect(&self->trackbox.signal_resize, self,
		sequencetrackbox_onresize);
	self->sequence = sequence;
	self->trackidx = trackidx;
	self->preventedit = TRUE;
	self->workspace = workspace;
	self->component.id = (intptr_t)self->trackidx;
	self->trackbox.track.component.id = (intptr_t)self->trackidx;
	trackbox_setindex(&self->trackbox, trackidx);	
	if (self->sequence) {
		psy_signal_connect(&self->sequence->signal_solochanged, self,
			sequencetrackbox_onsolochanged);
		psy_signal_connect(&self->sequence->signal_mutechanged, self,
			sequencetrackbox_onmutechanged);
	}
	psy_signal_connect(&self->trackbox.signal_solo, self,
		sequencetrackbox_onsolotrack);
	psy_signal_connect(&self->trackbox.signal_mute, self,
		sequencetrackbox_onmutetrack);	
	psy_signal_connect(&self->trackbox.desc.component.signal_mousedown, self,
		sequencetrackbox_onlabelclick);	
}

void sequencetrackbox_ondestroy(SequenceTrackBox* self)
{
	assert(self);

	if (self->sequence) {
		psy_signal_disconnect(&self->sequence->signal_solochanged, self,
			sequencetrackbox_onsolochanged);
		psy_signal_disconnect(&self->sequence->signal_mutechanged, self,
			sequencetrackbox_onmutechanged);
	}
	psy_signal_dispose(&self->signal_resize);	
}

SequenceTrackBox* sequencetrackbox_alloc(void)
{
	return (SequenceTrackBox*)malloc(sizeof(SequenceTrackBox));
}

SequenceTrackBox* sequencetrackbox_allocinit(psy_ui_Component* parent,
	psy_ui_Component* view, psy_audio_Sequence* sequence,
	uintptr_t trackidx, Workspace* workspace)
{
	SequenceTrackBox* rv;	

	rv = sequencetrackbox_alloc();
	if (rv) {
		sequencetrackbox_init(rv, parent, view, sequence, trackidx, workspace);
		psy_ui_component_deallocateafterdestroyed(sequencetrackbox_base(rv));
	}
	return rv;
}

void sequencetrackbox_onsolotrack(SequenceTrackBox* self, TrackBox* sender)
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

void sequencetrackbox_onmutetrack(SequenceTrackBox* self, TrackBox* sender)
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

void sequencetrackbox_onsolochanged(SequenceTrackBox* self,
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

void sequencetrackbox_onmutechanged(SequenceTrackBox* self,
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
			trackbox_setdescription(&self->trackbox, track->name);
		}
	}
}

void sequencetrackbox_onlabelclick(SequenceTrackBox* self, psy_ui_Label* sender,
	psy_ui_MouseEvent* ev)
{
	psy_audio_OrderIndex editposition;	
	
	editposition = self->workspace->sequenceselection.editposition;
	editposition.track = self->trackidx;
	workspace_setseqeditposition(self->workspace, editposition);
}

void sequencetrackbox_onresize(SequenceTrackBox* self, psy_ui_Component* sender,
	double* offset)
{	
	psy_signal_emit(&self->signal_resize, self, 1, (void*)offset);
}

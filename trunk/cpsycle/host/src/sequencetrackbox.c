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
static void sequencetrackbox_editname(SequenceTrackBox*, psy_ui_Edit*);
static void sequencetrackbox_updatetrackname(SequenceTrackBox*);
static void sequencetrackbox_oneditchange(SequenceTrackBox*, psy_ui_Edit* sender);
static void sequencetrackbox_oneditfocuslost(SequenceTrackBox*, psy_ui_Component* sender);
static void sequencetrackbox_oneditkeydown(SequenceTrackBox*, psy_ui_Component* sender,
	psy_ui_KeyEvent*);
static void sequencetrackbox_onmousehook(SequenceTrackBox*, psy_ui_App* sender,
	psy_ui_MouseEvent*);
// vtable
static psy_ui_ComponentVtable sequencetrackbox_vtable;
static psy_ui_ComponentVtable sequencetrackbox_super_vtable;
static bool sequencetrackbox_vtable_initialized = FALSE;

static void sequencetrackbox_vtable_init(SequenceTrackBox* self)
{
	if (!sequencetrackbox_vtable_initialized) {
		sequencetrackbox_vtable = *(sequencetrackbox_base(self)->vtable);
		sequencetrackbox_super_vtable = sequencetrackbox_vtable;
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
	uintptr_t trackidx, psy_ui_Edit* edit)
{
	assert(self);

	trackbox_init(&self->trackbox, parent, view);
	sequencetrackbox_vtable_init(self);
	self->sequence = sequence;
	self->trackidx = trackidx;
	self->edit = edit;
	self->preventedit = TRUE;
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
	if (self->edit) {
		psy_signal_connect(&self->trackbox.desc.component.signal_mousedown,
		self, sequencetrackbox_onlabelclick);	
		psy_signal_connect(&psy_ui_app()->signal_mousehook, self,
			sequencetrackbox_onmousehook);
	}
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
	psy_signal_disconnect(&psy_ui_app()->signal_mousehook, self,
		sequencetrackbox_onmousehook);
	sequencetrackbox_super_vtable.ondestroy(&self->trackbox.component);
}

SequenceTrackBox* sequencetrackbox_alloc(void)
{
	return (SequenceTrackBox*)malloc(sizeof(SequenceTrackBox));
}

SequenceTrackBox* sequencetrackbox_allocinit(psy_ui_Component* parent,
	psy_ui_Component* view, psy_audio_Sequence* sequence,
	uintptr_t trackidx, psy_ui_Edit* edit)
{
	SequenceTrackBox* rv;	

	rv = sequencetrackbox_alloc();
	if (rv) {
		sequencetrackbox_init(rv, parent, view, sequence, trackidx, edit);
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
	if (self->edit) {
		psy_ui_RealRectangle position;
		psy_ui_RealRectangle labelposition;
		psy_ui_RealSize size;
		const psy_ui_TextMetric* tm;
		double centery;

		position = psy_ui_component_position(&self->trackbox.component);
		size = psy_ui_component_sizepx(&self->trackbox.component);
		labelposition = psy_ui_component_position(&self->trackbox.desc.component);
		tm = psy_ui_component_textmetric(&self->trackbox.component);
		centery = (size.height - tm->tmHeight) / 2;
		position.top += centery;
		position.left = labelposition.left;
		position.right = labelposition.right;
		psy_ui_component_setposition(&self->edit->component,
			psy_ui_rectangle_make_px(&position));
		psy_ui_edit_settext(self->edit, self->trackbox.desc.text);
		sequencetrackbox_editname(self, self->edit);
		psy_ui_component_show(&self->edit->component);
		psy_ui_component_setfocus(&self->edit->component);

	}	
}

void sequencetrackbox_editname(SequenceTrackBox* self, psy_ui_Edit* edit)
{
	assert(self);
			
	psy_signal_disconnectall(&edit->component.signal_focuslost);
	psy_signal_disconnectall(&edit->component.signal_keydown);
	psy_signal_disconnectall(&edit->signal_change);
	psy_signal_connect(&edit->signal_change, self, sequencetrackbox_oneditchange);
	psy_signal_connect(&edit->component.signal_keydown, self,
		sequencetrackbox_oneditkeydown);
	psy_signal_connect(&edit->component.signal_focuslost, self,
		sequencetrackbox_oneditfocuslost);		
	self->preventedit = FALSE;	
	psy_ui_edit_setsel(self->edit, 0, -1);
}

void sequencetrackbox_oneditkeydown(SequenceTrackBox* self, psy_ui_Component* sender,
	psy_ui_KeyEvent* ev)
{
	assert(self);

	switch (ev->keycode) {
	case psy_ui_KEY_ESCAPE:			
		self->preventedit = TRUE;
		psy_ui_component_hide(sender);
		psy_ui_keyevent_preventdefault(ev);
		sequencetrackbox_showtrackname(self);
		break;
	case psy_ui_KEY_RETURN:
		sequencetrackbox_updatetrackname(self);		
		psy_ui_component_hide(sender);
		psy_ui_keyevent_preventdefault(ev);
		psy_ui_component_invalidate(&self->trackbox.component);
		break;
	default:
		break;
	}
	psy_ui_keyevent_stoppropagation(ev);
}


void sequencetrackbox_oneditchange(SequenceTrackBox* self, psy_ui_Edit* sender)
{	
}

void sequencetrackbox_oneditfocuslost(SequenceTrackBox* self, psy_ui_Component* sender)
{
	assert(self);

	if (!self->preventedit) {
		sequencetrackbox_updatetrackname(self);
	}
	psy_ui_component_hide(sender);
	psy_ui_component_invalidate(&self->trackbox.component);
	self->preventedit = TRUE;
}

void sequencetrackbox_onmousehook(SequenceTrackBox* self, psy_ui_App* sender,
	psy_ui_MouseEvent* ev)
{
	if (psy_ui_component_visible(&self->edit->component)) {
		psy_ui_RealRectangle position;
		
		position = psy_ui_component_screenposition(&self->edit->component);
		if (!psy_ui_realrectangle_intersect(&position, ev->pt)) {
			if (!self->preventedit) {
				sequencetrackbox_updatetrackname(self);
			}
			psy_ui_component_hide(&self->edit->component);
				psy_ui_component_invalidate(&self->trackbox.component);
				self->preventedit = TRUE;
		}
	}	
}

void sequencetrackbox_updatetrackname(SequenceTrackBox* self)
{
	if (self->sequence) {
		psy_audio_SequenceTrack* track;

		track = psy_audio_sequence_track_at(self->sequence, self->trackidx);
		if (track) {
			psy_audio_sequencetrack_setname(track, psy_ui_edit_text(self->edit));
			sequencetrackbox_showtrackname(self);
		}
	}
}

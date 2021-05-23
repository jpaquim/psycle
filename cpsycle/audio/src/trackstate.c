// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "trackstate.h"

void psy_audio_trackstate_init(psy_audio_TrackState* self)
{	
	psy_signal_init(&self->signal_changed);
	psy_table_init(&self->mute);
	psy_table_init(&self->record);
	self->soloactive = 0;	
	self->soloedtrack = 0;
}

void psy_audio_trackstate_dispose(psy_audio_TrackState* self)
{
	psy_signal_dispose(&self->signal_changed);
	psy_table_dispose(&self->mute);
	psy_table_dispose(&self->record);
	self->soloactive = 0;	
	self->soloedtrack = 0;
}

void psy_audio_trackstate_copy(psy_audio_TrackState* self, const psy_audio_TrackState* other)
{
	psy_TableIterator it;

	assert(self);

	psy_table_clear(&self->mute);
	for (it = psy_table_begin((psy_Table*)(&other->mute));
			!psy_tableiterator_equal(&it, psy_table_end());
			psy_tableiterator_inc(&it)) {
		uintptr_t set;

		set = (uintptr_t)psy_tableiterator_value(&it);				
		psy_table_insert(&self->mute, psy_tableiterator_key(&it), (void*)set);
	}
	psy_table_clear(&self->record);
	for (it = psy_table_begin((psy_Table*)(&other->record));
		!psy_tableiterator_equal(&it, psy_table_end());
		psy_tableiterator_inc(&it)) {
		uintptr_t set;

		set = (uintptr_t)psy_tableiterator_value(&it);
		psy_table_insert(&self->record, psy_tableiterator_key(&it), (void*)set);
	}
	self->soloactive = other->soloactive;
	self->soloedtrack = other->soloedtrack;
}

void psy_audio_trackstate_activatesolotrack(psy_audio_TrackState* self, uintptr_t track)
{
	psy_audio_trackstate_setsolotrack(self, track);
	psy_table_clear(&self->mute);
	psy_table_insert(&self->mute, track, (void*)(uintptr_t) 1);
	psy_signal_emit(&self->signal_changed, self, 0);
}

void psy_audio_trackstate_setsolotrack(psy_audio_TrackState* self, uintptr_t track)
{
	self->soloactive = TRUE;
	self->soloedtrack = track;	
}

void psy_audio_trackstate_deactivatesolotrack(psy_audio_TrackState* self)
{
	self->soloactive = FALSE;
	psy_table_clear(&self->mute);
	psy_signal_emit(&self->signal_changed, self, 0);
}

void psy_audio_trackstate_mutetrack(psy_audio_TrackState* self, uintptr_t track)
{
	if (!self->soloactive) {
		psy_table_insert(&self->mute, track, (void*)(uintptr_t) 1);
	} else {
		psy_table_remove(&self->mute, track);
	}
	psy_signal_emit(&self->signal_changed, self, 0);
}

void psy_audio_trackstate_unmutetrack(psy_audio_TrackState* self, uintptr_t track)
{
	if (!self->soloactive) {
		psy_table_remove(&self->mute, track);
	} else {
		psy_table_insert(&self->mute, track, (void*)(uintptr_t) 1);
	}
	psy_signal_emit(&self->signal_changed, self, 0);
}

bool psy_audio_trackstate_istrackmuted(const psy_audio_TrackState* self, uintptr_t track)
{
	return (self->soloactive)
		? !psy_table_exists(&self->mute, track)
		: psy_table_exists(&self->mute, track);
}

bool psy_audio_trackstate_istracksoloed(const psy_audio_TrackState* self, uintptr_t track)
{
	return self->soloactive && self->soloedtrack == track;
}

uintptr_t psy_audio_trackstate_tracksoloed(const psy_audio_TrackState* self)
{
	if (self->soloactive) {
		return self->soloedtrack;
	}
	return psy_INDEX_INVALID;
}

void psy_audio_trackstate_armtrack(psy_audio_TrackState* self, uintptr_t track)
{	
	psy_table_insert(&self->record, track, (void*)(uintptr_t)TRUE);
	psy_signal_emit(&self->signal_changed, self, 0);
}

void psy_audio_trackstate_unarmtrack(psy_audio_TrackState* self, uintptr_t track)
{	
	psy_table_remove(&self->record, track);
	psy_signal_emit(&self->signal_changed, self, 0);
}

bool psy_audio_trackstate_istrackarmed(const psy_audio_TrackState* self, uintptr_t track)
{
	return psy_table_exists(&self->record, track);
}

uintptr_t psy_audio_trackstate_trackarmedcount(const psy_audio_TrackState* self)
{
	return psy_table_size(&self->record);
}

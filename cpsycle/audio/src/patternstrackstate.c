// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "patternstrackstate.h"

void psy_audio_trackstate_init(psy_audio_TrackState* self)
{	
	psy_table_init(&self->mute);
	psy_table_init(&self->record);
	self->soloactive = 0;	
	self->soloedtrack = 0;
}

void psy_audio_trackstate_dispose(psy_audio_TrackState* self)
{
	psy_table_dispose(&self->mute);
	psy_table_dispose(&self->record);
	self->soloactive = 0;	
	self->soloedtrack = 0;
}

void psy_audio_trackstate_activatesolotrack(psy_audio_TrackState* self, uintptr_t track)
{
	self->soloactive = 1;
	self->soloedtrack = track;
	psy_table_clear(&self->mute);
	psy_table_insert(&self->mute, track, (void*)(uintptr_t) 1);
}

void psy_audio_trackstate_deactivatesolotrack(psy_audio_TrackState* self)
{
	self->soloactive = 0;
	psy_table_clear(&self->mute);
}

void psy_audio_trackstate_mutetrack(psy_audio_TrackState* self, uintptr_t track)
{
	if (!self->soloactive) {
		psy_table_insert(&self->mute, track, (void*)(uintptr_t) 1);
	} else {
		psy_table_remove(&self->mute, track);
	}
}

void psy_audio_trackstate_unmutetrack(psy_audio_TrackState* self, uintptr_t track)
{
	if (!self->soloactive) {
		psy_table_remove(&self->mute, track);
	} else {
		psy_table_insert(&self->mute, track, (void*)(uintptr_t) 1);
	}
}

int psy_audio_trackstate_istrackmuted(const psy_audio_TrackState* self, uintptr_t track)
{
	return self->soloactive
		? !psy_table_exists(&self->mute, track)
		: psy_table_exists(&self->mute, track);
}

int psy_audio_trackstate_istracksoloed(const psy_audio_TrackState* self, uintptr_t track)
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

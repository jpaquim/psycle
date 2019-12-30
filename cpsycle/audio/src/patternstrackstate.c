// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "patternstrackstate.h"

void patternstrackstate_init(psy_audio_PatternsTrackState* self)
{	
	psy_table_init(&self->mute);
	psy_table_init(&self->record);
	self->soloactive = 0;	
	self->soloedtrack = 0;
}

void patternstrackstate_dispose(psy_audio_PatternsTrackState* self)
{
	psy_table_dispose(&self->mute);
	psy_table_dispose(&self->record);
	self->soloactive = 0;	
	self->soloedtrack = 0;
}

void patternstrackstate_activatesolotrack(psy_audio_PatternsTrackState* self, uintptr_t track)
{
	self->soloactive = 1;
	self->soloedtrack = track;
	psy_table_clear(&self->mute);
	psy_table_insert(&self->mute, track, (void*)(uintptr_t) 1);
}

void patternstrackstate_deactivatesolotrack(psy_audio_PatternsTrackState* self)
{
	self->soloactive = 0;
	psy_table_clear(&self->mute);
}

void patternstrackstate_mutetrack(psy_audio_PatternsTrackState* self, uintptr_t track)
{
	if (!self->soloactive) {
		psy_table_insert(&self->mute, track, (void*)(uintptr_t) 1);
	} else {
		psy_table_remove(&self->mute, track);
	}
}

void patternstrackstate_unmutetrack(psy_audio_PatternsTrackState* self, uintptr_t track)
{
	if (!self->soloactive) {
		psy_table_remove(&self->mute, track);
	} else {
		psy_table_insert(&self->mute, track, (void*)(uintptr_t) 1);
	}
}

int patternstrackstate_istrackmuted(psy_audio_PatternsTrackState* self, uintptr_t track)
{
	return self->soloactive
		? !psy_table_exists(&self->mute, track)
		: psy_table_exists(&self->mute, track);
}

int patternstrackstate_istracksoloed(psy_audio_PatternsTrackState* self, uintptr_t track)
{
	return self->soloactive && self->soloedtrack == track;		
}

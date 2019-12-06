// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "patternstrackstate.h"

void patternstrackstate_init(PatternsTrackState* self)
{	
	table_init(&self->mute);
	table_init(&self->record);
	self->soloactive = 0;	
	self->soloedtrack = 0;
}

void patternstrackstate_dispose(PatternsTrackState* self)
{
	table_dispose(&self->mute);
	table_dispose(&self->record);
	self->soloactive = 0;	
	self->soloedtrack = 0;
}

void patternstrackstate_activatesolotrack(PatternsTrackState* self, uintptr_t track)
{
	self->soloactive = 1;
	self->soloedtrack = track;
}

void patternstrackstate_deactivatesolotrack(PatternsTrackState* self)
{
	self->soloactive = 0;
	table_clear(&self->mute);
}

void patternstrackstate_mutetrack(PatternsTrackState* self, uintptr_t track)
{
	if (!self->soloactive) {
		table_insert(&self->mute, track, (void*)(uintptr_t) 1);
	} else {
		table_remove(&self->mute, track);
	}
}

void patternstrackstate_unmutetrack(PatternsTrackState* self, uintptr_t track)
{
	if (!self->soloactive) {
		table_remove(&self->mute, track);
	} else {
		table_insert(&self->mute, track, (void*)(uintptr_t) 1);
	}
}

int patternstrackstate_istrackmuted(PatternsTrackState* self, uintptr_t track)
{
	return self->soloactive
		? !table_exists(&self->mute, track)
		: table_exists(&self->mute, track);
}

int patternstrackstate_istracksoloed(PatternsTrackState* self, uintptr_t track)
{
	return self->soloactive && self->soloedtrack == track;		
}

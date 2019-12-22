// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "patterns.h"
#include <stdlib.h>

static void patterns_disposeslots(psy_audio_Patterns*);

void patterns_init(psy_audio_Patterns* self)
{
	psy_table_init(&self->slots);
	self->songtracks = 16;	
	self->sharetracknames = 0;
	patternstrackstate_init(&self->trackstate);
}

void patterns_dispose(psy_audio_Patterns* self)
{	
	patterns_disposeslots(self);
	patternstrackstate_dispose(&self->trackstate);
}

void patterns_disposeslots(psy_audio_Patterns* self)
{	
	psy_TableIterator it;

	for (it = psy_table_begin(&self->slots);
			!psy_tableiterator_equal(&it, psy_table_end()); psy_tableiterator_inc(&it)) {
		psy_audio_Pattern* pattern;
		
		pattern = (psy_audio_Pattern*)psy_tableiterator_value(&it);
		pattern_dispose(pattern);
		free(pattern);
	}
	psy_table_dispose(&self->slots);
}

void patterns_clear(psy_audio_Patterns* self)
{
	patterns_disposeslots(self);	
	psy_table_init(&self->slots);	
}

void patterns_insert(psy_audio_Patterns* self, uintptr_t slot, psy_audio_Pattern* pattern)
{
	psy_table_insert(&self->slots, slot, pattern);
}

int patterns_append(psy_audio_Patterns* self, psy_audio_Pattern* pattern)
{
	int slot = 0;
	
	while (psy_table_at(&self->slots, slot)) {
		++slot;
	}
	psy_table_insert(&self->slots, slot, pattern);	
	return slot;
}

psy_audio_Pattern* patterns_at(psy_audio_Patterns* self, uintptr_t slot)
{
	return psy_table_at(&self->slots, slot);
}

void patterns_erase(psy_audio_Patterns* self, uintptr_t slot)
{

	psy_table_remove(&self->slots, slot);
}

void patterns_remove(psy_audio_Patterns* self, uintptr_t slot)
{
	psy_audio_Pattern* pattern;
	
	pattern = (psy_audio_Pattern*) psy_table_at(&self->slots, slot);
	psy_table_remove(&self->slots, slot);
	pattern_dispose(pattern);
	free(pattern);	
}

uintptr_t patterns_size(psy_audio_Patterns* self)
{
	return self->slots.count;
}

void patterns_activatesolotrack(psy_audio_Patterns* self, uintptr_t track)
{
	patternstrackstate_activatesolotrack(&self->trackstate, track);
}

void patterns_deactivatesolotrack(psy_audio_Patterns* self)
{
	patternstrackstate_deactivatesolotrack(&self->trackstate);
}

void patterns_mutetrack(psy_audio_Patterns* self, uintptr_t track)
{
	patternstrackstate_mutetrack(&self->trackstate, track);
}

void patterns_unmutetrack(psy_audio_Patterns* self, uintptr_t track)
{
	patternstrackstate_unmutetrack(&self->trackstate, track);
}

int patterns_istrackmuted(psy_audio_Patterns* self, uintptr_t track)
{
	return patternstrackstate_istrackmuted(&self->trackstate, track);
}

int patterns_istracksoloed(psy_audio_Patterns* self, uintptr_t track)
{
	return patternstrackstate_istracksoloed(&self->trackstate, track);
}

void patterns_setsongtracks(psy_audio_Patterns* self, uintptr_t trackcount)
{
	self->songtracks = trackcount;
}

uintptr_t patterns_songtracks(psy_audio_Patterns* self)
{
	return self->songtracks;
}

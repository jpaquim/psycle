// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "patterns.h"
#include <stdlib.h>

static void patterns_init_signals(psy_audio_Patterns*);
static void patterns_dispose_signals(psy_audio_Patterns*);
static uintptr_t patterns_firstfreeslot(psy_audio_Patterns*);
static void patterns_disposeslots(psy_audio_Patterns*);
static void patterns_onpatternnamechanged(psy_audio_Patterns*,
	psy_audio_Pattern* sender);
static void patterns_onpatternlengthchanged(psy_audio_Patterns*,
	psy_audio_Pattern* sender);

void patterns_init(psy_audio_Patterns* self)
{
	psy_table_init(&self->slots);
	self->songtracks = 16;	
	self->sharetracknames = 0;
	patternstrackstate_init(&self->trackstate);
	patterns_init_signals(self);
}

void patterns_init_signals(psy_audio_Patterns* self)
{
	psy_signal_init(&self->signal_namechanged);
	psy_signal_init(&self->signal_lengthchanged);
}

void patterns_dispose(psy_audio_Patterns* self)
{	
	patterns_disposeslots(self);
	patternstrackstate_dispose(&self->trackstate);
	patterns_dispose_signals(self);
}

void patterns_dispose_signals(psy_audio_Patterns* self)
{
	psy_signal_dispose(&self->signal_namechanged);
	psy_signal_dispose(&self->signal_lengthchanged);
}

void patterns_disposeslots(psy_audio_Patterns* self)
{	
	psy_TableIterator it;

	for (it = psy_table_begin(&self->slots);
			!psy_tableiterator_equal(&it, psy_table_end()); psy_tableiterator_inc(&it)) {
		psy_audio_Pattern* pattern;
		
		pattern = (psy_audio_Pattern*)psy_tableiterator_value(&it);
		psy_audio_pattern_dispose(pattern);
		free(pattern);
	}
	psy_table_dispose(&self->slots);
	psy_signal_disconnectall(&self->signal_namechanged);
}

void patterns_clear(psy_audio_Patterns* self)
{
	patterns_disposeslots(self);	
	psy_table_init(&self->slots);	
}

void patterns_insert(psy_audio_Patterns* self, uintptr_t slot,
	psy_audio_Pattern* pattern)
{
	psy_table_insert(&self->slots, slot, pattern);
	psy_signal_connect(&pattern->signal_namechanged, self,
		patterns_onpatternnamechanged);
	psy_signal_connect(&pattern->signal_lengthchanged, self,
		patterns_onpatternlengthchanged);
}

uintptr_t patterns_append(psy_audio_Patterns* self, psy_audio_Pattern* pattern)
{
	uintptr_t slot;
	
	slot = patterns_firstfreeslot(self);
	patterns_insert(self, slot, pattern);
	return slot;
}

uintptr_t patterns_firstfreeslot(psy_audio_Patterns* self)
{
	uintptr_t rv = 0;

	// find first free slot
	while (psy_table_exists(&self->slots, rv)) {
		++rv;
	}
	return rv;
}

psy_audio_Pattern* patterns_at(psy_audio_Patterns* self, uintptr_t slot)
{
	return psy_table_at(&self->slots, slot);
}

uintptr_t patterns_slot(psy_audio_Patterns* self, psy_audio_Pattern* pattern)
{
	uintptr_t rv = UINTPTR_MAX;
	psy_TableIterator it;

	for (it = psy_table_begin(&self->slots);
		!psy_tableiterator_equal(&it, psy_table_end());
		psy_tableiterator_inc(&it)) {
		if (pattern == psy_tableiterator_value(&it)) {
			rv = psy_tableiterator_key(&it);
			break;
		}
	}
	return rv;
}

void patterns_erase(psy_audio_Patterns* self, uintptr_t slot)
{
	psy_audio_Pattern* pattern;
	
	pattern = patterns_at(self, slot);
	psy_table_remove(&self->slots, slot);
	if (pattern) {
		psy_signal_disconnect(&pattern->signal_namechanged, self,
			patterns_onpatternnamechanged);
		psy_signal_disconnect(&pattern->signal_lengthchanged, self,
			patterns_onpatternlengthchanged);
	}
}

void patterns_remove(psy_audio_Patterns* self, uintptr_t slot)
{
	psy_audio_Pattern* pattern;
	
	pattern = patterns_at(self, slot);
	psy_table_remove(&self->slots, slot);
	if (pattern) {
		psy_audio_pattern_dispose(pattern);
		free(pattern);
	}
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

void patterns_onpatternnamechanged(psy_audio_Patterns* self,
	psy_audio_Pattern* sender)
{
	uintptr_t slot;
	
	slot = patterns_slot(self, sender);
	if (slot != UINTPTR_MAX) {
		psy_signal_emit(&self->signal_namechanged, self, 1, slot);
	}	
}

void patterns_onpatternlengthchanged(psy_audio_Patterns* self,
	psy_audio_Pattern* sender)
{
	uintptr_t slot;

	slot = patterns_slot(self, sender);
	if (slot != UINTPTR_MAX) {
		psy_signal_emit(&self->signal_lengthchanged, self, 1, slot);
	}
}
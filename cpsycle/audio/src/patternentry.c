// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "patternentry.h"

#include <list.h>
#include <stdlib.h>
#include <string.h>

void psy_audio_patternentry_init(psy_audio_PatternEntry* self)
{
	psy_audio_PatternEvent first;

	assert(self);
	memset(self, 0, sizeof(psy_audio_PatternEntry));
	psy_audio_patternevent_clear(&first);
	psy_audio_patternentry_addevent(self,	&first);
}

void psy_audio_patternentry_init_all(psy_audio_PatternEntry* self,
	const psy_audio_PatternEvent* event,
	psy_dsp_big_beat_t offset,
	psy_dsp_big_beat_t delta,
	psy_dsp_big_beat_t bpm,
	uintptr_t track)	
{
	assert(self);
	self->events = NULL;
	psy_audio_patternentry_addevent(self,	event);	
	self->offset = offset;
	self->delta = delta;
	self->bpm = bpm;
	self->track = track;
}

void psy_audio_patternentry_dispose(psy_audio_PatternEntry* self)
{
	psy_List* p;

	assert(self);
	for (p = self->events; p != NULL; psy_list_next(&p)) {
		free(psy_list_entry(p));
	}
	psy_list_free(self->events);
	self->events = 0;
}

psy_audio_PatternEntry* psy_audio_patternentry_alloc(void)
{
	return (psy_audio_PatternEntry*)malloc(sizeof(psy_audio_PatternEntry));
}

psy_audio_PatternEntry* psy_audio_patternentry_allocinit(void)
{
	psy_audio_PatternEntry* rv;
	
	rv = psy_audio_patternentry_alloc();
	if (rv) {
		psy_audio_patternentry_init(rv);
	}
	return rv;
}

psy_audio_PatternEntry* psy_audio_patternentry_allocinit_all(
	const psy_audio_PatternEvent* event,
	psy_dsp_big_beat_t offset,
	psy_dsp_big_beat_t delta,
	psy_dsp_big_beat_t bpm,
	uintptr_t track)
{
	psy_audio_PatternEntry* rv;

	rv = psy_audio_patternentry_alloc();
	if (rv) {
		psy_audio_patternentry_init_all(rv, event, offset, delta, bpm, track);
	}
	return rv;
}

psy_audio_PatternEntry* psy_audio_patternentry_clone(psy_audio_PatternEntry* entry)
{
	psy_audio_PatternEntry* rv;
	if (entry) {
		psy_List* p;
		rv = psy_audio_patternentry_alloc();
		rv->bpm = entry->bpm;
		rv->delta = entry->delta;
		rv->offset = entry->offset;
		rv->track = entry->track;
		rv->events = 0;
		rv->priority = 0;
		for (p = entry->events; p != NULL; psy_list_next(&p)) {
			psy_audio_PatternEvent* copy;

			copy = (psy_audio_PatternEvent*)
				malloc(sizeof(psy_audio_PatternEvent));
			if (copy) {
				*copy = *((psy_audio_PatternEvent*)p->entry);
				psy_list_append(&rv->events, copy);
			}
		}		
	} else {
		rv = 0;
	}
	return rv;
}

void psy_audio_patternentry_addevent(psy_audio_PatternEntry* self,
	const psy_audio_PatternEvent* event)
{
	psy_audio_PatternEvent* copy;

	assert(self);
	copy = (psy_audio_PatternEvent*)malloc(sizeof(psy_audio_PatternEvent));
	if (copy) {
		*copy = *event;
		psy_list_append(&self->events, copy);
	}
}

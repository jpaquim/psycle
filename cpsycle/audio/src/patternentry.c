// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "patternentry.h"

#include <list.h>
#include <stdlib.h>
#include <string.h>

void patternentry_init(psy_audio_PatternEntry* self)
{
	psy_audio_PatternEvent first;

	memset(self, 0, sizeof(psy_audio_PatternEntry));
	patternevent_clear(&first);
	patternentry_addevent(self,	&first);
}

void patternentry_init_all(psy_audio_PatternEntry* self,
	const psy_audio_PatternEvent* event,
	psy_dsp_beat_t offset,
	psy_dsp_beat_t delta,
	psy_dsp_beat_t bpm,
	uintptr_t track)	
{
	self->events = 0;
	patternentry_addevent(self,	event);	
	self->offset = offset;
	self->delta = delta;
	self->bpm = bpm;
	self->track = track;
}

void patternentry_dispose(psy_audio_PatternEntry* self)
{
	psy_List* p;

	for (p = self->events; p != NULL; psy_list_next(&p)) {
		free(psy_list_entry(p));
	}
	psy_list_free(self->events);
	self->events = 0;
}

psy_audio_PatternEntry* patternentry_alloc(void)
{
	return (psy_audio_PatternEntry*) malloc(sizeof(psy_audio_PatternEntry));
}

psy_audio_PatternEntry* patternentry_allocinit(void)
{
	psy_audio_PatternEntry* rv;
	
	rv = patternentry_alloc();
	if (rv) {
		patternentry_init(rv);
	}
	return rv;
}

psy_audio_PatternEntry* patternentry_allocinit_all(
	const psy_audio_PatternEvent* event,
	psy_dsp_beat_t offset,
	psy_dsp_beat_t delta,
	psy_dsp_beat_t bpm,
	uintptr_t track)
{
	psy_audio_PatternEntry* rv;

	rv = patternentry_alloc();
	if (rv) {
		patternentry_init_all(rv, event, offset, delta, bpm, track);
	}
	return rv;
}

psy_audio_PatternEntry* patternentry_clone(psy_audio_PatternEntry* entry)
{
	psy_audio_PatternEntry* rv;
	if (entry) {
		psy_List* p;
		rv = patternentry_alloc();
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
			*copy = *((psy_audio_PatternEvent*)p->entry);
			psy_list_append(&rv->events, copy);
		}		
	} else {
		rv = 0;
	}
	return rv;
}

void patternentry_addevent(psy_audio_PatternEntry* self,
	const psy_audio_PatternEvent* event)
{
	psy_audio_PatternEvent* copy;

	copy = (psy_audio_PatternEvent*)malloc(sizeof(psy_audio_PatternEvent));
	*copy = *event;
	psy_list_append(&self->events, copy);
}

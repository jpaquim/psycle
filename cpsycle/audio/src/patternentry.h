/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net
*/

#ifndef psy_audio_PATTERNENTRY_H
#define psy_audio_PATTERNENTRY_H

#include "patternevent.h"

#include "../../detail/stdint.h"
#include "../../detail/psydef.h"

#include <assert.h>
#include <list.h>

#ifdef __cplusplus
extern "C" {
#endif

/* an event with additional position and track information */
typedef struct psy_audio_PatternEntry {
	psy_List* events;
	/* position in beat unit */
	psy_dsp_big_beat_t offset;	
	/* sound driver callback event position */
	psy_dsp_big_beat_t delta;
	/* current sequencer bpm */
	psy_dsp_big_beat_t bpm;
	/* the tracker channel */
	uintptr_t track; 
	uintptr_t priority;	
} psy_audio_PatternEntry;

void psy_audio_patternentry_init(psy_audio_PatternEntry*);
void psy_audio_patternentry_init_all(psy_audio_PatternEntry*,
	const psy_audio_PatternEvent* event,
	psy_dsp_big_beat_t offset,
	psy_dsp_big_beat_t delta,
	psy_dsp_big_beat_t bpm,
	uintptr_t track);
void psy_audio_patternentry_dispose(psy_audio_PatternEntry*);

psy_audio_PatternEntry* psy_audio_patternentry_alloc(void);
psy_audio_PatternEntry* psy_audio_patternentry_allocinit(void);
psy_audio_PatternEntry* psy_audio_patternentry_allocinit_all(
	const psy_audio_PatternEvent* event,
	psy_dsp_big_beat_t offset,
	psy_dsp_big_beat_t delta,
	psy_dsp_big_beat_t bpm,
	uintptr_t track);
psy_audio_PatternEntry* psy_audio_patternentry_clone(psy_audio_PatternEntry*);

INLINE psy_audio_PatternEvent* psy_audio_patternentry_front(psy_audio_PatternEntry* self)
{
	assert(self);

	return (psy_audio_PatternEvent*)(self->events->entry);
}

INLINE const psy_audio_PatternEvent* psy_audio_patternentry_front_const(
	const psy_audio_PatternEntry* self)
{
	assert(self);

	return (const psy_audio_PatternEvent*)(self->events->entry);
}

INLINE psy_audio_PatternEvent* psy_audio_patternentry_at(psy_audio_PatternEntry* self,
	uintptr_t index)
{
	psy_List* p;

	p = psy_list_at(self->events, index);
	if (p) {
		return (psy_audio_PatternEvent*)p->entry;
	}
	return NULL;
}

INLINE void psy_audio_patternentry_setbpm(psy_audio_PatternEntry* self, psy_dsp_big_beat_t bpm)
{
	assert(self);

	self->bpm = bpm;
}

INLINE psy_dsp_big_beat_t psy_audio_patternentry_offset(
	const psy_audio_PatternEntry* self)
{
	return self->offset;
}

void psy_audio_patternentry_addevent(psy_audio_PatternEntry*,
	const psy_audio_PatternEvent*);
void psy_audio_patternentry_setevent(psy_audio_PatternEntry*,
	const psy_audio_PatternEvent*, uintptr_t index);


/* a list of event entries ordered by position in beat unit */
typedef psy_List psy_audio_PatternNode;

INLINE psy_audio_PatternEntry* psy_audio_patternnode_entry(psy_audio_PatternNode* self)
{
	assert(self);

	return (psy_audio_PatternEntry*)self->entry;
}

INLINE void psy_audio_patternnode_next(psy_audio_PatternNode** self)
{
	assert(self);

	psy_list_next(self);
}

#ifdef __cplusplus
}
#endif

#endif /* psy_audio_PATTERNENTRY_H */

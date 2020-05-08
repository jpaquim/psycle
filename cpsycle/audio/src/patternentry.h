// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#ifndef psy_audio_PATTERNENTRY_H
#define psy_audio_PATTERNENTRY_H

#include "patternevent.h"
#include <list.h>
#include "../../detail/stdint.h"
#include "../../detail/psydef.h"

#ifdef __cplusplus
extern "C" {
#endif

/// an event with additional position and track information
typedef struct {
	psy_List* events;
	/// position in beat unit
	psy_dsp_beat_t offset;
	/// sound driver callback event position
	psy_dsp_beat_t delta;
	/// current sequencer bpm
	psy_dsp_beat_t bpm;
	/// the tracker channel
	uintptr_t track; 
	uintptr_t priority;
} psy_audio_PatternEntry;

void patternentry_init(psy_audio_PatternEntry*);
void patternentry_init_all(psy_audio_PatternEntry*,
	const psy_audio_PatternEvent* event,
	psy_dsp_beat_t offset,
	psy_dsp_beat_t delta,
	psy_dsp_beat_t bpm,
	uintptr_t track);
void patternentry_dispose(psy_audio_PatternEntry*);

psy_audio_PatternEntry* patternentry_alloc(void);
psy_audio_PatternEntry* patternentry_allocinit(void);
psy_audio_PatternEntry* patternentry_allocinit_all(
	const psy_audio_PatternEvent* event,
	psy_dsp_beat_t offset,
	psy_dsp_beat_t delta,
	psy_dsp_beat_t bpm,
	uintptr_t track);
psy_audio_PatternEntry* patternentry_clone(psy_audio_PatternEntry*);

INLINE psy_audio_PatternEvent* patternentry_front(psy_audio_PatternEntry* self)
{
	return (psy_audio_PatternEvent*)(self->events->entry);
}

INLINE const psy_audio_PatternEvent* patternentry_front_const(
	const psy_audio_PatternEntry* self)
{
	return (const psy_audio_PatternEvent*)(self->events->entry);
}

INLINE void patternentry_setbpm(psy_audio_PatternEntry* self, psy_dsp_beat_t bpm)
{
	self->bpm = bpm;
}

void patternentry_addevent(psy_audio_PatternEntry*,
	const psy_audio_PatternEvent*);


/// a list of event entries ordered by position in beat unit
typedef psy_List PatternNode;

INLINE psy_audio_PatternEntry* psy_audio_patternnode_entry(PatternNode* self)
{
	return (psy_audio_PatternEntry*)self->entry;
}

#ifdef __cplusplus
}
#endif

#endif /* psy_audio_PATTERNENTRY_H */

/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net
*/

#ifndef psy_audio_SEQUENCEENTRY_H
#define psy_audio_SEQUENCEENTRY_H

/* audio */
#include "patterns.h"
#include "samples.h"
/* container */
#include <list.h>
#include <signal.h>
#include <command.h>

#include "../../detail/psydef.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
** psy_audio_SequenceEntry
**
** item of the playorder list of a song (multisequence)
*/

typedef enum psy_audio_SequenceEntryType {
	psy_audio_SEQUENCEENTRY_PATTERN = 1,
	psy_audio_SEQUENCEENTRY_SAMPLE,
	psy_audio_SEQUENCEENTRY_MARKER
} psy_audio_SequenceEntryType;

/*
** psy_audio_SequenceEntry
**
** Base class entry inside a track of a sequence
*/

struct psy_audio_SequenceEntry;

typedef	void (*psy_audio_fp_sequenceentry_dispose)
	(struct psy_audio_SequenceEntry*);
typedef struct psy_audio_SequenceEntry* (*psy_audio_fp_sequenceentry_clone)
	(const struct psy_audio_SequenceEntry*);
typedef	psy_dsp_big_beat_t (*psy_audio_fp_sequenceentry_length)
	(const struct psy_audio_SequenceEntry*);
typedef	void (*psy_audio_fp_sequenceentry_setlength)
	(struct psy_audio_SequenceEntry*, psy_dsp_big_beat_t);

typedef struct psy_audio_SequenceEntryVtable {
	psy_audio_fp_sequenceentry_dispose dispose;
	psy_audio_fp_sequenceentry_clone clone;
	psy_audio_fp_sequenceentry_length length;
	psy_audio_fp_sequenceentry_setlength setlength;
} psy_audio_SequenceEntryVtable;

typedef struct psy_audio_SequenceEntry {
	psy_audio_SequenceEntryVtable* vtable;
	int type;
	/* absolute start position (in beats) in the song */
	psy_dsp_big_beat_t offset;	
	/*
	** offset to the playlist index position allowing free positioning
	** needed to reposition the sequence
	*/
	psy_dsp_big_beat_t repositionoffset;
	uintptr_t row;
	bool selplay;
} psy_audio_SequenceEntry;

void psy_audio_sequenceentry_init_all(psy_audio_SequenceEntry*,
	psy_audio_SequenceEntryType type, psy_dsp_big_beat_t offset);

INLINE psy_audio_SequenceEntry* psy_audio_sequenceentry_clone(
	const psy_audio_SequenceEntry* self)
{
	return self->vtable->clone(self);
}

INLINE psy_dsp_big_beat_t psy_audio_sequenceentry_length(
	const psy_audio_SequenceEntry* self)
{
	return self->vtable->length(self);
}

INLINE void psy_audio_sequenceentry_dispose(psy_audio_SequenceEntry* self)
{
	self->vtable->dispose(self);
}

INLINE void psy_audio_sequenceentry_setlength(
	psy_audio_SequenceEntry* self, psy_dsp_big_beat_t length)
{
	self->vtable->setlength(self, length);
}

INLINE psy_dsp_big_beat_t psy_audio_sequenceentry_offset(
	const psy_audio_SequenceEntry* self)
{
	return self->offset;
}

INLINE psy_dsp_big_beat_t psy_audio_sequenceentry_rightoffset(
	const psy_audio_SequenceEntry* self)
{
	return self->offset + psy_audio_sequenceentry_length(self);
}

/*
** psy_audio_SequencePatternEntry
**
** Entry inside a track of a sequence with a pattern index
*/
typedef struct psy_audio_SequencePatternEntry {
	/* inherits */
	psy_audio_SequenceEntry entry;	
	/* internal */
	/* playorder value (the pattern to be played) */
	uintptr_t patternslot;
	psy_audio_Patterns* patterns;
	/*
	** sample index to be played if psycle will support audio patterns
	** not used now
	*/
	psy_audio_SampleIndex sampleindex;	
} psy_audio_SequencePatternEntry;

void psy_audio_sequencepatternentry_init(psy_audio_SequencePatternEntry*,
	uintptr_t patternslot, psy_dsp_big_beat_t offset);

psy_audio_SequencePatternEntry* psy_audio_sequencepatternentry_alloc(void);
psy_audio_SequencePatternEntry* psy_audio_sequencepatternentry_allocinit(
	uintptr_t patternslot, psy_dsp_big_beat_t offset);

INLINE void psy_audio_sequencepatternentry_setpatternslot(
	psy_audio_SequencePatternEntry* self, uintptr_t slot)
{
	assert(self);

	self->patternslot = slot;
}

INLINE uintptr_t psy_audio_sequencepatternentry_patternslot(const
	psy_audio_SequencePatternEntry* self)
{
	assert(self);

	return self->patternslot;
}

INLINE psy_audio_Pattern* psy_audio_sequencepatternentry_pattern(const
	psy_audio_SequencePatternEntry* self, psy_audio_Patterns* patterns)
{
	assert(self);

	if (patterns) {
		return (psy_audio_Pattern*)psy_audio_patterns_at(patterns,
			self->patternslot);		
	}
	return NULL;
}

/*
** psy_audio_SequenceSampleEntry
**
** Entry inside a track of a sequence with a pattern index
*/
typedef struct psy_audio_SequenceSampleEntry {
	/* inherits*/
	psy_audio_SequenceEntry entry;
	/* internal */
	/* playorder value (the pattern to be played) */	
	psy_audio_Samples* samples;
	/*
	** sample index to be played if psycle will support audio patterns
	** not used now
	*/	
	psy_audio_SampleIndex sampleindex;
} psy_audio_SequenceSampleEntry;

void psy_audio_sequencesampleentry_init(psy_audio_SequenceSampleEntry*,
	psy_dsp_big_beat_t offset, psy_audio_SampleIndex);

psy_audio_SequenceSampleEntry* psy_audio_sequencesampleentry_alloc(void);
psy_audio_SequenceSampleEntry* psy_audio_sequencesampleentry_allocinit(
	psy_dsp_big_beat_t offset, psy_audio_SampleIndex);

INLINE void psy_audio_sequencesampleentry_setsampleslot(
	psy_audio_SequenceSampleEntry* self, psy_audio_SampleIndex index)
{
	assert(self);

	self->sampleindex = index;
}

INLINE psy_audio_SampleIndex psy_audio_sequencesampleentry_samplesindex(const
	psy_audio_SequenceSampleEntry* self)
{
	assert(self);

	return self->sampleindex;
}

INLINE psy_audio_Sample* psy_audio_sequencesampleentry_sample(const
	psy_audio_SequenceSampleEntry* self, psy_audio_Samples* samples)
{
	assert(self);

	if (samples) {
		return (psy_audio_Sample*)psy_audio_samples_at(samples,
			self->sampleindex);
	}
	return NULL;
}

/*
** psy_audio_SequenceMarkerEntry
**
** Entry inside a track of a sequence with a label
*/
typedef struct psy_audio_SequenceMarkerEntry {
	/* inherits */
	psy_audio_SequenceEntry entry;
	/* internal */
	char* text;
	psy_dsp_big_beat_t length;
} psy_audio_SequenceMarkerEntry;

void psy_audio_sequencemarkerentry_init(psy_audio_SequenceMarkerEntry*,
	psy_dsp_big_beat_t offset, const char* text);

psy_audio_SequenceMarkerEntry* psy_audio_sequencemarkerentry_alloc(void);
psy_audio_SequenceMarkerEntry* psy_audio_sequencemarkerentry_allocinit(
	psy_dsp_big_beat_t offset, const char* text);

#ifdef __cplusplus
}
#endif

#endif /* psy_audio_SEQUENCEENTRY_H */

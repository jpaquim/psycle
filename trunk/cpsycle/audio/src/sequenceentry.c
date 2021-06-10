/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "sequenceentry.h"
/* platform */
#include "../../detail/portable.h"

/* psy_audio_SequenceEntry */
static void psy_audio_static_sequenceentry_dispose(
	psy_audio_SequenceEntry* self)
{
}

static psy_audio_SequenceEntry* psy_audio_static_sequenceentry_clone(
		const psy_audio_SequenceEntry* self)
{
	psy_audio_SequenceEntry* rv;

	rv = (psy_audio_SequenceEntry*)malloc(sizeof(psy_audio_SequenceEntry));
	if (rv) {
		psy_audio_sequenceentry_init_all(rv, self->type, self->offset);
	}
	return rv;
}

static psy_dsp_big_beat_t psy_audio_static_sequenceentry_length(
	const psy_audio_SequenceEntry* self)
{
	return 0.0;
}

static void psy_audio_static_sequenceentry_setlength(
	psy_audio_SequenceEntry* self, psy_dsp_big_beat_t length)
{
	
}
/* vtable */
static psy_audio_SequenceEntryVtable psy_audio_sequenceentry_vtable;
static bool psy_audio_sequenceentry_vtable_initialized = FALSE;

static void psy_audio_sequenceentry_vtable_init(void)
{
	if (!psy_audio_sequenceentry_vtable_initialized) {
		psy_audio_sequenceentry_vtable.dispose =
			psy_audio_static_sequenceentry_dispose;
		psy_audio_sequenceentry_vtable.clone =			
			psy_audio_static_sequenceentry_clone;
		psy_audio_sequenceentry_vtable.length =
			psy_audio_static_sequenceentry_length;
		psy_audio_sequenceentry_vtable.setlength =
			psy_audio_static_sequenceentry_setlength;
		psy_audio_sequenceentry_vtable_initialized = TRUE;
	}	
}
/* implementation */
void psy_audio_sequenceentry_init_all(psy_audio_SequenceEntry* self,
	psy_audio_SequenceEntryType type, psy_dsp_big_beat_t offset)
{
	psy_audio_sequenceentry_vtable_init();
	self->vtable = &psy_audio_sequenceentry_vtable;
	self->type = type;
	self->row = 0;
	self->offset = offset;
	self->repositionoffset = 0.0;
	self->selplay = 0;
}

/* psy_audio_SequencePatternEntry */
psy_audio_SequenceEntry* psy_audio_static_sequencepatternentry_clone(
	const psy_audio_SequencePatternEntry* self) {
	psy_audio_SequencePatternEntry* rv;

	rv = (psy_audio_SequencePatternEntry*)malloc(
		sizeof(psy_audio_SequencePatternEntry));
	if (rv) {		
		*rv = *self;
		return &rv->entry;
	}	
	return NULL;
}
static psy_dsp_big_beat_t psy_audio_static_sequencepatternentry_length(
	const psy_audio_SequencePatternEntry*);
static void psy_audio_static_sequencepatternentry_setlength(
	psy_audio_SequencePatternEntry*, psy_dsp_big_beat_t length);
/* vtable */
static psy_audio_SequenceEntryVtable psy_audio_sequencepatternentry_vtable;
static bool psy_audio_sequencepatternentry_vtable_initialized = FALSE;

static void psy_audio_sequencepatternentry_vtable_init(
	psy_audio_SequencePatternEntry* self)
{
	if (!psy_audio_sequencepatternentry_vtable_initialized) {
		psy_audio_sequencepatternentry_vtable = *(self->entry.vtable);
		psy_audio_sequencepatternentry_vtable.clone =
			(psy_audio_fp_sequenceentry_clone)
			psy_audio_static_sequencepatternentry_clone;
		psy_audio_sequencepatternentry_vtable.length =
			(psy_audio_fp_sequenceentry_length)
			psy_audio_static_sequencepatternentry_length;
		psy_audio_sequencepatternentry_vtable.setlength =
			(psy_audio_fp_sequenceentry_setlength)
			psy_audio_static_sequencepatternentry_setlength;
		psy_audio_sequencepatternentry_vtable_initialized = TRUE;
	}
	self->entry.vtable = &psy_audio_sequencepatternentry_vtable;
}
/* implementation */
void psy_audio_sequencepatternentry_init(psy_audio_SequencePatternEntry* self,
	uintptr_t patternslot,
	psy_dsp_big_beat_t offset)
{	
	psy_audio_sequenceentry_init_all(&self->entry,
		psy_audio_SEQUENCEENTRY_PATTERN, offset);
	psy_audio_sequencepatternentry_vtable_init(self);
	self->patternslot = patternslot;
	self->patterns = NULL;	
}

psy_audio_SequencePatternEntry* psy_audio_sequencepatternentry_alloc(void)
{
	return (psy_audio_SequencePatternEntry*)malloc(sizeof(
		psy_audio_SequencePatternEntry));
}

psy_audio_SequencePatternEntry* psy_audio_sequencepatternentry_allocinit(
	uintptr_t pattern, psy_dsp_big_beat_t offset)
{
	psy_audio_SequencePatternEntry* rv;

	rv = psy_audio_sequencepatternentry_alloc();
	if (rv) {
		psy_audio_sequencepatternentry_init(rv, pattern, offset);
	}
	return rv;
}

psy_dsp_big_beat_t psy_audio_static_sequencepatternentry_length(
	const psy_audio_SequencePatternEntry* self)
{
	const psy_audio_Pattern* pattern;

	if (self->patterns) {
		pattern = psy_audio_patterns_at_const(self->patterns,
			psy_audio_sequencepatternentry_patternslot(self));
		if (pattern) {
			return psy_audio_pattern_length(pattern);
		}
	}
	return 0.0;
}

void psy_audio_static_sequencepatternentry_setlength(
	psy_audio_SequencePatternEntry* self, psy_dsp_big_beat_t length)
{
	psy_audio_Pattern* pattern;

	if (self->patterns) {
		pattern = psy_audio_patterns_at(self->patterns,
			psy_audio_sequencepatternentry_patternslot(self));
		if (pattern) {
			psy_audio_pattern_setlength(pattern, length);
		}
	}
}

/* psy_audio_SequenceSampleEntry */
psy_audio_SequenceEntry* psy_audio_static_sequencesampleentry_clone(
	const psy_audio_SequenceSampleEntry* self) {
	psy_audio_SequenceSampleEntry* rv;

	rv = (psy_audio_SequenceSampleEntry*)malloc(
		sizeof(psy_audio_SequenceSampleEntry));
	if (rv) {
		*rv = *self;
		return &rv->entry;
	}
	return NULL;
}
static psy_dsp_big_beat_t psy_audio_static_sequencesampleentry_length(
	const psy_audio_SequenceSampleEntry*);
static void psy_audio_sequencesampleentry_dispose(
	psy_audio_SequenceSampleEntry*);
/* vtable */
static psy_audio_SequenceEntryVtable psy_audio_sequencesampleentry_vtable;
static bool psy_audio_sequencesampleentry_vtable_initialized = FALSE;

static void psy_audio_sequencesampleentry_vtable_init(
	psy_audio_SequenceSampleEntry* self)
{
	if (!psy_audio_sequencesampleentry_vtable_initialized) {
		psy_audio_sequencesampleentry_vtable = *(self->entry.vtable);
		psy_audio_sequencesampleentry_vtable.dispose =
			(psy_audio_fp_sequenceentry_dispose)
			psy_audio_sequencesampleentry_dispose;
		psy_audio_sequencesampleentry_vtable.clone =
			(psy_audio_fp_sequenceentry_clone)
			psy_audio_static_sequencesampleentry_clone;
		psy_audio_sequencesampleentry_vtable.length =
			(psy_audio_fp_sequenceentry_length)
			psy_audio_static_sequencesampleentry_length;		
		psy_audio_sequencesampleentry_vtable_initialized = TRUE;
	}
	self->entry.vtable = &psy_audio_sequencesampleentry_vtable;
}
/* implementation */
void psy_audio_sequencesampleentry_init(psy_audio_SequenceSampleEntry* self,	
	psy_dsp_big_beat_t offset, psy_audio_SampleIndex sampleindex)
{
	psy_audio_sequenceentry_init_all(&self->entry,
		psy_audio_SEQUENCEENTRY_SAMPLE, offset);
	psy_audio_sequencesampleentry_vtable_init(self);
	psy_signal_init(&self->signal_samplechanged);
	self->samples = NULL;
	self->sampleindex = sampleindex;
	self->samplerindex = psy_INDEX_INVALID;
}

void psy_audio_sequencesampleentry_dispose(psy_audio_SequenceSampleEntry* self)
{
	psy_signal_dispose(&self->signal_samplechanged);
}

psy_audio_SequenceSampleEntry* psy_audio_sequencesampleentry_alloc(void)
{
	return (psy_audio_SequenceSampleEntry*)malloc(sizeof(
		psy_audio_SequenceSampleEntry));
}

psy_audio_SequenceSampleEntry* psy_audio_sequencesampleentry_allocinit(
	psy_dsp_big_beat_t offset, psy_audio_SampleIndex sampleindex)
{
	psy_audio_SequenceSampleEntry* rv;

	rv = psy_audio_sequencesampleentry_alloc();
	if (rv) {
		psy_audio_sequencesampleentry_init(rv, offset, sampleindex);
	}
	return rv;
}

psy_dsp_big_beat_t psy_audio_static_sequencesampleentry_length(
	const psy_audio_SequenceSampleEntry* self)
{
	const psy_audio_Sample* sample;

	if (self->samples) {
		sample = psy_audio_samples_at_const(self->samples,
			psy_audio_sequencesampleentry_samplesindex(self));
		if (sample) {
			psy_dsp_big_beat_t beatspersample;

			beatspersample = (125.0 * 1.0) / (44100.0 * 60.0);
			return psy_audio_sample_numframes(sample) *
				beatspersample * 44100.0 / sample->samplerate;
		}
	}
	return 16.0;
}

/* psy_audio_SequenceMarkerEntry */
static void psy_audio_static_sequencemarkerentry_dispose(
	psy_audio_SequenceMarkerEntry*);
static psy_audio_SequenceEntry* psy_audio_static_sequencemarkerentry_clone(
	const psy_audio_SequenceMarkerEntry* self) {
	psy_audio_SequenceMarkerEntry* rv;

	rv = (psy_audio_SequenceMarkerEntry*)malloc(
		sizeof(psy_audio_SequenceMarkerEntry));
	if (rv) {
		*rv = *self;
		rv->text = psy_strdup(self->text);
		return &rv->entry;
	}
	return NULL;
}

static psy_dsp_big_beat_t psy_audio_static_sequencemarkerentry_length(
		const psy_audio_SequenceMarkerEntry* self) {
	return self->length;
}

static void psy_audio_static_sequencemarkerentry_setlength(
		psy_audio_SequenceMarkerEntry* self, psy_dsp_big_beat_t length) {
	self->length = length;
}
/* vtable */
static psy_audio_SequenceEntryVtable psy_audio_sequencemarkerentry_vtable;
static bool psy_audio_sequencemarkerentry_vtable_initialized = FALSE;

static void psy_audio_sequencemarkerentry_vtable_init(
	psy_audio_SequenceMarkerEntry* self)
{
	if (!psy_audio_sequencemarkerentry_vtable_initialized) {
		psy_audio_sequencemarkerentry_vtable = *(self->entry.vtable);
		psy_audio_sequencemarkerentry_vtable.dispose =
			(psy_audio_fp_sequenceentry_dispose)
			psy_audio_static_sequencemarkerentry_dispose;
		psy_audio_sequencemarkerentry_vtable.clone =
			(psy_audio_fp_sequenceentry_clone)
			psy_audio_static_sequencemarkerentry_clone;
		psy_audio_sequencemarkerentry_vtable.length =
			(psy_audio_fp_sequenceentry_length)
			psy_audio_static_sequencemarkerentry_length;
		psy_audio_sequencemarkerentry_vtable.setlength =
			(psy_audio_fp_sequenceentry_setlength)
			psy_audio_static_sequencemarkerentry_setlength;
		psy_audio_sequencemarkerentry_vtable_initialized = TRUE;
	}
	self->entry.vtable = &psy_audio_sequencemarkerentry_vtable;
}
/* implementation */
void psy_audio_sequencemarkerentry_init(psy_audio_SequenceMarkerEntry* self,
	psy_dsp_big_beat_t offset, const char* text)
{
	psy_audio_sequenceentry_init_all(&self->entry,
		psy_audio_SEQUENCEENTRY_MARKER, offset);
	psy_audio_sequencemarkerentry_vtable_init(self);
	self->text = psy_strdup(text);
	self->length = 16.0;
}

void psy_audio_static_sequencemarkerentry_dispose(
	psy_audio_SequenceMarkerEntry* self)
{
	free(self->text);
	self->text = NULL;
}

psy_audio_SequenceMarkerEntry* psy_audio_sequencemarkerentry_alloc(void)
{
	return (psy_audio_SequenceMarkerEntry*)malloc(sizeof(
		psy_audio_SequenceMarkerEntry));
}

psy_audio_SequenceMarkerEntry* psy_audio_sequencemarkerentry_allocinit(
	psy_dsp_big_beat_t offset, const char* text)
{
	psy_audio_SequenceMarkerEntry* rv;

	rv = psy_audio_sequencemarkerentry_alloc();
	if (rv) {
		psy_audio_sequencemarkerentry_init(rv, offset, text);
	}
	return rv;
}

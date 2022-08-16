/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "sequencecmds.h"
/* local */
#include "exclusivelock.h"
#include "exclusivelock.h"
/* container */
#include <properties.h>
/* platform */
#include "../../detail/portable.h"

/* psy_audio_SequenceInsertCommand */

/* prototypes */
static void psy_audio_sequenceinsertcommand_dispose(psy_audio_SequenceInsertCommand*);
static void psy_audio_sequenceinsertcommand_execute(psy_audio_SequenceInsertCommand*,
	psy_Property* params);
static void psy_audio_sequenceinsertcommand_revert(psy_audio_SequenceInsertCommand*);

/* vtable */
static psy_CommandVtable psy_audio_sequenceinsertcommand_vtable;
static bool psy_audio_sequenceinsertcommand_vtable_initialized = FALSE;

static void psy_audio_sequenceinsertcommand_vtable_init(psy_audio_SequenceInsertCommand* self)
{
	if (!psy_audio_sequenceinsertcommand_vtable_initialized) {
		psy_audio_sequenceinsertcommand_vtable = *(self->command.vtable);
		psy_audio_sequenceinsertcommand_vtable.dispose =
			(psy_fp_command)
			psy_audio_sequenceinsertcommand_dispose;
		psy_audio_sequenceinsertcommand_vtable.execute =
			(psy_fp_command_params)
			psy_audio_sequenceinsertcommand_execute;
		psy_audio_sequenceinsertcommand_vtable.revert =
			(psy_fp_command)
			psy_audio_sequenceinsertcommand_revert;
		psy_audio_sequenceinsertcommand_vtable_initialized = TRUE;
	}
}
// implementation
psy_audio_SequenceInsertCommand* psy_audio_sequenceinsertcommand_alloc(	
	psy_audio_Sequence* sequence, psy_audio_SequenceSelection* selection,
	psy_audio_OrderIndex index, uintptr_t patidx)
{
	psy_audio_SequenceInsertCommand* rv;

	rv = malloc(sizeof(psy_audio_SequenceInsertCommand));
	if (rv) {
		psy_command_init(&rv->command);
		psy_audio_sequenceinsertcommand_vtable_init(rv);
		rv->command.vtable = &psy_audio_sequenceinsertcommand_vtable;
		rv->sequence = sequence;
		rv->selection = selection;
		psy_audio_sequenceselection_init(&rv->restoreselection);
		rv->index = index;
		rv->patidx = patidx;				
	}
	return rv;
}

void psy_audio_sequenceinsertcommand_dispose(psy_audio_SequenceInsertCommand* self)
{
	psy_audio_sequenceselection_dispose(&self->restoreselection);
}

void psy_audio_sequenceinsertcommand_execute(psy_audio_SequenceInsertCommand* self,
	psy_Property* params)
{	
	psy_audio_SequenceCursor cursor;	

	assert(self);
	assert(self->sequence);
	
	psy_audio_sequenceselection_copy(&self->restoreselection, self->selection);
	psy_audio_sequence_insert(self->sequence, self->index, self->patidx);	
	self->index = psy_audio_orderindex_make(self->index.track,
		self->index.order + 1);	
	cursor = psy_audio_sequence_cursor(self->sequence);
	psy_audio_sequencecursor_set_order_index(&cursor, self->index);	
	psy_audio_sequence_set_cursor(self->sequence, cursor);	
}

void psy_audio_sequenceinsertcommand_revert(psy_audio_SequenceInsertCommand* self)
{	
	psy_audio_sequence_remove(self->sequence, self->index);
	self->index = psy_audio_orderindex_make(
		self->index.track, self->index.order - 1);	
	psy_audio_sequenceselection_copy(self->selection, &self->restoreselection);
	psy_audio_sequenceselection_select_first(self->selection, self->index);	
}

/*
** psy_audio_SequenceSampleInsertCommand
*/
/* prototypes */
static void psy_audio_sequencesampleinsertcommand_dispose(psy_audio_SequenceSampleInsertCommand*);
static void psy_audio_sequencesampleinsertcommand_execute(psy_audio_SequenceSampleInsertCommand*,
	psy_Property* params);
static void psy_audio_sequencesampleinsertcommand_revert(psy_audio_SequenceSampleInsertCommand*);
/* vtable */
static psy_CommandVtable psy_audio_sequencesampleinsertcommand_vtable;
static bool psy_audio_sequencesampleinsertcommand_vtable_initialized = FALSE;

static void psy_audio_sequencesampleinsertcommand_vtable_init(psy_audio_SequenceSampleInsertCommand* self)
{
	if (!psy_audio_sequencesampleinsertcommand_vtable_initialized) {
		psy_audio_sequencesampleinsertcommand_vtable = *(self->command.vtable);
		psy_audio_sequencesampleinsertcommand_vtable.dispose =
			(psy_fp_command)
			psy_audio_sequencesampleinsertcommand_dispose;
		psy_audio_sequencesampleinsertcommand_vtable.execute =
			(psy_fp_command_params)
			psy_audio_sequencesampleinsertcommand_execute;
		psy_audio_sequencesampleinsertcommand_vtable.revert =
			(psy_fp_command)
			psy_audio_sequencesampleinsertcommand_revert;
		psy_audio_sequencesampleinsertcommand_vtable_initialized = TRUE;
	}
}
// implementation
psy_audio_SequenceSampleInsertCommand* psy_audio_sequencesampleinsertcommand_alloc(
	psy_audio_Sequence* sequence, psy_audio_SequenceSelection* selection,
	psy_audio_OrderIndex index, psy_audio_SampleIndex sampleindex)
{
	psy_audio_SequenceSampleInsertCommand* rv;

	rv = malloc(sizeof(psy_audio_SequenceSampleInsertCommand));
	if (rv) {
		psy_command_init(&rv->command);
		psy_audio_sequencesampleinsertcommand_vtable_init(rv);
		rv->command.vtable = &psy_audio_sequencesampleinsertcommand_vtable;
		rv->sequence = sequence;
		rv->selection = selection;
		psy_audio_sequenceselection_init(&rv->restoreselection);
		rv->index = index;
		rv->sampleindex = sampleindex;
	}
	return rv;
}

void psy_audio_sequencesampleinsertcommand_dispose(psy_audio_SequenceSampleInsertCommand* self)
{
	psy_audio_sequenceselection_dispose(&self->restoreselection);	
}

void psy_audio_sequencesampleinsertcommand_execute(psy_audio_SequenceSampleInsertCommand* self,
	psy_Property* params)
{
	psy_audio_sequenceselection_copy(&self->restoreselection, self->selection);
	psy_audio_sequence_insert_sample(self->sequence, self->index, self->sampleindex);
	self->index = psy_audio_orderindex_make(
		self->index.track, self->index.order + 1);
	psy_audio_sequenceselection_select_first(self->selection, self->index);
	// psy_audio_sequenceselection_seteditposition(self->selection, self->index);
}

void psy_audio_sequencesampleinsertcommand_revert(psy_audio_SequenceSampleInsertCommand* self)
{
	psy_audio_sequence_remove(self->sequence, self->index);
	self->index = psy_audio_orderindex_make(
		self->index.track, self->index.order - 1);
	psy_audio_sequenceselection_copy(self->selection, &self->restoreselection);
	psy_audio_sequenceselection_select_first(self->selection, self->index);
	// psy_audio_sequenceselection_seteditposition(self->selection, self->index);
}

/*
** psy_audio_SequenceMarkerInsertCommand
*/
/* prototypes */
static void psy_audio_sequencemarkerinsertcommand_dispose(psy_audio_SequenceMarkerInsertCommand*);
static void psy_audio_sequencemarkerinsertcommand_execute(psy_audio_SequenceMarkerInsertCommand*,
	psy_Property* params);
static void psy_audio_sequencemarkerinsertcommand_revert(psy_audio_SequenceMarkerInsertCommand*);
/* vtable */
static psy_CommandVtable psy_audio_sequencemarkerinsertcommand_vtable;
static bool psy_audio_sequencemarkerinsertcommand_vtable_initialized = FALSE;

static void psy_audio_sequencemarkerinsertcommand_vtable_init(psy_audio_SequenceMarkerInsertCommand* self)
{
	if (!psy_audio_sequencemarkerinsertcommand_vtable_initialized) {
		psy_audio_sequencemarkerinsertcommand_vtable = *(self->command.vtable);
		psy_audio_sequencemarkerinsertcommand_vtable.dispose =
			(psy_fp_command)
			psy_audio_sequencemarkerinsertcommand_dispose;
		psy_audio_sequencemarkerinsertcommand_vtable.execute =
			(psy_fp_command_params)
			psy_audio_sequencemarkerinsertcommand_execute;
		psy_audio_sequencemarkerinsertcommand_vtable.revert =
			(psy_fp_command)
			psy_audio_sequencemarkerinsertcommand_revert;
		psy_audio_sequencemarkerinsertcommand_vtable_initialized = TRUE;
	}
}

/* implementation */
psy_audio_SequenceMarkerInsertCommand* psy_audio_sequencemarkerinsertcommand_alloc(
	psy_audio_Sequence* sequence, psy_audio_SequenceSelection* selection,
	psy_audio_OrderIndex index, const char* text)
{
	psy_audio_SequenceMarkerInsertCommand* rv;

	rv = malloc(sizeof(psy_audio_SequenceMarkerInsertCommand));
	if (rv) {
		psy_command_init(&rv->command);
		psy_audio_sequencemarkerinsertcommand_vtable_init(rv);
		rv->command.vtable = &psy_audio_sequencemarkerinsertcommand_vtable;
		rv->sequence = sequence;
		rv->selection = selection;
		psy_audio_sequenceselection_init(&rv->restoreselection);
		rv->index = index;
		rv->text = psy_strdup(text);
	}
	return rv;
}

void psy_audio_sequencemarkerinsertcommand_dispose(psy_audio_SequenceMarkerInsertCommand* self)
{
	psy_audio_sequenceselection_dispose(&self->restoreselection);
	free(self->text);
	self->text = NULL;
}

void psy_audio_sequencemarkerinsertcommand_execute(psy_audio_SequenceMarkerInsertCommand* self,
	psy_Property* params)
{
	psy_audio_sequenceselection_copy(&self->restoreselection, self->selection);
	psy_audio_sequence_insert_marker(self->sequence, self->index, self->text);
	self->index = psy_audio_orderindex_make(
		self->index.track, self->index.order + 1);
	psy_audio_sequenceselection_select_first(self->selection, self->index);
}

void psy_audio_sequencemarkerinsertcommand_revert(psy_audio_SequenceMarkerInsertCommand* self)
{
	psy_audio_sequence_remove(self->sequence, self->index);
	self->index = psy_audio_orderindex_make(
		self->index.track, self->index.order - 1);
	psy_audio_sequenceselection_copy(self->selection, &self->restoreselection);
	psy_audio_sequenceselection_select_first(self->selection, self->index);
}

// psy_audio_SequenceRemoveCommand
// prototypes
static void psy_audio_sequenceremovecommand_dispose(psy_audio_SequenceRemoveCommand*);
static void psy_audio_sequenceremovecommand_execute(psy_audio_SequenceRemoveCommand*,
	psy_Property* params);
static void psy_audio_sequenceremovecommand_revert(psy_audio_SequenceRemoveCommand*);
// vtable
static psy_CommandVtable psy_audio_sequenceremovecommand_vtable;
static bool psy_audio_sequenceremovecommand_vtable_initialized = FALSE;

static void psy_audio_sequenceremovecommand_vtable_init(psy_audio_SequenceRemoveCommand* self)
{
	if (!psy_audio_sequenceremovecommand_vtable_initialized) {
		psy_audio_sequenceremovecommand_vtable = *(self->command.vtable);
		psy_audio_sequenceremovecommand_vtable.dispose =
			(psy_fp_command)
			psy_audio_sequenceremovecommand_dispose;
		psy_audio_sequenceremovecommand_vtable.execute =
			(psy_fp_command_params)
			psy_audio_sequenceremovecommand_execute;
		psy_audio_sequenceremovecommand_vtable.revert =
			(psy_fp_command)
			psy_audio_sequenceremovecommand_revert;
		psy_audio_sequenceremovecommand_vtable_initialized = TRUE;
	}
}
// implementation
psy_audio_SequenceRemoveCommand* psy_audio_sequenceremovecommand_alloc(
	psy_audio_Sequence* sequence, psy_audio_SequenceSelection* selection)
{
	psy_audio_SequenceRemoveCommand* rv;

	rv = malloc(sizeof(psy_audio_SequenceRemoveCommand));
	if (rv) {
		psy_command_init(&rv->command);
		psy_audio_sequenceremovecommand_vtable_init(rv);
		rv->command.vtable = &psy_audio_sequenceremovecommand_vtable;
		rv->sequence = sequence;
		rv->selection = selection;
		psy_audio_sequenceselection_init(&rv->restoreselection);		
		psy_audio_sequence_init(&rv->restoresequence, sequence->patterns, sequence->samples);		
	}
	return rv;
}

void psy_audio_sequenceremovecommand_dispose(psy_audio_SequenceRemoveCommand* self)
{
	psy_audio_sequenceselection_dispose(&self->restoreselection);
	psy_audio_sequence_dispose(&self->restoresequence);
}

void psy_audio_sequenceremovecommand_execute(psy_audio_SequenceRemoveCommand* self,
	psy_Property* params)
{
	psy_audio_OrderIndex editposition;	
	psy_audio_SequenceCursor cursor;
	psy_dsp_big_beat_t pattern_offset;

	assert(self);
	assert(self->sequence);

	pattern_offset = psy_audio_sequencecursor_offset(&self->sequence->cursor);
	cursor = psy_audio_sequence_cursor(self->sequence);
	psy_audio_sequenceselection_copy(&self->restoreselection, self->selection);
	psy_audio_sequence_copy(&self->restoresequence, self->sequence);	
	psy_audio_exclusivelock_enter();
	editposition = psy_audio_sequenceselection_first(self->selection);	
	psy_audio_sequence_remove_selection(self->sequence, self->selection);	
	if (psy_audio_sequence_track_size(self->sequence, 0) == 0) {
		psy_audio_sequence_insert(self->sequence,
			psy_audio_orderindex_make(0, 0), 0);
	}
	psy_audio_exclusivelock_leave();
	psy_audio_sequenceselection_clear(self->selection);
	if (editposition.order >= psy_audio_sequence_track_size(self->sequence,
			editposition.track)) {
		editposition.order = psy_audio_sequence_track_size(self->sequence,
			editposition.track);
		if (editposition.order > 0) {
			--editposition.order;
		}
	}
	psy_audio_sequenceselection_select_first(self->selection, editposition);
	psy_audio_sequencecursor_set_order_index(&cursor, editposition);	
	psy_audio_sequencecursor_set_offset(&cursor, pattern_offset);
	psy_audio_sequence_set_cursor(self->sequence, cursor);	
}

void psy_audio_sequenceremovecommand_revert(psy_audio_SequenceRemoveCommand* self)
{
	psy_audio_exclusivelock_enter();	
	psy_audio_sequence_copy(self->sequence, &self->restoresequence);
	psy_audio_sequenceselection_copy(self->selection, &self->restoreselection);
//	psy_audio_sequenceselection_update(self->selection);		
	psy_audio_exclusivelock_leave();
}

// psy_audio_SequenceClearCommand
// prototypes
static void psy_audio_sequenceclearcommand_dispose(psy_audio_SequenceClearCommand*);
static void psy_audio_sequenceclearcommand_execute(psy_audio_SequenceClearCommand*,
	psy_Property* params);
static void psy_audio_sequenceclearcommand_revert(psy_audio_SequenceClearCommand*);
// vtable
static psy_CommandVtable psy_audio_sequenceclearcommand_vtable;
static bool psy_audio_sequenceclearcommand_vtable_initialized = FALSE;

static void psy_audio_sequenceclearcommand_vtable_init(psy_audio_SequenceClearCommand* self)
{
	if (!psy_audio_sequenceclearcommand_vtable_initialized) {
		psy_audio_sequenceclearcommand_vtable = *(self->command.vtable);
		psy_audio_sequenceclearcommand_vtable.dispose =
			(psy_fp_command)
			psy_audio_sequenceclearcommand_dispose;
		psy_audio_sequenceclearcommand_vtable.execute =
			(psy_fp_command_params)
			psy_audio_sequenceclearcommand_execute;
		psy_audio_sequenceclearcommand_vtable.revert =
			(psy_fp_command)
			psy_audio_sequenceclearcommand_revert;
		psy_audio_sequenceclearcommand_vtable_initialized = TRUE;
	}
	self->command.vtable = &psy_audio_sequenceclearcommand_vtable;
}

/* implementation */
psy_audio_SequenceClearCommand* psy_audio_sequenceclearcommand_alloc(
	psy_audio_Sequence* sequence, psy_audio_SequenceSelection* selection)
{
	psy_audio_SequenceClearCommand* rv;

	rv = malloc(sizeof(psy_audio_SequenceClearCommand));
	if (rv) {
		psy_command_init(&rv->command);
		psy_audio_sequenceclearcommand_vtable_init(rv);		
		rv->sequence = sequence;
		rv->selection = selection;
		psy_audio_sequenceselection_init(&rv->restoreselection);
		psy_audio_sequence_init(&rv->restoresequence, sequence->patterns, sequence->samples);
	}
	return rv;
}

void psy_audio_sequenceclearcommand_dispose(psy_audio_SequenceClearCommand* self)
{
	psy_audio_sequenceselection_dispose(&self->restoreselection);
	psy_audio_sequence_dispose(&self->restoresequence);	
}

void psy_audio_sequenceclearcommand_execute(psy_audio_SequenceClearCommand* self,
	psy_Property* params)
{	
	psy_audio_SequenceCursor cursor;

	psy_audio_sequencecursor_init(&cursor);
	psy_audio_sequence_set_cursor(self->sequence, cursor);
	psy_audio_exclusivelock_enter();
	psy_audio_sequenceselection_copy(&self->restoreselection, self->selection);
	psy_audio_sequence_copy(&self->restoresequence, self->sequence);
	psy_audio_sequence_clear(self->sequence);
	psy_audio_sequence_append_track(self->sequence,
		psy_audio_sequencetrack_allocinit());
	psy_audio_sequence_insert(self->sequence,
		psy_audio_orderindex_make(0, 0), 0);
	psy_audio_sequence_set_cursor(self->sequence, cursor);
	psy_audio_exclusivelock_leave();
}

void psy_audio_sequenceclearcommand_revert(psy_audio_SequenceClearCommand* self)
{	
	psy_audio_exclusivelock_enter();
	psy_audio_sequenceselection_copy(self->selection, &self->restoreselection);
	psy_audio_sequence_copy(self->sequence, &self->restoresequence);
	psy_audio_sequence_resetpatterns(self->sequence);
	psy_audio_sequenceselection_select_first(self->selection,
		psy_audio_sequenceselection_first(self->selection));
	psy_audio_exclusivelock_leave();
}

/* psy_audio_SequenceChangePatternCommand */
/* prototypes */
static void psy_audio_sequencechangepatterncommand_dispose(
	psy_audio_SequenceChangePatternCommand*);
static void psy_audio_sequencechangepatterncommand_execute(
	psy_audio_SequenceChangePatternCommand*, psy_Property* params);
static void psy_audio_sequencechangepatterncommand_revert(
	psy_audio_SequenceChangePatternCommand*);

/* vtable */
static psy_CommandVtable psy_audio_sequencechangepatterncommand_vtable;
static bool psy_audio_sequencechangepatterncommand_vtable_initialized = FALSE;

static void psy_audio_sequencechangepatterncommand_vtable_init(
	psy_audio_SequenceChangePatternCommand* self)
{
	if (!psy_audio_sequencechangepatterncommand_vtable_initialized) {
		psy_audio_sequencechangepatterncommand_vtable = *(self->command.vtable);
		psy_audio_sequencechangepatterncommand_vtable.dispose = 
			(psy_fp_command)
			psy_audio_sequencechangepatterncommand_dispose;
		psy_audio_sequencechangepatterncommand_vtable.execute =
			(psy_fp_command_params)
			psy_audio_sequencechangepatterncommand_execute;
		psy_audio_sequencechangepatterncommand_vtable.revert =
			(psy_fp_command)
			psy_audio_sequencechangepatterncommand_revert;
		psy_audio_sequencechangepatterncommand_vtable_initialized = TRUE;
	}
	self->command.vtable = &psy_audio_sequencechangepatterncommand_vtable;
}

/* implementation */
psy_audio_SequenceChangePatternCommand* psy_audio_sequencechangepatterncommand_alloc(
	psy_audio_Sequence* sequence, psy_audio_SequenceSelection* selection, intptr_t step)
{
	psy_audio_SequenceChangePatternCommand* rv;

	rv = malloc(sizeof(psy_audio_SequenceChangePatternCommand));
	if (rv) {
		psy_command_init(&rv->command);
		psy_audio_sequencechangepatterncommand_vtable_init(rv);		
		rv->sequence = sequence;
		rv->selection = selection;
		rv->step = step;
		rv->success = FALSE;
		psy_audio_sequenceselection_init(&rv->restoreselection);
		psy_audio_sequence_init(&rv->restoresequence, sequence->patterns,
			sequence->samples);
	}
	return rv;
}

void psy_audio_sequencechangepatterncommand_dispose(
	psy_audio_SequenceChangePatternCommand* self)
{
	psy_audio_sequenceselection_dispose(&self->restoreselection);
	psy_audio_sequence_dispose(&self->restoresequence);	
}

void psy_audio_sequencechangepatterncommand_execute(
	psy_audio_SequenceChangePatternCommand* self,
	psy_Property* params)
{		
	psy_audio_SequenceSelectionIterator ite;

	assert(self);

	self->success = FALSE;
	psy_audio_sequenceselection_copy(&self->restoreselection, self->selection);
	psy_audio_sequence_copy(&self->restoresequence, self->sequence);
	psy_audio_exclusivelock_enter();	
	ite = psy_audio_sequenceselection_begin(self->selection);
	for (; ite != NULL; psy_list_next(&ite)) {
		psy_audio_OrderIndex* orderindex;
		psy_audio_SequenceEntry* seqentry;
		psy_audio_SequencePatternEntry* seqpatternentry;

		orderindex = ite->entry;
		assert(orderindex);
		seqentry = psy_audio_sequence_entry(self->sequence, *orderindex);		
		if (!seqentry || seqentry->type != psy_audio_SEQUENCEENTRY_PATTERN) {
			return;
		}
		seqpatternentry = (psy_audio_SequencePatternEntry*)seqentry;
		if ((self->step > 0 || (self->step < 0 &&
				seqpatternentry->patternslot >= (uintptr_t)(self->step * (-1))))) {
			psy_audio_SequenceCursor cursor;

			cursor = self->sequence->cursor;
			psy_audio_sequence_setpatternindex(self->sequence,
				*orderindex, seqpatternentry->patternslot + self->step);			
			psy_audio_sequence_set_cursor(self->sequence, cursor);
			self->success = TRUE;
		}
	}	
	psy_audio_exclusivelock_leave();	
}

void psy_audio_sequencechangepatterncommand_revert(
	psy_audio_SequenceChangePatternCommand* self)
{	
	psy_audio_exclusivelock_enter();
	if (self->success) {
		psy_audio_sequence_copy(self->sequence, &self->restoresequence);
		psy_audio_sequenceselection_copy(self->selection, &self->restoreselection);
//		psy_audio_sequenceselection_update(self->selection);
		self->success = FALSE;
	}
	psy_audio_exclusivelock_leave();
}


/* psy_audio_PatternEntryRemoveCommand */

/* prototypes */
static void psy_audio_patternentryremovecommand_dispose(
	psy_audio_PatternEntryRemoveCommand* self) { }
static void psy_audio_patternentryremovecommand_execute(
	psy_audio_PatternEntryRemoveCommand*, psy_Property* params);
static void psy_audio_patternentryremovecommand_revert(
	psy_audio_PatternEntryRemoveCommand* self) { }

/* vtable */
static psy_CommandVtable psy_audio_patternentryremovecommand_vtable;
static bool psy_audio_patternentryremovecommand_vtable_initialized = FALSE;

static void psy_audio_patternentryremovecommand_vtable_init(
	psy_audio_PatternEntryRemoveCommand* self)
{
	if (!psy_audio_patternentryremovecommand_vtable_initialized) {
		psy_audio_patternentryremovecommand_vtable = *(self->command.vtable);
		psy_audio_patternentryremovecommand_vtable.dispose =
			(psy_fp_command)
			psy_audio_patternentryremovecommand_dispose;
		psy_audio_patternentryremovecommand_vtable.execute =
			(psy_fp_command_params)
			psy_audio_patternentryremovecommand_execute;
		psy_audio_patternentryremovecommand_vtable.revert =
			(psy_fp_command)
			psy_audio_patternentryremovecommand_revert;
		psy_audio_patternentryremovecommand_vtable_initialized = TRUE;
	}
	self->command.vtable = &psy_audio_patternentryremovecommand_vtable;
}

void psy_audio_patternentryremovecommand_init(
	psy_audio_PatternEntryRemoveCommand* self)
{
	psy_command_init(&self->command);
	psy_audio_patternentryremovecommand_vtable_init(self);
}

void psy_audio_patternentryremovecommand_execute(
	psy_audio_PatternEntryRemoveCommand* self, psy_Property* params)
{
	psy_audio_PatternNode* node;
	psy_audio_Pattern* pattern;

	assert(self);
	assert(params);

	node = (psy_audio_PatternNode*)psy_property_at_int(params, "node", 0);
	pattern = (psy_audio_Pattern*)psy_property_at_int(params, "pattern", 0);
	if (pattern && node) {
		psy_audio_pattern_remove(pattern, node);
	}
}

/* psy_audio_PatternEntryTransposeCommand */

/* prototypes */
static void psy_audio_patternentrytransposecommand_dispose(
	psy_audio_PatternEntryTransposeCommand* self);
static void psy_audio_patternentrytransposecommand_execute(
	psy_audio_PatternEntryTransposeCommand*, psy_Property* params);
static void psy_audio_patternentrytransposecommand_revert(
	psy_audio_PatternEntryTransposeCommand* self) { }

/* vtable */
static psy_CommandVtable psy_audio_patternentrytranspose_vtable;
static bool psy_audio_patternentrytranspose_vtable_initialized = FALSE;

static void psy_audio_patternentrytranspose_vtable_init(
	psy_audio_PatternEntryTransposeCommand* self)
{
	if (!psy_audio_patternentrytranspose_vtable_initialized) {
		psy_audio_patternentrytranspose_vtable = *(self->command.vtable);
		psy_audio_patternentrytranspose_vtable.dispose =
			(psy_fp_command)
			psy_audio_patternentrytransposecommand_dispose;
		psy_audio_patternentrytranspose_vtable.execute =
			(psy_fp_command_params)
			psy_audio_patternentrytransposecommand_execute;
		psy_audio_patternentrytranspose_vtable.revert =
			(psy_fp_command)
			psy_audio_patternentrytransposecommand_revert;
		psy_audio_patternentrytranspose_vtable_initialized = TRUE;
	}
	self->command.vtable = &psy_audio_patternentrytranspose_vtable;
}

void psy_audio_patternentrytransposecommand_init(
	psy_audio_PatternEntryTransposeCommand* self,
	intptr_t offset)
{
	psy_command_init(&self->command);
	psy_audio_patternentrytranspose_vtable_init(self);
	self->offset = offset;
	psy_table_init(&self->nodemark);
}

void psy_audio_patternentrytransposecommand_dispose(
	psy_audio_PatternEntryTransposeCommand* self)
{
	psy_table_dispose(&self->nodemark);
}

void psy_audio_patternentrytransposecommand_execute(
	psy_audio_PatternEntryTransposeCommand* self, psy_Property* params)
{
	psy_audio_PatternNode* node;
	psy_audio_Pattern* pattern;	

	assert(self);
	assert(params);

	node = (psy_audio_PatternNode*)psy_property_at_int(params, "node", 0);
	pattern = (psy_audio_Pattern*)psy_property_at_int(params, "pattern", 0);
	if (!psy_table_exists(&self->nodemark, (uintptr_t)node)) {
		psy_audio_PatternEntry* entry;

		entry = (psy_audio_PatternEntry*)psy_list_entry(node);
		if (psy_audio_patternentry_front(entry)->note < psy_audio_NOTECOMMANDS_RELEASE) {
			if (psy_audio_patternentry_front(entry)->note + self->offset < 0) {
				psy_audio_patternentry_front(entry)->note = 0;
			} else {
				psy_audio_patternentry_front(entry)->note = (uint8_t)
					psy_min(255, psy_audio_patternentry_front(entry)->note + 
						self->offset);
			}
			if (psy_audio_patternentry_front(entry)->note >= psy_audio_NOTECOMMANDS_RELEASE) {
				psy_audio_patternentry_front(entry)->note = psy_audio_NOTECOMMANDS_RELEASE - 1;
			}
		}
		psy_table_insert(&self->nodemark, (uintptr_t)node, 0);
	}
}

/* psy_audio_PatternEntryCopyCommand */

/* prototypes */
static void psy_audio_patternentrycopycommand_dispose(
	psy_audio_PatternEntryCopyCommand* self) { }
static void psy_audio_patternentrycopycommand_execute(
	psy_audio_PatternEntryCopyCommand*, psy_Property* params);
static void psy_audio_patternentrycopycommand_revert(
	psy_audio_PatternEntryCopyCommand* self) { }

/* vtable */
static psy_CommandVtable psy_audio_patternentrycopycommand_vtable;
static bool psy_audio_patternentrycopycommand_vtable_initialized = FALSE;

static void psy_audio_patternentrycopycommand_vtable_init(
	psy_audio_PatternEntryCopyCommand* self)
{
	if (!psy_audio_patternentrycopycommand_vtable_initialized) {
		psy_audio_patternentrycopycommand_vtable = *(self->command.vtable);
		psy_audio_patternentrycopycommand_vtable.dispose =
			(psy_fp_command)
			psy_audio_patternentrycopycommand_dispose;
		psy_audio_patternentrycopycommand_vtable.execute =
			(psy_fp_command_params)
			psy_audio_patternentrycopycommand_execute;
		psy_audio_patternentrycopycommand_vtable.revert =
			(psy_fp_command)
			psy_audio_patternentrycopycommand_revert;
		psy_audio_patternentrycopycommand_vtable_initialized = TRUE;
	}
	self->command.vtable = &psy_audio_patternentrycopycommand_vtable;
}

void psy_audio_patternentrycopycommand_init(
	psy_audio_PatternEntryCopyCommand* self,
	psy_audio_Pattern* dest,
	psy_dsp_big_beat_t offset,
	intptr_t trackoffset)
{
	psy_command_init(&self->command);
	psy_audio_patternentrycopycommand_vtable_init(self);
	self->dest = dest;
	self->prev = 0;
	self->offset = offset;
	self->trackoffset = trackoffset;
}

void psy_audio_patternentrycopycommand_execute(
	psy_audio_PatternEntryCopyCommand* self, psy_Property* params)
{
	psy_audio_PatternNode* node;
	psy_audio_Pattern* pattern;
	
	assert(self);
	assert(params);

	node = (psy_audio_PatternNode*)psy_property_at_int(params, "node", 0);
	pattern = (psy_audio_Pattern*)psy_property_at_int(params, "pattern", 0);
	if (pattern && node) {
		psy_audio_PatternEntry* entry;	
				
		entry = psy_audio_patternnode_entry(node);
		self->prev = psy_audio_pattern_insert(self->dest,
			self->prev, entry->track - self->trackoffset,
			entry->offset - self->offset,
			psy_audio_patternentry_front(entry));
	}
}

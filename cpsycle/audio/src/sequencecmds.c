// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "sequencecmds.h"
// local
#include "exclusivelock.h"

// psy_audio_SequenceInsertCommand
// prototypes
static void psy_audio_sequenceinsertcommand_dispose(psy_audio_SequenceInsertCommand*);
static void psy_audio_sequenceinsertcommand_execute(psy_audio_SequenceInsertCommand*);
static void psy_audio_sequenceinsertcommand_revert(psy_audio_SequenceInsertCommand*);
// vtable
static psy_CommandVtable psy_audio_sequenceinsertcommand_vtable;
static bool psy_audio_sequenceinsertcommand_vtable_initialized = FALSE;

static void psy_audio_sequenceinsertcommand_vtable_init(psy_audio_SequenceInsertCommand* self)
{
	if (!psy_audio_sequenceinsertcommand_vtable_initialized) {
		psy_audio_sequenceinsertcommand_vtable = *(self->command.vtable);
		psy_audio_sequenceinsertcommand_vtable.dispose = (psy_fp_command)psy_audio_sequenceinsertcommand_dispose;
		psy_audio_sequenceinsertcommand_vtable.execute = (psy_fp_command)psy_audio_sequenceinsertcommand_execute;
		psy_audio_sequenceinsertcommand_vtable.revert = (psy_fp_command)psy_audio_sequenceinsertcommand_revert;
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

void psy_audio_sequenceinsertcommand_execute(psy_audio_SequenceInsertCommand* self)
{		
	psy_audio_sequenceselection_copy(&self->restoreselection, self->selection);
	psy_audio_sequence_insert(self->sequence, self->index, self->patidx);	
	self->index = psy_audio_orderindex_make(
		self->index.track, self->index.order + 1);
	psy_audio_sequenceselection_seteditposition(self->selection, self->index);
}

void psy_audio_sequenceinsertcommand_revert(psy_audio_SequenceInsertCommand* self)
{
	psy_audio_sequence_remove(self->sequence, self->index);
	self->index = psy_audio_orderindex_make(
		self->index.track, self->index.order - 1);
	psy_audio_sequenceselection_copy(self->selection, &self->restoreselection);
	psy_audio_sequenceselection_seteditposition(self->selection, self->index);	
}

// psy_audio_SequenceRemoveCommand
// prototypes
static void psy_audio_sequenceremovecommand_dispose(psy_audio_SequenceRemoveCommand*);
static void psy_audio_sequenceremovecommand_execute(psy_audio_SequenceRemoveCommand*);
static void psy_audio_sequenceremovecommand_revert(psy_audio_SequenceRemoveCommand*);
// vtable
static psy_CommandVtable psy_audio_sequenceremovecommand_vtable;
static bool psy_audio_sequenceremovecommand_vtable_initialized = FALSE;

static void psy_audio_sequenceremovecommand_vtable_init(psy_audio_SequenceRemoveCommand* self)
{
	if (!psy_audio_sequenceremovecommand_vtable_initialized) {
		psy_audio_sequenceremovecommand_vtable = *(self->command.vtable);
		psy_audio_sequenceremovecommand_vtable.dispose = (psy_fp_command)psy_audio_sequenceremovecommand_dispose;
		psy_audio_sequenceremovecommand_vtable.execute = (psy_fp_command)psy_audio_sequenceremovecommand_execute;
		psy_audio_sequenceremovecommand_vtable.revert = (psy_fp_command)psy_audio_sequenceremovecommand_revert;
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
		psy_audio_sequence_init(&rv->restoresequence, sequence->patterns);		
	}
	return rv;
}

void psy_audio_sequenceremovecommand_dispose(psy_audio_SequenceRemoveCommand* self)
{
	psy_audio_sequenceselection_dispose(&self->restoreselection);
	psy_audio_sequence_dispose(&self->restoresequence);
}

void psy_audio_sequenceremovecommand_execute(psy_audio_SequenceRemoveCommand* self)
{
	psy_audio_OrderIndex editposition;

	psy_audio_sequenceselection_copy(&self->restoreselection, self->selection);
	psy_audio_sequence_copy(&self->restoresequence, self->sequence);
	psy_audio_exclusivelock_enter();
	editposition = self->selection->editposition;
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
	psy_audio_sequenceselection_seteditposition(self->selection,
		editposition);		
}

void psy_audio_sequenceremovecommand_revert(psy_audio_SequenceRemoveCommand* self)
{
	psy_audio_exclusivelock_enter();	
	psy_audio_sequence_copy(self->sequence, &self->restoresequence);
	psy_audio_sequenceselection_copy(self->selection, &self->restoreselection);
	psy_audio_sequenceselection_update(self->selection);		
	psy_audio_exclusivelock_leave();
}

// psy_audio_SequenceClearCommand
// prototypes
static void psy_audio_sequenceclearcommand_dispose(psy_audio_SequenceClearCommand*);
static void psy_audio_sequenceclearcommand_execute(psy_audio_SequenceClearCommand*);
static void psy_audio_sequenceclearcommand_revert(psy_audio_SequenceClearCommand*);
// vtable
static psy_CommandVtable psy_audio_sequenceclearcommand_vtable;
static bool psy_audio_sequenceclearcommand_vtable_initialized = FALSE;

static void psy_audio_sequenceclearcommand_vtable_init(psy_audio_SequenceClearCommand* self)
{
	if (!psy_audio_sequenceclearcommand_vtable_initialized) {
		psy_audio_sequenceclearcommand_vtable = *(self->command.vtable);
		psy_audio_sequenceclearcommand_vtable.dispose = (psy_fp_command)psy_audio_sequenceclearcommand_dispose;
		psy_audio_sequenceclearcommand_vtable.execute = (psy_fp_command)psy_audio_sequenceclearcommand_execute;
		psy_audio_sequenceclearcommand_vtable.revert = (psy_fp_command)psy_audio_sequenceclearcommand_revert;
		psy_audio_sequenceclearcommand_vtable_initialized = TRUE;
	}
}
// implementation
psy_audio_SequenceClearCommand* psy_audio_sequenceclearcommand_alloc(
	psy_audio_Sequence* sequence, psy_audio_SequenceSelection* selection)
{
	psy_audio_SequenceClearCommand* rv;

	rv = malloc(sizeof(psy_audio_SequenceClearCommand));
	if (rv) {
		psy_command_init(&rv->command);
		psy_audio_sequenceclearcommand_vtable_init(rv);
		rv->command.vtable = &psy_audio_sequenceclearcommand_vtable;
		rv->sequence = sequence;
		rv->selection = selection;
		psy_audio_sequenceselection_init(&rv->restoreselection);
		psy_audio_sequence_init(&rv->restoresequence, sequence->patterns);
	}
	return rv;
}

void psy_audio_sequenceclearcommand_dispose(psy_audio_SequenceClearCommand* self)
{
	psy_audio_sequenceselection_dispose(&self->restoreselection);
	psy_audio_sequence_dispose(&self->restoresequence);	
}

void psy_audio_sequenceclearcommand_execute(psy_audio_SequenceClearCommand* self)
{	
	psy_audio_exclusivelock_enter();
	psy_audio_sequenceselection_copy(&self->restoreselection, self->selection);
	psy_audio_sequence_copy(&self->restoresequence, self->sequence);
	psy_audio_sequence_clear(self->sequence);
	psy_audio_sequence_appendtrack(self->sequence,
		psy_audio_sequencetrack_allocinit());
	psy_audio_sequence_insert(self->sequence,
		psy_audio_orderindex_make(0, 0), 0);
	psy_audio_sequenceselection_seteditposition(self->selection, 
		psy_audio_orderindex_make(0, 0));
	psy_audio_exclusivelock_leave();
}

void psy_audio_sequenceclearcommand_revert(psy_audio_SequenceClearCommand* self)
{	
	psy_audio_exclusivelock_enter();
	psy_audio_sequenceselection_copy(self->selection, &self->restoreselection);
	psy_audio_sequence_copy(self->sequence, &self->restoresequence);
	psy_audio_sequence_resetpatterns(self->sequence);
	psy_audio_sequenceselection_seteditposition(self->selection,
		self->selection->editposition);
	psy_audio_exclusivelock_leave();
}

// psy_audio_SequenceChangePatternCommand
// prototypes
static void psy_audio_sequencechangepatterncommand_dispose(
	psy_audio_SequenceChangePatternCommand*);
static void psy_audio_sequencechangepatterncommand_execute(
	psy_audio_SequenceChangePatternCommand*);
static void psy_audio_sequencechangepatterncommand_revert(
	psy_audio_SequenceChangePatternCommand*);
// vtable
static psy_CommandVtable psy_audio_sequencechangepatterncommand_vtable;
static bool psy_audio_sequencechangepatterncommand_vtable_initialized = FALSE;

static void psy_audio_sequencechangepatterncommand_vtable_init(
	psy_audio_SequenceChangePatternCommand* self)
{
	if (!psy_audio_sequencechangepatterncommand_vtable_initialized) {
		psy_audio_sequencechangepatterncommand_vtable = *(self->command.vtable);
		psy_audio_sequencechangepatterncommand_vtable.dispose = 
			(psy_fp_command)psy_audio_sequencechangepatterncommand_dispose;
		psy_audio_sequencechangepatterncommand_vtable.execute =
			(psy_fp_command)psy_audio_sequencechangepatterncommand_execute;
		psy_audio_sequencechangepatterncommand_vtable.revert =
			(psy_fp_command)psy_audio_sequencechangepatterncommand_revert;
		psy_audio_sequencechangepatterncommand_vtable_initialized = TRUE;
	}
}
// implementation
psy_audio_SequenceChangePatternCommand* psy_audio_sequencechangepatterncommand_alloc(
	psy_audio_Sequence* sequence, psy_audio_SequenceSelection* selection, intptr_t step)
{
	psy_audio_SequenceChangePatternCommand* rv;

	rv = malloc(sizeof(psy_audio_SequenceChangePatternCommand));
	if (rv) {
		psy_command_init(&rv->command);
		psy_audio_sequencechangepatterncommand_vtable_init(rv);
		rv->command.vtable = &psy_audio_sequencechangepatterncommand_vtable;
		rv->sequence = sequence;
		rv->selection = selection;
		rv->step = step;
		rv->success = FALSE;
		psy_audio_sequenceselection_init(&rv->restoreselection);
		psy_audio_sequence_init(&rv->restoresequence, sequence->patterns);
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
	psy_audio_SequenceChangePatternCommand* self)
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
		psy_audio_SequenceEntry* entry;

		orderindex = ite->entry;
		assert(orderindex);
		entry = psy_audio_sequence_entry(self->sequence, *orderindex);
		if (entry && (self->step > 0 ||
				(self->step < 0 &&
				entry->patternslot >= (uintptr_t)(self->step * (-1))))) {
			psy_audio_sequence_setpatternindex(self->sequence,
				*orderindex, entry->patternslot + self->step);
			self->success = TRUE;
		}
	}
	
	psy_audio_exclusivelock_leave();
	psy_audio_sequenceselection_update(self->selection);
}

void psy_audio_sequencechangepatterncommand_revert(
	psy_audio_SequenceChangePatternCommand* self)
{	
	psy_audio_exclusivelock_enter();
	if (self->success) {
		psy_audio_sequence_copy(self->sequence, &self->restoresequence);
		psy_audio_sequenceselection_copy(self->selection, &self->restoreselection);
		psy_audio_sequenceselection_update(self->selection);
		self->success = FALSE;
	}
	psy_audio_exclusivelock_leave();
}

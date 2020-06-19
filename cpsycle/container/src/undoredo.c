// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "undoredo.h"

#include <stdlib.h> 

static void psy_undoredo_clear_redo(psy_UndoRedo*);
static void psy_undoredo_clear_undo(psy_UndoRedo*);
static void psy_undoredo_clear_list(psy_List* list);
static psy_Command* psy_undoredo_swap(psy_List** first, psy_List** second);

void psy_undoredo_init(psy_UndoRedo* self)
{
	self->undo = NULL;
	self->redo = NULL;
}

void psy_undoredo_dispose(psy_UndoRedo* self)
{
	psy_undoredo_clear_undo(self);
	psy_undoredo_clear_redo(self);
}

void psy_undoredo_undo(psy_UndoRedo* self)
{	
	psy_Command* command;

	command = psy_undoredo_swap(&self->undo, &self->redo);
	if (command) {
		psy_command_revert(command);
	}
}

void psy_undoredo_redo(psy_UndoRedo* self)
{	
	psy_Command* command;
	
	command = psy_undoredo_swap(&self->redo, &self->undo);
	if (command) {
		psy_command_execute(command);
	}	
}

psy_Command* psy_undoredo_swap(psy_List** first, psy_List** second)
{
	if (*first) {		
		psy_List* last;

		last = psy_list_last(*first);
		if (last) {
			psy_Command* rv;

			rv = last->entry;
			psy_list_append(second, rv);
			psy_list_remove(first, last);
			return rv;
		}
	} 
	return NULL;
}

void psy_undoredo_execute(psy_UndoRedo* self, psy_Command* command)
{		
	psy_list_append(&self->undo, command);	
	psy_command_execute(command);
	psy_undoredo_clear_redo(self);
}

void psy_undoredo_clear_undo(psy_UndoRedo* self)
{
	psy_undoredo_clear_list(self->undo);
	self->undo = NULL;
}

void psy_undoredo_clear_redo(psy_UndoRedo* self)
{
	psy_undoredo_clear_list(self->redo);
	self->redo = NULL;
}

void psy_undoredo_clear_list(psy_List* list)
{
	psy_List* p;

	for (p = list; p != NULL; p = p->next) {
		psy_Command* command;

		command = (psy_Command*) p->entry;
		psy_command_dispose(command);		
		free(command);
	}
	psy_list_free(list);	
}

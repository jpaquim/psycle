// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "undoredo.h"
#include <stdlib.h> 

static void clear_redo(UndoRedo*);
static void clear_undo(UndoRedo*);
static void clear_list(psy_List* list);
static Command* swap(psy_List** first, psy_List** second);

void undoredo_init(UndoRedo* self)
{
	self->undo = 0;
	self->redo = 0;
}

void undoredo_dispose(UndoRedo* self)
{
	clear_undo(self);
	clear_redo(self);
}

void undoredo_undo(UndoRedo* self)
{	
	Command* command;

	command = swap(&self->undo, &self->redo);
	if (command) {
		command->revert(command);
	}
}

void undoredo_redo(UndoRedo* self)
{	
	Command* command;
	
	command = swap(&self->redo, &self->undo);
	if (command) {
		command->execute(command);
	}	
}

Command* swap(psy_List** first, psy_List** second)
{
	if (*first) {		
		psy_List* last;

		last = psy_list_last(*first);
		if (last) {
			Command* rv;

			rv = last->entry;
			psy_list_append(second, rv);
			psy_list_remove(first, last);
			return rv;
		}
	} 
	return 0;
}

void undoredo_execute(UndoRedo* self, Command* command)
{	
	psy_list_append(&self->undo, command);	
	command->execute(command);
	clear_redo(self);
}

void clear_undo(UndoRedo* self)
{
	clear_list(self->undo);
	self->undo = 0;
}

void clear_redo(UndoRedo* self)
{
	clear_list(self->redo);
	self->redo = 0;
}

void clear_list(psy_List* list)
{
	psy_List* p;

	for (p = list; p != NULL; p = p->next) {
		Command* command;

		command = (Command*) p->entry;
		command->dispose(command);		
		free(command);
	}
	psy_list_free(list);	
}

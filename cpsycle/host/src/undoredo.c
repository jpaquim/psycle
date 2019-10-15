// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "undoredo.h"
#include <stdlib.h> 

static void clear_redo(UndoRedo*);
static void clear_undo(UndoRedo*);
static void clear_list(List* list);
static Command* swap(List** first, List** second);

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

Command* swap(List** first, List** second)
{
	Command* rv;

	if (*first) {		
		rv = (Command*)list_last(*first)->entry;
		list_append(second, rv);
		list_remove(first, list_last(*first));		
	} else {
		rv = 0;
	}
	return rv;
}

void undoredo_execute(UndoRedo* self, Command* command)
{	
	list_append(&self->undo, command);	
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

void clear_list(List* list)
{
	List* p;

	for (p = list; p != 0; p = p->next) {
		Command* command;

		command = (Command*) p->entry;
		command->dispose(command);		
		free(command);
	}
	list_free(list);	
}

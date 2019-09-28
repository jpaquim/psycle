// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#include "undoredo.h"
#include <stdlib.h> 

static void clear_redo(UndoRedo* self);
static void clear_undo(UndoRedo* self);
static void clear_list(List* list);

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
	if (self->undo) {
		List* last;
		Command* command;

		last = self->undo->tail;
		command = (Command*)last->entry;
		command->revert(command);
		if (self->redo == 0) {
			self->redo = list_create(command);
		} else {
			list_append(self->redo, command);		
		}
		list_remove(&self->undo, last);
	}
}

void undoredo_redo(UndoRedo* self)
{
	if (self->redo) {
		List* last;
		Command* command;

		last = self->redo->tail;
		command = (Command*)last->entry;
		command->execute(command);
		if (self->undo == 0) {
			self->undo = list_create(command);
		} else {
			list_append(self->undo, command);		
		}
		list_remove(&self->redo, last);
	}
}

void undoredo_execute(UndoRedo* self, Command* command)
{
	if (self->undo == 0) {
		self->undo = list_create(command);
	} else {
		list_append(self->undo, command);		
	}
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
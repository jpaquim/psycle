// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#if !defined(psy_UNDOREDO_H)
#define psy_UNDOREDO_H

#ifdef __cplusplus
extern "C" {
#endif

#include "command.h"

#include "list.h"
#include "signal.h"

// Command Handler for undo redo commands

typedef struct {
	psy_List* undo;
	psy_List* redo;
} psy_UndoRedo;

void psy_undoredo_init(psy_UndoRedo*);
void psy_undoredo_dispose(psy_UndoRedo*);
void psy_undoredo_undo(psy_UndoRedo*);
void psy_undoredo_redo(psy_UndoRedo*);
void psy_undoredo_execute(psy_UndoRedo*, psy_Command*);

#ifdef __cplusplus
}
#endif

#endif /* psy_UNDOREDO_H */

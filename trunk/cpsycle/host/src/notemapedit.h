// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#if !defined(NOTEMAPEDIT_H)
#define NOTEMAPEDIT_H

#include "uicomponent.h"
#include <event.h>
#include "noteinputs.h"

typedef struct {
	int note;
	int col;
} NoteMapEditCursor;

typedef struct {	
	ui_component component;	
	NoteMapEditCursor cursor;
	PatternEvent map[120];
	NoteInputs* noteinputs;
	int dy;
} NoteMapEdit;

void InitNoteMapEdit(NoteMapEdit*, ui_component* parent);

#endif

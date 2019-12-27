// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#if !defined(UNDOREDOBAR_H)
#define UNDOREDOBAR_H

#include "uibutton.h"
#include "workspace.h"

typedef struct {
	psy_ui_Component component;
	psy_ui_Button undobutton;
	psy_ui_Button redobutton;
	Workspace* workspace;
} UndoRedoBar;

void undoredobar_init(UndoRedoBar*, psy_ui_Component* parent, Workspace*);

#endif

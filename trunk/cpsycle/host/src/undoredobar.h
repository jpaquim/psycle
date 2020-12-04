// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#if !defined(UNDOREDOBAR_H)
#define UNDOREDOBAR_H

// host
#include "workspace.h"
// ui
#include "uibutton.h"

// UndoRedoBar
//
// ToolBar for the workspace undo redo handler.

#ifdef __cplusplus
extern "C" {
#endif

typedef struct UndoRedoBar {
	// inherits
	psy_ui_Component component;
	// ui elements
	psy_ui_Button undobutton;
	psy_ui_Button redobutton;
	// references
	Workspace* workspace;
} UndoRedoBar;

void undoredobar_init(UndoRedoBar*, psy_ui_Component* parent, Workspace*);

INLINE psy_ui_Component* undoredobar_base(UndoRedoBar* self)
{
	assert(self);

	return &self->component;
}

#ifdef __cplusplus
}
#endif

#endif /* UNDOREDOBAR_H */

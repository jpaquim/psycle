// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#if !defined(STEPBOX_H)
#define STEPBOX_H

// host
#include "workspace.h"
// ui
#include "uicombobox.h"
#include "uilabel.h"

#ifdef __cplusplus
extern "C" {
#endif

// PatternCursorStepBox
//
// Pattern Step – Select how many lines the cursor moves after a note is
// entered(0 - 16). To make this apply only for entering notes(not for movement
// with the arrow keys, use the option in the Settingsview

typedef struct PatternCursorStepBox {
	// inherits
	psy_ui_Component component;
	// ui elements
	psy_ui_Label header;
	psy_ui_ComboBox combobox;
	// references
	Workspace* workspace;
} PatternCursorStepBox;

void patterncursorstepbox_init(PatternCursorStepBox*, psy_ui_Component* parent,
	Workspace*);
void patterncursorstepbox_update(PatternCursorStepBox*);

INLINE psy_ui_Component* patterncursorstepbox_base(PatternCursorStepBox* self)
{
	return &self->component;
}

#ifdef __cplusplus
}
#endif

#endif /* STEPBOX_H */

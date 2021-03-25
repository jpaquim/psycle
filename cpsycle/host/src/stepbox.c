// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "stepbox.h"
// platform
#include "../../detail/portable.h"

// prototypes
static void patterncursorstepbox_build(PatternCursorStepBox*);
static void patterncursorstepbox_onselectionchanged(PatternCursorStepBox*,
	psy_ui_Component* sender, int index);

void patterncursorstepbox_init(PatternCursorStepBox* self, psy_ui_Component*
	parent, Workspace* workspace)
{	
	psy_ui_component_init(patterncursorstepbox_base(self), parent, NULL);
	self->workspace = workspace;
	psy_ui_component_setdefaultalign(patterncursorstepbox_base(self),
		psy_ui_ALIGN_LEFT, psy_ui_defaults_hmargin(psy_ui_defaults()));
	psy_ui_component_setalignexpand(patterncursorstepbox_base(self),
		psy_ui_HORIZONTALEXPAND);
	psy_ui_label_init(&self->header, patterncursorstepbox_base(self), NULL);
	psy_ui_label_settext(&self->header, "patternview.step");
	psy_ui_combobox_init(&self->combobox, patterncursorstepbox_base(self), NULL);
	psy_signal_connect(&self->combobox.signal_selchanged, self,
		patterncursorstepbox_onselectionchanged);
	psy_ui_combobox_setcharnumber(&self->combobox, 3);
	patterncursorstepbox_build(self);
	patterncursorstepbox_update(self);	
}

void patterncursorstepbox_build(PatternCursorStepBox* self)
{
	int step;

	for (step = 0; step <= 16; ++step) {
		char text[20];

		psy_snprintf(text, 20, "%d", step);
		psy_ui_combobox_addtext(&self->combobox, text);
	}	
}

void patterncursorstepbox_onselectionchanged(PatternCursorStepBox* self,
	psy_ui_Component* sender, int index)
{		
	if (index >= 0) {
		workspace_setcursorstep(self->workspace, index);
	}
}

void patterncursorstepbox_update(PatternCursorStepBox* self)
{
	psy_ui_combobox_setcursel(&self->combobox,
		workspace_cursorstep(self->workspace));
}

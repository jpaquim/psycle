// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "stepbox.h"
#include <portable.h>

static void build(StepBox*);
static void onselectionchanged(StepBox*, ui_component* sender, int index);

void stepbox_init(StepBox* self, ui_component* parent, Workspace* workspace)
{		
	self->workspace = workspace;
	ui_component_init(&self->component, parent);	
	ui_component_enablealign(&self->component);
	ui_component_setalignexpand(&self->component, UI_HORIZONTALEXPAND);	
	ui_label_init(&self->header, &self->component);	
	ui_label_settext(&self->header, "Step");
	ui_combobox_init(&self->combobox, &self->component);
	psy_signal_connect(&self->combobox.signal_selchanged, self,
		onselectionchanged);
	ui_combobox_setcharnumber(&self->combobox, 3);
	build(self);
	ui_combobox_setcursel(&self->combobox, workspace_cursorstep(workspace) - 1);	
	{
		ui_margin margin;

		ui_margin_init(&margin, ui_value_makepx(0), ui_value_makepx(0),
			ui_value_makepx(0), ui_value_makepx(0));
		psy_list_free(ui_components_setalign(		
			ui_component_children(&self->component, 0),
			UI_ALIGN_LEFT,
			&margin));
	}
}

void build(StepBox* self)
{
	int step;

	for (step = 1; step <= 16; ++step) {
		char text[20];
		psy_snprintf(text, 20, "%d", step);
		ui_combobox_addstring(&self->combobox, text);
	}	
}

void onselectionchanged(StepBox* self, ui_component* sender, int index)
{		
	if (index >= 0) {
		workspace_setcursorstep(self->workspace, index + 1);
	}
}


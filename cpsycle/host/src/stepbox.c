// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "stepbox.h"
#include "../../detail/portable.h"

static void stepbox_updatetext(StepBox*, Translator*);
static void stepbox_onlanguagechanged(StepBox*, Translator* sender);
static void stepbox_build(StepBox*);
static void stepbox_onselectionchanged(StepBox*, psy_ui_Component* sender,
	int index);

void stepbox_init(StepBox* self, psy_ui_Component* parent, Workspace* workspace)
{
	self->workspace = workspace;
	psy_ui_component_init(&self->component, parent);
	psy_ui_component_setalignexpand(&self->component, psy_ui_HORIZONTALEXPAND);
	psy_ui_label_init(&self->header, &self->component);		
	psy_ui_combobox_init(&self->combobox, &self->component);
	psy_signal_connect(&self->combobox.signal_selchanged, self,
		stepbox_onselectionchanged);
	psy_ui_combobox_setcharnumber(&self->combobox, 3);
	stepbox_build(self);
	psy_ui_combobox_setcursel(&self->combobox,
		workspace_cursorstep(workspace) - 1);		
	psy_list_free(psy_ui_components_setalign(		
		psy_ui_component_children(&self->component, psy_ui_NONRECURSIVE),
		psy_ui_ALIGN_LEFT,
		NULL));
	psy_signal_connect(&workspace->signal_languagechanged, self,
		stepbox_onlanguagechanged);
	stepbox_updatetext(self, &workspace->translator);
}

void stepbox_updatetext(StepBox* self, Translator* translator)
{
	psy_ui_label_settext(&self->header, 
		translator_translate(translator, "patternview.step"));
}

void stepbox_onlanguagechanged(StepBox* self, Translator* sender)
{
	stepbox_updatetext(self, sender);
}

void stepbox_build(StepBox* self)
{
	int step;

	for (step = 1; step <= 16; ++step) {
		char text[20];
		psy_snprintf(text, 20, "%d", step);
		psy_ui_combobox_addtext(&self->combobox, text);
	}	
}

void stepbox_onselectionchanged(StepBox* self, psy_ui_Component* sender, int index)
{		
	if (index >= 0) {
		workspace_setcursorstep(self->workspace, index + 1);
	}
}

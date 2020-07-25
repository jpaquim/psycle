// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "checkunsaved.h"

#include <stdlib.h>
#include <string.h>

static void checkunsavedbox_updatetext(CheckUnsavedBox*);
static void checkunsavedbox_initalign(CheckUnsavedBox*);
static void checkunsavedbox_onlanguagechanged(CheckUnsavedBox*,
	Translator* sender);
static void checkunsavedbox_onsaveandexit(CheckUnsavedBox*, psy_ui_Component* sender);
static void checkunsavedbox_onjustexit(CheckUnsavedBox*, psy_ui_Component* sender);
static void checkunsavedbox_oncontinue(CheckUnsavedBox*, psy_ui_Component* sender);
static void checkunsavedbox_ondestroy(CheckUnsavedBox*, psy_ui_Component* sender);

void checkunsavedbox_init(CheckUnsavedBox* self, psy_ui_Component* parent, Workspace* workspace)
{	
	self->workspace = workspace;
	self->mode = CHECKUNSAVE_CLOSE;
	self->titlestr = strdup("");
	self->savestr = strdup("");
	self->nosavestr = strdup("");
	psy_ui_component_init(checkunsavedbox_base(self), parent);
	psy_ui_component_enablealign(checkunsavedbox_base(self));	
	psy_ui_component_init(&self->view, checkunsavedbox_base(self));
	psy_ui_component_enablealign(&self->view);
	psy_ui_component_setalign(&self->view, psy_ui_ALIGN_CENTER);	
	psy_ui_label_init(&self->title, &self->view);
	psy_ui_label_init(&self->header, &self->view);
	psy_ui_button_init(&self->saveandexit, &self->view);
	psy_signal_connect(&self->saveandexit.signal_clicked, self,
		checkunsavedbox_onsaveandexit);
	psy_ui_button_init(&self->exit, &self->view);
	psy_signal_connect(&self->exit.signal_clicked, self,
		checkunsavedbox_onjustexit);
	psy_ui_button_init(&self->cont, &self->view);
	psy_signal_connect(&self->cont.signal_clicked, self,
		checkunsavedbox_oncontinue);
	psy_signal_connect(&self->workspace->signal_languagechanged, self,
		checkunsavedbox_onlanguagechanged);	
	checkunsavedbox_updatetext(self);
	checkunsavedbox_initalign(self);
	psy_signal_init(&self->signal_execute);
	psy_signal_connect(&self->component.signal_destroy, self,
		checkunsavedbox_ondestroy);
}

void checkunsavedbox_ondestroy(CheckUnsavedBox* self, psy_ui_Component* sender)
{
	psy_signal_dispose(&self->signal_execute);
	free(self->titlestr);
	free(self->savestr);
	free(self->nosavestr);	
}

void checkunsavedbox_updatetext(CheckUnsavedBox* self)
{
	psy_ui_label_settext(&self->title, self->titlestr); // "Exit Psycle, but your Song is not saved!");
	psy_ui_label_settext(&self->header, "");
	psy_ui_button_settext(&self->saveandexit,
		workspace_translate(self->workspace, self->savestr)); // "Save and Exit"));
	psy_ui_button_settext(&self->exit,
		workspace_translate(self->workspace, self->nosavestr)); // "Exit (no save)"));
	psy_ui_button_settext(&self->cont,
		workspace_translate(self->workspace, "continue"));
	psy_ui_component_align(&self->component);
}

void checkunsavedbox_initalign(CheckUnsavedBox* self)
{	
	psy_ui_Margin margin;

	psy_ui_margin_init_all(&margin, psy_ui_value_makepx(0),
		psy_ui_value_makepx(0), psy_ui_value_makeeh(1),
		psy_ui_value_makepx(0));
	psy_list_free(psy_ui_components_setalign(
		psy_ui_component_children(&self->view, psy_ui_NONRECURSIVE),
		psy_ui_ALIGN_TOP,
		&margin));
	psy_ui_margin_init_all(&margin, psy_ui_value_makepx(0),
		psy_ui_value_makepx(0), psy_ui_value_makeeh(2),
		psy_ui_value_makepx(0));
	psy_ui_component_setmargin(&self->header.component, &margin);
}

void checkunsavedbox_onlanguagechanged(CheckUnsavedBox* self, Translator* sender)
{
	checkunsavedbox_updatetext(self);
	psy_ui_component_align(checkunsavedbox_base(self));
}

void checkunsavedbox_setlabels(CheckUnsavedBox* self, const char* title,
	const char* savestr, const char* nosavestr)
{
	free(self->titlestr);
	self->titlestr = strdup(title);
	free(self->savestr);
	self->savestr = strdup(savestr);
	free(self->nosavestr);
	self->nosavestr = strdup(nosavestr);
	checkunsavedbox_updatetext(self);
}

void checkunsavedbox_onsaveandexit(CheckUnsavedBox* self, psy_ui_Component* sender)
{
	psy_signal_emit(&self->signal_execute, self, 2, 0, self->mode);
}

void checkunsavedbox_onjustexit(CheckUnsavedBox* self, psy_ui_Component* sender)
{
	psy_signal_emit(&self->signal_execute, self, 2, 1, self->mode);
}

void checkunsavedbox_oncontinue(CheckUnsavedBox* self, psy_ui_Component* sender)
{
	psy_signal_emit(&self->signal_execute, self, 2, 2, self->mode);
}

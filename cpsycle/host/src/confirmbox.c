// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "confirmbox.h"
// std
#include <stdlib.h>
#include <string.h>

static void confirmbox_initalign(ConfirmBox*);
static void confirmbox_onok(ConfirmBox*, psy_ui_Component* sender);
static void confirmbox_onno(ConfirmBox*, psy_ui_Component* sender);
static void confirmbox_oncontinue(ConfirmBox*, psy_ui_Component* sender);
static void checkunsavedbox_ondestroy(ConfirmBox*, psy_ui_Component* sender);

void confirmbox_init(ConfirmBox* self, psy_ui_Component* parent, Workspace* workspace)
{	
	psy_ui_component_init(confirmbox_base(self), parent);
	self->workspace = workspace;
	self->mode = CONFIRM_CLOSE;
	self->titlestr = strdup("");
	self->yesstr = strdup("");
	self->nostr = strdup("");	
	psy_ui_component_init_align(&self->view, confirmbox_base(self),
		psy_ui_ALIGN_CENTER);
	psy_ui_label_init(&self->title, &self->view);
	psy_ui_label_init(&self->header, &self->view);
	psy_ui_button_init_connect(&self->yes, &self->view,
		self, confirmbox_onok);
	psy_ui_button_init_connect(&self->no, &self->view, self,
		confirmbox_onno);
	psy_ui_button_init_connect(&self->cont, &self->view,
		self, confirmbox_oncontinue);
	psy_ui_label_settext(&self->title, self->titlestr); // "no Psycle, but your Song is not saved!");
	psy_ui_label_preventtranslation(&self->title);
	//psy_ui_label_setcharnumber(&self->title, 48);
	psy_ui_label_settext(&self->header, "");
	psy_ui_button_settext(&self->yes, self->yesstr); // "Save and no"));
	psy_ui_button_settext(&self->no, self->nostr); // "no (no save)"));
	psy_ui_button_settext(&self->cont, "continue");
	psy_ui_component_align(&self->component);
	confirmbox_initalign(self);
	psy_signal_init(&self->signal_execute);
	psy_signal_connect(&self->component.signal_destroy, self,
		checkunsavedbox_ondestroy);
}

void checkunsavedbox_ondestroy(ConfirmBox* self, psy_ui_Component* sender)
{
	psy_signal_dispose(&self->signal_execute);
	free(self->titlestr);
	free(self->yesstr);
	free(self->nostr);
}

void confirmbox_initalign(ConfirmBox* self)
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

void confirmbox_setlabels(ConfirmBox* self, const char* title,
	const char* yesstr, const char* nostr)
{
	free(self->titlestr);
	self->titlestr = strdup(title);
	free(self->yesstr);
	self->yesstr = strdup(yesstr);
	free(self->nostr);
	self->nostr = strdup(nostr);
	psy_ui_label_settext(&self->title, self->titlestr); // "no Psycle, but your Song is not saved!");
	psy_ui_label_settext(&self->header, "");
	psy_ui_button_settext(&self->yes, self->yesstr); // "Save and no"));
	psy_ui_button_settext(&self->no, self->nostr); // "no (no save)"));
	psy_ui_button_settext(&self->cont, "Continue");
	psy_ui_component_align(&self->component);
}

void confirmbox_onok(ConfirmBox* self, psy_ui_Component* sender)
{
	psy_signal_emit(&self->signal_execute, self, 2, 0, self->mode);
}

void confirmbox_onno(ConfirmBox* self, psy_ui_Component* sender)
{
	psy_signal_emit(&self->signal_execute, self, 2, 1, self->mode);
}

void confirmbox_oncontinue(ConfirmBox* self, psy_ui_Component* sender)
{
	psy_signal_emit(&self->signal_execute, self, 2, 2, self->mode);
}

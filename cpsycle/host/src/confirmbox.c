// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "confirmbox.h"
// std
#include <stdlib.h>
#include <string.h>

static void confirmbox_onok(ConfirmBox*, psy_ui_Component* sender);
static void confirmbox_onno(ConfirmBox*, psy_ui_Component* sender);
static void confirmbox_oncontinue(ConfirmBox*, psy_ui_Component* sender);
static void checkunsavedbox_ondestroy(ConfirmBox*, psy_ui_Component* sender);

void confirmbox_init(ConfirmBox* self, psy_ui_Component* parent, Workspace* workspace)
{	
	psy_ui_Margin spacing;
	
	psy_ui_component_init(confirmbox_base(self), parent, NULL);
	self->workspace = workspace;
	self->mode = CONFIRM_CLOSE;	
	psy_ui_component_init_align(&self->view, confirmbox_base(self),
		psy_ui_ALIGN_CENTER);
	psy_ui_component_setdefaultalign(&self->view,
		psy_ui_ALIGN_TOP, psy_ui_margin_zero());
	psy_ui_label_init_text(&self->title, &self->view, NULL, "msg.psyreq");
	psy_ui_label_init_text(&self->header, &self->view, NULL, "");
	psy_ui_button_init_text_connect(&self->yes, &self->view, NULL, "msg.yes",
		self, confirmbox_onok);
	psy_ui_button_init_text_connect(&self->no, &self->view, NULL, "msg.no",
		self, confirmbox_onno);
	psy_ui_button_init_text_connect(&self->cont, &self->view, NULL, "msg.cont",
		self, confirmbox_oncontinue);	
	psy_ui_margin_init_em(&spacing, 0.5, 0.0, 0.5, 0.0);
	psy_ui_component_setspacing_children(&self->view, spacing);
	psy_signal_init(&self->signal_execute);
	psy_signal_connect(&self->component.signal_destroy, self,
		checkunsavedbox_ondestroy);
}

void checkunsavedbox_ondestroy(ConfirmBox* self, psy_ui_Component* sender)
{
	psy_signal_dispose(&self->signal_execute);	
}

void confirmbox_setlabels(ConfirmBox* self, const char* title,
	const char* yesstr, const char* nostr)
{		
	psy_ui_label_settext(&self->title, title); 
	psy_ui_label_settext(&self->header, "");
	psy_ui_button_settext(&self->yes, yesstr);
	psy_ui_button_settext(&self->no, nostr);
	psy_ui_button_settext(&self->cont, "msg.cont");
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

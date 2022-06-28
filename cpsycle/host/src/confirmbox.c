/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "confirmbox.h"
/* std */
#include <stdlib.h>
#include <string.h>


/* prototypes */
static void confirmbox_on_destroyed(ConfirmBox*);
static void confirmbox_onok(ConfirmBox*, psy_ui_Component* sender);
static void confirmbox_onno(ConfirmBox*, psy_ui_Component* sender);
static void confirmbox_oncontinue(ConfirmBox*, psy_ui_Component* sender);

/* vtable */
static psy_ui_ComponentVtable vtable;
static bool vtable_initialized = FALSE;

static void vtable_init(ConfirmBox* self)
{
	if (!vtable_initialized) {
		vtable = *(self->component.vtable);
		vtable.on_destroyed =
			(psy_ui_fp_component_event)
			confirmbox_on_destroyed;
		vtable_initialized = TRUE;
	}
	psy_ui_component_set_vtable(&self->component, &vtable);
}

/* implementation */
void confirmbox_init(ConfirmBox* self, psy_ui_Component* parent)
{	
	psy_ui_Margin padding;
	
	psy_ui_component_init(confirmbox_base(self), parent, NULL);
	vtable_init(self);	
	self->mode = CONFIRM_CLOSE;	
	psy_ui_component_init_align(&self->view, confirmbox_base(self), NULL,
		psy_ui_ALIGN_CENTER);
	psy_ui_component_set_default_align(&self->view,
		psy_ui_ALIGN_TOP, psy_ui_margin_zero());
	psy_ui_margin_init_em(&padding, 0.5, 0.0, 0.5, 0.0);
	psy_ui_label_init_text(&self->title, &self->view, "msg.psyreq");
	psy_ui_component_set_padding(&self->title.component, padding);
	psy_ui_label_init_text(&self->header, &self->view, "");
	psy_ui_component_set_padding(&self->header.component, padding);
	psy_ui_button_init_text_connect(&self->yes, &self->view, "msg.yes",
		self, confirmbox_onok);
	psy_ui_component_set_padding(&self->yes.component, padding);
	psy_ui_button_init_text_connect(&self->no, &self->view, "msg.no",
		self, confirmbox_onno);
	psy_ui_component_set_padding(&self->no.component, padding);
	psy_ui_button_init_text_connect(&self->cont, &self->view, "msg.cont",
		self, confirmbox_oncontinue);
	psy_ui_component_set_padding(&self->cont.component, padding);
	psy_signal_init(&self->signal_execute);	
}

void confirmbox_on_destroyed(ConfirmBox* self)
{
	psy_signal_dispose(&self->signal_execute);	
}

void confirmbox_set_labels(ConfirmBox* self, const char* title,
	const char* yesstr, const char* nostr)
{		
	psy_ui_label_set_text(&self->title, title); 
	psy_ui_label_set_text(&self->header, "");
	psy_ui_button_set_text(&self->yes, yesstr);
	psy_ui_button_set_text(&self->no, nostr);
	psy_ui_button_set_text(&self->cont, "msg.cont");
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

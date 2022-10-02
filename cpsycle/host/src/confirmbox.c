/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "confirmbox.h"


/* prototypes */
static void confirmbox_on_accept(ConfirmBox*, psy_ui_Component* sender);
static void confirmbox_on_reject(ConfirmBox*, psy_ui_Component* sender);
static void confirmbox_on_continue(ConfirmBox*, psy_ui_Component* sender);

/* implementation */
void confirmbox_init(ConfirmBox* self, psy_ui_Component* parent)
{	
	psy_ui_Margin padding;
	
	assert(self);
	
	psy_ui_component_init(confirmbox_base(self), parent, NULL);	
	self->slot_accept = psy_slot_make(NULL, NULL);
	self->slot_reject = psy_slot_make(NULL, NULL);
	self->slot_cont = psy_slot_make(NULL, NULL);
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
		self, confirmbox_on_accept);
	psy_ui_component_set_padding(&self->yes.component, padding);
	psy_ui_button_init_text_connect(&self->no, &self->view, "msg.no",
		self, confirmbox_on_reject);
	psy_ui_component_set_padding(&self->no.component, padding);
	psy_ui_button_init_text_connect(&self->cont, &self->view, "msg.cont",
		self, confirmbox_on_continue);
	psy_ui_component_set_padding(&self->cont.component, padding);	
}

void confirmbox_set_labels(ConfirmBox* self, const char* title,
	const char* yesstr, const char* nostr)
{		
	assert(self);
	
	psy_ui_label_set_text(&self->title, title); 
	psy_ui_label_set_text(&self->header, "");
	psy_ui_button_set_text(&self->yes, yesstr);
	psy_ui_button_set_text(&self->no, nostr);
	psy_ui_button_set_text(&self->cont, "msg.cont");
	psy_ui_component_align(&self->component);
}

void confirmbox_on_accept(ConfirmBox* self, psy_ui_Component* sender)
{
	assert(self);
	
	psy_slot_emit(&self->slot_accept);	
}

void confirmbox_on_reject(ConfirmBox* self, psy_ui_Component* sender)
{
	assert(self);
	
	psy_slot_emit(&self->slot_reject);	
}

void confirmbox_on_continue(ConfirmBox* self, psy_ui_Component* sender)
{
	assert(self);
	
	psy_slot_emit(&self->slot_cont);	
}

void confirmbox_set_callbacks(ConfirmBox* self, psy_Slot accept,
	psy_Slot reject, psy_Slot cont)
{
	assert(self);
	
	self->slot_accept = accept;
	self->slot_reject = reject;
	self->slot_cont = cont;
}

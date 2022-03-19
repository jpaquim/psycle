/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
**  copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "uicombobox.h"
/* local */
#include "uiicondraw.h"
/* std*/
#include <stdlib.h>

/* prototypes */
static void psy_ui_combobox_ondestroy(psy_ui_ComboBox*);
static bool psy_ui_combobox_haspreventry(const psy_ui_ComboBox*);
static bool psy_ui_combobox_hasnextentry(const psy_ui_ComboBox*);
static void psy_ui_combobox_onselchange(psy_ui_ComboBox*,
	psy_ui_Component* sender, intptr_t index);
static void psy_ui_combobox_onless(psy_ui_ComboBox*, psy_ui_Button* sender);
static void psy_ui_combobox_onmore(psy_ui_ComboBox*, psy_ui_Button* sender);
static void psy_ui_combobox_onexpand(psy_ui_ComboBox*, psy_ui_Button* sender,
	psy_ui_MouseEvent* ev);
static void psy_ui_combobox_ontextfield(psy_ui_ComboBox*, psy_ui_Label* sender,
	psy_ui_MouseEvent* ev);
static void psy_ui_combobox_expand(psy_ui_ComboBox*);
static void psy_ui_combobox_onmousewheel(psy_ui_ComboBox*, psy_ui_MouseEvent*);
/* vtable */
static psy_ui_ComponentVtable vtable;
static bool vtable_initialized = FALSE;

static void vtable_init(psy_ui_ComboBox* self)
{
	if (!vtable_initialized) {
		vtable = *(self->component.vtable);
		vtable.ondestroy =
			(psy_ui_fp_component_event)
			psy_ui_combobox_ondestroy;
		vtable.onmousewheel =
			(psy_ui_fp_component_onmouseevent)
			psy_ui_combobox_onmousewheel;
		vtable_initialized = TRUE;
	}
	self->component.vtable = &vtable;
}

/* implementation */
void psy_ui_combobox_init(psy_ui_ComboBox* self, psy_ui_Component* parent)
{
	assert(self);
	
	psy_ui_component_init(&self->component, parent, NULL);
	vtable_init(self);
	psy_signal_init(&self->signal_selchanged);	
	psy_table_init(&self->itemdata);	
	psy_ui_component_setstyletype(&self->component, psy_ui_STYLE_COMBOBOX);
	psy_ui_component_setalignexpand(psy_ui_combobox_base(self),
		psy_ui_HEXPAND);
	/* dropdown */
	psy_ui_dropdownbox_init(&self->dropdown, self->component.view);
	psy_ui_component_init(&self->pane, &self->dropdown.component, &self->dropdown.component);
	psy_ui_component_setalign(&self->pane, psy_ui_ALIGN_CLIENT);
	/* listbox */
	psy_ui_listbox_init(&self->listbox, &self->pane);
	psy_signal_connect(&self->listbox.signal_selchanged, self,
		psy_ui_combobox_onselchange);
	psy_ui_component_setalign(&self->listbox.component,
		psy_ui_ALIGN_CLIENT);	
	/* textfield */
	psy_ui_label_init(&self->textfield, &self->component);
	psy_ui_label_preventtranslation(&self->textfield);
	psy_ui_component_setstyletype(psy_ui_label_base(&self->textfield),
		psy_ui_STYLE_COMBOBOX_TEXT);
	psy_ui_component_setalign(psy_ui_label_base(&self->textfield),
		psy_ui_ALIGN_LEFT);
	psy_ui_label_setcharnumber(&self->textfield, 10.0);
	psy_signal_connect(&psy_ui_label_base(&self->textfield)->signal_mousedown,
		self, psy_ui_combobox_ontextfield);
	/* less */
	psy_ui_button_init_connect(&self->less, &self->component,
		self, psy_ui_combobox_onless);
	psy_ui_component_setalign(psy_ui_button_base(&self->less),
		psy_ui_ALIGN_LEFT);
	psy_ui_button_seticon(&self->less, psy_ui_ICON_LESS);
	/* more */
	psy_ui_button_init_connect(&self->more, &self->component,
		self, psy_ui_combobox_onmore);
	psy_ui_button_seticon(&self->more, psy_ui_ICON_MORE);
	psy_ui_component_setalign(psy_ui_button_base(&self->more),
		psy_ui_ALIGN_LEFT);
	/* expand */
	psy_ui_button_init(&self->expand, &self->component);
	psy_signal_connect(&self->expand.component.signal_mousedown,
		self, psy_ui_combobox_onexpand);
	psy_ui_component_setalign(psy_ui_button_base(&self->expand),
		psy_ui_ALIGN_LEFT);
	psy_ui_button_seticon(&self->expand, psy_ui_ICON_DOWN);	
}

void psy_ui_combobox_ondestroy(psy_ui_ComboBox* self)
{
	assert(self);

	psy_table_dispose(&self->itemdata);
	psy_signal_dispose(&self->signal_selchanged);
}

psy_ui_ComboBox* psy_ui_combobox_alloc(void)
{	
	return (psy_ui_ComboBox*)malloc(sizeof(psy_ui_ComboBox));
}

psy_ui_ComboBox* psy_ui_combobox_allocinit(psy_ui_Component* parent)
{
	psy_ui_ComboBox* rv;	

	rv = psy_ui_combobox_alloc();
	if (rv) {
		psy_ui_combobox_init(rv, parent);
		psy_ui_component_deallocateafterdestroyed(&rv->component);
	}
	return rv;
}

intptr_t psy_ui_combobox_addtext(psy_ui_ComboBox* self, const char* text)
{	
	assert(self);

	return psy_ui_listbox_addtext(&self->listbox, text);	
}

void psy_ui_combobox_settext(psy_ui_ComboBox* self, const char* text,
	intptr_t index)
{
	assert(self);

	psy_ui_listbox_settext(&self->listbox, text, index);
	if (index == psy_ui_combobox_cursel(self)) {
		char text[512];
		
		psy_ui_combobox_text(self, text);
		psy_ui_label_settext(&self->textfield, text);
	}
}

void psy_ui_combobox_text(psy_ui_ComboBox* self, char* text)
{
	assert(self);

	psy_ui_listbox_text(&self->listbox, text,
		psy_ui_listbox_cursel(&self->listbox));
}

intptr_t psy_ui_combobox_count(const psy_ui_ComboBox* self)
{
	assert(self);

	return psy_ui_listbox_count(&self->listbox);
}

void psy_ui_combobox_clear(psy_ui_ComboBox* self)
{
	assert(self);

	psy_ui_listbox_clear(&self->listbox);
	psy_table_clear(&self->itemdata);
	psy_ui_label_settext(&self->textfield, "");	
}

void psy_ui_combobox_setcursel(psy_ui_ComboBox* self, intptr_t index)
{
	char text[512];

	assert(self);	

	psy_ui_listbox_setcursel(&self->listbox, index);
	psy_ui_combobox_text(self, text);	
	psy_ui_label_settext(&self->textfield, text);
}

intptr_t psy_ui_combobox_cursel(const psy_ui_ComboBox* self)
{
	assert(self);

	return psy_ui_listbox_cursel(&self->listbox);
}

void psy_ui_combobox_setcharnumber(psy_ui_ComboBox* self, double number)
{
	assert(self);

	psy_ui_label_setcharnumber(&self->textfield, number);	
}

void psy_ui_combobox_setitemdata(psy_ui_ComboBox* self, uintptr_t index,
	intptr_t data)
{
	assert(self);

	psy_table_insert(&self->itemdata, index, (void*)data);
}

intptr_t psy_ui_combobox_itemdata(psy_ui_ComboBox* self, uintptr_t index)
{
	assert(self);

	if (psy_table_exists(&self->itemdata, index)) {
		return (intptr_t)psy_table_at(&self->itemdata, index);
	}
	return -1;
}

bool psy_ui_combobox_haspreventry(const psy_ui_ComboBox* self)
{
	assert(self);

	return psy_ui_combobox_cursel(self) > 0;
}

bool psy_ui_combobox_hasnextentry(const psy_ui_ComboBox* self)
{
	intptr_t count;

	assert(self);

	count = psy_ui_combobox_count(self);
	return (psy_ui_combobox_cursel(self) < count - 1);
}

void psy_ui_combobox_onselchange(psy_ui_ComboBox* self,
	psy_ui_Component* sender, intptr_t index)
{
	char text[512];

	assert(self);

	psy_ui_dropdownbox_hide(&self->dropdown);	
	psy_ui_listbox_setcursel(&self->listbox, index);
	psy_ui_combobox_text(self, text);
	psy_ui_label_settext(&self->textfield, text);
	psy_signal_emit(&self->signal_selchanged, self, 1, index);
}

void psy_ui_combobox_onless(psy_ui_ComboBox* self, psy_ui_Button* sender)
{
	intptr_t index;

	assert(self);

	index = psy_ui_combobox_cursel(self);
	if (index > 0) {
		psy_ui_combobox_setcursel(self, index - 1);
		psy_signal_emit(&self->signal_selchanged, self, 1, index - 1);
	}	
}

void psy_ui_combobox_onmore(psy_ui_ComboBox* self, psy_ui_Button* sender)
{
	intptr_t count;
	intptr_t index;

	assert(self);

	index = psy_ui_combobox_cursel(self);
	count = psy_ui_combobox_count(self);
	if (index < count - 1) {
		psy_ui_combobox_setcursel(self, index + 1);
		psy_signal_emit(&self->signal_selchanged, self, 1, index + 1);
	}	
}

void psy_ui_combobox_ontextfield(psy_ui_ComboBox* self, psy_ui_Label* sender,
	psy_ui_MouseEvent* ev)
{	
	assert(self);

	psy_ui_combobox_expand(self);
	psy_ui_mouseevent_stop_propagation(ev);
}

void psy_ui_combobox_onexpand(psy_ui_ComboBox* self, psy_ui_Button* sender,
	psy_ui_MouseEvent* ev)
{	
	assert(self);

	sender->component.vtable->onmouseup(&sender->component, ev);
	psy_ui_combobox_expand(self);
}

void psy_ui_combobox_expand(psy_ui_ComboBox* self)
{	
	assert(self);
	
	if (!psy_ui_component_visible(&self->dropdown.component)) {		
		psy_ui_dropdownbox_show(&self->dropdown, &self->component);		
	}	
}

void psy_ui_combobox_onmousewheel(psy_ui_ComboBox* self, psy_ui_MouseEvent* ev)
{
	assert(self);

	if (psy_ui_mouseevent_delta(ev) != 0) {
		intptr_t index;
		
		index = psy_ui_combobox_cursel(self) +
			psy_sgn(psy_ui_mouseevent_delta(ev));
		if (index >= 0 && index < psy_ui_combobox_count(self)) {
			psy_ui_combobox_setcursel(self, index);
			psy_signal_emit(&self->signal_selchanged, self, 1, index);
		}		
	}
	psy_ui_mouseevent_prevent_default(ev);
}

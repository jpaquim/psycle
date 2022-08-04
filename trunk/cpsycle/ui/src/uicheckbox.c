/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "uicheckbox.h"
/* std */
#include <stdlib.h>


/* prototypes */
static void psy_ui_checkbox_on_destroyed(psy_ui_CheckBox*);
static void psy_ui_checkbox_mark(psy_ui_CheckBox*);
static void psy_ui_checkbox_unmark(psy_ui_CheckBox*);
static void psy_ui_checkbox_on_mouse_down(psy_ui_CheckBox*,
	psy_ui_MouseEvent*);
static void psy_ui_checkbox_on_property_changed(psy_ui_CheckBox*,
	psy_Property* sender);
static void psy_ui_checkbox_before_property_destroyed(psy_ui_CheckBox*,
	psy_Property* sender);

/* vtable */
static psy_ui_ComponentVtable vtable;
static psy_ui_ComponentVtable super_vtable;
static bool vtable_initialized = FALSE;

static void vtable_init(psy_ui_CheckBox* self)
{
	assert(self);

	if (!vtable_initialized) {
		vtable = *(self->component.vtable);
		super_vtable = *(psy_ui_checkbox_base(self)->vtable);
		vtable.on_destroyed =
			(psy_ui_fp_component_event)
			psy_ui_checkbox_on_destroyed;
		vtable.on_mouse_down =
			(psy_ui_fp_component_on_mouse_event)
			psy_ui_checkbox_on_mouse_down;
		vtable_initialized = TRUE;
	}
	psy_ui_component_set_vtable(&self->component, &vtable);
}

/* implementation */
void psy_ui_checkbox_init(psy_ui_CheckBox* self,
	psy_ui_Component* parent)
{  	
	assert(self);

	psy_ui_component_init(psy_ui_checkbox_base(self), parent, NULL);
	vtable_init(self);
	self->property = NULL;
	psy_ui_component_init(&self->checkmark, psy_ui_checkbox_base(self),
		NULL);
	psy_ui_component_set_align(&self->checkmark, psy_ui_ALIGN_LEFT);
	psy_ui_component_set_style_types(&self->checkmark,
		psy_ui_STYLE_CHECKMARK, psy_ui_STYLE_CHECKMARK_HOVER,
		psy_ui_STYLE_CHECKMARK_SELECT, psy_INDEX_INVALID);	
	psy_ui_component_set_preferred_size(&self->checkmark,
		psy_ui_size_make_em(2.0, 1.0));
	psy_ui_component_setmaximumsize(&self->checkmark,
		psy_ui_size_make_em(2.0, 1.0));
	psy_ui_label_init(&self->text, psy_ui_checkbox_base(self));
	psy_ui_component_set_margin(&self->text.component,
		psy_ui_margin_make_em(0.0, 0.0, 0.0, 1.0));
	psy_ui_label_prevent_wrap(&self->text);
	psy_ui_component_set_align(&self->text.component,
		psy_ui_ALIGN_LEFT);
	psy_ui_component_set_align_expand(&self->text.component,
		psy_ui_HEXPAND);	
	psy_signal_init(&self->signal_clicked);	
}

void psy_ui_checkbox_init_text(psy_ui_CheckBox* self,
	psy_ui_Component* parent, const char* text)
{
	assert(self);

	psy_ui_checkbox_init(self, parent);
	psy_ui_checkbox_set_text(self, text);
}

void psy_ui_checkbox_init_exchange(psy_ui_CheckBox* self,
	psy_ui_Component* parent, psy_Property* property)
{
	psy_ui_checkbox_init(self, parent);
	psy_ui_checkbox_data_exchange(self, property);
}

void psy_ui_checkbox_on_destroyed(psy_ui_CheckBox* self)
{	
	assert(self);

	if (self->property) {
		psy_property_disconnect(self->property, self);
	}
	psy_signal_dispose(&self->signal_clicked);
}

psy_ui_CheckBox* psy_ui_checkbox_alloc(void)
{
	return (psy_ui_CheckBox*)malloc(sizeof(psy_ui_CheckBox));
}

psy_ui_CheckBox* psy_ui_checkbox_allocinit(psy_ui_Component* parent)
{
	psy_ui_CheckBox* rv;

	rv = psy_ui_checkbox_alloc();
	if (rv) {
		psy_ui_checkbox_init(rv, parent);
		psy_ui_component_deallocate_after_destroyed(&rv->component);
	}
	return rv;
}

psy_ui_CheckBox* psy_ui_checkbox_allocinit_exchange(
	psy_ui_Component* parent, psy_Property* property)
{
	psy_ui_CheckBox* rv;

	rv = psy_ui_checkbox_alloc();
	if (rv) {		
		psy_ui_checkbox_init_exchange(rv, parent, property);
		psy_ui_component_deallocate_after_destroyed(&rv->component);
	}
	return rv;
}

void psy_ui_checkbox_set_text(psy_ui_CheckBox* self, const char* text)
{
	assert(self);

	psy_ui_label_set_text(&self->text, text);	
}

const char* psy_ui_checkbox_text(psy_ui_CheckBox* self)
{	
	assert(self);

	return psy_ui_label_text(&self->text);
}

void psy_ui_checkbox_mark(psy_ui_CheckBox* self)
{
	assert(self);

	if (!psy_ui_checkbox_checked(self)) {
		psy_ui_component_add_style_state(&self->checkmark,
			psy_ui_STYLESTATE_SELECT);
	}
}

void psy_ui_checkbox_unmark(psy_ui_CheckBox* self)
{
	assert(self);

	if (psy_ui_checkbox_checked(self)) {
		psy_ui_component_remove_style_state(&self->checkmark,
			psy_ui_STYLESTATE_SELECT);
	}
}

void psy_ui_checkbox_check(psy_ui_CheckBox* self)
{
	assert(self);

	if (self->property) {
		psy_property_set_item_bool(self->property, TRUE);
	} else {
		psy_ui_checkbox_mark(self);
	}	
}

void psy_ui_checkbox_disable_check(psy_ui_CheckBox* self)
{
	assert(self);
	
	if (self->property) {
		psy_property_set_item_bool(self->property, FALSE);
	} else {
		psy_ui_checkbox_unmark(self);
	}
}

bool psy_ui_checkbox_checked(const psy_ui_CheckBox* self)
{
	assert(self);

	return (psy_ui_componentstyle_state(&self->checkmark.style) &
		psy_ui_STYLESTATE_SELECT) == psy_ui_STYLESTATE_SELECT;
}

void psy_ui_checkbox_prevent_wrap(psy_ui_CheckBox* self)
{
	assert(self);

	psy_ui_label_prevent_wrap(&self->text);
}

void psy_ui_checkbox_enablewrap(psy_ui_CheckBox* self)
{
	assert(self);

	psy_ui_label_enable_wrap(&self->text);
}

void psy_ui_checkbox_data_exchange(psy_ui_CheckBox* self,
	psy_Property* property)
{
	assert(self);
	assert(property);

	self->property = property;
	if (property) {
		psy_ui_checkbox_on_property_changed(self, property);
		psy_ui_checkbox_set_text(self, psy_property_text(property));
		psy_property_connect(property, self,
			psy_ui_checkbox_on_property_changed);
		psy_signal_connect(&self->property->before_destroyed, self,
			psy_ui_checkbox_before_property_destroyed);
	}
}

void psy_ui_checkbox_on_property_changed(psy_ui_CheckBox* self,
	psy_Property* sender)
{
	if (psy_property_is_choice_item(sender)) {
		bool checked;
		
		checked = (psy_property_at_choice(psy_property_parent(sender))
			== sender);
		if (checked) {
			psy_ui_checkbox_mark(self);
		} else {
			psy_ui_checkbox_unmark(self);
		}
	} else if (psy_property_item_bool(sender)) {		
		psy_ui_checkbox_mark(self);
	} else {
		psy_ui_checkbox_unmark(self);
	}
}

void psy_ui_checkbox_before_property_destroyed(psy_ui_CheckBox* self,
	psy_Property* sender)
{
	assert(self);

	self->property = NULL;
}

void psy_ui_checkbox_on_mouse_down(psy_ui_CheckBox* self,
	psy_ui_MouseEvent* ev)
{	
	assert(self);

	super_vtable.on_mouse_down(psy_ui_checkbox_base(self), ev);
	if (self->property) {
		if (psy_property_is_choice_item(self->property)) {
			intptr_t index;

			if (!psy_property_parent(self->property)) {
				return;
			}
			index = psy_property_index(self->property);
			psy_property_set_item_int(psy_property_parent(self->property), index);
		} else {
			psy_property_set_item_bool(self->property, 
				!psy_ui_checkbox_checked(self));
		}
	} else {
		if (psy_ui_checkbox_checked(self)) {
			psy_ui_checkbox_disable_check(self);
		} else {
			psy_ui_checkbox_check(self);
		}
	}
	psy_signal_emit(&self->signal_clicked, &self->component, 0);
}

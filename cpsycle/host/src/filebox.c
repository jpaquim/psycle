/*
** This source is free software; you can redistribute itand /or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "filebox.h"
/* host */
#include "styles.h"
/* ui */
#include <uifolderdialog.h>
/* platform */
#include "../../detail/portable.h"

/* FontBox */

/* prototypes */
static void filebox_on_destroyed(FileBox*);
static void filebox_on_property_changed(FileBox*,
	psy_Property* sender);
static void filebox_before_property_destroyed(FileBox*, psy_Property* sender);
static void filebox_on_dialog(FileBox*, psy_ui_Component* sender);
static void filebox_on_edit_keydown(FileBox*, psy_ui_Component* sender,
	psy_ui_KeyboardEvent*);
static void filebox_on_edit_reject(FileBox*, psy_ui_TextArea* sender);

/* vtable */
static psy_ui_ComponentVtable vtable;
static psy_ui_ComponentVtable super_vtable;
static bool vtable_initialized = FALSE;

static void vtable_init(FileBox* self)
{
	assert(self);

	if (!vtable_initialized) {
		vtable = *(self->component.vtable);
		super_vtable = *(self->component.vtable);
		vtable.on_destroyed =
			(psy_ui_fp_component_event)
			filebox_on_destroyed;		
		vtable_initialized = TRUE;
	}
	psy_ui_component_set_vtable(filebox_base(self), &vtable);
}

/* implementation */
void filebox_init(FileBox* self, psy_ui_Component* parent)
{
	assert(self);

	psy_ui_component_init(&self->component, parent, NULL);
	vtable_init(self);
	self->property = NULL;	
	psy_ui_textarea_init_single_line(&self->edit, &self->component);
	psy_ui_component_set_align(psy_ui_textarea_base(&self->edit),
		psy_ui_ALIGN_CLIENT);
	psy_ui_textarea_enable_input_field(&self->edit);
	psy_signal_connect(&self->edit.signal_reject, self,
		filebox_on_edit_reject);
	psy_signal_connect(&self->edit.component.signal_keydown, self,
		filebox_on_edit_keydown);	
	psy_ui_button_init(&self->dialog, &self->component);
	psy_ui_component_set_align(psy_ui_button_base(&self->dialog),
		psy_ui_ALIGN_RIGHT);
	psy_ui_button_prevent_translation(&self->dialog);
	psy_ui_button_set_text(&self->dialog, "...");
	psy_signal_connect(&self->dialog.signal_clicked, self,
		filebox_on_dialog);
}

void filebox_on_destroyed(FileBox* self)
{	
	assert(self);

	if (self->property) {
		psy_property_disconnect(self->property, self);
	}
}

FileBox* filebox_alloc(void)
{
	return (FileBox*)malloc(sizeof(FileBox));
}

FileBox* filebox_allocinit(psy_ui_Component* parent)
{
	FileBox* rv;

	rv = filebox_alloc();
	if (rv) {
		filebox_init(rv, parent);
		psy_ui_component_deallocate_after_destroyed(&rv->component);
	}
	return rv;
}

void filebox_data_exchange(FileBox* self, psy_Property* property)
{
	assert(self);
	assert(property);

	self->property = property;
	if (property) {
		filebox_on_property_changed(self, property);
		psy_ui_textarea_data_exchange(&self->edit, self->property);
		psy_property_connect(property, self,
			filebox_on_property_changed);
		psy_signal_connect(&self->property->before_destroyed, self,
			filebox_before_property_destroyed);
	}
}

void filebox_on_property_changed(FileBox* self, psy_Property* sender)
{
}

void filebox_before_property_destroyed(FileBox* self, psy_Property* sender)
{
	assert(self);

	self->property = NULL;
}

void filebox_on_dialog(FileBox* self,psy_ui_Component* sender)
{
	psy_ui_FolderDialog dialog;

	psy_ui_folderdialog_init_all(&dialog, 0, psy_ui_translate(
		psy_property_text(self->property)), "");
	if (psy_ui_folderdialog_execute(&dialog)) {
		psy_property_set_item_str(self->property,
			psy_ui_folderdialog_path(&dialog));
		psy_ui_folderdialog_dispose(&dialog);
	}
}

void filebox_on_edit_keydown(FileBox* self, psy_ui_Component* sender,
	psy_ui_KeyboardEvent* ev)
{
	if (psy_property_is_hex(self->property)) {
		if ((psy_ui_keyboardevent_keycode(ev) >= psy_ui_KEY_DIGIT0 && psy_ui_keyboardevent_keycode(ev) <= psy_ui_KEY_DIGIT9) ||
			(psy_ui_keyboardevent_keycode(ev) >= psy_ui_KEY_A && psy_ui_keyboardevent_keycode(ev) <= psy_ui_KEY_F) ||
			(psy_ui_keyboardevent_keycode(ev) < psy_ui_KEY_HELP)) {
			return;
		}
		psy_ui_keyboardevent_prevent_default(ev);
	}
	psy_ui_keyboardevent_stop_propagation(ev);
}

void filebox_on_edit_reject(FileBox* self, psy_ui_TextArea* sender)
{
	psy_ui_component_set_focus(&self->component);
}

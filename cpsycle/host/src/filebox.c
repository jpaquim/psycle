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
static void fileedit_on_destroyed(FileEdit*);
static void fileedit_on_property_changed(FileEdit*,
	psy_Property* sender);
static void fileedit_before_property_destroyed(FileEdit*, psy_Property* sender);
static void fileedit_on_dialog(FileEdit*, psy_ui_Component* sender);
static void fileedit_on_edit_keydown(FileEdit*, psy_ui_Component* sender,
	psy_ui_KeyboardEvent*);
static void fileedit_on_edit_reject(FileEdit*, psy_ui_TextArea* sender);

/* vtable */
static psy_ui_ComponentVtable vtable;
static psy_ui_ComponentVtable super_vtable;
static bool vtable_initialized = FALSE;

static void vtable_init(FileEdit* self)
{
	assert(self);

	if (!vtable_initialized) {
		vtable = *(self->component.vtable);
		super_vtable = *(self->component.vtable);
		vtable.on_destroyed =
			(psy_ui_fp_component_event)
			fileedit_on_destroyed;		
		vtable_initialized = TRUE;
	}
	psy_ui_component_set_vtable(fileedit_base(self), &vtable);
}

/* implementation */
void fileedit_init(FileEdit* self, psy_ui_Component* parent)
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
		fileedit_on_edit_reject);
	psy_signal_connect(&self->edit.component.signal_keydown, self,
		fileedit_on_edit_keydown);	
	psy_ui_button_init(&self->dialog, &self->component);
	psy_ui_component_set_align(psy_ui_button_base(&self->dialog),
		psy_ui_ALIGN_RIGHT);
	psy_ui_button_prevent_translation(&self->dialog);
	psy_ui_button_set_text(&self->dialog, "...");
	psy_signal_connect(&self->dialog.signal_clicked, self,
		fileedit_on_dialog);
}

void fileedit_on_destroyed(FileEdit* self)
{	
	assert(self);

	if (self->property) {
		psy_property_disconnect(self->property, self);
	}
}

FileEdit* fileedit_alloc(void)
{
	return (FileEdit*)malloc(sizeof(FileEdit));
}

FileEdit* fileedit_allocinit(psy_ui_Component* parent)
{
	FileEdit* rv;

	rv = fileedit_alloc();
	if (rv) {
		fileedit_init(rv, parent);
		psy_ui_component_deallocate_after_destroyed(&rv->component);
	}
	return rv;
}

void fileedit_data_exchange(FileEdit* self, psy_Property* property)
{
	assert(self);
	assert(property);

	self->property = property;
	if (property) {
		fileedit_on_property_changed(self, property);
		psy_ui_textarea_data_exchange(&self->edit, self->property);
		psy_property_connect(property, self,
			fileedit_on_property_changed);
		psy_signal_connect(&self->property->before_destroyed, self,
			fileedit_before_property_destroyed);
	}
}

void fileedit_on_property_changed(FileEdit* self, psy_Property* sender)
{
}

void fileedit_before_property_destroyed(FileEdit* self, psy_Property* sender)
{
	assert(self);

	self->property = NULL;
}

void fileedit_on_dialog(FileEdit* self,psy_ui_Component* sender)
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

void fileedit_on_edit_keydown(FileEdit* self, psy_ui_Component* sender,
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

void fileedit_on_edit_reject(FileEdit* self, psy_ui_TextArea* sender)
{
	psy_ui_component_set_focus(&self->component);
}

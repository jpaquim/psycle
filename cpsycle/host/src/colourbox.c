/*
** This source is free software; you can redistribute itand /or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "colourbox.h"
/* host */
#include "styles.h"
/* ui */
#include <uicolordialog.h>
/* platform */
#include "../../detail/portable.h"

/* FontBox */

/* prototypes */
static void colourbox_on_destroyed(ColourBox*);
static void colourbox_on_property_changed(ColourBox*,
	psy_Property* sender);
static void colourbox_before_property_destroyed(ColourBox*, psy_Property* sender);
static void colourbox_on_dialog(ColourBox*, psy_ui_Component* sender);

/* vtable */
static psy_ui_ComponentVtable vtable;
static psy_ui_ComponentVtable super_vtable;
static bool vtable_initialized = FALSE;

static void vtable_init(ColourBox* self)
{
	assert(self);

	if (!vtable_initialized) {
		vtable = *(self->component.vtable);
		super_vtable = *(self->component.vtable);
		vtable.on_destroyed =
			(psy_ui_fp_component)
			colourbox_on_destroyed;		
		vtable_initialized = TRUE;
	}
	psy_ui_component_set_vtable(colourbox_base(self), &vtable);
}

/* implementation */
void colourbox_init(ColourBox* self, psy_ui_Component* parent)
{	
	assert(self);
	
	psy_ui_component_init(&self->component, parent, NULL);
	vtable_init(self);
	self->property = NULL;
	psy_ui_component_set_default_align(&self->component,
		psy_ui_ALIGN_LEFT, psy_ui_margin_make_em(
			0.0, 1.0, 0.0, 0.0));	
	// psy_ui_component_set_align_expand(&self->component, psy_ui_HEXPAND);
	psy_ui_textarea_init_single_line(&self->edit, &self->component);	
	psy_ui_textarea_set_char_number(&self->edit, (int)20.0);
	psy_ui_component_init(&self->colour, &self->component, NULL);	
	psy_ui_component_set_preferred_size(&self->colour,
		psy_ui_size_make_em(4.0, 1.0));	
	psy_ui_textarea_data_exchange(&self->edit, self->property);
	psy_ui_button_init_connect(&self->dialog, &self->component,
		self, colourbox_on_dialog);
	psy_ui_button_prevent_translation(&self->dialog);
	psy_ui_button_set_text(&self->dialog, "...");
}

void colourbox_on_destroyed(ColourBox* self)
{	
	assert(self);

	if (self->property) {
		psy_property_disconnect(self->property, self);
	}
}

ColourBox* colourbox_alloc(void)
{
	return (ColourBox*)malloc(sizeof(ColourBox));
}

ColourBox* colourbox_allocinit(psy_ui_Component* parent)
{
	ColourBox* rv;

	rv = colourbox_alloc();
	if (rv) {
		colourbox_init(rv, parent);
		psy_ui_component_deallocate_after_destroyed(&rv->component);
	}
	return rv;
}

void colourbox_data_exchange(ColourBox* self, psy_Property* property)
{
	assert(self);
	assert(property);

	self->property = property;
	if (property) {
		colourbox_on_property_changed(self, property);
		psy_ui_textarea_data_exchange(&self->edit, property);
		psy_property_connect(property, self,
			colourbox_on_property_changed);
		psy_signal_connect(&self->property->before_destroyed, self,
			colourbox_before_property_destroyed);
	}
}

void colourbox_on_property_changed(ColourBox* self, psy_Property* sender)
{	
	psy_ui_component_set_background_colour(&self->colour,
		psy_ui_colour_make((uint32_t)psy_property_item_int(
			self->property)));	
}

void colourbox_before_property_destroyed(ColourBox* self, psy_Property* sender)
{
	assert(self);

	self->property = NULL;
}

void colourbox_on_dialog(ColourBox* self,psy_ui_Component* sender)
{
	psy_ui_ColourDialog dialog;

	psy_ui_colourdialog_init(&dialog, &self->component);
	psy_ui_colourdialog_setcolour(&dialog,
		psy_ui_colour_make((uint32_t)psy_property_item_int(self->property)));
	if (psy_ui_colourdialog_execute(&dialog)) {
		psy_ui_Colour colour;

		colour = psy_ui_colourdialog_colour(&dialog);
		psy_property_set_item_int(self->property,
			psy_ui_colour_colorref(&colour));
	}
	psy_ui_colourdialog_dispose(&dialog);
}

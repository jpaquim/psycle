/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "searchfield.h"
/* host */
#include "resources/resource.h"
#include "styles.h"
/* platform */
#include "../../detail/portable.h"


/* prototypes */
static void searchfield_on_destroyed(SearchField*);
static void searchfield_on_edit_focus(SearchField*, psy_ui_Component* sender);
static void searchfield_on_edit_change(SearchField*, psy_ui_Component* sender);
static void searchfield_on_accept(SearchField*, psy_ui_Component* sender);
static void searchfield_on_reject(SearchField*, psy_ui_Component* sender);
static void searchfield_reset(SearchField*);
static void searchfield_on_language_changed(SearchField*);
static void searchfield_check_default(SearchField*);

/* vtable */
static psy_ui_ComponentVtable searchfield_vtable;
static bool searchfield_vtable_initialized = FALSE;

static void searchfield_vtable_init(SearchField* self)
{
	if (!searchfield_vtable_initialized) {
		searchfield_vtable = *(self->component.vtable);
		searchfield_vtable.on_destroyed =
			(psy_ui_fp_component_event)
			searchfield_on_destroyed;
		searchfield_vtable.onlanguagechanged =
			(psy_ui_fp_component_onlanguagechanged)
			searchfield_on_language_changed;
		searchfield_vtable_initialized = TRUE;
	}
	psy_ui_component_set_vtable(&self->component, &searchfield_vtable);
}

/* implementation */
void searchfield_init(SearchField* self, psy_ui_Component* parent)
{
	psy_ui_component_init(&self->component, parent, NULL);
	searchfield_vtable_init(self);
	psy_signal_init(&self->signal_changed);
	self->defaulttext = strdup("Search");
	psy_ui_component_set_style_type(&self->component, STYLE_SEARCHFIELD);
	psy_ui_component_set_style_type_select(&self->component,
		STYLE_SEARCHFIELD_SELECT);	
	psy_ui_image_init_resource_transparency(&self->image, &self->component,
		IDB_SEARCH_DARK, psy_ui_colour_white());	
	psy_ui_component_set_align(psy_ui_image_base(&self->image),
		psy_ui_ALIGN_LEFT);
	psy_ui_component_set_margin(psy_ui_image_base(&self->image),
		psy_ui_margin_make_em(0.0, 1.0, 0.0, 1.0));	
	psy_ui_textarea_init_single_line(&self->edit, &self->component);	
	psy_ui_textarea_set_char_number(&self->edit, 42);
	psy_ui_component_set_align(psy_ui_textarea_base(&self->edit),
		psy_ui_ALIGN_LEFT);
	searchfield_reset(self);
	psy_signal_connect(&self->edit.pane.component.signal_focus,
		self, searchfield_on_edit_focus);
	psy_signal_connect(&self->edit.signal_change,
		self, searchfield_on_edit_change);
	psy_ui_textarea_enable_input_field(&self->edit);
	psy_signal_connect(&self->edit.signal_accept,
		self, searchfield_on_accept);
	psy_signal_connect(&self->edit.signal_reject,
		self, searchfield_on_reject);		
}

void searchfield_on_destroyed(SearchField* self)
{
	psy_signal_dispose(&self->signal_changed);
	free(self->defaulttext);
	self->defaulttext = NULL;
}

void searchfield_set_default_text(SearchField* self, const char* text)
{
	psy_strreset(&self->defaulttext, text);
	psy_signal_prevent_all(&self->edit.signal_change);
	searchfield_check_default(self);
	psy_signal_enable_all(&self->edit.signal_change);
}

const char* searchfield_text(const SearchField* self)
{
	if (self->hasdefaulttext) {
		return "";
	}
	return psy_ui_textarea_text(&self->edit);
}

void searchfield_on_edit_focus(SearchField* self, psy_ui_Component* sender)
{
	psy_ui_component_add_style_state(&self->component, psy_ui_STYLESTATE_SELECT);
	if (self->hasdefaulttext) {
		psy_ui_textarea_set_text(&self->edit, "");
	}
}

void searchfield_on_edit_change(SearchField* self, psy_ui_Component* sender)
{
	self->hasdefaulttext = FALSE;
	psy_signal_emit(&self->signal_changed, self, 0);	
}

void searchfield_on_accept(SearchField* self,
	psy_ui_Component* sender)
{
	if (psy_strlen(psy_ui_textarea_text(&self->edit)) == 0) {
		searchfield_reset(self);
	}	
}

void searchfield_on_reject(SearchField* self,
	psy_ui_Component* sender)
{	
	searchfield_reset(self);	
}

void searchfield_reset(SearchField* self)
{			
	self->hasdefaulttext = TRUE;
	searchfield_check_default(self);	
	psy_signal_emit(&self->signal_changed, self, 0);	
	psy_ui_component_remove_style_state(&self->component,
		psy_ui_STYLESTATE_SELECT);
	psy_ui_component_set_focus(psy_ui_component_parent(&self->component));	
}

void searchfield_on_language_changed(SearchField* self)
{
	searchfield_check_default(self);
}

void searchfield_check_default(SearchField* self)
{
	if (self->hasdefaulttext) {
		psy_signal_prevent_all(&self->edit.signal_change);
		psy_ui_textarea_set_text(&self->edit, psy_ui_translate(self->defaulttext));
		psy_signal_enable_all(&self->edit.signal_change);		
	}
}

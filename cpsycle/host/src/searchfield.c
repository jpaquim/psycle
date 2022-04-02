/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
**  copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "searchfield.h"
/* host */
#include "resources/resource.h"
#include "styles.h"
/* platform */
#include "../../detail/portable.h"

/*
** SearchField
** prototypes
*/
static void searchfield_on_destroy(SearchField*);
static void searchfield_oneditfocus(SearchField*, psy_ui_Component* sender);
static void searchfield_oneditchange(SearchField*, psy_ui_Component* sender);
static void searchfield_onaccept(SearchField*, psy_ui_Component* sender);
static void searchfield_onreject(SearchField*, psy_ui_Component* sender);
static void searchfield_reset(SearchField*);
static void searchfield_onlanguagechanged(SearchField*);
static void searchfield_checkdefault(SearchField*);
/* vtable */
static psy_ui_ComponentVtable searchfield_vtable;
static bool searchfield_vtable_initialized = FALSE;

static void searchfield_vtable_init(SearchField* self)
{
	if (!searchfield_vtable_initialized) {
		searchfield_vtable = *(self->component.vtable);
		searchfield_vtable.on_destroy =
			(psy_ui_fp_component_event)
			searchfield_on_destroy;
		searchfield_vtable.onlanguagechanged =
			(psy_ui_fp_component_onlanguagechanged)
			searchfield_onlanguagechanged;
		searchfield_vtable_initialized = TRUE;
	}
	self->component.vtable = &searchfield_vtable;
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
	psy_ui_textarea_setcharnumber(&self->edit, 42);
	searchfield_reset(self);
	psy_signal_connect(&self->edit.component.signal_focus,
		self, searchfield_oneditfocus);
	psy_signal_connect(&self->edit.signal_change,
		self, searchfield_oneditchange);
	psy_ui_textarea_enableinputfield(&self->edit);
	psy_signal_connect(&self->edit.signal_accept,
		self, searchfield_onaccept);
	psy_signal_connect(&self->edit.signal_reject,
		self, searchfield_onreject);
	psy_ui_component_set_align(psy_ui_textarea_base(&self->edit),
		psy_ui_ALIGN_CLIENT);	
}

void searchfield_on_destroy(SearchField* self)
{
	psy_signal_dispose(&self->signal_changed);
	free(self->defaulttext);
	self->defaulttext = NULL;
}

void searchfield_setdefaulttext(SearchField* self, const char* text)
{
	psy_strreset(&self->defaulttext, text);
	psy_signal_preventall(&self->edit.signal_change);
	searchfield_checkdefault(self);
	psy_signal_enableall(&self->edit.signal_change);
}

const char* searchfield_text(const SearchField* self)
{
	if (self->hasdefaulttext) {
		return "";
	}
	return psy_ui_textarea_text(&self->edit);
}

void searchfield_oneditfocus(SearchField* self, psy_ui_Component* sender)
{
	psy_ui_component_addstylestate(&self->component, psy_ui_STYLESTATE_SELECT);
	if (self->hasdefaulttext) {
		psy_ui_textarea_settext(&self->edit, "");
	}
}

void searchfield_oneditchange(SearchField* self, psy_ui_Component* sender)
{
	self->hasdefaulttext = FALSE;
	psy_signal_emit(&self->signal_changed, self, 0);	
}

void searchfield_onaccept(SearchField* self,
	psy_ui_Component* sender)
{
	if (psy_strlen(psy_ui_textarea_text(&self->edit)) == 0) {
		searchfield_reset(self);
	}	
}

void searchfield_onreject(SearchField* self,
	psy_ui_Component* sender)
{	
	searchfield_reset(self);	
}

void searchfield_reset(SearchField* self)
{			
	self->hasdefaulttext = TRUE;
	searchfield_checkdefault(self);	
	psy_signal_emit(&self->signal_changed, self, 0);	
	psy_ui_component_removestylestate(&self->component,
		psy_ui_STYLESTATE_SELECT);
	psy_ui_component_set_focus(psy_ui_component_parent(&self->component));	
}

void searchfield_onlanguagechanged(SearchField* self)
{
	searchfield_checkdefault(self);
}

void searchfield_checkdefault(SearchField* self)
{
	if (self->hasdefaulttext) {
		psy_signal_preventall(&self->edit.signal_change);
		psy_ui_textarea_settext(&self->edit, psy_ui_translate(self->defaulttext));
		psy_signal_enableall(&self->edit.signal_change);		
	}
}

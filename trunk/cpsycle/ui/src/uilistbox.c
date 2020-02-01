// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "uilistbox.h"
#include "uiimpfactory.h"
#include "uiapp.h"

extern psy_ui_App app;

static void ondestroy(psy_ui_ListBox*, psy_ui_Component* sender);

void psy_ui_listbox_init(psy_ui_ListBox* self, psy_ui_Component* parent)
{  
	self->imp = psy_ui_impfactory_allocinit_listboximp(psy_ui_app_impfactory(&app), &self->component, parent);
	psy_ui_component_init_imp(psy_psy_ui_listbox_base(self), parent,
		&self->imp->component_imp);	
	psy_signal_connect(&self->component.signal_destroy, self, ondestroy);
	psy_signal_init(&self->signal_selchanged);		
}

void psy_ui_listbox_init_multiselect(psy_ui_ListBox* self, psy_ui_Component* parent)
{  
	self->imp = psy_ui_impfactory_allocinit_listboximp_multiselect(psy_ui_app_impfactory(&app), &self->component, parent);
	psy_ui_component_init_imp(psy_psy_ui_listbox_base(self), parent,
		&self->imp->component_imp);
	psy_signal_connect(&self->component.signal_destroy, self, ondestroy);
	psy_signal_init(&self->signal_selchanged);
}

void ondestroy(psy_ui_ListBox* self, psy_ui_Component* sender)
{
	psy_signal_dispose(&self->signal_selchanged);
}

intptr_t psy_ui_listbox_addtext(psy_ui_ListBox* self, const char* text)
{	
	return self->imp->vtable->dev_addtext(self->imp, text);
}

void psy_ui_listbox_settext(psy_ui_ListBox* self, const char* text, intptr_t index)
{
	self->imp->vtable->dev_settext(self->imp, text, index);
}

void psy_ui_listbox_clear(psy_ui_ListBox* self)
{
	self->imp->vtable->dev_clear(self->imp);
}

void psy_ui_listbox_setcursel(psy_ui_ListBox* self, intptr_t index)
{
	self->imp->vtable->dev_setcursel(self->imp, index);
}

intptr_t psy_ui_listbox_cursel(psy_ui_ListBox* self)
{
	return self->imp->vtable->dev_cursel(self->imp);
}

void psy_ui_listbox_selitems(psy_ui_ListBox* self, int* items, int maxitems)
{	
	self->imp->vtable->dev_selitems(self->imp, items, maxitems);
}

intptr_t psy_ui_listbox_selcount(psy_ui_ListBox* self)
{
	return self->imp->vtable->dev_selcount(self->imp);
}

// psy_ui_ListBoxImp vtable
static int dev_addtext(psy_ui_ListBoxImp* self, const char* text) { return -1; }
static void dev_settext(psy_ui_ListBoxImp* self, const char* text, intptr_t index) { }
static void dev_text(psy_ui_ListBoxImp* self, char* text, intptr_t index) { }
static void dev_setstyle(psy_ui_ListBoxImp* self, int style) { }
static void dev_clear(psy_ui_ListBoxImp* self) { }
static void dev_setcursel(psy_ui_ListBoxImp* self, intptr_t index) { }
static intptr_t dev_cursel(psy_ui_ListBoxImp* self) { return -1; }
static void dev_selitems(psy_ui_ListBoxImp* self, int* items, int maxitems) { }
static  intptr_t dev_selcount(psy_ui_ListBoxImp* self) { return 0;  }

static psy_ui_ListBoxImpVTable listbox_imp_vtable;
static int listbox_imp_vtable_initialized = 0;

static void listbox_imp_vtable_init(void)
{
	if (!listbox_imp_vtable_initialized) {
		listbox_imp_vtable.dev_addtext = dev_addtext;
		listbox_imp_vtable.dev_settext = dev_settext;
		listbox_imp_vtable.dev_text = dev_text;
		listbox_imp_vtable.dev_setstyle = dev_setstyle;
		listbox_imp_vtable.dev_clear = dev_clear;
		listbox_imp_vtable.dev_setcursel = dev_setcursel;
		listbox_imp_vtable.dev_cursel = dev_cursel;
		listbox_imp_vtable.dev_selitems = dev_selitems;
		listbox_imp_vtable.dev_selcount = dev_selcount;
		listbox_imp_vtable_initialized = 1;
	}
}

void psy_ui_listboximp_init(psy_ui_ListBoxImp* self)
{
	listbox_imp_vtable_init();
	self->vtable = &listbox_imp_vtable;
}

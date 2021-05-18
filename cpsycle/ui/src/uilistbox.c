/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
**  copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "uilistbox.h"
/* local */
#ifdef PSY_USE_PLATFORM_LISTBOX
#include "uiimpfactory.h"
#else
#include "imps/natives/uinativelistboximp.h"
#endif
#include "uiapp.h"
/* platform */
#include "../../detail/portable.h"
#include "../../detail/trace.h"

extern psy_ui_App app;

static void ondestroy(psy_ui_ListBox*, psy_ui_Component* sender);
static void psy_ui_listbox_onpreferredsize(psy_ui_ListBox*,
	psy_ui_Size* limit, psy_ui_Size* rv);

static psy_ui_ComponentVtable psy_ui_listbox_vtable;
static bool psy_ui_listbox_vtable_initialized = FALSE;

static void psy_ui_listbox_vtable_init(psy_ui_ListBox* self)
{
	if (!psy_ui_listbox_vtable_initialized) {
		psy_ui_listbox_vtable = *(self->component.vtable);		
		psy_ui_listbox_vtable.onpreferredsize =
			(psy_ui_fp_component_onpreferredsize)
			psy_ui_listbox_onpreferredsize;		
		psy_ui_listbox_vtable_initialized = TRUE;
	}
}

void psy_ui_listbox_init(psy_ui_ListBox* self, psy_ui_Component* parent)
{  
	psy_ui_ComponentImp* imp;

#ifdef PSY_USE_PLATFORM_LISTBOX
	imp = psy_ui_impfactory_allocinit_listboximp(
		psy_ui_app_impfactory(psy_ui_app()), &self->component, parent);
#else
	imp = psy_ui_native_listboximp_allocinit(&self->component, parent)->imp;
#endif
	psy_ui_component_init_imp(psy_ui_listbox_base(self), parent, imp);
	psy_ui_listbox_vtable_init(self);
	self->component.vtable = &psy_ui_listbox_vtable;	
	psy_signal_connect(&self->component.signal_destroy, self, ondestroy);
	psy_signal_init(&self->signal_selchanged);
	self->charnumber = 0;
}

void psy_ui_listbox_init_multiselect(psy_ui_ListBox* self, psy_ui_Component*
	parent)
{  
	psy_ui_ComponentImp* imp;

#ifdef PSY_USE_PLATFORM_LISTBOX
	imp = psy_ui_impfactory_allocinit_listboximp_multiselect(
		psy_ui_app_impfactory(psy_ui_app()), &self->component, parent);
#else
	imp = psy_ui_native_listboximp_multiselect_allocinit(&self->component, parent)->imp;
#endif
	psy_ui_component_init_imp(psy_ui_listbox_base(self), parent, imp);
	psy_signal_connect(&self->component.signal_destroy, self, ondestroy);
	psy_signal_init(&self->signal_selchanged);
	self->charnumber = 0;
}

void ondestroy(psy_ui_ListBox* self, psy_ui_Component* sender)
{
	psy_signal_dispose(&self->signal_selchanged);
}

intptr_t psy_ui_listbox_addtext(psy_ui_ListBox* self, const char* text)
{	
	return psy_ui_listboximp_vtable(self->component.imp)->dev_addtext(
		self->component.imp->extended_imp, text);
}

void psy_ui_listbox_settext(psy_ui_ListBox* self, const char* text,
	intptr_t index)
{	
	if (index >= psy_ui_listbox_count(self)) {
		intptr_t i;

		for (i = psy_ui_listbox_count(self); i <= index; ++i) {
			psy_ui_listbox_addtext(self, "");
		}
	}	
	psy_ui_listboximp_vtable(self->component.imp)->dev_settext(
		self->component.imp->extended_imp, text, index);
}

void psy_ui_listbox_clear(psy_ui_ListBox* self)
{	
	psy_ui_listboximp_vtable(self->component.imp)->dev_clear(
		self->component.imp->extended_imp);
}

void psy_ui_listbox_setcursel(psy_ui_ListBox* self, intptr_t index)
{	
	psy_ui_listboximp_vtable(self->component.imp)->dev_setcursel(
		self->component.imp->extended_imp, index);
}

intptr_t psy_ui_listbox_cursel(psy_ui_ListBox* self)
{		
	return psy_ui_listboximp_vtable(self->component.imp)->dev_cursel(
		self->component.imp->extended_imp);
}

void psy_ui_listbox_addsel(psy_ui_ListBox* self, intptr_t index)
{		
	psy_ui_listboximp_vtable(self->component.imp)->dev_addsel(
		self->component.imp->extended_imp, index);
}

void psy_ui_listbox_selitems(psy_ui_ListBox* self, intptr_t* items, intptr_t maxitems)
{		
	psy_ui_listboximp_vtable(self->component.imp)->dev_selitems(
		self->component.imp->extended_imp, items, maxitems);
}

intptr_t psy_ui_listbox_selcount(psy_ui_ListBox* self)
{		
	return psy_ui_listboximp_vtable(self->component.imp)->dev_selcount(
		self->component.imp->extended_imp);
}

intptr_t psy_ui_listbox_count(psy_ui_ListBox* self)
{	
	return psy_ui_listboximp_vtable(self->component.imp)->dev_count(
		self->component.imp->extended_imp);
}

void psy_ui_listbox_setcharnumber(psy_ui_ListBox* self, double num)
{
	self->charnumber = num;
}

void psy_ui_listbox_onpreferredsize(psy_ui_ListBox* self,
	psy_ui_Size* limit, psy_ui_Size* rv)
{
	const psy_ui_TextMetric* tm;
	
	tm = psy_ui_component_textmetric(&self->component);
	rv->width = (self->charnumber == 0)
		? psy_ui_value_make_px(tm->tmAveCharWidth * 40) // tm.tmAveCharWidth
		: psy_ui_value_make_px(tm->tmAveCharWidth * self->charnumber); // 
	rv->height = psy_ui_value_make_px((tm->tmHeight * 1.2) *
		psy_ui_listbox_count(self));	
}

/* psy_ui_ComponentImp vtable */
static int dev_addtext(psy_ui_ComponentImp* self, const char* text) { return -1; }
static void dev_settext(psy_ui_ComponentImp* self, const char* text,
	intptr_t index)
{
}
static void dev_text(psy_ui_ComponentImp* self, char* text, intptr_t index) { }
static void dev_setstyle(psy_ui_ComponentImp* self, int style) { }
static void dev_clear(psy_ui_ComponentImp* self) { }
static void dev_setcursel(psy_ui_ComponentImp* self, intptr_t index) { }
static intptr_t dev_cursel(psy_ui_ComponentImp* self) { return -1; }
static void dev_addsel(psy_ui_ComponentImp* self, intptr_t index) { }
static void dev_selitems(psy_ui_ComponentImp* self, intptr_t* items, intptr_t maxitems) { }
static intptr_t dev_selcount(psy_ui_ComponentImp* self) { return 0;  }
static intptr_t dev_count(psy_ui_ComponentImp* self) { return 0; }

static psy_ui_ListBoxImpVTable listbox_imp_vtable;
static bool listbox_imp_vtable_initialized = FALSE;

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
		listbox_imp_vtable.dev_addsel = dev_addsel;
		listbox_imp_vtable.dev_selitems = dev_selitems;
		listbox_imp_vtable.dev_selcount = dev_selcount;
		listbox_imp_vtable.dev_count = dev_count;
		listbox_imp_vtable_initialized = TRUE;
	}
}

void psy_ui_listboximp_extend(psy_ui_ComponentImp* self)
{
	listbox_imp_vtable_init();
	self->extended_vtable = &listbox_imp_vtable;
}


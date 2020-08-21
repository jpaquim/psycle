// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "uilistbox.h"
#include "uiimpfactory.h"
#include "uiapp.h"

#include <string.h>

#include "../../detail/portable.h"

#ifdef PSY_USE_PLATFORM_LISTBOX

extern psy_ui_App app;

static void ondestroy(psy_ui_ListBox*, psy_ui_Component* sender);
static void psy_ui_listbox_onpreferredsize(psy_ui_ListBox*,
	psy_ui_Size* limit, psy_ui_Size* rv);

static psy_ui_ComponentVtable psy_ui_listbox_vtable;
static int psy_ui_listbox_vtable_initialized = 0;

static void psy_ui_listbox_vtable_init(psy_ui_ListBox* self)
{
	if (!psy_ui_listbox_vtable_initialized) {
		psy_ui_listbox_vtable = *(self->component.vtable);		
		psy_ui_listbox_vtable.onpreferredsize =
			(psy_ui_fp_onpreferredsize)
			psy_ui_listbox_onpreferredsize;		
		psy_ui_listbox_vtable_initialized = 1;
	}
}

void psy_ui_listbox_init(psy_ui_ListBox* self, psy_ui_Component* parent)
{  
	self->imp = psy_ui_impfactory_allocinit_listboximp(
		psy_ui_app_impfactory(&app), &self->component, parent);
	psy_ui_component_init_imp(psy_ui_listbox_base(self), parent,
		&self->imp->component_imp);
	psy_ui_listbox_vtable_init(self);
	self->component.vtable = &psy_ui_listbox_vtable;
	psy_signal_connect(&self->component.signal_destroy, self, ondestroy);
	psy_signal_init(&self->signal_selchanged);

}

void psy_ui_listbox_init_multiselect(psy_ui_ListBox* self, psy_ui_Component*
	parent)
{  
	self->imp = psy_ui_impfactory_allocinit_listboximp_multiselect(
		psy_ui_app_impfactory(&app), &self->component, parent);
	psy_ui_component_init_imp(psy_ui_listbox_base(self), parent,
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

void psy_ui_listbox_settext(psy_ui_ListBox* self, const char* text,
	intptr_t index)
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

void psy_ui_listbox_setcharnumber(psy_ui_ListBox* self, uintptr_t num)
{
	self->charnumber = num;
}

void psy_ui_listbox_onpreferredsize(psy_ui_ListBox* self,
	psy_ui_Size* limit, psy_ui_Size* rv)
{
	psy_ui_TextMetric tm;

	tm = psy_ui_component_textmetric(&self->component);
	rv->width = (self->charnumber == 0)
		? psy_ui_value_makepx(tm.tmAveCharWidth * 40)
		: psy_ui_value_makepx(tm.tmAveCharWidth * self->charnumber);
	rv->height = psy_ui_value_makepx((int)(tm.tmHeight * 1.2) *
		psy_ui_listbox_selcount(self));
}

#endif

// psy_ui_ListBoxImp vtable
static int dev_addtext(psy_ui_ListBoxImp* self, const char* text) { return -1; }
static void dev_settext(psy_ui_ListBoxImp* self, const char* text,
	intptr_t index)
{
}
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

#ifndef PSY_USE_PLATFORM_LISTBOX

// ListBoxClient

static void psy_ui_listboxclient_ondestroy(psy_ui_ListBoxClient*,
	psy_ui_Component* sender);
static void psy_ui_listboxclient_ondraw(psy_ui_ListBoxClient*,
	psy_ui_Graphics*);
static void psy_ui_listboxclient_onpreferredsize(psy_ui_ListBoxClient*,
	psy_ui_Size* limit, psy_ui_Size* rv);
static void psy_ui_listboxclient_onmousedown(psy_ui_ListBoxClient*,
	psy_ui_MouseEvent*);
static void psy_ui_listboxclient_onsize(psy_ui_ListBoxClient*, const psy_ui_Size*);

static psy_ui_ComponentVtable psy_ui_listboxclient_vtable;
static int psy_ui_listboxclient_vtable_initialized = 0;

static void psy_ui_listboxclient_vtable_init(psy_ui_ListBoxClient* self)
{
	if (!psy_ui_listboxclient_vtable_initialized) {
		psy_ui_listboxclient_vtable = *(self->component.vtable);
		psy_ui_listboxclient_vtable.ondraw = (psy_ui_fp_ondraw)
			psy_ui_listboxclient_ondraw;
		psy_ui_listboxclient_vtable.onpreferredsize =
			(psy_ui_fp_onpreferredsize)
			psy_ui_listboxclient_onpreferredsize;
		psy_ui_listboxclient_vtable.onmousedown = (psy_ui_fp_onmousedown)
			psy_ui_listboxclient_onmousedown;
		psy_ui_listboxclient_vtable.onsize = (psy_ui_fp_onsize)
			psy_ui_listboxclient_onsize;
		psy_ui_listboxclient_vtable_initialized = 1;
	}
}

void psy_ui_listboxclient_init(psy_ui_ListBoxClient* self, psy_ui_Component*
	parent)
{
	psy_ui_component_init(&self->component, parent);
	psy_ui_listboxclient_vtable_init(self);
	self->component.vtable = &psy_ui_listboxclient_vtable;
	psy_signal_connect(&self->component.signal_destroy, self,
		psy_ui_listboxclient_ondestroy);
	psy_table_init(&self->items);
	psy_ui_component_setwheelscroll(&self->component, 4);
	psy_ui_component_setoverflow(&self->component, psy_ui_OVERFLOW_VSCROLL);
	self->selindex = -1;
	self->charnumber = 0;
	psy_signal_init(&self->signal_selchanged);
}

void psy_ui_listboxclient_ondestroy(psy_ui_ListBoxClient* self,
	psy_ui_Component* sender)
{
	psy_signal_dispose(&self->signal_selchanged);
	psy_table_disposeall(&self->items, NULL);
}

void psy_ui_listboxclient_onsize(psy_ui_ListBoxClient* self, const psy_ui_Size* size)
{
	psy_ui_TextMetric tm;
	int lineheight;

	tm = psy_ui_component_textmetric(&self->component);
	lineheight = (int)(tm.tmHeight * 1.2);
	self->component.scrollstepy = lineheight;
}

void psy_ui_listboxclient_ondraw(psy_ui_ListBoxClient* self, psy_ui_Graphics* g)
{
	psy_TableIterator it;
	psy_ui_TextMetric tm;
	psy_ui_IntSize size;
	int cpx = 0;
	int cpy = 0;
	int lineheight;
		
	tm = psy_ui_component_textmetric(&self->component);
	size = psy_ui_intsize_init_size(psy_ui_component_size(&self->component),
		&tm);
	lineheight = (int)(tm.tmHeight * 1.2);
	for (it = psy_table_begin(&self->items);
			!psy_tableiterator_equal(&it, psy_table_end());
			psy_tableiterator_inc(&it)) {
		char* itemtext;

		itemtext = (char*)psy_tableiterator_value(&it);
		if (self->selindex != -1 && self->selindex == (intptr_t)
				psy_tableiterator_key(&it)) {
			psy_ui_Rectangle r;

			r = psy_ui_rectangle_make(0, cpy, size.width, lineheight);			
			psy_ui_drawsolidrectangle(g, r, psy_ui_color_make(0x009B7800));
		}
		psy_ui_textout(g, cpx, cpy, itemtext, strlen(itemtext));
		cpy += lineheight;
	}
}

void psy_ui_listboxclient_onpreferredsize(psy_ui_ListBoxClient* self,
	psy_ui_Size* limit, psy_ui_Size* rv)
{
	psy_ui_TextMetric tm;

	tm = psy_ui_component_textmetric(&self->component);
	rv->width = (self->charnumber == 0)
		? psy_ui_value_makepx(tm.tmAveCharWidth * 30)
		: psy_ui_value_makepx(tm.tmAveCharWidth * self->charnumber);
	rv->height = psy_ui_value_makepx((int)(tm.tmHeight * 1.2) * psy_table_size(
		&self->items));
}

void psy_ui_listboxclient_onmousedown(psy_ui_ListBoxClient* self,
	psy_ui_MouseEvent* ev)
{
	psy_ui_TextMetric tm;
	intptr_t index;

	tm = psy_ui_component_textmetric(&self->component);
	index = ev->y / (int)(tm.tmHeight * 1.2);
	if (index < (intptr_t)psy_table_size(&self->items)) {
		self->selindex = index;
		psy_ui_component_invalidate(&self->component);
		psy_signal_emit(&self->signal_selchanged, self, 0);
	}
}

void psy_ui_listboxclient_clear(psy_ui_ListBoxClient* self)
{
	self->selindex = -1;
	psy_table_disposeall(&self->items, NULL);
	psy_table_init(&self->items);
	psy_ui_component_updateoverflow(&self->component);
	psy_ui_component_invalidate(&self->component);
}

intptr_t psy_ui_listboxclient_addtext(psy_ui_ListBoxClient* self,
	const char* text)
{
	psy_table_insert(&self->items, psy_table_size(&self->items),
		(text)
			? strdup(text)
			: "");
	psy_ui_component_updateoverflow(&self->component);
	psy_ui_component_invalidate(&self->component);
	return psy_table_size(&self->items);
}

void psy_ui_listboxclient_setcursel(psy_ui_ListBoxClient* self, intptr_t index)
{
	self->selindex = index;
	psy_ui_component_invalidate(&self->component);
}

intptr_t psy_ui_listboxclient_cursel(psy_ui_ListBoxClient* self)
{
	return self->selindex;
}

void psy_ui_listboxclient_setcharnumber(psy_ui_ListBoxClient* self, uintptr_t num)
{
	self->charnumber = num;
}

// ListBox
static void ondestroy(psy_ui_ListBox*, psy_ui_Component* sender);
static void psy_ui_listbox_onselchanged(psy_ui_ListBox*, psy_ui_ListBoxClient*
	sender);

void psy_ui_listbox_init(psy_ui_ListBox* self, psy_ui_Component* parent)
{
	psy_ui_component_init(&self->component, parent);
	psy_ui_listboxclient_init(&self->client, &self->component);
	psy_ui_scroller_init(&self->scroller, &self->client.component,
		&self->component);
	psy_ui_component_setalign(&self->scroller.component, psy_ui_ALIGN_CLIENT);
	psy_signal_connect(&self->component.signal_destroy, self, ondestroy);
	psy_signal_init(&self->signal_selchanged);
	psy_signal_connect(&self->client.signal_selchanged, self,
		psy_ui_listbox_onselchanged);
}

void psy_ui_listbox_init_multiselect(psy_ui_ListBox* self, psy_ui_Component*
	parent)
{
	psy_ui_listbox_init(self, parent);
}

void ondestroy(psy_ui_ListBox* self, psy_ui_Component* sender)
{
	psy_signal_dispose(&self->signal_selchanged);
}

intptr_t psy_ui_listbox_addtext(psy_ui_ListBox* self, const char* text)
{
	return psy_ui_listboxclient_addtext(&self->client, text);
}

void psy_ui_listbox_settext(psy_ui_ListBox* self, const char* text, intptr_t
	index)
{

}

void psy_ui_listbox_text(psy_ui_ListBox* self, char* text, uintptr_t index)
{
	char* itemtext;

	itemtext = (char*)psy_table_at(&self->client.items, index);
	if (itemtext) {
		psy_snprintf(text, 256, "%s", itemtext);
	}	
}

void psy_ui_listbox_clear(psy_ui_ListBox* self)
{
	psy_ui_listboxclient_clear(&self->client);
}

void psy_ui_listbox_setcursel(psy_ui_ListBox* self, intptr_t index)
{
	psy_ui_listboxclient_setcursel(&self->client, index);
}

intptr_t psy_ui_listbox_cursel(psy_ui_ListBox* self)
{
	return psy_ui_listboxclient_cursel(&self->client);
}

void psy_ui_listbox_selitems(psy_ui_ListBox* self, int* items, int maxitems)
{

}

intptr_t psy_ui_listbox_selcount(psy_ui_ListBox* self)
{
	return 0;
}

void psy_ui_listbox_onselchanged(psy_ui_ListBox* self, psy_ui_ListBoxClient*
	sender)
{
	psy_signal_emit(&self->signal_selchanged, self, 1,
		psy_ui_listbox_cursel(self));
}

void psy_ui_listbox_setcharnumber(psy_ui_ListBox* self, uintptr_t num)
{
	psy_ui_listboxclient_setcharnumber(&self->client, num);
}

#endif

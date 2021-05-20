/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
**  copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "uinativelistboximp.h"
/* local */
#include "../../uicomponent.h"
#include "../../uiapp.h"
#include "../../uiimpfactory.h"
/* portable */
#include "../../detail/portable.h"

/* ListBoxClient */

static void psy_ui_listboxclient_ondestroy(psy_ui_ListBoxClient*,
	psy_ui_Component* sender);
static void psy_ui_listboxclient_ondraw(psy_ui_ListBoxClient*,
	psy_ui_Graphics*);
static void psy_ui_listboxclient_onpreferredsize(psy_ui_ListBoxClient*,
	psy_ui_Size* limit, psy_ui_Size* rv);
static void psy_ui_listboxclient_onmousedown(psy_ui_ListBoxClient*,
	psy_ui_MouseEvent*);
static void psy_ui_listboxclient_onsize(psy_ui_ListBoxClient*, const psy_ui_Size*);
static intptr_t  psy_ui_listboxclient_count(psy_ui_ListBoxClient* self);

static psy_ui_ComponentVtable psy_ui_listboxclient_vtable;
static int psy_ui_listboxclient_vtable_initialized = 0;

static void psy_ui_listboxclient_vtable_init(psy_ui_ListBoxClient* self)
{
	if (!psy_ui_listboxclient_vtable_initialized) {
		psy_ui_listboxclient_vtable = *(self->component.vtable);
		psy_ui_listboxclient_vtable.ondraw =
			(psy_ui_fp_component_ondraw)
			psy_ui_listboxclient_ondraw;
		psy_ui_listboxclient_vtable.onpreferredsize =
			(psy_ui_fp_component_onpreferredsize)
			psy_ui_listboxclient_onpreferredsize;
		psy_ui_listboxclient_vtable.onmousedown =
			(psy_ui_fp_component_onmouseevent)
			psy_ui_listboxclient_onmousedown;
		psy_ui_listboxclient_vtable.onsize =
			(psy_ui_fp_component_onsize)
			psy_ui_listboxclient_onsize;
		psy_ui_listboxclient_vtable_initialized = 1;
	}
}

void psy_ui_listboxclient_init(psy_ui_ListBoxClient* self, psy_ui_Component*
	parent)
{
	psy_ui_component_init(&self->component, parent, NULL);
	psy_ui_listboxclient_vtable_init(self);
	self->component.vtable = &psy_ui_listboxclient_vtable;
	psy_signal_connect(&self->component.signal_destroy, self,
		psy_ui_listboxclient_ondestroy);
	psy_table_init(&self->items);
	self->selindex = -1;
	self->charnumber = 0;
	psy_signal_init(&self->signal_selchanged);
	psy_ui_component_setwheelscroll(&self->component, 4);
	psy_ui_component_setoverflow(&self->component, psy_ui_OVERFLOW_VSCROLL);
}

void psy_ui_listboxclient_ondestroy(psy_ui_ListBoxClient* self,
	psy_ui_Component* sender)
{
	psy_signal_dispose(&self->signal_selchanged);
	psy_table_disposeall(&self->items, NULL);
}

void psy_ui_listboxclient_onsize(psy_ui_ListBoxClient* self, const psy_ui_Size* size)
{
	const psy_ui_TextMetric* tm;
	int lineheight;

	tm = psy_ui_component_textmetric(&self->component);
	lineheight = (int)(tm->tmHeight * 1.2);
	psy_ui_component_setscrollstep_height(&self->component,
		psy_ui_value_make_px(lineheight));
}

void psy_ui_listboxclient_ondraw(psy_ui_ListBoxClient* self, psy_ui_Graphics* g)
{
	psy_TableIterator it;
	const psy_ui_TextMetric* tm;
	psy_ui_RealSize size;
	int cpx = 0;
	int cpy = 0;
	int lineheight;

	tm = psy_ui_component_textmetric(&self->component);
	size = psy_ui_component_scrollsize_px(&self->component);
	lineheight = (int)(tm->tmHeight * 1.2);
	for (it = psy_table_begin(&self->items);
		!psy_tableiterator_equal(&it, psy_table_end());
		psy_tableiterator_inc(&it)) {
		char* itemtext;

		itemtext = (char*)psy_tableiterator_value(&it);
		if (self->selindex != -1 && self->selindex == (intptr_t)
			psy_tableiterator_key(&it)) {
			psy_ui_RealRectangle r;

			r = psy_ui_realrectangle_make(
				psy_ui_realpoint_make(0, cpy),
				psy_ui_realsize_make(size.width, lineheight));
			psy_ui_drawsolidrectangle(g, r, psy_ui_colour_make(0x009B7800));
		}
		psy_ui_textout(g, cpx, cpy, itemtext, psy_strlen(itemtext));
		cpy += lineheight;
	}
}

void psy_ui_listboxclient_onpreferredsize(psy_ui_ListBoxClient* self,
	psy_ui_Size* limit, psy_ui_Size* rv)
{
	const psy_ui_TextMetric* tm;

	tm = psy_ui_component_textmetric(&self->component);
	rv->width = (self->charnumber == 0)
		? psy_ui_value_make_px(tm->tmAveCharWidth * 40)
		: psy_ui_value_make_px(tm->tmAveCharWidth * self->charnumber);
	rv->height = psy_ui_value_make_px((int)(tm->tmHeight * 1.2) *
		(double)psy_ui_listboxclient_count(self));
}

void psy_ui_listboxclient_onmousedown(psy_ui_ListBoxClient* self,
	psy_ui_MouseEvent* ev)
{
	const psy_ui_TextMetric* tm;
	intptr_t index;

	tm = psy_ui_component_textmetric(&self->component);
	index = (intptr_t)(ev->pt.y / (intptr_t)(tm->tmHeight * 1.2));
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

void psy_ui_listboxclient_setcharnumber(psy_ui_ListBoxClient* self, double num)
{
	self->charnumber = num;
}

intptr_t  psy_ui_listboxclient_count(psy_ui_ListBoxClient* self)
{
	return psy_table_size(&self->items);
}

/* ListBoxImp VTable */

static intptr_t dev_addtext(psy_ui_native_ListBoxImp*, const char* text);
static void dev_settext(psy_ui_native_ListBoxImp*, const char* text, intptr_t index);
static void dev_text(psy_ui_native_ListBoxImp*, char* text, intptr_t index);
static void dev_setstyle(psy_ui_native_ListBoxImp*, int style);
static void dev_clear(psy_ui_native_ListBoxImp*);
static void dev_setcursel(psy_ui_native_ListBoxImp*, intptr_t index);
static intptr_t dev_cursel(psy_ui_native_ListBoxImp*);
static void dev_addsel(psy_ui_native_ListBoxImp*, intptr_t index);
static void dev_selitems(psy_ui_native_ListBoxImp*, intptr_t* items,
	intptr_t maxitems);
static intptr_t dev_selcount(psy_ui_native_ListBoxImp*);
static intptr_t dev_count(psy_ui_native_ListBoxImp*);

static psy_ui_ListBoxImpVTable listboximp_vtable;
static bool listboximp_vtable_initialized = FALSE;

static void listboximp_imp_vtable_init(psy_ui_native_ListBoxImp* self)
{
	if (!listboximp_vtable_initialized) {
		listboximp_vtable = *((psy_ui_ListBoxImpVTable*)self->imp->extended_vtable);
		listboximp_vtable.dev_addtext = (psy_ui_fp_listboximp_dev_addtext)dev_addtext;
		listboximp_vtable.dev_settext = (psy_ui_fp_listboximp_dev_settext)dev_settext;
		listboximp_vtable.dev_text = (psy_ui_fp_listboximp_dev_text)dev_text;
		listboximp_vtable.dev_setstyle = (psy_ui_fp_listboximp_dev_setstyle)dev_setstyle;
		listboximp_vtable.dev_clear = (psy_ui_fp_listboximp_dev_clear)dev_clear;
		listboximp_vtable.dev_setcursel = (psy_ui_fp_listboximp_dev_setcursel)dev_setcursel;
		listboximp_vtable.dev_cursel = (psy_ui_fp_listboximp_dev_cursel)dev_cursel;
		listboximp_vtable.dev_addsel = (psy_ui_fp_listboximp_dev_addsel)dev_addsel;
		listboximp_vtable.dev_selitems = (psy_ui_fp_listboximp_dev_selitems)dev_selitems;
		listboximp_vtable.dev_selcount = (psy_ui_fp_listboximp_dev_selcount)dev_selcount;
		listboximp_vtable.dev_count = (psy_ui_fp_listboximp_dev_count)dev_count;
		listboximp_vtable_initialized = TRUE;
	}
	self->imp->extended_vtable = (void*)&listboximp_vtable;
}

static void dev_dispose(psy_ui_ComponentImp*);
static void onselchanged(psy_ui_native_ListBoxImp*, psy_ui_ListBoxClient* sender);
static void dev_initialized(psy_ui_ComponentImp* context);
/* vtable */
static psy_ui_ComponentImpVTable vtable;
static bool vtable_initialized = FALSE;

static void vtable_init(psy_ui_native_ListBoxImp* self)
{
	if (!vtable_initialized) {
		vtable = *self->imp->vtable;
		vtable.dev_dispose =
			(psy_ui_fp_componentimp_dev_dispose)
			dev_dispose;
		vtable.dev_initialized =
			(psy_ui_fp_componentimp_dev_initialized)
			dev_initialized;
		vtable_initialized = TRUE;
	}
	self->imp->vtable = &vtable;
}


void psy_ui_native_listboximp_init(psy_ui_native_ListBoxImp* self,
	psy_ui_Component* component,
	psy_ui_Component* parent)
{
	self->imp = psy_ui_impfactory_allocinit_componentimp(
		psy_ui_app_impfactory(psy_ui_app()), component, parent);
	vtable_init(self);	
	psy_ui_listboximp_extend(self->imp);	
	listboximp_imp_vtable_init(self);
	self->imp->extended_imp = (void*)self;
	self->component = component;	
}

void psy_ui_native_listboximp_multiselect_init(psy_ui_native_ListBoxImp* self,
	psy_ui_Component* component,
	psy_ui_Component* parent)
{
	self->imp = psy_ui_impfactory_allocinit_componentimp(
		psy_ui_app_impfactory(psy_ui_app()), component, parent);
	vtable_init(self);	
	psy_ui_listboximp_extend(self->imp);
	self->imp->extended_imp = (void*)self;	
	listboximp_imp_vtable_init(self);	
	self->component = component;	
}

void dev_initialized(psy_ui_ComponentImp* context)
{
	psy_ui_native_ListBoxImp* self;

	self = (psy_ui_native_ListBoxImp*)context->extended_imp;
	psy_ui_listboxclient_init(&self->client, self->component);
	psy_ui_scroller_init(&self->scroller, &self->client.component,
		self->component, NULL);
	psy_ui_component_setalign(&self->scroller.component, psy_ui_ALIGN_CLIENT);		
	psy_signal_connect(&self->client.signal_selchanged, self,
		onselchanged);
}

void dev_dispose(psy_ui_ComponentImp* context)
{
	psy_ui_native_ListBoxImp* self;	
		
	self = (psy_ui_native_ListBoxImp*)context->extended_imp;
	free(self);	
}

psy_ui_native_ListBoxImp* psy_ui_native_listboximp_alloc(void)
{
	return (psy_ui_native_ListBoxImp*) malloc(sizeof(psy_ui_native_ListBoxImp));
}

psy_ui_native_ListBoxImp* psy_ui_native_listboximp_allocinit(
	struct psy_ui_Component* component,
	psy_ui_Component* parent)
{
	psy_ui_native_ListBoxImp* rv;

	rv = psy_ui_native_listboximp_alloc();
	if (rv) {
		psy_ui_native_listboximp_init(rv, component, parent);
	}
	return rv;
}

psy_ui_native_ListBoxImp* psy_ui_native_listboximp_multiselect_allocinit(
	struct psy_ui_Component* component,
	psy_ui_Component* parent)
{
	psy_ui_native_ListBoxImp* rv;

	rv = psy_ui_native_listboximp_alloc();
	if (rv) {
		psy_ui_native_listboximp_multiselect_init(rv, component, parent);
	}
	return rv;
}

intptr_t dev_addtext(psy_ui_native_ListBoxImp* self, const char* text)
{
	return psy_ui_listboxclient_addtext(&self->client, text);
}

void dev_settext(psy_ui_native_ListBoxImp* self, const char* text, intptr_t index)
{
	
}

void dev_setstyle(psy_ui_native_ListBoxImp* self, int style)
{

}

void dev_text(psy_ui_native_ListBoxImp* self, char* text, intptr_t index)
{
	char* itemtext;

	itemtext = (char*)psy_table_at(&self->client.items, index);
	if (itemtext) {
		psy_snprintf(text, 256, "%s", itemtext);
	}
}

void dev_clear(psy_ui_native_ListBoxImp* self)
{	
	psy_ui_listboxclient_clear(&self->client);
}

void dev_setcursel(psy_ui_native_ListBoxImp* self, intptr_t index)
{
	psy_ui_listboxclient_setcursel(&self->client, index);
}

void dev_addsel(psy_ui_native_ListBoxImp* self, intptr_t index)
{
	
}

intptr_t dev_cursel(psy_ui_native_ListBoxImp* self)
{
	return psy_ui_listboxclient_cursel(&self->client);
}

void dev_selitems(psy_ui_native_ListBoxImp* self, intptr_t* items, intptr_t maxitems)
{
	
}

intptr_t dev_selcount(psy_ui_native_ListBoxImp* self)
{
	return 0;
}

intptr_t dev_count(psy_ui_native_ListBoxImp* self)
{
	return psy_ui_listboxclient_count(&self->client);
}

void onselchanged(psy_ui_native_ListBoxImp* self, psy_ui_ListBoxClient* sender)
{	
	intptr_t sel;
	
	sel = dev_cursel(self);
	psy_signal_emit(&((psy_ui_ListBox*)(self->component))->signal_selchanged,
		self, 1, sel);
}

/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
**  copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "uilistbox.h"
/* local */
#include "uiimpfactory.h"
#include "uiapp.h"
/* platform */
#include "../../detail/portable.h"


/* ListBoxClient */

/* prototypes*/
static void psy_ui_listboxclient_on_destroyed(psy_ui_ListBoxClient*);
static void psy_ui_listboxclient_on_draw(psy_ui_ListBoxClient*,
	psy_ui_Graphics*);
static void psy_ui_listboxclient_on_preferred_size(psy_ui_ListBoxClient*,
	psy_ui_Size* limit, psy_ui_Size* rv);
static void psy_ui_listboxclient_on_mouse_down(psy_ui_ListBoxClient*,
	psy_ui_MouseEvent*);
static void psy_ui_listboxclient_on_size(psy_ui_ListBoxClient*);
static intptr_t psy_ui_listboxclient_count(const psy_ui_ListBoxClient*);

/* vtable */
static psy_ui_ComponentVtable psy_ui_listboxclient_vtable;
static bool psy_ui_listboxclient_vtable_initialized = FALSE;

static void psy_ui_listboxclient_vtable_init(psy_ui_ListBoxClient* self)
{
	if (!psy_ui_listboxclient_vtable_initialized) {
		psy_ui_listboxclient_vtable = *(self->component.vtable);
		psy_ui_listboxclient_vtable.on_destroyed =
			(psy_ui_fp_component)
			psy_ui_listboxclient_on_destroyed;
		psy_ui_listboxclient_vtable.ondraw =
			(psy_ui_fp_component_ondraw)
			psy_ui_listboxclient_on_draw;
		psy_ui_listboxclient_vtable.onpreferredsize =
			(psy_ui_fp_component_on_preferred_size)
			psy_ui_listboxclient_on_preferred_size;
		psy_ui_listboxclient_vtable.on_mouse_down =
			(psy_ui_fp_component_on_mouse_event)
			psy_ui_listboxclient_on_mouse_down;
		psy_ui_listboxclient_vtable.onsize =
			(psy_ui_fp_component)
			psy_ui_listboxclient_on_size;
		psy_ui_listboxclient_vtable_initialized = TRUE;
	}
	psy_ui_component_set_vtable(&self->component, &psy_ui_listboxclient_vtable);
}

/* implementation */
void psy_ui_listboxclient_init(psy_ui_ListBoxClient* self, psy_ui_Component*
	parent)
{
	psy_ui_component_init(&self->component, parent, NULL);
	psy_ui_listboxclient_vtable_init(self);	
	psy_table_init(&self->items);
	self->selindex = -1;
	self->charnumber = 0;
	psy_signal_init(&self->signal_selchanged);
	psy_ui_component_set_wheel_scroll(&self->component, 4);
	psy_ui_component_set_overflow(&self->component, psy_ui_OVERFLOW_VSCROLL);
}

void psy_ui_listboxclient_on_destroyed(psy_ui_ListBoxClient* self)
{
	psy_signal_dispose(&self->signal_selchanged);
	psy_table_dispose_all(&self->items, NULL);
}

void psy_ui_listboxclient_on_size(psy_ui_ListBoxClient* self)
{
	const psy_ui_TextMetric* tm;
	int line_height;

	tm = psy_ui_component_textmetric(&self->component);
	line_height = (int)(tm->tmHeight * 1.2);
	psy_ui_component_set_scroll_step_height(&self->component,
		psy_ui_value_make_px(line_height));
}

void psy_ui_listboxclient_on_draw(psy_ui_ListBoxClient* self,
	psy_ui_Graphics* g)
{
	uintptr_t i;
	const psy_ui_TextMetric* tm;
	psy_ui_RealSize size;
	psy_ui_RealPoint cp;	
	double line_height;

	tm = psy_ui_component_textmetric(&self->component);
	size = psy_ui_component_scroll_size_px(&self->component);
	line_height = floor(tm->tmHeight * 1.2);
	cp = psy_ui_realpoint_zero();
	for (i = 0; i < psy_table_size(&self->items); ++i) {		
		char* itemtext;

		itemtext = (char*)psy_table_at(&self->items, i);
		if (self->selindex != -1 && self->selindex == i) {			
			psy_ui_drawsolidrectangle(g,
				psy_ui_realrectangle_make(cp,
					psy_ui_realsize_make(size.width, line_height)),
				psy_ui_colour_make(0x009B7800));
		}
		psy_ui_textout(g, cp, itemtext, psy_strlen(itemtext));
		cp.y += line_height;
	}
}

void psy_ui_listboxclient_on_preferred_size(psy_ui_ListBoxClient* self,
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

void psy_ui_listboxclient_on_mouse_down(psy_ui_ListBoxClient* self,
	psy_ui_MouseEvent* ev)
{
	const psy_ui_TextMetric* tm;
	intptr_t index;
	double line_height;

	tm = psy_ui_component_textmetric(&self->component);
	line_height = floor(tm->tmHeight * 1.2);	
	index = (intptr_t)(psy_ui_mouseevent_pt(ev).y / line_height);	
	if (index < (intptr_t)psy_table_size(&self->items)) {
		self->selindex = index;		
		psy_ui_component_invalidate(&self->component);
		psy_signal_emit(&self->signal_selchanged, self, 0);
	}
}

void psy_ui_listboxclient_clear(psy_ui_ListBoxClient* self)
{
	self->selindex = -1;
	psy_table_dispose_all(&self->items, NULL);
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
	return psy_table_size(&self->items) - 1;
}

void psy_ui_listboxclient_setcursel(psy_ui_ListBoxClient* self, intptr_t index)
{
	self->selindex = index;
	psy_ui_component_invalidate(&self->component);
}

intptr_t psy_ui_listboxclient_cursel(const psy_ui_ListBoxClient* self)
{
	return self->selindex;
}

void psy_ui_listboxclient_setcharnumber(psy_ui_ListBoxClient* self, double num)
{
	self->charnumber = num;
}

intptr_t  psy_ui_listboxclient_count(const psy_ui_ListBoxClient* self)
{
	return psy_table_size(&self->items);
}

/* psy_ui_ListBox*/
static void psy_ui_listbox_on_destroyed(psy_ui_ListBox*);
static void psy_ui_listbox_onpreferredsize(psy_ui_ListBox*,
	psy_ui_Size* limit, psy_ui_Size* rv);
static void psy_ui_listbox_onselchanged(psy_ui_ListBox*,
	psy_ui_ListBoxClient* sender);

/* vtable */
static psy_ui_ComponentVtable psy_ui_listbox_vtable;
static bool psy_ui_listbox_vtable_initialized = FALSE;

static void psy_ui_listbox_vtable_init(psy_ui_ListBox* self)
{
	if (!psy_ui_listbox_vtable_initialized) {
		psy_ui_listbox_vtable = *(self->component.vtable);
		psy_ui_listbox_vtable.on_destroyed =
			(psy_ui_fp_component)
			psy_ui_listbox_on_destroyed;
		psy_ui_listbox_vtable.onpreferredsize =
			(psy_ui_fp_component_on_preferred_size)
			psy_ui_listbox_onpreferredsize;		
		psy_ui_listbox_vtable_initialized = TRUE;
	}
	self->component.vtable = &psy_ui_listbox_vtable;
}

/* implementation */
void psy_ui_listbox_init(psy_ui_ListBox* self, psy_ui_Component* parent)
{  
	psy_ui_component_init(&self->component, parent, NULL);	
	psy_ui_listbox_vtable_init(self);
	psy_ui_component_set_style_type(psy_ui_listbox_base(self),
		psy_ui_STYLE_LISTBOX);
	psy_signal_init(&self->signal_selchanged);
	self->charnumber = 0.0;	
	psy_ui_listboxclient_init(&self->client, &self->component);
	psy_ui_scroller_init(&self->scroller, &self->component, NULL, NULL);
	psy_ui_scroller_set_client(&self->scroller, &self->client.component);
	psy_ui_component_set_align(&self->scroller.component, psy_ui_ALIGN_CLIENT);
	psy_signal_connect(&self->client.signal_selchanged, self,
		psy_ui_listbox_onselchanged);
}

void psy_ui_listbox_init_multiselect(psy_ui_ListBox* self, psy_ui_Component*
	parent)
{  
	psy_ui_listbox_init(self, parent);
}

void psy_ui_listbox_on_destroyed(psy_ui_ListBox* self)
{
	psy_signal_dispose(&self->signal_selchanged);
}

intptr_t psy_ui_listbox_add_text(psy_ui_ListBox* self, const char* text)
{	
	return psy_ui_listboxclient_addtext(&self->client, text);
}

void psy_ui_listbox_settext(psy_ui_ListBox* self, const char* text,
	intptr_t index)
{	
	if (index >= psy_ui_listbox_count(self)) {
		intptr_t i;

		for (i = psy_ui_listbox_count(self); i <= index; ++i) {
			psy_ui_listbox_add_text(self, "");
		}
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

intptr_t psy_ui_listbox_cursel(const psy_ui_ListBox* self)
{		
	return psy_ui_listboxclient_cursel(&self->client);
}

void psy_ui_listbox_addsel(psy_ui_ListBox* self, intptr_t index)
{		
	
}

void psy_ui_listbox_selitems(psy_ui_ListBox* self, intptr_t* items,
	intptr_t maxitems)
{		
	
}

intptr_t psy_ui_listbox_selcount(psy_ui_ListBox* self)
{		
	return 0;
}

intptr_t psy_ui_listbox_count(const psy_ui_ListBox* self)
{	
	return psy_ui_listboxclient_count(&self->client);
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

void psy_ui_listbox_onselchanged(psy_ui_ListBox* self,
	psy_ui_ListBoxClient* sender)
{
	intptr_t sel;

	sel = psy_ui_listbox_cursel(self);	
	psy_signal_emit(&self->signal_selchanged, self, 1, sel);
}

void psy_ui_listbox_text(psy_ui_ListBox* self, char* text,
	uintptr_t index)
{
	char* itemtext;

	itemtext = (char*)psy_table_at(&self->client.items, index);
	if (itemtext) {
		psy_snprintf(text, 256, "%s", itemtext);
	} else {
		psy_snprintf(text, 256, "%s", "");
	}
}

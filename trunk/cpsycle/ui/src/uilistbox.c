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


/* ListBoxPane */

/* prototypes*/
static void psy_ui_listboxpane_on_destroyed(psy_ui_ListBoxPane*);
static void psy_ui_listboxpane_on_draw(psy_ui_ListBoxPane*,
	psy_ui_Graphics*);
static void psy_ui_listboxpane_on_preferred_size(psy_ui_ListBoxPane*,
	psy_ui_Size* limit, psy_ui_Size* rv);
static void psy_ui_listboxpane_on_mouse_down(psy_ui_ListBoxPane*,
	psy_ui_MouseEvent*);
static void psy_ui_listboxpane_on_size(psy_ui_ListBoxPane*);
static intptr_t psy_ui_listboxpane_count(const psy_ui_ListBoxPane*);

/* vtable */
static psy_ui_ComponentVtable psy_ui_listboxpane_vtable;
static bool psy_ui_listboxpane_vtable_initialized = FALSE;

static void psy_ui_listboxpane_vtable_init(psy_ui_ListBoxPane* self)
{
	if (!psy_ui_listboxpane_vtable_initialized) {
		psy_ui_listboxpane_vtable = *(self->component.vtable);
		psy_ui_listboxpane_vtable.on_destroyed =
			(psy_ui_fp_component)
			psy_ui_listboxpane_on_destroyed;
		psy_ui_listboxpane_vtable.ondraw =
			(psy_ui_fp_component_ondraw)
			psy_ui_listboxpane_on_draw;
		psy_ui_listboxpane_vtable.onpreferredsize =
			(psy_ui_fp_component_on_preferred_size)
			psy_ui_listboxpane_on_preferred_size;
		psy_ui_listboxpane_vtable.on_mouse_down =
			(psy_ui_fp_component_on_mouse_event)
			psy_ui_listboxpane_on_mouse_down;
		psy_ui_listboxpane_vtable.onsize =
			(psy_ui_fp_component)
			psy_ui_listboxpane_on_size;
		psy_ui_listboxpane_vtable_initialized = TRUE;
	}
	psy_ui_component_set_vtable(&self->component, &psy_ui_listboxpane_vtable);
}

/* implementation */
void psy_ui_listboxpane_init(psy_ui_ListBoxPane* self, psy_ui_Component*
	parent)
{
	psy_ui_component_init(&self->component, parent, NULL);
	psy_ui_listboxpane_vtable_init(self);	
	psy_table_init(&self->items);
	self->selindex = -1;
	self->charnumber = 0;
	psy_signal_init(&self->signal_selchanged);
	psy_ui_component_set_wheel_scroll(&self->component, 4);
	psy_ui_component_set_overflow(&self->component, psy_ui_OVERFLOW_VSCROLL);
}

void psy_ui_listboxpane_on_destroyed(psy_ui_ListBoxPane* self)
{
	psy_signal_dispose(&self->signal_selchanged);
	psy_table_dispose_all(&self->items, NULL);
}

void psy_ui_listboxpane_on_size(psy_ui_ListBoxPane* self)
{
	const psy_ui_TextMetric* tm;
	int line_height;

	tm = psy_ui_component_textmetric(&self->component);
	line_height = (int)(tm->tmHeight * 1.2);
	psy_ui_component_set_scroll_step_height(&self->component,
		psy_ui_value_make_px(line_height));
}

void psy_ui_listboxpane_on_draw(psy_ui_ListBoxPane* self,
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

void psy_ui_listboxpane_on_preferred_size(psy_ui_ListBoxPane* self,
	psy_ui_Size* limit, psy_ui_Size* rv)
{
	const psy_ui_TextMetric* tm;

	tm = psy_ui_component_textmetric(&self->component);
	rv->width = (self->charnumber == 0)
		? psy_ui_value_make_px(tm->tmAveCharWidth * 40)
		: psy_ui_value_make_px(tm->tmAveCharWidth * self->charnumber);
	rv->height = psy_ui_value_make_px((int)(tm->tmHeight * 1.2) *
		(double)psy_ui_listboxpane_count(self));
}

void psy_ui_listboxpane_on_mouse_down(psy_ui_ListBoxPane* self,
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

void psy_ui_listboxpane_clear(psy_ui_ListBoxPane* self)
{
	self->selindex = -1;
	psy_table_dispose_all(&self->items, NULL);
	psy_table_init(&self->items);
	psy_ui_component_updateoverflow(&self->component);
	psy_ui_component_invalidate(&self->component);
}

intptr_t psy_ui_listboxpane_add_text(psy_ui_ListBoxPane* self,
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

void psy_ui_listboxpane_set_cur_sel(psy_ui_ListBoxPane* self, intptr_t index)
{
	self->selindex = index;
	psy_ui_component_invalidate(&self->component);
}

intptr_t psy_ui_listboxpane_cur_sel(const psy_ui_ListBoxPane* self)
{
	return self->selindex;
}

void psy_ui_listboxpane_set_char_number(psy_ui_ListBoxPane* self, double num)
{
	self->charnumber = num;
}

intptr_t  psy_ui_listboxpane_count(const psy_ui_ListBoxPane* self)
{
	return psy_table_size(&self->items);
}

/* psy_ui_ListBox*/
static void psy_ui_listbox_on_destroyed(psy_ui_ListBox*);
static void psy_ui_listbox_on_preferred_size(psy_ui_ListBox*,
	psy_ui_Size* limit, psy_ui_Size* rv);
static void psy_ui_listbox_on_sel_changed(psy_ui_ListBox*,
	psy_ui_ListBoxPane* sender);

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
			psy_ui_listbox_on_preferred_size;		
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
	psy_ui_listboxpane_init(&self->pane, &self->component);
	psy_ui_scroller_init(&self->scroller, &self->component, NULL, NULL);
	psy_ui_scroller_set_client(&self->scroller, &self->pane.component);
	psy_ui_component_set_align(&self->scroller.component, psy_ui_ALIGN_CLIENT);
	psy_signal_connect(&self->pane.signal_selchanged, self,
		psy_ui_listbox_on_sel_changed);
}

void psy_ui_listbox_init_multi_select(psy_ui_ListBox* self, psy_ui_Component*
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
	return psy_ui_listboxpane_add_text(&self->pane, text);
}

void psy_ui_listbox_set_text(psy_ui_ListBox* self, const char* text,
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
	psy_ui_listboxpane_clear(&self->pane);
}

void psy_ui_listbox_set_cur_sel(psy_ui_ListBox* self, intptr_t index)
{	
	psy_ui_listboxpane_set_cur_sel(&self->pane, index);
}

intptr_t psy_ui_listbox_cur_sel(const psy_ui_ListBox* self)
{		
	return psy_ui_listboxpane_cur_sel(&self->pane);
}

void psy_ui_listbox_add_sel(psy_ui_ListBox* self, intptr_t index)
{		
	
}

void psy_ui_listbox_sel_items(psy_ui_ListBox* self, intptr_t* items,
	intptr_t maxitems)
{		
	
}

intptr_t psy_ui_listbox_sel_count(psy_ui_ListBox* self)
{		
	return 0;
}

intptr_t psy_ui_listbox_count(const psy_ui_ListBox* self)
{	
	return psy_ui_listboxpane_count(&self->pane);
}

void psy_ui_listbox_set_char_number(psy_ui_ListBox* self, double num)
{
	self->charnumber = num;
}

void psy_ui_listbox_on_preferred_size(psy_ui_ListBox* self,
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

void psy_ui_listbox_on_sel_changed(psy_ui_ListBox* self,
	psy_ui_ListBoxPane* sender)
{
	intptr_t sel;

	sel = psy_ui_listbox_cur_sel(self);	
	psy_signal_emit(&self->signal_selchanged, self, 1, sel);
}

void psy_ui_listbox_text(psy_ui_ListBox* self, char* text,
	uintptr_t index)
{
	char* itemtext;

	itemtext = (char*)psy_table_at(&self->pane.items, index);
	if (itemtext) {
		psy_snprintf(text, 256, "%s", itemtext);
	} else {
		psy_snprintf(text, 256, "%s", "");
	}
}

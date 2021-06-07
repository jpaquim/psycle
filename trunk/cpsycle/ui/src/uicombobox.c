/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
**  copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "uicombobox.h"
/* local */
#include "uiapp.h"
#include "uiicondraw.h"
#include "uiimpfactory.h"
/* platform */
#include "../../detail/portable.h"

/* prototypes */
static void onpreferredsize(psy_ui_ComboBox*, const psy_ui_Size* limit,
	psy_ui_Size* rv);
static void ondestroy(psy_ui_ComboBox*, psy_ui_Component* sender);
static void ondraw(psy_ui_ComboBox*, psy_ui_Graphics*);
static void onmousedown(psy_ui_ComboBox*, psy_ui_MouseEvent*);
static void onmousemove(psy_ui_ComboBox*, psy_ui_MouseEvent*);
static void onmouseenter(psy_ui_ComboBox*);
static void onmouseleave(psy_ui_ComboBox*);
static bool haspreventry(const psy_ui_ComboBox*);
static bool hasnextentry(const psy_ui_ComboBox*);
static void onselchange(psy_ui_ComboBox*, psy_ui_Component* sender, intptr_t index);
/* vtable */
static psy_ui_ComponentVtable vtable;
static bool vtable_initialized = FALSE;

static void vtable_init(psy_ui_ComboBox* self)
{
	if (!vtable_initialized) {
		vtable = *(self->component.vtable);
		vtable.ondraw =
			(psy_ui_fp_component_ondraw)
			ondraw;
		vtable.onpreferredsize =
			(psy_ui_fp_component_onpreferredsize)
			onpreferredsize;
		vtable.onmousedown =
			(psy_ui_fp_component_onmouseevent)
			onmousedown;
		vtable.onmousemove =
			(psy_ui_fp_component_onmouseevent)
			onmousemove;
		vtable.onmouseenter =
			(psy_ui_fp_component_onmouseenter)
			onmouseenter;
		vtable.onmouseleave =
			(psy_ui_fp_component_onmouseleave)
			onmouseleave;		
		vtable_initialized = TRUE;
	}
	self->component.vtable = &vtable;
}
/* implementation */
void psy_ui_combobox_init(psy_ui_ComboBox* self, psy_ui_Component* parent,
	psy_ui_Component* view)
{
	assert(self);

	psy_ui_component_init(&self->component, parent, view);
/*	psy_ui_component_setstyletypes(&self->component,
		psy_ui_STYLE_COMBOBOX,
		psy_ui_STYLE_COMBOBOX_HOVER,
		psy_ui_STYLE_COMBOBOX_SELECT,
		psy_ui_STYLE_LABEL_DISABLED); */
	psy_ui_component_doublebuffer(&self->component);	
	psy_ui_dropdownbox_init(&self->dropdown, &self->component);
	psy_ui_listbox_init(&self->listbox, &self->dropdown.component);
	psy_signal_connect(&self->listbox.signal_selchanged, self,
		onselchange);
	psy_ui_component_setalign(&self->listbox.component,
		psy_ui_ALIGN_CLIENT);
	psy_signal_connect(&self->component.signal_destroy, self, ondestroy);
	psy_signal_init(&self->signal_selchanged);
	vtable_init(self);
	self->charnumber = 0.0;
	self->hover = psy_ui_COMBOBOXHOVER_NONE;
	psy_table_init(&self->itemdata);
}

void ondestroy(psy_ui_ComboBox* self, psy_ui_Component* sender)
{
	assert(self);

	psy_table_dispose(&self->itemdata);
	psy_signal_dispose(&self->signal_selchanged);
}

psy_ui_ComboBox* psy_ui_combobox_alloc(void)
{
	return (psy_ui_ComboBox*)malloc(sizeof(psy_ui_ComboBox));
}

psy_ui_ComboBox* psy_ui_combobox_allocinit(psy_ui_Component* parent,
	psy_ui_Component* view)
{
	psy_ui_ComboBox* rv;

	rv = psy_ui_combobox_alloc();
	if (rv) {
		psy_ui_combobox_init(rv, parent, view);
		psy_ui_component_deallocateafterdestroyed(&rv->component);
	}
	return rv;
}


intptr_t psy_ui_combobox_addtext(psy_ui_ComboBox* self, const char* text)
{
	assert(self);

	return psy_ui_listbox_addtext(&self->listbox, text);
}

void psy_ui_combobox_settext(psy_ui_ComboBox* self, const char* text,
	intptr_t index)
{
	assert(self);

	psy_ui_listbox_settext(&self->listbox, text, index);	
}

void psy_ui_combobox_text(psy_ui_ComboBox* self, char* text)
{
	psy_ui_listbox_text(&self->listbox, text,
		psy_ui_listbox_cursel(&self->listbox));
}

intptr_t psy_ui_combobox_count(const psy_ui_ComboBox* self)
{
	return psy_ui_listbox_count(&self->listbox);
}

void psy_ui_combobox_clear(psy_ui_ComboBox* self)
{
	assert(self);

	psy_ui_listbox_clear(&self->listbox);
	psy_table_clear(&self->itemdata);
	psy_ui_component_invalidate(&self->component);
}

void psy_ui_combobox_setcursel(psy_ui_ComboBox* self, intptr_t index)
{
	assert(self);

	psy_ui_listbox_setcursel(&self->listbox, index);
	psy_ui_component_invalidate(&self->component);
}

intptr_t psy_ui_combobox_cursel(const psy_ui_ComboBox* self)
{
	assert(self);

	return psy_ui_listbox_cursel(&self->listbox);
}

void psy_ui_combobox_setcharnumber(psy_ui_ComboBox* self, double number)
{
	assert(self);

	self->charnumber = number;
}

void psy_ui_combobox_setitemdata(psy_ui_ComboBox* self, uintptr_t index,
	intptr_t data)
{
	assert(self);

	psy_table_insert(&self->itemdata, index, (void*)data);
}

intptr_t psy_ui_combobox_itemdata(psy_ui_ComboBox* self, uintptr_t index)
{
	assert(self);

	if (psy_table_exists(&self->itemdata, index)) {
		return (intptr_t)psy_table_at(&self->itemdata, index);
	}
	return -1;
}

void onpreferredsize(psy_ui_ComboBox* self, const psy_ui_Size* limit,
	psy_ui_Size* rv)
{
	assert(self);

	if (self->charnumber == 0) {
		rv->width = psy_ui_value_make_ew(9.0);
	} else {
		const psy_ui_TextMetric* tm;

		tm = psy_ui_component_textmetric(&self->component);
		rv->width = psy_ui_value_make_px(
			self->charnumber * tm->tmAveCharWidth + 40);
	}
	rv->height = psy_ui_value_make_eh(1.0);
}

void ondraw(psy_ui_ComboBox* self, psy_ui_Graphics* g)
{
	psy_ui_RealSize size;
	intptr_t sel;
	double varrowcenter;
	psy_ui_IconDraw icondraw;

	assert(self);

	size = psy_ui_component_scrollsize_px(&self->component);
	varrowcenter = (size.height - 10) / 2;
	sel = psy_ui_combobox_cursel(self);
	if (sel != -1) {
		char text[512];

		text[0] = '\0';

		psy_ui_combobox_text(self, text);
		if (psy_strlen(text)) {
			const psy_ui_TextMetric* tm;
			double vcenter;
			psy_ui_RealRectangle r;

			tm = psy_ui_component_textmetric(&self->component);
			vcenter = (size.height - tm->tmHeight) / 2;
			psy_ui_setrectangle(&r, 0, 0, size.width - 42, size.height);
			psy_ui_textoutrectangle(g, psy_ui_realpoint_make(0, vcenter),
				psy_ui_ETO_CLIPPED, r, text, strlen(text));
		}
	}
	psy_ui_icondraw_init(&icondraw, psy_ui_ICON_DOWN);
	psy_ui_icondraw_draw(&icondraw, g,
		psy_ui_realpoint_make(size.width - 10, 4 + varrowcenter),
		psy_ui_component_colour(&self->component));
	if (hasnextentry(self)) {
		psy_ui_icondraw_init(&icondraw, psy_ui_ICON_MORE);
		psy_ui_icondraw_draw(&icondraw, g,
			psy_ui_realpoint_make(size.width - 25, 2 + varrowcenter),
			psy_ui_component_colour(&self->component));
	}
	if (haspreventry(self)) {
		psy_ui_icondraw_init(&icondraw, psy_ui_ICON_LESS);
		psy_ui_icondraw_draw(&icondraw, g,
			psy_ui_realpoint_make(size.width - 40, 2 + varrowcenter),
			psy_ui_component_colour(&self->component));
	}
}

void onmousedown(psy_ui_ComboBox* self, psy_ui_MouseEvent* ev)
{
	psy_ui_RealSize size;

	assert(self);

	size = psy_ui_component_scrollsize_px(&self->component);
	if (ev->pt.x >= (size.width - 40) && (ev->pt.x < size.width - 25)) {
		intptr_t index;

		index = psy_ui_combobox_cursel(self);
		if (index > 0) {
			psy_ui_combobox_setcursel(self, index - 1);
			psy_signal_emit(&self->signal_selchanged, self, 1, index - 1);
		}
	} else if (ev->pt.x >= (size.width - 25) && ev->pt.x < (size.width - 10)) {
		intptr_t count;
		intptr_t index;

		index = psy_ui_combobox_cursel(self);
		count = psy_ui_combobox_count(self);
		if (index < count - 1) {
			psy_ui_combobox_setcursel(self, index + 1);
			psy_signal_emit(&self->signal_selchanged, self, 1, index + 1);
		}
	} else {
		psy_ui_dropdownbox_show(&self->dropdown, &self->component);
	}
}

void onmousemove(psy_ui_ComboBox* self, psy_ui_MouseEvent* ev)
{
	assert(self);

	if (self->hover != psy_ui_COMBOBOXHOVER_NONE) {
		psy_ui_ComboBoxHover hover;
		psy_ui_RealSize size;

		size = psy_ui_component_scrollsize_px(&self->component);
		hover = self->hover;
		if (ev->pt.x >= (size.width - 40) && ev->pt.x < (size.width - 25)) {
			intptr_t index = psy_ui_combobox_cursel(self);
			if (index > 0) {
				self->hover = psy_ui_COMBOBOXHOVER_LESS;
			}
		} else if (ev->pt.x >= (size.width - 25) &&
				(ev->pt.x < (size.width - 10))) {
			intptr_t count;
			intptr_t index;

			count = psy_ui_combobox_count(self);
			index = psy_ui_combobox_cursel(self);
			if (index < count - 1) {
				self->hover = psy_ui_COMBOBOXHOVER_MORE;
			}
		} else {
			self->hover = psy_ui_COMBOBOXHOVER_DOWN;
		}
		if (hover != self->hover) {			
			psy_ui_component_invalidate(&self->component);
		}
	}
}

void onmouseenter(psy_ui_ComboBox* self)
{
	assert(self);

	self->hover = psy_ui_COMBOBOXHOVER_DOWN;
	psy_ui_component_invalidate(&self->component);
}

void onmouseleave(psy_ui_ComboBox* self)
{
	assert(self);

	self->hover = psy_ui_COMBOBOXHOVER_NONE;	
	psy_ui_component_invalidate(&self->component);
}

bool haspreventry(const psy_ui_ComboBox* self)
{
	assert(self);

	return psy_ui_combobox_cursel(self) > 0;
}

bool hasnextentry(const psy_ui_ComboBox* self)
{
	intptr_t count;

	assert(self);

	count = psy_ui_combobox_count(self);
	return (psy_ui_combobox_cursel(self) < count - 1);
}

void onselchange(psy_ui_ComboBox* self, psy_ui_Component* sender, intptr_t index)
{
	psy_ui_dropdownbox_hide(&self->dropdown);
	psy_ui_component_invalidate(&self->component);
	psy_signal_emit(&self->signal_selchanged, self, 1, index);
}

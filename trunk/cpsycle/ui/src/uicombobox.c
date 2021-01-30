// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "uiapp.h"
#include "uicombobox.h"
#include "uiimpfactory.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

static void onpreferredsize(psy_ui_ComboBox*, psy_ui_Size* limit, psy_ui_Size* rv);
static void ondestroy(psy_ui_ComboBox*, psy_ui_Component* sender);
static void onownerdraw(psy_ui_ComboBox*, psy_ui_Graphics*);
static void onmousedown(psy_ui_ComboBox*, psy_ui_MouseEvent*);
static void onmousemove(psy_ui_ComboBox*, psy_ui_MouseEvent*);
static void onmouseenter(psy_ui_ComboBox*);
static void onmouseleave(psy_ui_ComboBox*);
static bool haspreventry(psy_ui_ComboBox*);
static bool hasnextentry(psy_ui_ComboBox*);

static psy_ui_ComponentVtable vtable;
static int vtable_initialized = 0;

static void vtable_init(psy_ui_ComboBox* self)
{
	if (!vtable_initialized) {
		vtable = *(self->component.vtable);
		vtable.ondraw = (psy_ui_fp_component_ondraw)onownerdraw;
		vtable.onpreferredsize = (psy_ui_fp_component_onpreferredsize)onpreferredsize;
		vtable.onmousedown = (psy_ui_fp_component_onmousedown)onmousedown;
		vtable.onmousemove = (psy_ui_fp_component_onmousemove)onmousemove;
		vtable.onmouseenter = (psy_ui_fp_component_onmouseenter)onmouseenter;
		vtable.onmouseleave = (psy_ui_fp_component_onmouseleave)onmouseleave;
		vtable_initialized = 1;
	}
}

void psy_ui_combobox_init(psy_ui_ComboBox* self, psy_ui_Component* parent)
{
	self->imp = psy_ui_impfactory_allocinit_comboboximp(
		psy_ui_app_impfactory(psy_ui_app()),
		&self->component, parent);
	psy_ui_component_init_imp(psy_psy_ui_combobox_base(self), parent,
		&self->imp->component_imp);
	psy_signal_connect(&self->component.signal_destroy, self, ondestroy);
	psy_signal_init(&self->signal_selchanged);
	vtable_init(self);
	self->component.vtable = &vtable;
	self->charnumber = 0;
	self->hover = 0;
	psy_table_init(&self->itemdata);
}

void ondestroy(psy_ui_ComboBox* self, psy_ui_Component* sender)
{
	psy_table_dispose(&self->itemdata);
	psy_signal_dispose(&self->signal_selchanged);
}

intptr_t psy_ui_combobox_addtext(psy_ui_ComboBox* self, const char* text)
{
	return self->imp->vtable->dev_addtext(self->imp, text);
}

void psy_ui_combobox_settext(psy_ui_ComboBox* self, const char* text, intptr_t index)
{
	self->imp->vtable->dev_settext(self->imp, text, index);
}

void psy_ui_combobox_clear(psy_ui_ComboBox* self)
{
	self->imp->vtable->dev_clear(self->imp);
}

void psy_ui_combobox_setcursel(psy_ui_ComboBox* self, intptr_t index)
{
	self->imp->vtable->dev_setcursel(self->imp, index);
}

intptr_t psy_ui_combobox_cursel(psy_ui_ComboBox* self)
{
	return self->imp->vtable->dev_cursel(self->imp);
}

void psy_ui_combobox_setcharnumber(psy_ui_ComboBox* self, int number)
{
	self->charnumber = number;
}

void psy_ui_combobox_setitemdata(psy_ui_ComboBox* self, uintptr_t index, intptr_t data)
{
	psy_table_insert(&self->itemdata, index, (void*)data);
}

intptr_t psy_ui_combobox_itemdata(psy_ui_ComboBox* self, uintptr_t index)
{
	if (psy_table_exists(&self->itemdata, index)) {
		return (intptr_t)psy_table_at(&self->itemdata, index);
	}
	return -1;
}

void onpreferredsize(psy_ui_ComboBox* self, psy_ui_Size* limit,
	psy_ui_Size* rv)
{
	if (rv) {		
		if (self->charnumber == 0) {
			rv->width = psy_ui_value_makeew(9);
		} else {
			const psy_ui_TextMetric* tm;

			tm = psy_ui_component_textmetric(&self->component);
			rv->width = psy_ui_value_makepx(
				self->charnumber * tm->tmAveCharWidth + 40);
		}
		rv->height = psy_ui_value_makeeh(1);
	}
}

void onownerdraw(psy_ui_ComboBox* self, psy_ui_Graphics* g)
{
	psy_ui_Size size;
	psy_ui_RealRectangle r;
	psy_ui_RealPoint arrow_down[4];
	psy_ui_RealPoint arrow_left[4];
	psy_ui_RealPoint arrow_right[4];
	double ax;
	double ay;
	intptr_t sel;
	const psy_ui_TextMetric* tm;
	double vcenter;
	double varrowcenter;
	uint32_t arrowcolour;
	uint32_t arrowhighlightcolour;

	arrowcolour = psy_ui_style(psy_ui_STYLE_BUTTON)->colour.value;
	arrowhighlightcolour = psy_ui_style(psy_ui_STYLE_BUTTON)->colour.value;
	size = psy_ui_component_size(&self->component);
	tm = psy_ui_component_textmetric(&self->component);
	psy_ui_setrectangle(&r, 0, 0, 
		psy_ui_value_px(&size.width, tm) - 42,
		psy_ui_value_px(&size.height, tm));	
	vcenter = (psy_ui_value_px(&size.height, tm) - tm->tmHeight) / 2;
	varrowcenter = (psy_ui_value_px(&size.height, tm) - 10) / 2;
	sel = psy_ui_combobox_cursel(self);	
	if (sel != -1) {
		char text[512];

		text[0] = '\0';
		self->imp->vtable->dev_text(self->imp, text);
		if (strlen(text)) {		
			psy_ui_setbackgroundmode(g, psy_ui_TRANSPARENT);
			if (self->hover) {
				psy_ui_settextcolour(g, 
					psy_ui_style(psy_ui_STYLE_BUTTON_HOVER)->colour);
			} else {
				psy_ui_settextcolour(g,
					psy_ui_style(psy_ui_STYLE_BUTTON)->colour);
			}
			psy_ui_textoutrectangle(g, psy_ui_realpoint_make(0, vcenter),
				psy_ui_ETO_CLIPPED, r, text, strlen(text));			
		}
	}
	ax = psy_ui_value_px(&size.width, tm) - 10;
	ay = 4 + varrowcenter;

	arrow_down[0].x = 0 + ax;
	arrow_down[0].y = 0 + ay;
	arrow_down[1].x = 8 + ax;
	arrow_down[1].y = 0 + ay;
	arrow_down[2].x = 4 + ax;
	arrow_down[2].y = 4 + ay;
	arrow_down[3] = arrow_down[0];
	if (self->hover == 1) {
		psy_ui_drawsolidpolygon(g, arrow_down, 4, arrowhighlightcolour,
			arrowhighlightcolour);
	} else {
		psy_ui_drawsolidpolygon(g, arrow_down, 4, arrowcolour, arrowcolour);
	}
	if (hasnextentry(self)) {
		ax = psy_ui_value_px(&size.width, tm) - 25;
		ay = 2 + varrowcenter;

		arrow_right[0].x = 0 + ax;
		arrow_right[0].y = 0 + ay;
		arrow_right[1].x = 0 + ax;
		arrow_right[1].y = 8 + ay;
		arrow_right[2].x = 4 + ax;
		arrow_right[2].y = 4 + ay;
		arrow_right[3] = arrow_right[0];

		if (self->hover == 3) {
			psy_ui_drawsolidpolygon(g, arrow_right, 4, arrowhighlightcolour,
				arrowhighlightcolour);
		} else {
			psy_ui_drawsolidpolygon(g, arrow_right, 4, arrowcolour, arrowcolour);
		}
	}

	if (haspreventry(self)) {
		ax = psy_ui_value_px(&size.width, tm) - 40;
		ay = 2 + varrowcenter;

		arrow_left[0].x = 4 + ax;
		arrow_left[0].y = 0 + ay;
		arrow_left[1].x = 4 + ax;
		arrow_left[1].y = 8 + ay;
		arrow_left[2].x = 0 + ax;
		arrow_left[2].y = 4 + ay;
		arrow_left[3] = arrow_left[0];

		if (self->hover == 2) {
			psy_ui_drawsolidpolygon(g, arrow_left, 4, arrowhighlightcolour,
				arrowhighlightcolour);
		} else {
			psy_ui_drawsolidpolygon(g, arrow_left, 4, arrowcolour, arrowcolour);
		}
	}
}

void onmousedown(psy_ui_ComboBox* self, psy_ui_MouseEvent* ev)
{
	const psy_ui_TextMetric* tm;
	psy_ui_Size size = psy_ui_component_size(&self->component);
	
	tm = psy_ui_component_textmetric(&self->component);
	if (ev->pt.x >= psy_ui_value_px(&size.width, tm) - 40 && ev->pt.x < psy_ui_value_px(&size.width, tm) - 25) {
		intptr_t index;

		printf("combobox mousedown 1\n");
		index = psy_ui_combobox_cursel(self);
		if (index > 0) {
			psy_ui_combobox_setcursel(self, index - 1);
			psy_signal_emit(&self->signal_selchanged, self, 1, index - 1);
		}
	} else if (ev->pt.x >= psy_ui_value_px(&size.width, tm) - 25 && ev->pt.x < psy_ui_value_px(&size.width, tm) - 10) {
		intptr_t count;
		intptr_t index;
		
		printf("combobox mousedown 2\n");
		index = psy_ui_combobox_cursel(self);
		count = self->imp->vtable->dev_count(self->imp);
		if (index < count - 1) {
			psy_ui_combobox_setcursel(self, index + 1);
			psy_signal_emit(&self->signal_selchanged, self, 1, index + 1);
		}
	} else {
		printf("combobox mousedown 3\n");
		self->imp->vtable->dev_showdropdown(self->imp);
	}
}

void onmousemove(psy_ui_ComboBox* self, psy_ui_MouseEvent* ev)
{
	if (self->hover) {
		const psy_ui_TextMetric* tm;
		int hover = self->hover;
		psy_ui_Size size = psy_ui_component_size(&self->component);

		tm = psy_ui_component_textmetric(&self->component);
		if (ev->pt.x >= psy_ui_value_px(&size.width, tm) - 40 && ev->pt.x <
			psy_ui_value_px(&size.width, tm) - 25) {
			intptr_t index = psy_ui_combobox_cursel(self);
			if (index > 0) {
				self->hover = 2;
			}
		}
		else
			if (ev->pt.x >= psy_ui_value_px(&size.width, tm) - 25 && ev->pt.x <
					psy_ui_value_px(&size.width, tm) - 10) {
				intptr_t count;
				intptr_t index;
				count = self->imp->vtable->dev_count(self->imp); 
				index = psy_ui_combobox_cursel(self);
				if (index < count - 1) {
					self->hover = 3;
				}
			}
			else {
				self->hover = 1;
			}
		if (hover != self->hover) {
			psy_ui_component_invalidate(&self->component);
		}
	}
}

void onmouseenter(psy_ui_ComboBox* self)
{
	self->hover = 1;
	psy_ui_component_invalidate(&self->component);
}

void onmouseleave(psy_ui_ComboBox* self)
{
	self->hover = 0;
	psy_ui_component_invalidate(&self->component);
}

bool haspreventry(psy_ui_ComboBox* self)
{
	intptr_t index;

	index = psy_ui_combobox_cursel(self);
	return (index > 0);
}

bool hasnextentry(psy_ui_ComboBox* self)
{
	intptr_t count;
	intptr_t index;

	index = psy_ui_combobox_cursel(self);
	count = self->imp->vtable->dev_count(self->imp);
	return (index < count - 1);
}

// psy_ui_ComboBoxImp vtable
static int dev_addtext(psy_ui_ComboBoxImp* self, const char* text) { return -1; }
static void dev_settext(psy_ui_ComboBoxImp* self, const char* text, intptr_t index) { }
static void dev_text(psy_ui_ComboBoxImp* self, char* text) { }
static void dev_setstyle(psy_ui_ComboBoxImp* self, int style) { }
static void dev_clear(psy_ui_ComboBoxImp* self) { }
static void dev_setcursel(psy_ui_ComboBoxImp* self, intptr_t index) { }
static intptr_t dev_cursel(psy_ui_ComboBoxImp* self) { return -1; }
static void dev_selitems(psy_ui_ComboBoxImp* self, intptr_t* items, intptr_t maxitems) { }
static intptr_t dev_count(psy_ui_ComboBoxImp* self) { return 0; }
static intptr_t dev_selcount(psy_ui_ComboBoxImp* self) { return 0; }
static void dev_showdropdown(psy_ui_ComboBoxImp* self) { }

static psy_ui_ComboBoxImpVTable combobox_imp_vtable;
static int combobox_imp_vtable_initialized = 0;

static void combobox_imp_vtable_init(void)
{
	if (!combobox_imp_vtable_initialized) {
		combobox_imp_vtable.dev_addtext = dev_addtext;
		combobox_imp_vtable.dev_settext = dev_settext;
		combobox_imp_vtable.dev_text = dev_text;
		combobox_imp_vtable.dev_setstyle = dev_setstyle;
		combobox_imp_vtable.dev_clear = dev_clear;
		combobox_imp_vtable.dev_setcursel = dev_setcursel;
		combobox_imp_vtable.dev_cursel = dev_cursel;
		combobox_imp_vtable.dev_count = dev_count;
		combobox_imp_vtable.dev_selitems = dev_selitems;
		combobox_imp_vtable.dev_selcount = dev_selcount;
		combobox_imp_vtable.dev_showdropdown = dev_showdropdown;
		combobox_imp_vtable_initialized = 1;
	}
}

void psy_ui_comboboximp_init(psy_ui_ComboBoxImp* self)
{
	combobox_imp_vtable_init();
	self->vtable = &combobox_imp_vtable;
}

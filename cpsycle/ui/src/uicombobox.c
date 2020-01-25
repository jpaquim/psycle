// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "uicombobox.h"
#include "uiwincomponentimp.h"
#include <stdlib.h>

static void onpreferredsize(psy_ui_ComboBox*, psy_ui_Size* limit, psy_ui_Size* rv);
static void oncommand(psy_ui_ComboBox*, psy_ui_Component* sender, WPARAM wParam,
	LPARAM lParam);
static void ondestroy(psy_ui_ComboBox*, psy_ui_Component* sender);
static void psy_ui_combobox_create_system(psy_ui_ComboBox*,
	psy_ui_Component* parent);
static void psy_ui_combobox_create_ownerdrawn(psy_ui_ComboBox*,
	psy_ui_Component* parent);
static void onownerdraw(psy_ui_ComboBox*, psy_ui_Graphics*);
static void onmousedown(psy_ui_ComboBox*, psy_ui_MouseEvent*);
static void onmousemove(psy_ui_ComboBox*, psy_ui_MouseEvent*);
static void onmouseenter(psy_ui_ComboBox*);
static void onmouseleave(psy_ui_ComboBox*);

static psy_ui_win_ComponentImp* psy_ui_win_component_details(psy_ui_Component* self)
{
	return (psy_ui_win_ComponentImp*)self->imp;
}

static psy_ui_ComponentVtable vtable;
static int vtable_initialized = 0;

static void vtable_init(psy_ui_ComboBox* self)
{
	if (!vtable_initialized) {
		vtable = *(self->component.vtable);
		vtable.ondraw = (psy_ui_fp_ondraw) onownerdraw;
		vtable.onpreferredsize = (psy_ui_fp_onpreferredsize) onpreferredsize;
		vtable.onmousedown = (psy_ui_fp_onmousedown) onmousedown;
		vtable.onmousemove = (psy_ui_fp_onmousemove) onmousemove;
		vtable.onmouseenter = (psy_ui_fp_onmouseenter) onmouseenter;
		vtable.onmouseleave = (psy_ui_fp_onmouseleave) onmouseleave;
		vtable_initialized = 1;
	}
}

void psy_ui_combobox_init(psy_ui_ComboBox* self, psy_ui_Component* parent)
{  			
	psy_ui_combobox_create_ownerdrawn(self, parent);
	vtable_init(self);
	self->component.vtable = &vtable;
	self->charnumber = 0;
	self->hover = 0;
	psy_signal_init(&self->signal_selchanged);
	psy_signal_connect(&self->component.signal_destroy, self, ondestroy);	
}

static void psy_ui_combobox_create_system(psy_ui_ComboBox* self,
	psy_ui_Component* parent)
{	
	psy_ui_win32_component_init(&self->component, parent, TEXT("COMBOBOX"),
		0, 0, 100, 20,
		WS_CHILD | WS_VISIBLE | WS_VSCROLL | CBS_DROPDOWNLIST,
		1);
	psy_ui_component_resize(&self->component, 90, 200);
	psy_signal_connect(&self->component.signal_command, self, oncommand);	
	self->currcombo = &self->component;
}

void psy_ui_combobox_create_ownerdrawn(psy_ui_ComboBox* self, psy_ui_Component* parent)
{		
	psy_ui_component_init(&self->component, parent);
	psy_ui_win32_component_init(&self->combo, &self->component, TEXT("COMBOBOX"), 
		0, 0, 100, 200,
		WS_CHILD | WS_VISIBLE | WS_VSCROLL | CBS_DROPDOWNLIST,
		1);	
	psy_ui_component_hide(&self->combo);
	psy_signal_connect(&self->combo.signal_command, self, oncommand);	
	self->ownerdrawn = 1;	
	self->currcombo = &self->combo;	
}

void ondestroy(psy_ui_ComboBox* self, psy_ui_Component* sender)
{
	psy_signal_dispose(&self->signal_selchanged);	
}

int psy_ui_combobox_addstring(psy_ui_ComboBox* self, const char* text)
{
	LRESULT index;

	index = SendMessage((HWND)psy_ui_win_component_details(self->currcombo)->hwnd, CB_ADDSTRING, (WPARAM)0,
		(LPARAM)text);
	if (self->ownerdrawn) {
		psy_ui_component_invalidate(&self->component);
	}
	return (int)index;
}

void psy_ui_combobox_clear(psy_ui_ComboBox* self)
{
	SendMessage((HWND)psy_ui_win_component_details(self->currcombo)->hwnd, CB_RESETCONTENT, 0, (LPARAM)0);
	if (self->ownerdrawn) {
		psy_ui_component_invalidate(&self->component);
	}
}

void psy_ui_combobox_setcursel(psy_ui_ComboBox* self, intptr_t index)
{
	SendMessage((HWND)psy_ui_win_component_details(self->currcombo)->hwnd, CB_SETCURSEL, (WPARAM)index,
		(LPARAM)0);
	if (self->ownerdrawn) {
		psy_ui_component_invalidate(&self->component);
	}
}

intptr_t psy_ui_combobox_cursel(psy_ui_ComboBox* self)
{
	return SendMessage((HWND)psy_ui_win_component_details(self->currcombo)->hwnd, CB_GETCURSEL, (WPARAM)0,
		(LPARAM)0);	
}

void psy_ui_combobox_setcharnumber(psy_ui_ComboBox* self, int number)
{
	self->charnumber = number;
}

void onpreferredsize(psy_ui_ComboBox* self, psy_ui_Size* limit,
	psy_ui_Size* rv)
{
	if (rv) {
		psy_ui_TextMetric tm;

		tm = psy_ui_component_textmetric(&self->component);
		if (self->charnumber == 0) {
			rv->width = 90;
		} else {		
			rv->width = tm.tmAveCharWidth * self->charnumber + 40;
		}
		rv->height = tm.tmHeight;
	}
}

void oncommand(psy_ui_ComboBox* self, psy_ui_Component* sender, WPARAM wParam,
	LPARAM lParam) {
	switch(HIWORD(wParam))
    {
        case CBN_SELCHANGE :
        {
            if (self->signal_selchanged.slots) {
				intptr_t sel = SendMessage((HWND)psy_ui_win_component_details(self->currcombo)->hwnd,
					CB_GETCURSEL, (WPARAM)0, (LPARAM)0);
				psy_signal_emit(&self->signal_selchanged, self, 1, sel);
			}
			if (self->ownerdrawn) {
				psy_ui_component_invalidate(&self->component);
			}
        }
		break;
		default:
		break;
    }
}

void onownerdraw(psy_ui_ComboBox* self, psy_ui_Graphics* g)
{
	psy_ui_Size size;
	psy_ui_Rectangle r;
	psy_ui_Point arrow_down[4];
	psy_ui_Point arrow_left[4];
	psy_ui_Point arrow_right[4];
	int ax;
	int ay;
	intptr_t sel;
	psy_ui_TextMetric tm;
	int vcenter;
	int varrowcenter;
	unsigned int arrowcolor = 0x00777777;
	unsigned int arrowhighlightcolor = 0x00FFFFFF;

	size = psy_ui_component_size(&self->component);
	psy_ui_setrectangle(&r, 0, 0, size.width, size.height);

	tm = psy_ui_component_textmetric(&self->component);
	vcenter = (size.height - tm.tmHeight) / 2;
	varrowcenter = (size.height - 10) / 2;
	sel = SendMessage((HWND)psy_ui_win_component_details(&self->combo)->hwnd, CB_GETCURSEL, (WPARAM)0, (LPARAM)0);
	if (sel != CB_ERR) {
		intptr_t len;
		
		len = SendMessage((HWND)psy_ui_win_component_details(&self->combo)->hwnd, CB_GETLBTEXTLEN, (WPARAM)sel, 0);
		if (len > 0) {
			char* txt;

			txt = (char*)malloc(len + 1);			
			SendMessage((HWND)psy_ui_win_component_details(&self->combo)->hwnd, CB_GETLBTEXT, (WPARAM)sel,
				(LPARAM)txt);

			psy_ui_setbackgroundmode(g, psy_ui_TRANSPARENT);
			if (self->hover) {
				psy_ui_settextcolor(g, 0x00FFFFFF);
			} else {
				psy_ui_settextcolor(g, 0x00CACACA);
			}
			psy_ui_textoutrectangle(g, 0, vcenter, ETO_CLIPPED, r, txt, strlen(txt));
			free(txt);
		}
	}
	ax = size.width - 10;
	ay = 4 + varrowcenter;
	
	arrow_down[0].x = 0 + ax;
	arrow_down[0].y = 0 + ay;
	arrow_down[1].x = 8 + ax;
	arrow_down[1].y = 0 + ay;
	arrow_down[2].x = 4 + ax;
	arrow_down[2].y = 4 + ay;
	arrow_down[3] = arrow_down[0];

	if (self->hover == 1) {
		psy_ui_drawsolidpolygon(g, arrow_down, 4, arrowhighlightcolor,
			arrowhighlightcolor);
	} else {
		psy_ui_drawsolidpolygon(g, arrow_down, 4, arrowcolor, arrowcolor);
	}

	ax = size.width - 25;
	ay = 2 + varrowcenter;

	arrow_right[0].x = 0 + ax;
	arrow_right[0].y = 0 + ay;
	arrow_right[1].x = 0 + ax;
	arrow_right[1].y = 8 + ay;
	arrow_right[2].x = 4 + ax;
	arrow_right[2].y = 4 + ay;
	arrow_right[3] = arrow_right[0];

	if (self->hover == 3) {
		psy_ui_drawsolidpolygon(g, arrow_right, 4, arrowhighlightcolor,
			arrowhighlightcolor);
	} else {
		psy_ui_drawsolidpolygon(g, arrow_right, 4, arrowcolor, arrowcolor);
	}

	ax = size.width - 40;
	ay = 2 + varrowcenter;

	arrow_left[0].x = 4 + ax;
	arrow_left[0].y = 0 + ay;
	arrow_left[1].x = 4 + ax;
	arrow_left[1].y = 8 + ay;
	arrow_left[2].x = 0 + ax;
	arrow_left[2].y = 4 + ay;
	arrow_left[3] = arrow_left[0];

	if (self->hover == 2) {
		psy_ui_drawsolidpolygon(g, arrow_left, 4, arrowhighlightcolor,
			arrowhighlightcolor);
	} else {		
		psy_ui_drawsolidpolygon(g, arrow_left, 4, arrowcolor, arrowcolor);
	}
}

void onmousedown(psy_ui_ComboBox* self, psy_ui_MouseEvent* ev)
{
	psy_ui_Size size = psy_ui_component_size(&self->component);	

	if (ev->x >= size.width - 40 && ev->x < size.width - 25) {
		intptr_t index = psy_ui_combobox_cursel(self);
		if (index > 0) {
			psy_ui_combobox_setcursel(self,  index - 1);
			psy_signal_emit(&self->signal_selchanged, self, 1, index - 1);
		}
	} else
	if (ev->x >= size.width - 25 && ev->x < size.width - 10) {
		intptr_t count;
		intptr_t index;

		count = SendMessage((HWND)psy_ui_win_component_details(self->currcombo)->hwnd, CB_GETCOUNT, 0, (LPARAM)0);
		index = psy_ui_combobox_cursel(self);
		if (index < count - 1) {
			psy_ui_combobox_setcursel(self, index + 1);
			psy_signal_emit(&self->signal_selchanged, self, 1, index + 1);
		}
	} else {
		SetWindowPos((HWND)psy_ui_win_component_details(&self->combo)->hwnd, NULL, 
		   0,
		   0,
		   size.width, size.height, SWP_NOZORDER | SWP_NOMOVE);
		SendMessage((HWND)psy_ui_win_component_details(&self->combo)->hwnd, CB_SHOWDROPDOWN,
			(WPARAM)TRUE, (LPARAM)0);
	}
}

void onmousemove(psy_ui_ComboBox* self, psy_ui_MouseEvent* ev)
{
	if (self->hover) {
		int hover = self->hover;
		psy_ui_Size size = psy_ui_component_size(&self->component);	

		if (ev->x >= size.width - 40 && ev->x < size.width - 25) {
			intptr_t index = psy_ui_combobox_cursel(self);
			if (index > 0) {
				self->hover = 2;
			}
		} else
		if (ev->x >= size.width - 25 && ev->x < size.width - 10) {
			intptr_t count;
			intptr_t index;
			count = SendMessage((HWND)psy_ui_win_component_details(self->currcombo)->hwnd, CB_GETCOUNT, 0,
				(LPARAM)0);
			index = psy_ui_combobox_cursel(self);
			if (index < count - 1) {
				self->hover = 3;
			}
		} else {
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

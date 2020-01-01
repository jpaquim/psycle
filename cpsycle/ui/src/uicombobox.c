// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "uicombobox.h"

static void preferredsize(psy_ui_ComboBox*, ui_size* limit, ui_size* rv);
static void oncommand(psy_ui_ComboBox*, psy_ui_Component* sender, WPARAM wParam,
	LPARAM lParam);
static void ondestroy(psy_ui_ComboBox*, psy_ui_Component* sender);
static void ui_combobox_create_system(psy_ui_ComboBox*,
	psy_ui_Component* parent);
static void ui_combobox_create_ownerdrawn(psy_ui_ComboBox*,
	psy_ui_Component* parent);
static void onownerdraw(psy_ui_ComboBox*, psy_ui_Component* sender,
	psy_ui_Graphics*);
static void onmousedown(psy_ui_ComboBox*, psy_ui_Component* sender,
	psy_ui_MouseEvent* ev);
static void onmousemove(psy_ui_ComboBox*, psy_ui_Component* sender,
	psy_ui_MouseEvent* ev);
static void onmouseenter(psy_ui_ComboBox*, psy_ui_Component* sender);
static void onmouseleave(psy_ui_ComboBox*, psy_ui_Component* sender);

static psy_ui_ComponentVtable vtable;
static int vtable_initialized = 0;

static void vtable_init(psy_ui_ComboBox* self)
{
	if (!vtable_initialized) {
		vtable = *(self->component.vtable);
		vtable.preferredsize = (psy_ui_fp_preferredsize) preferredsize;
	}
}

void ui_combobox_init(psy_ui_ComboBox* self, psy_ui_Component* parent)
{  			
	ui_combobox_create_ownerdrawn(self, parent);
	vtable_init(self);
	self->component.vtable = &vtable;
	self->charnumber = 0;
	self->hover = 0;
	psy_signal_init(&self->signal_selchanged);
	psy_signal_connect(&self->component.signal_destroy, self, ondestroy);
	
	
}

void ui_combobox_create_system(psy_ui_ComboBox* self, psy_ui_Component* parent)
{	
	ui_win32_component_init(&self->component, parent, TEXT("COMBOBOX"), 
		0, 0, 100, 20,
		WS_CHILD | WS_VISIBLE | WS_VSCROLL | CBS_DROPDOWNLIST,
		1);
	ui_component_resize(&self->component, 90, 200);
	psy_signal_connect(&self->component.signal_command, self, oncommand);	
	self->currcombo = &self->component;
}

void ui_combobox_create_ownerdrawn(psy_ui_ComboBox* self, psy_ui_Component* parent)
{		
	ui_component_init(&self->component, parent);	
	psy_signal_connect(&self->component.signal_draw, self, onownerdraw);	
	psy_signal_connect(&self->component.signal_mousedown, self, onmousedown);
	psy_signal_connect(&self->component.signal_mousemove, self, onmousemove);
	psy_signal_connect(&self->component.signal_mouseenter, self, onmouseenter);
	psy_signal_connect(&self->component.signal_mouseleave, self, onmouseleave);		
	ui_win32_component_init(&self->combo, &self->component, TEXT("COMBOBOX"), 
		0, 0, 100, 200,
		WS_CHILD | WS_VISIBLE | WS_VSCROLL | CBS_DROPDOWNLIST,
		1);
	ui_component_hide(&self->combo);	
	psy_signal_connect(&self->combo.signal_command, self, oncommand);	
	self->ownerdrawn = 1;	
	self->currcombo = &self->combo;	
}

void ondestroy(psy_ui_ComboBox* self, psy_ui_Component* sender)
{
	psy_signal_dispose(&self->signal_selchanged);
	ui_component_dispose(&self->combo);
}

int ui_combobox_addstring(psy_ui_ComboBox* self, const char* text)
{
	LRESULT index;

	index = SendMessage((HWND)self->currcombo->hwnd, CB_ADDSTRING, (WPARAM)0,
		(LPARAM)text);
	if (self->ownerdrawn) {
		ui_component_invalidate(&self->component);
	}
	return (int)index;
}

void ui_combobox_clear(psy_ui_ComboBox* self)
{
	SendMessage((HWND)self->currcombo->hwnd, CB_RESETCONTENT, 0, (LPARAM)0);
	if (self->ownerdrawn) {
		ui_component_invalidate(&self->component);
	}
}

void ui_combobox_setcursel(psy_ui_ComboBox* self, intptr_t index)
{
	SendMessage((HWND)self->currcombo->hwnd, CB_SETCURSEL, (WPARAM)index,
		(LPARAM)0);
	if (self->ownerdrawn) {
		ui_component_invalidate(&self->component);
	}
}

intptr_t ui_combobox_cursel(psy_ui_ComboBox* self)
{
	return SendMessage((HWND)self->currcombo->hwnd, CB_GETCURSEL, (WPARAM)0,
		(LPARAM)0);	
}

void ui_combobox_setcharnumber(psy_ui_ComboBox* self, int number)
{
	self->charnumber = number;
}

void preferredsize(psy_ui_ComboBox* self, ui_size* limit, ui_size* rv)
{
	if (rv) {
		ui_textmetric tm;

		tm = ui_component_textmetric(&self->component);
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
				intptr_t sel = SendMessage((HWND)self->currcombo->hwnd,
					CB_GETCURSEL, (WPARAM)0, (LPARAM)0);
				psy_signal_emit(&self->signal_selchanged, self, 1, sel);
			}
			if (self->ownerdrawn) {
				ui_component_invalidate(&self->component);
			}
        }
		break;
		default:
		break;
    }
}

void onownerdraw(psy_ui_ComboBox* self, psy_ui_Component* sender,
	psy_ui_Graphics* g)
{
	ui_size size;
	ui_rectangle r;
	ui_point arrow_down[4];
	ui_point arrow_left[4];
	ui_point arrow_right[4];
	int ax;
	int ay;
	intptr_t sel;
	ui_textmetric tm;
	int vcenter;
	int varrowcenter;
	unsigned int arrowcolor = 0x00777777;
	unsigned int arrowhighlightcolor = 0x00FFFFFF;

	size = ui_component_size(&self->component);
	ui_setrectangle(&r, 0, 0, size.width, size.height);

	tm = ui_component_textmetric(&self->component);
	vcenter = (size.height - tm.tmHeight) / 2;
	varrowcenter = (size.height - 10) / 2;
	sel = SendMessage((HWND)self->combo.hwnd, CB_GETCURSEL, (WPARAM)0, (LPARAM)0);
	if (sel != CB_ERR) {
		intptr_t len;
		
		len = SendMessage((HWND)self->combo.hwnd, CB_GETLBTEXTLEN, (WPARAM)sel, 0);
		if (len > 0) {
			char* txt;

			txt = (char*)malloc(len + 1);			
			SendMessage((HWND)self->combo.hwnd, CB_GETLBTEXT, (WPARAM)sel,
				(LPARAM)txt);

			ui_setbackgroundmode(g, TRANSPARENT);
			if (self->hover) {
				ui_settextcolor(g, 0x00FFFFFF);
			} else {
				ui_settextcolor(g, 0x00CACACA);
			}
			ui_textoutrectangle(g, 0, vcenter, ETO_CLIPPED, r, txt, strlen(txt));
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
		ui_drawsolidpolygon(g, arrow_down, 4, arrowhighlightcolor,
			arrowhighlightcolor);
	} else {
		ui_drawsolidpolygon(g, arrow_down, 4, arrowcolor, arrowcolor);
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
		ui_drawsolidpolygon(g, arrow_right, 4, arrowhighlightcolor,
			arrowhighlightcolor);
	} else {
		ui_drawsolidpolygon(g, arrow_right, 4, arrowcolor, arrowcolor);
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
		ui_drawsolidpolygon(g, arrow_left, 4, arrowhighlightcolor,
			arrowhighlightcolor);
	} else {		
		ui_drawsolidpolygon(g, arrow_left, 4, arrowcolor, arrowcolor);
	}
}

void onmousedown(psy_ui_ComboBox* self, psy_ui_Component* sender,
	psy_ui_MouseEvent* ev)
{
	ui_size size = ui_component_size(sender);	

	if (ev->x >= size.width - 40 && ev->x < size.width - 25) {
		intptr_t index = ui_combobox_cursel(self);
		if (index > 0) {
			ui_combobox_setcursel(self,  index - 1);
			psy_signal_emit(&self->signal_selchanged, self, 1, index - 1);
		}
	} else
	if (ev->x >= size.width - 25 && ev->x < size.width - 10) {
		intptr_t count;
		intptr_t index;

		count = SendMessage((HWND)self->currcombo->hwnd, CB_GETCOUNT, 0, (LPARAM)0);
		index = ui_combobox_cursel(self);
		if (index < count - 1) {
			ui_combobox_setcursel(self, index + 1);
			psy_signal_emit(&self->signal_selchanged, self, 1, index + 1);
		}
	} else {
		SetWindowPos((HWND)self->combo.hwnd, NULL, 
		   0,
		   0,
		   size.width, size.height, SWP_NOZORDER | SWP_NOMOVE);
		SendMessage((HWND)self->combo.hwnd, CB_SHOWDROPDOWN,
			(WPARAM)TRUE, (LPARAM)0);
	}
}

void onmouseenter(psy_ui_ComboBox* self, psy_ui_Component* sender)
{
	self->hover = 1;
	ui_component_invalidate(&self->component);
}

void onmousemove(psy_ui_ComboBox* self, psy_ui_Component* sender,
	psy_ui_MouseEvent* ev)
{
	if (self->hover) {
		int hover = self->hover;
		ui_size size = ui_component_size(sender);	

		if (ev->x >= size.width - 40 && ev->x < size.width - 25) {
			intptr_t index = ui_combobox_cursel(self);
			if (index > 0) {
				self->hover = 2;
			}
		} else
		if (ev->x >= size.width - 25 && ev->x < size.width - 10) {
			intptr_t count;
			intptr_t index;
			count = SendMessage((HWND)self->currcombo->hwnd, CB_GETCOUNT, 0,
				(LPARAM)0);
			index = ui_combobox_cursel(self);
			if (index < count - 1) {
				self->hover = 3;
			}
		} else {
			self->hover = 1;
		}
		if (hover != self->hover) {
			ui_component_invalidate(&self->component);
		}
	}
}

void onmouseleave(psy_ui_ComboBox* self, psy_ui_Component* sender)
{		
	self->hover = 0;
	ui_component_invalidate(&self->component);
}


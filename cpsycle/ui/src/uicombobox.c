// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#include "uicombobox.h"

static void onpreferredsize(ui_combobox*, ui_component* sender, ui_size* limit,
	int* width, int* height);
static void oncommand(ui_combobox*, ui_component* sender, WPARAM wParam,
	LPARAM lParam);
static void OnDestroy(ui_combobox*, ui_component* sender);
static void ui_combobox_create_system(ui_combobox*, ui_component* parent);
static void ui_combobox_create_ownerdrawn(ui_combobox*, ui_component* parent);
static void onownerdraw(ui_combobox*, ui_component* sender, ui_graphics*);
static void onmousedown(ui_combobox*, ui_component* sender, int x, int y,
	int button);
static void onmousemove(ui_combobox*, ui_component* sender, int x, int y,
	int button);
static void onmouseenter(ui_combobox*, ui_component* sender);
static void onmouseleave(ui_combobox*, ui_component* sender);

void ui_combobox_init(ui_combobox* combobox, ui_component* parent)
{  
	combobox->hover = 0;
	ui_component_init_signals(&combobox->component);
	signal_init(&combobox->signal_selchanged);
	signal_connect(&combobox->component.signal_destroy, combobox, OnDestroy);
	ui_combobox_create_ownerdrawn(combobox, parent);
	ui_component_init_base(&combobox->component);
	if (combobox->ownerdrawn) {
		ui_component_setbackgroundmode(&combobox->component, BACKGROUND_SET);
	}
	signal_connect(&combobox->component.signal_preferredsize,
		combobox, onpreferredsize);
	combobox->charnumber = 0;
}

void ui_combobox_create_system(ui_combobox* self, ui_component* parent)
{	
	ui_win32_component_init(&self->component, parent, TEXT("COMBOBOX"), 
		0, 0, 100, 20,
		WS_CHILD | WS_VISIBLE | WS_VSCROLL | CBS_DROPDOWNLIST,
		1);
	ui_component_resize(&self->component, 90, 200);
	signal_connect(&self->component.signal_command, self, oncommand);	
	self->currcombo = &self->component;
}

void ui_combobox_create_ownerdrawn(ui_combobox* self, ui_component* parent)
{		
	ui_component_init(&self->component, parent);	
	signal_connect(&self->component.signal_draw, self, onownerdraw);	
	signal_connect(&self->component.signal_mousedown, self, onmousedown);
	signal_connect(&self->component.signal_mousemove, self, onmousemove);
	signal_connect(&self->component.signal_mouseenter, self, onmouseenter);
	signal_connect(&self->component.signal_mouseleave, self, onmouseleave);		
	ui_win32_component_init(&self->combo, &self->component, TEXT("COMBOBOX"), 
		0, 0, 100, 200,
		WS_CHILD | WS_VISIBLE | WS_VSCROLL | CBS_DROPDOWNLIST,
		1);
	ui_component_hide(&self->combo);	
	signal_connect(&self->combo.signal_command, self, oncommand);	
	self->ownerdrawn = 1;	
	self->currcombo = &self->combo;	
}

void OnDestroy(ui_combobox* self, ui_component* sender)
{
	signal_dispose(&self->signal_selchanged);	
}

int ui_combobox_addstring(ui_combobox* self, const char* text)
{
	int index;

	index = SendMessage(self->currcombo->hwnd, CB_ADDSTRING, 0, (LPARAM)text);
	if (self->ownerdrawn) {
		ui_invalidate(&self->component);
	}
	return index;
}

void ui_combobox_clear(ui_combobox* self)
{
	SendMessage(self->currcombo->hwnd, CB_RESETCONTENT, 0, (LPARAM)0);
	if (self->ownerdrawn) {
		ui_invalidate(&self->component);
	}
}

void ui_combobox_setcursel(ui_combobox* self, int index)
{
	SendMessage(self->currcombo->hwnd, CB_SETCURSEL, (WPARAM)index, (LPARAM)0);
	if (self->ownerdrawn) {
		ui_invalidate(&self->component);
	}
}

int ui_combobox_cursel(ui_combobox* self)
{
	return SendMessage(self->currcombo->hwnd, CB_GETCURSEL, (WPARAM)0,
		(LPARAM)0);	
}

void ui_combobox_setcharnumber(ui_combobox* self, int number)
{
	self->charnumber = number;
}

void onpreferredsize(ui_combobox* self, ui_component* sender, ui_size* limit,
	int* width, int* height)
{
	TEXTMETRIC tm;

	tm = ui_component_textmetric(&self->component);
	if (self->charnumber == 0) {
		*width = 90;
	} else {		
		*width = tm.tmAveCharWidth * self->charnumber + 40;
	}
	*height = tm.tmHeight;
}

void oncommand(ui_combobox* self, ui_component* sender, WPARAM wParam,
	LPARAM lParam) {
	switch(HIWORD(wParam))
    {
        case CBN_SELCHANGE :
        {
            if (self->signal_selchanged.slots) {
				int sel = SendMessage(self->currcombo->hwnd, CB_GETCURSEL,
					(WPARAM)0, (LPARAM)0);
				signal_emit(&self->signal_selchanged, self, 1, sel);			
			}
			if (self->ownerdrawn) {
				ui_invalidate(&self->component);
			}
        }
		break;
		default:
		break;
    }
}

void onownerdraw(ui_combobox* self, ui_component* sender, ui_graphics* g)
{
	ui_size size;
	ui_rectangle r;
	ui_point arrow_down[4];
	ui_point arrow_left[4];
	ui_point arrow_right[4];
	int ax;
	int ay;
	int sel;
	unsigned int arrowcolor = 0x00777777;
	unsigned int arrowhighlightcolor = 0x00FFFFFF;

	size = ui_component_size(&self->component);
	ui_setrectangle(&r, 0, 0, size.width, size.height);

	sel = SendMessage(self->combo.hwnd, CB_GETCURSEL, (WPARAM)0, (LPARAM)0);
	if (sel != CB_ERR) {
		int len;
		
		len = SendMessage(self->combo.hwnd, CB_GETLBTEXTLEN, (WPARAM)sel, 0);
		if (len > 0) {
			char* txt;

			txt = (char*)malloc(len + 1);			
			SendMessage(self->combo.hwnd, CB_GETLBTEXT, (WPARAM)sel,
				(LPARAM)txt);

			ui_setbackgroundmode(g, TRANSPARENT);
			if (self->hover) {
				ui_settextcolor(g, 0x00FFFFFF);
			} else {
				ui_settextcolor(g, 0x00CACACA);
			}
			ui_textoutrectangle(g, 0, 0, ETO_CLIPPED, r, txt, strlen(txt));
			free(txt);
		}
	}
	ax = size.width - 10;
	ay = 4;
	
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
	ay = 2;

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
	ay = 2;

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

void onmousedown(ui_combobox* self, ui_component* sender, int x, int y,
	int button)
{
	ui_size size = ui_component_size(sender);	

	if (x >= size.width - 40 && x < size.width - 25) {
		int index = ui_combobox_cursel(self);
		if (index > 0) {
			ui_combobox_setcursel(self,  index - 1);
			signal_emit(&self->signal_selchanged, self, 1, index - 1);
		}
	} else
	if (x >= size.width - 25 && x < size.width - 10) {
		int count;
		int index;

		count = SendMessage(self->currcombo->hwnd, CB_GETCOUNT, 0, (LPARAM)0);
		index = ui_combobox_cursel(self);
		if (index < count - 1) {
			ui_combobox_setcursel(self, index + 1);
			signal_emit(&self->signal_selchanged, self, 1, index + 1);
		}
	} else {
		SetWindowPos (self->combo.hwnd, NULL, 
		   0,
		   0,
		   size.width, size.height, SWP_NOZORDER | SWP_NOMOVE);
		SendMessage(self->combo.hwnd, CB_SHOWDROPDOWN, (WPARAM)TRUE, (LPARAM)0);
	}
}

void onmouseenter(ui_combobox* self, ui_component* sender)
{
	self->hover = 1;
	ui_invalidate(&self->component);
}

void onmousemove(ui_combobox* self, ui_component* sender, int x, int y,
	int button)
{
	if (self->hover) {
		int hover = self->hover;
		ui_size size = ui_component_size(sender);	

		if (x >= size.width - 40 && x < size.width - 25) {
			int index = ui_combobox_cursel(self);
			if (index > 0) {
				self->hover = 2;
			}
		} else
		if (x >= size.width - 25 && x < size.width - 10) {
			int count;
			int index;
			count = SendMessage(self->currcombo->hwnd, CB_GETCOUNT, 0,
				(LPARAM)0);
			index = ui_combobox_cursel(self);
			if (index < count - 1) {
				self->hover = 3;
			}
		} else {
			self->hover = 1;
		}
		if (hover != self->hover) {
			ui_invalidate(&self->component);
		}
	}
}

void onmouseleave(ui_combobox* self, ui_component* sender)
{		
	self->hover = 0;
	ui_invalidate(&self->component);
}


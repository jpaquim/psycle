// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#include "uibutton.h"
#include <string.h>

static unsigned int arrowcolor = 0x00777777;
static unsigned int arrowhighlightcolor = 0x00FFFFFF;

static void ui_button_create_system(ui_button*, ui_component* parent);
static void ui_button_create_ownerdrawn(ui_button*, ui_component* parent);
static void ondestroy(ui_button*, ui_component* sender);
static void onownerdraw(ui_button*, ui_component* sender, ui_graphics*);
static void drawicon(ui_button*, ui_graphics*);
static void drawarrow(ui_button*, ui_point* arrow, ui_graphics*);
static void makearrow(ui_point*, ButtonIcon icon, int x, int y);
static void onmousedown(ui_button*, ui_component* sender);
static void onmouseenter(ui_button*, ui_component* sender);
static void onmouseleave(ui_button*, ui_component* sender);
static void oncommand(ui_button*, ui_component* sender, WPARAM wParam,
	LPARAM lParam);
static void onpreferredsize(ui_button*, ui_component* sender, ui_size* limit,
	int* width, int* height);

void ui_button_init(ui_button* self, ui_component* parent)
{	
	self->hover = 0;
	self->icon = UI_ICON_NONE;
	if (self->ownerdrawn) {
		ui_button_create_ownerdrawn(self, parent);
		ui_component_setbackgroundmode(&self->component, BACKGROUND_SET);
	} else {
		ui_button_create_system(self, parent);
	}
	signal_init(&self->signal_clicked);	
	signal_connect(&self->component.signal_destroy, self, ondestroy);	
	signal_disconnectall(&self->component.signal_preferredsize);
	signal_connect(&self->component.signal_preferredsize, self,
		onpreferredsize);
}

void ui_button_create_system(ui_button* self, ui_component* parent)
{    	
	self->text = 0;
	ui_win32_component_init(&self->component, parent, TEXT("BUTTON"), 
		0, 0, 100, 20,
		WS_CHILD | WS_VISIBLE,
		1);
	signal_connect(&self->component.signal_command, self, oncommand);	
}

void ui_button_create_ownerdrawn(ui_button* self, ui_component* parent)
{
	self->text = _strdup("");	
	ui_component_init(&self->component, parent);
	signal_connect(&self->component.signal_draw, self, onownerdraw);	
	signal_connect(&self->component.signal_mousedown, self, onmousedown);
	signal_connect(&self->component.signal_mouseenter, self, onmouseenter);
	signal_connect(&self->component.signal_mouseleave, self, onmouseleave);
}

void ondestroy(ui_button* self, ui_component* sender)
{
	if (self->ownerdrawn == 1) {
		free(self->text);
	}
	signal_dispose(&self->signal_clicked);
}

void onownerdraw(ui_button* self, ui_component* sender, ui_graphics* g)
{
	ui_size size;
	ui_size textsize;
	ui_rectangle r;
	
	size = ui_component_size(&self->component);
	textsize = ui_component_textsize(&self->component, self->text);
	ui_setrectangle(&r, 0, 0, size.width, size.height);
	ui_setbackgroundmode(g, TRANSPARENT);
	if (self->hover) {
		ui_settextcolor(g, 0x00FFFFFF);
	} else {
		ui_settextcolor(g, 0x00CACACA);
	}
	ui_textoutrectangle(g, 
		(size.width - textsize.width) / 2,
		(size.height - textsize.height) / 2,
		ETO_CLIPPED,
		r,
		self->text,
		strlen(self->text));
	if (self->icon != UI_ICON_NONE) {
		drawicon(self, g);
	}
}

void drawicon(ui_button* self, ui_graphics* g)
{
	ui_size size;
	ui_point arrow[4];			
	
	size = ui_component_size(&self->component);
	if (self->icon == UI_ICON_LESSLESS) {
		makearrow(arrow,
			UI_ICON_LESS,
			(size.width) / 2 - 4,
			(size.height - 8) / 2);
		drawarrow(self, arrow, g);
		makearrow(arrow,
			UI_ICON_LESS,
			(size.width) / 2 + 4,
			(size.height - 8) / 2);	
		drawarrow(self, arrow, g);
	} else
	if (self->icon == UI_ICON_MOREMORE) {
		makearrow(arrow,
			UI_ICON_MORE,
			(size.width) / 2 - 4,
			(size.height - 8) / 2);	
		drawarrow(self, arrow, g);
		makearrow(arrow,
			UI_ICON_MORE,
			(size.width) / 2 + 4,
			(size.height - 8) / 2);	
		drawarrow(self, arrow, g);
	} else {
		makearrow(arrow,
			self->icon, 
			(size.width - 4) / 2,
			(size.height - 8) / 2);
		drawarrow(self, arrow, g);
	}
}

void drawarrow(ui_button* self, ui_point* arrow, ui_graphics* g)
{
	if (self->hover == 1) {
		ui_drawsolidpolygon(g, arrow, 4, arrowhighlightcolor,
			arrowhighlightcolor);
	} else {		
		ui_drawsolidpolygon(g, arrow, 4, arrowcolor, arrowcolor);
	}		
}

void makearrow(ui_point* arrow, ButtonIcon icon, int x, int y)
{
	switch (icon) {
		case UI_ICON_LESS:			
			arrow[0].x = 4 + x;
			arrow[0].y = 0 + y;
			arrow[1].x = 4 + x;
			arrow[1].y = 8 + y;
			arrow[2].x = 0 + x;
			arrow[2].y = 4 + y;
			arrow[3] = arrow[0];
		break;
		case UI_ICON_MORE:						
			arrow[0].x = 0 + x;
			arrow[0].y = 0 + y;
			arrow[1].x = 0 + x;
			arrow[1].y = 8 + y;
			arrow[2].x = 4 + x;
			arrow[2].y = 4 + y;
			arrow[3] = arrow[0];
		break;
		default:
		break;
	}
}

void onpreferredsize(ui_button* self, ui_component* sender, ui_size* limit,
	int* width, int* height)
{		
	if (self->ownerdrawn) {
		ui_size size;

		if (self->icon) {
			if (self->icon == UI_ICON_LESS || self->icon == UI_ICON_MORE) {
				size = ui_component_textsize(&self->component, "<");
			} else {
				size = ui_component_textsize(&self->component, "<<");
			}
		} else {
			size = ui_component_textsize(&self->component, self->text);			
		}
		*width = size.width + 4;
		*height = size.height + 4;
	} else {
		ui_size size;

		size = ui_component_size(&self->component);
		*width = size.width;
		*height = size.height;
	}
}

void onmousedown(ui_button* self, ui_component* sender)
{
	signal_emit(&self->signal_clicked, self, 0);
}

void onmouseenter(ui_button* self, ui_component* sender)
{
	self->hover = 1;
	ui_invalidate(&self->component);
}

void onmouseleave(ui_button* self, ui_component* sender)
{		
	self->hover = 0;
	ui_invalidate(&self->component);
}

void ui_button_settext(ui_button* self, const char* text)
{
	if (self->ownerdrawn) {
		free(self->text);
		self->text = _strdup(text);
		ui_invalidate(&self->component);
	} else {
		SetWindowText(self->component.hwnd, text);
	}
}

void ui_button_seticon(ui_button* self, ButtonIcon icon)
{
	self->icon = icon;
}

void ui_button_highlight(ui_button* self)
{
	SendMessage(self->component.hwnd, BM_SETSTATE, (WPARAM)1, (LPARAM)0);
}

void ui_button_disablehighlight(ui_button* self)
{
	SendMessage(self->component.hwnd, BM_SETSTATE, (WPARAM)0, (LPARAM)0);
}

void oncommand(ui_button* self, ui_component* sender, WPARAM wParam,
	LPARAM lParam)
{
	switch(HIWORD(wParam))
    {
        case BN_CLICKED:
        {            
			if (self->signal_clicked.slots) {
				signal_emit(&self->signal_clicked, self, 0);
			}
        }
		break;
		default:
		break;
    }
}


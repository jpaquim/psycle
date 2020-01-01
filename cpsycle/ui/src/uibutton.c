// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "uibutton.h"
#include <string.h>

static unsigned int arrowcolor = 0x00777777;
static unsigned int arrowhighlightcolor = 0x00FFFFFF;

static void ui_button_create_system(psy_ui_Button*, psy_ui_Component* parent);
static void ui_button_create_ownerdrawn(psy_ui_Button*, psy_ui_Component* parent);
static void ondestroy(psy_ui_Button*, psy_ui_Component* sender);
static void draw(psy_ui_Button*, psy_ui_Graphics*);
static void drawicon(psy_ui_Button*, psy_ui_Graphics*);
static void drawarrow(psy_ui_Button*, ui_point* arrow, psy_ui_Graphics*);
static void makearrow(ui_point*, ButtonIcon icon, int x, int y);
static void onmousedown(psy_ui_Button*, psy_ui_Component* sender);
static void onmouseenter(psy_ui_Button*, psy_ui_Component* sender);
static void onmouseleave(psy_ui_Button*, psy_ui_Component* sender);
static void oncommand(psy_ui_Button*, psy_ui_Component* sender, WPARAM wParam,
	LPARAM lParam);
static void preferredsize(psy_ui_Button*, ui_size* limit, ui_size* size);

static psy_ui_ComponentVtable vtable;
static int vtable_initialized = 0;

static void vtable_init(psy_ui_Button* self)
{
	if (!vtable_initialized) {
		vtable = *(self->component.vtable);
		vtable.draw = (psy_ui_fp_draw) draw;
		vtable.preferredsize = (psy_ui_fp_preferredsize) preferredsize;		
	}
}

void ui_button_init(psy_ui_Button* self, psy_ui_Component* parent)
{	
	self->ownerdrawn = 1;
	self->hover = 0;
	self->highlight = 0;
	self->icon = UI_ICON_NONE;
	self->charnumber = 0;
	self->textalignment = UI_ALIGNMENT_CENTER_VERTICAL |
		UI_ALIGNMENT_CENTER_HORIZONTAL;
	if (self->ownerdrawn) {
		ui_button_create_ownerdrawn(self, parent);
	} else {
		ui_button_create_system(self, parent);
	}
	vtable_init(self);
	self->component.vtable = &vtable;
	psy_signal_init(&self->signal_clicked);
	psy_signal_connect(&self->component.signal_destroy, self, ondestroy);
}	

void ui_button_create_system(psy_ui_Button* self, psy_ui_Component* parent)
{    	
	self->text = 0;
	ui_win32_component_init(&self->component, parent, TEXT("BUTTON"), 
		0, 0, 100, 20,
		WS_CHILD | WS_VISIBLE,
		1);
	psy_signal_connect(&self->component.signal_command, self, oncommand);	
}

void ui_button_create_ownerdrawn(psy_ui_Button* self, psy_ui_Component* parent)
{
	self->text = _strdup("");	
	ui_component_init(&self->component, parent);
	psy_signal_connect(&self->component.signal_mousedown, self, onmousedown);
	psy_signal_connect(&self->component.signal_mouseenter, self, onmouseenter);
	psy_signal_connect(&self->component.signal_mouseleave, self, onmouseleave);
}

void ondestroy(psy_ui_Button* self, psy_ui_Component* sender)
{
	if (self->ownerdrawn == 1) {
		free(self->text);
	}
	psy_signal_dispose(&self->signal_clicked);
}

void draw(psy_ui_Button* self, psy_ui_Graphics* g)
{
	ui_size size;
	ui_size textsize;
	ui_rectangle r;
	int centerx = 0;
	int centery = 0;
		
	size = ui_component_size(&self->component);
	textsize = ui_component_textsize(&self->component, self->text);
	ui_setrectangle(&r, 0, 0, size.width, size.height);
	ui_setbackgroundmode(g, TRANSPARENT);
	if (self->hover) {
		ui_settextcolor(g, 0x00FFFFFF);
	} else 
	if (self->highlight) {
		ui_settextcolor(g, 0x00FFFFFF);
	} else
	{
		ui_settextcolor(g, 0x00CACACA);
	}
	if ((self->textalignment & UI_ALIGNMENT_CENTER_HORIZONTAL) ==
		UI_ALIGNMENT_CENTER_HORIZONTAL) {
		centerx = (size.width - textsize.width) / 2;		
	}
	if ((self->textalignment & UI_ALIGNMENT_CENTER_VERTICAL) ==
		UI_ALIGNMENT_CENTER_VERTICAL) {
		centery = (size.height - textsize.height) / 2;
	}	
	ui_textoutrectangle(g, 
		centerx,
		centery,
		ETO_CLIPPED,
		r,
		self->text,
		strlen(self->text));
	if (self->icon != UI_ICON_NONE) {
		drawicon(self, g);
	}
}

void drawicon(psy_ui_Button* self, psy_ui_Graphics* g)
{
	ui_size size;
	ui_point arrow[4];
	int centerx = 4;
	int centery = 0;
	
	size = ui_component_size(&self->component);
	if ((self->textalignment & UI_ALIGNMENT_CENTER_HORIZONTAL) ==
		UI_ALIGNMENT_CENTER_HORIZONTAL) {
		centerx = (size.width - 4) / 2;		
	}
	if ((self->textalignment & UI_ALIGNMENT_CENTER_VERTICAL) ==
		UI_ALIGNMENT_CENTER_VERTICAL) {
		centery = (size.height - 8) / 2;
	}
	if (self->icon == UI_ICON_LESSLESS) {
		makearrow(arrow,
			UI_ICON_LESS,
			centerx - 4,
			centery);
		drawarrow(self, arrow, g);
		makearrow(arrow,
			UI_ICON_LESS,
			centerx + 4,
			centery);	
		drawarrow(self, arrow, g);
	} else
	if (self->icon == UI_ICON_MOREMORE) {
		makearrow(arrow,
			UI_ICON_MORE,
			centerx - 6,
			centery);
		drawarrow(self, arrow, g);
		makearrow(arrow,
			UI_ICON_MORE,
			centerx + 2,
			centery);	
		drawarrow(self, arrow, g);
	} else {
		makearrow(arrow,
			self->icon, 
			centerx - 2,
			centery);
		drawarrow(self, arrow, g);
	}
}

void drawarrow(psy_ui_Button* self, ui_point* arrow, psy_ui_Graphics* g)
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

void ui_button_setcharnumber(psy_ui_Button* self, int number)
{
	self->charnumber = number;
}

void preferredsize(psy_ui_Button* self, ui_size* limit, ui_size* rv)
{		
	if (rv) {
		if (self->ownerdrawn) {
			ui_textmetric tm;	
			ui_size size;

			tm = ui_component_textmetric(&self->component);
			if (self->charnumber == 0) {
				if (self->icon) {
					if (self->icon == UI_ICON_LESS || self->icon == UI_ICON_MORE) {
						size = ui_component_textsize(&self->component, "<");
					} else {
						size = ui_component_textsize(&self->component, "<<");
					}
				} else {
					size = ui_component_textsize(&self->component, self->text);			
				}
				rv->width = size.width + 4;				
			} else {
				rv->width = tm.tmAveCharWidth * self->charnumber;
			}
			rv->height = tm.tmHeight;
		} else {
			*rv = ui_component_size(&self->component);		
		}
	}
}

void onmousedown(psy_ui_Button* self, psy_ui_Component* sender)
{
	psy_signal_emit(&self->signal_clicked, self, 0);
}

void onmouseenter(psy_ui_Button* self, psy_ui_Component* sender)
{
	self->hover = 1;
	ui_component_invalidate(&self->component);
}

void onmouseleave(psy_ui_Button* self, psy_ui_Component* sender)
{		
	self->hover = 0;
	ui_component_invalidate(&self->component);
}

void ui_button_settext(psy_ui_Button* self, const char* text)
{
	if (self->ownerdrawn) {
		free(self->text);
		self->text = _strdup(text);
		ui_component_invalidate(&self->component);
	} else {
		SetWindowText((HWND)self->component.hwnd, text);
	}
}

void ui_button_seticon(psy_ui_Button* self, ButtonIcon icon)
{
	self->icon = icon;
}

void ui_button_highlight(psy_ui_Button* self)
{
	if (!self->highlight) {
		self->highlight = 1;
		SendMessage((HWND)self->component.hwnd, BM_SETSTATE, (WPARAM)1, (LPARAM)0);
		if (self->ownerdrawn) {
			ui_component_invalidate(&self->component);
		}
	}
}

void ui_button_disablehighlight(psy_ui_Button* self)
{
	if (self->highlight) {
		self->highlight = 0;
		SendMessage((HWND)self->component.hwnd, BM_SETSTATE, (WPARAM)0, (LPARAM)0);
		if (self->ownerdrawn) {
			ui_component_invalidate(&self->component);
		}
	}
}

void ui_button_settextalignment(psy_ui_Button* self, UiAlignment alignment)
{
	self->textalignment = alignment;
}

void oncommand(psy_ui_Button* self, psy_ui_Component* sender, WPARAM wParam,
	LPARAM lParam)
{
	switch(HIWORD(wParam))
    {
        case BN_CLICKED:
        {            
			if (self->signal_clicked.slots) {
				psy_signal_emit(&self->signal_clicked, self, 0);
			}
        }
		break;
		default:
		break;
    }
}

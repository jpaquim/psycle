// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net


#include "uibutton.h"
#include "hashtbl.h"
#include <string.h>

extern IntHashTable selfmap;
extern IntHashTable winidmap;
extern int winid;

static void OnCommand(ui_button* self, WPARAM wParam, LPARAM lParam);
static void ondestroy(ui_button* self, ui_component* sender);
static void ui_button_create_system(ui_button*, ui_component* parent);
static void ui_button_create_ownerdrawn(ui_button*, ui_component* parent);
static void onownerdraw(ui_button*, ui_component* sender, ui_graphics*);
static void onmousedown(ui_button*, ui_component* sender);
static void onmouseenter(ui_button*, ui_component* sender);
static void onmouseleave(ui_button*, ui_component* sender);
static void onpreferredsize(ui_button*, ui_component* sender, ui_size* limit, int* width, int* height);

void ui_button_init(ui_button* button, ui_component* parent)
{  
	memset(&button->component.events, 0, sizeof(ui_events));	
	button->hover = 0;
	ui_component_init_signals(&button->component);
	signal_init(&button->signal_clicked);
	button->component.doublebuffered = 0;
	ui_button_create_ownerdrawn(button, parent);
	signal_connect(&button->component.signal_destroy, button,  ondestroy);	
	ui_component_init_base(&button->component);
	signal_disconnectall(&button->component.signal_preferredsize);
	signal_connect(&button->component.signal_preferredsize, button, onpreferredsize);
	if (button->ownerdrawn) {
		ui_component_setbackgroundmode(&button->component, BACKGROUND_SET);
	}
}

void ui_button_create_system(ui_button* button, ui_component* parent)
{    	
	button->ownerdrawn = 0;
	button->text = 0;
	button->component.hwnd = CreateWindow (TEXT("BUTTON"), NULL,
		WS_CHILD | WS_VISIBLE,
		0, 0, 90, 90,
		parent->hwnd, (HMENU)winid,
		(HINSTANCE) GetWindowLong (parent->hwnd, GWL_HINSTANCE),
		NULL);		
	InsertIntHashTable(&selfmap, (int)button->component.hwnd, &button->component);	
	button->component.events.target = button;
	button->component.events.cmdtarget = button;
	InsertIntHashTable(&winidmap, (int)winid, &button->component);
	winid++;	
	button->component.events.command = OnCommand;
}

void ui_button_create_ownerdrawn(ui_button* self, ui_component* parent)
{
	self->ownerdrawn = 1;
	self->text = strdup("");
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
	ui_rectangle r;

	size = ui_component_size(&self->component);
	ui_setrectangle(&r, 0, 0, size.width, size.height);
	ui_setbackgroundmode(g, TRANSPARENT);
	if (self->hover) {
		ui_settextcolor(g, 0x00FFFFFF);
	} else {
		ui_settextcolor(g, 0x00CACACA);
	}
	ui_textoutrectangle(g, 0, 0, ETO_CLIPPED, r, self->text,
		strlen(self->text));
}

void onpreferredsize(ui_button* self, ui_component* sender, ui_size* limit, int* width, int* height)
{		
	if (self->ownerdrawn) {
		ui_size size;

		size = ui_component_textsize(&self->component, self->text);
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
		self->text = strdup(text);
	} else {
		SetWindowText(self->component.hwnd, text);
	}
}

void ui_button_highlight(ui_button* self)
{
	SendMessage(self->component.hwnd, BM_SETSTATE, (WPARAM)1, (LPARAM)0);
}

void ui_button_disablehighlight(ui_button* self)
{
	SendMessage(self->component.hwnd, BM_SETSTATE, (WPARAM)0, (LPARAM)0);
}

static void OnCommand(ui_button* self, WPARAM wParam, LPARAM lParam)
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


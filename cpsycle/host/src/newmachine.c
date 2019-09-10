// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net
#include "newmachine.h"
#include <plugin_interface.h>

static void OnDestroy(NewMachine* self, ui_component* component);
static void OnDraw(NewMachine* self, ui_component* sender, ui_graphics* g);
static void DrawBackground(NewMachine* self, ui_graphics* g);
static void OnSize(NewMachine* self, ui_component* sender, int width, int height);
static void OnMouseDown(NewMachine* self, ui_component* sender, int x, int y, int button);
static void HitTest(NewMachine* self, int x, int y);
static int OnPropertiesEnum(NewMachine* self, Properties* curr_properties, int level);
static void OnScroll(NewMachine* self, ui_component* sender, int cx, int cy);
static int OnPropertiesCount(NewMachine* self, Properties* property, int level);
static void OnMouseDoubleClick(NewMachine* self, ui_component* sender, int x, int y, int button);
static void OnKeyDown(NewMachine* self, ui_component* sender, int keycode, int keydata);

static int cpy;
static int dy;
static int count;

void InitNewMachine(NewMachine* self, ui_component* parent, Player* player, Properties* properties)
{	
	Properties* property;
	
	ui_component_init(&self->component, parent);	
	ui_component_showverticalscrollbar(&self->component);
	signal_connect(&self->component.signal_destroy, self,OnDestroy);	
	signal_connect(&self->component.signal_size, self, OnSize);
	signal_connect(&self->component.signal_scroll, self, OnScroll);	
	signal_connect(&self->component.signal_keydown, self, OnKeyDown);
	signal_connect(&self->component.signal_mousedown, self, OnMouseDown);
	signal_connect(&self->component.signal_mousedoubleclick, self, OnMouseDoubleClick);
	signal_connect(&self->component.signal_draw, self, OnDraw);
	ui_component_move(&self->component, 0, 0);	
	self->selectedplugin = 0;
	plugincatcher_init(&self->plugincatcher);
	plugincatcher_scan(&self->plugincatcher, 0, 3);		
	property = properties_read(properties, "vstdir");
	if (property) {
		plugincatcher_scan(&self->plugincatcher, property->item.value.s, 1);
	}
	property = properties_read(properties, "plugindir");
	if (property) {
		plugincatcher_scan(&self->plugincatcher, property->item.value.s, 0);
	}
	dy = 0;
	self-> selectioncontext = parent;
}

void OnDestroy(NewMachine* self, ui_component* component)
{
	plugincatcher_dispose(&self->plugincatcher);
}

void OnDraw(NewMachine* self, ui_component* sender, ui_graphics* g)
{	   	
	DrawBackground(self, g);
	self->g = g;
	cpy = 0;
	ui_setbackgroundcolor(g, 0x009a887c);
	ui_settextcolor(g, 0x00000000);
	properties_enumerate(self->plugincatcher.plugins, self, OnPropertiesEnum);
}


void DrawBackground(NewMachine* self, ui_graphics* g)
{
	ui_rectangle r;
	
	ui_setrectangle(&r, 0, 0, self->cx, self->cy);
	ui_drawsolidrectangle(g, r, 0x009a887c);	
}

int OnPropertiesEnum(NewMachine* self, Properties* property, int level)
{		
	if (property->item.key) {				
		CMachineInfo* pInfo;		
	//	ui_textout(self->g, 20, 40 + cpy + dy, property->item.key, strlen(property->item.key));				
		if (property == self->selectedplugin) {
			ui_rectangle r = { 19, 19, 300, 37 };
			r.top += cpy + dy;
			r.bottom += cpy + dy;
			ui_drawrectangle(self->g, r);
		}
		pInfo = (CMachineInfo*)property->item.value.ud;				
		ui_textout(self->g, 20, 20 + cpy + dy, pInfo->Name, strlen(pInfo->Name));	
		// ui_textout(self->g, 200, 20 + cpy + dy, pInfo->Author, strlen(pInfo->Author));	
		cpy += 20;
	}
	return 1;
}

void OnSize(NewMachine* self, ui_component* sender, int width, int height)
{
	self->cx = width;
	self->cy = height;

	ui_component_setverticalscrollrange(&self->component, 0, 100);
}

void OnScroll(NewMachine* self, ui_component* sender, int cx, int cy)
{
	dy += cy;
}

void OnMouseDown(NewMachine* self, ui_component* sender, int x, int y, int button)
{
	HitTest(self, x, y);
}

void HitTest(NewMachine* self, int x, int y)
{	
	self->pluginpos = (y - dy) / 20;
	count = 0;	
	properties_enumerate(self->plugincatcher.plugins, self, OnPropertiesCount);
	InvalidateRect(self->component.hwnd, NULL, TRUE);
}

int OnPropertiesCount(NewMachine* self, Properties* property, int level)
{
	if (self->pluginpos == count) {
		self->selectedplugin = property;
		return 0;
	}
	++count;
	return 1;
}

void OnMouseDoubleClick(NewMachine* self, ui_component* sender, int x, int y, int button)
{
	if (self->selectedplugin && self->selected) {
		self->selected(self->selectioncontext,
		(CMachineInfo*)self->selectedplugin->item.value.ud,
		self->selectedplugin->item.key);
	}
	ui_component_hide(&self->component);
}

void OnKeyDown(NewMachine* self, ui_component* sender, int keycode, int keydata)
{
	if (keycode == VK_ESCAPE) {
		ui_component_hide(&self->component);
	}
}


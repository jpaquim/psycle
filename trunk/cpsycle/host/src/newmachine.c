// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net
#include "newmachine.h"
#include <plugin_interface.h>

static void OnDestroy(NewMachine* self, ui_component* component);
static void OnDraw(NewMachine* self, ui_component* sender, ui_graphics* g);
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

void InitNewMachine(NewMachine* self, ui_component* parent, Workspace* workspace)
{	
	ui_component_init(&self->component, parent);
	ui_component_setbackgroundmode(&self->component, BACKGROUND_SET);
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
	self->workspace = workspace;	
	dy = 0;	
	signal_init(&self->signal_selected);
}

void OnDestroy(NewMachine* self, ui_component* component)
{
	signal_dispose(&self->signal_selected);
}

void OnDraw(NewMachine* self, ui_component* sender, ui_graphics* g)
{	   	
	Properties* pluginlist;

	self->g = g;
	cpy = 0;	
	pluginlist = workspace_pluginlist(self->workspace);
	if (pluginlist) {
		properties_enumerate(pluginlist, self, OnPropertiesEnum);
	}
}

int OnPropertiesEnum(NewMachine* self, Properties* property, int level)
{		
	if (property->item.key && property->children) {						
		Properties* p;
	//	ui_textout(self->g, 20, 40 + cpy + dy, property->item.key, strlen(property->item.key));				
		if (property == self->selectedplugin) {
			ui_setbackgroundcolor(self->g, 0x009B7800);
			ui_settextcolor(self->g, 0x00FFFFFF);		
		} else {
			ui_setbackgroundcolor(self->g, 0x003E3E3E);
			ui_settextcolor(self->g, 0x00CACACA);		
		}
		if (property == self->selectedplugin) {
			ui_rectangle r = { 19, 19, 300, 37 };
			r.top += cpy + dy;
			r.bottom += cpy + dy;
			ui_setcolor(self->g, 0x00FFFFFF);
			ui_drawrectangle(self->g, r);
		}
		p = properties_read(property->children, "name");
		if (p && p->item.key && p->item.typ == PROPERTY_TYP_STRING) {
			ui_textout(self->g, 20, 20 + cpy + dy, p->item.value.s,
				strlen(p->item.value.s));	
		}
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
	ui_component_setfocus(&self->component);
}

void HitTest(NewMachine* self, int x, int y)
{	
	Properties* pluginlist;

	pluginlist = workspace_pluginlist(self->workspace);
	if (pluginlist) {
		self->pluginpos = (y - dy) / 20;
		count = 0;			
		properties_enumerate(pluginlist, self,OnPropertiesCount);
		ui_invalidate(&self->component);
	}	
}

int OnPropertiesCount(NewMachine* self, Properties* property, int level)
{
	if (self->pluginpos == count && level == 0) {
		self->selectedplugin = property;
		return 0;
	}
	if (level == 0) {
		++count;
	}
	return 1;
}

void OnMouseDoubleClick(NewMachine* self, ui_component* sender, int x, int y, int button)
{
	if (self->selectedplugin) {
		signal_emit(&self->signal_selected, self, 1, self->selectedplugin);		
	}	
}

void OnKeyDown(NewMachine* self, ui_component* sender, int keycode, int keydata)
{
	if (keycode == VK_ESCAPE) {	
		self->component.propagateevent = 1;
	}
}


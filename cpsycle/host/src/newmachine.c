// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net
#include "newmachine.h"
#include <plugin_interface.h>

static void OnDestroy(PluginsView*, ui_component* component);
static void OnDraw(PluginsView*, ui_component* sender, ui_graphics* g);
static void OnSize(NewMachine*, ui_component* sender, int width, int height);
static void OnPluginsSize(PluginsView*, ui_component* sender, int width, int height);
static void OnMouseDown(PluginsView*, ui_component* sender, int x, int y, int button);
static void HitTest(PluginsView*, int x, int y);
static int OnPropertiesEnum(PluginsView*, Properties* curr_properties, int level);
static void OnScroll(PluginsView*, ui_component* sender, int cx, int cy);
static int OnPropertiesCount(PluginsView*, Properties* property, int level);
static void OnMouseDoubleClick(PluginsView*, ui_component* sender, int x, int y, int button);
static void OnKeyDown(NewMachine*, ui_component* sender, int keycode, int keydata);
static void OnPluginsKeyDown(PluginsView*, ui_component* sender, int keycode, int keydata);
static void onrescan(NewMachineBar*, ui_component* sender);
static void onplugincachechanged(PluginsView* self, PluginCatcher* sender);

static int cpy;
static int count;

void InitNewMachineBar(NewMachineBar* self, ui_component* parent, Workspace* workspace)
{
	self->workspace = workspace;
	ui_component_init(&self->component, parent);
	ui_component_setbackgroundmode(&self->component, BACKGROUND_SET);
	ui_button_init(&self->rescan, &self->component);
	ui_button_settext(&self->rescan, "Rescan");
	signal_connect(&self->rescan.signal_clicked, self, onrescan);
	ui_component_setposition(&self->rescan.component, 0, 0, 100, 20);	
}

void onrescan(NewMachineBar* self, ui_component* sender)
{
	workspace_scanplugins(self->workspace);
}

void InitPluginsView(PluginsView* self, ui_component* parent, Workspace* workspace)
{	
	ui_component_init(&self->component, parent);
	ui_component_setbackgroundmode(&self->component, BACKGROUND_SET);
	ui_component_showverticalscrollbar(&self->component);
	signal_connect(&self->component.signal_destroy, self,OnDestroy);	
	signal_connect(&self->component.signal_size, self, OnPluginsSize);
	signal_connect(&self->component.signal_scroll, self, OnScroll);	
	signal_connect(&self->component.signal_keydown, self, OnPluginsKeyDown);
	signal_connect(&self->component.signal_mousedown, self, OnMouseDown);
	signal_connect(&self->component.signal_mousedoubleclick, self, OnMouseDoubleClick);
	signal_connect(&self->component.signal_draw, self, OnDraw);
	ui_component_move(&self->component, 0, 0);	
	self->selectedplugin = 0;
	self->workspace = workspace;	
	self->dy = 0;	
	self->calledbygear = 0;
	signal_init(&self->signal_selected);
	signal_connect(&workspace->plugincatcher.signal_changed, self,
		onplugincachechanged);
}

void OnDestroy(PluginsView* self, ui_component* component)
{
	signal_dispose(&self->signal_selected);
}

void OnDraw(PluginsView* self, ui_component* sender, ui_graphics* g)
{	   	
	Properties* plugins;

	self->g = g;
	cpy = 0;	
	plugins = workspace_pluginlist(self->workspace);
	if (plugins) {
		properties_enumerate(plugins, self, OnPropertiesEnum);
	}
}

int OnPropertiesEnum(PluginsView* self, Properties* property, int level)
{		
	if (property->item.key && property->children) {						
		Properties* p;
	//	ui_textout(self->g, 20, 40 + cpy + self->dy, property->item.key, strlen(property->item.key));				
		if (property == self->selectedplugin) {
			ui_setbackgroundcolor(self->g, 0x009B7800);
			ui_settextcolor(self->g, 0x00FFFFFF);		
		} else {
			ui_setbackgroundcolor(self->g, 0x003E3E3E);
			ui_settextcolor(self->g, 0x00CACACA);		
		}		
		p = properties_read(property, "name");
		if (p && p->item.key && p->item.typ == PROPERTY_TYP_STRING) {
			ui_textout(self->g, 22, cpy + self->dy + 2, p->item.value.s,
				strlen(p->item.value.s));	
			cpy += 20;
		}
		// ui_textout(self->g, 200, 20 + cpy + self->dy, pInfo->Author, strlen(pInfo->Author));			
	}
	return 1;
}

void OnPluginsSize(PluginsView* self, ui_component* sender, int width, int height)
{
	self->cx = width;
	self->cy = height;

	ui_component_setverticalscrollrange(&self->component, 0, 100);
}

void OnScroll(PluginsView* self, ui_component* sender, int cx, int cy)
{
	self->dy += cy;
}

void OnMouseDown(PluginsView* self, ui_component* sender, int x, int y, int button)
{
	HitTest(self, x, y);
	ui_component_setfocus(&self->component);
}

void HitTest(PluginsView* self, int x, int y)
{	
	Properties* plugins;

	plugins = workspace_pluginlist(self->workspace);
	if (plugins) {
		self->pluginpos = (y - self->dy) / 20;
		count = 0;			
		properties_enumerate(plugins, self,OnPropertiesCount);
		ui_invalidate(&self->component);
	}	
}

int OnPropertiesCount(PluginsView* self, Properties* property, int level)
{
	if (self->pluginpos == count && level == 1) {
		self->selectedplugin = property;
		return 0;
	}
	if (level == 1) {
		++count;
	}
	return 1;
}

void OnMouseDoubleClick(PluginsView* self, ui_component* sender, int x, int y, int button)
{
	if (self->selectedplugin) {
		signal_emit(&self->signal_selected, self, 1, self->selectedplugin);
		self->calledbygear = 0;
	}	
}

void OnPluginsKeyDown(PluginsView* self, ui_component* sender, int keycode, int keydata)
{
	if (keycode == VK_ESCAPE) {	
		self->component.propagateevent = 1;
	}
}

void onplugincachechanged(PluginsView* self, PluginCatcher* sender)
{
	self->dy = 0;
	ui_invalidate(&self->component);
}

void InitNewMachine(NewMachine* self, ui_component* parent, Workspace* workspace)
{
	ui_component_init(&self->component, parent);
	ui_component_setbackgroundmode(&self->component, BACKGROUND_SET);
	InitPluginsView(&self->pluginsview, &self->component, workspace);
	InitNewMachineBar(&self->bar, &self->component, workspace);
	signal_connect(&self->component.signal_size, self, OnSize);
}

void OnKeyDown(NewMachine* self, ui_component* sender, int keycode, int keydata)
{
	if (keycode == VK_ESCAPE) {	
		self->component.propagateevent = 1;
	}
}

void OnSize(NewMachine* self, ui_component* sender, int width, int height)
{	
	ui_size barsize;
	
	barsize.height = 20;		
	ui_component_resize(&self->pluginsview.component,
		width,
		height - barsize.height);
	ui_component_setposition(&self->bar.component,
		0,
		height - barsize.height,		 
		width, 
		barsize.height);
}

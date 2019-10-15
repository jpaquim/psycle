// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "newmachine.h"
#include <plugin_interface.h>

static void OnDestroy(PluginsView*, ui_component* component);
static void OnDraw(PluginsView*, ui_component* sender, ui_graphics* g);
static void OnSize(NewMachine*, ui_component* sender, int width, int height);
static void OnPluginsSize(PluginsView*, ui_component* sender, int width, int height);
static void OnMouseDown(PluginsView*, ui_component* sender, int x, int y, int button);
static void HitTest(PluginsView*, int x, int y);
static void ComputeTextSizes(PluginsView* self);
static int OnDrawPropertiesEnum(PluginsView*, Properties* curr_properties, int level);
static void OnScroll(PluginsView*, ui_component* sender, int cx, int cy);
static int OnPropertiesCount(PluginsView*, Properties* property, int level);
static void OnMouseDoubleClick(PluginsView*, ui_component* sender, int x, int y, int button);
static void OnKeyDown(NewMachine*, ui_component* sender, int keycode, int keydata);
static void OnPluginsKeyDown(PluginsView*, ui_component* sender, int keycode, int keydata);
static void onrescan(NewMachineBar*, ui_component* sender);
static void onplugincachechanged(PluginsView* self, PluginCatcher* sender);
static void InitNewMachineDetail(NewMachineDetail* self, ui_component* parent);
static void onpluginselectionchanged(NewMachine*, ui_component* parent, Properties*);

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

void InitNewMachineDetail(NewMachineDetail* self, ui_component* parent)
{
	ui_component_init(&self->component, parent);
	ui_component_setbackgroundmode(&self->component, BACKGROUND_SET);
	ui_label_init(&self->desclabel, &self->component);
	ui_label_settext(&self->desclabel, "");	
	ui_component_setposition(&self->desclabel.component, 0, 0, 600, 50);
}

void InitPluginsView(PluginsView* self, ui_component* parent, Workspace* workspace)
{	
	ui_component_init(&self->component, parent);
	ui_component_setbackgroundmode(&self->component, BACKGROUND_SET);
	ui_component_showverticalscrollbar(&self->component);
	self->component.doublebuffered = 1;
	signal_connect(&self->component.signal_destroy, self,OnDestroy);	
	signal_connect(&self->component.signal_size, self, OnPluginsSize);
	signal_connect(&self->component.signal_scroll, self, OnScroll);	
	signal_connect(&self->component.signal_keydown, self, OnPluginsKeyDown);
	signal_connect(&self->component.signal_mousedown, self, OnMouseDown);
	signal_connect(&self->component.signal_mousedoubleclick, self, OnMouseDoubleClick);
	signal_connect(&self->component.signal_draw, self, OnDraw);	
	self->selectedplugin = 0;
	self->workspace = workspace;	
	self->dy = 0;	
	self->calledbygear = 0;
	signal_init(&self->signal_selected);
	signal_init(&self->signal_changed);
	signal_connect(&workspace->plugincatcher.signal_changed, self,
		onplugincachechanged);
}

void OnDestroy(PluginsView* self, ui_component* component)
{
	signal_dispose(&self->signal_selected);
	signal_dispose(&self->signal_changed);
}

void OnDraw(PluginsView* self, ui_component* sender, ui_graphics* g)
{	   		
	self->g = g;
	self->cpx = 0;
	self->cpy = 0;
	ComputeTextSizes(self);
	if (workspace_pluginlist(self->workspace)) {
		properties_enumerate(workspace_pluginlist(self->workspace),
			self, OnDrawPropertiesEnum);
	}
}

void ComputeTextSizes(PluginsView* self)
{
	TEXTMETRIC tm;
	
	tm = ui_component_textmetric(&self->component);
	self->avgcharwidth = tm.tmAveCharWidth;
	self->lineheight = (int) (tm.tmHeight * 1.5);
	self->columnwidth = tm.tmAveCharWidth * 50;
	self->identwidth = tm.tmAveCharWidth * 4;
	self->numcols = self->cx / self->columnwidth;
}

int OnDrawPropertiesEnum(PluginsView* self, Properties* property, int level)
{		
	if (property->item.key && property->children) {						
		Properties* p;	
		if (property == self->selectedplugin) {
			ui_setbackgroundcolor(self->g, 0x009B7800);
			ui_settextcolor(self->g, 0x00FFFFFF);		
		} else {
			ui_setbackgroundcolor(self->g, 0x00232323);
			ui_settextcolor(self->g, 0x00CACACA);		
		}		
		p = properties_read(property, "name");
		if (p && p->item.key && p->item.typ == PROPERTY_TYP_STRING) {
			int mode;
			int typ;

			ui_textout(self->g, self->cpx, self->cpy + self->dy + 2,
				properties_valuestring(p),
				strlen(properties_valuestring(p)));
			mode = properties_int(property, "mode", -1);
			self->cpx += self->columnwidth;
			if (mode != -1) {
				const char* modestr;
				
				modestr = ((mode & 3) == 3) ? "gn" : "fx";
				if ((mode &3)) {
					ui_settextcolor(self->g, 0x00B1C8B0);
				} else {
					ui_settextcolor(self->g, 0x00D1C5B6);
				}
				ui_textout(self->g,
					self->cpx - 10 * self->avgcharwidth,
					self->cpy + self->dy + 2,
					modestr,
					strlen(modestr));				
			}		
			typ = properties_int(property, "type", -1);
			if (typ != -1) {
				const char* typestr;
				
				switch (typ) {
					case MACH_PLUGIN:
						typestr = "psy";
					break;
					case MACH_VST:
						typestr = "vst";
					break;
					default:
						typestr = "int";
					break;
				}				
				ui_textout(self->g,
					self->cpx - 7 * self->avgcharwidth,
					self->cpy + self->dy + 2,
					typestr,
					strlen(typestr));				
			}
			if (self->cpx >= self->numcols * self->columnwidth) {
				self->cpx = 0;
				self->cpy += self->lineheight;
			}
		}		
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
	signal_emit(&self->signal_changed, self, 1, self->selectedplugin);
	ui_component_setfocus(&self->component);
}

void HitTest(PluginsView* self, int x, int y)
{	
	Properties* plugins;

	plugins = workspace_pluginlist(self->workspace);
	if (plugins) {
		ComputeTextSizes(self);
		self->pluginpos = (x / self->columnwidth) + 
			self->numcols * ((y - self->dy) / self->lineheight);
		self->count = 0;
		properties_enumerate(plugins, self,OnPropertiesCount);
		ui_invalidate(&self->component);
	}	
}

int OnPropertiesCount(PluginsView* self, Properties* property, int level)
{
	if (self->pluginpos == self->count && level == 1) {
		self->selectedplugin = property;
		return 0;
	}
	if (level == 1) {
		++self->count;
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
	InitNewMachineDetail(&self->detail, &self->component);
	signal_connect(&self->component.signal_size, self, OnSize);
	signal_connect(&self->pluginsview.signal_changed, self,
		onpluginselectionchanged);
}

void onpluginselectionchanged(NewMachine* self, ui_component* parent, Properties* selected)
{
	char* text;
	char detail[1024];

	properties_readstring(selected, "name", &text, "");
	strcpy(detail, text);
	properties_readstring(selected, "desc", &text, "");
	strcat(detail, "  ");
	strcat(detail, text);
	ui_label_settext(&self->detail.desclabel, detail); 
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
	ui_size detailsize;
	
	barsize.height = 20;
	detailsize.height = 100;
	ui_component_resize(&self->pluginsview.component,
		width,
		height - barsize.height - detailsize.height);
	ui_component_setposition(&self->detail.component,
		0,
		height - barsize.height - detailsize.height,		 
		width, 
		detailsize.height);
	ui_component_setposition(&self->bar.component,
		0,
		height - barsize.height,		 
		width, 
		barsize.height);
}

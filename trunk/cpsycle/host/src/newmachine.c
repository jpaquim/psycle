// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "newmachine.h"
#include <plugin_interface.h>

static void newmachine_onpluginselected(NewMachine*, ui_component* parent,
	Properties*);
static void newmachine_onplugincachechanged(NewMachine*, PluginCatcher*);
static void newmachine_onkeydown(NewMachine*, ui_component* sender, 
	int keycode, int keydata);

static void pluginsview_ondestroy(PluginsView*, ui_component* component);
static void pluginsview_ondraw(PluginsView*, ui_component* sender,
	ui_graphics*);
static void pluginsview_drawitem(PluginsView*, ui_graphics* g, Properties*,
	int x, int y);
static void pluginsview_onsize(PluginsView*, ui_component* sender,
	ui_size* size);
static void pluginsview_onmousedown(PluginsView*, ui_component* sender,
	int x, int y, int button);
static void pluginsview_hittest(PluginsView*, int x, int y);
static void pluginsview_computetextsizes(PluginsView* self);
static void pluginsview_onscroll(PluginsView*, ui_component* sender,
	int cx, int cy);
static void pluginsview_onmousedoubleclick(PluginsView*, ui_component* sender,
	int x, int y, int button);
static void pluginsview_onkeydown(PluginsView*, ui_component* sender,
	int keycode, int keydata);
static void pluginsview_onplugincachechanged(PluginsView*,
	PluginCatcher* sender);
static void pluginsview_adjustscroll(PluginsView*);

static void pluginname(Properties*, char* txt);
static int plugintype(Properties*, char* txt);
static int pluginmode(Properties*, char* txt);

static void newmachinebar_onrescan(NewMachineBar*, ui_component* sender);

static void newmachinedetail_reset(NewMachineDetail*);

void newmachinebar_init(NewMachineBar* self, ui_component* parent,
	Workspace* workspace)
{
	self->workspace = workspace;
	ui_component_init(&self->component, parent);	
	ui_button_init(&self->rescan, &self->component);
	ui_button_settext(&self->rescan, "Rescan");
	signal_connect(&self->rescan.signal_clicked, self,
		newmachinebar_onrescan);
	ui_component_setposition(&self->rescan.component, 0, 0, 80, 20);
}

void newmachinebar_onrescan(NewMachineBar* self, ui_component* sender)
{
	workspace_scanplugins(self->workspace);
}

void newmachinedetail_init(NewMachineDetail* self, ui_component* parent,
	Workspace* workspace)
{
	ui_component_init(&self->component, parent);	
	newmachinebar_init(&self->bar, &self->component, workspace);
	ui_component_setposition(&self->bar.component, 0, 10, 80, 100);
	ui_label_init(&self->desclabel, &self->component);
	ui_label_setstyle(&self->desclabel, WS_CHILD | WS_VISIBLE | SS_CENTER);
	ui_label_settext(&self->desclabel, 
		"Select a plugin to view its description.");	
	ui_component_setposition(&self->desclabel.component, 0, 110, 80, 100);	
}

void newmachinedetail_reset(NewMachineDetail* self)
{
	ui_label_settext(&self->desclabel, 
		"Select a plugin to view its description.");	
}

void pluginsview_init(PluginsView* self, ui_component* parent,
	Workspace* workspace)
{	
	ui_component_init(&self->component, parent);	
	ui_component_showverticalscrollbar(&self->component);
	self->component.doublebuffered = 1;
	signal_connect(&self->component.signal_destroy, self,
		pluginsview_ondestroy);	
	signal_connect(&self->component.signal_size, self,
		pluginsview_onsize);
	signal_connect(&self->component.signal_scroll, self,
		pluginsview_onscroll);	
	signal_connect(&self->component.signal_keydown, self,
		pluginsview_onkeydown);
	signal_connect(&self->component.signal_mousedown, self,
		pluginsview_onmousedown);
	signal_connect(&self->component.signal_mousedoubleclick, self,
		pluginsview_onmousedoubleclick);
	signal_connect(&self->component.signal_draw, self, pluginsview_ondraw);
	self->selectedplugin = 0;
	self->workspace = workspace;	
	self->dy = 0;	
	self->calledbygear = 0;
	signal_init(&self->signal_selected);
	signal_init(&self->signal_changed);
	signal_connect(&workspace->plugincatcher.signal_changed, self,
		pluginsview_onplugincachechanged);
}

void pluginsview_ondestroy(PluginsView* self, ui_component* component)
{
	signal_dispose(&self->signal_selected);
	signal_dispose(&self->signal_changed);
}

void pluginsview_ondraw(PluginsView* self, ui_component* sender,
	ui_graphics* g)
{	
	Properties* p;
	int cpx = 0;
	int cpy = 0;
	
	pluginsview_computetextsizes(self);
	p = workspace_pluginlist(self->workspace);
	if (p) {
		p = p->children;
	}
	for ( ; p != 0; p = p->next) {
		pluginsview_drawitem(self, g, p, cpx, cpy);
		cpx += self->columnwidth;
		if (cpx >= self->numparametercols * self->columnwidth) {
			cpx = 0;
			cpy += self->lineheight;
		}
	}	
}

void pluginsview_drawitem(PluginsView* self, ui_graphics* g,
	Properties* property, int x, int y)
{
	char txt[128];

	if (property == self->selectedplugin) {
		ui_setbackgroundcolor(g, 0x009B7800);
		ui_settextcolor(g, 0x00FFFFFF);		
	} else {
		ui_setbackgroundcolor(g, 0x00232323);
		ui_settextcolor(g, 0x00CACACA);		
	}		
	pluginname(property, txt);			
	ui_textout(g, x, y + self->dy + 2, txt, strlen(txt));
	plugintype(property, txt);
	ui_textout(g, x + self->columnwidth - 7 * self->avgcharwidth,
		y + self->dy + 2, txt, strlen(txt));	
	if (pluginmode(property, txt) & 3) {
		ui_settextcolor(g, 0x00B1C8B0);
	} else {		
		ui_settextcolor(g, 0x00D1C5B6);	
	}
	ui_textout(g, x + self->columnwidth - 10 * self->avgcharwidth,
		y + self->dy + 2, txt, strlen(txt));	
}

void pluginsview_computetextsizes(PluginsView* self)
{
	TEXTMETRIC tm;
	ui_size size;
	
	size = ui_component_size(&self->component);
	tm = ui_component_textmetric(&self->component);
	self->avgcharwidth = tm.tmAveCharWidth;
	self->lineheight = (int) (tm.tmHeight * 1.5);
	self->columnwidth = tm.tmAveCharWidth * 45;
	self->identwidth = tm.tmAveCharWidth * 4;
	self->numparametercols = max(1, size.width / self->columnwidth);
}

void pluginname(Properties* property, char* txt)
{
	Properties* p;
	
	p = properties_read(property, "name");	
	_snprintf(txt, 128, "%s", 
		p && properties_valuestring(p) != '\0'
		? properties_valuestring(p)
		: properties_key(property));	
}

int plugintype(Properties* property, char* txt)
{	
	int rv;
	
	rv = properties_int(property, "type", -1);
	switch (rv) {
		case MACH_PLUGIN:
			strcpy(txt, "psy");
		break;
		case MACH_VST:
			strcpy(txt, "vst");
		break;
		default:
			strcpy(txt, "int");
		break;
	}
	return rv;
}

int pluginmode(Properties* property, char* txt)
{			
	int rv;

	rv = properties_int(property, "mode", -1);
	strcpy(txt, rv & 3 ? "gn" : "fx");
	return rv;
}

void pluginsview_onsize(PluginsView* self, ui_component* sender, ui_size* size)
{
	pluginsview_adjustscroll(self);
}

void pluginsview_adjustscroll(PluginsView* self)
{
	Properties* p;

	p = workspace_pluginlist(self->workspace);
	if (p) {
		ui_size size;
		int visilines;
		int currlines;

		pluginsview_computetextsizes(self);
		size = ui_component_size(&self->component);		
		visilines = size.height / self->lineheight;
		currlines = properties_size(p) / self->numparametercols;
		self->component.scrollstepy = self->lineheight;
		ui_component_setverticalscrollrange(&self->component,
			0, currlines - visilines);
	}
}

void pluginsview_onscroll(PluginsView* self, ui_component* sender, 
	int cx, int cy)
{
	self->dy += cy;
}

void pluginsview_onmousedown(PluginsView* self, ui_component* sender,
	int x, int y, int button)
{
	pluginsview_hittest(self, x, y);
	ui_invalidate(&self->component);
	signal_emit(&self->signal_changed, self, 1, self->selectedplugin);
	ui_component_setfocus(&self->component);
}

void pluginsview_hittest(PluginsView* self, int x, int y)
{	
	Properties* p;
	int cpx = 0;
	int cpy = 0;
	
	pluginsview_computetextsizes(self);
	p = workspace_pluginlist(self->workspace);
	if (p) {			
		for (p = p->children ; p != 0; p = p->next) {
			ui_rectangle r;

			ui_setrectangle(&r, cpx, cpy, self->columnwidth, self->lineheight);
			if (ui_rectangle_intersect(&r, x, y - self->dy)) {
				self->selectedplugin = p;
				break;
			}		
			cpx += self->columnwidth;
			if (cpx >= self->numparametercols * self->columnwidth) {
				cpx = 0;
				cpy += self->lineheight;
			}
		}
	}
}

void pluginsview_onmousedoubleclick(PluginsView* self, ui_component* sender,
	int x, int y, int button)
{
	if (self->selectedplugin) {
		signal_emit(&self->signal_selected, self, 1, self->selectedplugin);
		self->calledbygear = 0;
	}	
}

void pluginsview_onkeydown(PluginsView* self, ui_component* sender, 
	int keycode, int keydata)
{
	if (keycode == VK_ESCAPE) {	
		self->component.propagateevent = 1;
	}
}

void pluginsview_onplugincachechanged(PluginsView* self, PluginCatcher* sender)
{
	self->dy = 0;
	self->selectedplugin = 0;
	pluginsview_adjustscroll(self);	
	ui_invalidate(&self->component);
}

void newmachine_init(NewMachine* self, ui_component* parent,
	Workspace* workspace)
{
	ui_component_init(&self->component, parent);
	ui_component_setbackgroundmode(&self->component, BACKGROUND_NONE);
	ui_component_enablealign(&self->component);	
	newmachinedetail_init(&self->detail, &self->component, workspace);
	ui_component_setalign(&self->detail.component, UI_ALIGN_LEFT);	
	pluginsview_init(&self->pluginsview, &self->component, workspace);
	ui_component_setalign(&self->pluginsview.component, UI_ALIGN_CLIENT);	
	signal_connect(&self->pluginsview.signal_changed, self,
		newmachine_onpluginselected);
	signal_connect(&workspace->plugincatcher.signal_changed, self,
		newmachine_onplugincachechanged);
}

void newmachine_onpluginselected(NewMachine* self, ui_component* parent,
	Properties* selected)
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

void newmachine_onplugincachechanged(NewMachine* self, PluginCatcher* sender)
{
	newmachinedetail_reset(&self->detail);
}

void newmachine_onkeydown(NewMachine* self, ui_component* sender,
	int keycode, int keydata)
{
	if (keycode == VK_ESCAPE) {	
		self->component.propagateevent = 1;
	}
}

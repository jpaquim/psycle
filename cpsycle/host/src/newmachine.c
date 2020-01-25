// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "newmachine.h"
#include <plugin_interface.h>
#include <qsort.h>
#include <string.h>
#include <stdlib.h>
#include "../../detail/portable.h"

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

// newmachine
static void newmachine_onpluginselected(NewMachine*, psy_ui_Component* parent,
	psy_Properties*);
static void newmachine_onplugincachechanged(NewMachine*, psy_audio_PluginCatcher*);
static void newmachine_onkeydown(NewMachine*, psy_ui_KeyEvent*);
static void newmachine_onsortbyname(NewMachine*, psy_ui_Component* sender);
static void newmachine_onsortbytype(NewMachine*, psy_ui_Component* sender);
static void newmachine_onsortbymode(NewMachine*, psy_ui_Component* sender);
// pluginsview
static void pluginsview_ondestroy(PluginsView*, psy_ui_Component* component);
static void pluginsview_ondraw(PluginsView*, psy_ui_Graphics*);
static void pluginsview_drawitem(PluginsView*, psy_ui_Graphics*, psy_Properties*,
	int x, int y);
static void pluginsview_onsize(PluginsView*, psy_ui_Component* sender,
	psy_ui_Size* size);
static void pluginsview_onmousedown(PluginsView*, psy_ui_MouseEvent*);
static void pluginsview_onmousedoubleclick(PluginsView*, psy_ui_MouseEvent*);
static void pluginsview_hittest(PluginsView*, int x, int y);
static void pluginsview_computetextsizes(PluginsView* self);
static void pluginsview_onscroll(PluginsView*, psy_ui_Component* sender,
	int stepx, int stepy);
static void pluginsview_onplugincachechanged(PluginsView*,
	psy_audio_PluginCatcher* sender);
static void pluginsview_adjustscroll(PluginsView*);

static void pluginname(psy_Properties*, char* text);
static int plugintype(psy_Properties*, char* text);
static int pluginmode(psy_Properties*, char* text);

static void newmachinebar_onrescan(NewMachineBar*, psy_ui_Component* sender);

static void newmachinedetail_reset(NewMachineDetail*);

psy_Properties* newmachine_sort(psy_Properties* source, psy_fp_comp);
static int newmachine_comp_name(psy_Properties* p, psy_Properties* q);
static int newmachine_comp_type(psy_Properties* p, psy_Properties* q);
static int newmachine_comp_mode(psy_Properties* p, psy_Properties* q);
static int newmachine_isplugin(int type);

void newmachinebar_init(NewMachineBar* self, psy_ui_Component* parent,
	Workspace* workspace)
{
	self->workspace = workspace;	
	psy_ui_component_init(&self->component, parent);
	psy_ui_component_enablealign(&self->component);	
	psy_ui_button_init(&self->rescan, &self->component);
	psy_ui_button_settext(&self->rescan, "Rescan");
	psy_ui_component_setalign(&self->rescan.component, psy_ui_ALIGN_TOP);
	psy_ui_button_init(&self->sortbyname, &self->component);
	psy_ui_button_settext(&self->sortbyname, "Sort By Name");
	psy_ui_component_setalign(&self->sortbyname.component, psy_ui_ALIGN_TOP);
	psy_ui_button_init(&self->sortbytype, &self->component);
	psy_ui_button_settext(&self->sortbytype, "Sort By Type");
	psy_ui_component_setalign(&self->sortbytype.component, psy_ui_ALIGN_TOP);
	psy_ui_button_init(&self->sortbymode, &self->component);
	psy_ui_button_settext(&self->sortbymode, "Sort By Mode");
	psy_ui_component_setalign(&self->sortbymode.component, psy_ui_ALIGN_TOP);
	psy_signal_connect(&self->rescan.signal_clicked, self,
		newmachinebar_onrescan);
}

void newmachinebar_onrescan(NewMachineBar* self, psy_ui_Component* sender)
{
	workspace_scanplugins(self->workspace);
}

void newmachinedetail_init(NewMachineDetail* self, psy_ui_Component* parent,
	Workspace* workspace)
{
	psy_ui_component_init(&self->component, parent);	
	newmachinebar_init(&self->bar, &self->component, workspace);
	psy_ui_component_setposition(&self->bar.component, 0, 10, 80, 100);
	psy_ui_label_init(&self->desclabel, &self->component);
	psy_ui_label_setstyle(&self->desclabel, WS_CHILD | WS_VISIBLE | SS_CENTER);
	psy_ui_label_settext(&self->desclabel, 
		"Select a plugin to view its description.");	
	psy_ui_component_setposition(&self->desclabel.component, 0, 110, 80, 100);	
}

void newmachinedetail_reset(NewMachineDetail* self)
{
	psy_ui_label_settext(&self->desclabel, 
		"Select a plugin to view its description.");	
}

// PluginsView
static psy_ui_ComponentVtable pluginsview_vtable;
static int pluginsview_vtable_initialized = 0;

static void pluginsview_vtable_init(PluginsView* self)
{
	if (!pluginsview_vtable_initialized) {
		pluginsview_vtable = *(self->component.vtable);				
		pluginsview_vtable.ondraw = (psy_ui_fp_ondraw) pluginsview_ondraw;		
		pluginsview_vtable.onmousedown = (psy_ui_fp_onmousedown)
			pluginsview_onmousedown;
		pluginsview_vtable.onmousedoubleclick = (psy_ui_fp_onmousedoubleclick)
			pluginsview_onmousedoubleclick;
	}
}

void pluginsview_init(PluginsView* self, psy_ui_Component* parent,
	Workspace* workspace)
{	
	psy_ui_component_init(&self->component, parent);
	pluginsview_vtable_init(self);
	self->workspace = workspace;
	if (workspace_pluginlist(workspace)) {
		self->plugins = psy_properties_clone(workspace_pluginlist(workspace), 1);
	} else {
		self->plugins = 0;
	}
	self->component.vtable = &pluginsview_vtable;
	psy_ui_component_doublebuffer(&self->component);
	psy_ui_component_showverticalscrollbar(&self->component);	
	psy_signal_connect(&self->component.signal_destroy, self,
		pluginsview_ondestroy);	
	psy_signal_connect(&self->component.signal_size, self,
		pluginsview_onsize);
	psy_signal_connect(&self->component.signal_scroll, self,
		pluginsview_onscroll);	
	self->selectedplugin = 0;	
	self->dy = 0;	
	self->calledby = 0;
	psy_signal_init(&self->signal_selected);
	psy_signal_init(&self->signal_changed);
	psy_signal_connect(&workspace->plugincatcher.signal_changed, self,
		pluginsview_onplugincachechanged);
}

void pluginsview_ondestroy(PluginsView* self, psy_ui_Component* component)
{
	psy_signal_dispose(&self->signal_selected);
	psy_signal_dispose(&self->signal_changed);
	if (self->plugins) {
		properties_free(self->plugins);
	}
}

void pluginsview_ondraw(PluginsView* self, psy_ui_Graphics* g)
{	
	psy_Properties* p;
	int cpx = 0;
	int cpy = 0;
	
	pluginsview_computetextsizes(self);
	p = self->plugins;
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

void pluginsview_drawitem(PluginsView* self, psy_ui_Graphics* g,
	psy_Properties* property, int x, int y)
{
	char text[128];

	if (property == self->selectedplugin) {
		psy_ui_setbackgroundcolor(g, 0x009B7800);
		psy_ui_settextcolor(g, 0x00FFFFFF);		
	} else {
		psy_ui_setbackgroundcolor(g, 0x00232323);
		psy_ui_settextcolor(g, 0x00CACACA);		
	}		
	pluginname(property, text);			
	psy_ui_textout(g, x, y + self->dy + 2, text, strlen(text));
	plugintype(property, text);
	psy_ui_textout(g, x + self->columnwidth - 7 * self->avgcharwidth,
		y + self->dy + 2, text, strlen(text));
	if (pluginmode(property, text) == MACHMODE_FX) {
		psy_ui_settextcolor(g, 0x00B1C8B0);
	} else {		
		psy_ui_settextcolor(g, 0x00D1C5B6);	
	}
	psy_ui_textout(g, x + self->columnwidth - 10 * self->avgcharwidth,
		y + self->dy + 2, text, strlen(text));	
}

void pluginsview_computetextsizes(PluginsView* self)
{
	psy_ui_TextMetric tm;
	psy_ui_Size size;
	
	size = psy_ui_component_size(&self->component);
	tm = psy_ui_component_textmetric(&self->component);
	self->avgcharwidth = tm.tmAveCharWidth;
	self->lineheight = (int) (tm.tmHeight * 1.5);
	self->columnwidth = tm.tmAveCharWidth * 45;
	self->identwidth = tm.tmAveCharWidth * 4;
	self->numparametercols = max(1, size.width / self->columnwidth);
}

void pluginname(psy_Properties* property, char* text)
{
	psy_Properties* p;
	
	p = psy_properties_read(property, "name");	
	psy_snprintf(text, 128, "%s", 
		p && strlen(psy_properties_valuestring(p)) != 0
		? psy_properties_valuestring(p)
		: psy_properties_key(property));	
}

int plugintype(psy_Properties* property, char* text)
{	
	int rv;
	
	rv = psy_properties_int(property, "type", -1);
	switch (rv) {
		case MACH_PLUGIN:
			strcpy(text, "psy");
		break;
		case MACH_LUA:
			strcpy(text, "lua");
		break;
		case MACH_VST:
			strcpy(text, "vst");
		break;
		case MACH_VSTFX:
			strcpy(text, "vst");
		break;
		default:
			strcpy(text, "int");
		break;
	}
	return rv;
}

int pluginmode(psy_Properties* property, char* text)
{			
	int rv;

	rv = psy_properties_int(property, "mode", -1);
	strcpy(text, rv == MACHMODE_FX ? "fx" : "gn");
	return rv;
}

void pluginsview_onsize(PluginsView* self, psy_ui_Component* sender,
	psy_ui_Size* size)
{
	pluginsview_adjustscroll(self);
}

void pluginsview_adjustscroll(PluginsView* self)
{
	psy_Properties* p;

	p = self->plugins;
	if (p) {
		psy_ui_Size size;
		int visilines;
		int currlines;

		pluginsview_computetextsizes(self);
		size = psy_ui_component_size(&self->component);
		visilines = size.height / self->lineheight;
		currlines = (int) (psy_properties_size(p) /
			(float) self->numparametercols + 0.5f);
		self->component.scrollstepy = self->lineheight;
		psy_ui_component_setverticalscrollrange(&self->component,
			0, currlines - visilines);
	}
}

void pluginsview_onscroll(PluginsView* self, psy_ui_Component* sender, 
	int stepx, int stepy)
{
	self->dy += sender->scrollstepy * stepy;
}

void pluginsview_onmousedown(PluginsView* self, psy_ui_MouseEvent* ev)
{
	pluginsview_hittest(self, ev->x, ev->y);
	psy_ui_component_invalidate(&self->component);
	psy_signal_emit(&self->signal_changed, self, 1, self->selectedplugin);
	psy_ui_component_setfocus(&self->component);
}

void pluginsview_hittest(PluginsView* self, int x, int y)
{	
	psy_Properties* p;
	int cpx = 0;
	int cpy = 0;
	
	pluginsview_computetextsizes(self);
	p = self->plugins;
	if (p) {			
		for (p = p->children ; p != 0; p = p->next) {
			psy_ui_Rectangle r;

			psy_ui_setrectangle(&r, cpx, cpy, self->columnwidth,
				self->lineheight);
			if (psy_ui_rectangle_intersect(&r, x, y - self->dy)) {
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

void pluginsview_onmousedoubleclick(PluginsView* self, psy_ui_MouseEvent* ev)
{
	if (self->selectedplugin) {
		psy_signal_emit(&self->signal_selected, self, 1,
			self->selectedplugin);
		workspace_selectview(self->workspace, self->calledby);
		psy_ui_mouseevent_stoppropagation(ev);
	}	
}

void pluginsview_onplugincachechanged(PluginsView* self,
	psy_audio_PluginCatcher* sender)
{
	self->dy = 0;
	self->selectedplugin = 0;
	if (self->plugins) {
		properties_free(self->plugins);
	}
	if (sender->plugins) {		
		self->plugins = psy_properties_clone(sender->plugins, 1);
	} else {
		self->plugins = 0;
	}	
	pluginsview_adjustscroll(self);
	psy_ui_component_invalidate(&self->component);
}

// NewMachine
static psy_ui_ComponentVtable newmachine_vtable;
static int newmachine_vtable_initialized = 0;

static void newmachine_vtable_init(NewMachine* self)
{
	if (!newmachine_vtable_initialized) {
		newmachine_vtable = *(self->component.vtable);				
		newmachine_vtable.onkeydown = (psy_ui_fp_onkeydown)
			newmachine_onkeydown;		
	}
}

void newmachine_init(NewMachine* self, psy_ui_Component* parent,
	Workspace* workspace)
{
	psy_ui_component_init(&self->component, parent);
	newmachine_vtable_init(self);
	self->component.vtable = &newmachine_vtable;
	psy_ui_component_setbackgroundmode(&self->component, BACKGROUND_NONE);
	psy_ui_component_enablealign(&self->component);	
	newmachinedetail_init(&self->detail, &self->component, workspace);
	psy_ui_component_setalign(&self->detail.component, psy_ui_ALIGN_LEFT);	
	pluginsview_init(&self->pluginsview, &self->component, workspace);
	psy_ui_component_setalign(&self->pluginsview.component, psy_ui_ALIGN_CLIENT);	
	psy_signal_connect(&self->pluginsview.signal_changed, self,
		newmachine_onpluginselected);
	psy_signal_connect(&workspace->plugincatcher.signal_changed, self,
		newmachine_onplugincachechanged);
	psy_signal_connect(&self->detail.bar.sortbyname.signal_clicked, self,
		newmachine_onsortbyname);
	psy_signal_connect(&self->detail.bar.sortbytype.signal_clicked, self,
		newmachine_onsortbytype);
	psy_signal_connect(&self->detail.bar.sortbymode.signal_clicked, self,
		newmachine_onsortbymode);
}

void newmachine_onpluginselected(NewMachine* self, psy_ui_Component* parent,
	psy_Properties* selected)
{
	const char* text;
	char detail[1024];

	text = psy_properties_readstring(selected, "name", "");
	strcpy(detail, text);
	text = psy_properties_readstring(selected, "desc", "");
	strcat(detail, "  ");
	strcat(detail, text);
	psy_ui_label_settext(&self->detail.desclabel, detail);
}

void newmachine_onplugincachechanged(NewMachine* self,
	psy_audio_PluginCatcher* sender)
{
	newmachinedetail_reset(&self->detail);
}

void newmachine_onsortbyname(NewMachine* self, psy_ui_Component* sender)
{
	psy_Properties* sorted;
	
	if (self->pluginsview.plugins) {
		sorted = newmachine_sort(self->pluginsview.plugins,
			newmachine_comp_name);
		properties_free(self->pluginsview.plugins);
		self->pluginsview.plugins = sorted;
		newmachinedetail_reset(&self->detail);
		self->pluginsview.dy = 0;
		pluginsview_adjustscroll(&self->pluginsview);
		psy_ui_component_invalidate(&self->pluginsview.component);
	}
}

void newmachine_onsortbytype(NewMachine* self, psy_ui_Component* parent)
{
	psy_Properties* sorted;
	
	if (self->pluginsview.plugins) {
		sorted = newmachine_sort(self->pluginsview.plugins,
			newmachine_comp_type);
		properties_free(self->pluginsview.plugins);
		self->pluginsview.plugins = sorted;
		newmachinedetail_reset(&self->detail);
		self->pluginsview.dy = 0;
		pluginsview_adjustscroll(&self->pluginsview);
		psy_ui_component_invalidate(&self->pluginsview.component);
	}
}

void newmachine_onsortbymode(NewMachine* self, psy_ui_Component* parent)
{
	psy_Properties* sorted;
	
	if (self->pluginsview.plugins) {
		sorted = newmachine_sort(self->pluginsview.plugins,
			newmachine_comp_mode);
		properties_free(self->pluginsview.plugins);
		self->pluginsview.plugins = sorted;
		newmachinedetail_reset(&self->detail);
		self->pluginsview.dy = 0;
		pluginsview_adjustscroll(&self->pluginsview);
		psy_ui_component_invalidate(&self->pluginsview.component);
	}
}

void newmachine_onkeydown(NewMachine* self, psy_ui_KeyEvent* ev)
{
	if (ev->keycode != psy_ui_KEY_ESCAPE) {	
		psy_ui_keyevent_stoppropagation(ev);
	}
}

psy_Properties* newmachine_sort(psy_Properties* source, psy_fp_comp comp)
{		
	psy_Properties* rv = 0;

	if (source) {
		int i;
		int num;
		psy_Properties* p;
		psy_Properties** propertiesptr;
		
		p = source;
		num = psy_properties_size(p);
		propertiesptr = malloc(sizeof(psy_Properties*) * num);
		p = p->children;		
		for (i =0 ; p != 0 && i < num; p = p->next, ++i) {
			propertiesptr[i] = p;
		}		
		psy_qsort(propertiesptr, 0, num - 1, comp);

		rv = psy_properties_create();		
		for (i = 0; i < num; ++i) {
			psy_properties_append_property(rv, psy_properties_clone(
				propertiesptr[i], 0));
		}		
		free(propertiesptr);
	}
	return rv;
}

int newmachine_comp_name(psy_Properties* p, psy_Properties* q)
{
	const char* left;
	const char* right;

	left = psy_properties_readstring(p, "name", "");
	if (strlen(left) == 0) {
		left = psy_properties_key(p);
	}
	right = psy_properties_readstring(q, "name", "");
	if (strlen(right) == 0) {
		right = psy_properties_key(q);
	}
	return strcmp(left, right);		
}

int newmachine_comp_type(psy_Properties* p, psy_Properties* q)
{
	int left;
	int right;
	
	left = psy_properties_int(p, "type", 128);
	left = newmachine_isplugin(left) ? left : 0;
	right = psy_properties_int(q, "type", 128);
	right = newmachine_isplugin(right) ? right : 0;
	return left - right;		
}

int newmachine_isplugin(int type)
{
	return (type == MACH_PLUGIN) ||
	   (type == MACH_VST) ||
	   (type == MACH_VSTFX) ||
	   (type == MACH_LUA) ||
	   (type == MACH_LADSPA);
}

int newmachine_comp_mode(psy_Properties* p, psy_Properties* q)
{	
	return psy_properties_int(p, "mode", 128) -
		psy_properties_int(q, "mode", 128);	
}

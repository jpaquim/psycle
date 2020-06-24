// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "newmachine.h"
#include <plugin_interface.h>
#include <qsort.h>
#include <string.h>
#include <stdlib.h>
#include "../../detail/portable.h"

// newmachine
static void newmachine_onpluginselected(NewMachine*, psy_ui_Component* parent,
	psy_Properties*);
static void newmachine_onplugincachechanged(NewMachine*, psy_audio_PluginCatcher*);
static void newmachine_onkeydown(NewMachine*, psy_ui_KeyEvent*);
static void newmachine_onsortbyname(NewMachine*, psy_ui_Component* sender);
static void newmachine_onsortbytype(NewMachine*, psy_ui_Component* sender);
static void newmachine_onsortbymode(NewMachine*, psy_ui_Component* sender);
static void newmachine_onfocus(NewMachine*, psy_ui_Component* sender);

static void plugindisplayname(psy_Properties*, char* text);
static int plugintype(psy_Properties*, char* text);
static int pluginmode(psy_Properties*, char* text);

static void newmachinebar_onrescan(NewMachineBar*, psy_ui_Component* sender);
static void newmachinebar_onselectdirectories(NewMachineBar*, psy_ui_Component* sender);

static void newmachinedetail_reset(NewMachineDetail*);

psy_Properties* newmachine_sort(psy_Properties* source, psy_fp_comp);
static int newmachine_comp_name(psy_Properties* p, psy_Properties* q);
static int newmachine_comp_type(psy_Properties* p, psy_Properties* q);
static int newmachine_comp_mode(psy_Properties* p, psy_Properties* q);
static int newmachine_isplugin(int type);
static void newmachinedetail_onloadnewblitz(NewMachineDetail*, psy_ui_Component* sender);

void newmachinebar_init(NewMachineBar* self, psy_ui_Component* parent,
	Workspace* workspace)
{
	psy_ui_Margin margin;
			
	psy_ui_component_init(&self->component, parent);
	psy_ui_component_enablealign(&self->component);
	self->workspace = workspace;
	psy_ui_button_init(&self->rescan, &self->component);
	psy_ui_button_settext(&self->rescan, "Rescan");
	psy_ui_button_setcharnumber(&self->rescan, 30);
	psy_ui_button_init(&self->selectdirectories, &self->component);
	psy_ui_button_settext(&self->selectdirectories, "Select plugin directories");
	psy_ui_button_setcharnumber(&self->rescan, 30);
	psy_ui_button_init(&self->sortbyname, &self->component);
	psy_ui_button_settext(&self->sortbyname, "Sort By Name");	
	psy_ui_button_init(&self->sortbytype, &self->component);
	psy_ui_button_settext(&self->sortbytype, "Sort By Type");	
	psy_ui_button_init(&self->sortbymode, &self->component);
	psy_ui_button_settext(&self->sortbymode, "Sort By Mode");	
	psy_signal_connect(&self->rescan.signal_clicked, self,
		newmachinebar_onrescan);
	psy_signal_connect(&self->selectdirectories.signal_clicked, self,
		newmachinebar_onselectdirectories);
	psy_ui_margin_init_all(&margin, psy_ui_value_makepx(0),
		psy_ui_value_makepx(0), psy_ui_value_makeeh(0.5),
		psy_ui_value_makepx(0));
	psy_list_free(psy_ui_components_setalign(
		psy_ui_component_children(&self->component, psy_ui_NONRECURSIVE),
		psy_ui_ALIGN_TOP,
		&margin));	
}

void newmachinebar_onrescan(NewMachineBar* self, psy_ui_Component* sender)
{
	workspace_scanplugins(self->workspace);
}

void newmachinebar_onselectdirectories(NewMachineBar* self, psy_ui_Component* sender)
{
	workspace_selectview(self->workspace, TABPAGE_SETTINGSVIEW, 3, 0);
}

void newmachinedetail_init(NewMachineDetail* self, psy_ui_Component* parent,
	Workspace* workspace)
{
	psy_ui_Margin margin;

	psy_ui_component_init(&self->component, parent);
	psy_ui_component_enablealign(&self->component);
	self->workspace = workspace;
	newmachinebar_init(&self->bar, &self->component, workspace);
	psy_ui_component_setalign(&self->bar.component, psy_ui_ALIGN_TOP);
	psy_ui_label_init(&self->desclabel, &self->component);
	psy_ui_label_settextalignment(&self->desclabel, psy_ui_ALIGNMENT_CENTER_HORIZONTAL);
	psy_ui_label_settext(&self->desclabel,
		"Select a plugin to view its description.");
	psy_ui_component_setalign(&self->desclabel.component, psy_ui_ALIGN_CLIENT);
	psy_ui_checkbox_init(&self->compatblitzgamefx, &self->component);
	psy_ui_checkbox_settext(&self->compatblitzgamefx,
		"Load new gamefx and Blitz if version unknown");
	if (workspace_loadnewblitz(self->workspace)) {
		psy_ui_checkbox_check(&self->compatblitzgamefx);
	}
	psy_signal_connect(&self->compatblitzgamefx.signal_clicked, self,
		newmachinedetail_onloadnewblitz);
	psy_ui_component_setalign(&self->compatblitzgamefx.component, psy_ui_ALIGN_BOTTOM);
	psy_ui_label_init(&self->compatlabel, &self->component);
	psy_ui_label_settextalignment(&self->compatlabel, psy_ui_ALIGNMENT_LEFT);
	psy_ui_label_settext(&self->compatlabel,
		"Song loading compatibility");	
	psy_ui_component_setalign(&self->compatlabel.component, psy_ui_ALIGN_BOTTOM);
	psy_ui_margin_init_all(&margin, psy_ui_value_makepx(0),
		psy_ui_value_makeew(2), psy_ui_value_makeeh(1.5),
		psy_ui_value_makepx(0));
	psy_list_free(psy_ui_components_setmargin(
		psy_ui_component_children(&self->component, 0),
		&margin));
}

void newmachinedetail_reset(NewMachineDetail* self)
{
	psy_ui_label_settext(&self->desclabel, 
		"Select a plugin to view its description.");	
}

void newmachinedetail_onloadnewblitz(NewMachineDetail* self, psy_ui_Component* sender)
{
	if (psy_ui_checkbox_checked(&self->compatblitzgamefx)) {
		workspace_setloadnewblitz(self->workspace, 1);
	} else {
		workspace_setloadnewblitz(self->workspace, 0);
	}
}

// pluginsview
static void pluginsview_ondestroy(PluginsView*, psy_ui_Component* component);
static void pluginsview_ondraw(PluginsView*, psy_ui_Graphics*);
static void pluginsview_drawitem(PluginsView*, psy_ui_Graphics*, psy_Properties*,
	int x, int y);
static void pluginsview_onpreferredsize(PluginsView* self, const psy_ui_Size* limit,
	psy_ui_Size* rv);
static void pluginsview_onkeydown(PluginsView*, psy_ui_KeyEvent*);
static void pluginsview_cursorposition(PluginsView*, psy_Properties* plugin,
	int* col, int* row);
static psy_Properties* pluginsview_pluginbycursorposition(PluginsView*,
	int col, int row);
static void pluginsview_onmousedown(PluginsView*, psy_ui_MouseEvent*);
static void pluginsview_onmousedoubleclick(PluginsView*, psy_ui_MouseEvent*);
static void pluginsview_hittest(PluginsView*, int x, int y);
static void pluginsview_computetextsizes(PluginsView*);
static void pluginsview_onplugincachechanged(PluginsView*,
	psy_audio_PluginCatcher* sender);

static psy_ui_ComponentVtable pluginsview_vtable;
static int pluginsview_vtable_initialized = 0;

static void pluginsview_vtable_init(PluginsView* self)
{
	if (!pluginsview_vtable_initialized) {
		pluginsview_vtable = *(self->component.vtable);				
		pluginsview_vtable.ondraw = (psy_ui_fp_ondraw) pluginsview_ondraw;		
		pluginsview_vtable.onkeydown = (psy_ui_fp_onkeydown)
			pluginsview_onkeydown;
		pluginsview_vtable.onmousedown = (psy_ui_fp_onmousedown)
			pluginsview_onmousedown;
		pluginsview_vtable.onmousedoubleclick = (psy_ui_fp_onmousedoubleclick)
			pluginsview_onmousedoubleclick;
		pluginsview_vtable.onpreferredsize = (psy_ui_fp_onpreferredsize)
			pluginsview_onpreferredsize;		
	}
}

void pluginsview_init(PluginsView* self, psy_ui_Component* parent,
	Workspace* workspace)
{	
	psy_ui_component_init(&self->component, parent);
	pluginsview_vtable_init(self);
	self->component.vtable = &pluginsview_vtable;
	self->workspace = workspace;
	if (workspace_pluginlist(workspace)) {
		self->plugins = psy_properties_clone(workspace_pluginlist(workspace), 1);
	} else {
		self->plugins = 0;
	}	
	psy_ui_component_doublebuffer(&self->component);
	psy_ui_component_showverticalscrollbar(&self->component);
	psy_ui_component_setwheelscroll(&self->component, 4);
	psy_signal_connect(&self->component.signal_destroy, self,
		pluginsview_ondestroy);
	self->selectedplugin = 0;
	self->calledby = 0;
	psy_signal_init(&self->signal_selected);
	psy_signal_init(&self->signal_changed);
	psy_signal_connect(&workspace->plugincatcher.signal_changed, self,
		pluginsview_onplugincachechanged);
	pluginsview_computetextsizes(self);
	self->component.overflow = psy_ui_OVERFLOW_VSCROLL;
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
	for ( ; p != NULL; p = psy_properties_next(p)) {
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
	plugindisplayname(property, text);	
	psy_ui_textout(g, x, y + 2, text, strlen(text));
	plugintype(property, text);
	psy_ui_textout(g, x + self->columnwidth - 7 * self->avgcharwidth,
		y + 2, text, strlen(text));
	if (pluginmode(property, text) == MACHMODE_FX) {
		psy_ui_settextcolor(g, 0x00B1C8B0);
	} else {		
		psy_ui_settextcolor(g, 0x00D1C5B6);	
	}
	psy_ui_textout(g, x + self->columnwidth - 10 * self->avgcharwidth,
		y + 2, text, strlen(text));
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
	self->numparametercols = max(1, psy_ui_value_px(&size.width, &tm) / self->columnwidth);
	self->component.scrollstepy = self->lineheight;
}

void plugindisplayname(psy_Properties* property, char* text)
{	
	const char* label;

	label = psy_properties_readstring(property, "shortname", "");
	if (strcmp(label, "") == 0) {
		label = psy_properties_key(property);
	}
	psy_snprintf(text, 128, "%s", label);
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
		case MACH_LADSPA:
			strcpy(text, "lad");
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

void pluginsview_onpreferredsize(PluginsView* self, const psy_ui_Size* limit,
	psy_ui_Size* rv)
{
	if (self->plugins) {
		psy_ui_Size size;
		int currlines;

		pluginsview_computetextsizes(self);
		size = psy_ui_component_size(&self->component);
		currlines = (int)(psy_properties_size(self->plugins) /
			(float)self->numparametercols + 0.5f);
		rv->height = psy_ui_value_makepx(self->component.scrollstepy * currlines);
		rv->width = size.width;		
	}
}

void pluginsview_onkeydown(PluginsView* self, psy_ui_KeyEvent* ev)
{
	if (self->selectedplugin) {
		psy_Properties* plugin;
		int col;
		int row;

		col = 0;
		row = 0;
		plugin = NULL;
		pluginsview_cursorposition(self, self->selectedplugin, &col, &row);
		switch (ev->keycode) {
			case psy_ui_KEY_RETURN:
				if (self->selectedplugin) {
					psy_signal_emit(&self->signal_selected, self, 1,
						self->selectedplugin);
					workspace_selectview(self->workspace, self->calledby, 0, 0);
					psy_ui_keyevent_stoppropagation(ev);
				}
				break;
			case psy_ui_KEY_DOWN: 
				++row;
				psy_ui_keyevent_stoppropagation(ev);
				break;		
			case psy_ui_KEY_UP:
				if (row > 0) {
					--row;				
				}
				psy_ui_keyevent_stoppropagation(ev);
				break;		
			case psy_ui_KEY_LEFT:			
				if (col > 0) {
					--col;		
				}
				psy_ui_keyevent_stoppropagation(ev);
				break;
			case psy_ui_KEY_RIGHT: {					
				++col;				
				psy_ui_keyevent_stoppropagation(ev);
				break;
			}
			default:			
				break;
		}		
		plugin = pluginsview_pluginbycursorposition(self, col, row);
		if (plugin) {
			self->selectedplugin = plugin;
			psy_signal_emit(&self->signal_changed, self, 1,
				self->selectedplugin);
			psy_ui_component_invalidate(&self->component);
		}
	} else
	if (ev->keycode >= psy_ui_KEY_LEFT && ev->keycode <= psy_ui_KEY_DOWN) {
		if (self->plugins->children)
		self->selectedplugin = self->plugins->children;
		psy_signal_emit(&self->signal_changed, self, 1,
			self->selectedplugin);
		psy_ui_component_invalidate(&self->component);			
	}	
}

void pluginsview_cursorposition(PluginsView* self, psy_Properties* plugin,
	int* col, int* row)
{		
	*col = 0;
	*row = 0;
	if (plugin) {
		psy_Properties* p;
		pluginsview_computetextsizes(self);
		p = self->plugins;
		if (p) {
			for (p = p->children; p != NULL; p = p->next) {
				if (p == plugin) {
					break;
				}
				++(*col);
				if (*col >= self->numparametercols) {
					*col = 0;
					++(*row);
				}
			}
		}
	}	
}

psy_Properties* pluginsview_pluginbycursorposition(PluginsView* self, int col, int row)
{		
	psy_Properties* rv;	
	int currcol;
	int currrow;

	rv = NULL;
	currcol = 0;
	currrow = 0;
	pluginsview_computetextsizes(self);	
	if (self->plugins) {
		psy_Properties* p;

		p = self->plugins;
		for (p = p->children; p != NULL; p = p->next) {
			if (currcol == col && currrow == row) {
				rv = p;
				break;
			}
			++currcol;
			if (currcol >= self->numparametercols) {
				currcol = 0;
				++currrow;
			}
		}		
	}
	return rv;
}


void pluginsview_onmousedown(PluginsView* self, psy_ui_MouseEvent* ev)
{
	if (ev->button == 1) {
		psy_ui_component_setfocus(&self->component);
		pluginsview_hittest(self, ev->x, ev->y);
		psy_ui_component_invalidate(&self->component);
		psy_signal_emit(&self->signal_changed, self, 1,
			self->selectedplugin);
		psy_ui_component_setfocus(&self->component);
		psy_ui_mouseevent_stoppropagation(ev);
	}
}

void pluginsview_hittest(PluginsView* self, int x, int y)
{	
	psy_Properties* p;
	int cpx = 0;
	int cpy = 0;
	
	pluginsview_computetextsizes(self);
	p = self->plugins;
	if (p) {			
		for (p = p->children ; p != NULL; p = p->next) {
			psy_ui_Rectangle r;

			psy_ui_setrectangle(&r, cpx, cpy, self->columnwidth,
				self->lineheight);
			if (psy_ui_rectangle_intersect(&r, x, y)) {
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
		workspace_selectview(self->workspace, self->calledby, 0, 0);
		psy_ui_mouseevent_stoppropagation(ev);
	}	
}

void pluginsview_onplugincachechanged(PluginsView* self,
	psy_audio_PluginCatcher* sender)
{
	psy_ui_component_setscrolltop(&self->component, 0);
	self->selectedplugin = 0;
	if (self->plugins) {
		properties_free(self->plugins);
	}
	if (sender->plugins) {		
		self->plugins = psy_properties_clone(sender->plugins, 1);
	} else {
		self->plugins = 0;
	}
	psy_ui_component_setscrolltop(&self->component, 0);	
	psy_ui_component_updateoverflow(&self->component);
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
	psy_ui_component_setbackgroundmode(&self->component,
		psy_ui_BACKGROUND_NONE);
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
	psy_signal_connect(&self->component.signal_focus, self,
		newmachine_onfocus);
}

void newmachine_onpluginselected(NewMachine* self, psy_ui_Component* parent,
	psy_Properties* selected)
{
	const char* text;
	char detail[1024];

	text = psy_properties_readstring(selected, "name", "");
	strcpy(detail, text);
	// text = psy_properties_readstring(selected, "desc", "");
	// strcat(detail, "  ");
	// strcat(detail, text);	
	text = psy_properties_readstring(selected, "author", "");
	strcat(detail, "\n(");
	strcat(detail, text);
	strcat(detail, ")");
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
		psy_ui_component_setscrolltop(&self->pluginsview.component, 0);
		psy_ui_component_updateoverflow(&self->pluginsview.component);
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
		psy_ui_component_setscrolltop(&self->pluginsview.component, 0);
		psy_ui_component_updateoverflow(&self->pluginsview.component);
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
		psy_ui_component_setscrolltop(&self->pluginsview.component, 0);
		psy_ui_component_updateoverflow(&self->pluginsview.component);
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
		for (i =0 ; p != NULL && i < num; p = p->next, ++i) {
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

void newmachine_onfocus(NewMachine* self, psy_ui_Component* sender)
{
	psy_ui_component_setfocus(&self->pluginsview.component);
}

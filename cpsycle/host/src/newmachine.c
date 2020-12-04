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
static void plugindisplayname(psy_Property*, char* text);
static int plugintype(psy_Property*, char* text);
static int pluginmode(psy_Property*, char* text);

static void newmachinebar_onrescan(NewMachineBar*, psy_ui_Component* sender);
static void newmachinebar_onselectdirectories(NewMachineBar*, psy_ui_Component* sender);

psy_Property* newmachine_sort(psy_Property* source, psy_fp_comp);
psy_Property* newmachine_favorites(psy_Property* source);
static int newmachine_comp_favorite(psy_Property* p, psy_Property* q);
static int newmachine_comp_name(psy_Property* p, psy_Property* q);
static int newmachine_comp_type(psy_Property* p, psy_Property* q);
static int newmachine_comp_mode(psy_Property* p, psy_Property* q);
static int newmachine_isplugin(int type);

static void newmachinebar_updatetext(NewMachineBar*, psy_Translator*);
static void newmachinebar_onlanguagechanged(NewMachineBar*, psy_Translator* sender);

void newmachinebar_init(NewMachineBar* self, psy_ui_Component* parent,
	Workspace* workspace)
{
	psy_ui_Margin margin;
			
	psy_ui_component_init(&self->component, parent);	
	self->workspace = workspace;
	psy_ui_button_init(&self->rescan, &self->component);
	psy_ui_button_setcharnumber(&self->rescan, 30);
	psy_ui_button_init(&self->selectdirectories, &self->component);
	psy_ui_button_setcharnumber(&self->rescan, 30);
	psy_ui_button_init(&self->sortbyfavorite, &self->component);
	psy_ui_button_init(&self->sortbyname, &self->component);
	psy_ui_button_init(&self->sortbytype, &self->component);
	psy_ui_button_init(&self->sortbymode, &self->component);
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
	newmachinebar_updatetext(self, workspace_translator(workspace));
	psy_signal_connect(&workspace->signal_languagechanged, self,
		newmachinebar_onlanguagechanged);
}

void newmachinebar_updatetext(NewMachineBar* self, psy_Translator* translator)
{
	psy_ui_button_settext(&self->rescan,
		psy_translator_translate(translator, "newmachine.rescan"));
	psy_ui_button_settext(&self->selectdirectories,
		psy_translator_translate(translator, "newmachine.select-plugin-directories"));
	psy_ui_button_settext(&self->sortbyfavorite,
		psy_translator_translate(translator, "newmachine.sort-by-favorite"));
	psy_ui_button_settext(&self->sortbyname,
		psy_translator_translate(translator, "newmachine.sort-by-name"));
	psy_ui_button_settext(&self->sortbytype,
		psy_translator_translate(translator, "newmachine.sort-by-type"));
	psy_ui_button_settext(&self->sortbymode,
		psy_translator_translate(translator, "newmachine.sort-by-mode"));
}

void newmachinebar_onlanguagechanged(NewMachineBar* self, psy_Translator* sender)
{
	newmachinebar_updatetext(self, sender);
}

void newmachinebar_onrescan(NewMachineBar* self, psy_ui_Component* sender)
{
	workspace_scanplugins(self->workspace);
}

void newmachinebar_onselectdirectories(NewMachineBar* self, psy_ui_Component* sender)
{
	workspace_selectview(self->workspace, TABPAGE_SETTINGSVIEW, 3, 0);
}

// NewMachineDetail
static void newmachinedetail_reset(NewMachineDetail*);
static void newmachinedetail_updatetext(NewMachineDetail*, psy_Translator*);
static void newmachinedetail_onlanguagechanged(NewMachineDetail*, psy_Translator* sender);
static void newmachinedetail_onloadnewblitz(NewMachineDetail*, psy_ui_Component* sender);

void newmachinedetail_init(NewMachineDetail* self, psy_ui_Component* parent,
	Workspace* workspace)
{
	psy_ui_Margin margin;

	psy_ui_component_init(&self->component, parent);	
	self->workspace = workspace;
	newmachinebar_init(&self->bar, &self->component, workspace);
	psy_ui_component_setalign(&self->bar.component, psy_ui_ALIGN_TOP);
	psy_ui_label_init_text(&self->desclabel, &self->component,
		"Select a plugin to view its description");
	psy_ui_label_settextalignment(&self->desclabel, psy_ui_ALIGNMENT_CENTER_HORIZONTAL);	
	psy_ui_component_setalign(&self->desclabel.component, psy_ui_ALIGN_CLIENT);
	psy_ui_checkbox_init(&self->compatblitzgamefx, &self->component);
	//psy_ui_component_setmaximumsize(&self->compatblitzgamefx.component,
	//	psy_ui_size_make(psy_ui_value_makeew(10),
	//	psy_ui_value_makeeh(0)));
	if (workspace_loadnewblitz(self->workspace)) {
		psy_ui_checkbox_check(&self->compatblitzgamefx);
	}
	psy_signal_connect(&self->compatblitzgamefx.signal_clicked, self,
		newmachinedetail_onloadnewblitz);
	psy_ui_component_setalign(&self->compatblitzgamefx.component, psy_ui_ALIGN_BOTTOM);
	psy_ui_label_init(&self->compatlabel, &self->component);
	psy_ui_label_settextalignment(&self->compatlabel, psy_ui_ALIGNMENT_LEFT);	
	psy_ui_component_setalign(&self->compatlabel.component, psy_ui_ALIGN_BOTTOM);
	psy_ui_margin_init_all(&margin, psy_ui_value_makepx(0),
		psy_ui_value_makeew(2), psy_ui_value_makeeh(1.5),
		psy_ui_value_makepx(0));
	psy_list_free(psy_ui_components_setmargin(
		psy_ui_component_children(&self->component, 0),
		&margin));
	newmachinedetail_updatetext(self, workspace_translator(workspace));
	psy_signal_connect(&workspace->signal_languagechanged, self,
		newmachinedetail_onlanguagechanged);
}

void newmachinedetail_updatetext(NewMachineDetail* self, psy_Translator* translator)
{	
	if (self->empty) {
		psy_ui_label_settext(&self->desclabel,
			psy_translator_translate(translator,
				"newmachine.select-plugin-to-view-description"));
	}
	psy_ui_checkbox_settext(&self->compatblitzgamefx,
		psy_translator_translate(translator,
			"newmachine.jme-version-unknown"));
	psy_ui_label_settext(&self->compatlabel,
		psy_translator_translate(translator,
			"newmachine.song-loading-compatibility"));
}

void newmachinedetail_onlanguagechanged(NewMachineDetail* self, psy_Translator* sender)
{
	newmachinedetail_updatetext(self, sender);
}

void newmachinedetail_reset(NewMachineDetail* self)
{
	psy_ui_label_settext(&self->desclabel,
		workspace_translate(self->workspace,
			"newmachine.select-plugin-to-view-description"));
	self->empty = TRUE;
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
static void pluginsview_drawitem(PluginsView*, psy_ui_Graphics*, psy_Property*,
	int x, int y);
static void pluginsview_onpreferredsize(PluginsView* self, const psy_ui_Size* limit,
	psy_ui_Size* rv);
static void pluginsview_onkeydown(PluginsView*, psy_ui_KeyEvent*);
static void pluginsview_cursorposition(PluginsView*, psy_Property* plugin,
	int* col, int* row);
static psy_Property* pluginsview_pluginbycursorposition(PluginsView*,
	int col, int row);
static void pluginsview_onmousedown(PluginsView*, psy_ui_MouseEvent*);
static void pluginsview_onmousedoubleclick(PluginsView*, psy_ui_MouseEvent*);
static void pluginsview_hittest(PluginsView*, int x, int y);
static void pluginsview_computetextsizes(PluginsView*, const psy_ui_Size*);
static void pluginsview_onplugincachechanged(PluginsView*,
	psy_audio_PluginCatcher* sender);
static int pluginsview_visilines(PluginsView*);
static int pluginsview_topline(PluginsView*);
static void pluginsview_settopline(PluginsView*, int line);
static int pluginsview_numlines(const PluginsView*);

static psy_ui_ComponentVtable pluginsview_vtable;
static int pluginsview_vtable_initialized = 0;

static void pluginsview_vtable_init(PluginsView* self)
{
	if (!pluginsview_vtable_initialized) {
		pluginsview_vtable = *(self->component.vtable);				
		pluginsview_vtable.ondraw = (psy_ui_fp_component_ondraw) pluginsview_ondraw;		
		pluginsview_vtable.onkeydown = (psy_ui_fp_component_onkeydown)
			pluginsview_onkeydown;
		pluginsview_vtable.onmousedown = (psy_ui_fp_component_onmousedown)
			pluginsview_onmousedown;
		pluginsview_vtable.onmousedoubleclick = (psy_ui_fp_component_onmousedoubleclick)
			pluginsview_onmousedoubleclick;
		pluginsview_vtable.onpreferredsize = (psy_ui_fp_component_onpreferredsize)
			pluginsview_onpreferredsize;		
	}
}

void pluginsview_init(PluginsView* self, psy_ui_Component* parent,
	bool favorites,
	Workspace* workspace)
{	
	psy_ui_Size size;

	psy_ui_component_init(&self->component, parent);
	pluginsview_vtable_init(self);
	self->component.vtable = &pluginsview_vtable;
	self->workspace = workspace;
	self->onlyfavorites = favorites;
	if (workspace_pluginlist(workspace)) {
		if (favorites) {
			self->plugins = newmachine_favorites(
				workspace_pluginlist(workspace));
		} else {
			self->plugins = psy_property_clone(
				workspace_pluginlist(workspace));
		}
	} else {
		self->plugins = NULL;
	}	
	psy_ui_component_doublebuffer(&self->component);
	psy_ui_component_setwheelscroll(&self->component, 4);
	psy_ui_component_setoverflow(&self->component, psy_ui_OVERFLOW_VSCROLL);
	psy_signal_connect(&self->component.signal_destroy, self,
		pluginsview_ondestroy);
	self->selectedplugin = NULL;
	self->calledby = 0;
	psy_signal_init(&self->signal_selected);
	psy_signal_init(&self->signal_changed);
	psy_signal_connect(&workspace->plugincatcher.signal_changed, self,
		pluginsview_onplugincachechanged);
	size = psy_ui_component_size(&self->component);
	pluginsview_computetextsizes(self, &size);	
}

void pluginsview_ondestroy(PluginsView* self, psy_ui_Component* component)
{
	psy_signal_dispose(&self->signal_selected);
	psy_signal_dispose(&self->signal_changed);
	if (self->plugins) {
		psy_property_deallocate(self->plugins);
	}
}

void pluginsview_ondraw(PluginsView* self, psy_ui_Graphics* g)
{		
	
	if (self->plugins) {
		psy_ui_Size size;
		psy_List* p;		
		int cpx;
		int cpy;
		
		size = psy_ui_component_size(&self->component);
		pluginsview_computetextsizes(self, &size);
		psy_ui_setbackgroundmode(g, psy_ui_TRANSPARENT);
		for (p = psy_property_children(self->plugins), cpx = 0, cpy = 0;
				p != NULL; psy_list_next(&p)) {
			pluginsview_drawitem(self, g, (psy_Property*)psy_list_entry(p),
				cpx, cpy);
			cpx += self->columnwidth;
			if (cpx >= self->numparametercols * self->columnwidth) {
				cpx = 0;
				cpy += self->lineheight;
			}
		}
	}
}

void pluginsview_drawitem(PluginsView* self, psy_ui_Graphics* g,
	psy_Property* property, int x, int y)
{
	char text[128];

	if (property == self->selectedplugin) {
		psy_ui_setbackgroundcolour(g, psy_ui_colour_make(0x009B7800));
		psy_ui_settextcolour(g, psy_ui_colour_make(0x00FFFFFF));
	} else {
		psy_ui_setbackgroundcolour(g, psy_ui_colour_make(0x00232323));
		psy_ui_settextcolour(g, psy_ui_colour_make(0x00CACACA));
	}		
	plugindisplayname(property, text);	
	psy_ui_textout(g, x, y + 2, text, strlen(text));
	plugintype(property, text);
	psy_ui_textout(g, x + self->columnwidth - 7 * self->avgcharwidth,
		y + 2, text, strlen(text));
	if (pluginmode(property, text) == MACHMODE_FX) {
		psy_ui_settextcolour(g, psy_ui_colour_make(0x00B1C8B0));
	} else {		
		psy_ui_settextcolour(g, psy_ui_colour_make(0x00D1C5B6));
	}
	psy_ui_textout(g, x + self->columnwidth - 10 * self->avgcharwidth,
		y + 2, text, strlen(text));
}

void pluginsview_computetextsizes(PluginsView* self, const psy_ui_Size* size)
{
	psy_ui_TextMetric tm;
	
	tm = psy_ui_component_textmetric(&self->component);
	self->avgcharwidth = tm.tmAveCharWidth;
	self->lineheight = (int) (tm.tmHeight * 1.5);
	self->columnwidth = tm.tmAveCharWidth * 45;
	self->identwidth = tm.tmAveCharWidth * 4;
	self->numparametercols = psy_max(1, psy_ui_value_px(&size->width, &tm) / self->columnwidth);
	self->component.scrollstepy = self->lineheight;
}

void plugindisplayname(psy_Property* property, char* text)
{	
	const char* label;

	label = psy_property_at_str(property, "shortname", "");
	if (strcmp(label, "") == 0) {
		label = psy_property_key(property);
	}
	psy_snprintf(text, 128, "%s", label);
}

int plugintype(psy_Property* property, char* text)
{	
	int rv;
	
	rv = psy_property_at_int(property, "type", -1);
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

int pluginmode(psy_Property* property, char* text)
{			
	int rv;

	rv = psy_property_at_int(property, "mode", -1);
	strcpy(text, rv == MACHMODE_FX ? "fx" : "gn");
	return rv;
}

void pluginsview_onpreferredsize(PluginsView* self, const psy_ui_Size* limit,
	psy_ui_Size* rv)
{
	if (self->plugins) {
		pluginsview_computetextsizes(self, limit);
		rv->height = psy_ui_value_makepx(self->lineheight *
			pluginsview_numlines(self));
	}
}

void pluginsview_onkeydown(PluginsView* self, psy_ui_KeyEvent* ev)
{
	if (self->selectedplugin) {
		psy_Property* plugin;
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
			case psy_ui_KEY_DELETE:
				if (self->selectedplugin) {
					psy_Property* p;
					p = psy_property_find(self->workspace->plugincatcher.plugins,
							psy_property_key(self->selectedplugin),
							PSY_PROPERTY_TYPE_NONE);
					if (!self->onlyfavorites && p) {
						psy_property_remove(self->workspace->plugincatcher.plugins, p);
					} else {						
						psy_property_set_int(p, "favorite", 0);
					}
					plugincatcher_save(&self->workspace->plugincatcher);
					psy_signal_emit(&self->workspace->plugincatcher.signal_changed,
						&self->workspace->plugincatcher, 0);
				}
				break;
			case psy_ui_KEY_DOWN:
				if (row + 1 < pluginsview_numlines(self)) {
					++row;
					if (row > pluginsview_topline(self) + pluginsview_visilines(self)) {
						pluginsview_settopline(self, row - pluginsview_visilines(self));
					}
				} else {
					psy_ui_component_focus_next(psy_ui_component_parent(&self->component));
				}
				psy_ui_keyevent_stoppropagation(ev);
				break;		
			case psy_ui_KEY_UP:
				if (row > 0) {
					--row;
					if (row < pluginsview_topline(self)) {
						pluginsview_settopline(self, row);
					}
				} else {
					psy_ui_component_focus_prev(psy_ui_component_parent(
						&self->component));
				}
				psy_ui_keyevent_stoppropagation(ev);
				break;
			case psy_ui_KEY_PRIOR:
				if (row > 0) {
					row = psy_max(0, row - 4);
					if (row < pluginsview_topline(self)) {
						pluginsview_settopline(self, row);
					}
				}
				psy_ui_keyevent_stoppropagation(ev);
				break;
			case psy_ui_KEY_NEXT:
				row += 4;
				if (row >= pluginsview_numlines(self) - 1) {
					row = pluginsview_numlines(self) - 1;
				}
				if (row > pluginsview_topline(self) + pluginsview_visilines(self)) {
					pluginsview_settopline(self, row - pluginsview_visilines(self));
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
		if (self->plugins && !psy_property_empty(self->plugins)) {
			self->selectedplugin = psy_property_first(self->plugins);
			psy_signal_emit(&self->signal_changed, self, 1,
				self->selectedplugin);
			psy_ui_component_invalidate(&self->component);
		}
	}	
}

int pluginsview_visilines(PluginsView* self)
{
	psy_ui_IntSize size;

	size = psy_ui_component_intsize(&self->component);
	return size.height / self->lineheight;
}

int pluginsview_topline(PluginsView* self)
{	
	return psy_ui_component_scrolltop(&self->component) / self->lineheight;
}

int pluginsview_numlines(const PluginsView* self)
{
	return psy_property_size(self->plugins) /
		self->numparametercols + 1;
}

void pluginsview_settopline(PluginsView* self, int line)
{
	psy_ui_component_setscrolltop(&self->component, line * self->lineheight);
}

void pluginsview_cursorposition(PluginsView* self, psy_Property* plugin,
	int* col, int* row)
{		
	*col = 0;
	*row = 0;
	if (plugin && self->plugins) {
		psy_List* p;
		
		for (p = psy_property_children(self->plugins); p != NULL;
				psy_list_next(&p)) {	
			if (p->entry == plugin) {
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

psy_Property* pluginsview_pluginbycursorposition(PluginsView* self, int col, int row)
{				
	if (self->plugins) {
		psy_Property* rv;
		psy_List* p;
		int currcol;
		int currrow;
		psy_ui_Size size;
		rv = NULL;

		currcol = 0;
		currrow = 0;
		size = psy_ui_component_size(&self->component);
		pluginsview_computetextsizes(self, &size);
		for (p = psy_property_children(self->plugins); p != NULL;
				psy_list_next(&p)) {			
			if (currcol == col && currrow == row) {
				rv = (psy_Property*)p->entry;
				break;
			}
			++currcol;
			if (currcol >= self->numparametercols) {
				currcol = 0;
				++currrow;
			}
		}
		return rv;
	}
	return NULL;
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
	if (self->plugins) {
		psy_ui_Size size;
		psy_List* p;
		int cpx;
		int cpy;

		size = psy_ui_component_size(&self->component);
		pluginsview_computetextsizes(self, &size);
		for (p = psy_property_children(self->plugins), cpx = 0, cpy = 0;
				p != NULL; psy_list_next(&p)) {
			psy_ui_Rectangle r;

			psy_ui_setrectangle(&r, cpx, cpy, self->columnwidth,
				self->lineheight);
			if (psy_ui_rectangle_intersect(&r, x, y)) {
				self->selectedplugin = (psy_Property*)psy_list_entry(p);
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
		psy_property_deallocate(self->plugins);
	}
	if (sender->plugins) {
		if (self->onlyfavorites) {
			self->plugins = newmachine_favorites(sender->plugins);
		} else {
			self->plugins = psy_property_clone(sender->plugins);
		}
	} else {
		self->plugins = 0;
	}
	psy_ui_component_setscrolltop(&self->component, 0);	
	psy_ui_component_updateoverflow(&self->component);
	psy_ui_component_invalidate(&self->component);
}

// NewMachine
// prototypes
static void newmachine_ondestroy(NewMachine*, psy_ui_Component* component);
static void newmachine_onpluginselected(NewMachine*, psy_ui_Component* parent,
	psy_Property*);
static void newmachine_onpluginchanged(NewMachine*, psy_ui_Component* parent,
	psy_Property*);
static void newmachine_onplugincachechanged(NewMachine*, psy_audio_PluginCatcher*);
static void newmachine_onkeydown(NewMachine*, psy_ui_KeyEvent*);
static void newmachine_onsortbyfavorite(NewMachine*, psy_ui_Component* sender);
static void newmachine_onsortbyname(NewMachine*, psy_ui_Component* sender);
static void newmachine_onsortbytype(NewMachine*, psy_ui_Component* sender);
static void newmachine_onsortbymode(NewMachine*, psy_ui_Component* sender);
static void newmachine_onfocus(NewMachine*, psy_ui_Component* sender);

// vtable
static psy_ui_ComponentVtable newmachine_vtable;
static int newmachine_vtable_initialized = 0;

static void newmachine_vtable_init(NewMachine* self)
{
	if (!newmachine_vtable_initialized) {
		newmachine_vtable = *(self->component.vtable);				
		newmachine_vtable.onkeydown = (psy_ui_fp_component_onkeydown)
			newmachine_onkeydown;		
	}
}
// implementation
void newmachine_init(NewMachine* self, psy_ui_Component* parent,
	MachineViewSkin* skin, Workspace* workspace)
{
	psy_ui_Margin margin;
	psy_ui_Border sectionborder;
	
	psy_ui_component_init(&self->component, parent);	
	newmachine_vtable_init(self);
	self->component.vtable = &newmachine_vtable;
	self->skin = skin;		
	newmachinedetail_init(&self->detail, &self->component, workspace);
	psy_ui_component_setalign(&self->detail.component, psy_ui_ALIGN_LEFT);	
	// header margin
	psy_ui_margin_init_all(&margin, psy_ui_value_makeeh(1),
		psy_ui_value_makepx(0), psy_ui_value_makeeh(1),
		psy_ui_value_makepx(0));
	// section border, define for the top line above a section label
	psy_ui_border_init_all(&sectionborder, psy_ui_BORDER_NONE,
		psy_ui_BORDER_NONE, psy_ui_BORDER_SOLID, psy_ui_BORDER_NONE);
	// favorite view
	psy_ui_label_init_text(&self->favoriteheader, &self->component,
			"newmachine.favorites");
	psy_ui_label_settextalignment(&self->favoriteheader,
		psy_ui_ALIGNMENT_LEFT |
		psy_ui_ALIGNMENT_CENTER_VERTICAL);
	psy_ui_component_setalign(&self->favoriteheader.component, psy_ui_ALIGN_TOP);
	psy_ui_component_setmargin(&self->favoriteheader.component, &margin);
	self->favoriteheader.component.preventpreferredsizeatalign = TRUE;
	psy_ui_component_resize(&self->favoriteheader.component,
		psy_ui_size_make(psy_ui_value_makepx(0), psy_ui_value_makeeh(2)));	
	psy_ui_component_setborder(&self->favoriteheader.component, sectionborder);
	pluginsview_init(&self->favoriteview, &self->component, TRUE, workspace);	
	psy_ui_component_setmaximumsize(&self->favoriteview.component,
		psy_ui_size_make(psy_ui_value_makepx(0), psy_ui_value_makeeh(4.0)));
	psy_ui_scroller_init(&self->scroller_fav, &self->favoriteview.component,
		&self->component);
	psy_ui_component_settabindex(&self->scroller_fav.component, 0);
	psy_ui_component_setalign(&self->scroller_fav.component, psy_ui_ALIGN_TOP);
	// plugin view
	psy_ui_label_init_text(&self->pluginsheader, &self->component,
		"newmachine.all");
	psy_ui_label_settextalignment(&self->pluginsheader,
		psy_ui_ALIGNMENT_LEFT |
		psy_ui_ALIGNMENT_CENTER_VERTICAL);
	psy_ui_component_setalign(&self->pluginsheader.component, psy_ui_ALIGN_TOP);
	psy_ui_component_setmargin(&self->pluginsheader.component, &margin);
	self->pluginsheader.component.preventpreferredsizeatalign = TRUE;
	psy_ui_component_resize(&self->pluginsheader.component,
		psy_ui_size_make(psy_ui_value_makepx(0), psy_ui_value_makeeh(2)));
	psy_ui_component_setborder(&self->pluginsheader.component, sectionborder);
	pluginsview_init(&self->pluginsview, &self->component, FALSE, workspace);	
	psy_ui_scroller_init(&self->scroller_main, &self->pluginsview.component,
		&self->component);
	psy_ui_component_settabindex(&self->scroller_main.component, 1);
	psy_ui_component_setalign(&self->scroller_main.component, psy_ui_ALIGN_CLIENT);
	psy_signal_init(&self->signal_selected);	
	psy_signal_connect(&self->pluginsview.signal_selected, self,
		newmachine_onpluginselected);
	psy_signal_connect(&self->favoriteview.signal_selected, self,
		newmachine_onpluginselected);
	psy_signal_connect(&self->pluginsview.signal_changed, self,
		newmachine_onpluginchanged);
	psy_signal_connect(&self->favoriteview.signal_changed, self,
		newmachine_onpluginchanged);
	psy_signal_connect(&workspace->plugincatcher.signal_changed, self,
		newmachine_onplugincachechanged);
	psy_signal_connect(&self->detail.bar.sortbyfavorite.signal_clicked, self,
		newmachine_onsortbyfavorite);
	psy_signal_connect(&self->detail.bar.sortbyname.signal_clicked, self,
		newmachine_onsortbyname);
	psy_signal_connect(&self->detail.bar.sortbytype.signal_clicked, self,
		newmachine_onsortbytype);
	psy_signal_connect(&self->detail.bar.sortbymode.signal_clicked, self,
		newmachine_onsortbymode);
	psy_signal_connect(&self->component.signal_focus, self,
		newmachine_onfocus);
	psy_signal_connect(&self->component.signal_destroy, self,
		newmachine_ondestroy);
	newmachine_updateskin(self);
}

void newmachine_ondestroy(NewMachine* self, psy_ui_Component* component)
{
	psy_signal_dispose(&self->signal_selected);
}

void newmachine_updateskin(NewMachine* self)
{
	psy_ui_component_setbackgroundcolour(&self->component, self->skin->colour);
	psy_ui_component_setcolour(&self->component, self->skin->effect_fontcolour);
}

void newmachine_onpluginselected(NewMachine* self, psy_ui_Component* parent,
	psy_Property* selected)
{
	const char* text;
	char detail[1024];
	psy_Property* sorted;

	text = psy_property_at_str(selected, "name", "");
	strcpy(detail, text);
	// text = psy_property_at_str(selected, "desc", "");
	// strcat(detail, "  ");
	// strcat(detail, text);	
	text = psy_property_at_str(selected, "author", "");
	strcat(detail, "\n(");
	strcat(detail, text);
	strcat(detail, ")");
	psy_ui_label_settext(&self->detail.desclabel, detail);
	self->detail.empty = FALSE;
	psy_signal_emit(&self->signal_selected, self, 1, selected);
	psy_property_sync(workspace_pluginlist(self->pluginsview.workspace), self->pluginsview.plugins);
	plugincatcher_save(&self->pluginsview.workspace->plugincatcher);
	pluginsview_onplugincachechanged(&self->favoriteview,
		&self->favoriteview.workspace->plugincatcher);
	if (self->favoriteview.plugins) {
		sorted = newmachine_sort(self->favoriteview.plugins,
			newmachine_comp_favorite);
		psy_property_deallocate(self->favoriteview.plugins);
		self->favoriteview.plugins = sorted;
		newmachinedetail_reset(&self->detail);
		psy_ui_component_setscrolltop(&self->favoriteview.component, 0);
		psy_ui_component_updateoverflow(&self->favoriteview.component);
		psy_ui_component_invalidate(&self->favoriteview.component);
	}
}

void newmachine_onpluginchanged(NewMachine* self, psy_ui_Component* parent,
	psy_Property* selected)
{
	const char* text;
	char detail[1024];

	if (selected) {
		text = psy_property_at_str(selected, "name", "");
		strcpy(detail, text);
		text = psy_property_at_str(selected, "desc", "");
		strcat(detail, "  ");
		strcat(detail, text);
		text = psy_property_at_str(selected, "author", "");
		strcat(detail, "\n(");
		strcat(detail, text);
		strcat(detail, ")");
	} else {
		detail[0] = '\0';
	}
	psy_ui_label_settext(&self->detail.desclabel, detail);
}

void newmachine_onplugincachechanged(NewMachine* self,
	psy_audio_PluginCatcher* sender)
{
	newmachinedetail_reset(&self->detail);	
}

void newmachine_onsortbyfavorite(NewMachine* self, psy_ui_Component* sender)
{
	psy_Property* sorted;

	if (self->pluginsview.plugins) {
		sorted = newmachine_sort(self->pluginsview.plugins,
			newmachine_comp_favorite);
		psy_property_deallocate(self->pluginsview.plugins);
		self->pluginsview.plugins = sorted;
		newmachinedetail_reset(&self->detail);
		psy_ui_component_setscrolltop(&self->pluginsview.component, 0);
		psy_ui_component_updateoverflow(&self->pluginsview.component);
		psy_ui_component_invalidate(&self->pluginsview.component);
	}
}

void newmachine_onsortbyname(NewMachine* self, psy_ui_Component* sender)
{
	psy_Property* sorted;
	
	if (self->pluginsview.plugins) {
		sorted = newmachine_sort(self->pluginsview.plugins,
			newmachine_comp_name);
		psy_property_deallocate(self->pluginsview.plugins);
		self->pluginsview.plugins = sorted;
		newmachinedetail_reset(&self->detail);
		psy_ui_component_setscrolltop(&self->pluginsview.component, 0);
		psy_ui_component_updateoverflow(&self->pluginsview.component);
		psy_ui_component_invalidate(&self->pluginsview.component);
	}
}

void newmachine_onsortbytype(NewMachine* self, psy_ui_Component* parent)
{
	psy_Property* sorted;
	
	if (self->pluginsview.plugins) {
		sorted = newmachine_sort(self->pluginsview.plugins,
			newmachine_comp_type);
		psy_property_deallocate(self->pluginsview.plugins);
		self->pluginsview.plugins = sorted;
		newmachinedetail_reset(&self->detail);
		psy_ui_component_setscrolltop(&self->pluginsview.component, 0);
		psy_ui_component_updateoverflow(&self->pluginsview.component);
		psy_ui_component_invalidate(&self->pluginsview.component);
	}
}

void newmachine_onsortbymode(NewMachine* self, psy_ui_Component* parent)
{
	psy_Property* sorted;
	
	if (self->pluginsview.plugins) {
		sorted = newmachine_sort(self->pluginsview.plugins,
			newmachine_comp_mode);
		psy_property_deallocate(self->pluginsview.plugins);
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

psy_Property* newmachine_favorites(psy_Property* source)
{
	psy_Property* rv = 0;

	if (source && !psy_property_empty(source)) {
		psy_List* p;

		rv = psy_property_allocinit_key(NULL);
		for (p = psy_property_children(source); p != NULL; psy_list_next(&p)) {
			psy_Property* property;

			property = (psy_Property*)psy_list_entry(p);
			if (psy_property_at_int(property, "favorite", 0) != FALSE) {				
				psy_property_append_property(rv, psy_property_clone(
					property));
			}
		}		
	}
	return rv;
}

psy_Property* newmachine_sort(psy_Property* source, psy_fp_comp comp)
{		
	psy_Property* rv = NULL;

	if (source) {
		int i;
		int num;
		psy_List* p;
		psy_Property** propertiesptr;
		
		num = psy_property_size(source);
		propertiesptr = malloc(sizeof(psy_Property*) * num);
		if (propertiesptr) {
			p = psy_property_children(source);
			for (i = 0; p != NULL && i < num; psy_list_next(&p), ++i) {
				propertiesptr[i] = (psy_Property*)psy_list_entry(p);
			}
			psy_qsort(propertiesptr, 0, num - 1, comp);
			rv = psy_property_allocinit_key(NULL);
			for (i = 0; i < num; ++i) {
				psy_property_append_property(rv, psy_property_clone(
					propertiesptr[i]));
			}
			free(propertiesptr);
		}
	}
	return rv;
}

int newmachine_comp_favorite(psy_Property* p, psy_Property* q)
{
	int left;
	int right;
	
	left = psy_property_at_int(p, "favorite", 0);
	right = psy_property_at_int(q, "favorite", 0);
	return right - left;
}

int newmachine_comp_name(psy_Property* p, psy_Property* q)
{
	const char* left;
	const char* right;

	left = psy_property_at_str(p, "name", "");
	if (strlen(left) == 0) {
		left = psy_property_key(p);
	}
	right = psy_property_at_str(q, "name", "");
	if (strlen(right) == 0) {
		right = psy_property_key(q);
	}
	return strcmp(left, right);		
}

int newmachine_comp_type(psy_Property* p, psy_Property* q)
{
	int left;
	int right;
	
	left = psy_property_at_int(p, "type", 128);
	left = newmachine_isplugin(left) ? left : 0;
	right = psy_property_at_int(q, "type", 128);
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

int newmachine_comp_mode(psy_Property* p, psy_Property* q)
{	
	return psy_property_at_int(p, "mode", 128) -
		psy_property_at_int(q, "mode", 128);	
}

void newmachine_onfocus(NewMachine* self, psy_ui_Component* sender)
{
	psy_ui_component_setfocus(&self->pluginsview.component);
}

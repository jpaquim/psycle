// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "pluginsview.h"
// host
#include "styles.h"
// container
#include <qsort.h>
// platform
#include "../../detail/portable.h"
#include "../../detail/strcasestr.h"

static void plugindisplayname(psy_Property*, char* text);
static uintptr_t plugintype(psy_Property*, char* text);
static uintptr_t pluginmode(psy_Property*, char* text);
static psy_Property* search(psy_Property* source, NewMachineFilter*);
static void searchfilter(psy_Property* plugin, NewMachineFilter*,
	psy_Property* parent);
static int isplugin(int type);

// Quicksort callbacks
psy_Property* newmachine_sort(psy_Property* source, psy_fp_comp);
static int newmachine_comp_favorite(psy_Property* p, psy_Property* q);
static int newmachine_comp_name(psy_Property* p, psy_Property* q);
static int newmachine_comp_type(psy_Property* p, psy_Property* q);
static int newmachine_comp_mode(psy_Property* p, psy_Property* q);


psy_Property* newmachine_sort(psy_Property* source, psy_fp_comp comp)
{
	psy_Property* rv = NULL;

	if (source) {
		uintptr_t i;
		uintptr_t num;
		psy_List* p;
		psy_Property** propertiesptr;

		num = psy_property_size(source);
		propertiesptr = malloc(sizeof(psy_Property*) * num);
		if (propertiesptr) {
			p = psy_property_begin(source);
			for (i = 0; p != NULL && i < num; psy_list_next(&p), ++i) {
				propertiesptr[i] = (psy_Property*)psy_list_entry(p);
			}
			psy_qsort(propertiesptr, 0, (int)(num - 1), comp);
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

	left = (int)psy_property_at_int(p, "favorite", 0);
	right = (int)psy_property_at_int(q, "favorite", 0);
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

	left = (int)psy_property_at_int(p, "type", 128);
	left = isplugin(left) ? left : 0;
	right = (int)psy_property_at_int(q, "type", 128);
	right = isplugin(right) ? right : 0;
	return left - right;
}

int newmachine_comp_mode(psy_Property* p, psy_Property* q)
{
	return (int)psy_property_at_int(p, "mode", 128) -
		(int)psy_property_at_int(q, "mode", 128);
}


void newmachinesort_init(NewMachineSort* self)
{
	self->mode = NEWMACHINESORTMODE_NONE;
	psy_signal_init(&self->signal_changed);
}

void newmachinesort_dispose(NewMachineSort* self)
{
	psy_signal_dispose(&self->signal_changed);
}

void newmachinesort_sort(NewMachineSort* self, NewMachineSortMode mode)
{
	self->mode = mode;
	psy_signal_emit(&self->signal_changed, self, 0);
}

int isplugin(int type)
{
	return (type == psy_audio_PLUGIN) ||
		(type == psy_audio_VST) ||
		(type == psy_audio_VSTFX) ||
		(type == psy_audio_LUA) ||
		(type == psy_audio_LADSPA);
}


// NewMachineFilter
void newmachinefilter_init(NewMachineFilter* self)
{
	self->text = NULL;
	self->gen = TRUE;
	self->effect = TRUE;
	psy_table_init(&self->categories);
	newmachinefilter_setalltypes(self);		
	psy_signal_init(&self->signal_changed);
}

void newmachinefilter_dispose(NewMachineFilter* self)
{	
	psy_table_disposeall(&self->categories, NULL);
	psy_signal_dispose(&self->signal_changed);
	free(self->text);
}

void newmachinefilter_notify(NewMachineFilter* self)
{
	psy_signal_emit(&self->signal_changed, self, 0);
}

void newmachinefilter_reset(NewMachineFilter* self)
{
	self->gen = TRUE;
	self->effect = TRUE;
	newmachinefilter_setalltypes(self);
	psy_strreset(&self->text, "");
	psy_table_disposeall(&self->categories, NULL);
	psy_table_init(&self->categories);
	newmachinefilter_notify(self);
}

void newmachinefilter_settext(NewMachineFilter* self, const char* text)
{
	psy_strreset(&self->text, text);
	newmachinefilter_notify(self);
}

void newmachinefilter_setalltypes(NewMachineFilter* self)
{
	self->effect = TRUE;
	self->intern = TRUE;
	self->native = TRUE;
	self->vst = TRUE;
	self->lua = TRUE;
	self->ladspa = TRUE;
}

void newmachinefilter_cleartypes(NewMachineFilter* self)
{
	self->effect = FALSE;
	self->intern = FALSE;
	self->native = FALSE;
	self->vst = FALSE;
	self->lua = FALSE;
	self->ladspa = FALSE;
}

bool newmachinefilter_all(const NewMachineFilter* self)
{
	return (self->gen && self->effect);
}

void newmachinefilter_addcategory(NewMachineFilter* self, const char* category)
{
	if (psy_strlen(category) > 0) {
		if (!psy_table_exists_strhash(&self->categories, category)) {
			psy_table_insert_strhash(&self->categories, category,
				(void*)psy_strdup(category));
			newmachinefilter_notify(self);
		}
	}
}

void newmachinefilter_removecategory(NewMachineFilter* self, const char* category)
{
	if (psy_strlen(category) > 0) {
		if (psy_table_exists_strhash(&self->categories, category)) {
			char* item;
			
			item = psy_table_at(&self->categories, psy_strhash(category));
			free(item);
			psy_table_remove(&self->categories, psy_strhash(category));
			newmachinefilter_notify(self);
		}
	}
}

void newmachinefilter_anycategory(NewMachineFilter* self)
{
	psy_table_disposeall(&self->categories, NULL);
	psy_table_init(&self->categories);
	newmachinefilter_notify(self);
}

bool newmachinefilter_useanycategory(const NewMachineFilter* self)
{
	return psy_table_size(&self->categories) == 0;	
}

bool newmachinefilter_hascategory(const NewMachineFilter* self,
	const char* category)
{
	if (newmachinefilter_useanycategory(self)) {
		return TRUE;
	}
	if (!category) {
		return FALSE;
	}
	return psy_table_exists_strhash(&self->categories, category);
}

psy_Property* search(psy_Property* source, NewMachineFilter* filter)
{
	psy_Property* rv;
	psy_List* p;
	uintptr_t num;
	uintptr_t i;
	char* text;

	assert(filter);

	rv = psy_property_allocinit_key(NULL);
	if (!source) {
		return rv;
	}
	num = psy_property_size(source);
	p = psy_property_begin(source);
	text = filter->text;
	for (i = 0; p != NULL && i < num; psy_list_next(&p), ++i) {
		psy_Property* q;

		q = (psy_Property*)psy_list_entry(p);
		if (psy_strlen(text) == 0) {
			searchfilter(q, filter, rv);
		} else if (psy_property_text(q) &&
			strcasestr(psy_property_text(q), text)) {
			searchfilter(q, filter, rv);
		}
	}
	return rv;
}

void searchfilter(psy_Property* plugin, NewMachineFilter* filter,
	psy_Property* parent)
{
	psy_audio_MachineInfo machineinfo;
	bool isintern;

	machineinfo_init(&machineinfo);
	psy_audio_machineinfo_from_property(plugin, &machineinfo);
	if (!filter->vst && (machineinfo.type == psy_audio_VST || machineinfo.type == psy_audio_VSTFX)) {
		return;
	}
	if (!filter->native && machineinfo.type == psy_audio_PLUGIN) {
		return;
	}
	if (!filter->ladspa && machineinfo.type == psy_audio_LADSPA) {
		return;
	}
	if (!filter->lua && machineinfo.type == psy_audio_LUA) {
		return;
	}
	isintern = FALSE;
	switch (machineinfo.type) {
	case psy_audio_PLUGIN:
	case psy_audio_LUA:
	case psy_audio_VST:
	case psy_audio_VSTFX:
	case psy_audio_LADSPA:
		break;
	default:
		isintern = TRUE;
		break;
	}
	if (!newmachinefilter_hascategory(filter, machineinfo.category)) {
		return;
	}
	if (!filter->intern && isintern) {
		return;
	}
	if (filter->effect && machineinfo.mode == psy_audio_MACHMODE_FX) {
		psy_property_append_property(parent, psy_property_clone(plugin));
	}
	if (filter->gen && machineinfo.mode == psy_audio_MACHMODE_GENERATOR) {
		psy_property_append_property(parent, psy_property_clone(plugin));
	}
}

// PluginsView
static void pluginsview_ondestroy(PluginsView*);
static void pluginsview_ondraw(PluginsView*, psy_ui_Graphics*);
static void pluginsview_drawitem(PluginsView*, psy_ui_Graphics*, psy_Property*,
	psy_ui_RealPoint topleft);
static void pluginsview_onpreferredscrollsize(PluginsView*, const psy_ui_Size* limit,
	psy_ui_Size* rv);
static void pluginsview_onkeydown(PluginsView*, psy_ui_KeyEvent*);
static void pluginsview_cursorposition(PluginsView*, psy_Property* plugin,
	intptr_t* col, intptr_t* row);
static psy_Property* pluginsview_pluginbycursorposition(PluginsView*,
	intptr_t col, intptr_t row);
static void pluginsview_onmousedown(PluginsView*, psy_ui_MouseEvent*);
static void pluginsview_onmousedoubleclick(PluginsView*, psy_ui_MouseEvent*);
static void pluginsview_hittest(PluginsView*, double x, double y);
static void pluginsview_computetextsizes(PluginsView*, psy_ui_RealSize);
static uintptr_t pluginsview_visilines(PluginsView*);
static uintptr_t pluginsview_topline(PluginsView*);
static void pluginsview_settopline(PluginsView*, intptr_t line);
static uintptr_t pluginsview_numlines(const PluginsView*);
static uintptr_t pluginenabled(const PluginsView*, psy_Property* property);
static void pluginsview_onfilterchanged(PluginsView*, NewMachineFilter* sender);
static void pluginsview_onsortchanged(PluginsView*, NewMachineSort* sender);
static void pluginsview_ondragstart(PluginsView*, psy_ui_DragEvent*);

static psy_ui_ComponentVtable pluginsview_vtable;
static psy_ui_ComponentVtable pluginsview_super_vtable;
static bool pluginsview_vtable_initialized = FALSE;

static void pluginsview_vtable_init(PluginsView* self)
{
	if (!pluginsview_vtable_initialized) {
		pluginsview_vtable = *(self->component.vtable);
		pluginsview_super_vtable = pluginsview_vtable;
		pluginsview_vtable.ondestroy =
			(psy_ui_fp_component_ondestroy)
			pluginsview_ondestroy;
		pluginsview_vtable.ondraw =
			(psy_ui_fp_component_ondraw)
			pluginsview_ondraw;
		pluginsview_vtable.onkeydown =
			(psy_ui_fp_component_onkeyevent)
			pluginsview_onkeydown;
		pluginsview_vtable.onmousedown =
			(psy_ui_fp_component_onmouseevent)
			pluginsview_onmousedown;
		pluginsview_vtable.onmousedoubleclick =
			(psy_ui_fp_component_onmouseevent)
			pluginsview_onmousedoubleclick;
		pluginsview_vtable.onpreferredsize =
			(psy_ui_fp_component_onpreferredscrollsize)
			pluginsview_onpreferredscrollsize;
		pluginsview_vtable.ondragstart =
			(psy_ui_fp_component_ondragstart)
			pluginsview_ondragstart;
		pluginsview_vtable_initialized = TRUE;
	}
	self->component.vtable = &pluginsview_vtable;
}

void pluginsview_init(PluginsView* self, psy_ui_Component* parent)
{
	psy_ui_component_init(&self->component, parent, NULL);
	pluginsview_vtable_init(self);
	psy_ui_component_doublebuffer(&self->component);
	self->component.draggable = TRUE;
	psy_signal_init(&self->signal_selected);
	psy_signal_init(&self->signal_changed);	
	self->mode = NEWMACHINE_APPEND;
	self->currplugins = NULL;
	self->plugins = NULL;
	self->filteredplugins = NULL;
	self->selectedplugin = NULL;
	self->filter = NULL;
	self->sort = NULL;
	self->generatorsenabled = TRUE;
	self->effectsenabled = TRUE;		
	pluginsview_computetextsizes(self, 
		psy_ui_component_innersize_px(&self->component));	
}

void pluginsview_ondestroy(PluginsView* self)
{
	psy_signal_dispose(&self->signal_selected);
	psy_signal_dispose(&self->signal_changed);
	pluginsview_clear(self);
	if (self->filter) {
		psy_signal_disconnect(&self->filter->signal_changed, self,
			pluginsview_onfilterchanged);
	}
}

void pluginsview_clear(PluginsView* self)
{
	if (self->plugins) {
		psy_property_deallocate(self->filteredplugins);
		self->filteredplugins = NULL;
		psy_property_deallocate(self->plugins);		
		self->plugins = NULL;
		self->selectedplugin = NULL;
		self->currplugins = NULL;
	}				
}

void pluginsview_clearfilter(PluginsView* self)
{
	self->selectedplugin = NULL;
	self->currplugins = self->plugins;
	if (self->filteredplugins) {
		psy_property_deallocate(self->filteredplugins);
		self->filteredplugins = NULL;
	}
}

void pluginsview_setplugins(PluginsView* self, const psy_Property* property)
{
	pluginsview_clear(self);
	self->plugins = psy_property_clone(property);
	self->currplugins = self->plugins;	
	psy_ui_component_invalidate(&self->component);
}

void pluginsview_setfilter(PluginsView* self, NewMachineFilter* filter)
{
	pluginsview_clearfilter(self);
	if (self->filter) {
		psy_signal_disconnect(&self->filter->signal_changed, self,
			pluginsview_onfilterchanged);
	}
	self->filter = filter;
	if (self->filter) {
		psy_signal_connect(&self->filter->signal_changed, self,
			pluginsview_onfilterchanged);
	}
}

void pluginsview_setsort(PluginsView* self, NewMachineSort* sort)
{
	if (self->sort) {
		psy_signal_disconnect(&self->sort->signal_changed, self,
			pluginsview_onsortchanged);
	}
	self->sort = sort;
	if (self->sort) {
		psy_signal_connect(&self->sort->signal_changed, self,
			pluginsview_onsortchanged);
	}
}

void pluginsview_ondraw(PluginsView* self, psy_ui_Graphics* g)
{	
	if (self->plugins) {
		psy_ui_RealSize size;
		psy_List* p;
		psy_ui_RealPoint cp;
		bool odd;		
		psy_ui_Colour bgcolour;
		psy_ui_Colour overlaycolour;
		int overlay;
		psy_ui_Colour oddlinebgcolour;
		
		size = psy_ui_component_innersize_px(&self->component);
		pluginsview_computetextsizes(self, size);
		psy_ui_setbackgroundmode(g, psy_ui_TRANSPARENT);
		psy_ui_realpoint_init(&cp);
		odd = FALSE;
		bgcolour = psy_ui_component_backgroundcolour(&self->component);
		overlaycolour = psy_ui_colour_white();
		overlay = 2;
		oddlinebgcolour = psy_ui_colour_overlayed(&bgcolour, &overlaycolour,
			overlay / 100.0);
		for (p = psy_property_begin(self->currplugins);
				p != NULL; psy_list_next(&p)) {			
			pluginsview_drawitem(self, g, (psy_Property*)psy_list_entry(p),
				cp);
			cp.x += self->columnwidth;
			if (cp.x >= self->numparametercols * self->columnwidth) {
				cp.x = 0.0;
				cp.y += self->lineheight;
				odd = !odd;
				if (odd && p->next) {
					psy_ui_RealRectangle r;

					psy_ui_realrectangle_init_all(&r,
						psy_ui_realpoint_make(0, cp.y),
						psy_ui_realsize_make(size.width, self->lineheight));
					psy_ui_drawsolidrectangle(g, r, oddlinebgcolour);
				}
			}
		}
	}
}

void pluginsview_drawitem(PluginsView* self, psy_ui_Graphics* g,
	psy_Property* property, psy_ui_RealPoint topleft)
{
	char text[128];

	if (property == self->selectedplugin) {
		psy_ui_setbackgroundcolour(g, psy_ui_colour_make(0x009B7800));		
		psy_ui_settextcolour(g, psy_ui_colour_make(0x00FFFFFF));
	} else {
		psy_ui_setbackgroundcolour(g, psy_ui_colour_make(0x00232323));
		if (pluginenabled(self, property)) {
			psy_ui_settextcolour(g, psy_ui_colour_make(0x00CACACA));
		} else {
			psy_ui_settextcolour(g, psy_ui_colour_make(0x00666666));
		}
	}		
	plugindisplayname(property, text);	
	psy_ui_textout(g, topleft.x, topleft.y + 2, text, strlen(text));
	plugintype(property, text);
	psy_ui_textout(g, topleft.x + self->columnwidth - self->avgcharwidth * 7,
		topleft.y + 2, text, strlen(text));
	if (pluginmode(property, text) == psy_audio_MACHMODE_FX) {
		psy_ui_settextcolour(g, psy_ui_colour_make(0x00B1C8B0));
	} else {		
		psy_ui_settextcolour(g, psy_ui_colour_make(0x00D1C5B6));
	}
	psy_ui_textout(g, topleft.x + self->columnwidth - 10 * self->avgcharwidth,
		topleft.y + 2, text, strlen(text));
}

void pluginsview_computetextsizes(PluginsView* self, psy_ui_RealSize size)
{
	const psy_ui_TextMetric* tm;
	
	tm = psy_ui_component_textmetric(&self->component);
	self->avgcharwidth = tm->tmAveCharWidth;
	self->lineheight = floor(tm->tmHeight * 1.5);
	self->columnwidth = tm->tmAveCharWidth * 45;
	self->identwidth = tm->tmAveCharWidth * 4;	
	self->numparametercols = (uintptr_t)psy_max(1, size.width /
		self->columnwidth);	
	psy_ui_component_setscrollstep_height(&self->component,
		psy_ui_value_makepx(self->lineheight));
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

uintptr_t plugintype(psy_Property* property, char* text)
{	
	uintptr_t rv;
	
	rv = (uintptr_t)psy_property_at_int(property, "type", -1);
	switch (rv) {
		case psy_audio_PLUGIN:
			strcpy(text, "psy");
		break;
		case psy_audio_LUA:
			strcpy(text, "lua");
		break;
		case psy_audio_VST:
			strcpy(text, "vst");
		break;
		case psy_audio_VSTFX:
			strcpy(text, "vst");
		break;
		case psy_audio_LADSPA:
			strcpy(text, "lad");
			break;
		default:
			strcpy(text, "int");
		break;
	}
	return rv;
}

uintptr_t pluginmode(psy_Property* property, char* text)
{			
	uintptr_t rv;

	rv = (uintptr_t)psy_property_at_int(property, "mode", -1);
	strcpy(text, rv == psy_audio_MACHMODE_FX ? "fx" : "gn");
	return rv;
}

uintptr_t pluginenabled(const PluginsView* self, psy_Property* property)
{
	uintptr_t mode;
	
	mode = psy_property_at_int(property, "mode", psy_audio_MACHMODE_FX);
	if (self->effectsenabled && mode == psy_audio_MACHMODE_FX) {
		return TRUE;
	}
	if (self->generatorsenabled && mode == psy_audio_MACHMODE_GENERATOR) {
		return TRUE;
	}
	return FALSE;
}

void pluginsview_onpreferredscrollsize(PluginsView* self, const psy_ui_Size* limit,
	psy_ui_Size* rv)
{
	if (self->currplugins) {		
		psy_ui_RealSize parentsize;
		
		parentsize = psy_ui_component_innersize_px(psy_ui_component_parent(&self->component));
		pluginsview_computetextsizes(self, parentsize);
		rv->width = psy_ui_value_makepx(parentsize.width);
		rv->height = psy_ui_value_makepx(self->lineheight *
			pluginsview_numlines(self));
	}
}

void pluginsview_onkeydown(PluginsView* self, psy_ui_KeyEvent* ev)
{
	if (self->selectedplugin) {
		psy_Property* plugin;
		uintptr_t col;
		uintptr_t row;

		col = 0;
		row = 0;
		plugin = NULL;
		pluginsview_cursorposition(self, self->selectedplugin, &col, &row);
		switch (ev->keycode) {
			case psy_ui_KEY_RETURN:
				if (self->selectedplugin) {
					psy_signal_emit(&self->signal_selected, self, 1,
						self->selectedplugin);					
					psy_ui_keyevent_stoppropagation(ev);
				}
				break;
			case psy_ui_KEY_DELETE:
				if (self->selectedplugin) {
					// psy_Property* p;
					// p = psy_property_find(self->workspace->plugincatcher.plugins,
					//		psy_property_key(self->selectedplugin),
					// 		PSY_PROPERTY_TYPE_NONE);
					// if (!self->onlyfavorites && p) {
						// psy_property_remove(self->workspace->plugincatcher.plugins, p);
					// } else {						
						//psy_property_set_int(p, "favorite", 0);
					// }
					// psy_audio_plugincatcher_save(&self->workspace->plugincatcher);
					// psy_signal_emit(&self->workspace->plugincatcher.signal_changed,
					//	&self->workspace->plugincatcher, 0);
					psy_ui_keyevent_stoppropagation(ev);
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
		if (self->currplugins && !psy_property_empty(self->currplugins)) {
			self->selectedplugin = psy_property_first(self->currplugins);
			psy_signal_emit(&self->signal_changed, self, 1,
				self->selectedplugin);
			psy_ui_component_invalidate(&self->component);
		}
	}	
}

uintptr_t pluginsview_visilines(PluginsView* self)
{
	psy_ui_RealSize size;

	size = psy_ui_component_innersize_px(&self->component);
	return (uintptr_t)(size.height / self->lineheight);
}

uintptr_t pluginsview_topline(PluginsView* self)
{
	return (uintptr_t)(psy_ui_component_scrolltoppx(&self->component)
		/ self->lineheight);
}

uintptr_t pluginsview_numlines(const PluginsView* self)
{
	return psy_property_size(self->currplugins) /
		self->numparametercols + 1;
}

void pluginsview_settopline(PluginsView* self, intptr_t line)
{
	
	psy_ui_component_setscrolltop(&self->component,
		psy_ui_value_makepx(line * self->lineheight));
}

void pluginsview_cursorposition(PluginsView* self, psy_Property* plugin,
	intptr_t* col, intptr_t* row)
{		
	if (plugin && self->currplugins) {
		uintptr_t index;

		pluginsview_computetextsizes(self,
			psy_ui_component_innersize_px(&self->component));
		index = psy_property_index(plugin);
		*row = index / self->numparametercols;
		*col = index % self->numparametercols;
		return;
	}
	*col = 0;
	*row = 0;	
}

psy_Property* pluginsview_pluginbycursorposition(PluginsView* self, intptr_t col, intptr_t row)
{				
	if (self->plugins) {		
		pluginsview_computetextsizes(self, 
			psy_ui_component_innersize_px(&self->component));
		return psy_property_at_index(self->currplugins,
			self->numparametercols * row + col);
	}
	return NULL;
}

void pluginsview_onmousedown(PluginsView* self, psy_ui_MouseEvent* ev)
{	
	if (ev->button == 1) {		
		pluginsview_hittest(self, ev->pt.x, ev->pt.y);
		psy_ui_component_invalidate(&self->component);
		psy_signal_emit(&self->signal_changed, self, 1,
			self->selectedplugin);		
		psy_ui_component_setfocus(&self->component);		
	}
	pluginsview_super_vtable.onmousedown(&self->component, ev);
}

void pluginsview_hittest(PluginsView* self, double x, double y)
{				
	if (self->plugins) {
		psy_List* p;
		double cpx;
		double cpy;

		pluginsview_computetextsizes(self,
			psy_ui_component_innersize_px(&self->component));
		for (p = psy_property_begin(self->currplugins), cpx = 0, cpy = 0;
				p != NULL; psy_list_next(&p)) {
			psy_ui_RealRectangle r;

			psy_ui_setrectangle(&r, cpx, cpy, self->columnwidth,
				self->lineheight);
			if (psy_ui_realrectangle_intersect(&r,
					psy_ui_realpoint_make(x, y))) {
				if (pluginenabled(self, (psy_Property*)psy_list_entry(p))) {
					self->selectedplugin = (psy_Property*)psy_list_entry(p);
				}
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
//		workspace_selectview(self->workspace, VIEW_ID_MACHINEVIEW,
//			SECTION_ID_MACHINEVIEW_WIRES, 0);
		psy_ui_mouseevent_stoppropagation(ev);		
	}	
}

void pluginsview_onfilterchanged(PluginsView* self, NewMachineFilter* sender)
{
	pluginsview_filter(self);	
}

void pluginsview_filter(PluginsView* self)
{
	if (self->filter && self->plugins) {
		pluginsview_clearfilter(self);
		self->filteredplugins = search(self->plugins, self->filter);
		self->currplugins = self->filteredplugins;
		psy_signal_emit(&self->signal_changed, self, 1,
			self->selectedplugin);
		psy_ui_component_setscrolltop(&self->component, psy_ui_value_zero());
		psy_ui_component_updateoverflow(&self->component);
		psy_ui_component_invalidate(&self->component);
	}
}

void pluginsview_onsortchanged(PluginsView* self, NewMachineSort* sender)
{
	pluginsview_sort(self, sender->mode);
}

void pluginsview_sort(PluginsView* self, NewMachineSortMode mode)
{
	if (self->sort && self->plugins) {
		psy_Property* sorted;
		psy_fp_comp fp_compare;

		fp_compare = NULL;
		switch (mode) {
		case NEWMACHINESORTMODE_FAVORITE:
			fp_compare = newmachine_comp_favorite;
			break;
		case NEWMACHINESORTMODE_NAME:
			fp_compare = newmachine_comp_name;
			break;
		case NEWMACHINESORTMODE_TYPE:
			fp_compare = newmachine_comp_type;
			break;
		case NEWMACHINESORTMODE_MODE:
			fp_compare = newmachine_comp_mode;
			break;
		default:
			break;
		}
		if (fp_compare) {
			sorted = newmachine_sort(self->plugins, fp_compare);
			pluginsview_setplugins(self, sorted);
			psy_signal_emit(&self->signal_changed, self, 1,
				self->selectedplugin);
		}
	}
}

void pluginsview_ondragstart(PluginsView* self, psy_ui_DragEvent* ev)
{
	if (self->selectedplugin) {		
		ev->dataTransfer = psy_property_clone(self->selectedplugin);
	}
}

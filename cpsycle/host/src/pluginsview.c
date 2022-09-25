/*
** This source is free software; you can redistribute itand /or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "pluginsview.h"
/* host */
#include "styles.h"
/* ui */
#include <uiapp.h>
/* audio */
#include <plugincatcher.h>
/* container */
#include <qsort.h>
/* platform */
#include "../../detail/portable.h"
#include "../../detail/strcasestr.h"

static void plugindisplayname(psy_Property*, char* text);
static uintptr_t plugintype(psy_Property*, char* text);
static uintptr_t pluginmode(psy_Property*, char* text);
static psy_Property* search(psy_Property* source, PluginFilter*);
static void searchfilter(psy_Property* plugin, PluginFilter*,
	psy_Property* parent);
static int isplugin(int type);

/* Quicksort callbacks */
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
		psy_Table propertiesptr;

		num = psy_property_size(source);
		psy_table_init(&propertiesptr);		
		p = psy_property_begin(source);
		for (i = 0; p != NULL && i < num; psy_list_next(&p), ++i) {
			psy_table_insert(&propertiesptr, i,
				(psy_Property*)psy_list_entry(p));
		}
		psy_qsort(&propertiesptr,
			(psy_fp_set_index_double)psy_table_insert,
			(psy_fp_index_double)psy_table_at,
			0, (int)(num - 1), comp);
		rv = psy_property_allocinit_key(NULL);
		for (i = 0; i < num; ++i) {
			psy_property_append_property(rv, psy_property_clone(
				psy_table_at(&propertiesptr, i)));
		}		
		psy_table_dispose(&propertiesptr);
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
	if (psy_strlen(left) == 0) {
		left = psy_property_key(p);
	}
	right = psy_property_at_str(q, "name", "");
	if (psy_strlen(right) == 0) {
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

/* NewMachineSelection */
void newmachineselection_init(NewMachineSelection* self)
{
	self->items = NULL;
}

void newmachineselection_dispose(NewMachineSelection* self)
{
	psy_list_free(self->items);
	self->items = NULL;
}

void newmachineselection_clear(NewMachineSelection* self)
{
	psy_list_free(self->items);
	self->items = NULL;
}

void newmachineselection_select(NewMachineSelection* self, uintptr_t index)
{
	if (!psy_list_findentry(self->items, (void*)index)) {
		psy_list_append(&self->items, (void*)index);
	}
}

void newmachineselection_singleselect(NewMachineSelection* self, uintptr_t index)
{
	newmachineselection_clear(self);
	psy_list_append(&self->items, (void*)index);
}

void newmachineselection_selectall(NewMachineSelection* self, uintptr_t size)
{
	uintptr_t i;

	newmachineselection_clear(self);
	for (i = 0; i < size; ++i) {
		psy_list_append(&self->items, (void*)i);
	}	
}

void newmachineselection_deselect(NewMachineSelection* self, uintptr_t index)
{
	psy_List* p;

	p = psy_list_findentry(self->items, (void*)index);
	if (p) {
		psy_list_remove(&self->items, p);
	}	
}

void newmachineselection_toggle(NewMachineSelection* self, uintptr_t index)
{
	psy_List* p;

	p = psy_list_findentry(self->items, (void*)index);
	if (p) {
		psy_list_remove(&self->items, p);
	} else {
		psy_list_append(&self->items, (void*)index);
	}
}

bool newmachineselection_isselected(const NewMachineSelection* self,
	uintptr_t index)
{
	return psy_list_findentry(self->items, (void*)index) != NULL;
}

uintptr_t newmachineselection_first(const NewMachineSelection* self)
{
	if (self->items) {
		return (uintptr_t)self->items->entry;
	}
	return psy_INDEX_INVALID;
}

/* NewMachineSort */
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
		(type == psy_audio_LADSPA) || 
		(type == psy_audio_LV2);
}

/* PluginFilterGroup */

void pluginfilteritem_init(PluginFilterItem* self, uintptr_t key,
	const char* name, bool active)
{
	self->key = key;
	self->name = psy_strdup(name);
	self->active = active;
}

void pluginfilteritem_dispose(PluginFilterItem* self)
{	
	free(self->name);
	self->name = NULL;	
}

PluginFilterItem* pluginfilteritem_allocinit(uintptr_t key, const char* name,
	bool active)
{
	PluginFilterItem* rv;
	
	rv = (PluginFilterItem*)malloc(sizeof(PluginFilterItem));
	if (rv) {
		pluginfilteritem_init(rv, key, name, active);
	}
	return rv;
}

/* PluginFilterGroup */

static PluginFilterItem* pluginfiltergroup_item(PluginFilterGroup*,
	uintptr_t key);

void pluginfiltergroup_init(PluginFilterGroup* self, uintptr_t id,
	const char* label)
{
	self->items = NULL;	
	self->id = id;
	self->label = psy_strdup(label);
}

void pluginfiltergroup_dispose(PluginFilterGroup* self)
{
	psy_list_deallocate(&self->items, (psy_fp_disposefunc)
		pluginfilteritem_dispose);
	free(self->label);
	self->label = NULL;
}

PluginFilterGroup* pluginfiltergroup_allocinit(uintptr_t id,
	const char* label)
{
	PluginFilterGroup* rv;
	
	rv = (PluginFilterGroup*)malloc(sizeof(PluginFilterGroup));
	if (rv) {
		pluginfiltergroup_init(rv, id, label);
	}
	return rv;
}


bool pluginfiltergroup_add(PluginFilterGroup* self,
	uintptr_t key, const char* name, bool active)
{
	if (key == psy_INDEX_INVALID) {
		return FALSE;
	}
	if (pluginfiltergroup_item(self, key)) {
		return FALSE;
	}
	psy_list_append(&self->items, pluginfilteritem_allocinit(key, name,
		active));
	return TRUE;
}

void pluginfiltergroup_clear(PluginFilterGroup* self)
{
	psy_list_deallocate(&self->items, (psy_fp_disposefunc)
		pluginfilteritem_dispose);
}

bool pluginfiltergroup_remove(PluginFilterGroup* self, uintptr_t key)
{
	psy_List* p;
	
	for (p = self->items; p != NULL; p = p->next) {
		PluginFilterItem* curr;
		
		curr = (PluginFilterItem*)p->entry;
		if (curr->key && key == curr->key) {
			pluginfilteritem_dispose(curr);
			psy_list_remove(&self->items, p);
			break;
		}
	}	
	return (p != NULL);
}

void pluginfiltergroup_select(PluginFilterGroup* self, uintptr_t key)
{
	PluginFilterItem* item;
	
	item = pluginfiltergroup_item(self, key);
	if (item) {
		item->active = TRUE;
	}	
}

void pluginfiltergroup_select_all(PluginFilterGroup* self)
{
	psy_List* p;

	for (p = self->items; p != NULL; p = p->next) {
		PluginFilterItem* curr;
		
		curr = (PluginFilterItem*)p->entry;
		curr->active = TRUE;
	}	
}

void pluginfiltergroup_deselect(PluginFilterGroup* self, uintptr_t key)
{
	PluginFilterItem* item;
	
	item = pluginfiltergroup_item(self, key);
	if (item) {
		item->active = FALSE;
	}
}

void pluginfiltergroup_deselect_all(PluginFilterGroup* self)
{
	psy_List* p;

	for (p = self->items; p != NULL; p = p->next) {
		PluginFilterItem* curr;
		
		curr = (PluginFilterItem*)p->entry;
		curr->active = FALSE;
	}	
}

void pluginfiltergroup_toggle(PluginFilterGroup* self, uintptr_t key)
{
	if (pluginfiltergroup_selected(self, key)) {
		pluginfiltergroup_deselect(self, key);
	} else {
		pluginfiltergroup_select(self, key);
	}	
}

bool pluginfiltergroup_selected(const PluginFilterGroup* self, uintptr_t key)
{
	PluginFilterItem* item;
	
	item = pluginfiltergroup_item((PluginFilterGroup*)self, key);
	if (item) {
		return item->active;
	}
	return FALSE;
}

bool pluginfiltergroup_exists(const PluginFilterGroup* self, uintptr_t key)
{
	return (pluginfiltergroup_item((PluginFilterGroup*)self, key) != NULL);
}

PluginFilterItem* pluginfiltergroup_item(PluginFilterGroup* self, uintptr_t id)
{
	PluginFilterItem* rv;
	
	psy_List* p;
	rv = NULL;
	for (p = self->items; p != NULL; p = p->next) {
		PluginFilterItem* curr;
		
		curr = (PluginFilterItem*)p->entry;
		if (curr->key && id == curr->key) {
			rv = curr;
			break;
		}
	}
	return rv;
}

/* PluginFilter */

void pluginfilter_init(PluginFilter* self)
{
	assert(self);

	self->search_text = NULL;	
	pluginfiltergroup_init(&self->mode, psy_strhash("mode"), "Mode");
	pluginfiltergroup_add(&self->mode,
		psy_strhash("generator"), "Generator", TRUE);
	pluginfiltergroup_add(&self->mode,
		psy_strhash("effect"), "Effect", TRUE);		
	pluginfiltergroup_init(&self->types, psy_strhash("type"), "Type");
	pluginfiltergroup_add(&self->types,
		psy_strhash("intern"), "Internal", TRUE);
	pluginfiltergroup_add(&self->types,
		psy_strhash("native"), "Native", TRUE);
	pluginfiltergroup_add(&self->types,
		psy_strhash("vst"), "Vst", TRUE);
	pluginfiltergroup_add(&self->types,
		psy_strhash("lua"), "Lua", TRUE);		
	pluginfiltergroup_add(&self->types,
		psy_strhash("ladspa"), "Ladspa", TRUE);
	pluginfiltergroup_add(&self->types,
		psy_strhash("lv2"), "LV2", TRUE);
	pluginfiltergroup_init(&self->categories, psy_strhash("categories"),
		"newmachine.categories");
	pluginfiltergroup_init(&self->sort, psy_strhash("sort"),
		"newmachine.sort");		
	pluginfiltergroup_add(&self->sort, NEWMACHINESORTMODE_FAVORITE,
		"newmachine.favorite", FALSE);
	pluginfiltergroup_add(&self->sort, NEWMACHINESORTMODE_NAME,
		"newmachine.name",  FALSE);
	pluginfiltergroup_add(&self->sort, NEWMACHINESORTMODE_TYPE,
		"newmachine.type", FALSE);
	pluginfiltergroup_add(&self->sort, NEWMACHINESORTMODE_MODE,
		"newmachine.mode", FALSE);	
	psy_signal_init(&self->signal_changed);
}

void pluginfilter_dispose(PluginFilter* self)
{		
	pluginfiltergroup_dispose(&self->mode);
	pluginfiltergroup_dispose(&self->types);
	pluginfiltergroup_dispose(&self->categories);
	pluginfiltergroup_dispose(&self->sort);
	psy_signal_dispose(&self->signal_changed);
	free(self->search_text);
	self->search_text = NULL;	
}

void pluginfilter_notify(PluginFilter* self)
{	
	psy_signal_emit(&self->signal_changed, self, 0);
}

void pluginfilter_select_all(PluginFilter* self)
{	
	psy_strreset(&self->search_text, "");
	pluginfiltergroup_select_all(&self->mode);
	pluginfiltergroup_select_all(&self->types);	
	pluginfiltergroup_select_all(&self->categories);
	pluginfilter_notify(self);
}

void pluginfilter_set_search_text(PluginFilter* self, const char* text)
{
	psy_strreset(&self->search_text, text);
	pluginfilter_notify(self);
}

psy_Property* search(psy_Property* source, PluginFilter* filter)
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
	text = filter->search_text;
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

static bool internal_type(int type);

void searchfilter(psy_Property* plugin, PluginFilter* filter,
	psy_Property* parent)
{
	psy_Property* p;
	bool intern;
	int mactype;
	int macmode;	
	const char* category;
				
	mactype = psy_property_at_int(plugin, "type", psy_audio_UNDEFINED);
	intern = internal_type(mactype);
	macmode = psy_property_at_int(plugin, "mode", 0);
	category = psy_property_at_str(plugin, "category", "");			
	if ((psy_strlen(category) > 0) && (!pluginfiltergroup_selected(
			&filter->categories, psy_strhash(category)))) {
		return;
	}	
	if (!pluginfiltergroup_selected(&filter->types, psy_strhash("vst")) &&
			(mactype == psy_audio_VST || mactype == psy_audio_VSTFX)) {		
		return;
	}	
	if (!pluginfiltergroup_selected(&filter->types, psy_strhash("native")) &&
			mactype == psy_audio_PLUGIN) {
		return;
	}	
	if (!pluginfiltergroup_selected(&filter->types, psy_strhash("ladspa")) &&
			mactype == psy_audio_LADSPA) {		
		return;
	}
	if (!pluginfiltergroup_selected(&filter->types, psy_strhash("lv2")) &&
			mactype == psy_audio_LV2) {
		return;
	}	
	if (!pluginfiltergroup_selected(&filter->types, psy_strhash("lua")) &&
			mactype == psy_audio_LUA) {
		return;
	}
	if (!pluginfiltergroup_selected(&filter->types, psy_strhash("intern")) &&
			intern) {
		return;
	}	
	if (!pluginfiltergroup_selected(&filter->mode, psy_strhash("effect")) &&
			(macmode == psy_audio_MACHMODE_FX)) {
		return;		
	}
	if (!pluginfiltergroup_selected(&filter->mode, psy_strhash("generator")) &&
			(macmode == psy_audio_MACHMODE_GENERATOR)) {
		return;
	}
	psy_property_append_property(parent, psy_property_clone(plugin));
}

bool internal_type(int type)
{	
	return (!(type == psy_audio_PLUGIN ||
		type == psy_audio_LUA ||
		type == psy_audio_VST ||
		type == psy_audio_VSTFX ||
		type == psy_audio_LADSPA));
}

/* PluginsView */

/* prototypes */
static void pluginsview_on_destroyed(PluginsView*);
static void pluginsview_ondraw(PluginsView*, psy_ui_Graphics*);
static void pluginsview_drawitem(PluginsView*, psy_ui_Graphics*, psy_Property*,
	psy_ui_RealPoint topleft, bool sel);
static void pluginsview_onpreferredscrollsize(PluginsView*, const psy_ui_Size* limit,
	psy_ui_Size* rv);
static void pluginsview_on_key_down(PluginsView*, psy_ui_KeyboardEvent*);
static void pluginsview_cursorposition(PluginsView*, psy_Property* plugin,
	uintptr_t* col, uintptr_t* row);
static psy_Property* pluginsview_pluginbycursorposition(PluginsView*,
	uintptr_t col, uintptr_t row);
static void pluginsview_on_mouse_down(PluginsView*, psy_ui_MouseEvent*);
static void pluginsview_on_mouse_up(PluginsView*, psy_ui_MouseEvent*);
static void pluginsview_onmousedoubleclick(PluginsView*, psy_ui_MouseEvent*);
static uintptr_t pluginsview_hittest(PluginsView*, psy_ui_RealPoint);
static void pluginsview_computetextsizes(PluginsView*, double width);
static uintptr_t pluginsview_visilines(PluginsView*);
static uintptr_t pluginsview_topline(PluginsView*);
static void pluginsview_settopline(PluginsView*, intptr_t line);
static uintptr_t pluginsview_numlines(const PluginsView*);
static uintptr_t pluginenabled(const PluginsView*, psy_Property* property);
static void pluginsview_onfilterchanged(PluginsView*, PluginFilter* sender);
static void pluginsview_onsortchanged(PluginsView*, NewMachineSort* sender);
static void pluginsview_ondragstart(PluginsView*, psy_ui_DragEvent*);

/* vtable */
static psy_ui_ComponentVtable pluginsview_vtable;
static psy_ui_ComponentVtable pluginsview_super_vtable;
static bool pluginsview_vtable_initialized = FALSE;

static void pluginsview_vtable_init(PluginsView* self)
{
	if (!pluginsview_vtable_initialized) {
		pluginsview_vtable = *(self->component.vtable);
		pluginsview_super_vtable = pluginsview_vtable;
		pluginsview_vtable.on_destroyed =
			(psy_ui_fp_component)
			pluginsview_on_destroyed;
		pluginsview_vtable.ondraw =
			(psy_ui_fp_component_ondraw)
			pluginsview_ondraw;
		pluginsview_vtable.on_key_down =
			(psy_ui_fp_component_on_key_event)
			pluginsview_on_key_down;
		pluginsview_vtable.on_mouse_down =
			(psy_ui_fp_component_on_mouse_event)
			pluginsview_on_mouse_down;
		pluginsview_vtable.on_mouse_up =
			(psy_ui_fp_component_on_mouse_event)
			pluginsview_on_mouse_up;
		pluginsview_vtable.on_mouse_double_click =
			(psy_ui_fp_component_on_mouse_event)
			pluginsview_onmousedoubleclick;
		pluginsview_vtable.onpreferredsize =
			(psy_ui_fp_component_on_preferred_size)
			pluginsview_onpreferredscrollsize;
		pluginsview_vtable.ondragstart =
			(psy_ui_fp_component_on_drag_event)
			pluginsview_ondragstart;
		pluginsview_vtable_initialized = TRUE;
	}
	psy_ui_component_set_vtable(pluginsview_base(self), &pluginsview_vtable);
}

/* implementation */
void pluginsview_init(PluginsView* self, psy_ui_Component* parent)
{
	psy_ui_component_init(&self->component, parent, NULL);	
	pluginsview_vtable_init(self);	
	self->component.draggable = TRUE;
	psy_signal_init(&self->signal_selected);
	psy_signal_init(&self->signal_changed);	
	self->currplugins = NULL;
	self->plugins = NULL;
	self->filteredplugins = NULL;	
	self->filter = NULL;
	self->sort = NULL;
	self->generatorsenabled = TRUE;
	self->effectsenabled = TRUE;
	self->readonly = FALSE;
	newmachineselection_init(&self->selection);
	pluginsview_computetextsizes(self, 1024.0);
}

void pluginsview_on_destroyed(PluginsView* self)
{	
	psy_signal_dispose(&self->signal_selected);
	psy_signal_dispose(&self->signal_changed);
	pluginsview_clear(self);
	if (self->filter) {
		psy_signal_disconnect(&self->filter->signal_changed, self,
			pluginsview_onfilterchanged);
	}
	newmachineselection_dispose(&self->selection);	
}

void pluginsview_clear(PluginsView* self)
{
	if (self->filteredplugins) {
		psy_property_deallocate(self->filteredplugins);
		self->filteredplugins = NULL;
	}
	if (self->plugins) {		
		psy_property_deallocate(self->plugins);		
		self->plugins = NULL;
		newmachineselection_clear(&self->selection);		
		self->currplugins = NULL;
	}				
}

void pluginsview_clearfilter(PluginsView* self)
{
	newmachineselection_clear(&self->selection);
	self->currplugins = self->plugins;
	if (self->filteredplugins) {
		psy_property_deallocate(self->filteredplugins);
		self->filteredplugins = NULL;
	}
}

void pluginsview_setplugins(PluginsView* self, const psy_Property* property)
{
	pluginsview_clear(self);
	self->plugins = (property)
		? psy_property_clone(property)
		: NULL;
	self->currplugins = self->plugins;	
	psy_ui_component_invalidate(&self->component);
}

void pluginsview_set_filter(PluginsView* self, PluginFilter* filter)
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
		uintptr_t i;
		
		size = psy_ui_component_size_px(&self->component);
		pluginsview_computetextsizes(self, size.width);
		psy_ui_setbackgroundmode(g, psy_ui_TRANSPARENT);
		psy_ui_realpoint_init(&cp);
		odd = FALSE;				
		for (p = psy_property_begin(self->currplugins), i = 0;
				p != NULL; psy_list_next(&p), ++i) {			
			pluginsview_drawitem(self, g, (psy_Property*)psy_list_entry(p),
				cp, newmachineselection_isselected(&self->selection, i));
			cp.x += self->columnwidth;
			if (cp.x >= self->numparametercols * self->columnwidth) {
				cp.x = 0.0;
				cp.y += self->line_height;
				odd = !odd;
				if (odd && p->next) {
					// psy_ui_RealRectangle r;

					// psy_ui_realrectangle_init_all(&r,
					//	psy_ui_realpoint_make(0, cp.y),
					//	psy_ui_realsize_make(size.width, self->line_height));
					// psy_ui_drawsolidrectangle(g, r, oddlinebgcolour);
				}
			}
		}
	}
}

void pluginsview_drawitem(PluginsView* self, psy_ui_Graphics* g,
	psy_Property* property, psy_ui_RealPoint topleft, bool sel)
{
	char text[128];	
	psy_ui_Style* itemstyle;
	psy_ui_Colour bgcolour;

	if (sel) {
		itemstyle = psy_ui_style(psy_ui_STYLE_LIST_ITEM_SELECT);
	} else {
		itemstyle = psy_ui_style(psy_ui_STYLE_LIST_ITEM);
	}

	if (itemstyle->background.colour.mode.transparent) {
		bgcolour = psy_ui_component_background_colour(&self->component);
	} else {
		bgcolour = itemstyle->background.colour;
	}
	if (!itemstyle->colour.mode.transparent) {
		psy_ui_set_text_colour(g, itemstyle->colour);
	} else {
		psy_ui_set_text_colour(g, psy_ui_component_colour(&self->component));
	}
	if (!itemstyle->background.colour.mode.transparent) {
		psy_ui_drawsolidrectangle(g,
			psy_ui_realrectangle_make(
				topleft,
				psy_ui_realsize_make(self->columnwidth - 5, self->line_height)),
			bgcolour);
	}			
	/* if (pluginenabled(self, property)) {
		psy_ui_set_text_colour(g, psy_ui_colour_make(0x00CACACA));
	} else {
		psy_ui_set_text_colour(g, psy_ui_colour_make(0x00666666));
	}*/		
	plugindisplayname(property, text);	
	psy_ui_textout(g, psy_ui_realpoint_make(topleft.x, topleft.y + 2), text, psy_strlen(text));
	plugintype(property, text);
	psy_ui_textout(g, psy_ui_realpoint_make(topleft.x + self->columnwidth - self->avgcharwidth * 7,
		topleft.y + 2), text, psy_strlen(text));
	if (pluginmode(property, text) == psy_audio_MACHMODE_FX) {
		psy_ui_set_text_colour(g, psy_ui_colour_make(0x00B1C8B0));
	} else {		
		psy_ui_set_text_colour(g, psy_ui_colour_make(0x00D1C5B6));
	}
	psy_ui_textout(g, psy_ui_realpoint_make(topleft.x + self->columnwidth - 10 * self->avgcharwidth,
		topleft.y + 2), text, psy_strlen(text));
}

void pluginsview_computetextsizes(PluginsView* self, double width)
{
	const psy_ui_TextMetric* tm;
	
	tm = psy_ui_component_textmetric(&self->component);
	self->avgcharwidth = tm->tmAveCharWidth;
	self->line_height = floor(tm->tmHeight * 1.5);
	self->columnwidth = tm->tmAveCharWidth * 45;
	self->identwidth = tm->tmAveCharWidth * 4;	
	self->numparametercols = (uintptr_t)psy_max(1, width /
		self->columnwidth);	
	psy_ui_component_set_scroll_step_height(&self->component,
		psy_ui_value_make_px(self->line_height));
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
		case psy_audio_LV2:
			strcpy(text, "lv2");
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
		if (limit) {
			rv->width = limit->width;			
		} else {
			rv->width = psy_ui_value_make_px(65535.0);
		}
		pluginsview_computetextsizes(self, psy_ui_value_px(&rv->width,
			psy_ui_component_textmetric(&self->component), NULL));
		rv->height = psy_ui_value_make_px(self->line_height *
			pluginsview_numlines(self));
	}
}

void pluginsview_on_key_down(PluginsView* self, psy_ui_KeyboardEvent* ev)
{
	psy_Property* selected;

	if (psy_ui_keyboardevent_keycode(ev) == psy_ui_KEY_CONTROL) {
		psy_ui_keyboardevent_stop_propagation(ev);
		return;
	}
	if (psy_ui_keyboardevent_ctrlkey(ev) && psy_ui_keyboardevent_keycode(ev) == psy_ui_KEY_A) {
		if (self->plugins) {
			newmachineselection_selectall(&self->selection,
				psy_property_size(self->currplugins));
			psy_ui_component_invalidate(&self->component);
			psy_signal_emit(&self->signal_changed, self, 0);
		}
		psy_ui_keyboardevent_stop_propagation(ev);
		return;
	}
	selected = pluginsview_selectedplugin(self);
	if (self->selection.items || selected) {
		psy_Property* plugin;
		uintptr_t col;
		uintptr_t row;

		col = 0;
		row = 0;
		plugin = NULL;
		pluginsview_cursorposition(self, selected, &col, &row);
		switch (psy_ui_keyboardevent_keycode(ev)) {
			case psy_ui_KEY_RETURN:
				if (selected) {
					psy_signal_emit(&self->signal_selected, self, 0);
					psy_ui_keyboardevent_stop_propagation(ev);
				}
				break;
			case psy_ui_KEY_DELETE:
				if (self->plugins && self->selection.items && !self->readonly) {	
					psy_List* p;

					for (p = self->selection.items; p != NULL; p = p->next) {
						uintptr_t index;
						psy_Property* plugin;

						index = (uintptr_t)p->entry;
						plugin = psy_property_at_index(self->plugins, index);
						if (plugin) {
							psy_property_remove(self->plugins, plugin);
						}						
					}					
					newmachineselection_clear(&self->selection);
					psy_ui_component_align(psy_ui_component_parent(
						psy_ui_component_parent(&self->component)));
					psy_ui_component_invalidate(&self->component);
					psy_ui_keyboardevent_stop_propagation(ev);
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
				psy_ui_keyboardevent_stop_propagation(ev);
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
				psy_ui_keyboardevent_stop_propagation(ev);
				break;
			case psy_ui_KEY_PRIOR:
				if (row > 0) {
					row = psy_max(0, row - 4);
					if (row < pluginsview_topline(self)) {
						pluginsview_settopline(self, row);
					}
				}
				psy_ui_keyboardevent_stop_propagation(ev);
				break;
			case psy_ui_KEY_NEXT:
				row += 4;
				if (row >= pluginsview_numlines(self) - 1) {
					row = pluginsview_numlines(self) - 1;
				}
				if (row > pluginsview_topline(self) + pluginsview_visilines(self)) {
					pluginsview_settopline(self, row - pluginsview_visilines(self));
				}
				psy_ui_keyboardevent_stop_propagation(ev);
				break;
			case psy_ui_KEY_LEFT:			
				if (col > 0) {
					--col;		
				}
				psy_ui_keyboardevent_stop_propagation(ev);
				break;
			case psy_ui_KEY_RIGHT: {					
				++col;				
				psy_ui_keyboardevent_stop_propagation(ev);
				break;
			}
			default:			
				break;
		}		
		plugin = pluginsview_pluginbycursorposition(self, col, row);
		if (plugin) {
			newmachineselection_singleselect(&self->selection,
				row * self->numparametercols + col);
			psy_signal_emit(&self->signal_changed, self, 0);
			psy_ui_component_invalidate(&self->component);
		}
	} else
	if (psy_ui_keyboardevent_keycode(ev) >= psy_ui_KEY_LEFT && psy_ui_keyboardevent_keycode(ev) <= psy_ui_KEY_DOWN) {
		if (self->currplugins && !psy_property_empty(self->currplugins)) {
			newmachineselection_singleselect(&self->selection, 0);			
			psy_signal_emit(&self->signal_changed, self, 0);
			psy_ui_component_invalidate(&self->component);
		}
	}	
}

uintptr_t pluginsview_visilines(PluginsView* self)
{
	psy_ui_RealSize size;

	size = psy_ui_component_size_px(&self->component);
	return (uintptr_t)(size.height / self->line_height);
}

uintptr_t pluginsview_topline(PluginsView* self)
{
	return (uintptr_t)(psy_ui_component_scroll_top_px(&self->component)
		/ self->line_height);
}

uintptr_t pluginsview_numlines(const PluginsView* self)
{
	if (self->numparametercols > 0) {
		return psy_property_size(self->currplugins) /
			self->numparametercols + 1;
	}
	return 1;
}

void pluginsview_settopline(PluginsView* self, intptr_t line)
{
	
	psy_ui_component_set_scroll_top(&self->component,
		psy_ui_value_make_px(line * self->line_height));
}

void pluginsview_cursorposition(PluginsView* self, psy_Property* plugin,
	uintptr_t* col, uintptr_t* row)
{		
	if (plugin && self->currplugins) {
		uintptr_t index;
		psy_ui_RealSize size;

		size = psy_ui_component_size_px(&self->component);
		pluginsview_computetextsizes(self, size.width);
		index = psy_property_index(plugin);
		*row = index / self->numparametercols;
		*col = index % self->numparametercols;
		return;
	}
	*col = 0;
	*row = 0;	
}

psy_Property* pluginsview_pluginbycursorposition(PluginsView* self, uintptr_t col, uintptr_t row)
{				
	if (self->plugins) {	
		psy_ui_RealSize size;

		size = psy_ui_component_size_px(&self->component);
		pluginsview_computetextsizes(self, size.width);
		return psy_property_at_index(self->currplugins,
			self->numparametercols * row + col);
	}
	return NULL;
}

void pluginsview_on_mouse_down(PluginsView* self, psy_ui_MouseEvent* ev)
{	
	self->multidrag = FALSE;
	if (psy_ui_mouseevent_button(ev) == 1) {
		uintptr_t index;

		index = pluginsview_hittest(self, psy_ui_mouseevent_pt(ev));
		if (index != psy_INDEX_INVALID) {			
			if (psy_ui_mouseevent_ctrl_key(ev)) {
				newmachineselection_toggle(&self->selection, index);
			} else {
				if (psy_list_size(self->selection.items) > 1 &&
					newmachineselection_isselected(&self->selection, index)) {
					self->dragpt = psy_ui_mouseevent_pt(ev);
					self->multidrag = TRUE;
					self->dragindex = index;
				} else {
					newmachineselection_singleselect(&self->selection, index);
				}
			}
			self->component.dbg = 400;
			psy_ui_component_invalidate(&self->component);
			psy_signal_emit(&self->signal_changed, self, 0);
			psy_ui_component_set_focus(&self->component);
		}
	}
	pluginsview_super_vtable.on_mouse_down(&self->component, ev);
}

void pluginsview_on_mouse_up(PluginsView* self, psy_ui_MouseEvent* ev)
{
	psy_ui_RealPoint pt;

	pt = psy_ui_mouseevent_pt(ev);
	if (self->multidrag && psy_ui_realpoint_equal(&self->dragpt, &pt)) {
		newmachineselection_singleselect(&self->selection, self->dragindex);
		psy_ui_component_invalidate(&self->component);
		psy_signal_emit(&self->signal_changed, self, 0);
		psy_ui_component_set_focus(&self->component);
	}
}


uintptr_t pluginsview_hittest(PluginsView* self, psy_ui_RealPoint pt)
{				
	if (self->plugins) {
		psy_List* p;
		double cpx;
		double cpy;
		psy_ui_RealSize size;
		uintptr_t i;

		size = psy_ui_component_size_px(&self->component);
		pluginsview_computetextsizes(self, size.width);
		for (p = psy_property_begin(self->currplugins), cpx = 0, cpy = 0, i = 0;
				p != NULL; psy_list_next(&p), ++i) {
			psy_ui_RealRectangle r;
			
			r = psy_ui_realrectangle_make(
					psy_ui_realpoint_make(cpx, cpy),
					psy_ui_realsize_make(self->columnwidth, self->line_height));
			if (psy_ui_realrectangle_intersect(&r, pt)) {				
				break;
			}		
			cpx += self->columnwidth;
			if (cpx >= self->numparametercols * self->columnwidth) {
				cpx = 0;
				cpy += self->line_height;
			}
		}
		if (p) {
			return i;
		}
	}
	return psy_INDEX_INVALID;
}

void pluginsview_onmousedoubleclick(PluginsView* self, psy_ui_MouseEvent* ev)
{
	if (newmachineselection_first(&self->selection) != psy_INDEX_INVALID) {		
		psy_signal_emit(&self->signal_selected, self, 0);
		psy_ui_mouseevent_stop_propagation(ev);		
	}	
}

void pluginsview_onfilterchanged(PluginsView* self, PluginFilter* sender)
{
	pluginsview_filter(self);	
}

void pluginsview_filter(PluginsView* self)
{
	if (self->filter && self->plugins) {
		pluginsview_clearfilter(self);
		self->filteredplugins = search(self->plugins, self->filter);
		self->currplugins = self->filteredplugins;
		psy_signal_emit(&self->signal_changed, self, 0);
		psy_ui_component_set_scroll_top(&self->component, psy_ui_value_zero());
		psy_ui_component_align_full(psy_ui_component_parent(
			psy_ui_component_parent(&self->component)));
		psy_ui_component_updateoverflow(psy_ui_component_parent(
			psy_ui_component_parent(&self->component)));		
		psy_ui_component_invalidate(psy_ui_component_parent(psy_ui_component_parent(&self->component)));
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
			fp_compare = (psy_fp_comp)newmachine_comp_favorite;
			break;
		case NEWMACHINESORTMODE_NAME:
			fp_compare = (psy_fp_comp)newmachine_comp_name;
			break;
		case NEWMACHINESORTMODE_TYPE:
			fp_compare = (psy_fp_comp)newmachine_comp_type;
			break;
		case NEWMACHINESORTMODE_MODE:
			fp_compare = (psy_fp_comp)newmachine_comp_mode;
			break;
		default:
			break;
		}
		if (fp_compare) {
			sorted = newmachine_sort(self->plugins, fp_compare);
			pluginsview_setplugins(self, sorted);
			psy_signal_emit(&self->signal_changed, self, 0);
		}
	}
}

psy_Property* pluginsview_selectedplugin(PluginsView* self)
{
	uintptr_t first;

	first = newmachineselection_first(&self->selection);
	if (first != psy_INDEX_INVALID) {
		return psy_property_at_index(self->currplugins, first);
	}
	return NULL;
}

void pluginsview_clearselection(PluginsView* self)
{
	if (!newmachineselection_empty(&self->selection)) {
		newmachineselection_clear(&self->selection);
		psy_ui_component_invalidate(&self->component);
	}
}

void pluginsview_enableall(PluginsView* self)
{
	self->effectsenabled = TRUE;
	self->generatorsenabled = TRUE;
	psy_ui_component_invalidate(&self->component);
}

void pluginsview_enablegenerators(PluginsView* self)
{
	self->generatorsenabled = TRUE;	
	psy_ui_component_invalidate(&self->component);
}

void pluginsview_preventgenerators(PluginsView* self)
{
	self->generatorsenabled = FALSE;
	psy_ui_component_invalidate(&self->component);
}

void pluginsview_enableeffects(PluginsView* self)
{
	self->effectsenabled = TRUE;
	psy_ui_component_invalidate(&self->component);
}

void pluginsview_preventeffects(PluginsView* self)
{
	self->effectsenabled = FALSE;
	psy_ui_component_invalidate(&self->component);
}

void pluginsview_ondragstart(PluginsView* self, psy_ui_DragEvent* ev)
{
	if (self->plugins && self->selection.items) {		
		psy_List* p;

		ev->dataTransfer = psy_property_allocinit_key(NULL);		
		for (p = self->selection.items; p != NULL; p = p->next) {
			psy_Property* plugin;

			plugin = psy_property_at_index(self->currplugins, (uintptr_t)p->entry);
			if (plugin) {				
				plugin = psy_property_append_section(ev->dataTransfer,
					psy_property_key(plugin));
				psy_property_append_str(plugin, "section",
					psy_property_key(self->plugins));
			}
		}
	}
}

// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "pluginsview.h"
// host
#include "styles.h"
// platform
#include "../../detail/portable.h"
#include "../../detail/strcasestr.h"

static void plugindisplayname(psy_Property*, char* text);
static uintptr_t plugintype(psy_Property*, char* text);
static uintptr_t pluginmode(psy_Property*, char* text);
static psy_Property* search(psy_Property* source, NewMachineFilter*);
static void searchfilter(psy_Property* plugin, NewMachineFilter*,
	psy_Property* parent);

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
static void pluginsview_ondestroy(PluginsView*, psy_ui_Component* component);
static void pluginsview_ondraw(PluginsView*, psy_ui_Graphics*);
static void pluginsview_drawitem(PluginsView*, psy_ui_Graphics*, psy_Property*,
	psy_ui_RealPoint topleft);
static void pluginsview_onpreferredsize(PluginsView*, const psy_ui_Size* limit,
	psy_ui_Size* rv);
static void pluginsview_onkeydown(PluginsView*, psy_ui_KeyEvent*);
static void pluginsview_cursorposition(PluginsView*, psy_Property* plugin,
	intptr_t* col, intptr_t* row);
static psy_Property* pluginsview_pluginbycursorposition(PluginsView*,
	intptr_t col, intptr_t row);
static void pluginsview_onmousedown(PluginsView*, psy_ui_MouseEvent*);
static void pluginsview_onmousedoubleclick(PluginsView*, psy_ui_MouseEvent*);
static void pluginsview_hittest(PluginsView*, double x, double y);
static void pluginsview_computetextsizes(PluginsView*, const psy_ui_Size*);
static uintptr_t pluginsview_visilines(PluginsView*);
static uintptr_t pluginsview_topline(PluginsView*);
static void pluginsview_settopline(PluginsView*, intptr_t line);
static uintptr_t pluginsview_numlines(const PluginsView*);
static uintptr_t pluginenabled(const PluginsView*, psy_Property* property);
static void pluginsview_onfilterchanged(PluginsView*, NewMachineFilter* sender);

static psy_ui_ComponentVtable pluginsview_vtable;
static int pluginsview_vtable_initialized = 0;

static void pluginsview_vtable_init(PluginsView* self)
{
	if (!pluginsview_vtable_initialized) {
		pluginsview_vtable = *(self->component.vtable);				
		pluginsview_vtable.ondraw = (psy_ui_fp_component_ondraw) pluginsview_ondraw;		
		pluginsview_vtable.onkeydown = (psy_ui_fp_component_onkeyevent)
			pluginsview_onkeydown;
		pluginsview_vtable.onmousedown =
			(psy_ui_fp_component_onmouseevent)
			pluginsview_onmousedown;
		pluginsview_vtable.onmousedoubleclick =
			(psy_ui_fp_component_onmouseevent)
			pluginsview_onmousedoubleclick;
		pluginsview_vtable.onpreferredsize =
			(psy_ui_fp_component_onpreferredsize)
			pluginsview_onpreferredsize;		
	}
	self->component.vtable = &pluginsview_vtable;
}

void pluginsview_init(PluginsView* self, psy_ui_Component* parent,
	bool favorites, Workspace* workspace)
{	
	psy_ui_Size size;

	psy_ui_component_init(&self->component, parent, NULL);
	pluginsview_vtable_init(self);
	newmachinefilter_init(&self->filters);
	self->workspace = workspace;	
	self->mode = NEWMACHINE_APPEND;
	self->plugins = NULL;	
	psy_ui_component_doublebuffer(&self->component);	
	psy_signal_connect(&self->component.signal_destroy, self,
		pluginsview_ondestroy);
	self->selectedplugin = NULL;
	self->generatorsenabled = TRUE;
	self->effectsenabled = TRUE;
	psy_signal_init(&self->signal_selected);
	psy_signal_init(&self->signal_changed);	
	size = psy_ui_component_size(&self->component);
	pluginsview_computetextsizes(self, &size);	
	psy_signal_connect(&self->filters.signal_changed, self,
		pluginsview_onfilterchanged);
}

void pluginsview_ondestroy(PluginsView* self, psy_ui_Component* component)
{
	psy_signal_dispose(&self->signal_selected);
	psy_signal_dispose(&self->signal_changed);
	if (self->plugins) {
		psy_property_deallocate(self->plugins);
	}
	newmachinefilter_dispose(&self->filters);
}

void pluginsview_setplugins(PluginsView* self, const psy_Property* property)
{
	if (self->plugins) {
		psy_property_deallocate(self->plugins);
	}
	self->plugins = psy_property_clone(property);
	self->selectedplugin = NULL;
}

void pluginsview_ondraw(PluginsView* self, psy_ui_Graphics* g)
{	
	if (self->plugins) {
		psy_ui_Size size;
		psy_List* p;
		psy_ui_RealPoint cp;		
		
		size = psy_ui_component_size(&self->component);
		pluginsview_computetextsizes(self, &size);
		psy_ui_setbackgroundmode(g, psy_ui_TRANSPARENT);
		psy_ui_realpoint_init(&cp);
		for (p = psy_property_begin(self->plugins);
				p != NULL; psy_list_next(&p)) {
			pluginsview_drawitem(self, g, (psy_Property*)psy_list_entry(p),
				cp);
			cp.x += self->columnwidth;
			if (cp.x >= self->numparametercols * self->columnwidth) {
				cp.x = 0.0;
				cp.y += self->lineheight;
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

void pluginsview_computetextsizes(PluginsView* self, const psy_ui_Size* size)
{
	const psy_ui_TextMetric* tm;
	
	tm = psy_ui_component_textmetric(&self->component);
	self->avgcharwidth = tm->tmAveCharWidth;
	self->lineheight = (int) (tm->tmHeight * 1.5);
	self->columnwidth = tm->tmAveCharWidth * 45;
	self->identwidth = tm->tmAveCharWidth * 4;
	self->numparametercols = 
		(uintptr_t)psy_max(1, psy_ui_value_px(&size->width, tm) / self->columnwidth);
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

void pluginsview_onpreferredsize(PluginsView* self, const psy_ui_Size* limit,
	psy_ui_Size* rv)
{
	if (self->plugins) {		
		psy_ui_Size parentsize;
		
		parentsize = psy_ui_component_size(psy_ui_component_parent(&self->component));
		pluginsview_computetextsizes(self, &parentsize);
		rv->width = parentsize.width;
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
					workspace_selectview(self->workspace, VIEW_ID_MACHINEVIEW,
						SECTION_ID_MACHINEVIEW_WIRES, 0);
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

uintptr_t pluginsview_visilines(PluginsView* self)
{
	psy_ui_RealSize size;

	size = psy_ui_component_sizepx(&self->component);
	return (uintptr_t)(size.height / self->lineheight);
}

uintptr_t pluginsview_topline(PluginsView* self)
{
	return (uintptr_t)(psy_ui_component_scrolltoppx(&self->component)
		/ self->lineheight);
}

uintptr_t pluginsview_numlines(const PluginsView* self)
{
	return psy_property_size(self->plugins) /
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
	*col = 0;
	*row = 0;
	if (plugin && self->plugins) {
		psy_List* p;
		
		for (p = psy_property_begin(self->plugins); p != NULL;
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

psy_Property* pluginsview_pluginbycursorposition(PluginsView* self, intptr_t col, intptr_t row)
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
		for (p = psy_property_begin(self->plugins); p != NULL;
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
		pluginsview_hittest(self, ev->pt.x, ev->pt.y);
		psy_ui_component_invalidate(&self->component);
		psy_signal_emit(&self->signal_changed, self, 1,
			self->selectedplugin);		
		psy_ui_component_setfocus(&self->component);		
	}
}

void pluginsview_hittest(PluginsView* self, double x, double y)
{				
	if (self->plugins) {
		psy_ui_Size size;
		psy_List* p;
		double cpx;
		double cpy;

		size = psy_ui_component_size(&self->component);
		pluginsview_computetextsizes(self, &size);
		for (p = psy_property_begin(self->plugins), cpx = 0, cpy = 0;
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
	self->selectedplugin = 0;
	if (self->plugins) {
		psy_property_deallocate(self->plugins);
	}
	self->plugins = search(workspace_pluginlist(self->workspace), &self->filters);
	psy_signal_emit(&self->signal_changed, self, 1,
		self->selectedplugin);
	psy_ui_component_setscrolltop(&self->component, psy_ui_value_zero());
	psy_ui_component_updateoverflow(&self->component);
	psy_ui_component_invalidate(&self->component);
}

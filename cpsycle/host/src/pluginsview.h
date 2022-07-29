/*
** This source is free software; you can redistribute itand /or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#if !defined(PLUGINSVIEW_H)
#define PLUGINSVIEW_H

/* ui */
#include <uicomponent.h>
/* audio */
#include <machine.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
** PluginsView
**
** Displays plugin properties in a list. Used by NewMachine.
*/

typedef enum NewMachineSortMode {
	NEWMACHINESORTMODE_NONE,
	NEWMACHINESORTMODE_FAVORITE,
	NEWMACHINESORTMODE_NAME,
	NEWMACHINESORTMODE_TYPE,
	NEWMACHINESORTMODE_MODE,
} NewMachineSortMode;

typedef struct NewMachineSort {
 	psy_Signal signal_changed;
	NewMachineSortMode mode;
} NewMachineSort;

void newmachinesort_init(NewMachineSort*);
void newmachinesort_dispose(NewMachineSort*);

void newmachinesort_sort(NewMachineSort*, NewMachineSortMode);

/* PluginFilter */

typedef struct PluginFilterItem {
	uintptr_t key;
	char* name;
	bool active;	
} PluginFilterItem;

void pluginfilteritem_init(PluginFilterItem*, uintptr_t key, const char* name,
	bool active);
void pluginfilteritem_dispose(PluginFilterItem*);
	
PluginFilterItem* pluginfilteritem_allocinit(uintptr_t key, const char* name,
	bool active);

typedef struct PluginFilterGroup {	
	psy_List* items;
	uintptr_t id;
	char* label;
} PluginFilterGroup;

void pluginfiltergroup_init(PluginFilterGroup*, uintptr_t id,
	const char* label);
void pluginfiltergroup_dispose(PluginFilterGroup*);

PluginFilterGroup* pluginfiltergroup_allocinit(uintptr_t id,
	const char* label);

void pluginfiltergroup_clear(PluginFilterGroup*);
bool pluginfiltergroup_add(PluginFilterGroup*,
	uintptr_t key, const char* name, bool active);
bool pluginfiltergroup_remove(PluginFilterGroup*, uintptr_t key);

void pluginfiltergroup_select(PluginFilterGroup*, 
	uintptr_t key);
void pluginfiltergroup_select_all(PluginFilterGroup*);
void pluginfiltergroup_deselect(PluginFilterGroup*, 
	uintptr_t key);
void pluginfiltergroup_deselect_all(PluginFilterGroup*);	
void pluginfiltergroup_toggle(PluginFilterGroup*, 
	uintptr_t key);
bool pluginfiltergroup_selected(const PluginFilterGroup*, 
	uintptr_t key);
bool pluginfiltergroup_exists(const PluginFilterGroup*, 
	uintptr_t key);	

typedef struct PluginFilter {
	psy_Signal signal_changed;
	char_dyn_t* search_text;	
	PluginFilterGroup mode;
	PluginFilterGroup types;
	PluginFilterGroup categories;
	PluginFilterGroup sort;
} PluginFilter;

void pluginfilter_init(PluginFilter*);
void pluginfilter_dispose(PluginFilter*);

void pluginfilter_select_all(PluginFilter*);
void pluginfilter_notify(PluginFilter*);
void pluginfilter_set_search_text(PluginFilter*, const char* text);

typedef struct NewMachineSelection {
	psy_List* items;
} NewMachineSelection;

void newmachineselection_init(NewMachineSelection*);
void newmachineselection_dispose(NewMachineSelection*);

INLINE bool newmachineselection_empty(const NewMachineSelection* self)
{
	return self->items == NULL;
}

void newmachineselection_clear(NewMachineSelection*);
void newmachineselection_select(NewMachineSelection*, uintptr_t index);
void newmachineselection_singleselect(NewMachineSelection*, uintptr_t index);
void newmachineselection_selectall(NewMachineSelection*, uintptr_t size);
void newmachineselection_deselect(NewMachineSelection*, uintptr_t index);
void newmachineselection_toggle(NewMachineSelection*, uintptr_t index);
bool newmachineselection_isselected(const NewMachineSelection*,
	uintptr_t index);
uintptr_t newmachineselection_first(const NewMachineSelection*);

/* PluginsView */
typedef struct PluginsView {
	/* inherits */
	psy_ui_Component component;
	/* signals */
	psy_Signal signal_selected;	
	psy_Signal signal_changed;
	/* internal */
	double line_height;
	double columnwidth;
	double identwidth;
	intptr_t numparametercols;
	double avgcharwidth;
	intptr_t pluginpos;	
	/* ptr to unfiltered/filtered plugins */
	psy_Property* currplugins;
	/* Unfiltered Plugins */
	psy_Property* plugins;
	/* Filtered Plugins */
	psy_Property* filteredplugins;
	bool generatorsenabled;
	bool effectsenabled;	
	psy_ui_RealPoint dragpt;
	bool multidrag;
	uintptr_t dragindex;
	NewMachineSelection selection;
	bool readonly;
	/* References */
	PluginFilter* filter;
	NewMachineSort* sort;	
} PluginsView;

void pluginsview_init(PluginsView*, psy_ui_Component* parent);

void pluginsview_clear(PluginsView*);
void pluginsview_clearfilter(PluginsView*);
void pluginsview_setplugins(PluginsView*, const psy_Property*);
void pluginsview_set_filter(PluginsView*, PluginFilter*);
void pluginsview_filter(PluginsView*);
void pluginsview_setsort(PluginsView*, NewMachineSort*);
void pluginsview_sort(PluginsView*, NewMachineSortMode);
psy_Property* pluginsview_selectedplugin(PluginsView*);
void pluginsview_clearselection(PluginsView*);
void pluginsview_enableall(PluginsView*);
void pluginsview_enablegenerators(PluginsView*);
void pluginsview_preventgenerators(PluginsView*);
void pluginsview_enableeffects(PluginsView*);
void pluginsview_preventeffects(PluginsView*);

INLINE psy_ui_Component* pluginsview_base(PluginsView* self)
{
	return &self->component;
}

#ifdef __cplusplus
}
#endif

#endif /* PLUGINSVIEW_H */

// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#if !defined(PLUGINSVIEW_H)
#define PLUGINSVIEW_H

// host
#include "workspace.h"

#ifdef __cplusplus
extern "C" {
#endif

// PluginsView
//
// Displays plugin properties in a list. Used by NewMachine.

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

// NewMachineFilter
typedef struct NewMachineFilter {
	psy_Signal signal_changed;
	bool gen;
	bool effect;
	bool intern;
	bool native;
	bool vst;
	bool lua;
	bool ladspa;
	psy_audio_MachineType type;
	char_dyn_t* text;
	psy_Table categories;	
} NewMachineFilter;

void newmachinefilter_init(NewMachineFilter*);
void newmachinefilter_dispose(NewMachineFilter*);

void newmachinefilter_reset(NewMachineFilter*);
bool newmachinefilter_all(const NewMachineFilter*);
void newmachinefilter_notify(NewMachineFilter*);
void newmachinefilter_settext(NewMachineFilter*, const char* text);
void newmachinefilter_setalltypes(NewMachineFilter*);
void newmachinefilter_cleartypes(NewMachineFilter*);
void newmachinefilter_addcategory(NewMachineFilter*, const char* category);
void newmachinefilter_removecategory(NewMachineFilter*, const char* category);
void newmachinefilter_anycategory(NewMachineFilter*);
bool newmachinefilter_useanycategory(const NewMachineFilter*);
bool newmachinefilter_hascategory(const NewMachineFilter*,
	const char* category);

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

// PluginsView
typedef struct PluginsView {
	// inherits
	psy_ui_Component component;
	// signals	
	psy_Signal signal_selected;	
	psy_Signal signal_changed;
	// internal	
	double lineheight;
	double columnwidth;
	double identwidth;
	intptr_t numparametercols;
	double avgcharwidth;
	intptr_t pluginpos;	
	// ptr to unfiltered/filtered plugins
	psy_Property* currplugins;
	// Unfiltered Plugins
	psy_Property* plugins;
	// Filtered Plugins
	psy_Property* filteredplugins;
	bool generatorsenabled;
	bool effectsenabled;
	int mode;
	psy_ui_RealPoint dragpt;
	bool multidrag;
	uintptr_t dragindex;
	NewMachineSelection selection;
	// References
	NewMachineFilter* filter;
	NewMachineSort* sort;	
} PluginsView;

void pluginsview_init(PluginsView*, psy_ui_Component* parent);

void pluginsview_clear(PluginsView*);
void pluginsview_clearfilter(PluginsView*);
void pluginsview_setplugins(PluginsView*, const psy_Property*);
void pluginsview_setfilter(PluginsView*, NewMachineFilter*);
void pluginsview_filter(PluginsView*);
void pluginsview_setsort(PluginsView*, NewMachineSort*);
void pluginsview_sort(PluginsView*, NewMachineSortMode);
psy_Property* pluginsview_selectedplugin(PluginsView*);
void pluginsview_clearselection(PluginsView*);

#ifdef __cplusplus
}
#endif

#endif /* PLUGINSVIEW_H */

/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#if !defined(NEWMACHINE_H)
#define NEWMACHINE_H

/* host */
#include "labelpair.h"
#include "newmachinesection.h"
#include "newmachinedetail.h"
#include "pluginsview.h"
#include "pluginscanview.h"
#include "searchfield.h"
#include "workspace.h"
/* ui */
#include <uibutton.h>
#include <uicheckbox.h>
#include <uilabel.h>
#include <uitabbar.h>
#include <uinotebook.h>
#include <uiscroller.h>
#include <uisplitbar.h>

#ifdef __cplusplus
extern "C" {
#endif

/* NewMachine */
		
struct NewMachine;

typedef struct NewMachineSearchBar {
	/* inherits */
	psy_ui_Component component;
	/* intern */
	SearchField search;
	PluginFilter* filter;
} NewMachineSearchBar;

void newmachinesearchbar_init(NewMachineSearchBar*, psy_ui_Component* parent,
	PluginFilter* filter);

void newmachinesearchbar_setfilter(NewMachineSearchBar*, PluginFilter*);

/* NewMachineRescanBar */
typedef struct NewMachineRescanBar {
	/* inherits */
	psy_ui_Component component;
	/* intern */
	psy_ui_Button rescan;
	psy_ui_Label desc;
	psy_ui_Button selectdirectories;
	psy_ui_Button add;
	psy_ui_Button cancel;
	/* references */
	Workspace* workspace;
} NewMachineRescanBar;

void newmachinerescanbar_init(NewMachineRescanBar*, psy_ui_Component* parent,
	Workspace*);

/* NewMachineSectionBar */
typedef struct NewMachineSectionBar {
	/* inherits */
	psy_ui_Component component;
	/* intern */
	psy_ui_Button createsection;
	psy_ui_Button removesection;
	psy_ui_Button clearsection;
	psy_ui_Button addtosection;
	psy_ui_Button removefromsection;
	psy_ui_Button addpane;
	psy_ui_Button removepane;	
} NewMachineSectionBar;

void newmachinesectionbar_init(NewMachineSectionBar*, psy_ui_Component* parent);

/* NewMachineFilterGroup */
typedef struct NewMachineFilterGroup {
	/* inherits */
	psy_ui_Component component;
	/* signals */
	psy_Signal signal_selected;
	/* intern */
	psy_ui_Component header;
	psy_ui_Label desc;
	psy_ui_Component types;
	psy_ui_Component client;
	/* references */
	PluginFilter* filter;
	PluginFilterGroup* filter_group;
} NewMachineFilterGroup;

void newmachinefiltergroup_init(NewMachineFilterGroup*,
	psy_ui_Component* parent, PluginFilter* filter,
	PluginFilterGroup*);
	
void newmachinefiltergroup_set_filter(NewMachineFilterGroup*,
	PluginFilter*, PluginFilterGroup*);
void newmachinefiltergroup_add(NewMachineFilterGroup*,
	const char* label, uintptr_t id, bool active);
void newmachinefiltergroup_set(NewMachineFilterGroup*,
	uintptr_t id, bool);
void newmachinefiltergroup_mark(NewMachineFilterGroup*,
	uintptr_t id);	
void newmachinefiltergroup_unmark(NewMachineFilterGroup*,
	uintptr_t id);	
	
INLINE psy_ui_Component* newmachinefiltergroup_base(NewMachineFilterGroup* self)
{
	return &self->component;
}		
	
/* NewMachineCategoryBar */
typedef struct NewMachineCategoryBar {
	/* inherits */
	NewMachineFilterGroup group;
	/* intern */	
	/* references */
	PluginFilter* filter;
	psy_audio_PluginCatcher* plugincatcher;
} NewMachineCategoryBar;

void newmachinecategorybar_init(NewMachineCategoryBar*, psy_ui_Component* parent,
	PluginFilter*, psy_audio_PluginCatcher*);

void newmachinecategorybar_setfilter(NewMachineCategoryBar*, PluginFilter*);
void newmachinecategorybar_build(NewMachineCategoryBar*);


INLINE psy_ui_Component* newmachinecategorybar_base(NewMachineCategoryBar* self)
{
	return &self->group.component;
}

/* NewMachineFiltersBar */
typedef struct NewMachineFiltersBar {
	/* inherits */
	psy_ui_Component component;
	/* intern */	
	psy_ui_Component expandfiltersbar;
	psy_ui_Component filters;
	psy_ui_Button togglefilters;
	NewMachineFilterGroup modebar;
	NewMachineFilterGroup typebar;
	NewMachineFilterGroup sortbar;
	NewMachineCategoryBar categorybar;	
	/* references */	
	psy_audio_PluginCatcher* plugincatcher;
} NewMachineFiltersBar;

void newmachinefiltersbar_init(NewMachineFiltersBar*, psy_ui_Component* parent,
	PluginFilter*, psy_audio_PluginCatcher*);

void newmachinefiltersbar_setfilter(NewMachineFiltersBar*, PluginFilter*);
void newmachinefiltersbar_build(NewMachineFiltersBar*);


INLINE psy_ui_Component* newmachinefiltersbar_base(NewMachineFiltersBar* self)
{
	return &self->component;
}


typedef struct NewMachineSectionsHeader {
	/* inherits */
	psy_ui_Component component;
	/* internal */
	psy_ui_Image icon;
	psy_ui_Button expand;	
} NewMachineSectionsHeader;

void newmachinesectionsheader_init(NewMachineSectionsHeader*,
	psy_ui_Component* parent, uintptr_t iconresourceid);

struct NewMachine;

typedef struct NewMachineSectionsPane {
	/* inherits */
	psy_ui_Component component;
	/* internal */
	NewMachineSectionsHeader sectionsheader;
	psy_ui_TabBar navsections;
	psy_ui_Component pane;
	psy_ui_Component sections;
	psy_ui_Scroller scroller_sections;
	NewMachineFiltersBar filtersbar;
	PluginScanThread* plugin_scan_thread;
	psy_Table newmachinesections;
	PluginFilter filter;	
	/* references */
	struct NewMachine* newmachine;
} NewMachineSectionsPane;

void newmachinesectionspane_init(NewMachineSectionsPane*,
	psy_ui_Component* parent, struct NewMachine*,
	PluginScanThread*);

NewMachineSectionsPane* newmachinesectionspane_alloc(void);
NewMachineSectionsPane* newmachinesectionspane_allocinit(
	psy_ui_Component* parent, struct NewMachine*, PluginScanThread*);

void newmachinesectionspane_checkselections(NewMachineSectionsPane*,
	PluginsView* sender);
void newmachinesectionspane_buildsections(NewMachineSectionsPane*);

typedef struct NewMachine {
	/* inherits */
	psy_ui_Component component;
	/* signals */
	psy_Signal signal_selected;	
	/* internal */
	/* ui elements */
	psy_ui_Notebook notebook;
	NewMachineSearchBar searchbar;
	psy_ui_Component client;	
	psy_ui_Label pluginslabel;	
	psy_ui_Component sections;	
	NewMachineDetail detail;
	PluginScanView scanview;
	NewMachineSectionBar sectionbar;
	NewMachineRescanBar rescanbar;		
	psy_ui_Scroller scroller_all;		
	// int mode;
	uintptr_t newsectioncount;	
	/* references */
	Workspace* workspace;
	uintptr_t restoresection;
	psy_Property* selectedplugin;	
	NewMachineSection* selectedsection;	
	NewMachineSectionsPane* curr_sections_pane;
} NewMachine;

void newmachine_init(NewMachine*, psy_ui_Component* parent, Workspace*);

void newmachine_updateplugins(NewMachine*);

void newmachine_insertmode(NewMachine*);
void newmachine_appendmode(NewMachine*);
void newmachine_addeffectmode(NewMachine*);

void newmachine_onpluginselected(NewMachine*, PluginsView* sender);
void newmachine_onsectionselected(NewMachine*,
	NewMachineSection* sender);
void newmachine_onpluginchanged(NewMachine*, PluginsView* parent);
void newmachine_setfilter(NewMachine*, PluginFilter*);
void newmachine_select_pane(NewMachine*, NewMachineSectionsPane*);

INLINE psy_ui_Component* newmachine_base(NewMachine* self)
{
	return &self->component;
}

#ifdef __cplusplus
}
#endif

#endif /* NEWMACHINE_H */

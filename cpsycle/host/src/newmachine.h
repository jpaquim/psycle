/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net
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
	NewMachineFilter* filter;
} NewMachineSearchBar;

void newmachinesearchbar_init(NewMachineSearchBar*, psy_ui_Component* parent,
	NewMachineFilter* filter);

void newmachinesearchbar_setfilter(NewMachineSearchBar*, NewMachineFilter*);

/* NewMachineRescanBar */
typedef struct NewMachineRescanBar {
	/* inherits */
	psy_ui_Component component;
	/* intern */
	psy_ui_Button rescan;
	psy_ui_Label desc;
	psy_ui_Button selectdirectories;
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
	/* references */
	Workspace* workspace;
} NewMachineSectionBar;

void newmachinesectionbar_init(NewMachineSectionBar*, psy_ui_Component* parent,
	Workspace*);

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
} NewMachineFilterGroup;

void newmachinefiltergroup_init(NewMachineFilterGroup*,
	psy_ui_Component* parent, const char* label);
	
void newmachinefiltergroup_add(NewMachineFilterGroup*,
	const char* label, uintptr_t id);
void newmachinefiltergroup_set(NewMachineFilterGroup*,
	uintptr_t id, bool);
void newmachinefiltergroup_mark(NewMachineFilterGroup*,
	uintptr_t id);	
void newmachinefiltergroup_unmark(NewMachineFilterGroup*,
	uintptr_t id);	
	
/* NewMachineSortBar */
typedef struct NewMachineSortBar {
	/* inherits */
	NewMachineFilterGroup group;
	/* references */
	NewMachineSort* sort;	
} NewMachineSortBar;

void newmachinesortbar_init(NewMachineSortBar*, psy_ui_Component* parent,
	NewMachineSort*);
	
INLINE psy_ui_Component* newmachinesortbar_base(NewMachineSortBar* self)
{
	return &self->group.component;
}	

/* NewMachineFilterBar */

typedef struct NewMachineModeBar {
	/* inherits */
	NewMachineFilterGroup group;
	/* intern */
	psy_ui_Button gen;
	psy_ui_Button effects;	
	/* references */
	NewMachineFilter* filter;
} NewMachineModeBar;

void newmachinemodebar_init(NewMachineModeBar*, psy_ui_Component* parent,
	NewMachineFilter*);

void newmachinemodebar_setfilter(NewMachineModeBar*, NewMachineFilter*);
void newmachinemodebar_update(NewMachineModeBar*);

INLINE psy_ui_Component* newmachinemodebar_base(NewMachineModeBar* self)
{
	return &self->group.component;
}


typedef struct NewMachineFilterBar {
	/* inherits */
	NewMachineFilterGroup group;
	/* intern */
	psy_ui_Button gen;
	psy_ui_Button effects;
	psy_ui_Button intern;
	psy_ui_Button native;
	psy_ui_Button vst;
	psy_ui_Button lua;
	psy_ui_Button ladspa;
	/* references */
	NewMachineFilter* filter;
} NewMachineFilterBar;

void newmachinefilterbar_init(NewMachineFilterBar*, psy_ui_Component* parent,
	NewMachineFilter*);

void newmachinefilterbar_setfilter(NewMachineFilterBar*, NewMachineFilter*);
void newmachinefilterbar_update(NewMachineFilterBar*);

INLINE psy_ui_Component* newmachinefilterbar_base(NewMachineFilterBar* self)
{
	return &self->group.component;
}

/* NewMachineCategoryBar */
typedef struct NewMachineCategoryBar {
	/* inherits */
	NewMachineFilterGroup group;
	/* intern */	
	/* references */
	NewMachineFilter* filter;
	psy_audio_PluginCatcher* plugincatcher;
} NewMachineCategoryBar;

void newmachinecategorybar_init(NewMachineCategoryBar*, psy_ui_Component* parent,
	NewMachineFilter*, psy_audio_PluginCatcher*);

void newmachinecategorybar_setfilter(NewMachineCategoryBar*, NewMachineFilter*);
void newmachinecategorybar_build(NewMachineCategoryBar*);


INLINE psy_ui_Component* newmachinecategorybar_base(NewMachineCategoryBar* self)
{
	return &self->group.component;
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
	psy_ui_Component sections;	
	psy_ui_Scroller scroller_sections;
	Workspace* workspace;
	psy_Table newmachinesections;
	NewMachineFilter filter;
	/* references */
	struct NewMachine* newmachine;
} NewMachineSectionsPane;

void newmachinesectionspane_init(NewMachineSectionsPane*,
	psy_ui_Component* parent, struct NewMachine*, Workspace*);

NewMachineSectionsPane* newmachinesectionspane_alloc(void);
NewMachineSectionsPane* newmachinesectionspane_allocinit(
	psy_ui_Component* parent, struct NewMachine*, Workspace*);

void newmachinesectionspane_checkselections(NewMachineSectionsPane*,
	PluginsView* sender);

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
	psy_ui_Component filtersbar;
	psy_ui_Component expandfiltersbar;
	psy_ui_Component filters;
	psy_ui_Button togglefilters;
	NewMachineModeBar modebar;
	NewMachineFilterBar filterbar;
	NewMachineSortBar sortbar;
	NewMachineCategoryBar categorybar;
	psy_ui_Component sections;	
	NewMachineDetail detail;
	PluginScanView scanview;
	NewMachineSectionBar sectionbar;
	NewMachineRescanBar rescanbar;		
	psy_ui_Scroller scroller_all;
	/* data */
	bool appendstack;
	int mode;
	uintptr_t newsectioncount;	
	NewMachineSort sort;
	/* references */
	Workspace* workspace;
	uintptr_t restoresection;
	psy_Property* selectedplugin;
	NewMachineFilter* currfilter;
	NewMachineSection* selectedsection;	
	NewMachineSectionsPane* curr_sections_pane;
} NewMachine;

void newmachine_init(NewMachine*, psy_ui_Component* parent, Workspace*);

void newmachine_updateplugins(NewMachine*);
void newmachine_enableall(NewMachine*);
void newmachine_enablegenerators(NewMachine*);
void newmachine_preventgenerators(NewMachine*);
void newmachine_enableeffects(NewMachine*);
void newmachine_preventeffects(NewMachine*);

void newmachine_insertmode(NewMachine*);
void newmachine_appendmode(NewMachine*);
void newmachine_addeffectmode(NewMachine*);

bool newmachine_selectedmachineinfo(const NewMachine*, psy_audio_MachineInfo* rv);

void newmachine_onpluginselected(NewMachine*, PluginsView* sender);
void newmachine_onsectionselected(NewMachine*,
	NewMachineSection* sender);
void newmachine_onpluginchanged(NewMachine*, PluginsView* parent);
void newmachine_setfilter(NewMachine*, NewMachineFilter*);

INLINE psy_ui_Component* newmachine_base(NewMachine* self)
{
	return &self->component;
}

#ifdef __cplusplus
}
#endif

#endif /* NEWMACHINE_H */

// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#if !defined(NEWMACHINE_H)
#define NEWMACHINE_H

// host
#include "machineviewskin.h"
#include "workspace.h"
#include "labelpair.h"
// ui
#include <uibutton.h>
#include <uiedit.h>
#include <uiimage.h>
#include <uilabel.h>
#include <uicheckbox.h>
#include <uiscroller.h>
#include <uisplitbar.h>
#include <uinotebook.h>
#include <plugincatcher.h>
#include <hashtbl.h>

#ifdef __cplusplus
extern "C" {
#endif

// NewMachine
// Adding machines
		
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

struct NewMachine;

typedef struct NewMachineSearch {
	// inherits
	psy_ui_Component component;
	// intern
	psy_ui_Image image;
	psy_ui_Edit edit;
	bool hasdefaulttext;
	// references
	NewMachineFilter* filter;
} NewMachineSearch;

void newmachinesearch_init(NewMachineSearch*, psy_ui_Component* parent,
	NewMachineFilter* filter);

typedef struct NewMachineBar {
	// inherits
	psy_ui_Component component;	
	psy_ui_Button rescan;
    psy_ui_Button selectdirectories;
	psy_ui_Button sortbyfavorite;
	psy_ui_Button sortbyname;
	psy_ui_Button sortbytype;
	psy_ui_Button sortbymode;
	psy_ui_Button createsection;
	psy_ui_Button addtosection;
	psy_ui_Button removefromsection;
	psy_ui_Button removesection;
	Workspace* workspace;
} NewMachineBar;

void newmachinebar_init(NewMachineBar*, psy_ui_Component* parent,
	Workspace*);

typedef struct NewMachineDetail {
	// inherits
	psy_ui_Component component;
	// intern
	NewMachineSearch search;
	NewMachineBar bar;
	psy_ui_Label desclabel;
	LabelPair plugname;
	LabelPair dllname;
	LabelPair version;
	LabelPair apiversion;
	psy_ui_Component bottom;
    psy_ui_Label compatlabel;
    psy_ui_CheckBox compatblitzgamefx;
	// references
    Workspace* workspace;	
} NewMachineDetail;

void newmachinedetail_init(NewMachineDetail*, psy_ui_Component* parent,
	NewMachineFilter* filter, Workspace*);

void newmachinedetail_reset(NewMachineDetail*);
void newmachinedetail_update(NewMachineDetail*, psy_Property*);
void newmachinedetail_setdescription(NewMachineDetail*, const char* text);
void newmachinedetail_setplugname(NewMachineDetail*, const char* text);
void newmachinedetail_setdllname(NewMachineDetail*, const char* text);
void newmachinedetail_setplugversion(NewMachineDetail* self, int16_t version);
void newmachinedetail_setapiversion(NewMachineDetail* self,
	int16_t apiversion);

// NewMachineFilterBar
typedef struct NewMachineFilterBar {
	// inherits
	psy_ui_Component component;	
	// intern
	psy_ui_Button gen;
	psy_ui_Button effects;
	psy_ui_Button intern;
	psy_ui_Button native;
	psy_ui_Button vst;
	psy_ui_Button lua;
	psy_ui_Button ladspa;
	// references
	NewMachineFilter* filters;
} NewMachineFilterBar;

void newmachinefilterbar_init(NewMachineFilterBar*, psy_ui_Component* parent,
	NewMachineFilter*);

void newmachinefilterbar_setfilters(NewMachineFilterBar*, NewMachineFilter*);
void newmachinefilterbar_update(NewMachineFilterBar*);

// NewMachineCategoryBar
typedef struct NewMachineCategoryBar {
	// inherits
	psy_ui_Component component;
	// intern
	psy_ui_Component client;
	psy_Table categories;
	// references
	NewMachineFilter* filters;
	Workspace* workspace;
} NewMachineCategoryBar;

void newmachinecategorybar_init(NewMachineCategoryBar*, psy_ui_Component* parent,
	NewMachineFilter*, Workspace*);

void newmachinecategorybar_build(NewMachineCategoryBar*);

typedef struct PluginScanView {
	// inherits
	psy_ui_Component component;
	// ui elements
	psy_ui_Label scan;
} PluginScanView;

void pluginscanview_init(PluginScanView*, psy_ui_Component* parent);

typedef struct {
	// inherits
	psy_ui_Component component;
	// Signals
	psy_Signal signal_selected;
	psy_Signal signal_changed;
	// internal data
	intptr_t count;
	double lineheight;
	double columnwidth;
	double identwidth;
	intptr_t numparametercols;
	double avgcharwidth;
	intptr_t pluginpos;
	psy_Property* plugins;
	psy_Property* selectedplugin;  
	Workspace* workspace;	
	bool onlyfavorites;
	bool generatorsenabled;
	bool effectsenabled;
	NewMachineFilter filters;
	int mode;
} PluginsView;

void pluginsview_init(PluginsView*, psy_ui_Component* parent, bool favorites,
	Workspace*);

struct NewMachine;

typedef struct NewMachineSection {
	// inherits
	psy_ui_Component component;
	// intern
	psy_ui_Component header;
	psy_ui_Label label;
	PluginsView pluginview;
	bool preventedit;
	// references
	psy_Property* property;
	psy_ui_Edit* edit;
	struct NewMachine* newmachine;
} NewMachineSection;

void newmachinesection_init(NewMachineSection* self, psy_ui_Component* parent,
	psy_Property* property, psy_ui_Edit*, struct NewMachine*, Workspace*);

NewMachineSection* newmachinesection_alloc(void);
NewMachineSection* newmachinesection_allocinit(psy_ui_Component* parent,
	psy_Property* property, psy_ui_Edit*, struct NewMachine*, Workspace*);

typedef struct NewMachine {
	// inherits
	psy_ui_Component component;
	// Signals
	psy_Signal signal_selected;	
	// internal ui elements	
	psy_ui_Notebook notebook;
	psy_ui_Component client;
	psy_ui_Component sections;
	psy_ui_Component favoriteheader;
	psy_ui_Image favoriteicon;
	psy_ui_Label favoritelabel;
	PluginsView favoriteview;
	psy_ui_Component usersections;
	psy_ui_Component all;
	psy_ui_Component pluginsheader;
	psy_ui_Image pluginsicon;
	psy_ui_Label pluginslabel;
	NewMachineFilterBar filterbar;
	NewMachineCategoryBar categorybar;	
	PluginsView pluginsview;
	NewMachineDetail detail;
	PluginScanView scanview;
	MachineViewSkin* skin;
	psy_ui_Scroller scroller_fav;
	psy_ui_Scroller scroller_main;	
	psy_ui_Edit edit;
	// internal data
	bool scanending;
	bool appendstack;
	int mode;
	uintptr_t newsectioncount;
	// references
	Workspace* workspace;
	uintptr_t restoresection;
	psy_Property* selectedplugin;
	NewMachineSection* selectedsection;
} NewMachine;

void newmachine_init(NewMachine*, psy_ui_Component* parent, MachineViewSkin*,
	Workspace*);
void newmachine_updateskin(NewMachine*);

void newmachine_enableall(NewMachine*);
void newmachine_enablegenerators(NewMachine*);
void newmachine_preventgenerators(NewMachine*);
void newmachine_enableeffects(NewMachine*);
void newmachine_preventeffects(NewMachine*);

void newmachine_insertmode(NewMachine*);
void newmachine_appendmode(NewMachine*);
void newmachine_addeffectmode(NewMachine*);

bool newmachine_selectedmachineinfo(const NewMachine*, psy_audio_MachineInfo* rv);

INLINE psy_ui_Component* newmachine_base(NewMachine* self)
{
	return &self->component;
}

#ifdef __cplusplus
}
#endif

#endif

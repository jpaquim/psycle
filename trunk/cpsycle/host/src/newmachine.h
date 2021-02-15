// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#if !defined(NEWMACHINE_H)
#define NEWMACHINE_H

#include "machineviewskin.h"
#include "workspace.h"
#include <uibutton.h>
#include <uiimage.h>
#include <uilabel.h>
#include <uicheckbox.h>
#include <uiscroller.h>
#include <uinotebook.h>
#include <plugincatcher.h>
#include <hashtbl.h>

#ifdef __cplusplus
extern "C" {
#endif

// NewMachine
// Adding machines

typedef struct {
	psy_ui_Component component;	
	psy_ui_Button rescan;
    psy_ui_Button selectdirectories;
	psy_ui_Button sortbyfavorite;
	psy_ui_Button sortbyname;
	psy_ui_Button sortbytype;
	psy_ui_Button sortbymode;
	Workspace* workspace;
} NewMachineBar;

void newmachinebar_init(NewMachineBar*, psy_ui_Component* parent, Workspace*);

typedef struct {
	psy_ui_Component component;
	NewMachineBar bar;
	psy_ui_Label desclabel;
    psy_ui_Label compatlabel;
    psy_ui_CheckBox compatblitzgamefx;
    Workspace* workspace;
	bool empty;
} NewMachineDetail;

void newmachinedetail_init(NewMachineDetail*, psy_ui_Component* parent,
	Workspace*);

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
	int mode;
} PluginsView;

void pluginsview_init(PluginsView*, psy_ui_Component* parent, bool favorites,
	Workspace*);

typedef struct NewMachine {
	// inherits
	psy_ui_Component component;
	// Signals
	psy_Signal signal_selected;	
	// internal ui elements	
	psy_ui_Notebook notebook;
	psy_ui_Component client;
	psy_ui_Component favoriteheader;
	psy_ui_Image favoriteicon;
	psy_ui_Label favoritelabel;
	PluginsView favoriteview;
	psy_ui_Component pluginsheader;
	psy_ui_Image pluginsicon;
	psy_ui_Label pluginslabel;
	PluginsView pluginsview;
	NewMachineDetail detail;
	PluginScanView scanview;
	MachineViewSkin* skin;
	psy_ui_Scroller scroller_fav;
	psy_ui_Scroller scroller_main;	
	// internal data
	bool scanending;
	int mode;
	// references
	Workspace* workspace;
} NewMachine;

void newmachine_init(NewMachine*, psy_ui_Component* parent, MachineViewSkin*, Workspace*);
void newmachine_updateskin(NewMachine*);

void newmachine_enableall(NewMachine*);
void newmachine_enablegenerators(NewMachine*);
void newmachine_preventgenerators(NewMachine*);
void newmachine_enableeffects(NewMachine*);
void newmachine_preventeffects(NewMachine*);

void newmachine_insertmode(NewMachine*);
void newmachine_appendmode(NewMachine*);
void newmachine_addeffectmode(NewMachine*);

INLINE psy_ui_Component* newmachine_base(NewMachine* self)
{
	return &self->component;
}


#ifdef __cplusplus
}
#endif

#endif

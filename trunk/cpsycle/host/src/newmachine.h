// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#if !defined(NEWMACHINE_H)
#define NEWMACHINE_H

#include "machineviewskin.h"
#include "workspace.h"
#include <uibutton.h>
#include <uilabel.h>
#include <uicheckbox.h>
#include <uiscroller.h>
#include <uinotebook.h>
#include <plugincatcher.h>
#include <hashtbl.h>

#ifdef __cplusplus
extern "C" {
#endif

// aim: adding machines

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
   psy_ui_Component component;
   intptr_t count;
   intptr_t lineheight;
   intptr_t columnwidth;
   intptr_t identwidth;
   intptr_t numparametercols;
   intptr_t avgcharwidth;
   intptr_t pluginpos;
   psy_Property* plugins;
   psy_Property* selectedplugin;   
   Workspace* workspace;
   intptr_t calledby;
   bool onlyfavorites;
   // Signals
   psy_Signal signal_selected;
   psy_Signal signal_changed;
} PluginsView;

void pluginsview_init(PluginsView*, psy_ui_Component* parent, bool favorites, Workspace*);

typedef struct NewMachine {
	// inherits
	psy_ui_Component component;
	// ui elements	
	psy_ui_Notebook notebook;
	psy_ui_Component client;
	psy_ui_Label favoriteheader;
	PluginsView favoriteview;
	psy_ui_Label pluginsheader;
	PluginsView pluginsview;
	NewMachineDetail detail;
	PluginScanView scanview;
	MachineViewSkin* skin;
	psy_ui_Scroller scroller_fav;
	psy_ui_Scroller scroller_main;
	// internal data
	bool scanending;
	// Signals
	psy_Signal signal_selected;
	// references
	Workspace* workspace;
} NewMachine;

void newmachine_init(NewMachine*, psy_ui_Component* parent, MachineViewSkin*, Workspace*);
void newmachine_updateskin(NewMachine*);

INLINE psy_ui_Component* newmachine_base(NewMachine* self)
{
	return &self->component;
}


#ifdef __cplusplus
}
#endif

#endif

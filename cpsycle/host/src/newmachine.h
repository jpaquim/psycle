// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#if !defined(NEWMACHINE_H)
#define NEWMACHINE_H

#include "workspace.h"
#include <uibutton.h>
#include <uilabel.h>
#include <plugincatcher.h>
#include <hashtbl.h>


typedef struct {
	psy_ui_Component component;	
	psy_ui_Button rescan;
	Workspace* workspace;
} NewMachineBar;

void newmachinebar_init(NewMachineBar*, psy_ui_Component* parent, Workspace*);

typedef struct {
	psy_ui_Component component;
	NewMachineBar bar;
	psy_ui_Label desclabel;
} NewMachineDetail;

void newmachinedetail_init(NewMachineDetail*, psy_ui_Component* parent,
	Workspace*);

typedef struct {
   psy_ui_Component component;   
   int dy;
   int count;
   int lineheight;
   int columnwidth;
   int identwidth;
   int numparametercols;
   int avgcharwidth;
   int pluginpos;
   psy_Properties* selectedplugin;
   psy_Signal signal_selected;
   psy_Signal signal_changed;
   Workspace* workspace;
   int calledby;
} PluginsView;

void pluginsview_init(PluginsView*, psy_ui_Component* parent, Workspace*);

typedef struct {
   psy_ui_Component component;   
   PluginsView pluginsview;
   NewMachineDetail detail;   
} NewMachine;

void newmachine_init(NewMachine*, psy_ui_Component* parent, Workspace*);

#endif

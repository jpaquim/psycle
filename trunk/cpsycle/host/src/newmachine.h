// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#if !defined(NEWMACHINE)
#define NEWMACHINE

#include "workspace.h"
#include <uibutton.h>
#include <uilabel.h>
#include <plugincatcher.h>
#include <hashtbl.h>

typedef struct {
	ui_component component;
	ui_label desclabel;	
} NewMachineDetail;

typedef struct {
	ui_component component;	
	ui_button rescan;
	Workspace* workspace;
} NewMachineBar;

void InitNewMachineBar(NewMachineBar*, ui_component* parent, Workspace*);

typedef struct {
   ui_component component;   
   ui_graphics* g;
   int cpx;
   int cpy;
   int cx;
   int cy;   
   int dy;
   int count;
   int lineheight;
   int columnwidth;
   int identwidth;
   int numcols;
   int avgcharwidth;
   int pluginpos;
   Properties* selectedplugin;
   Signal signal_selected;
   Signal signal_changed;
   Workspace* workspace;
   int calledbygear;
} PluginsView;

void InitPluginsView(PluginsView*, ui_component* parent, Workspace*);

typedef struct {
   ui_component component;   
   PluginsView pluginsview;
   NewMachineDetail detail;
   NewMachineBar bar;
} NewMachine;

void InitNewMachine(NewMachine*, ui_component* parent, Workspace*);


#endif
// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#if !defined(NEWMACHINE)
#define NEWMACHINE

#include "workspace.h"
#include <uicomponent.h>
#include <uidef.h>
#include <plugincatcher.h>
#include <hashtbl.h>
#include <signal.h>

struct NewMachineStruct {
   ui_component component;   
   ui_graphics* g;
   int cx;
   int cy;   
   int pluginpos;
   Properties* selectedplugin;
   Signal signal_selected;
   Workspace* workspace;
};

typedef struct NewMachineStruct NewMachine;

void InitNewMachine(NewMachine* NewMachine, ui_component* parent, Workspace* workspace);


#endif
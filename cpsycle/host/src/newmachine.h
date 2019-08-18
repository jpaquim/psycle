// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#if !defined(NEWMACHINE)
#define NEWMACHINE

#include <uicomponent.h>
#include <uidef.h>
#include <player.h>
#include <plugincatcher.h>
#include <hashtbl.h>

struct NewMachineStruct {
   ui_component component;   
   ui_graphics* g;
   int cx;
   int cy;    
   PluginCatcher plugincatcher;
   int pluginpos;
   Properties* selectedplugin;
   void* selectioncontext;
   void (*selected)(void*, CMachineInfo* plugin, const char* path);
};

typedef struct NewMachineStruct NewMachine;

void InitNewMachine(NewMachine* NewMachine, ui_component* parent, Player* player, Properties* properties);


#endif
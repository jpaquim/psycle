// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#if !defined(WORKSPACE_H)
#define WORKSPACE_H

#include <song.h>
#include <player.h>
#include <properties.h>
#include <signal.h>
#include <plugincatcher.h>
#include <machinefactory.h>
#include <uicomponent.h>
#include "undoredo.h"
#include "inputmap.h"

enum {
	WORKSPACE_NEWSONG,
	WORKSPACE_LOADSONG
};

typedef struct {	
	Song* song;
	Player player;	
	Properties* config;
	Properties* inputoutput;
	Properties* keyboard;
	Properties* properties;	
	Properties* driverconfigure;	
	PluginCatcher plugincatcher;
	MachineFactory machinefactory;
	int octave;
	Signal signal_octavechanged;
	Signal signal_songchanged;	
	Signal signal_configchanged;
	ui_component* mainhandle;
	UndoRedo undoredo;
	Inputs noteinputs;
} Workspace;

void workspace_init(Workspace*);
void workspace_dispose(Workspace*);
void workspace_initplayer(Workspace*);
void workspace_newsong(Workspace*);
void workspace_loadsong(Workspace*, const char* path);
void workspace_scanplugins(Workspace*);
Properties* workspace_pluginlist(Workspace*);
void workspace_load_configuration(Workspace*);
void workspace_save_configuration(Workspace*);
void workspace_setoctave(Workspace*, int octave);
void workspace_updatedriver(Workspace*);
int workspace_octave(Workspace*);
int workspace_showsonginfoonload(Workspace*);
int workspace_showaboutatstart(Workspace*);
int workspace_showlinenumbers(Workspace*);
void workspace_configchanged(Workspace*, Properties* property,
	Properties* choice);
void workspace_undo(Workspace*);
void workspace_redo(Workspace*);
Inputs* workspace_noteinputs(Workspace*);

#endif

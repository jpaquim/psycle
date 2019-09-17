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

typedef struct {	
	Song* song;
	Player player;
	MachineCallback machinecallback;
	Properties* config;
	Properties* properties;	
	PluginCatcher plugincatcher;
	MachineFactory machinefactory;
	int octave;
	Signal signal_octavechanged;
	Signal signal_songchanged;
} Workspace;

void workspace_init(Workspace*);
void workspace_dispose(Workspace*);
void workspace_newsong(Workspace*);
void workspace_loadsong(Workspace*, const char* path);
void workspace_scanplugins(Workspace*);
Properties* workspace_pluginlist(Workspace*);
void workspace_load_configuration(Workspace*);
void workspace_save_configuration(Workspace*);
void workspace_setoctave(Workspace*, int octave);
int workspace_octave(Workspace*);

#endif

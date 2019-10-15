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
#include <sequence.h>

enum {
	WORKSPACE_NEWSONG,
	WORKSPACE_LOADSONG
};

typedef struct {	
	unsigned int track;
	double offset;
	unsigned int line;
	unsigned int subline;
	unsigned int totallines;
	unsigned int col;
} EditPosition;

typedef struct {	
	Song* song;
	Player player;	
	Properties* config;
	Properties* inputoutput;
	Properties* midi;
	Properties* keyboard;
	Properties* directories;
	Properties* properties;
	Properties* lang;	
	Properties* driverconfigure;
	Properties* midiconfigure;
	PluginCatcher plugincatcher;
	MachineFactory machinefactory;
	int octave;
	Signal signal_octavechanged;
	Signal signal_songchanged;	
	Signal signal_configchanged;
	Signal signal_editpositionchanged;	
	ui_component* mainhandle;
	UndoRedo undoredo;
	Inputs noteinputs;
	EditPosition editposition;
	int cursorstep;
	int hasplugincache;
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
int workspace_showmaximizedatstart(Workspace*);
int workspace_showlinenumbers(Workspace*);
void workspace_configchanged(Workspace*, Properties* property,
	Properties* choice);
void workspace_undo(Workspace*);
void workspace_redo(Workspace*);
Inputs* workspace_noteinputs(Workspace*);
void workspace_seteditposition(Workspace*, EditPosition);
EditPosition workspace_editposition(Workspace*);
void workspace_setcursorstep(Workspace*, int step);
int workspace_cursorstep(Workspace*);
const char* workspace_translate(Workspace*, const char* key);
int workspace_hasplugincache(Workspace*);

#endif

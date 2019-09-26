// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net


#if !defined(SETTINGSVIEW)
#define SETTINGSVIEW

#include <uicomponent.h>
#include <uiedit.h>
#include <properties.h>
#include "tabbar.h"
#include <hashtbl.h>

typedef struct {
	ui_component component;
	ui_component client;
	Properties* properties;
	ui_graphics* g;
	int lastlevel;
	Properties* selected;
	ui_rectangle selrect;
	int dy;
	int mx;
	int my;
	int cpy;
	int cpx;
	int fillchoice;
	int currchoice;
	int choicecount;
	Properties* choiceproperty;
	ui_edit edit;
	Signal signal_changed;
	TabBar tabbar;
	IntHashTable sectionpositions;
	int sectioncount;
} SettingsView;

void InitSettingsView(SettingsView* Settingsview, ui_component* parent, ui_component* tabbarparent, 
	Properties* properties);

#endif
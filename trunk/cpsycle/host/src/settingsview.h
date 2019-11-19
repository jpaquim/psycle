// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net


#if !defined(SETTINGSVIEW)
#define SETTINGSVIEW

#include <uicomponent.h>
#include <uiedit.h>
#include <properties.h>
#include "inputdefiner.h"
#include "tabbar.h"

typedef struct {
	ui_component component;
	ui_component client;
	Properties* properties;
	ui_graphics* g;
	int lastlevel;
	Properties* selected;
	Properties* search;
	ui_rectangle selrect;
	int dirbutton;
	int dy;
	int mx;
	int my;
	int cpy;
	int cpx;	
	int currchoice;
	int choicecount;
	int lineheight;
	int columnwidth;
	int identwidth;
	Properties* choiceproperty;
	ui_edit edit;
	InputDefiner inputdefiner;
	Signal signal_changed;
	TabBar tabbar;	
} SettingsView;

void InitSettingsView(SettingsView* Settingsview, ui_component* parent, ui_component* tabbarparent, 
	Properties* properties);

#endif

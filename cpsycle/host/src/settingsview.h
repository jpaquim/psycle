// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net


#if !defined(SETTINGSVIEW)
#define SETTINGSVIEW

#include <uicomponent.h>
#include <uiedit.h>
#include <properties.h>

typedef struct {
	ui_component component;
	Properties* properties;
	ui_graphics* g;
	int lastlevel;
	Properties* selected;
	ui_rectangle selrect;
	int mx;
	int my;
	int cpy;
	int cpx;
	int fillchoice;
	ui_edit edit;	
} SettingsView;

void InitSettingsView(SettingsView* Settingsview, ui_component* parent, Properties* properties);

#endif
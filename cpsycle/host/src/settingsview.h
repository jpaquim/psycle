// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net


#if !defined(SETTINGSVIEW)
#define SETTINGSVIEW

#include <uicomponent.h>
#include <uiedit.h>
#include <properties.h>
#include "inputdefiner.h"
#include "tabbar.h"

typedef struct {
	psy_ui_Component component;
	psy_ui_Component viewtabbar;
	psy_ui_Component client;
	psy_Properties* properties;
	psy_ui_Graphics* g;
	int lastlevel;
	psy_Properties* selected;
	psy_Properties* search;
	psy_ui_Rectangle selrect;	
	int button;
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
	psy_Properties* choiceproperty;
	psy_ui_Edit edit;
	InputDefiner inputdefiner;
	psy_Signal signal_changed;
	TabBar tabbar;	
} SettingsView;

void settingsview_init(SettingsView* Settingsview, psy_ui_Component* parent,
	psy_ui_Component* tabbarparent, psy_Properties* properties);
void settingsview_selectsection(SettingsView*, const char* key);

#endif

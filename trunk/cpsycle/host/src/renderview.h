// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#if !defined(RENDERVIEW_H)
#define RENDERVIEW_H

#include "settingsview.h"

#include <uicomponent.h>
#include <uilabel.h>
#include <uiedit.h>
#include <uibutton.h>
#include <uicheckbox.h>
#include "workspace.h"

typedef struct {
	ui_component component;
	SettingsView view;
	Properties* properties;
	Workspace* workspace;
	Driver* fileoutdriver;
	Driver* curraudiodriver;
	int restoreloopmode;
} RenderView;

void renderview_init(RenderView*, ui_component* parent,
	ui_component* tabbarparent, Workspace*);

#endif

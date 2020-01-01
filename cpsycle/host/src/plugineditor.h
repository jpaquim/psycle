// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#if !defined(PLUGINEDITOR_H)
#define PLUGINEDITOR_H

#include <uicomponent.h>
#include <uieditor.h>

#include "workspace.h"

typedef struct {
	psy_ui_Component component;
	psy_ui_Editor editor;
	Workspace* workspace;
} PluginEditor;

void plugineditor_init(PluginEditor*, psy_ui_Component* parent, Workspace*);

#endif

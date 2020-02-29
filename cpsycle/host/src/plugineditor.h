// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#if !defined(PLUGINEDITOR_H)
#define PLUGINEDITOR_H

#include <uicombobox.h>
#include <uieditor.h>
#include <uibutton.h>

#include "workspace.h"

typedef struct {
	psy_ui_Component component;
	psy_ui_Editor editor;
	psy_ui_Component bar;
	psy_ui_Button reload;
	psy_ui_Button save;
	psy_ui_ComboBox pluginselector;
	psy_ui_ComboBox fileselector;
	Workspace* workspace;
	const char* basepath;	
	uintptr_t instanceidx;
	psy_Table pluginmappping;

} PluginEditor;

void plugineditor_init(PluginEditor*, psy_ui_Component* parent, Workspace*);

#endif

// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#if !defined(PLUGINEDITOR_H)
#define PLUGINEDITOR_H

// host
#include "titlebar.h"
#include "workspace.h"
// ui
#include <uibutton.h>
#include <uicombobox.h>
#include <uitextarea.h>
#include <uieditor.h>
#include <uilabel.h>

#ifdef __cplusplus
extern "C" {
#endif

// PluginEditor
//
// Editor for lua plugins

typedef struct PluginEditorCreateBar {
	// inherits
	psy_ui_Component component;
	// internal	
	psy_ui_Label desc;
	psy_ui_TextArea name;
	psy_ui_Button create;
	// references
	Workspace* workspace;
} PluginEditorCreateBar;

void plugineditorcreatebar_init(PluginEditorCreateBar*, psy_ui_Component*
	parent, Workspace*);

typedef struct PluginEditor {
	// inherits
	psy_ui_Component component;
	// internal
	TitleBar titlebar;
	psy_ui_TextInputor editor;
	psy_ui_Component bar;
	psy_ui_Button newplugin;
	psy_ui_Button reload;
	psy_ui_Button save;
	psy_ui_Component row0;
	psy_ui_Label plugindesc;
	psy_ui_ComboBox pluginselector;
	psy_ui_Component row1;
	psy_ui_Label filedesc;
	psy_ui_ComboBox fileselector;
	PluginEditorCreateBar createbar;	
	// internal data
	const char* basepath;	
	uintptr_t instanceidx;
	psy_Table pluginmappping;
	// referneces
	Workspace* workspace;
} PluginEditor;

void plugineditor_init(PluginEditor*, psy_ui_Component* parent, Workspace*);

INLINE psy_ui_Component* plugineditor_base(PluginEditor* self)
{
	assert(self);

	return &self->component;
}

#ifdef __cplusplus
}
#endif

#endif /* PLUGINEDITOR_H */

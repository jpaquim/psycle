// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#if !defined(PLUGINSCANVIEW_H)
#define PLUGINSCANVIEW_H

// host
#include "pluginsview.h"
#include "workspace.h"
// ui
#include <uibutton.h>
#include <uilabel.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct PluginScanView {
	// inherits
	psy_ui_Component component;
	// ui elements
	psy_ui_Component left;
	psy_ui_Component right;
	psy_ui_Component client;
	psy_ui_Label scan;
	psy_ui_Label scanfile;
	psy_ui_Component abortbar;
	psy_ui_Button abort;
	// references
	Workspace* workspace;
} PluginScanView;

void pluginscanview_init(PluginScanView*, psy_ui_Component* parent,
	Workspace*);

void pluginscanview_reset(PluginScanView*);
void pluginscanview_selecttask(PluginScanView*, psy_audio_PluginScanTask*);


#ifdef __cplusplus
}
#endif

#endif /* PLUGINSCANVIEW_H */

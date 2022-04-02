/*
** This source is free software; you can redistribute itand /or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#if !defined(PLUGINSCANVIEW_H)
#define PLUGINSCANVIEW_H

/* host */
#include "clockbar.h"
#include "pluginsview.h"
#include "workspace.h"
/* ui */
#include <uibutton.h>
#include <uilabel.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct PluginScanDescView {
	/* inherits */
	psy_ui_Component component;
	/* internal */
} PluginScanDescView;

void pluginscandescview_init(PluginScanDescView*, psy_ui_Component* parent);

typedef struct PluginScanStatusView {
	/* inherits */
	psy_ui_Component component;
	/* internal */
	psy_ui_Label time_desc;
	ClockBar time;
	psy_ui_Label filenum_desc;
	psy_ui_Label filenum;
	psy_ui_Label pluginnum_desc;
	psy_ui_Label pluginnum;
	uintptr_t filecount;
	uintptr_t plugincount;
} PluginScanStatusView;

void pluginscanstatusview_init(PluginScanStatusView*, psy_ui_Component* parent);

void pluginscanstatusview_inc_file_count(PluginScanStatusView*);
void pluginscanstatusview_inc_plugin_count(PluginScanStatusView*);

typedef struct PluginScanTaskView {
	/* inherits */
	psy_ui_Component component;
	/* internal */
	psy_ui_Component tasks;
	psy_ui_Component buttons;
	psy_ui_Button pause;
	psy_ui_Button abort;
	/* references */
	psy_audio_PluginCatcher* plugincatcher;
} PluginScanTaskView;

void pluginscantaskview_init(PluginScanTaskView*, psy_ui_Component* parent,
	psy_audio_PluginCatcher*);

void pluginscantaskview_selecttask(PluginScanTaskView*,
	psy_audio_PluginScanTask*);

typedef struct PluginScanProcessView {
	/* inherits */
	psy_ui_Component component;
	/* internal */
	psy_ui_Component client;
	PluginScanDescView descview;
	PluginScanTaskView taskview;
	PluginScanStatusView statusview;	
	psy_ui_Component bottom;
	psy_ui_Label scan;
	psy_ui_Label scanfile;	
} PluginScanProcessView;

void pluginscanprocessview_init(PluginScanProcessView*, psy_ui_Component* parent,
	psy_audio_PluginCatcher*);

typedef struct PluginScanView {
	/* inherits */
	psy_ui_Component component;
	/* internal */			
	PluginScanProcessView processview;	
	/* references */
	Workspace* workspace;
} PluginScanView;

void pluginscanview_init(PluginScanView*, psy_ui_Component* parent,
	Workspace*);

void pluginscanview_reset(PluginScanView*);
void pluginscanview_scanstop(PluginScanView*);
void pluginscanview_selecttask(PluginScanView*, psy_audio_PluginScanTask*);


#ifdef __cplusplus
}
#endif

#endif /* PLUGINSCANVIEW_H */

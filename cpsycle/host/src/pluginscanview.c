/*
** This source is free software; you can redistribute itand /or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "pluginscanview.h"
/* host */
#include "resources/resource.h"
#include "styles.h"
/* platform */
#include "../../detail/portable.h"

/* PluginScanDescView */
void pluginscandescview_init(PluginScanDescView* self, psy_ui_Component* parent)
{
	psy_ui_component_init(&self->component, parent, NULL);
	psy_ui_component_set_preferred_size(&self->component,
		psy_ui_size_make_em(40.0, 0.0));
}

/* PluginScanStatusView */

static void pluginscanstatusview_update_file_count(PluginScanStatusView*);
static void pluginscanstatusview_update_plugin_count(PluginScanStatusView*);

void pluginscanstatusview_init(PluginScanStatusView* self, psy_ui_Component* parent)
{
	psy_ui_component_init(&self->component, parent, NULL);
	psy_ui_component_set_preferred_size(&self->component,
		psy_ui_size_make_em(40.0, 0.0));
	psy_ui_label_init_text(&self->time_desc, &self->component,
		"Scan time");
	psy_ui_component_set_align(psy_ui_label_base(&self->time_desc),
		psy_ui_ALIGN_TOP);
	clockbar_init(&self->time, &self->component);
	clockbar_display_minutes(&self->time);
	psy_ui_component_set_align(clockbar_base(&self->time),
		psy_ui_ALIGN_TOP);
	psy_ui_label_init_text(&self->filenum_desc, &self->component,
		"Scanned files");
	psy_ui_component_set_margin(psy_ui_label_base(&self->filenum_desc),
		psy_ui_margin_make_em(1.0, 0.0, 0.0, 0.0));
	psy_ui_component_set_align(psy_ui_label_base(&self->filenum_desc),
		psy_ui_ALIGN_TOP);	
	psy_ui_label_init(&self->filenum, &self->component);
	psy_ui_component_set_align(psy_ui_label_base(&self->filenum),
		psy_ui_ALIGN_TOP);
	/* plugin number */
	psy_ui_label_init_text(&self->pluginnum_desc, &self->component,
		"Found plugins");
	psy_ui_component_set_margin(psy_ui_label_base(&self->pluginnum_desc),
		psy_ui_margin_make_em(1.0, 0.0, 0.0, 0.0));
	psy_ui_component_set_align(psy_ui_label_base(&self->pluginnum_desc),
		psy_ui_ALIGN_TOP);
	psy_ui_label_init(&self->pluginnum, &self->component);
	psy_ui_component_set_align(psy_ui_label_base(&self->pluginnum),
		psy_ui_ALIGN_TOP);
	self->filecount = 0;
	self->plugincount = 0;
	pluginscanstatusview_update_file_count(self);
	pluginscanstatusview_update_plugin_count(self);
}

void pluginscanstatusview_inc_file_count(PluginScanStatusView* self)
{		
	++self->filecount;
	pluginscanstatusview_update_file_count(self);
}

void pluginscanstatusview_update_file_count(PluginScanStatusView* self)
{
	char text[80];

	psy_snprintf(text, 40, "%ld", (int)(self->filecount));
	psy_ui_label_set_text(&self->filenum, text);
}

void pluginscanstatusview_inc_plugin_count(PluginScanStatusView* self)
{	
	++self->plugincount;
	if (self->plugincount > self->filecount) {
		self->plugincount = self->filecount;
	}
	pluginscanstatusview_update_plugin_count(self);
}

void pluginscanstatusview_update_plugin_count(PluginScanStatusView* self)
{
	char text[80];

	psy_snprintf(text, 40, "%ld", (int)(self->plugincount));
	psy_ui_label_set_text(&self->pluginnum, text);
}

/* PluginScanTaskView */

/* prototypes */
static void pluginscantaskview_inittasklist(PluginScanTaskView*);

/* implementation */
void pluginscantaskview_init(PluginScanTaskView* self, psy_ui_Component* parent,
	psy_audio_PluginCatcher* plugincatcher)
{
	psy_ui_component_init(&self->component, parent, NULL);
	psy_ui_component_set_style_type(&self->component, STYLE_SCANTASK);
	self->plugincatcher = plugincatcher;
	psy_ui_component_init(&self->tasks, &self->component, NULL);
	psy_ui_component_set_align(&self->tasks, psy_ui_ALIGN_TOP);
	pluginscantaskview_inittasklist(self);
	/* buttons */
	psy_ui_component_init(&self->buttons, &self->component, NULL);
	psy_ui_component_set_margin(&self->buttons,
		psy_ui_margin_make_em(2.0, 0.0, 0.0, 0.0));
	psy_ui_component_set_align(&self->buttons, psy_ui_ALIGN_BOTTOM);
	psy_ui_button_init_text(&self->pause, &self->buttons,
		"Pause");
	psy_ui_component_set_align(psy_ui_button_base(&self->pause),
		psy_ui_ALIGN_LEFT);
	psy_ui_component_set_style_types(psy_ui_button_base(&self->pause),
		STYLE_DIALOG_BUTTON, STYLE_DIALOG_BUTTON_HOVER,
		STYLE_DIALOG_BUTTON_SELECT, psy_INDEX_INVALID);
	psy_ui_component_set_padding(psy_ui_button_base(&self->pause),
		psy_ui_margin_make_em(0.5, 4.0, 0.5, 4.0));
	psy_ui_button_init_text(&self->abort, &self->buttons,
		"newmachine.stop");	
	psy_ui_component_set_align(psy_ui_button_base(&self->abort),
		psy_ui_ALIGN_LEFT);	
	psy_ui_component_set_padding(psy_ui_button_base(&self->abort),
		psy_ui_margin_make_em(0.5, 4.0, 0.5, 4.0));
	psy_ui_component_set_margin(psy_ui_button_base(&self->abort),
		psy_ui_margin_make_em(0.0, 0.0, 0.0, 4.0));
	psy_ui_component_set_style_types(psy_ui_button_base(&self->abort),
		STYLE_DIALOG_BUTTON, STYLE_DIALOG_BUTTON_HOVER,
		STYLE_DIALOG_BUTTON_SELECT, psy_INDEX_INVALID);
}

void pluginscantaskview_inittasklist(PluginScanTaskView* self)
{
	psy_List* p;

	for (p = self->plugincatcher->scantasks; p != NULL; p = p->next) {
		psy_audio_PluginScanTask* task;
		psy_ui_Label* label;

		task = (psy_audio_PluginScanTask*)p->entry;
		label = psy_ui_label_allocinit(&self->tasks);
		psy_ui_label_prevent_translation(label);
		psy_ui_label_set_text(label, task->label);		
		psy_ui_component_set_margin(psy_ui_label_base(label),
			psy_ui_margin_make_em(0.0, 0.0, 1.0, 0.0));
		psy_ui_component_set_align(&label->component, psy_ui_ALIGN_TOP);
	}
}

void pluginscantaskview_selecttask(PluginScanTaskView* self,
	psy_audio_PluginScanTask* task)
{
	uintptr_t index;
	psy_List* p;

	index = 0;
	for (p = self->plugincatcher->scantasks; p != NULL; p = p->next,
		++index) {
		psy_audio_PluginScanTask* curr;

		curr = (psy_audio_PluginScanTask*)p->entry;
		if (strcmp(curr->label, task->label) == 0) {
			break;
		}
	}
	if (p) {
		psy_ui_Component* component;

		psy_ui_component_removestylestate_children(&self->tasks,
			psy_ui_STYLESTATE_SELECT);
		component = psy_ui_component_at(&self->tasks, index);
		if (component) {
			psy_ui_component_add_style_state(component, psy_ui_STYLESTATE_SELECT);
		}
	}
}

/* PluginScanProcessView */
void pluginscanprocessview_init(PluginScanProcessView* self, psy_ui_Component* parent,
	psy_audio_PluginCatcher* plugin_catcher)
{
	psy_ui_component_init(&self->component, parent, NULL);

	psy_ui_label_init_text(&self->scan, &self->component,
		"newmachine.scanning");
	psy_ui_component_set_margin(psy_ui_label_base(&self->scan),
		psy_ui_margin_make_em(2.0, 0.0, 2.0, 40.0));
	psy_ui_component_set_align(psy_ui_label_base(&self->scan),
		psy_ui_ALIGN_TOP);
	psy_ui_component_init(&self->client, &self->component, NULL);	
	psy_ui_component_set_align(&self->client, psy_ui_ALIGN_TOP);
	pluginscandescview_init(&self->descview, &self->client);
	psy_ui_component_set_align(&self->descview.component,
		psy_ui_ALIGN_LEFT);	
	pluginscanstatusview_init(&self->statusview, &self->client);
	psy_ui_component_set_align(&self->statusview.component,
		psy_ui_ALIGN_RIGHT);
	psy_ui_component_set_margin(&self->statusview.component,
		psy_ui_margin_make_em(0.0, 0.0, 0.0, 4.0));
	pluginscantaskview_init(&self->taskview, &self->client,
		plugin_catcher);
	psy_ui_component_set_align(&self->taskview.component,
		psy_ui_ALIGN_TOP);	
	// filename
	psy_ui_component_init(&self->bottom, &self->component, NULL);
	psy_ui_component_set_align(&self->bottom, psy_ui_ALIGN_BOTTOM);
	psy_ui_label_init(&self->scanfile, &self->bottom);
	psy_ui_label_prevent_translation(&self->scanfile);
	psy_ui_label_set_text_alignment(&self->scanfile,
		psy_ui_ALIGNMENT_CENTER_HORIZONTAL);
	psy_ui_component_set_align(psy_ui_label_base(&self->scanfile),
		psy_ui_ALIGN_TOP);
	psy_ui_component_set_margin(psy_ui_label_base(&self->scanfile),
		psy_ui_margin_make_em(0.0, 0.0, 0.0, 0.0));	
}

/* PluginScanView */

/* prototypes */
void pluginscanview_inittasklist(PluginScanView*);
void pluginscanview_onabort(PluginScanView*, psy_ui_Button* sender);

/* implementation */
void pluginscanview_init(PluginScanView* self, psy_ui_Component* parent,
	Workspace* workspace)
{
	psy_ui_component_init(&self->component, parent, NULL);
	self->workspace = workspace;
	psy_ui_component_set_padding(&self->component,
		psy_ui_margin_make_em(0.5, 0.5, 0.0, 0.0));
	pluginscanprocessview_init(&self->processview, &self->component,
		&workspace->player.plugincatcher);
	psy_ui_component_set_align(&self->processview.component,
		psy_ui_ALIGN_CLIENT);
	psy_signal_connect(&self->processview.taskview.abort.signal_clicked,
		self, pluginscanview_onabort);
}

void pluginscanview_onabort(PluginScanView* self, psy_ui_Button* sender)
{
	psy_ui_button_set_text(&self->processview.taskview.abort,
		"newmachine.stopping");
	psy_ui_component_align_full(&self->processview.taskview.buttons);
	psy_ui_component_invalidate(&self->processview.taskview.buttons);
	psy_audio_plugincatcher_abort(workspace_plugincatcher(self->workspace));
}

void pluginscanview_selecttask(PluginScanView* self,
	psy_audio_PluginScanTask* task)
{
	pluginscantaskview_selecttask(&self->processview.taskview, task);
}

void pluginscanview_reset(PluginScanView* self)
{
	psy_ui_button_set_text(&self->processview.taskview.abort,
		"newmachine.stop");
	self->processview.statusview.filecount = 0;
	clockbar_reset(&self->processview.statusview.time);
	clockbar_start(&self->processview.statusview.time);
}

void pluginscanview_scanstop(PluginScanView* self)
{		
	clockbar_stop(&self->processview.statusview.time);	
}

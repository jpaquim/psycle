// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "pluginscanview.h"
// host
#include "resources/resource.h"
#include "styles.h"
// platform
#include "../../detail/portable.h"

// PluginScanView
// prototypes
void pluginscanview_inittasklist(PluginScanView*);
void pluginscanview_onabort(PluginScanView*, psy_ui_Button* sender);
// implementation
void pluginscanview_init(PluginScanView* self, psy_ui_Component* parent,
	Workspace* workspace)
{
	psy_ui_component_init(&self->component, parent, NULL);
	self->workspace = workspace;
	psy_ui_component_set_padding(&self->component,
		psy_ui_margin_make_em(0.5, 0.5, 0.0, 0.0));
	psy_ui_label_init_text(&self->scan, &self->component,
		"newmachine.scanning");
	psy_ui_component_setalign(psy_ui_label_base(&self->scan),
		psy_ui_ALIGN_TOP);
	psy_ui_component_set_margin(psy_ui_label_base(&self->scan),
		psy_ui_margin_make_em(2.0, 0.0, 2.0, 0.0));
	psy_ui_component_init_align(&self->left, &self->component, NULL,
		psy_ui_ALIGN_LEFT);	
	psy_ui_component_set_padding(&self->left,
		psy_ui_margin_make_em(0.0, 4.0, 0.0, 4.0));
	psy_ui_component_init_align(&self->right, &self->component, NULL,
		psy_ui_ALIGN_RIGHT);
	psy_ui_component_init_align(&self->client, &self->component, NULL,
		psy_ui_ALIGN_CLIENT);	
	// filename
	psy_ui_label_init(&self->scanfile, &self->client);
	psy_ui_label_preventtranslation(&self->scanfile);	
	psy_ui_component_setalign(psy_ui_label_base(&self->scanfile),
		psy_ui_ALIGN_TOP);
	psy_ui_component_set_margin(psy_ui_label_base(&self->scanfile),
		psy_ui_margin_make_em(0.0, 0.0, 0.0, 0.0));
	psy_ui_component_init(&self->abortbar, &self->client, NULL);
	psy_ui_component_set_margin(&self->abortbar,
		psy_ui_margin_make_em(2.0, 0.0, 0.0, 20.0));
	psy_ui_component_setalign(&self->abortbar, psy_ui_ALIGN_TOP);
	psy_ui_button_init_text_connect(&self->abort, &self->abortbar,
		"newmachine.stop", self, pluginscanview_onabort);
	psy_ui_component_setalign(psy_ui_button_base(&self->abort),
		psy_ui_ALIGN_LEFT);	
	pluginscanview_inittasklist(self);
}

void pluginscanview_inittasklist(PluginScanView* self)
{
	psy_List* p;

	for (p = self->workspace->plugincatcher.scantasks; p != NULL; p = p->next) {
		psy_audio_PluginScanTask* task;
		psy_ui_Label* label;
		
		task = (psy_audio_PluginScanTask*)p->entry;
		label = psy_ui_label_allocinit(&self->left);
		psy_ui_label_preventtranslation(label);
		psy_ui_label_settext(label, task->label);
		psy_ui_component_setstyletype_select(&label->component,
			psy_ui_STYLE_BUTTON_SELECT);
		psy_ui_component_set_margin(psy_ui_label_base(label),
			psy_ui_margin_make_em(0.0, 0.0, 1.0, 0.0));
		psy_ui_component_setalign(&label->component, psy_ui_ALIGN_TOP);
	}
}

void pluginscanview_onabort(PluginScanView* self, psy_ui_Button* sender)
{
	psy_ui_button_settext(&self->abort, "newmachine.stopping");
	psy_ui_component_align_full(&self->client);	
	psy_audio_plugincatcher_abort(workspace_plugincatcher(self->workspace));
}

void pluginscanview_selecttask(PluginScanView* self, psy_audio_PluginScanTask* task)
{
	uintptr_t index;
	psy_List* p;

	index = 0;
	for (p = self->workspace->plugincatcher.scantasks; p != NULL; p = p->next,
			++index) {
		psy_audio_PluginScanTask* curr;

		curr = (psy_audio_PluginScanTask*)p->entry;
		if (strcmp(curr->label, task->label) == 0) {
			break;
		}
	}
	if (p) {
		psy_ui_Component* component;

		psy_ui_component_removestylestate_children(&self->left,
			psy_ui_STYLESTATE_SELECT);
		component = psy_ui_component_at(&self->left, index);
		if (component) {
			psy_ui_component_addstylestate(component, psy_ui_STYLESTATE_SELECT);
		}
	}
}

void pluginscanview_reset(PluginScanView* self)
{
	psy_ui_button_settext(&self->abort, "newmachine.stop");
}

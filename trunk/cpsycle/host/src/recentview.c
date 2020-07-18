// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "recentview.h"
#include <string.h>

static void recentview_onselected(RecentView*, PropertiesView* sender,
	psy_Properties*);
static void recentview_onclear(RecentView*, PropertiesView* sender);

void recentview_init(RecentView* self, psy_ui_Component* parent,
	psy_ui_Component* tabbarparent, Workspace* workspace)
{	
	psy_ui_component_init(&self->component, parent);
	psy_ui_component_enablealign(&self->component);
	psy_ui_component_setbackgroundmode(&self->component, psy_ui_BACKGROUND_NONE);
	self->workspace = workspace;
	psy_ui_button_init(&self->clear, &self->component);
	psy_ui_button_settext(&self->clear, "Clear");
	psy_ui_component_setalign(&self->clear.component, psy_ui_ALIGN_TOP);
	psy_signal_connect(&self->clear.signal_clicked, self,
		recentview_onclear);
	propertiesview_init(&self->view, &self->component, tabbarparent,
		workspace_recentsongs(workspace), workspace);
	propertiesview_setcolumnwidth(&self->view, 1.f, 0.f, 0.f);
	psy_ui_component_hide(&self->view.tabbar.component);
	propertiesrenderer_setfixedwidth(&self->view.renderer,
		psy_ui_value_makeew(40));	
	psy_ui_component_setalign(&self->view.component, psy_ui_ALIGN_CLIENT);
	psy_signal_connect(&self->view.signal_selected, self,
		recentview_onselected);
}

void recentview_onselected(RecentView* self, PropertiesView* sender,
	psy_Properties* property)
{
	if (psy_properties_insection(property, self->workspace->recentfiles)) {
		if (!self->workspace->filename || strcmp(self->workspace->filename,
				psy_properties_key(property)) != 0) {
			workspace_loadsong(self->workspace,
				psy_properties_key(property),
				workspace_playsongafterload(self->workspace));
		}
	}
}

void recentview_onclear(RecentView* self, PropertiesView* sender)
{
	workspace_clearrecentsongs(self->workspace);
	psy_ui_component_align(&self->view.component);
	psy_ui_component_invalidate(&self->view.component);
}

void recentview_onmousedown(RecentView* self, psy_ui_MouseEvent* ev)
{

}
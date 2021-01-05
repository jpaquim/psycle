// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "exportview.h"

#include "../../driver/audiodriversettings.h"

#include <string.h>

#include "../../detail/trace.h"

static void exportview_ondestroy(ExportView*, psy_ui_Component* sender);
static void exportview_makeproperties(ExportView*);
static void exportview_onsettingsviewchanged(ExportView*, PropertiesView* sender,
	psy_Property*);
static void exportview_exportmodule(ExportView*);
static void exportview_exportmidifile(ExportView*);
static void exportview_onfocus(ExportView*, psy_ui_Component* sender);

void exportview_init(ExportView* self, psy_ui_Component* parent,
	psy_ui_Component* tabbarparent, Workspace* workspace)
{	
	self->workspace = workspace;
	psy_ui_component_init(&self->component, parent);
	psy_signal_connect(&self->component.signal_destroy, self,
		exportview_ondestroy);
	psy_signal_connect(&self->component.signal_focus,
		self, exportview_onfocus);
	exportview_makeproperties(self);
	propertiesview_init(&self->view, &self->component, tabbarparent,
		self->properties, workspace);
	psy_signal_connect(&self->view.signal_changed, self,
		exportview_onsettingsviewchanged);
	psy_ui_component_setalign(&self->view.component, psy_ui_ALIGN_CLIENT);	
}

void exportview_ondestroy(ExportView* self, psy_ui_Component* sender)
{
	psy_property_deallocate(self->properties);	
}

void exportview_makeproperties(ExportView* self)
{	
	psy_Property* actions;

	self->properties = psy_property_allocinit_key(NULL);
	actions = psy_property_settext(
		psy_property_append_section(self->properties, "actions"),
		"Export");
	psy_property_settext(
		psy_property_append_action(actions, "exportmodule"),
		"export.export-module");
	psy_property_settext(
		psy_property_append_action(actions, "exportmidifile"),
		"export.export-midifile");
}

void exportview_onsettingsviewchanged(ExportView* self, PropertiesView* sender,
	psy_Property* property)
{
	if (psy_property_type(property) == PSY_PROPERTY_TYPE_ACTION) {
		if (strcmp(psy_property_key(property), "exportmodule") == 0) {
			exportview_exportmodule(self);
		} else if (strcmp(psy_property_key(property), "exportmidifile") == 0) {
			exportview_exportmidifile(self);
		}
	}
}

void exportview_exportmodule(ExportView* self)
{	
	workspace_exportsong(self->workspace);
}

void exportview_exportmidifile(ExportView* self)
{
	workspace_exportmidifile_fileselect(self->workspace);
}

void exportview_onfocus(ExportView* self, psy_ui_Component* sender)
{
	psy_ui_component_setfocus(&self->view.component);
}
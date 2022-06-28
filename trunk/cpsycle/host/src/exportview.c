/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "exportview.h"

#include "../../driver/audiodriversettings.h"

#include <string.h>

#include "../../detail/trace.h"

/* prototypes */
static void exportview_on_destroyed(ExportView*);
static void exportview_make_properties(ExportView*);
static void exportview_on_settings_view_changed(ExportView*,
	PropertiesView* sender, psy_Property*, uintptr_t* rebuild);
static void exportview_export_module(ExportView*);
static void exportview_export_midi_file(ExportView*);
static void exportview_export_ly_file(ExportView*);
static void exportview_on_focus(ExportView*, psy_ui_Component* sender);

/* vtable */
static psy_ui_ComponentVtable vtable;
static bool vtable_initialized = FALSE;

static void vtable_init(ExportView* self)
{
	if (!vtable_initialized) {
		vtable = *(self->component.vtable);
		vtable.on_destroyed =
			(psy_ui_fp_component_event)
			exportview_on_destroyed;
		vtable_initialized = TRUE;
	}
	psy_ui_component_set_vtable(&self->component, &vtable);
}

/* implementation */
void exportview_init(ExportView* self, psy_ui_Component* parent,
	psy_ui_Component* tabbarparent, Workspace* workspace)
{		
	psy_ui_component_init(&self->component, parent, NULL);
	vtable_init(self);
	psy_ui_component_set_id(&self->component, VIEW_ID_EXPORTVIEW);
	self->workspace = workspace;	
	psy_signal_connect(&self->component.signal_focus,
		self, exportview_on_focus);
	exportview_make_properties(self);
	propertiesview_init(&self->view, &self->component,
		tabbarparent, self->properties, 3, &workspace->inputhandler);
	psy_signal_connect(&self->view.signal_changed, self,
		exportview_on_settings_view_changed);
	psy_ui_component_set_align(&self->view.component, psy_ui_ALIGN_CLIENT);
}

void exportview_on_destroyed(ExportView* self)
{
	psy_property_deallocate(self->properties);	
}

void exportview_make_properties(ExportView* self)
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
	psy_property_settext(
		psy_property_append_action(actions, "exportlyfile"),
		"export.export-lyfile");
}

void exportview_on_settings_view_changed(ExportView* self, PropertiesView* sender,
	psy_Property* property, uintptr_t* rebuild)
{
	if (psy_property_type(property) == PSY_PROPERTY_TYPE_ACTION) {
		if (strcmp(psy_property_key(property), "exportmodule") == 0) {
			exportview_export_module(self);
		} else if (strcmp(psy_property_key(property), "exportmidifile") == 0) {
			exportview_export_midi_file(self);
		} else if (strcmp(psy_property_key(property), "exportlyfile") == 0) {
			exportview_export_ly_file(self);
		}
	}
}

void exportview_export_module(ExportView* self)
{	
	workspace_export_song(self->workspace);
}

void exportview_export_midi_file(ExportView* self)
{
	workspace_export_midi_fileselect(self->workspace);
}

void exportview_export_ly_file(ExportView* self)
{
	workspace_export_ly_fileselect(self->workspace);
}


void exportview_on_focus(ExportView* self, psy_ui_Component* sender)
{
	psy_ui_component_set_focus(&self->view.component);
}

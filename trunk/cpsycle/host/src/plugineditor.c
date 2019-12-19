// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "plugineditor.h"

#include <portable.h>

static void plugineditor_onmachineschangeslot(PluginEditor*, Machines*,
	uintptr_t slot);
static void plugineditor_onsongchanged(PluginEditor*, Workspace*);
static void plugineditor_connectmachinesignals(PluginEditor* self,
	Workspace* workspace);

static void plugineditor_ondestroy(PluginEditor*, ui_component* sender);

void plugineditor_init(PluginEditor* self, ui_component* parent,
	Workspace* workspace)
{							
	self->workspace = workspace;
	ui_component_init(&self->component, parent);
	ui_component_enablealign(&self->component);
	ui_editor_init(&self->editor, &self->component);
	ui_component_setalign(&self->editor.component, UI_ALIGN_CLIENT);
	psy_signal_connect(&self->component.signal_destroy, self,
		plugineditor_ondestroy);
	psy_signal_connect(&workspace->signal_songchanged, self,
		plugineditor_onsongchanged);	
	ui_component_resize(&self->component, 400, 0);
	plugineditor_connectmachinesignals(self, workspace);
}

void plugineditor_ondestroy(PluginEditor* self, ui_component* sender)
{
}

void plugineditor_onmachineschangeslot(PluginEditor* self, Machines* machines,
	uintptr_t slot)
{
	Machine* machine;

	machine = machines_at(machines, slot);
	if (machine && machine->vtable->info(machine)->type == MACH_LUA) {
		const char* path;
		
		path = plugincatcher_searchpath(
			&self->workspace->plugincatcher, 
			machine->vtable->info(machine)->Name,
			MACH_LUA);
		if (path) {
			ui_editor_load(&self->editor, path);
		}
	}
}

void plugineditor_onsongchanged(PluginEditor* self, Workspace* workspace)
{
	plugineditor_connectmachinesignals(self, workspace);
}

void plugineditor_connectmachinesignals(PluginEditor* self, Workspace* workspace)
{
	if (workspace->song) {
		psy_signal_connect(&workspace->song->machines.signal_slotchange, self,
			plugineditor_onmachineschangeslot);
	}
}


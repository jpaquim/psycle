// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "plugineditor.h"

#include "../../detail/portable.h"

static void plugineditor_onmachineschangeslot(PluginEditor*,
	psy_audio_Machines*, uintptr_t slot);
static void plugineditor_onsongchanged(PluginEditor*, Workspace*);
static void plugineditor_connectmachinesignals(PluginEditor*, Workspace*);
static void plugineditor_ondestroy(PluginEditor*, psy_ui_Component* sender);

void plugineditor_init(PluginEditor* self, psy_ui_Component* parent,
	Workspace* workspace)
{							
	self->workspace = workspace;
	psy_ui_component_init(&self->component, parent);
	psy_ui_component_enablealign(&self->component);
	psy_ui_editor_init(&self->editor, &self->component);
	psy_ui_component_setalign(&self->editor.component, psy_ui_ALIGN_CLIENT);
	psy_signal_connect(&self->component.signal_destroy, self,
		plugineditor_ondestroy);
	psy_signal_connect(&workspace->signal_songchanged, self,
		plugineditor_onsongchanged);	
	psy_ui_component_resize(&self->component, 400, 0);
	plugineditor_connectmachinesignals(self, workspace);
}

void plugineditor_ondestroy(PluginEditor* self, psy_ui_Component* sender)
{
}

void plugineditor_onmachineschangeslot(PluginEditor* self,
	psy_audio_Machines* machines, uintptr_t slot)
{
	psy_audio_Machine* machine;

	machine = machines_at(machines, slot);
	if (machine && psy_audio_machine_info(machine) &&
			psy_audio_machine_info(machine)->type == MACH_LUA) {
		const char* path;
		
		path = plugincatcher_searchpath(
			&self->workspace->plugincatcher, 
			machine->vtable->info(machine)->Name,
			MACH_LUA);
		if (path) {
			psy_ui_editor_load(&self->editor, path);
		}
	}
}

void plugineditor_onsongchanged(PluginEditor* self, Workspace* workspace)
{
	plugineditor_connectmachinesignals(self, workspace);
}

void plugineditor_connectmachinesignals(PluginEditor* self,
	Workspace* workspace)
{
	if (workspace->song) {
		psy_signal_connect(&workspace->song->machines.signal_slotchange, self,
			plugineditor_onmachineschangeslot);
	}
}


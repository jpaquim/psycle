// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "plugineditor.h"

#include <string.h>
#include <dir.h>

#include "../../detail/portable.h"

#include <luaplugin.h>
#include <exclusivelock.h>

#include <songio.h>

static void plugineditor_onmachineschangeslot(PluginEditor*,
	psy_audio_Machines*, uintptr_t slot);
static void plugineditor_onsongchanged(PluginEditor*, Workspace*, int flag,
	psy_audio_SongFile*);
static void plugineditor_connectmachinesignals(PluginEditor*, Workspace*);
static void plugineditor_ondestroy(PluginEditor*, psy_ui_Component* sender);
static void plugineditor_onreload(PluginEditor*, psy_ui_Component* sender);
static void plugineditor_onsave(PluginEditor*, psy_ui_Component* sender);
static void plugineditor_buildpluginlist(PluginEditor*);
static void plugineditor_buildfilelist(PluginEditor*);
static void plugineditor_onpluginselected(PluginEditor*, psy_ui_Component* sender, int slot);
static void plugineditor_onfileselected(PluginEditor*, psy_ui_Component* sender, int slot);
static int plugineditor_onenumdir(PluginEditor*, const char* path, int flag);

void plugineditor_init(PluginEditor* self, psy_ui_Component* parent,
	Workspace* workspace)
{	
	psy_ui_component_init(&self->component, parent);
	psy_ui_component_enablealign(&self->component);
	self->workspace = workspace;
	self->basepath = 0;	
	self->instanceidx = NOMACHINE_INDEX;
	psy_ui_component_init(&self->bar, &self->component);
	psy_ui_component_enablealign(&self->bar);
	psy_ui_component_setalign(&self->bar, psy_ui_ALIGN_TOP);
	psy_ui_button_init(&self->reload, &self->bar);
	psy_ui_component_setalign(&self->reload.component, psy_ui_ALIGN_LEFT);
	psy_ui_button_settext(&self->reload, "Reload");
	psy_signal_connect(&self->reload.signal_clicked, self,
		&plugineditor_onreload);
	psy_ui_button_init(&self->save, &self->bar);
	psy_ui_component_setalign(&self->save.component, psy_ui_ALIGN_LEFT);
	psy_ui_button_settext(&self->save, "Save");
	psy_signal_connect(&self->save.signal_clicked, self,
		&plugineditor_onsave);	
	psy_ui_combobox_init(&self->pluginselector, &self->bar);
	psy_ui_combobox_setcharnumber(&self->pluginselector, 32);
	psy_ui_component_setalign(&self->pluginselector.component, psy_ui_ALIGN_LEFT);
	psy_ui_combobox_init(&self->fileselector, &self->bar);
	psy_ui_combobox_setcharnumber(&self->fileselector, 32);
	psy_ui_component_setalign(&self->fileselector.component, psy_ui_ALIGN_LEFT);
	psy_ui_editor_init(&self->editor, &self->component);
	psy_ui_component_setalign(&self->editor.component, psy_ui_ALIGN_CLIENT);
	psy_signal_connect(&self->component.signal_destroy, self,
		plugineditor_ondestroy);
	psy_signal_connect(&workspace->signal_songchanged, self,
		plugineditor_onsongchanged);
	psy_signal_connect(&self->pluginselector.signal_selchanged, self,
		plugineditor_onpluginselected);
	psy_signal_connect(&self->fileselector.signal_selchanged, self,
		plugineditor_onfileselected);
	psy_ui_component_resize(&self->component, 400, 0);
	psy_table_init(&self->pluginmappping);
	plugineditor_buildpluginlist(self);
	plugineditor_connectmachinesignals(self, workspace);
}

void plugineditor_ondestroy(PluginEditor* self, psy_ui_Component* sender)
{
	psy_table_dispose(&self->pluginmappping);
}

void plugineditor_onmachineschangeslot(PluginEditor* self,
	psy_audio_Machines* machines, uintptr_t slot)
{
	psy_audio_Machine* machine;

	machine = machines_at(machines, slot);
	if (machine && psy_audio_machine_info(machine) &&
			psy_audio_machine_info(machine)->type == MACH_LUA) {
		const char* path;
		char catchername[256];
		
		psy_snprintf(catchername, 256, "%s:0",
			psy_audio_machine_info(machine)->Name);		
		path = plugincatcher_searchpath(&self->workspace->plugincatcher, catchername,
			MACH_LUA);
		if (path) {
			if (self->basepath == 0 || (strcmp(path, self->basepath) != 0)) {
				psy_ui_editor_load(&self->editor, path);
				self->basepath = path;
			}
			self->instanceidx = slot;
		}		
	}
}

void plugineditor_onsongchanged(PluginEditor* self, Workspace* workspace, int flag, psy_audio_SongFile* songfile)
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

void plugineditor_onreload(PluginEditor* self, psy_ui_Component* sender)
{
	if (self->workspace->song && self->instanceidx) {
		psy_audio_Machine* machine;

		machine = machines_at(&self->workspace->song->machines, self->instanceidx);
		if (machine) {			
			psy_audio_exclusivelock_enter();
			psy_audio_machine_reload(machine);
			psy_audio_exclusivelock_leave();
		}
	}
}

void plugineditor_onsave(PluginEditor* self, psy_ui_Component* sender)
{
	char path[4096];

	psy_psy_ui_combobox_text(&self->fileselector, path);
	if (path[0] != '\0') {
		psy_ui_editor_save(&self->editor, path);
	}
}

void plugineditor_buildpluginlist(PluginEditor* self)
{
	psy_Properties* p;
	int c;
	
	psy_table_clear(&self->pluginmappping);
	psy_ui_combobox_clear(&self->pluginselector);
	p = self->workspace->plugincatcher.plugins;
	if (p) {
		p = p->children;
	}
	for (c = 0; p != 0; p = psy_properties_next(p)) {
		if (psy_properties_int(p, "type", -1) == MACH_LUA) {
			psy_ui_combobox_addtext(&self->pluginselector, psy_properties_key(p));
			psy_table_insert(&self->pluginmappping, (uintptr_t) c, p);
			++c;
		}
	}
}

void plugineditor_onpluginselected(PluginEditor* self, psy_ui_Component* sender, int slot)
{
	psy_Properties* p;

	p = psy_table_at(&self->pluginmappping, slot);
	if (p) {
		const char* path;

		path = psy_properties_readstring(p, "path", 0);
		if (path) {
			if (self->basepath == 0 || (strcmp(path, self->basepath) != 0)) {
				psy_ui_editor_load(&self->editor, path);
				self->basepath = path;
				self->instanceidx = NOMACHINE_INDEX;
				plugineditor_buildfilelist(self);
			}
		}
	}	
}

void plugineditor_onfileselected(PluginEditor* self, psy_ui_Component* sender, int slot)
{
	char path[4096];

	psy_psy_ui_combobox_text(&self->fileselector, path);
	psy_ui_editor_load(&self->editor, path);	
}

void plugineditor_buildfilelist(PluginEditor* self)
{
	if (self->basepath) {
		char prefix[4096];
		char name[4096];
		char ext[4096];
		char path[4096];

		psy_ui_combobox_clear(&self->fileselector);
		psy_ui_combobox_addtext(&self->fileselector, self->basepath);
		psy_dir_extract_path(self->basepath, prefix, name, ext);		
		psy_snprintf(path, 4096, "%s\\%s", prefix, name);		
		psy_dir_enumerate_recursive(self, path, "*.lua",
			MACH_PLUGIN, plugineditor_onenumdir);
	}
}

int plugineditor_onenumdir(PluginEditor* self, const char* path, int type)
{
	psy_ui_combobox_addtext(&self->fileselector, path);
	return 1;
}
// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#define BLOCKSIZE 128 * 1024

#include "plugineditor.h"

#include <string.h>
#include <dir.h>

#include <luaplugin.h>
#include <exclusivelock.h>

#include <songio.h>

#include "../../detail/portable.h"

static const char* pluginsource =
"function psycle.info()""\n"
"  local machinemodes = require('psycle.machinemodes')""\n"
"  return  {""\n"
"	 vendor = 'psycle',""\n"
"	 name = 'newplugin',""\n"
"	 generator = 1, --machine.GENERATOR,""\n"
"	 mode = machinemodes.GENERATOR,""\n"
"	 version = 0,""\n"
"	 api = 0 --noteon = 1""\n"
"  }""\n"
"end""\n"
"""\n"
"function psycle.start()""\n"
"  psycle.setmachine(require('newplugin.machine'))""\n"
"end""\n"
"""\n";

static const char* machinesource =
"machine = require('psycle.machine'):new()""\n"
"""\n"
"function machine:init(samplerate)""\n"
"end""\n"
"""\n"
"function machine:work(num)""\n"
"end""\n"
"""\n"
"return machine""\n";

void plugineditorcreatebar_init(PluginEditorCreateBar* self,
	psy_ui_Component* parent, Workspace* workspace)
{
	psy_ui_component_init(&self->component, parent);	
	self->workspace = workspace;
	psy_ui_label_init(&self->desc, &self->component);
	psy_ui_label_settext(&self->desc, "Plugin Name");
	psy_ui_component_setalign(&self->desc.component, psy_ui_ALIGN_LEFT);
	psy_ui_edit_init(&self->name, &self->component);
	psy_ui_component_setalign(&self->name.component, psy_ui_ALIGN_CLIENT);
	psy_ui_edit_settext(&self->name, "newplugin");
	psy_ui_button_init(&self->create, &self->component);
	psy_ui_component_setalign(&self->create.component, psy_ui_ALIGN_RIGHT);
	psy_ui_button_settext(&self->create, "Create");
}

static void plugineditor_onmachineschangeslot(PluginEditor*,
	psy_audio_Machines*, uintptr_t slot);
static void plugineditor_onsongchanged(PluginEditor*, Workspace*, int flag,
	psy_audio_SongFile*);
static void plugineditor_connectmachinesignals(PluginEditor*, Workspace*);
static void plugineditor_ondestroy(PluginEditor*, psy_ui_Component* sender);
static void plugineditor_onnewplugin(PluginEditor*, psy_ui_Component* sender);
static void plugineditor_onreload(PluginEditor*, psy_ui_Component* sender);
static void plugineditor_onsave(PluginEditor*, psy_ui_Component* sender);
static void plugineditor_buildpluginlist(PluginEditor*);
static void plugineditor_buildfilelist(PluginEditor*);
static void plugineditor_onpluginselected(PluginEditor*, psy_ui_Component* sender, int slot);
static void plugineditor_onfileselected(PluginEditor*, psy_ui_Component* sender, int slot);
static int plugineditor_onenumdir(PluginEditor*, const char* path, int flag);
static void plugineditor_oncreatenewplugin(PluginEditor*, psy_ui_Component* sender);
static void writetext(const char* path, const char* text);

void plugineditor_init(PluginEditor* self, psy_ui_Component* parent,
	Workspace* workspace)
{	
	psy_ui_component_init(&self->component, parent);	
	self->workspace = workspace;
	self->basepath = 0;	
	self->instanceidx = UINTPTR_MAX;
	psy_ui_component_init(&self->bar, &self->component);
	psy_ui_component_enablealign(&self->bar);
	psy_ui_component_setalign(&self->bar, psy_ui_ALIGN_TOP);
	psy_ui_button_init(&self->reload, &self->bar);
	psy_ui_component_setalign(&self->reload.component, psy_ui_ALIGN_LEFT);
	psy_ui_button_init(&self->newplugin, &self->bar);
	psy_ui_button_settext(&self->newplugin, "New Plugin");
	psy_ui_component_setalign(&self->newplugin.component, psy_ui_ALIGN_LEFT);
	psy_signal_connect(&self->newplugin.signal_clicked, self,
		&plugineditor_onnewplugin);
	psy_ui_button_settext(&self->reload, "Reload");
	psy_signal_connect(&self->reload.signal_clicked, self,
		&plugineditor_onreload);
	psy_ui_button_init(&self->save, &self->bar);
	psy_ui_component_setalign(&self->save.component, psy_ui_ALIGN_LEFT);
	psy_ui_button_settext(&self->save, "Save");
	psy_signal_connect(&self->save.signal_clicked, self,
		&plugineditor_onsave);	
	// Plugin select
	psy_ui_component_init(&self->row0, &self->component);
	psy_ui_component_enablealign(&self->row0);
	psy_ui_component_setalign(&self->row0, psy_ui_ALIGN_TOP);
	psy_ui_label_init(&self->plugindesc, &self->row0);
	psy_ui_label_settext(&self->plugindesc, "Plugin");
	psy_ui_component_setalign(&self->plugindesc.component, psy_ui_ALIGN_LEFT);
	psy_ui_combobox_init(&self->pluginselector, &self->row0);
	//psy_ui_combobox_setcharnumber(&self->pluginselector, 32);
	psy_ui_component_setalign(&self->pluginselector.component, psy_ui_ALIGN_CLIENT);
	// File Select
	psy_ui_component_init(&self->row1, &self->component);
	psy_ui_component_enablealign(&self->row1);
	psy_ui_component_setalign(&self->row1, psy_ui_ALIGN_TOP);
	psy_ui_label_init(&self->filedesc, &self->row1);
	psy_ui_label_settext(&self->filedesc, "File");
	psy_ui_component_setalign(&self->filedesc.component, psy_ui_ALIGN_LEFT);
	psy_ui_combobox_init(&self->fileselector, &self->row1);
	//psy_ui_combobox_setcharnumber(&self->fileselector, 32);
	psy_ui_component_setalign(&self->fileselector.component, psy_ui_ALIGN_CLIENT);
	plugineditorcreatebar_init(&self->createbar, &self->component, workspace);
	psy_signal_connect(&self->createbar.create.signal_clicked, self,
		plugineditor_oncreatenewplugin);
	psy_ui_component_setalign(&self->createbar.component, psy_ui_ALIGN_TOP);
	psy_ui_component_hide(&self->createbar.component);
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
	psy_ui_component_setpreferredsize(&self->component,
		psy_ui_size_make(
		psy_ui_value_makeew(80), psy_ui_value_makepx(0)));
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

	machine = psy_audio_machines_at(machines, slot);
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

void plugineditor_onnewplugin(PluginEditor* self, psy_ui_Component* sender)
{
	psy_ui_component_show(&self->createbar.component);
	psy_ui_component_align(&self->component);
}

void plugineditor_onreload(PluginEditor* self, psy_ui_Component* sender)
{
	if (self->workspace->song && self->instanceidx) {
		psy_audio_Machine* machine;

		machine = psy_audio_machines_at(&self->workspace->song->machines, self->instanceidx);
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
	for (c = 0; p != NULL; p = psy_properties_next(p)) {
		if (psy_properties_at_int(p, "type", -1) == MACH_LUA) {
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

		path = psy_properties_at_str(p, "path", 0);
		if (path) {
			if (self->basepath == 0 || (strcmp(path, self->basepath) != 0)) {
				psy_ui_editor_clear(&self->editor);
				psy_ui_editor_load(&self->editor, path);
				self->basepath = path;
				self->instanceidx = UINTPTR_MAX;
				plugineditor_buildfilelist(self);
				psy_ui_combobox_setcursel(&self->fileselector, 1);
				plugineditor_onfileselected(self, &self->fileselector.component, 1);
			}
		}
	}	
}

void plugineditor_onfileselected(PluginEditor* self, psy_ui_Component* sender, int slot)
{
	char path[4096];

	psy_psy_ui_combobox_text(&self->fileselector, path);
	psy_ui_editor_clear(&self->editor);
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
			MACH_PLUGIN, (psy_fp_findfile)plugineditor_onenumdir);		
	}
}

int plugineditor_onenumdir(PluginEditor* self, const char* path, int type)
{
	psy_ui_combobox_addtext(&self->fileselector, path);
	return 1;
}

void plugineditor_oncreatenewplugin(PluginEditor* self, psy_ui_Component* sender)
{
	const char* dir;
	psy_Path path;	
	psy_Path plugindir;
	
	dir = workspace_luascripts_directory(self->workspace);
	psy_path_init(&path, "");
	psy_path_setprefix(&path, dir);
	psy_path_setname(&path, "newplugin.lua");
	writetext(psy_path_path(&path), pluginsource);
	psy_path_dispose(&path);
	psy_path_init(&plugindir, "");
	psy_path_setprefix(&plugindir, dir);
	psy_path_setname(&plugindir, "newplugin");
	if (!psy_direxists(psy_path_path(&plugindir))) {
		psy_mkdir(psy_path_path(&plugindir));
	}
	psy_path_init(&path, "");
	psy_path_setprefix(&path, psy_path_path(&plugindir));
	psy_path_dispose(&plugindir);	
	psy_path_setname(&path, "machine.lua");
	writetext(psy_path_path(&path), machinesource);
	psy_path_dispose(&path);
}

void writetext(const char* path, const char* text)
{
	FILE* fp;

	fp = fopen(path, "wb");
	if (fp) {		
		int lengthdoc;
		int i;

		lengthdoc = strlen(text);
		for (i = 0; i < lengthdoc; i += BLOCKSIZE) {
			int grabsize;

			grabsize = lengthdoc - i;
			if (grabsize > BLOCKSIZE) {
				grabsize = BLOCKSIZE;
			}
			fwrite(&text[i], grabsize, 1, fp);
		}
		fclose(fp);
	}
}

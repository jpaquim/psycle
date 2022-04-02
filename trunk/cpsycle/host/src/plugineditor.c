/*
** This source is free software; you can redistribute itand /or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "plugineditor.h"

/* host */
#include "styles.h"
/* file */
#include <dir.h>
/* audio */
#include <luaplugin.h>
#include <exclusivelock.h>
#include <songio.h>
/* platform */
#include "../../detail/portable.h"

#define BLOCKSIZE 128 * 1024

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
	psy_ui_component_init(&self->component, parent, NULL);
	self->workspace = workspace;
	psy_ui_label_init_text(&self->desc, &self->component,
		"Plugin Name");
	psy_ui_component_set_align(&self->desc.component, psy_ui_ALIGN_LEFT);
	psy_ui_textarea_init_single_line(&self->name, &self->component);	
	psy_ui_component_set_align(&self->name.component, psy_ui_ALIGN_CLIENT);
	psy_ui_textarea_settext(&self->name, "newplugin");
	psy_ui_button_init_text(&self->create, &self->component, "Create");
	psy_ui_component_set_align(&self->create.component, psy_ui_ALIGN_RIGHT);	
}

/* PluginEditor */
/* prototypes */
static void plugineditor_inittitlebar(PluginEditor*);
static void plugineditor_onmachineschangeslot(PluginEditor*,
	psy_audio_Machines*, uintptr_t slot);
static void plugineditor_onsongchanged(PluginEditor*, Workspace* sender);
static void plugineditor_connectmachinesignals(PluginEditor*, Workspace*);
static void plugineditor_on_destroy(PluginEditor*, psy_ui_Component* sender);
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

/* implementation */
void plugineditor_init(PluginEditor* self, psy_ui_Component* parent,
	Workspace* workspace)
{	
	psy_ui_component_init(&self->component, parent, NULL);	
	psy_ui_component_set_style_type(&self->component,
		STYLE_RECENTVIEW_MAINSECTION);
	plugineditor_inittitlebar(self);
	self->workspace = workspace;
	self->basepath = 0;	
	self->instanceidx = psy_INDEX_INVALID;
	self->zoom = 1.0;
	psy_ui_component_init(&self->bar, &self->component, NULL);
	psy_ui_component_set_align(&self->bar, psy_ui_ALIGN_TOP);
	psy_ui_button_init(&self->reload, &self->bar);
	psy_ui_component_set_align(&self->reload.component, psy_ui_ALIGN_LEFT);
	psy_ui_button_init(&self->newplugin, &self->bar);
	psy_ui_button_set_text(&self->newplugin, "New Plugin");
	psy_ui_component_set_align(&self->newplugin.component, psy_ui_ALIGN_LEFT);
	psy_signal_connect(&self->newplugin.signal_clicked, self,
		&plugineditor_onnewplugin);
	psy_ui_button_set_text(&self->reload, "Reload");
	psy_signal_connect(&self->reload.signal_clicked, self,
		&plugineditor_onreload);
	psy_ui_button_init(&self->save, &self->bar);
	psy_ui_component_set_align(&self->save.component, psy_ui_ALIGN_LEFT);
	psy_ui_button_set_text(&self->save, "Save");
	psy_signal_connect(&self->save.signal_clicked, self,
		&plugineditor_onsave);	
	/* Plugin select */
	psy_ui_component_init(&self->row0, &self->component, NULL);
	psy_ui_component_set_align(&self->row0, psy_ui_ALIGN_TOP);
	psy_ui_label_init_text(&self->plugindesc, &self->row0,
		"Plugin");
	psy_ui_component_set_align(&self->plugindesc.component, psy_ui_ALIGN_LEFT);
	psy_ui_combobox_init(&self->pluginselector, &self->row0);
	//psy_ui_combobox_setcharnumber(&self->pluginselector, 32);
	psy_ui_component_set_align(&self->pluginselector.component, psy_ui_ALIGN_CLIENT);
	/* File Select */
	psy_ui_component_init(&self->row1, &self->component, NULL);
	psy_ui_component_set_align(&self->row1, psy_ui_ALIGN_TOP);
	psy_ui_label_init_text(&self->filedesc, &self->row1, "File");
	psy_ui_component_set_align(&self->filedesc.component, psy_ui_ALIGN_LEFT);
	psy_ui_combobox_init(&self->fileselector, &self->row1);
	//psy_ui_combobox_setcharnumber(&self->fileselector, 32);
	psy_ui_component_set_align(&self->fileselector.component, psy_ui_ALIGN_CLIENT);
	plugineditorcreatebar_init(&self->createbar, &self->component, workspace);
	psy_signal_connect(&self->createbar.create.signal_clicked, self,
		plugineditor_oncreatenewplugin);
	psy_ui_component_set_align(&self->createbar.component, psy_ui_ALIGN_TOP);
	psy_ui_component_hide(&self->createbar.component);
	/* editor */
	psy_ui_editor_init(&self->editor, &self->component);
	self->editor.textarea.pane.component.id = 100;
	psy_ui_component_set_align(&self->editor.component, psy_ui_ALIGN_CLIENT);	
	psy_signal_connect(&self->component.signal_destroy, self,
		plugineditor_on_destroy);
	psy_signal_connect(&workspace->signal_songchanged, self,
		plugineditor_onsongchanged);
	psy_signal_connect(&self->pluginselector.signal_selchanged, self,
		plugineditor_onpluginselected);
	psy_signal_connect(&self->fileselector.signal_selchanged, self,
		plugineditor_onfileselected);
	psy_ui_component_set_preferred_size(&self->component,
		psy_ui_size_make_em(80.0, 0.0));		
	psy_table_init(&self->pluginmappping);
	plugineditor_buildpluginlist(self);
	plugineditor_connectmachinesignals(self, workspace);
}

void plugineditor_on_destroy(PluginEditor* self, psy_ui_Component* sender)
{
	psy_table_dispose(&self->pluginmappping);
}

void plugineditor_inittitlebar(PluginEditor* self)
{
	titlebar_init(&self->titlebar, &self->component, "Plugin Editor");
	titlebar_hideonclose(&self->titlebar);
}

void plugineditor_onmachineschangeslot(PluginEditor* self,
	psy_audio_Machines* machines, uintptr_t slot)
{
	psy_audio_Machine* machine;

	machine = psy_audio_machines_at(machines, slot);
	if (machine && psy_audio_machine_info(machine) &&
			psy_audio_machine_info(machine)->type == psy_audio_LUA) {
		const char* path;
		char catchername[256];
		
		psy_snprintf(catchername, 256, "%s:0",
			psy_audio_machine_info(machine)->name);		
		path = psy_audio_plugincatcher_searchpath(&self->workspace->plugincatcher, catchername,
			psy_audio_LUA);
		if (path) {
			if (self->basepath == 0 || (strcmp(path, self->basepath) != 0)) {
				psy_ui_editor_load(&self->editor, path);
				self->basepath = path;
			}
			self->instanceidx = slot;
		}		
	}
}

void plugineditor_onsongchanged(PluginEditor* self, Workspace* sender)
{
	plugineditor_connectmachinesignals(self, sender);
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
	if (workspace_song(self->workspace) && self->instanceidx) {
		psy_audio_Machine* machine;

		machine = psy_audio_machines_at(&workspace_song(self->workspace)->machines, self->instanceidx);
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

	psy_ui_combobox_text(&self->fileselector, path);
	if (path[0] != '\0') {
		psy_ui_editor_save(&self->editor, path);
	}
}

void plugineditor_buildpluginlist(PluginEditor* self)
{	
	psy_Property* all;

	all = psy_audio_pluginsections_section_plugins(
		&self->workspace->plugincatcher.sections, "all");
	psy_table_clear(&self->pluginmappping);
	psy_ui_combobox_clear(&self->pluginselector);
	if (all) {
		psy_List* p;
		int c;

		for (p = psy_property_begin(all), c = 0; p != NULL; p = p->next) {
			psy_Property* property;

			property = (psy_Property*)psy_list_entry(p);
			if (psy_property_at_int(property, "type", -1) == psy_audio_LUA) {
				psy_ui_combobox_addtext(&self->pluginselector, psy_property_key(property));
				psy_table_insert(&self->pluginmappping, (uintptr_t)c, property);
				++c;
			}
		}
	}	
}

void plugineditor_onpluginselected(PluginEditor* self, psy_ui_Component* sender, int slot)
{
	psy_Property* p;

	p = (psy_Property*)psy_table_at(&self->pluginmappping, slot);
	if (p) {
		const char* path;

		path = psy_property_at_str(p, "path", 0);
		if (path) {
			if (self->basepath == 0 || (strcmp(path, self->basepath) != 0)) {
				psy_ui_editor_clear(&self->editor);
				psy_ui_editor_load(&self->editor, path);
				self->basepath = path;
				self->instanceidx = psy_INDEX_INVALID;
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

	psy_ui_combobox_text(&self->fileselector, path);
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
			psy_audio_PLUGIN, (psy_fp_findfile)plugineditor_onenumdir);		
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
	
	dir = dirconfig_luascripts(&self->workspace->config.directories);
	psy_path_init(&path, "");
	psy_path_setprefix(&path, dir);
	psy_path_setname(&path, "newplugin.lua");
	writetext(psy_path_full(&path), pluginsource);
	psy_path_dispose(&path);
	psy_path_init(&plugindir, "");
	psy_path_setprefix(&plugindir, dir);
	psy_path_setname(&plugindir, "newplugin");
	if (!psy_direxists(psy_path_full(&plugindir))) {
		psy_mkdir(psy_path_full(&plugindir));
	}
	psy_path_init(&path, "");
	psy_path_setprefix(&path, psy_path_full(&plugindir));
	psy_path_dispose(&plugindir);	
	psy_path_setname(&path, "machine.lua");
	writetext(psy_path_full(&path), machinesource);
	psy_path_dispose(&path);
}

void writetext(const char* path, const char* text)
{
	FILE* fp;

	fp = fopen(path, "wb");
	if (fp) {		
		uintptr_t lengthdoc;
		uintptr_t i;

		lengthdoc = psy_strlen(text);
		for (i = 0; i < lengthdoc; i += BLOCKSIZE) {
			uintptr_t grabsize;

			grabsize = lengthdoc - i;
			if (grabsize > BLOCKSIZE) {
				grabsize = BLOCKSIZE;
			}
			fwrite(&text[i], grabsize, 1, fp);
		}
		fclose(fp);
	}
}

void plugineditor_on_focus(PluginEditor* self, psy_ui_Component* sender)
{
	
}

void plugineditor_updatefont(PluginEditor* self)
{	
	psy_ui_FontInfo fontinfo;
	psy_ui_Font font;
	double zoomrate;

	assert(self);

	zoomrate = psy_ui_app_zoomrate(psy_ui_app()) * self->zoom;	
	psy_ui_fontinfo_init(&fontinfo, "Consolas", -16);
	fontinfo.lfHeight = (int32_t)((double)fontinfo.lfHeight * zoomrate);
	psy_ui_font_init(&font, &fontinfo);
	psy_ui_component_setfont(&self->editor.textarea.pane.component, &font);
	psy_ui_font_dispose(&font);	
	if (psy_ui_component_visible(&self->component)) {
		psy_ui_component_align_full(&self->component);
		psy_ui_component_invalidate(&self->component);
	}
}

/*
** This source is free software; you can redistribute itand /or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "presetsview.h"
/* audio */
#include <presetio.h>
/* platform */
#include "../../detail/portable.h"


/* prototypes */
static void presetsview_on_destroyed(PresetsView*);
static void presetsview_init_buttons(PresetsView*);
static void presetsview_connect_fileview(PresetsView*);
static void presetsview_on_import_button(PresetsView*,
	psy_ui_Component* sender);	
static void presetsview_on_import(PresetsView*, psy_Property* sender);
static void presetsview_import(PresetsView*, const char* path);
static void presetsview_on_export_button(PresetsView*,
	psy_ui_Component* sender);
static void presetsview_on_export(PresetsView*, psy_Property* sender);
static void presetsview_setpresets(PresetsView*, psy_audio_Presets*,
	bool setfirstprog);
static void presetsview_buildprograms(PresetsView*, psy_audio_Machine*);
static void presetsview_on_use(PresetsView*, psy_ui_Component* sender);

/* vtable */
static psy_ui_ComponentVtable vtable;
static bool vtable_initialized = FALSE;

static void vtable_init(PresetsView* self)
{
	if (!vtable_initialized) {
		vtable = *(self->component.vtable);
		vtable.on_destroyed =
			(psy_ui_fp_component)
			presetsview_on_destroyed;		
		vtable_initialized = TRUE;
	}
	psy_ui_component_set_vtable(&self->component, &vtable);
}

/* implementation */
void presetsview_init(PresetsView* self, psy_ui_Component* parent,
	Workspace* workspace)
{	
	assert(self);
	
	psy_ui_component_init(&self->component, parent, NULL);
	vtable_init(self);
	self->workspace = workspace;
	self->mac_id = psy_INDEX_INVALID;
	psy_ui_component_set_preferred_width(&self->component,
		psy_ui_value_make_ew(50.0));
	titlebar_init(&self->title_bar, &self->component, "Preset");
	presetsview_init_buttons(self);
	psy_ui_component_init_align(&self->client, &self->component, NULL,
		psy_ui_ALIGN_CLIENT);
	psy_ui_component_set_default_align(&self->client, psy_ui_ALIGN_TOP,		
		psy_ui_margin_zero());	
	psy_ui_combobox_init_simple(&self->programbox, &self->client);
	psy_ui_component_set_align(&self->programbox.component, psy_ui_ALIGN_CLIENT);	
	presetsview_connect_fileview(self);
}

void presetsview_on_destroyed(PresetsView* self)
{	
	assert(self);
	
	psy_property_dispose(&self->presets_load);
	psy_property_dispose(&self->presets_save);
}

void presetsview_init_buttons(PresetsView* self)
{
	assert(self);
	
	psy_ui_component_init_align(&self->buttons, &self->component, NULL,
		psy_ui_ALIGN_RIGHT);
	psy_ui_component_set_default_align(&self->buttons, psy_ui_ALIGN_TOP,		
		psy_ui_margin_zero());
	psy_ui_button_init_text(&self->save, &self->buttons,
		"machineframe.saveas");
	psy_ui_button_init_text(&self->del, &self->buttons,
		"machineframe.delete");
	psy_ui_button_init_text_connect(&self->import, &self->buttons,
		"machineframe.import", self, presetsview_on_import_button);
	psy_ui_button_init_text(&self->export_preset, &self->buttons,
		"machineframe.export");	
	psy_ui_checkbox_init(&self->preview, &self->buttons);
	psy_ui_checkbox_set_text(&self->preview, "machineframe.preview");
	psy_ui_component_init_align(&self->bottom, &self->buttons, NULL,
		psy_ui_ALIGN_BOTTOM);
	psy_ui_component_set_default_align(&self->bottom, psy_ui_ALIGN_TOP,		
		psy_ui_margin_zero());
	psy_ui_button_init_text_connect(&self->use, &self->bottom,
		"machineframe.use", self, presetsview_on_use);
	psy_ui_button_init_text(&self->close, &self->bottom,
		"machineframe.close");
}

void presetsview_connect_fileview(PresetsView* self)
{
	assert(self);
	
	psy_property_init_type(&self->presets_load, "load",
		PSY_PROPERTY_TYPE_STRING);
	psy_property_connect(&self->presets_load, self, presetsview_on_import);
	psy_property_init_type(&self->presets_save, "save",
		PSY_PROPERTY_TYPE_STRING);
	psy_property_connect(&self->presets_save, self, presetsview_on_export);
}

void presetsview_on_import_button(PresetsView* self, psy_ui_Component* sender)
{
	assert(self);
	
	if (keyboardmiscconfig_ft2_file_view(psycleconfig_misc(
			workspace_conf(self->workspace)))) {		
		psy_Property types;
		
		fileview_set_callbacks(self->workspace->fileview,
			&self->presets_load, NULL);
		psy_property_init_type(&types, "types", PSY_PROPERTY_TYPE_CHOICE);		
		psy_property_set_text(psy_property_set_id(
			psy_property_append_str(&types, "prs", "*.prs"),
			FILEVIEWFILTER_PSY), "Presets");		
		fileview_set_filter(self->workspace->fileview, &types);
		psy_property_dispose(&types);
		workspace_select_view(self->workspace,
			viewindex_make(VIEW_ID_FILEVIEW));
	}	
}

void presetsview_on_import(PresetsView* self, psy_Property* sender)
{		
	assert(self);
	
	presetsview_import(self, psy_property_item_str(sender));
	workspace_select_view(self->workspace, viewindex_make(VIEW_ID_MACHINEVIEW));	
}

void presetsview_import(PresetsView* self, const char* path)
{		
	int status;
	psy_audio_Presets* presets;
	psy_audio_Machine* machine;
		
	assert(self);
	
	if (!self->workspace->song) {
		return;
	}
	machine = psy_audio_machines_at(&self->workspace->song->machines,
		self->mac_id);
	if (!machine) {
		return;
	}
	presets = psy_audio_presets_allocinit();
	status = psy_audio_presetsio_load(path,
		presets,
		psy_audio_machine_numtweakparameters(machine),
		psy_audio_machine_datasize(machine),
		dirconfig_plugins_curr_platform(&self->workspace->config.directories));
	if (status) {
		workspace_output_error(self->workspace,
			psy_audio_presetsio_statusstr(status));
		psy_audio_presets_dispose(presets);
		free(presets);
		presets = NULL;
	}	
	psy_audio_machine_setpresets(machine, presets);
	presetsview_buildprograms(self, machine);
}

void presetsview_on_export_button(PresetsView* self, psy_ui_Component* sender)
{
	assert(self);
	
}	

void presetsview_on_export(PresetsView* self, psy_Property* sender)
{
	assert(self);
}

void presetsview_setpresets(PresetsView* self, psy_audio_Presets* presets,
	bool setfirstprog)
{
	assert(self);
	
	
	/*if (setfirstprog) {
		presetsbar_setprogram(self, 0);
	} else {
		presetsbar_buildprograms(self);
		presetsbar_buildbanks(self);
	}*/
}

void presetsview_buildprograms(PresetsView* self, psy_audio_Machine* machine)
{			
	uintptr_t numprograms;
	uintptr_t i;
	
	assert(self);

	psy_ui_combobox_clear(&self->programbox);
	if (!machine) {
		return;
	}
	numprograms = psy_audio_machine_numprograms(machine);
	for (i = 0; i < numprograms; ++i) {
		char name[256];

		psy_audio_machine_programname(machine, 0, i, name);
		psy_ui_combobox_add_text(&self->programbox, name);
	}	
}

void presetsview_set_mac_id(PresetsView* self, uintptr_t mac_id)
{
	assert(self);
	
	self->mac_id = mac_id;
}

void presetsview_on_use(PresetsView* self, psy_ui_Component* sender)
{
	psy_audio_Machine* machine;
	uintptr_t prg;
		
	assert(self);
	
	if (!self->workspace->song) {
		return;
	}
	machine = psy_audio_machines_at(&self->workspace->song->machines,
		self->mac_id);
	if (!machine) {
		return;
	}
	prg =  psy_ui_combobox_cursel(&self->programbox);	
	psy_audio_machine_setcurrprogram(machine, prg);
}

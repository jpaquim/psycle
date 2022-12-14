/*
** This source is free software; you can redistribute itand /or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "presetsbar.h"

/* file */
#include <dir.h>
#include <presetio.h>
/* ui */
#include <uiframe.h>
#include <uiopendialog.h>
#include <uisavedialog.h>
/* std */
#include <stdlib.h>
#include <string.h>
/* platform */
#include "../../detail/portable.h"


/* prototypes */
static void presetsbar_on_destroyed(PresetsBar*);
static bool presetsbar_userpresetpath(PresetsBar*, psy_Path*);
static void presetsbar_setpresets(PresetsBar*, psy_audio_Presets*,
	bool setfirstprog);
static void presetsbar_setprogram(PresetsBar*, uintptr_t prog);
static void presetsbar_buildbanks(PresetsBar*);
static void presetsbar_buildprograms(PresetsBar*);
static void presetsbar_onprogramselected(PresetsBar*, psy_ui_Component* sender,
	int slot);
static void presetsbar_onbankselected(PresetsBar*, psy_ui_Component* sender,
	int slot);
static void presetsbar_onimport(PresetsBar*, psy_ui_Component* sender);
static void presetsbar_onexport(PresetsBar*, psy_ui_Component* sender);
static void presetsbar_onsavepresets(PresetsBar*, psy_ui_Component* sender);
static void presetsbar_onsavenameeditkeydown(PresetsBar*,
	psy_ui_Component* sender, psy_ui_KeyboardEvent*);
static void presetsbar_updatesavename(PresetsBar*);

/* vtable */
static psy_ui_ComponentVtable vtable;
static bool vtable_initialized = FALSE;

static void vtable_init(PresetsBar* self)
{
	if (!vtable_initialized) {
		vtable = *(self->component.vtable);
		vtable.on_destroyed =
			(psy_ui_fp_component)
			presetsbar_on_destroyed;
		vtable_initialized = TRUE;
	}
	psy_ui_component_set_vtable(presetsbar_base(self), &vtable);
}

/* implementation */
void presetsbar_init(PresetsBar* self, psy_ui_Component* parent,
	Workspace* workspace)
{
	psy_ui_Margin margin;

	psy_ui_component_init(&self->component, parent, NULL);
	vtable_init(self);
	psy_ui_component_set_align_expand(&self->component,
		psy_ui_HEXPAND);
	self->workspace = workspace;
	self->machine = NULL;	
	self->userpreset = FALSE;
	psy_path_init(&self->presetpath, "");	
	psy_ui_label_init_text(&self->bank, &self->component, "machineframe.bank");
	psy_ui_combobox_init(&self->bankselector, &self->component);
	psy_ui_combobox_set_char_number(&self->bankselector, 10);
	psy_ui_button_init_text(&self->importpresets, &self->component,
		"machineframe.import");
	psy_ui_button_init_text(&self->exportpresets, &self->component,
		"machineframe.export");
	psy_ui_label_init_text(&self->program, &self->component,
		"machineframe.program");
	psy_ui_combobox_init(&self->programbox, &self->component);
	psy_ui_combobox_set_char_number(&self->programbox, 20);
	psy_ui_button_init_text(&self->savepresets, &self->component,
		"machineframe.saveas");
	psy_ui_textarea_init_single_line(&self->savename, &self->component);	
	psy_ui_textarea_set_char_number(&self->savename, 12);
	psy_ui_margin_init_em(&margin, 0.0, 1.0, 0.0, 0.0);
	psy_list_free(psy_ui_components_setalign(
		psy_ui_component_children(&self->component, psy_ui_NONE_RECURSIVE),
		psy_ui_ALIGN_LEFT, margin));
	psy_signal_connect(&self->bankselector.signal_selchanged, self,
		presetsbar_onbankselected);
	psy_signal_connect(&self->programbox.signal_selchanged, self,
		presetsbar_onprogramselected);	
	psy_signal_connect(&self->importpresets.signal_clicked, self,
		presetsbar_onimport);
	psy_signal_connect(&self->exportpresets.signal_clicked, self,
		presetsbar_onexport);
	psy_signal_connect(&self->savepresets.signal_clicked, self,
		presetsbar_onsavepresets);
	psy_signal_connect(&self->savename.component.signal_keydown, self,
		presetsbar_onsavenameeditkeydown);
}

void presetsbar_on_destroyed(PresetsBar* self)
{
	psy_path_dispose(&self->presetpath);
}

void presetsbar_setmachine(PresetsBar* self, psy_audio_Machine* machine)
{
	const psy_audio_MachineInfo* info;

	self->machine = machine;
	info = psy_audio_machine_info(machine);
	if (info && info->modulepath) {
		int status;

		psy_path_setpath(&self->presetpath, info->modulepath);
		if (!presetsbar_userpresetpath(self, &self->presetpath)) {
			psy_path_setpath(&self->presetpath, info->modulepath);
			psy_path_setext(&self->presetpath, "prs");
			self->userpreset = FALSE;
		} else {
			self->userpreset = TRUE;
		}		
		if (psy_audio_machine_acceptpresets(self->machine)) {
			psy_audio_Presets* presets;

			if (psy_audio_machine_presets(self->machine)) {
				presets = psy_audio_machine_presets(self->machine);
			} else {
				presets = psy_audio_presets_allocinit();
				status = psy_audio_presetsio_load(psy_path_full(&self->presetpath),
					presets,
					psy_audio_machine_numtweakparameters(self->machine),
					psy_audio_machine_datasize(self->machine),
					dirconfig_plugins_curr_platform(
						&self->workspace->config.directories));
				if (status != psy_audio_PRESETIO_OK) {
					psy_audio_machine_setpresets(self->machine, NULL);
					psy_audio_presets_dispose(presets);
					presets = NULL;
				}
				if (status && status != psy_audio_PRESETIO_ERROR_OPEN) {
					workspace_output_error(self->workspace,
						psy_audio_presetsio_statusstr(status));
				}
			}
			presetsbar_setpresets(self, presets, FALSE);
		} else {
			psy_ui_component_hide(&self->importpresets.component);
			psy_ui_component_hide(&self->exportpresets.component);
			psy_ui_component_hide(&self->savepresets.component);
			psy_ui_component_hide(&self->savename.component);
			presetsbar_showprogram(self);
		}
	}		
}

bool presetsbar_userpresetpath(PresetsBar* self, psy_Path* path)
{
	const psy_audio_MachineInfo* info;

	info = psy_audio_machine_info(self->machine);
	if (info && info->modulepath) {
		char* name;

		psy_path_setpath(path, info->modulepath);
		psy_path_set_prefix(path,
			dirconfig_user_presets(&self->workspace->config.directories));
		psy_path_setext(path, "prs");
		if (psy_filereadable(psy_path_full(path))) {
			return TRUE;
		}
		name = strdup(psy_path_name(path));
		psy_replacechar(name, '-', '_');
		psy_path_set_name(path, name);
		free(name);
		if (psy_filereadable(psy_path_full(path))) {
			return TRUE;
		}
	}
	return FALSE;
}

void presetsbar_onimport(PresetsBar* self, psy_ui_Component* sender)
{
	psy_ui_OpenDialog dialog;

	psy_ui_opendialog_init_all(&dialog, NULL, "Import Presets",
		"Psycle Presets(*.prs)|*.prs", "PRS",
		dirconfig_user_presets(&self->workspace->config.directories));
	if (psy_ui_opendialog_execute(&dialog)) {
		int status;
		psy_audio_Presets* presets;
		
		presets = psy_audio_presets_allocinit();
		status = psy_audio_presetsio_load(
			psy_path_full(psy_ui_opendialog_path(&dialog)),
			presets,
			psy_audio_machine_numtweakparameters(self->machine),
			psy_audio_machine_datasize(self->machine),
			dirconfig_plugins_curr_platform(&self->workspace->config.directories));
		if (status) {
			workspace_output_error(self->workspace,
				psy_audio_presetsio_statusstr(status));
			psy_audio_presets_dispose(presets);
			free(presets);
			presets = NULL;
		}
		presetsbar_setpresets(self, presets, TRUE);
	}
	psy_ui_opendialog_dispose(&dialog);
}

void presetsbar_onexport(PresetsBar* self, psy_ui_Component* sender)
{
	if (self->machine && psy_audio_machine_presets(self->machine)) {
		psy_ui_SaveDialog dialog;

		psy_ui_savedialog_init_all(&dialog, NULL, "Export Presets",
			"Psycle Presets (*.prs)|*.prs", "PRS",
			dirconfig_songs(&self->workspace->config.directories));
		if (psy_ui_savedialog_execute(&dialog)) {
			int status;

			status = psy_audio_presetsio_save(
				psy_path_full(psy_ui_savedialog_path(&dialog)),
				psy_audio_machine_presets(self->machine));
			if (status) {
				workspace_output_error(self->workspace,
					psy_audio_presetsio_statusstr(status));
			}
		}
		psy_ui_savedialog_dispose(&dialog);
	}
}

void presetsbar_onsavepresets(PresetsBar* self, psy_ui_Component* sender)
{
	if (self->machine) {
		psy_audio_Presets* presets;
		psy_audio_Preset* preset;
		uintptr_t index;
		int status;

		if ((psy_audio_machine_type(self->machine) == psy_audio_VST ||
				psy_audio_machine_type(self->machine) == psy_audio_VSTFX)) {
			psy_ui_SaveDialog dialog;

			psy_ui_savedialog_init_all(&dialog, NULL, "Export Preset",
				"Vst Preset (*.fxp)|*.fxp", "FXP",
				dirconfig_songs(&self->workspace->config.directories));
			if (psy_ui_savedialog_execute(&dialog)) {
				int status;				
				
				preset = psy_audio_preset_allocinit();
				psy_audio_machine_currentpreset(self->machine, preset);
				psy_audio_preset_setname(preset,
					psy_path_name(psy_ui_savedialog_path(&dialog)));				
				status = psy_audio_presetio_savefxp(
					psy_path_full(psy_ui_savedialog_path(&dialog)),
					preset);
				if (status) {
					workspace_output_error(self->workspace,
						psy_audio_presetsio_statusstr(status));
				}
				psy_audio_preset_dispose(preset);
			}
			psy_ui_savedialog_dispose(&dialog);
		} else if (psy_audio_machine_presets(self->machine)) {
			presets = psy_audio_machine_presets(self->machine);
			preset = psy_audio_preset_allocinit();
			psy_audio_machine_currentpreset(self->machine, preset);
			index = psy_ui_combobox_cursel(&self->programbox);
			psy_audio_preset_setname(preset,
				psy_ui_textarea_text(&self->savename));
			psy_audio_presets_insert(presets, index, preset);
			if (!self->userpreset) {
				psy_path_set_prefix(&self->presetpath,
					dirconfig_user_presets(&self->workspace->config.directories));
				if (!psy_direxists(psy_path_prefix(&self->presetpath))) {
					psy_mkdir(psy_path_prefix(&self->presetpath));
				}
				self->userpreset = TRUE;
			}
			status = psy_audio_presetsio_save(
				psy_path_full(&self->presetpath), presets);
			if (status) {
				workspace_output_error(self->workspace,
					psy_audio_presetsio_statusstr(status));
			}
			psy_ui_component_set_focus(&self->component);
		}
	}
}

void presetsbar_onsavenameeditkeydown(PresetsBar* self,
	psy_ui_Component* sender, psy_ui_KeyboardEvent* ev)
{
	if (psy_ui_keyboardevent_keycode(ev) == psy_ui_KEY_RETURN) {
		intptr_t index;

		presetsbar_onsavepresets(self, &self->component);
		index = psy_ui_combobox_cursel(&self->programbox);
		presetsbar_buildprograms(self);
		psy_ui_combobox_select(&self->programbox, index);		
		psy_ui_component_set_focus(&self->component);		
		psy_ui_keyboardevent_prevent_default(ev);
	} else if (psy_ui_keyboardevent_keycode(ev) == psy_ui_KEY_ESCAPE) {
		presetsbar_updatesavename(self);
		psy_ui_component_set_focus(&self->component);
		psy_ui_keyboardevent_prevent_default(ev);
	}
}

void presetsbar_onbankselected(PresetsBar* self, psy_ui_Component* sender, int slot)
{
	if (self->machine) {
		psy_audio_machine_setcurrbank(self->machine, slot);
	}
}

void presetsbar_onprogramselected(PresetsBar* self, psy_ui_Component* sender, int slot)
{
	presetsbar_setprogram(self, slot);
}

void presetsbar_setpresets(PresetsBar* self, psy_audio_Presets* presets, bool setfirstprog)
{
	if (self->machine && psy_audio_machine_presets(self->machine) != presets) {
		psy_audio_machine_setpresets(self->machine, presets);
	}
	if (setfirstprog) {
		presetsbar_setprogram(self, 0);
	} else {
		presetsbar_buildprograms(self);
		presetsbar_buildbanks(self);
	}
}

void presetsbar_setprogram(PresetsBar* self, uintptr_t prog)
{	
	psy_ui_combobox_clear(&self->programbox);
	if (self->machine) {		
		psy_audio_machine_setcurrprogram(self->machine, prog);
		presetsbar_buildprograms(self);
		presetsbar_buildbanks(self);		
		psy_ui_combobox_select(&self->bankselector,
			psy_audio_machine_currbank(self->machine));
		psy_ui_combobox_select(&self->programbox,
			psy_audio_machine_currprogram(self->machine));
		presetsbar_updatesavename(self);
	}
}

void presetsbar_showprogram(PresetsBar* self)
{
	psy_ui_combobox_clear(&self->programbox);
	if (self->machine) {		
		presetsbar_buildprograms(self);
		presetsbar_buildbanks(self);
		psy_ui_combobox_select(&self->bankselector,
			psy_audio_machine_currbank(self->machine));
		psy_ui_combobox_select(&self->programbox,
			psy_audio_machine_currprogram(self->machine));
		presetsbar_updatesavename(self);
	}
}

void presetsbar_buildprograms(PresetsBar* self)
{
	if (self->machine) {
		uintptr_t numprograms;
		uintptr_t i;

		psy_ui_combobox_clear(&self->programbox);
		numprograms = psy_audio_machine_numprograms(self->machine);
		for (i = 0; i < numprograms; ++i) {
			char name[256];

			psy_audio_machine_programname(self->machine, 0, i, name);
			psy_ui_combobox_add_text(&self->programbox, name);
		}		
	}
}

void presetsbar_buildbanks(PresetsBar* self)
{
	if (self->machine) {
		uintptr_t numbanks;
		uintptr_t i;

		psy_ui_combobox_clear(&self->bankselector);
		numbanks = psy_audio_machine_numbanks(self->machine);
		for (i = 0; i < numbanks; ++i) {
			char name[256];

			psy_audio_machine_bankname(self->machine, i, name);
			psy_ui_combobox_add_text(&self->bankselector, name);
		}		
	}
}

void presetsbar_updatesavename(PresetsBar* self)
{
	if (self->machine) {
		char text[256];

		psy_audio_machine_programname(self->machine,
			psy_audio_machine_currbank(self->machine),
			psy_audio_machine_currprogram(self->machine),
			text);
		psy_ui_textarea_set_text(&self->savename, text);
	}
}

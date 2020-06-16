// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "presetsbar.h"
#include "resources/resource.h"
#include <dir.h>
#include <presetio.h>

#include <uiframe.h>
#include <uiopendialog.h>
#include <uisavedialog.h>

#include <stdlib.h>
#include <string.h>

#include "../../detail/portable.h"

static bool presetsbar_userpresetpath(PresetsBar*, psy_Path*);
static void presetsbar_ondestroy(PresetsBar*, psy_ui_Component* sender);
static void presetsbar_setprogram(PresetsBar*, psy_audio_Presets*, uintptr_t prog);
static void presetsbar_buildbanks(PresetsBar*);
static void presetsbar_buildprograms(PresetsBar*);
static void presetsbar_onprogramselected(PresetsBar*, psy_ui_Component* sender, int slot);
static void presetsbar_onbankselected(PresetsBar*, psy_ui_Component* sender, int slot);
static void presetsbar_onimport(PresetsBar*, psy_ui_Component* sender);
static void presetsbar_onexport(PresetsBar*, psy_ui_Component* sender);
static void presetsbar_onsavepresets(PresetsBar*, psy_ui_Component* sender);
static void presetsbar_onsavenameeditkeydown(PresetsBar*,
	psy_ui_Component* sender, psy_ui_KeyEvent*);

void presetsbar_init(PresetsBar* self, psy_ui_Component* parent,
	Workspace* workspace)
{
	psy_ui_Margin margin;

	psy_ui_component_init(&self->component, parent);
	self->workspace = workspace;
	self->machine = NULL;
	self->presets = NULL;
	self->presets = 0;
	self->userpreset = FALSE;
	psy_signal_init(&self->signal_presetchanged);
	psy_path_init(&self->presetpath, "");
	psy_ui_component_enablealign(&self->component);
	psy_ui_label_init(&self->bank, &self->component);
	psy_ui_label_settext(&self->bank, "Bank");
	psy_ui_combobox_init(&self->bankselector, &self->component);
	psy_ui_combobox_setcharnumber(&self->bankselector, 10);
	psy_ui_button_init(&self->importpresets, &self->component);
	psy_ui_button_settext(&self->importpresets, "Import");
	psy_ui_button_init(&self->exportpresets, &self->component);
	psy_ui_button_settext(&self->exportpresets, "Export");
	psy_ui_label_init(&self->program, &self->component);
	psy_ui_label_settext(&self->program, "Program");
	psy_ui_combobox_init(&self->programbox, &self->component);
	psy_ui_combobox_setcharnumber(&self->programbox, 20);
	psy_ui_button_init(&self->savepresets, &self->component);
	psy_ui_button_settext(&self->savepresets, "Save as");
	psy_ui_edit_init(&self->savename, &self->component);
	psy_ui_edit_setcharnumber(&self->savename, 12);
	psy_ui_margin_init_all(&margin, psy_ui_value_makepx(0),
		psy_ui_value_makeew(1), psy_ui_value_makepx(0),
		psy_ui_value_makepx(0));
	psy_list_free(psy_ui_components_setalign(
		psy_ui_component_children(&self->component, psy_ui_NONRECURSIVE),
		psy_ui_ALIGN_LEFT, &margin));
	psy_signal_connect(&self->bankselector.signal_selchanged, self,
		presetsbar_onbankselected);
	psy_signal_connect(&self->programbox.signal_selchanged, self,
		presetsbar_onprogramselected);
	psy_signal_connect(&self->component.signal_destroy, self,
		presetsbar_ondestroy);
	psy_signal_connect(&self->importpresets.signal_clicked, self,
		presetsbar_onimport);
	psy_signal_connect(&self->exportpresets.signal_clicked, self,
		presetsbar_onexport);
	psy_signal_connect(&self->savepresets.signal_clicked, self,
		presetsbar_onsavepresets);
	psy_signal_connect(&self->savename.component.signal_keydown, self,
		presetsbar_onsavenameeditkeydown);
}

void presetsbar_ondestroy(PresetsBar* self, psy_ui_Component* sender)
{
	if (self->presets) {
		psy_audio_presets_dispose(self->presets);
		self->presets = NULL;
	}
	psy_path_dispose(&self->presetpath);
	psy_signal_dispose(&self->signal_presetchanged);
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
		if (self->presets) {
			psy_audio_presets_dispose(self->presets);
		}
		self->presets = psy_audio_presets_allocinit();
		status = psy_audio_presetsio_load(psy_path_path(&self->presetpath),
			self->presets,
			psy_audio_machine_numtweakparameters(self->machine),
			psy_audio_machine_datasize(self->machine),
			workspace_plugins_directory(self->workspace));
		if (status && status != psy_audio_PRESETIO_ERROR_OPEN) {
			workspace_outputerror(self->workspace,
				psy_audio_presetsio_statusstr(status));
		}
	}
	presetsbar_buildbanks(self);
	presetsbar_buildprograms(self);
	presetsbar_setprogram(self, self->presets, 0);
}

bool presetsbar_userpresetpath(PresetsBar* self, psy_Path* path)
{
	const psy_audio_MachineInfo* info;

	info = psy_audio_machine_info(self->machine);
	if (info && info->modulepath) {
		char* name;

		psy_path_setpath(path, info->modulepath);
		psy_path_setprefix(path,
			workspace_userpresets_directory(self->workspace));
		psy_path_setext(path, "prs");
		if (psy_filereadable(psy_path_path(path))) {
			return TRUE;
		}
		name = strdup(psy_path_name(path));
		psy_replacechar(name, '-', '_');
		psy_path_setname(path, name);
		free(name);
		if (psy_filereadable(psy_path_path(path))) {
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
		workspace_userpresets_directory(self->workspace));
	if (psy_ui_opendialog_execute(&dialog)) {
		int status;

		if (self->presets) {
			psy_audio_presets_dispose(self->presets);
			free(self->presets);
		}
		self->presets = psy_audio_presets_allocinit();
		status = psy_audio_presetsio_load(psy_ui_opendialog_filename(&dialog),
			self->presets,
			psy_audio_machine_numtweakparameters(self->machine),
			psy_audio_machine_datasize(self->machine),
			workspace_plugins_directory(self->workspace));
		if (status) {
			workspace_outputerror(self->workspace,
				psy_audio_presetsio_statusstr(status));
			psy_audio_presets_dispose(self->presets);
			free(self->presets);
			self->presets = NULL;
		}
		presetsbar_setprogram(self, self->presets, 0);
	}
	psy_ui_opendialog_dispose(&dialog);
}

void presetsbar_onexport(PresetsBar* self, psy_ui_Component* sender)
{
	psy_ui_SaveDialog dialog;

	psy_ui_savedialog_init_all(&dialog, NULL, "Export Presets",
		"Psycle Presets (*.prs)|*.prs", "PRS",
		workspace_songs_directory(self->workspace));
	if (psy_ui_savedialog_execute(&dialog)) {
		int status;

		status = psy_audio_presetsio_save(psy_ui_savedialog_filename(&dialog),
			self->presets);
		if (status) {
			workspace_outputerror(self->workspace,
				psy_audio_presetsio_statusstr(status));
		}
	}
	psy_ui_savedialog_dispose(&dialog);
}

void presetsbar_onsavepresets(PresetsBar* self, psy_ui_Component* sender)
{
	if (self->machine) {
		psy_audio_Preset* preset;

		preset = psy_audio_preset_allocinit();
		psy_audio_machine_currentpreset(self->machine, preset);
		if (self->presets) {
			uintptr_t index;
			int status;

			index = psy_ui_combobox_cursel(&self->programbox);
			psy_audio_preset_setname(preset,
				psy_ui_edit_text(&self->savename));
			psy_audio_presets_insert(self->presets, index, preset);
			if (!self->userpreset) {
				psy_path_setprefix(&self->presetpath,
					workspace_userpresets_directory(self->workspace));
				if (!psy_direxists(psy_path_prefix(&self->presetpath))) {
					psy_mkdir(psy_path_prefix(&self->presetpath));
				}
				self->userpreset = TRUE;
			}
			status = psy_audio_presetsio_save(psy_path_path(&self->presetpath),
				self->presets);
			if (status) {
				workspace_outputerror(self->workspace,
					psy_audio_presetsio_statusstr(status));
			}
			presetsbar_setprogram(self, self->presets, index);
		} else {
			psy_audio_preset_dispose(preset);
			free(preset);
		}
		psy_ui_component_setfocus(&self->component);
	}
}

void presetsbar_onsavenameeditkeydown(PresetsBar* self,
	psy_ui_Component* sender, psy_ui_KeyEvent* ev)
{
	if (ev->keycode == psy_ui_KEY_RETURN) {
		presetsbar_onsavepresets(self, &self->component);
		psy_ui_component_setfocus(&self->component);
		psy_ui_keyevent_preventdefault(ev);
	} else if (ev->keycode == psy_ui_KEY_ESCAPE) {
		int index;
		psy_audio_Preset* preset;

		index = psy_ui_combobox_cursel(&self->programbox);
		preset = psy_audio_presets_at(self->presets, (uintptr_t)index);
		if (preset) {
			psy_ui_edit_settext(&self->savename,
				psy_audio_preset_name(preset));
		} else {
			psy_ui_edit_settext(&self->savename, "");
		}
		psy_ui_component_setfocus(&self->component);
		psy_ui_keyevent_preventdefault(ev);
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
	if (self->machine) {
		psy_audio_Preset* preset;

		psy_audio_machine_setcurrprogram(self->machine, slot);
		psy_signal_emit(&self->signal_presetchanged, self, 1, slot);		
		preset = psy_table_at(&self->presets->container, (uintptr_t)slot);
		if (preset) {
			psy_ui_edit_settext(&self->savename,
				psy_audio_preset_name(preset));
		}
	}
}

void presetsbar_setprogram(PresetsBar* self, psy_audio_Presets* presets,
	uintptr_t prog)
{
	self->presets = presets;
	psy_ui_combobox_clear(&self->programbox);
	if (self->presets) {
		uintptr_t index;

		for (index = 0; index < 128; ++index) {
			psy_audio_Preset* preset;
			char text[40];

			preset = (psy_audio_Preset*)psy_audio_presets_at(presets, index);
			if (preset) {
				psy_snprintf(text, 20, "%02X*:%s", index,
					psy_audio_preset_name(preset));
			} else {
				psy_snprintf(text, 20, "%02X:%s", index, "");
			}
			psy_ui_combobox_addtext(&self->programbox, text);
			if (preset && index == prog) {
				psy_ui_edit_settext(&self->savename,
					psy_audio_preset_name(preset));
			}
		}
		psy_ui_combobox_setcursel(&self->programbox, prog);
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
			psy_ui_combobox_addtext(&self->programbox, name);
		}
		psy_ui_combobox_setcursel(&self->programbox,
			psy_audio_machine_currprogram(self->machine));
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
			psy_ui_combobox_addtext(&self->bankselector, name);
		}
		psy_ui_combobox_setcursel(&self->bankselector,
			psy_audio_machine_currbank(self->machine));
	}
}

// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "machineframe.h"
#include "resources/resource.h"
#include <dir.h>
#include <presetio.h>

#include <uiframe.h>
#include <uiopendialog.h>
#include <uisavedialog.h>

#include <stdlib.h>
#include <string.h>

#include "../../detail/portable.h"

static void machineframe_ondestroyed(MachineFrame* self, psy_ui_Component* frame);
static void machineframe_onpresetchange(MachineFrame*, psy_ui_Component* sender, int index);
static void machineframe_toggleparameterbox(MachineFrame*,
	psy_ui_Component* sender);
static void machineframe_togglehelp(MachineFrame*,
	psy_ui_Component* sender);
static void machineframe_toggledock(MachineFrame*,
	psy_ui_Component* sender);
static void machineframe_onfloatview(MachineFrame*,
	psy_ui_Component* sender);
static void machineframe_onclose(MachineFrame*,
	psy_ui_Component* sender);
static void machineframe_resize(MachineFrame*);
static void machineframe_preferredviewsizechanged(MachineFrame*, psy_ui_Component* sender);
static void machineframe_setfloatbar(MachineFrame*);
static void machineframe_setdockbar(MachineFrame*);

static void machineframe_buildbanks(MachineFrame*);
static void machineframe_buildprograms(MachineFrame*);
static void plugineditor_onprogramselected(MachineFrame*, psy_ui_Component* sender, int slot);
static void plugineditor_onbankselected(MachineFrame*, psy_ui_Component* sender, int slot);
static void machineframe_onimport(MachineFrame*, psy_ui_Component* sender);
static void machineframe_onexport(MachineFrame*, psy_ui_Component* sender);
static void machineframe_onsavepresets(MachineFrame*, psy_ui_Component* sender);
static void machineframe_onsavenameeditkeydown(MachineFrame*,
	psy_ui_Component* sender, psy_ui_KeyEvent*);

static void ondefaultfontchanged(MachineFrame*, Workspace* sender);
static bool machineframe_userpresetpath(MachineFrame*, psy_Path*);
static int filereadable(const char* fname);


static void parameterbar_ondestroy(ParameterBar*, psy_ui_Component* sender);
static void parameterbar_setprogram(ParameterBar*, psy_audio_Presets*, uintptr_t prog);

void parameterbar_init(ParameterBar* self, psy_ui_Component* parent)
{			
	psy_ui_Margin margin;

	self->presets = 0;
	self->userpreset = FALSE;
	psy_path_init(&self->presetpath, "");
	psy_ui_component_init(&self->component, parent);
	psy_ui_component_enablealign(&self->component);
	// titlerow
	psy_ui_component_init(&self->titlerow, &self->component);
	psy_ui_component_enablealign(&self->titlerow);
	psy_ui_label_init(&self->title, &self->titlerow);
	psy_ui_label_settext(&self->title, "");		
	psy_ui_component_setalign(&self->floatview.component, psy_ui_ALIGN_RIGHT);
	psy_ui_button_init(&self->close, &self->titlerow);
	psy_ui_button_settext(&self->close, "X");
	psy_ui_button_init(&self->floatview, &self->titlerow);
	psy_ui_button_settext(&self->floatview, "Float");
	psy_ui_margin_init_all(&margin, psy_ui_value_makepx(0),
		psy_ui_value_makeew(1), psy_ui_value_makepx(0),
		psy_ui_value_makepx(0));
	psy_ui_component_setalign(&self->close.component, psy_ui_ALIGN_RIGHT);
	psy_ui_component_setalign(&self->floatview.component, psy_ui_ALIGN_RIGHT);
	// row0
	psy_ui_component_init(&self->row0, &self->component);
	psy_ui_component_enablealign(&self->row0);
	psy_ui_button_init(&self->mute, &self->row0);	
	psy_ui_button_settext(&self->mute, "Mute");
	psy_ui_button_init(&self->parameters, &self->row0);
	psy_ui_button_settext(&self->parameters, "Parameters");	
	psy_ui_button_init(&self->help, &self->row0);	
	psy_ui_button_settext(&self->help, "Help");
	psy_ui_button_init(&self->dock, &self->row0);
	psy_ui_button_settext(&self->dock, "Dock");	
	psy_list_free(psy_ui_components_setalign(
		psy_ui_component_children(&self->row0, psy_ui_NONRECURSIVE),
		psy_ui_ALIGN_LEFT, &margin));
	// row1
	psy_ui_component_init(&self->row1, &self->component);
	psy_ui_component_enablealign(&self->row1);
	psy_ui_component_setalign(&self->row1, psy_ui_ALIGN_TOP);
	psy_ui_label_init(&self->bank, &self->row1);
	psy_ui_label_settext(&self->bank, "Bank");
	psy_ui_combobox_init(&self->bankselector, &self->row1);
	psy_ui_combobox_setcharnumber(&self->bankselector, 10);
	psy_ui_button_init(&self->importpresets, &self->row1);
	psy_ui_button_settext(&self->importpresets, "Import");
	psy_ui_button_init(&self->exportpresets, &self->row1);
	psy_ui_button_settext(&self->exportpresets, "Export");
	psy_ui_label_init(&self->program, &self->row1);
	psy_ui_label_settext(&self->program, "Program");
	psy_ui_combobox_init(&self->programbox, &self->row1);
	psy_ui_combobox_setcharnumber(&self->programbox, 20);
	psy_ui_button_init(&self->savepresets, &self->row1);
	psy_ui_button_settext(&self->savepresets, "Save as");
	psy_ui_edit_init(&self->savename, &self->row1);
	psy_ui_edit_setcharnumber(&self->savename, 12);
	psy_list_free(psy_ui_components_setalign(
		psy_ui_component_children(&self->row1, psy_ui_NONRECURSIVE),
		psy_ui_ALIGN_LEFT, &margin));
	psy_ui_margin_init_all(&margin, psy_ui_value_makeeh(0.25),
		psy_ui_value_makepx(0), psy_ui_value_makeeh(0.25),
		psy_ui_value_makepx(0));
	psy_ui_components_setalign(
		psy_ui_component_children(&self->component, psy_ui_NONRECURSIVE),
		psy_ui_ALIGN_TOP, NULL);
	psy_ui_component_setmargin(&self->row1, &margin);		
	psy_signal_connect(&self->component.signal_destroy, self,
		parameterbar_ondestroy);
}

void parameterbar_ondestroy(ParameterBar* self, psy_ui_Component* sender)
{
	psy_path_dispose(&self->presetpath);
}

void parameterbar_setprogram(ParameterBar* self, psy_audio_Presets* presets,
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
			if (index == prog) {
				psy_ui_edit_settext(&self->savename,
					psy_audio_preset_name(preset));
			}
		}
		psy_ui_combobox_setcursel(&self->programbox, prog);
	}
}

void machineframe_init(MachineFrame* self, psy_ui_Component* parent, bool floated, Workspace* workspace)
{	
	self->view = 0;
	self->presets = 0;
	self->machine = 0;
	self->machineview = parent;
	self->dofloat = 0;
	self->dodock = 0;
	self->doclose = 0;
	self->floated = floated;
	self->workspace = workspace;
	if (floated) {
		psy_ui_frame_init(&self->component, parent);
		psy_ui_component_move(&self->component,
			psy_ui_point_make(
				psy_ui_value_makepx(200),
				psy_ui_value_makepx(150)));
	} else {
		psy_ui_component_init(&self->component, parent);		
	}
	psy_ui_component_seticonressource(&self->component, IDI_MACPARAM);		
	psy_ui_component_enablealign(&self->component);	
	parameterbar_init(&self->parameterbar, &self->component);
	psy_ui_component_setalign(&self->parameterbar.component, psy_ui_ALIGN_TOP);
	parameterlistbox_init(&self->parameterbox, &self->component, NULL);
	psy_ui_component_setalign(&self->parameterbox.component, psy_ui_ALIGN_RIGHT);
	psy_ui_notebook_init(&self->notebook, &self->component);
	psy_ui_component_enablealign(&self->notebook.component);
	psy_ui_component_setalign(psy_ui_notebook_base(&self->notebook),
		psy_ui_ALIGN_CLIENT);
	psy_ui_editor_init(&self->help, psy_ui_notebook_base(&self->notebook));
	psy_ui_editor_addtext(&self->help, "About");
	psy_signal_connect(&self->component.signal_destroyed, self,
		machineframe_ondestroyed);
	psy_signal_connect(&self->parameterbar.programbox.signal_selchanged,
		self, machineframe_onpresetchange);
	psy_signal_connect(&self->parameterbar.parameters.signal_clicked, self,
		machineframe_toggleparameterbox);
		psy_signal_connect(&self->parameterbar.help.signal_clicked, self,
		machineframe_togglehelp);
	psy_signal_connect(&self->parameterbar.dock.signal_clicked, self,
		machineframe_toggledock);
	psy_signal_connect(&self->parameterbar.dock.signal_clicked, self,
		machineframe_toggledock);	
	psy_signal_connect(&self->parameterbar.floatview.signal_clicked, self,
		machineframe_onfloatview);
	psy_signal_connect(&self->parameterbar.close.signal_clicked, self,
		machineframe_onclose);
	if (floated) {
		machineframe_setfloatbar(self);
	} else {
		machineframe_setdockbar(self);
	}
	psy_signal_connect(&self->parameterbar.bankselector.signal_selchanged, self,
		plugineditor_onbankselected);
	psy_signal_connect(&self->parameterbar.programbox.signal_selchanged, self,
		plugineditor_onprogramselected);
	psy_signal_connect(&self->workspace->signal_defaultfontchanged, self,
		ondefaultfontchanged);
	psy_signal_connect(&self->parameterbar.importpresets.signal_clicked, self,
		machineframe_onimport);
	psy_signal_connect(&self->parameterbar.exportpresets.signal_clicked, self,
		machineframe_onexport);
	psy_signal_connect(&self->parameterbar.savepresets.signal_clicked, self,
		machineframe_onsavepresets);
	psy_signal_connect(&self->parameterbar.savename.component.signal_keydown, self,
		machineframe_onsavenameeditkeydown);
}

void machineframe_setview(MachineFrame* self, psy_ui_Component* view,
	psy_audio_Machine* machine)
{
	const psy_audio_MachineInfo* info;	
	char text[128];
	int status;

	self->view = (psy_ui_Component*) view;
	self->machine = machine;
	psy_ui_component_setalign(self->view, psy_ui_ALIGN_CLIENT);	
	parameterlistbox_setmachine(&self->parameterbox, machine);
	psy_ui_component_hide(&self->parameterbox.component);
	info = psy_audio_machine_info(machine);
	if (info && info->modulepath) {		
		psy_path_setpath(&self->parameterbar.presetpath, info->modulepath);
		if (!machineframe_userpresetpath(self, &self->parameterbar.presetpath)) {
			psy_path_setpath(&self->parameterbar.presetpath, info->modulepath);
			psy_path_setext(&self->parameterbar.presetpath, "prs");
			self->parameterbar.userpreset = FALSE;
		} else {
			self->parameterbar.userpreset = TRUE;
		}
		if (self->presets) {
			psy_audio_presets_dispose(self->presets);
		}
		self->presets = psy_audio_presets_allocinit();
		status = psy_audio_presetsio_load(psy_path_path(&self->parameterbar.presetpath),
			self->presets,
			psy_audio_machine_numtweakparameters(self->machine),
			psy_audio_machine_datasize(self->machine),
			workspace_plugins_directory(self->workspace));
		if (status && status != psy_audio_PRESETIO_ERROR_OPEN) {
			workspace_outputerror(self->workspace,
				psy_audio_presetsio_statusstr(status));
		}
	}		
	machineframe_resize(self);
	if (self->machine && psy_audio_machine_editname(self->machine)) {
		psy_snprintf(text, 128, "%.2X : %s",
			psy_audio_machine_slot(self->machine),
			psy_audio_machine_editname(self->machine));
	} else {
		psy_ui_component_settitle(&self->component, text);
			psy_snprintf(text, 128, "%.2X :",
				psy_audio_machine_slot(self->machine));
	}
	psy_ui_notebook_setpageindex(&self->notebook, 1);
	psy_ui_component_settitle(&self->component, text);
	psy_ui_component_align(&self->component);
	psy_signal_connect(&view->signal_preferredsizechanged, self,
		machineframe_preferredviewsizechanged);
	if (self->machine) {
		psy_ui_label_settext(&self->parameterbar.title, psy_audio_machine_editname(self->machine));
	}
	else {
		psy_ui_label_settext(&self->parameterbar.title, "");
	}
	machineframe_buildbanks(self);
	machineframe_buildprograms(self);
	parameterbar_setprogram(&self->parameterbar, self->presets, 0);
}

bool machineframe_userpresetpath(MachineFrame* self, psy_Path* path)
{
	const psy_audio_MachineInfo* info;

	info = psy_audio_machine_info(self->machine);
	if (info && info->modulepath) {
		char* name;

		psy_path_setpath(path, info->modulepath);
		psy_path_setprefix(path,
			workspace_userpresets_directory(self->workspace));
		psy_path_setext(path, "prs");
		if (filereadable(psy_path_path(path))) {
			return TRUE;
		}
		name = strdup(psy_path_name(path));
		psy_replacechar(name, '-', '_');
		psy_path_setname(path, name);
		free(name);
		if (filereadable(psy_path_path(path))) {
			return TRUE;
		}
	}
	return FALSE;
}

int filereadable(const char* fname)
{
	FILE* file;
	if ((file = fopen(fname, "r")))
	{
		fclose(file);
		return 1;
	}
	return 0;
}

void machineframe_buildprograms(MachineFrame* self)
{
	if (self->machine) {
		uintptr_t numprograms;
		uintptr_t i;

		psy_ui_combobox_clear(&self->parameterbar.programbox);
		numprograms = psy_audio_machine_numprograms(self->machine);
		for (i = 0; i < numprograms; ++i) {
			char name[256];

			psy_audio_machine_programname(self->machine, 0, i, name);
			psy_ui_combobox_addtext(&self->parameterbar.programbox, name);
		}
		psy_ui_combobox_setcursel(&self->parameterbar.programbox,
			psy_audio_machine_currprogram(self->machine));
	}
}

void machineframe_buildbanks(MachineFrame* self)
{
	if (self->machine) {
		uintptr_t numbanks;
		uintptr_t i;

		psy_ui_combobox_clear(&self->parameterbar.bankselector);
		numbanks = psy_audio_machine_numbanks(self->machine);
		for (i = 0; i < numbanks; ++i) {
			char name[256];

			psy_audio_machine_bankname(self->machine, i, name);
			psy_ui_combobox_addtext(&self->parameterbar.bankselector, name);
		}
		psy_ui_combobox_setcursel(&self->parameterbar.bankselector,
			psy_audio_machine_currbank(self->machine));
	}
}

void plugineditor_onbankselected(MachineFrame* self, psy_ui_Component* sender, int slot)
{
	if (self->machine) {
		psy_audio_machine_setcurrbank(self->machine, slot);
	}
}

void plugineditor_onprogramselected(MachineFrame* self, psy_ui_Component* sender, int slot)
{
	if (self->machine) {
		psy_audio_machine_setcurrprogram(self->machine, slot);
		machineframe_onpresetchange(self, sender, slot);
	}
}

void machineframe_ondestroyed(MachineFrame* self, psy_ui_Component* frame)
{	
	if (self->presets) {
		psy_audio_presets_dispose(self->presets);
		self->presets = 0;
		if (self->view) {
			free(self->view);
		}
	}
	psy_signal_disconnect(&self->workspace->signal_defaultfontchanged, self,
		ondefaultfontchanged);
}

void machineframe_onpresetchange(MachineFrame* self, psy_ui_Component* sender, int index)
{
	if (self->presets && self->machine) {		
		psy_audio_Preset* preset;

		preset = psy_table_at(&self->presets->container, (uintptr_t)index);
		if (preset) {			
			psy_TableIterator it;

			for (it = psy_table_begin(&preset->parameters); 
					!psy_tableiterator_equal(&it, psy_table_end());
					psy_tableiterator_inc(&it)) {
				psy_audio_MachineParam* param;

				param = psy_audio_machine_tweakparameter(self->machine,
					psy_tableiterator_key(&it));
				if (param) {
					psy_audio_machine_parameter_tweak_scaled(self->machine, param,
						(intptr_t)psy_tableiterator_value(&it));
				}				
			}
			psy_ui_edit_settext(&self->parameterbar.savename,
				psy_audio_preset_name(preset));
		}
	}
}

MachineFrame* machineframe_alloc(void)
{
	return (MachineFrame*) malloc(sizeof(MachineFrame));
}

MachineFrame* machineframe_allocinit(psy_ui_Component* parent, bool floated, Workspace* workspace)
{
	MachineFrame* rv;

	rv = machineframe_alloc();
	if (rv) {
		machineframe_init(rv, parent, floated, workspace);
	}
	return rv;	
}

void machineframe_toggleparameterbox(MachineFrame* self,
	psy_ui_Component* sender)
{
	psy_ui_Size viewsize;

	viewsize = psy_ui_component_preferredsize(self->view, 0);
	if (psy_ui_component_visible(&self->parameterbox.component)) {
		psy_ui_component_hide(&self->parameterbox.component);		
		psy_ui_button_disablehighlight(&self->parameterbar.parameters);
	} else {
		psy_ui_component_show(&self->parameterbox.component);		
		psy_ui_button_highlight(&self->parameterbar.parameters);
	}
	machineframe_resize(self);
}

void machineframe_togglehelp(MachineFrame* self,
	psy_ui_Component* sender)
{	
	if (psy_ui_notebook_pageindex(&self->notebook) == 0) {			
		psy_ui_notebook_setpageindex(&self->notebook, 1);
		psy_ui_button_disablehighlight(&self->parameterbar.help);
	} else {		
		psy_ui_notebook_setpageindex(&self->notebook, 0);
		psy_ui_button_highlight(&self->parameterbar.help);
	}	
}

void machineframe_toggledock(MachineFrame* self, psy_ui_Component* sender)
{
	if (!self->floated) {
		self->dofloat = 1;
	} else {
		self->dodock = 1;
	}
}

void machineframe_onfloatview(MachineFrame* self, psy_ui_Component* sender)
{
	if (!self->floated) {
		self->dofloat = 1;
	}	
}

void machineframe_onclose(MachineFrame* self, psy_ui_Component* sender)
{
	self->doclose = 1;
}

void machineframe_resize(MachineFrame* self)
{
	psy_ui_Size viewsize;	
	psy_ui_Size bar;
	psy_ui_TextMetric tm;

	tm = psy_ui_component_textmetric(&self->component);	
	viewsize = psy_ui_component_preferredsize(self->view, 0);	
	if (psy_ui_component_visible(&self->parameterbox.component)) {				
		viewsize.width = psy_ui_value_makepx(psy_ui_value_px(&viewsize.width, &tm) + 150);
	}	
	bar = psy_ui_component_preferredsize(&self->parameterbar.component,
		&viewsize);	
	psy_ui_component_clientresize(&self->component,
		psy_ui_size_make(
			viewsize.width,
			psy_ui_add_values(bar.height, viewsize.height, &tm)));
}

void machineframe_preferredviewsizechanged(MachineFrame* self, psy_ui_Component* sender)
{
	machineframe_resize(self);
}

void ondefaultfontchanged(MachineFrame* self, Workspace* sender)
{
	machineframe_resize(self);
}

void machineframe_setfloatbar(MachineFrame* self)
{	
	psy_ui_component_hide(&self->parameterbar.titlerow);
	psy_ui_component_show(&self->parameterbar.dock.component);
}

void machineframe_setdockbar(MachineFrame* self)
{
	if (self->machine) {
		psy_ui_label_settext(&self->parameterbar.title,
			psy_audio_machine_editname(self->machine));
	} else {
		psy_ui_label_settext(&self->parameterbar.title, "");
	}
	psy_ui_component_hide(&self->parameterbar.dock.component);
	psy_ui_component_show(&self->parameterbar.titlerow);
}

void machineframe_onimport(MachineFrame* self, psy_ui_Component* sender)
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
		parameterbar_setprogram(&self->parameterbar, self->presets, 0);		
	}
	psy_ui_opendialog_dispose(&dialog);
}

void machineframe_onexport(MachineFrame* self, psy_ui_Component* sender)
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

void machineframe_onsavepresets(MachineFrame* self, psy_ui_Component* sender)
{
	if (self->machine) {
		psy_audio_Preset* preset;

		preset = psy_audio_preset_allocinit();
		psy_audio_machine_currentpreset(self->machine, preset);
		if (self->presets) {
			uintptr_t index;
			int status;			

			index = psy_ui_combobox_cursel(&self->parameterbar.programbox);			
			psy_audio_preset_setname(preset,
				psy_ui_edit_text(&self->parameterbar.savename));
			psy_audio_presets_insert(self->presets, index, preset);
			if (!self->parameterbar.userpreset) {
				psy_path_setprefix(&self->parameterbar.presetpath,
					workspace_userpresets_directory(self->workspace));
				self->parameterbar.userpreset = TRUE;
			}
			status = psy_audio_presetsio_save(psy_path_path(&self->parameterbar.presetpath),
				self->presets);
			if (status) {
				workspace_outputerror(self->workspace,
					psy_audio_presetsio_statusstr(status));
			}
			parameterbar_setprogram(&self->parameterbar, self->presets, index);
		} else {
			psy_audio_preset_dispose(preset);
			free(preset);
		}
		psy_ui_component_setfocus(&self->component);
	}
}

void machineframe_onsavenameeditkeydown(MachineFrame* self,
	psy_ui_Component* sender, psy_ui_KeyEvent* ev)
{
	if (ev->keycode == psy_ui_KEY_RETURN) {
		machineframe_onsavepresets(self, &self->component);
		psy_ui_component_setfocus(&self->component);
		psy_ui_keyevent_preventdefault(ev);		
	} else if (ev->keycode == psy_ui_KEY_ESCAPE) {
		int index;		
		psy_audio_Preset* preset;

		index = psy_ui_combobox_cursel(&self->parameterbar.programbox);
		preset = psy_audio_presets_at(self->presets, (uintptr_t)index);
		if (preset) {
			psy_ui_edit_settext(&self->parameterbar.savename,
				psy_audio_preset_name(preset));
		} else {
			psy_ui_edit_settext(&self->parameterbar.savename, "");
		}
		psy_ui_component_setfocus(&self->component);
		psy_ui_keyevent_preventdefault(ev);
	}
}
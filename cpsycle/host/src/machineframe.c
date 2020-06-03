// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "machineframe.h"
#include "resources/resource.h"
#include <dir.h>
#include <presetio.h>
#include <uiframe.h>
#include <string.h>
#include <stdlib.h>
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

static void parameterbar_setpresetlist(ParameterBar*, psy_audio_Presets*);
static void machineframe_buildbanks(MachineFrame*);
static void machineframe_buildprograms(MachineFrame*);
static void plugineditor_onprogramselected(MachineFrame*, psy_ui_Component* sender, int slot);
static void plugineditor_onbankselected(MachineFrame*, psy_ui_Component* sender, int slot);

static void ondefaultfontchanged(MachineFrame*, Workspace* sender);

void parameterbar_init(ParameterBar* self, psy_ui_Component* parent)
{				
	self->presets = 0;	
	psy_ui_component_init(&self->component, parent);
	psy_ui_component_enablealign(&self->component);
	// titlerow
	psy_ui_component_init(&self->titlerow, &self->component);
	psy_ui_component_enablealign(&self->titlerow);
	psy_ui_component_setalign(&self->titlerow, psy_ui_ALIGN_TOP);
	psy_ui_label_init(&self->title, &self->titlerow);
	psy_ui_label_settext(&self->title, "");
	psy_ui_component_setalign(&self->title.component, psy_ui_ALIGN_LEFT);	
	psy_ui_button_init(&self->close, &self->titlerow);
	psy_ui_button_settext(&self->close, "X");
	psy_ui_component_setalign(&self->close.component, psy_ui_ALIGN_RIGHT);
	psy_ui_button_init(&self->floatview, &self->titlerow);
	psy_ui_button_settext(&self->floatview, "Float");
	psy_ui_component_setalign(&self->floatview.component, psy_ui_ALIGN_RIGHT);
	// row0
	psy_ui_component_init(&self->row0, &self->component);
	psy_ui_component_enablealign(&self->row0);
	psy_ui_component_setalign(&self->row0, psy_ui_ALIGN_TOP);
	psy_ui_button_init(&self->mute, &self->row0);	
	psy_ui_button_settext(&self->mute, "Mute");
	psy_ui_component_setalign(&self->mute.component, psy_ui_ALIGN_LEFT);
	psy_ui_button_init(&self->parameters, &self->row0);
	psy_ui_button_settext(&self->parameters, "Parameters");	
	psy_ui_component_setalign(&self->parameters.component, psy_ui_ALIGN_LEFT);
	psy_ui_button_init(&self->help, &self->row0);	
	psy_ui_button_settext(&self->help, "Help");
	psy_ui_component_setalign(&self->help.component, psy_ui_ALIGN_LEFT);
	psy_ui_button_init(&self->dock, &self->row0);
	psy_ui_button_settext(&self->dock, "Dock");
	psy_ui_component_setalign(&self->dock.component, psy_ui_ALIGN_LEFT);
	psy_ui_combobox_init(&self->presetsbox, &self->component);
	psy_ui_component_setalign(&self->presetsbox.component, psy_ui_ALIGN_TOP);
	// row1
	psy_ui_component_init(&self->row1, &self->component);
	psy_ui_component_enablealign(&self->row1);
	psy_ui_component_setalign(&self->row1, psy_ui_ALIGN_TOP);
	psy_ui_combobox_init(&self->bankselector, &self->row1);
	psy_ui_combobox_setcharnumber(&self->bankselector, 20);
	psy_ui_component_setalign(&self->bankselector.component, psy_ui_ALIGN_LEFT);
	psy_ui_combobox_init(&self->programbox, &self->row1);
	psy_ui_combobox_setcharnumber(&self->programbox, 32);
	psy_ui_component_setalign(&self->programbox.component, psy_ui_ALIGN_LEFT);
}

void parameterbar_setpresetlist(ParameterBar* self, psy_audio_Presets* presets)
{
	psy_List* p;
	self->presets = presets;
	
	psy_ui_combobox_clear(&self->presetsbox);
	if (self->presets) {
		for (p = presets->container; p != NULL; p = p->next) {
			psy_audio_Preset* preset;

			preset = (psy_audio_Preset*) p->entry;
			psy_ui_combobox_addtext(&self->presetsbox,
				psy_audio_preset_name(preset));
		}
		psy_ui_combobox_setcursel(&self->presetsbox, 0);
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
		psy_ui_component_move(&self->component, psy_ui_value_makepx(200),
			psy_ui_value_makepx(150));
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
	psy_signal_connect(&self->parameterbar.presetsbox.signal_selchanged,
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
}

void machineframe_setview(MachineFrame* self, psy_ui_Component* view,
	psy_audio_Machine* machine)
{
	const psy_audio_MachineInfo* info;
	char name[4096];
	char prefix[4096];
	char ext[4096];
	char prspath[4096];	
	char text[128];

	self->view = (psy_ui_Component*) view;
	psy_ui_component_setalign(self->view, psy_ui_ALIGN_CLIENT);
	self->machine = machine;
	parameterlistbox_setmachine(&self->parameterbox, machine);
	psy_ui_component_hide(&self->parameterbox.component);
	info = psy_audio_machine_info(machine);
	if (info && info->modulepath) {			
		psy_dir_extract_path(info->modulepath, prefix, name, ext);
		psy_snprintf(prspath, 4096, "%s\\%s%s", prefix, name, ".prs");
		if (self->presets) {
			psy_audio_presets_dispose(self->presets);
		}
		self->presets = psy_audio_presets_allocinit();
		psy_audio_presetsio_load(prspath, self->presets,
			psy_audio_machine_numtweakparameters(self->machine),
			psy_audio_machine_datasize(self->machine),
			workspace_plugins_directory(self->workspace));
	}
	parameterbar_setpresetlist(&self->parameterbar, self->presets);		
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
		psy_List* p;
		int c = 0;

		p = self->presets->container;
		while (p != NULL) {
			if (c == index) {
				psy_audio_Preset* preset;
				psy_TableIterator it;

				preset = (psy_audio_Preset*) p->entry;
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
				break;
			}
			++c;
			p = p->next;
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
		psy_ui_value_px(&viewsize.width, &tm),
		psy_ui_value_px(&bar.height, &tm) + psy_ui_value_px(&viewsize.height, &tm));
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
		psy_ui_label_settext(&self->parameterbar.title, psy_audio_machine_editname(self->machine));
	} else {
		psy_ui_label_settext(&self->parameterbar.title, "");
	}
	psy_ui_component_hide(&self->parameterbar.dock.component);
	psy_ui_component_show(&self->parameterbar.titlerow);
}
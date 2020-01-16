// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "machineframe.h"
#include "resources/resource.h"
#include <dir.h>
#include "../../detail/portable.h"
#include <presetio.h>
#include <windows.h>

static void machineframe_ondestroy(MachineFrame* self, psy_ui_Component* frame);
static void machineframe_onpresetchange(MachineFrame*, psy_ui_Component* sender, int index);
static void machineframe_toggleparameterbox(MachineFrame*,
	psy_ui_Component* sender);
static void machineframe_togglehelp(MachineFrame*,
	psy_ui_Component* sender);
static void machineframe_resize(MachineFrame*);

static void parameterbar_setpresetlist(ParameterBar*, psy_audio_Presets*);

void parameterbar_init(ParameterBar* self, psy_ui_Component* parent)
{				
	self->presets = 0;
	ui_component_init(&self->component, parent);
	ui_component_enablealign(&self->component);
	// row0
	ui_component_init(&self->row0, &self->component);
	ui_component_enablealign(&self->row0);
	ui_component_setalign(&self->row0, psy_ui_ALIGN_TOP);
	psy_ui_button_init(&self->mute, &self->row0);	
	psy_ui_button_settext(&self->mute, "Mute");
	ui_component_setalign(&self->mute.component, psy_ui_ALIGN_LEFT);
	psy_ui_button_init(&self->parameters, &self->row0);
	psy_ui_button_settext(&self->parameters, "Parameters");	
	ui_component_setalign(&self->parameters.component, psy_ui_ALIGN_LEFT);
	psy_ui_button_init(&self->help, &self->row0);	
	psy_ui_button_settext(&self->help, "Help");
	ui_component_setalign(&self->help.component, psy_ui_ALIGN_LEFT);
	// row1 
	//ui_component_init(&self->row1, &self->component);
	//ui_component_enablealign(&self->row0);
	//ui_component_setalign(&self->row1, UI_ALIGN_TOP);
	ui_combobox_init(&self->presetsbox, &self->component);
	ui_component_setalign(&self->presetsbox.component, psy_ui_ALIGN_TOP);	
}

void parameterbar_setpresetlist(ParameterBar* self, psy_audio_Presets* presets)
{
	psy_List* p;
	self->presets = presets;
	
	ui_combobox_clear(&self->presetsbox);
	if (self->presets) {
		for (p = presets->container; p != 0; p = p->next) {
			psy_audio_Preset* preset;

			preset = (psy_audio_Preset*) p->entry;
			ui_combobox_addstring(&self->presetsbox,
				psy_audio_preset_name(preset));
		}
		ui_combobox_setcursel(&self->presetsbox, 0);
	}
}

void machineframe_init(MachineFrame* self, psy_ui_Component* parent)
{		
	self->view = 0;
	self->presets = 0;
	self->machine = 0;
	ui_frame_init(&self->component, parent);
	ui_component_seticonressource(&self->component, IDI_MACPARAM);
	ui_component_move(&self->component, 200, 150);	
	ui_component_enablealign(&self->component);	
	parameterbar_init(&self->parameterbar, &self->component);
	ui_component_setalign(&self->parameterbar.component, psy_ui_ALIGN_TOP);
	parameterlistbox_init(&self->parameterbox, &self->component, 0);
	ui_component_setalign(&self->parameterbox.component, psy_ui_ALIGN_RIGHT);
	psy_ui_notebook_init(&self->notebook, &self->component);
	ui_component_setalign(psy_ui_notebook_base(&self->notebook),
		psy_ui_ALIGN_CLIENT);
	ui_editor_init(&self->help, psy_ui_notebook_base(&self->notebook));
	ui_editor_addtext(&self->help, "About");
	psy_signal_connect(&self->component.signal_destroy, self,
		machineframe_ondestroy);	
	psy_signal_connect(&self->parameterbar.presetsbox.signal_selchanged,
		self, machineframe_onpresetchange);
	psy_signal_connect(&self->parameterbar.parameters.signal_clicked, self,
		machineframe_toggleparameterbox);
		psy_signal_connect(&self->parameterbar.help.signal_clicked, self,
		machineframe_togglehelp);
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
	ui_component_setalign(self->view, psy_ui_ALIGN_CLIENT);
	self->machine = machine;
	parameterlistbox_setmachine(&self->parameterbox, machine);
	ui_component_hide(&self->parameterbox.component);
	info = psy_audio_machine_info(machine);
	if (info && info->modulepath) {
		psy_dir_extract_path(info->modulepath, prefix, name, ext);
		psy_snprintf(prspath, 4096, "%s\\%s%s", prefix, name, ".prs");
		if (self->presets) {
			psy_audio_presets_dispose(self->presets);
		}
		self->presets = psy_audio_presets_allocinit();
		psy_audio_presetsio_load(prspath, self->presets);		
	}
	parameterbar_setpresetlist(&self->parameterbar, self->presets);		
	machineframe_resize(self);
	if (self->machine && psy_audio_machine_info(self->machine)) {
		psy_snprintf(text, 128, "%.2X : %s",
			psy_audio_machine_slot(self->machine),
			psy_audio_machine_info(self->machine)->ShortName);
	} else {
		ui_component_settitle(&self->component, text);
			psy_snprintf(text, 128, "%.2X :",
				psy_audio_machine_slot(self->machine));
	}
	psy_ui_notebook_setpageindex(&self->notebook, 1);
	ui_component_settitle(&self->component, text);
	ui_component_align(&self->component);
}

void machineframe_ondestroy(MachineFrame* self, psy_ui_Component* frame)
{
	self->component.hwnd = 0; //.platform = 0;
	if (self->presets) {
		psy_audio_presets_dispose(self->presets);
		self->presets = 0;
	}
}

void machineframe_onpresetchange(MachineFrame* self, psy_ui_Component* sender, int index)
{
	if (self->presets && self->machine) {
		psy_List* p;
		int c = 0;

		p = self->presets->container;
		while (p != 0) {
			if (c == index) {
				psy_audio_Preset* preset;
				psy_TableIterator it;

				preset = (psy_audio_Preset*) p->entry;
				for (it = psy_table_begin(&preset->parameters); 
						!psy_tableiterator_equal(&it, psy_table_end());
						psy_tableiterator_inc(&it)) {
					psy_audio_machine_parametertweak(self->machine,
						psy_tableiterator_key(&it),
						(uintptr_t)psy_tableiterator_value(&it));
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

MachineFrame* machineframe_allocinit(psy_ui_Component* parent)
{
	MachineFrame* rv;

	rv = machineframe_alloc();
	if (rv) {
		machineframe_init(rv, parent);
	}
	return rv;	
}

void machineframe_toggleparameterbox(MachineFrame* self,
	psy_ui_Component* sender)
{
	psy_ui_Size viewsize;

	viewsize = ui_component_preferredsize(self->view, 0);
	if (ui_component_visible(&self->parameterbox.component)) {
		ui_component_hide(&self->parameterbox.component);		
		psy_ui_button_disablehighlight(&self->parameterbar.parameters);
	} else {
		ui_component_show(&self->parameterbox.component);		
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

void machineframe_resize(MachineFrame* self)
{
	psy_ui_Size viewsize;	
	psy_ui_Size bar;
	
	viewsize = ui_component_preferredsize(self->view, 0);	
	if (ui_component_visible(&self->parameterbox.component)) {				
		viewsize.width += 150;		
	}	
	bar = ui_component_preferredsize(&self->parameterbar.component,
		&viewsize);	
	ui_component_clientresize(&self->component,		
		viewsize.width,
		bar.height + viewsize.height);
}

// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "machineframe.h"
#include "resources/resource.h"
#include <dir.h>
#include <portable.h>
#include <presetio.h>

static void machineframe_ondestroy(MachineFrame* self, psy_ui_Component* frame);
static void machineframe_onpresetchange(MachineFrame*, psy_ui_Component* sender, int index);
static void parameterbar_setpresetlist(ParameterBar*, psy_audio_Presets*);

void parameterbar_init(ParameterBar* self, psy_ui_Component* parent)
{				
	self->presets = 0;
	ui_component_init(&self->component, parent);
	ui_component_enablealign(&self->component);
	ui_button_init(&self->mute, &self->component);
	ui_button_settext(&self->mute, "Mute");
	ui_combobox_init(&self->presetsbox, &self->component);
	ui_combobox_setcharnumber(&self->presetsbox, 30);	
	psy_list_free(ui_components_setalign(
		ui_component_children(&self->component, 0),
		UI_ALIGN_LEFT,
		0));	
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
			ui_combobox_addstring(&self->presetsbox, preset_name(preset));
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
	ui_component_resize(&self->component, 800, 800);	

	ui_component_enablealign(&self->component);
	parameterbar_init(&self->parameterbar, &self->component);
	ui_component_setalign(&self->parameterbar.component, UI_ALIGN_TOP);	
	parameterlistbox_init(&self->parameterbox, &self->component, 0);
	ui_component_setalign(&self->parameterbox.component, UI_ALIGN_RIGHT);
	psy_signal_connect(&self->component.signal_destroy, self,
		machineframe_ondestroy);	
	psy_signal_connect(&self->parameterbar.presetsbox.signal_selchanged,
		self, machineframe_onpresetchange);
}

void machineframe_setview(MachineFrame* self, psy_ui_Component* view,
	psy_audio_Machine* machine)
{
	const psy_audio_MachineInfo* info;
	char name[4096];
	char prefix[4096];
	char ext[4096];
	char prspath[4096];
	ui_size viewsize;
	char text[128];

	self->view = (psy_ui_Component*) view;
	ui_component_setalign(self->view, UI_ALIGN_CLIENT);
	self->machine = machine;
	parameterlistbox_setmachine(&self->parameterbox, machine);
	info = machine->vtable->info(machine);
	if (info && info->modulepath) {
		psy_dir_extract_path(info->modulepath, prefix, name, ext);
		psy_snprintf(prspath, 4096, "%s\\%s%s", prefix, name, ".prs");
		if (self->presets) {
			presets_dispose(self->presets);
		}
		self->presets = presets_allocinit();
		presetsio_load(prspath, self->presets);		
	}
	parameterbar_setpresetlist(&self->parameterbar, self->presets);	
	viewsize = ui_component_preferredsize(view, 0);
	ui_component_resize(&self->component, viewsize.width + 150,
		viewsize.height + 28);
	if (self->machine && self->machine->vtable->info(self->machine)) {
		psy_snprintf(text, 128, "%.2X : %s",
			self->machine->vtable->slot(self->machine),
		self->machine->vtable->info(self->machine)->ShortName);
	} else {
		ui_component_settitle(&self->component, text);
			psy_snprintf(text, 128, "%.2X :",
				self->machine->vtable->slot(self->machine));
	}
	ui_component_settitle(&self->component, text);
	ui_component_align(&self->component);
}

void machineframe_ondestroy(MachineFrame* self, psy_ui_Component* frame)
{
	self->component.hwnd = 0;
	if (self->presets) {
		presets_dispose(self->presets);
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
					self->machine->vtable->parametertweak(self->machine,
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


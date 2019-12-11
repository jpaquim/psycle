// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "machineframe.h"
#include "resources/resource.h"
#include <dir.h>
#include <portable.h>
#include <presetio.h>

static void machineframe_ondestroy(MachineFrame* self, ui_component* frame);
static void machineframe_onalign(MachineFrame*, ui_component* sender);
static void machineframe_onpresetchange(MachineFrame*, ui_component* sender, int index);
static void parameterbar_setpresetlist(ParameterBar*, Presets*);

void parameterbar_init(ParameterBar* self, ui_component* parent)
{				
	self->presets = 0;
	ui_component_init(&self->component, parent);
	ui_component_enablealign(&self->component);
	ui_button_init(&self->mute, &self->component);
	ui_button_settext(&self->mute, "Mute");
	ui_combobox_init(&self->presetsbox, &self->component);
	ui_combobox_setcharnumber(&self->presetsbox, 30);	
	list_free(ui_components_setalign(
		ui_component_children(&self->component, 0),
		UI_ALIGN_LEFT,
		0));	
}

void parameterbar_setpresetlist(ParameterBar* self, Presets* presets)
{
	List* p;
	self->presets = presets;
	
	ui_combobox_clear(&self->presetsbox);
	for (p = presets->container; p != 0; p = p->next) {
		Preset* preset;

		preset = (Preset*) p->entry;
		ui_combobox_addstring(&self->presetsbox, preset_name(preset));
	}
	ui_combobox_setcursel(&self->presetsbox, 0);
}

void machineframe_init(MachineFrame* self, ui_component* parent)
{		
	self->view = 0;
	self->presets = 0;
	self->machine = 0;
	ui_frame_init(&self->component, parent);
	ui_component_enablealign(&self->component);
	parameterbar_init(&self->parameterbar, &self->component);
	ui_component_seticonressource(&self->component, IDI_MACPARAM);
	ui_component_move(&self->component, 200, 150);
	ui_component_resize(&self->component, 400, 400);	
	psy_signal_connect(&self->component.signal_destroy, self,
		machineframe_ondestroy);
	psy_signal_connect(&self->component.signal_align, self,
		machineframe_onalign);
	psy_signal_connect(&self->parameterbar.presetsbox.signal_selchanged,
		self, machineframe_onpresetchange);
}

void machineframe_setview(MachineFrame* self, ui_component* view,
	Machine* machine)
{
	const MachineInfo* info;
	char name[4096];
	char prefix[4096];
	char ext[4096];
	char prspath[4096];

	self->view = (ui_component*) view;
	self->machine = machine;
	info = machine->vtable->info(machine);
	extract_path(info->modulepath, prefix, name, ext);
	psy_snprintf(prspath, 4096, "%s\\%s%s", prefix, name, ".prs");
	if (self->presets) {
		presets_dispose(self->presets);
	}
	self->presets = presets_allocinit();
	presetsio_load(prspath, self->presets);
	parameterbar_setpresetlist(&self->parameterbar, self->presets);
}

void machineframe_ondestroy(MachineFrame* self, ui_component* frame)
{
	self->component.hwnd = 0;
	if (self->presets) {
		presets_dispose(self->presets);
		self->presets = 0;
	}
}

void machineframe_onalign(MachineFrame* self, ui_component* sender)
{
	ui_size size;

	size = ui_component_size(&self->component);
	ui_component_setposition(&self->parameterbar.component, 0, 0, size.width, 20);
	if (self->view) {		
		ui_component_setposition(self->view, 0, 20, size.width, size.height - 20);
	}
}

void machineframe_onpresetchange(MachineFrame* self, ui_component* sender, int index)
{
	if (self->presets && self->machine) {
		List* p;
		int c = 0;

		p = self->presets->container;
		while (p != 0) {
			if (c == index) {
				Preset* preset;
				TableIterator it;

				preset = (Preset*) p->entry;
				for (it = table_begin(&preset->parameters); 
						!tableiterator_equal(&it, table_end());
						tableiterator_inc(&it)) {
					self->machine->vtable->parametertweak(self->machine,
						tableiterator_key(&it),
						(uintptr_t)tableiterator_value(&it));
				}		
				break;
			}
			++c;
			p = p->next;
		}
	}
}

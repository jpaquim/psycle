// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "paramlistbox.h"
#include <stdio.h>
#include <portable.h>

static void parameterlistbox_build(ParameterListBox*);
static void parameterlistbox_onpreferredsize(ParameterListBox*,
	ui_component* sender, ui_size* limit, ui_size* rv);
static void parameterlistbox_ondescribe(ParameterListBox*, ui_slider*,
	char* txt);
static void parameterlistbox_ontweak(ParameterListBox*, ui_slider*,
	float value);
static void parameterlistbox_onvalue(ParameterListBox*, ui_slider*,
	float* value);

void parameterlistbox_init(ParameterListBox* self, ui_component* parent,
	psy_audio_Machine* machine)
{	
	ui_component_init(&self->component, parent);	
	ui_component_enablealign(&self->component);
	ui_listbox_init(&self->listbox, &self->component);
	ui_component_setalign(&self->listbox.component, UI_ALIGN_CLIENT);
	ui_slider_init(&self->slider, &self->component);
	ui_slider_setcharnumber(&self->slider, 4);
	ui_slider_showvertical(&self->slider);
	ui_component_resize(&self->slider.component, 20, 0);
	ui_component_setalign(&self->slider.component, UI_ALIGN_RIGHT);
	ui_component_resize(&self->component, 150, 200);
	parameterlistbox_setmachine(self, machine);
		psy_signal_disconnectall(&self->component.signal_preferredsize);
	psy_signal_connect(&self->component.signal_preferredsize, self,
		parameterlistbox_onpreferredsize);	
	ui_slider_connect(&self->slider, self, parameterlistbox_ondescribe,
		parameterlistbox_ontweak, parameterlistbox_onvalue);
	// psy_signal_connect(&self->parameterlist.signal_selchanged, self,
		//onparameterlistchanged);
}

void parameterlistbox_build(ParameterListBox* self)
{	
	ui_listbox_clear(&self->listbox);
	if (self->machine) {
		uintptr_t param = 0;
		char label[256];
		char text[256];

		for (param = 0; param < machine_numparameters(self->machine); ++param) {		
			if (machine_parameterlabel(self->machine, label, param)) {
				psy_snprintf(text, 256, "%02X:%s", (int) param, label);
			} else
			if (machine_parametername(self->machine, label, param)) {
				psy_snprintf(text, 256, "%02X:%s", (int) param, label);
			} else {			
				psy_snprintf(text, 256, "%02X: Parameter", (int) param);
			}
			ui_listbox_addstring(&self->listbox, text);
		}
	}
}

// void OnParameterListChanged(ParametersBox* self, ui_component* sender, int slot)
// {
//	Parameters_changeslot(self->Parameters, slot);
// }

void parameterlistbox_setmachine(ParameterListBox* self,
	psy_audio_Machine* machine)
{
	self->machine = machine;
	parameterlistbox_build(self);	
}

int parameterlistbox_selected(ParameterListBox* self)
{
	return ui_listbox_cursel(&self->listbox);
}

void parameterlistbox_onpreferredsize(ParameterListBox* self,
	ui_component* sender, ui_size* limit, ui_size* rv)
{
	if (rv) {
		*rv = ui_component_size(&self->component);

	}
}

void parameterlistbox_ondescribe(ParameterListBox* self, ui_slider* slider, char* txt)
{
	uintptr_t param = 0;

	txt[0] = '\0';
	if (self->machine) {
		if (!machine_describevalue(self->machine, txt, 0,
				machine_parametervalue(self->machine, param))) {
			int intval;

			intval = machine_parametervalue(self->machine, param);
			psy_snprintf(txt, 256, "%d", intval);
		}
	}
}

void parameterlistbox_ontweak(ParameterListBox* self, ui_slider* slider, float value)
{
	uintptr_t param = 0;

	if (self->machine) {
		int intval;
		int minval;
		int maxval;

		machine_parameterrange(self->machine, param, &minval, &maxval);
		intval = (int) (value * (maxval - minval)) + minval;
		machine_parametertweak(self->machine, intval, param);
	}
}

void parameterlistbox_onvalue(ParameterListBox* self, ui_slider* slider, float* value)
{
	uintptr_t param = 0;
	
	*value = 0.f;
	if (self->machine) {		
		int intval;
		int minval;
		int maxval;

		machine_parameterrange(self->machine, param, &minval, &maxval);
		intval = machine_parametervalue(self->machine, param);
		*value = (intval - minval) / (float) (maxval - minval);
	}
}

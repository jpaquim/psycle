// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "paramlistbox.h"
#include <stdio.h>
#include "../../detail/portable.h"

static void parameterlistbox_build(ParameterListBox*);
static void parameterlistbox_ondescribe(ParameterListBox*, psy_ui_Slider*,
	char* txt);
static void parameterlistbox_ontweak(ParameterListBox*, psy_ui_Slider*,
	float value);
static void parameterlistbox_onvalue(ParameterListBox*, psy_ui_Slider*,
	float* value);

void parameterlistbox_init(ParameterListBox* self, psy_ui_Component* parent,
	psy_audio_Machine* machine)
{	
	psy_ui_component_init(&self->component, parent);	
	psy_ui_listbox_init(&self->listbox, &self->component);
	psy_ui_component_setalign(&self->listbox.component, psy_ui_ALIGN_CLIENT);
	psy_ui_component_setpreferredsize(&self->listbox.component,
		psy_ui_size_make(psy_ui_value_makeew(40),
			psy_ui_value_makeeh(20)));
	psy_ui_slider_init(&self->slider, &self->component);
	psy_ui_slider_setcharnumber(&self->slider, 4);
	psy_ui_slider_showvertical(&self->slider);
	psy_ui_component_setpreferredsize(&self->slider.component,
		psy_ui_size_make(psy_ui_value_makeew(2), psy_ui_value_makepx(0)));
	psy_ui_component_setalign(&self->slider.component, psy_ui_ALIGN_RIGHT);	
	parameterlistbox_setmachine(self, machine);	
	psy_ui_slider_connect(&self->slider, self,
		(ui_slider_fpdescribe)parameterlistbox_ondescribe,
		(ui_slider_fptweak)parameterlistbox_ontweak,
		(ui_slider_fpvalue)parameterlistbox_onvalue);
	// psy_signal_connect(&self->parameterlist.signal_selchanged, self,
		//onparameterlistchanged);
}

void parameterlistbox_build(ParameterListBox* self)
{	
	psy_ui_listbox_clear(&self->listbox);
	if (self->machine) {
		uintptr_t i = 0;
		char label[256];
		char text[256];		
		

		for (i = 0; i < psy_audio_machine_numtweakparameters(self->machine); ++i) {
			psy_audio_MachineParam* param;

			param = psy_audio_machine_tweakparameter(self->machine, i);
			if (param) {
				if (psy_audio_machineparam_label(param, label)) {
					psy_snprintf(text, 256, "%02X:%s", (int)i, label);
				} else
					if (psy_audio_machineparam_name(param, label)) {
						psy_snprintf(text, 256, "%02X:%s", (int)i, label);
					} else {
						psy_snprintf(text, 256, "%02X: Parameter", (int)i);
					}
				psy_ui_listbox_addtext(&self->listbox, text);
			} else {
				psy_snprintf(text, 256, "%s", "--------");
			}
		}
	}
}

// void OnParameterListChanged(ParametersBox* self, psy_ui_Component* sender, int slot)
// {
//	Parameters_changeslot(self->Parameters, slot);
// }

void parameterlistbox_setmachine(ParameterListBox* self,
	psy_audio_Machine* machine)
{
	self->machine = machine;
	parameterlistbox_build(self);	
}

intptr_t parameterlistbox_selected(ParameterListBox* self)
{
	return psy_ui_listbox_cursel(&self->listbox);
}

void parameterlistbox_ondescribe(ParameterListBox* self, psy_ui_Slider* slider, char* txt)
{
	uintptr_t param = 0;

	txt[0] = '\0';
	if (self->machine) {
		//if (!psy_audio_machine_describevalue(self->machine, txt, 0,
		//		machine_parametervalue_scaled(self->machine, param,
		//			psy_audio_machine_parametervalue(self->machine, param)))) {
		//	int intval;

		//	intval = machine_parametervalue_scaled(self->machine, param,
		//		psy_audio_machine_parametervalue(self->machine, param));
		//	psy_snprintf(txt, 256, "%d", intval);
		//}
	}
}

void parameterlistbox_ontweak(ParameterListBox* self, psy_ui_Slider* slider, float value)
{
	uintptr_t param = 0;

	if (self->machine) {		
		//psy_audio_machine_parametertweak(self->machine, param, value);
	}
}

void parameterlistbox_onvalue(ParameterListBox* self, psy_ui_Slider* slider, float* value)
{
	uintptr_t param = 0;
	
	*value = 0; // psy_audio_machine_parametervalue(self->machine, param);
}

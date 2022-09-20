/*
** This source is free software; you can redistribute itand /or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"

#include "paramlistbox.h"
#include "styles.h"
/* ui */
#include <uislider.h>
/* platform */
#include "../../detail/portable.h"

static void parameterlistbox_build(ParameterListBox*);
static void parameterlistbox_onlistboxselected(ParameterListBox*,
	psy_ui_Component* sender, intptr_t slot);

void parameterlistbox_init(ParameterListBox* self, psy_ui_Component* parent,
	psy_audio_Machine* machine, MachineParamConfig* config)
{	
	uintptr_t paramindex;	
	
	assert(config);

	psy_ui_component_init(&self->component, parent, NULL);
	psy_ui_component_set_style_type(&self->component, STYLE_MACPARAM_TITLE);	
	self->machine = machine;
	if (self->machine && psy_audio_machine_numtweakparameters(self->machine) > 0) {
		paramindex = 0;
	} else {
		paramindex = psy_INDEX_INVALID;
	}
	knobui_init(&self->knob, &self->component, machine, paramindex, NULL);
	psy_ui_component_set_align(&self->knob.component, psy_ui_ALIGN_TOP);
	psy_ui_listbox_init(&self->listbox, &self->component);	
	psy_ui_listbox_set_char_number(&self->listbox, 10.0);
	psy_signal_connect(&self->listbox.signal_selchanged, self,
		parameterlistbox_onlistboxselected);		
	psy_ui_component_set_align(&self->listbox.component, psy_ui_ALIGN_CLIENT);	
	parameterlistbox_setmachine(self, self->machine);	
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
				if (psy_audio_machineparam_name(param, label)) {
					psy_snprintf(text, 256, "%02X:%s", (int)i, label);
				} else
					if (psy_audio_machineparam_label(param, label)) {
						psy_snprintf(text, 256, "%02X:%s", (int)i, label);
					} else {
						psy_snprintf(text, 256, "%02X: Parameter", (int)i);
					}
				psy_ui_listbox_add_text(&self->listbox, text);
			} else {
				psy_snprintf(text, 256, "%s", "--------");
				psy_ui_listbox_add_text(&self->listbox, text);
			}
		}
	}
}

void parameterlistbox_onlistboxselected(ParameterListBox* self,
	psy_ui_Component* sender, intptr_t slot)
{
	uintptr_t paramindex;
			
	if (self->machine && slot < (intptr_t)psy_audio_machine_numtweakparameters(self->machine)) {
		paramindex = slot;		
	} else {
		paramindex = psy_INDEX_INVALID;
	}
	self->knob.paramidx = paramindex;
	psy_ui_component_invalidate(&self->knob.component);
}

void parameterlistbox_setmachine(ParameterListBox* self,
	psy_audio_Machine* machine)
{
	uintptr_t paramindex;

	self->machine = machine;
	parameterlistbox_build(self);
	self->knob.machine = machine;
	if (machine && psy_audio_machine_numtweakparameters(self->machine) > 0) {
		paramindex = 0;
	} else {
		paramindex = psy_INDEX_INVALID;
	}
	self->knob.paramidx = paramindex;
	psy_ui_component_invalidate(&self->knob.component);
}

intptr_t parameterlistbox_selected(ParameterListBox* self)
{
	return psy_ui_listbox_cur_sel(&self->listbox);
}

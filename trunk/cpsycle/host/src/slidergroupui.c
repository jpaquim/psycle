/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "slidergroupui.h"
#include "styles.h"

/* SliderUi */

/* implementation */
void slidergroupui_init(SliderGroupUi* self, psy_ui_Component* parent,
	psy_audio_Machine* machine, uintptr_t volumeparamidx,
	psy_audio_MachineParam* volume, uintptr_t levelparamidx,
	psy_audio_MachineParam* level)
{
	assert(self);	
	
	psy_ui_component_init(&self->component, parent, NULL);
	psy_ui_component_set_align_expand(&self->component, psy_ui_HEXPAND);
	psy_ui_component_set_style_type(&self->component, STYLE_MACPARAM_BOTTOM);		
	sliderui_init(&self->slider, &self->component, machine, volumeparamidx,
		volume);
	psy_ui_component_set_align(&self->slider.component, psy_ui_ALIGN_LEFT);
	levelui_init(&self->level, &self->component, machine, levelparamidx,
		level);
	psy_ui_component_set_align(&self->level.component, psy_ui_ALIGN_LEFT);
	labelui_init(&self->label, &self->component, machine, volumeparamidx,
		volume);
	psy_ui_component_set_align(&self->label.component, psy_ui_ALIGN_BOTTOM);
	psy_ui_component_init(&self->controls, &self->component, NULL);	
	psy_ui_component_set_align(&self->controls, psy_ui_ALIGN_CLIENT);
}

SliderGroupUi* slidergroupui_alloc(void)
{
	return (SliderGroupUi*)malloc(sizeof(SliderGroupUi));
}

SliderGroupUi* slidergroupui_allocinit(psy_ui_Component* parent,
	psy_audio_Machine* machine,
	uintptr_t volumeparamidx, psy_audio_MachineParam* volume,
	uintptr_t levelparamidx, psy_audio_MachineParam* level)
{
	SliderGroupUi* rv;

	rv = slidergroupui_alloc();
	if (rv) {
		slidergroupui_init(rv, parent, machine, volumeparamidx, volume,
			levelparamidx, level);
		rv->component.deallocate = TRUE;
	}
	return rv;
}

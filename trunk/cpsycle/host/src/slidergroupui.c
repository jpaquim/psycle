/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"

#include "slidergroupui.h"

/* SliderUi */

/* implementation */
void slidergroupui_init(SliderGroupUi* self, psy_ui_Component* parent,
	psy_audio_Machine* machine, uintptr_t volumeparamidx,
	psy_audio_MachineParam* volume, uintptr_t levelparamidx,
	psy_audio_MachineParam* level, ParamSkin* skin)
{
	assert(self);
	assert(skin);
	
	psy_ui_component_init(&self->component, parent, NULL);
	psy_ui_component_setalignexpand(&self->component, psy_ui_HEXPAND);
	psy_ui_component_setbackgroundcolour(&self->component,
		skin->bottomcolour);		
	sliderui_init(&self->slider, &self->component, machine, volumeparamidx,
		volume, skin);
	psy_ui_component_setalign(&self->slider.component, psy_ui_ALIGN_LEFT);
	levelui_init(&self->level, &self->component, machine, levelparamidx,
		level, skin);
	psy_ui_component_setalign(&self->level.component, psy_ui_ALIGN_LEFT);
	labelui_init(&self->label, &self->component, machine, volumeparamidx,
		volume, skin);
	psy_ui_component_setalign(&self->label.component, psy_ui_ALIGN_BOTTOM);
	psy_ui_component_init(&self->controls, &self->component, NULL);	
	psy_ui_component_setalign(&self->controls, psy_ui_ALIGN_CLIENT);
}

SliderGroupUi* slidergroupui_alloc(void)
{
	return (SliderGroupUi*)malloc(sizeof(SliderGroupUi));
}

SliderGroupUi* slidergroupui_allocinit(psy_ui_Component* parent,
	psy_audio_Machine* machine,
	uintptr_t volumeparamidx, psy_audio_MachineParam* volume,
	uintptr_t levelparamidx, psy_audio_MachineParam* level, ParamSkin* skin)
{
	SliderGroupUi* rv;

	rv = slidergroupui_alloc();
	if (rv) {
		slidergroupui_init(rv, parent, machine, volumeparamidx, volume,
			levelparamidx, level, skin);
		rv->component.deallocate = TRUE;
	}
	return rv;
}

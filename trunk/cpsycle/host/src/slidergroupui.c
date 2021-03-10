// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "slidergroupui.h"

// SliderUi
// implementation
void slidergroupui_init(SliderGroupUi* self, psy_ui_Component* parent,
	psy_ui_Component* view, psy_audio_MachineParam* volume,
	psy_audio_MachineParam* level, ParamSkin* skin)
{
	assert(self);
	assert(skin);
	
	psy_ui_component_init(&self->component, parent, view);
	psy_ui_component_setbackgroundcolour(&self->component,
		psy_ui_colour_make(0x00444444));
	sliderui_init(&self->slider, &self->component, view, volume, skin);
	psy_ui_component_setalign(&self->slider.component, psy_ui_ALIGN_LEFT);
	levelui_init(&self->level, &self->component, view, level, skin);
	psy_ui_component_setalign(&self->level.component, psy_ui_ALIGN_LEFT);
	labelui_init(&self->label, &self->component, view, volume, skin);
	psy_ui_component_setalign(&self->label.component, psy_ui_ALIGN_BOTTOM);
}

SliderGroupUi* slidergroupui_alloc(void)
{
	return (SliderGroupUi*)malloc(sizeof(SliderGroupUi));
}

SliderGroupUi* slidergroupui_allocinit(psy_ui_Component* parent,
	psy_ui_Component* view, psy_audio_MachineParam* volume,
	psy_audio_MachineParam* level, ParamSkin* skin)
{
	SliderGroupUi* rv;

	rv = slidergroupui_alloc();
	if (rv) {
		slidergroupui_init(rv, parent, view, volume, level, skin);
		rv->component.deallocate = TRUE;
	}
	return rv;
}

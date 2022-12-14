/*
** This source is free software; you can redistribute itand /or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "volslider.h"
/* audio */
#include <convert.h>
/* platform */
#include "../../detail/portable.h"


/* prototypes */
static void volslider_on_describe(VolSlider*, psy_ui_Slider*, char* text);
static void volslider_on_tweak(VolSlider*, psy_ui_Slider*, float value);
static void volslider_on_value(VolSlider*, psy_ui_Slider*, float* value);

/* implementation */
void volslider_init(VolSlider* self, psy_ui_Component* parent,
	Workspace* workspace)
{	
	assert(self);
	assert(workspace);
	
	psy_ui_component_init(&self->component, parent, NULL);
	self->workspace = workspace;	
	psy_ui_slider_init(&self->slider, &self->component);
	psy_ui_component_set_align(&self->slider.component, psy_ui_ALIGN_TOP);
	psy_ui_component_hide(&self->slider.desc.component);
	psy_ui_slider_setvaluecharnumber(&self->slider, 10.0);
	psy_ui_component_set_margin(&self->slider.pane.component,
		psy_ui_margin_make_em(0.0, 1.0, 0.0, 0.0));
	psy_ui_slider_startpoll(&self->slider);
	psy_ui_slider_set_wheel_step(&self->slider, 0.02);	
	psy_ui_slider_connect(&self->slider, self,
		(ui_slider_fpdescribe)volslider_on_describe,
		(ui_slider_fptweak)volslider_on_tweak,
		(ui_slider_fpvalue)volslider_on_value);
}

void volslider_on_describe(VolSlider* self, psy_ui_Slider* sender, char* text)
{
	assert(self);

	if (workspace_song(self->workspace) && psy_audio_machines_master(
			&workspace_song(self->workspace)->machines)) {
		psy_audio_MachineParam* param;

		param = psy_audio_machine_tweakparameter(
			psy_audio_machines_master(
				&workspace_song(self->workspace)->machines), 0);
		if (param) {
			float normvalue;
			float volume;

			normvalue = psy_audio_machineparam_normvalue(param);
			volume = (normvalue * 2) * (normvalue * 2);			
			psy_snprintf(text, 10, "%.2f dB",
				(float)psy_dsp_convert_amp_to_db(volume));
		}
	}
}

void volslider_on_tweak(VolSlider* self, psy_ui_Slider* sender, float value)
{
	assert(self);

	if (workspace_song(self->workspace) && psy_audio_machines_master(
			&workspace_song(self->workspace)->machines)) {
		psy_audio_MachineParam* param;

		param = psy_audio_machine_tweakparameter(
			psy_audio_machines_master(&workspace_song(self->workspace)->machines), 0);
		if (param) {
			psy_audio_machineparam_tweak(param, value);
		}
	}
}

void volslider_on_value(VolSlider* self, psy_ui_Slider* sender, float* rv)
{	
	assert(self);

	if (workspace_song(self->workspace) && psy_audio_machines_master(
			&workspace_song(self->workspace)->machines)) {
		psy_audio_MachineParam* param;

		param = psy_audio_machine_tweakparameter(
			psy_audio_machines_master(&workspace_song(self->workspace)->machines), 0);
		if (param) {
			*rv = psy_audio_machineparam_normvalue(param);
			return;
		}
	}
	*rv = 0.f;
}

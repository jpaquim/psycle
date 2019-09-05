// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#include "slidergroup.h"
#include <stdio.h>

static void OnDestroy(SliderGroup* self);
static void OnSize(SliderGroup* self, ui_component* sender, int width, int height);
static void OnSliderChanged(SliderGroup* self, ui_component* sender);

void InitSliderGroup(SliderGroup* self, ui_component* parent, const char* desc)
{
	ui_component_init(&self->component, parent);
	ui_label_init(&self->desclabel, &self->component);
	ui_slider_init(&self->slider, &self->component);	
	ui_slider_setrange(&self->slider, -32768, 32767);
	ui_label_init(&self->label, &self->component);		
	ui_label_settext(&self->desclabel, desc);
	ui_label_settext(&self->label, "-");	
	signal_connect(&self->component.signal_destroy, self, OnDestroy);
	signal_connect(&self->component.signal_size, self, OnSize);
	signal_connect(&self->slider.signal_changed, self, OnSliderChanged);
	signal_init(&self->signal_describevalue);
}

void OnDestroy(SliderGroup* self)
{
	signal_dispose(&self->signal_describevalue);
}

void OnSize(SliderGroup* self, ui_component* sender, int width, int height)
{	
	int descwidth = 100;
	int lblwidth = 70;
	int sliderwidth = width - descwidth - lblwidth - 15;
	ui_component_resize(&self->desclabel.component, descwidth, height);
	ui_component_resize(&self->slider.component, sliderwidth, height);
	ui_component_resize(&self->label.component, lblwidth, height);
	ui_component_move(&self->slider.component, descwidth + 5, 0);
	ui_component_move(&self->label.component, width - lblwidth - 5, 0);
}

void OnSliderChanged(SliderGroup* self, ui_component* sender)
{
	char buffer[20];
	buffer[0] = '\0';
	signal_emit(&self->signal_describevalue, self, 1, buffer);
	if (buffer[0] == '\0') {
	  _snprintf(buffer, 20, "%f", SliderGroupValue(self));	  
	}
	ui_label_settext(&self->label, buffer);
}

float SliderGroupValue(SliderGroup* self)
{	
	return (ui_slider_value(&self->slider) + 32768) / 65535.f;
}
// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "slidergroup.h"
#include <stdio.h>

static void OnDestroy(SliderGroup*);
static void OnSize(SliderGroup*, psy_ui_Component* sender, ui_size*);
static void OnSliderChanged(SliderGroup*, psy_ui_Component* sender);
static void DescribeValue(SliderGroup*);
static float Value(SliderGroup*);
static void OnTimer(SliderGroup*, psy_ui_Component* sender, int timerid);

static int timerid = 600;

void InitSliderGroup(SliderGroup* self, psy_ui_Component* parent, const char* desc)
{
	ui_component_init(&self->component, parent);	
	ui_label_init(&self->desclabel, &self->component);
	ui_slider_init(&self->slider, &self->component);		
	ui_label_init(&self->label, &self->component);		
	ui_label_settext(&self->desclabel, desc);
	ui_label_settext(&self->label, "-");	
	psy_signal_connect(&self->component.signal_destroy, self, OnDestroy);
	psy_signal_connect(&self->component.signal_timer, self, OnTimer);
	psy_signal_connect(&self->component.signal_size, self, OnSize);
	psy_signal_connect(&self->slider.signal_changed, self, OnSliderChanged);
	psy_signal_init(&self->signal_value);
	psy_signal_init(&self->signal_describevalue);
	psy_signal_init(&self->signal_tweakvalue);	
	ui_component_starttimer(&self->component, timerid, 50);
}

void OnDestroy(SliderGroup* self)
{
	psy_signal_dispose(&self->signal_value);
	psy_signal_dispose(&self->signal_describevalue);
	psy_signal_dispose(&self->signal_tweakvalue);
}

void OnSize(SliderGroup* self, psy_ui_Component* sender, ui_size* size)
{	
	int descwidth = 100;
	int lblwidth = 70;
	int sliderwidth = size->width - descwidth - lblwidth - 15;
	ui_component_resize(&self->desclabel.component, descwidth, size->height);
	ui_component_resize(&self->slider.component, sliderwidth, size->height);
	ui_component_resize(&self->label.component, lblwidth, size->height);
	ui_component_move(&self->slider.component, descwidth + 5, 0);
	ui_component_move(&self->label.component, size->width - lblwidth - 5, 0);
}

void OnSliderChanged(SliderGroup* self, psy_ui_Component* sender)
{	
	signal_emit_float(&self->signal_tweakvalue, self, Value(self));
	DescribeValue(self);
}

float Value(SliderGroup* self)
{	
	return (float)ui_slider_value(&self->slider);
}

void DescribeValue(SliderGroup* self)
{
	char buffer[20];
	buffer[0] = '\0';
	psy_signal_emit(&self->signal_describevalue, self, 2, buffer);
	if (buffer[0] == '\0') {
	  _snprintf(buffer, 20, "%f", Value(self));	  
	}
	ui_label_settext(&self->label, buffer);
}

void OnTimer(SliderGroup* self, psy_ui_Component* sender, int timerid)
{
	float value = 0;
	float* pValue;	

	pValue = &value;
	psy_signal_emit(&self->signal_value, self, 1, pValue);
	if (Value(self) != value) {		
		ui_slider_setvalue(&self->slider, value);
		DescribeValue(self);
	}
}

void SliderGroupConnect(SliderGroup* self, void* context, SliderGroupDescribe describe,
	SliderGroupTweak tweak, SliderGroupValue value)
{
	signal_connect(&self->signal_describevalue, context, describe);
	signal_connect(&self->signal_tweakvalue, context, tweak);
	signal_connect(&self->signal_value, context, value);
}

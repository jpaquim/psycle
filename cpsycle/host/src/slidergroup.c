// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#include "slidergroup.h"
#include <stdio.h>

static void OnDestroy(SliderGroup*);
static void OnSize(SliderGroup*, ui_component* sender, int width, int height);
static void OnSliderChanged(SliderGroup*, ui_component* sender);
static void DescribeValue(SliderGroup*);
static float Value(SliderGroup* self);
static void OnTimer(SliderGroup* self, ui_component* sender, int timerid);

static int timerid = 600;

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
	signal_connect(&self->component.signal_timer, self, OnTimer);
	signal_connect(&self->component.signal_size, self, OnSize);
	signal_connect(&self->slider.signal_changed, self, OnSliderChanged);	
	signal_init(&self->signal_describevalue);
	signal_init(&self->signal_tweakvalue);	
	SetTimer(self->component.hwnd, timerid, 50, 0);
}

void OnDestroy(SliderGroup* self)
{
	signal_dispose(&self->signal_describevalue);
	signal_dispose(&self->signal_tweakvalue);
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
	signal_emit_float(&self->signal_tweakvalue, self, Value(self));
	DescribeValue(self);
}

float Value(SliderGroup* self)
{	
	return (ui_slider_value(&self->slider) + 32768) / 65535.f;
}

void DescribeValue(SliderGroup* self)
{
	char buffer[20];
	buffer[0] = '\0';
	signal_emit(&self->signal_describevalue, self, 2, buffer);
	if (buffer[0] == '\0') {
	  _snprintf(buffer, 20, "%f", Value(self));	  
	}
	ui_label_settext(&self->label, buffer);
}

void OnTimer(SliderGroup* self, ui_component* sender, int timerid)
{
	float value = 0;
	float* pValue;
	int intvalue;

	pValue = &value;
	signal_emit(&self->signal_value, self, 1, pValue);

	if (Value(self) != value) {
		intvalue = (int)((value * 65535.f) - 32768.f);
		ui_slider_setvalue(&self->slider, intvalue);
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
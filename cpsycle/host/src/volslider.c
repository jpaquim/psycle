// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#include "volslider.h"
#include <stdio.h>
#include <math.h>

static void OnDestroy(VolSlider*);
static void OnSize(VolSlider*, ui_component* sender, int width, int height);
static void OnSliderChanged(VolSlider*, ui_component* sender);
static void DescribeValue(VolSlider*);
static float Value(VolSlider* self);
static void OnTimer(VolSlider* self, ui_component* sender, int timerid);
static void SetSliderValue(VolSlider* slider, float value);
static void OnSongChanged(VolSlider* self, Workspace* workspace);

static int timerid = 700;

void InitVolSlider(VolSlider* self, ui_component* parent, Workspace* workspace)
{		
	self->machines = &workspace->song->machines;
	ui_component_init(&self->component, parent);	
	ui_slider_init(&self->slider, &self->component);
	ui_component_setbackgroundmode(&self->component, BACKGROUND_SET);
	ui_slider_setrange(&self->slider, -32768, 32767);
	signal_connect(&self->slider.signal_changed, self, OnSliderChanged);
	signal_connect(&self->component.signal_timer, self, OnTimer);	
	signal_connect(&self->component.signal_size, self, OnSize);
	signal_connect(&workspace->signal_songchanged, self, OnSongChanged);
	SetTimer(self->component.hwnd, timerid, 50, 0);
}

void OnDestroy(VolSlider* self)
{	
}

void OnSize(VolSlider* self, ui_component* sender, int width, int height)
{			
	ui_component_resize(&self->slider.component, width, height);	
}

float Value(VolSlider* self)
{	
	return (ui_slider_value(&self->slider) + 32768) / 65535.f;
}

void OnTimer(VolSlider* self, ui_component* sender, int timerid)
{		
	if (self->machines) {
		SetSliderValue(self, (float) 
			sqrt(machines_volume(self->machines)) * 0.5f);
	}
}

void OnSliderChanged(VolSlider* self, ui_component* sender)
{	
	if (self->machines) {
		machines_setvolume(self->machines, Value(self) * Value(self) * 4.f);
	}
}

void SetSliderValue(VolSlider* self, float value)
{
	if (Value(self) != value) {
		int intvalue;

		intvalue = (int)((value * 65535.f) - 32768.f);
		ui_slider_setvalue(&self->slider, intvalue);
	}
}

void OnSongChanged(VolSlider* self, Workspace* workspace)
{
	self->machines = &workspace->song->machines;
}

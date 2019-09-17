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

static int timerid = 700;

void InitVolSlider(VolSlider* self, ui_component* parent, Player* player)
{	
	self->player = player;	
	ui_component_init(&self->component, parent);	
	ui_slider_init(&self->slider, &self->component);	
	ui_slider_setrange(&self->slider, -32768, 32767);
	signal_connect(&self->slider.signal_changed, self, OnSliderChanged);
	signal_connect(&self->component.signal_timer, self, OnTimer);
	SetTimer(self->component.hwnd, timerid, 50, 0);
	signal_connect(&self->component.signal_size, self, OnSize);
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
	if (self->player) {
		SetSliderValue(self, (float) sqrt(player_volume(self->player)) * 0.5f);
	}
}

void OnSliderChanged(VolSlider* self, ui_component* sender)
{	
	if (self->player) {
		player_setvolume(self->player, Value(self) * Value(self) * 4.f);
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

// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#if !defined(SLIDERGROUP_H)
#define SLIDERGROUP_H

#include <uilabel.h>
#include <uislider.h>

typedef struct {
	psy_ui_Component component;
	ui_label desclabel;
	ui_slider slider;
	ui_label label;	
	psy_Signal signal_describevalue;
	psy_Signal signal_tweakvalue;
	psy_Signal signal_value;
} SliderGroup;

typedef void (*SliderGroupDescribe)(void* context, SliderGroup*, char* txt);
typedef void (*SliderGroupTweak)(void* context, SliderGroup*, float value);
typedef void (*SliderGroupValue)(void* context, SliderGroup*, float* value);

void InitSliderGroup(SliderGroup*, psy_ui_Component* parent, const char* desc);
void SliderGroupConnect(SliderGroup*, void* context, SliderGroupDescribe,
	SliderGroupTweak, SliderGroupValue);

#endif

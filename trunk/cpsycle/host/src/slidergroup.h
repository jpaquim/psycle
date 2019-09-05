// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#if !defined(SLIDERGROUP_H)
#define SLIDERGROUP_H

#include <uilabel.h>
#include <uislider.h>

typedef struct {
	ui_component component;
	ui_label desclabel;
	ui_slider slider;
	ui_label label;	
	Signal signal_describevalue;
} SliderGroup;

void InitSliderGroup(SliderGroup*, ui_component* parent, const char* desc);
float SliderGroupValue(SliderGroup*);

#endif
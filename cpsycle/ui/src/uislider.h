// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#if !defined(UISLIDER_H)
#define UISLIDER_H

#include "uicomponent.h"

typedef struct {
	psy_ui_Component component;	
	double value;
	double rulerstep;
	int tweakbase;
	char label[128];
	char valuedescription[128];
	int labelsize;
	int valuelabelsize;
	int margin;	
	int charnumber;
	psy_ui_Orientation orientation;
	psy_Signal signal_clicked;
	psy_Signal signal_changed;
	psy_Signal signal_describevalue;
	psy_Signal signal_tweakvalue;
	psy_Signal signal_value;	
} psy_ui_Slider;

typedef void (*ui_slider_fpdescribe)(void*, psy_ui_Slider*, char* txt);
typedef void (*ui_slider_fptweak)(void*, psy_ui_Slider*, float value);
typedef void (*ui_slider_fpvalue)(void*, psy_ui_Slider*, float* value);

void ui_slider_init(psy_ui_Slider*, psy_ui_Component* parent);
void ui_slider_settext(psy_ui_Slider*, const char* text);
void ui_slider_setcharnumber(psy_ui_Slider*, int number);
void ui_slider_setvalue(psy_ui_Slider*, double value);
double ui_slider_value(psy_ui_Slider*);
void ui_slider_showvertical(psy_ui_Slider*);
void ui_slider_showhorizontal(psy_ui_Slider*);
psy_ui_Orientation ui_slider_orientation(psy_ui_Slider*);
void ui_slider_connect(psy_ui_Slider*, void* context, ui_slider_fpdescribe,
	ui_slider_fptweak, ui_slider_fpvalue);

#endif

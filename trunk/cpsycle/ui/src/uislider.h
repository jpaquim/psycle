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
	UiOrientation orientation;
	psy_Signal signal_clicked;
	psy_Signal signal_changed;
	psy_Signal signal_describevalue;
	psy_Signal signal_tweakvalue;
	psy_Signal signal_value;	
} ui_slider;

typedef void (*ui_slider_fpdescribe)(void*, ui_slider*, char* txt);
typedef void (*ui_slider_fptweak)(void*, ui_slider*, float value);
typedef void (*ui_slider_fpvalue)(void*, ui_slider*, float* value);

void ui_slider_init(ui_slider*, psy_ui_Component* parent);
void ui_slider_settext(ui_slider*, const char* text);
void ui_slider_setcharnumber(ui_slider*, int number);
void ui_slider_setvalue(ui_slider*, double value);
double ui_slider_value(ui_slider*);
void ui_slider_showvertical(ui_slider*);
void ui_slider_showhorizontal(ui_slider*);
UiOrientation ui_slider_orientation(ui_slider*);
void ui_slider_connect(ui_slider*, void* context, ui_slider_fpdescribe,
	ui_slider_fptweak, ui_slider_fpvalue);

#endif

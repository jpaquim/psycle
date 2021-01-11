// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#ifndef psy_ui_SLIDER_H
#define psy_ui_SLIDER_H

#include "uicomponent.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	psy_ui_Component component;	
	double value;
	double rulerstep;
	double tweakbase;
	char* label;
	char* translation;
	char valuedescription[128];	
	float step;
	double labelsize;
	double slidersize;
	double valuelabelsize;
	intptr_t margin;
	double charnumber;
	double valuecharnumber;
	bool labelvisible;
	bool valuelabelvisible;
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

void psy_ui_slider_init(psy_ui_Slider*, psy_ui_Component* parent);
void psy_ui_slider_init_text(psy_ui_Slider*, psy_ui_Component* parent,
	const char* text);
void psy_ui_slider_settext(psy_ui_Slider*, const char* text);
void psy_ui_slider_setcharnumber(psy_ui_Slider*, double number);
void psy_ui_slider_setvaluecharnumber(psy_ui_Slider*, double number);
void psy_ui_slider_setvalue(psy_ui_Slider*, double value);
double psy_ui_slider_value(psy_ui_Slider*);
void psy_ui_slider_showvertical(psy_ui_Slider*);
void psy_ui_slider_showhorizontal(psy_ui_Slider*);
void psy_ui_slider_showlabel(psy_ui_Slider*);
void psy_ui_slider_hidelabel(psy_ui_Slider*);
void psy_ui_slider_showvaluelabel(psy_ui_Slider*);
void psy_ui_slider_hidevaluelabel(psy_ui_Slider*);
psy_ui_Orientation psy_ui_slider_orientation(psy_ui_Slider*);
void psy_ui_slider_connect(psy_ui_Slider*, void* context, ui_slider_fpdescribe,
	ui_slider_fptweak, ui_slider_fpvalue);

INLINE psy_ui_Component* psy_ui_slider_base(psy_ui_Slider* self)
{
	return &self->component;
}

#ifdef __cplusplus
}
#endif

#endif /* psy_ui_SLIDER_H */

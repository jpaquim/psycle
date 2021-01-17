// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#ifndef psy_ui_SLIDER_H
#define psy_ui_SLIDER_H

#include "uicomponent.h"
#include "uilabel.h"

#ifdef __cplusplus
extern "C" {
#endif

struct psy_ui_Slider;

typedef struct {
	psy_ui_Component component;
	double value;
	double rulerstep;
	double tweakbase;	
	float step;	
	double slidersize;
	bool poll;
	uintptr_t timerinterval;
	psy_ui_Orientation orientation;
	struct psy_ui_Slider* slider;
	psy_Signal signal_clicked;
	psy_Signal signal_changed;
	psy_Signal signal_describevalue;
	psy_Signal signal_tweakvalue;
	psy_Signal signal_value;
} psy_ui_SliderPane;

typedef void (*ui_slider_fpdescribe)(void*, struct psy_ui_Slider*, char* txt);
typedef void (*ui_slider_fptweak)(void*, struct psy_ui_Slider*, float value);
typedef void (*ui_slider_fpvalue)(void*, struct psy_ui_Slider*, float* value);

void psy_ui_sliderpane_init(psy_ui_SliderPane*, psy_ui_Component* parent);
void psy_ui_sliderpane_setvalue(psy_ui_SliderPane*, double value);
double psy_ui_sliderpane_value(psy_ui_SliderPane*);
void psy_ui_sliderpane_showvertical(psy_ui_SliderPane*);
void psy_ui_sliderpane_showhorizontal(psy_ui_SliderPane*);
psy_ui_Orientation psy_ui_sliderpane_orientation(psy_ui_SliderPane*);
void psy_ui_sliderpane_connect(psy_ui_SliderPane*, void* context, ui_slider_fpdescribe,
	ui_slider_fptweak, ui_slider_fpvalue);

INLINE psy_ui_Component* psy_ui_sliderpane_base(psy_ui_SliderPane* self)
{
	return &self->component;
}

typedef struct psy_ui_Slider {
	psy_ui_Component component;
	psy_ui_Label desc;
	psy_ui_SliderPane pane;
	psy_ui_Label value;
} psy_ui_Slider;

void psy_ui_slider_init(psy_ui_Slider*, psy_ui_Component* parent);
void psy_ui_slider_init_text(psy_ui_Slider*, psy_ui_Component* parent,
	const char* text);

void psy_ui_slider_connect(psy_ui_Slider*, void* context, ui_slider_fpdescribe,
	ui_slider_fptweak, ui_slider_fpvalue);
void psy_ui_slider_settext(psy_ui_Slider*, const char* text);
void psy_ui_slider_setcharnumber(psy_ui_Slider*, double number);
void psy_ui_slider_setvaluecharnumber(psy_ui_Slider*, double number);
void psy_ui_slider_showvertical(psy_ui_Slider*);
void psy_ui_slider_showhorizontal(psy_ui_Slider*);
void psy_ui_slider_showlabel(psy_ui_Slider*);
void psy_ui_slider_hidelabel(psy_ui_Slider*);
void psy_ui_slider_showvaluelabel(psy_ui_Slider*);
void psy_ui_slider_hidevaluelabel(psy_ui_Slider*);
void psy_ui_slider_describevalue(psy_ui_Slider*);
void psy_ui_slider_startpoll(psy_ui_Slider*);
void psy_ui_slider_stoppoll(psy_ui_Slider*);
void psy_ui_slider_update(psy_ui_Slider*);

INLINE psy_ui_Component* psy_ui_slider_base(psy_ui_Slider* self)
{
	return &self->component;
}


#ifdef __cplusplus
}
#endif

#endif /* psy_ui_SLIDER_H */

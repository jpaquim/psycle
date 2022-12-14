/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "uislider.h"
/* local */
#include "uiapp.h"
/* std */
#include <math.h>
/* platform */
#include "../../detail/portable.h"


/* psy_ui_SliderPane */

/* prototypes */
static void psy_ui_sliderpane_on_destroyed(psy_ui_SliderPane*);
static void psy_ui_sliderpane_initsignals(psy_ui_SliderPane*);
static void psy_ui_sliderpane_disposesignals(psy_ui_SliderPane*);
static void psy_ui_sliderpane_ondraw(psy_ui_SliderPane*, psy_ui_Graphics*);
static void psy_ui_sliderpane_drawverticalruler(psy_ui_SliderPane*,
	psy_ui_Graphics*);
static void psy_ui_sliderpane_onalign(psy_ui_SliderPane*);
static void psy_ui_sliderpane_on_mouse_down(psy_ui_SliderPane*,
	psy_ui_MouseEvent*);
static void psy_ui_sliderpane_on_mouse_up(psy_ui_SliderPane*,
	psy_ui_MouseEvent*);
static void psy_ui_sliderpane_onmousemove(psy_ui_SliderPane*,
	psy_ui_MouseEvent*);
static void psy_ui_sliderpane_on_mouse_wheel(psy_ui_SliderPane*,
	psy_ui_MouseEvent*);
static void psy_ui_sliderpane_onmouseenter(psy_ui_SliderPane*);
static void psy_ui_sliderpane_onmouseleave(psy_ui_SliderPane*);
static void psy_ui_sliderpane_on_timer(psy_ui_SliderPane*, uintptr_t timerid);
static void psy_ui_sliderpane_updatevalue(psy_ui_SliderPane*);
static void psy_ui_sliderpane_describevalue(psy_ui_SliderPane*);
static void psy_ui_sliderpane_onpreferredsize(psy_ui_SliderPane*,
	psy_ui_Size* limit, psy_ui_Size* rv);
static psy_ui_RealRectangle psy_ui_sliderpane_sliderposition(
	const psy_ui_SliderPane*);

/* vtable */
static psy_ui_ComponentVtable vtable;
static bool vtable_initialized = FALSE;

static void vtable_init(psy_ui_SliderPane* self)
{
	if (!vtable_initialized) {
		vtable = *(self->component.vtable);
		vtable.on_destroyed =
			(psy_ui_fp_component)
			psy_ui_sliderpane_on_destroyed;
		vtable.ondraw =
			(psy_ui_fp_component_ondraw)
			psy_ui_sliderpane_ondraw;		
		vtable.onpreferredsize =
			(psy_ui_fp_component_on_preferred_size)
			psy_ui_sliderpane_onpreferredsize;
		vtable.on_mouse_down =
			(psy_ui_fp_component_on_mouse_event)
			psy_ui_sliderpane_on_mouse_down;
		vtable.onalign =
			(psy_ui_fp_component)
			psy_ui_sliderpane_onalign;
		vtable.on_mouse_move =
			(psy_ui_fp_component_on_mouse_event)
			psy_ui_sliderpane_onmousemove;
		vtable.on_mouse_wheel =
			(psy_ui_fp_component_on_mouse_event)
			psy_ui_sliderpane_on_mouse_wheel;
		vtable.on_mouse_up =
			(psy_ui_fp_component_on_mouse_event)
			psy_ui_sliderpane_on_mouse_up;
		vtable.onmouseenter =
			(psy_ui_fp_component)
			psy_ui_sliderpane_onmouseenter;
		vtable.onmouseleave = 
			(psy_ui_fp_component)
			psy_ui_sliderpane_onmouseleave;
		vtable.on_timer =
			(psy_ui_fp_component_on_timer)
			psy_ui_sliderpane_on_timer;
		vtable_initialized = TRUE;
	}
	psy_ui_component_set_vtable(&self->component, &vtable);
}

/* implementation */
void psy_ui_sliderpane_init(psy_ui_SliderPane* self, psy_ui_Component* parent)
{	
	assert(self);
	
	psy_ui_component_init(&self->component, parent, NULL);
	vtable_init(self);	
	self->slider = NULL;	
	self->tweakbase = -1;
	self->orientation = psy_ui_HORIZONTAL;
	self->value = 0.0;
	self->defaultvalue = 0.5;
	self->rulerstep = 0.1;
	self->wheelstep = 0.1;
	self->hslidersize = psy_ui_size_make_em(1.0, 1.0);
	self->vslidersize = psy_ui_size_make_em(1.0, 1.0);	
	self->slidersizepx = psy_ui_realsize_make(6.0, 6.0);
	self->poll = FALSE;
	self->hover = FALSE;
	psy_ui_sliderpane_initsignals(self);	
	psy_signal_connect(&self->component.signal_timer, self,
		psy_ui_sliderpane_on_timer);
	psy_ui_component_set_style_types(&self->component,
		psy_ui_STYLE_SLIDERPANE, psy_INDEX_INVALID, psy_INDEX_INVALID,
		psy_INDEX_INVALID);
}

void psy_ui_sliderpane_initsignals(psy_ui_SliderPane* self)
{
	psy_signal_init(&self->signal_clicked);
	psy_signal_init(&self->signal_changed);
	psy_signal_init(&self->signal_describevalue);
	psy_signal_init(&self->signal_tweakvalue);
	psy_signal_init(&self->signal_value);
	psy_signal_init(&self->signal_customdraw);
}

void psy_ui_sliderpane_on_destroyed(psy_ui_SliderPane* self)
{	
	psy_ui_sliderpane_disposesignals(self);
}

void psy_ui_sliderpane_disposesignals(psy_ui_SliderPane* self)
{
	psy_signal_dispose(&self->signal_clicked);
	psy_signal_dispose(&self->signal_changed);
	psy_signal_dispose(&self->signal_describevalue);
	psy_signal_dispose(&self->signal_tweakvalue);
	psy_signal_dispose(&self->signal_value);
	psy_signal_dispose(&self->signal_customdraw);
}

void psy_ui_sliderpane_setvalue(psy_ui_SliderPane* self, double value)
{
	self->value = value;
}

void psy_ui_sliderpane_setdefaultvalue(psy_ui_SliderPane* self, double value)
{
	self->defaultvalue = value;
}

double psy_ui_sliderpane_value(psy_ui_SliderPane* self)
{
	return self->value;
}

void psy_ui_sliderpane_ondraw(psy_ui_SliderPane* self, psy_ui_Graphics* g)
{	
	psy_ui_RealSize size;
	int styletype;

	psy_signal_emit(&self->signal_customdraw, self, 1, g);

	if (self->orientation == psy_ui_VERTICAL) {												
		psy_ui_sliderpane_drawverticalruler(self, g);		
	}
	if (self->hover) {
		styletype = psy_ui_STYLE_SLIDERTHUMB_HOVER;
	} else {
		styletype = psy_ui_STYLE_SLIDERTHUMB;
	}	
	psy_ui_drawsolidrectangle(g, psy_ui_sliderpane_sliderposition(self),
		psy_ui_style_const(styletype)->background.colour);
	size = psy_ui_component_size_px(&self->component);
	psy_ui_drawborder(g,
		psy_ui_realrectangle_make(psy_ui_realpoint_zero(), size),
		&psy_ui_style_const(psy_ui_STYLE_SLIDERPANE)->border,
		psy_ui_component_textmetric(&self->component));
}

void psy_ui_sliderpane_drawverticalruler(psy_ui_SliderPane* self,
	psy_ui_Graphics* g)
{
	double step = 0;
	double markwidth = 5;
	psy_ui_RealSize size;

	psy_ui_setcolour(g, psy_ui_style_const(psy_ui_STYLE_ROOT)->border.top.colour);
	size = psy_ui_component_size_px(&self->component);
	for (step = 0; step <= 1.0; step += self->rulerstep) {
		double cpy;

		cpy = step * size.height;
		psy_ui_drawline(g, psy_ui_realpoint_make(0, cpy),
			psy_ui_realpoint_make(markwidth, cpy));
		psy_ui_drawline(g, psy_ui_realpoint_make(size.width - markwidth, cpy),
			psy_ui_realpoint_make(size.width, cpy));
	}
}

void psy_ui_sliderpane_onalign(psy_ui_SliderPane* self)
{	
	self->slidersizepx = psy_ui_size_px(
		(self->orientation == psy_ui_HORIZONTAL)
			? &self->hslidersize
			: &self->vslidersize,
		psy_ui_component_textmetric(psy_ui_sliderpane_base(self)), NULL);
}

void psy_ui_sliderpane_on_mouse_down(psy_ui_SliderPane* self,
	psy_ui_MouseEvent* ev)
{
	if (psy_ui_mouseevent_button(ev) == 1) {
		psy_ui_RealSize size;

		size = psy_ui_component_scroll_size_px(&self->component);
		if (self->orientation == psy_ui_HORIZONTAL) {
			self->tweakbase = (psy_ui_mouseevent_pt(ev).x) -
				self->value * (size.width - self->slidersizepx.width);
		} else if (self->orientation == psy_ui_VERTICAL) {
			self->tweakbase = psy_ui_mouseevent_pt(ev).y -
				((1.0 - self->value) * (size.height -
					self->slidersizepx.width));
		}
		if (self->poll) {
			psy_ui_component_stop_timer(&self->component, 0);
		}
		psy_ui_component_capture(&self->component);
	} else {
		float value;
		float* pvalue;

		self->value = self->defaultvalue;
		psy_signal_emit_float(&self->signal_tweakvalue,
			self->slider, (float)self->value);
		value = (float)self->value;
		pvalue = &value;
		if (self->slider) {
			psy_signal_emit(&self->signal_value, self->slider, 1, pvalue);
		}
		if (self->value != value) {
			self->value = value;
			if (self->slider) {
				psy_signal_emit(&self->signal_changed, self->slider, 0);
			}
		}		
		psy_ui_sliderpane_describevalue(self);
		psy_ui_component_invalidate(&self->component);
	}
}

void psy_ui_sliderpane_onmousemove(psy_ui_SliderPane* self, psy_ui_MouseEvent* ev)
{
	if (self->tweakbase != -1) {
		psy_ui_RealSize size;
		float value;
		float* pvalue;		
		
		size = psy_ui_component_scroll_size_px(&self->component);
		if (self->orientation == psy_ui_HORIZONTAL) {			
			self->value = psy_max(0.0, psy_min(1.0,
				(psy_ui_mouseevent_pt(ev).x - self->tweakbase) /
				(size.width - self->slidersizepx.width)));
		} else {			
			self->value = psy_max(0.0, psy_min(1.0,
				1.0 - (psy_ui_mouseevent_pt(ev).y - self->tweakbase) /
				(size.height - self->slidersizepx.width)));
		}
		if (self->slider) {
			psy_signal_emit_float(&self->signal_tweakvalue,
				self->slider, (float)self->value);
		}
		pvalue = &value;
		value = 0.f;
		if (self->slider) {
			psy_signal_emit(&self->signal_value, self->slider, 1, pvalue);
		}
		if (self->value != value) {
			self->value = value;
			if (self->slider) {
				psy_signal_emit(&self->signal_changed, self->slider, 0);
			}
		}				
		psy_ui_sliderpane_describevalue(self);
		psy_ui_component_invalidate(&self->component);
	}
}

void psy_ui_sliderpane_on_mouse_up(psy_ui_SliderPane* self,
	psy_ui_MouseEvent* ev)
{
	if (self->tweakbase != -1) {
		self->tweakbase = -1;
		psy_ui_component_release_capture(&self->component);
		if (self->poll) {
			psy_ui_component_start_timer(&self->component, 0, 50);
		}
	}
}

void psy_ui_sliderpane_on_mouse_wheel(psy_ui_SliderPane* self,
	psy_ui_MouseEvent* ev)
{
	if (psy_ui_mouseevent_delta(ev) != 0) {
		if (psy_ui_mouseevent_delta(ev) > 0) {
			self->value += self->wheelstep;
		} else {
			self->value -= self->wheelstep;
		}
		self->value = psy_max(0.0, psy_min(1.0, self->value));
		if (self->slider) {
			psy_signal_emit_float(&self->signal_tweakvalue, self->slider,
				(float)self->value);
			psy_signal_emit(&self->signal_changed, self->slider, 0);
		}
		psy_ui_component_invalidate(&self->component);
		psy_ui_sliderpane_describevalue(self);
	}
	psy_ui_mouseevent_stop_propagation(ev);
}

void psy_ui_sliderpane_onmouseenter(psy_ui_SliderPane* self)
{
	self->hover = TRUE;
	psy_ui_component_invalidate(&self->component);
}

void psy_ui_sliderpane_onmouseleave(psy_ui_SliderPane* self)
{
	if (self->hover != FALSE) {
		self->hover = FALSE;
		psy_ui_component_invalidate(&self->component);
	}
}

void psy_ui_sliderpane_showvertical(psy_ui_SliderPane* self)
{
	self->orientation = psy_ui_VERTICAL;
}

void psy_ui_sliderpane_showhorizontal(psy_ui_SliderPane* self)
{
	self->orientation = psy_ui_HORIZONTAL;
}

psy_ui_Orientation psy_ui_sliderpane_orientation(psy_ui_SliderPane* self)
{
	return self->orientation;
}

void psy_ui_sliderpane_on_timer(psy_ui_SliderPane* self, uintptr_t timerid)
{	
	if (psy_ui_component_draw_visible(psy_ui_sliderpane_base(self))) {
		psy_ui_sliderpane_updatevalue(self);
		psy_ui_sliderpane_describevalue(self);
	}
}

void psy_ui_sliderpane_updatevalue(psy_ui_SliderPane* self)
{
	float value = 0;
	float* pvalue;

	pvalue = &value;
	if (self->slider) {
		psy_signal_emit(&self->signal_value, self->slider, 1, pvalue);
	}
	if (self->value != value) {
		self->value = value;
#ifndef PSYCLE_DEBUG_PREVENT_TIMER_DRAW
		psy_ui_component_invalidate(&self->component);
#endif
	}
}

void psy_ui_sliderpane_connect(psy_ui_SliderPane* self, void* context,
	ui_slider_fpdescribe fp_describe, ui_slider_fptweak fp_tweak,
	ui_slider_fpvalue fp_value)
{
	if (fp_describe) {
		psy_signal_connect(&self->signal_describevalue, context, fp_describe);
	}
	if (fp_tweak) {
		psy_signal_connect(&self->signal_tweakvalue, context, fp_tweak);
	}
	if (fp_value) {
		psy_signal_connect(&self->signal_value, context, fp_value);
	}
}

void psy_ui_sliderpane_describevalue(psy_ui_SliderPane* self)
{
	if (self->slider) {
		psy_ui_slider_describevalue(self->slider);
	}
}

void psy_ui_sliderpane_onpreferredsize(psy_ui_SliderPane* self,
	psy_ui_Size* limit, psy_ui_Size* rv)
{	
	if (self->orientation == psy_ui_HORIZONTAL) {
		rv->width = psy_ui_value_make_ew(20);
		rv->height = psy_ui_value_make_eh(1.3);
	} else {
		rv->width = psy_ui_value_make_ew(4.0);
		rv->height = psy_ui_value_make_eh(20.0);
	}
}

psy_ui_RealRectangle psy_ui_sliderpane_sliderposition(
	const psy_ui_SliderPane* self)
{
	psy_ui_RealSize size;

	size = psy_ui_component_scroll_size_px(&self->component);
	if (self->orientation == psy_ui_HORIZONTAL) {
		return psy_ui_realrectangle_make(psy_ui_realpoint_make(
			floor((size.width - self->slidersizepx.width) * self->value), 2.0),
			psy_ui_realsize_make(self->slidersizepx.width, size.height - 4));
	}
	return psy_ui_realrectangle_make(psy_ui_realpoint_make(2.0,
		floor(((size.height - self->slidersizepx.height) * (1 - self->value)))),
		psy_ui_realsize_make(size.width - 4, self->slidersizepx.height));
}

/* psy_ui_Slider */

/* implementation */
void psy_ui_slider_init(psy_ui_Slider* self, psy_ui_Component* parent)
{
	assert(self);
	
	psy_ui_component_init(&self->component, parent, NULL);
	psy_ui_label_init(&self->desc, &self->component);	
	psy_ui_sliderpane_init(&self->pane, &self->component);
	self->pane.slider = self;	
	psy_ui_component_set_align(psy_ui_sliderpane_base(&self->pane),
		psy_ui_ALIGN_CLIENT);
	psy_ui_label_init(&self->value, &self->component);
	psy_ui_label_prevent_translation(&self->value);	
	psy_ui_slider_show_horizontal(self);
}

void psy_ui_slider_init_text(psy_ui_Slider* self, psy_ui_Component* parent,
	const char* text)
{
	assert(self);
	
	psy_ui_slider_init(self, parent);
	psy_ui_slider_set_text(self, text);
}

void psy_ui_slider_connect(psy_ui_Slider* self, void* context,
	ui_slider_fpdescribe fp_describe, ui_slider_fptweak fp_tweak,
	ui_slider_fpvalue fp_value)
{
	assert(self);
	
	psy_ui_sliderpane_connect(&self->pane, context,
		fp_describe, fp_tweak, fp_value);	
}

void psy_ui_slider_set_text(psy_ui_Slider* self, const char* text)
{
	assert(self);
	
	psy_ui_label_set_text(&self->desc, text);
}

void psy_ui_slider_setcharnumber(psy_ui_Slider* self, double number)
{
	assert(self);
	
	psy_ui_label_set_char_number(&self->desc, number);
}

void psy_ui_slider_setvaluecharnumber(psy_ui_Slider* self, double number)
{
	assert(self);
	
	psy_ui_label_set_char_number(&self->value, number);
}

void psy_ui_slider_showvertical(psy_ui_Slider* self)
{
	assert(self);
	
	psy_ui_sliderpane_showvertical(&self->pane);
	psy_ui_component_set_align(psy_ui_label_base(&self->desc),
		psy_ui_ALIGN_BOTTOM);
	psy_ui_component_set_align(psy_ui_label_base(&self->value),
		psy_ui_ALIGN_BOTTOM);	
	psy_ui_component_set_margin(psy_ui_sliderpane_base(&self->pane),
		psy_ui_margin_make_em(0.0, 2.0, 0.0, 2.0));		
}

void psy_ui_slider_show_horizontal(psy_ui_Slider* self)
{
	assert(self);
	
	psy_ui_sliderpane_showhorizontal(&self->pane);
	psy_ui_component_set_align(psy_ui_label_base(&self->desc),
		psy_ui_ALIGN_LEFT);
	psy_ui_component_set_align(psy_ui_label_base(&self->value),
		psy_ui_ALIGN_RIGHT);	
	psy_ui_component_set_margin(psy_ui_sliderpane_base(&self->pane),
		psy_ui_margin_make_em(0.0, 2.0, 0.0, 2.0));		
}

void psy_ui_slider_showlabel(psy_ui_Slider* self)
{
	assert(self);
	
	psy_ui_component_show(&self->desc.component);
}

void psy_ui_slider_hidelabel(psy_ui_Slider* self)
{
	assert(self);
	
	psy_ui_component_hide(&self->desc.component);
}

void psy_ui_slider_showvaluelabel(psy_ui_Slider* self)
{
	assert(self);
	
	psy_ui_component_show(&self->value.component);
}

void psy_ui_slider_hidevaluelabel(psy_ui_Slider* self)
{
	assert(self);
	
	psy_ui_component_hide(&self->value.component);
}

void psy_ui_slider_describevalue(psy_ui_Slider* self)
{
	char text[256];

	assert(self);
	
	text[0] = '\0';
	psy_signal_emit(&self->pane.signal_describevalue, self, 1, text);
	if (text == '\0') {
		psy_snprintf(text, 256, "%f", (float)self->pane.value);
	}
	psy_ui_label_set_text(&self->value, text);	
}

void psy_ui_slider_startpoll(psy_ui_Slider* self)
{
	assert(self);
	
	self->pane.poll = TRUE;	
	psy_ui_component_start_timer(&self->pane.component, 0, 50);
}

void psy_ui_slider_stoppoll(psy_ui_Slider* self)
{
	assert(self);
	
	self->pane.poll = FALSE;
	psy_ui_component_stop_timer(&self->pane.component, 0);
}

void psy_ui_slider_update(psy_ui_Slider* self)
{	
	assert(self);
	
	psy_ui_sliderpane_updatevalue(&self->pane);
	psy_ui_sliderpane_describevalue(&self->pane);	
}

void psy_ui_slider_setdefaultvalue(psy_ui_Slider* self, double value)
{
	assert(self);
	
	psy_ui_sliderpane_setdefaultvalue(&self->pane, value);
}

void psy_ui_slider_set_ruler_step(psy_ui_Slider* self, double value)
{
	assert(self);
	
	self->pane.rulerstep = value;
}

void psy_ui_slider_set_wheel_step(psy_ui_Slider* self, double value)
{
	assert(self);
	
	self->pane.wheelstep = value;
}

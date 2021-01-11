// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "uislider.h"
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "uiapp.h"
#include "../../detail/portable.h"

#ifndef max
#define max(a, b)  (((a) > (b)) ? (a) : (b)) 
#endif

static void psy_ui_slider_initsignals(psy_ui_Slider*);
static void psy_ui_slider_disposesignals(psy_ui_Slider*);
static void psy_ui_slider_ondraw(psy_ui_Slider*, psy_ui_Graphics*);
static void psy_ui_slider_drawverticalruler(psy_ui_Slider*, psy_ui_Graphics*);
static void psy_ui_slider_onmousedown(psy_ui_Slider*, psy_ui_MouseEvent*);
static void psy_ui_slider_onmouseup(psy_ui_Slider*, psy_ui_MouseEvent*);
static void psy_ui_slider_onmousemove(psy_ui_Slider*, psy_ui_MouseEvent*);
static void psy_ui_slider_onmousewheel(psy_ui_Slider*, psy_ui_MouseEvent*);
static void psy_ui_slider_ondestroy(psy_ui_Slider*, psy_ui_Component* sender);
static void psy_ui_slider_ontimer(psy_ui_Slider*, psy_ui_Component* sender,
	uintptr_t timerid);
static void psy_ui_slider_updatevalue(psy_ui_Slider*);
static void psy_ui_slider_describevalue(psy_ui_Slider*);
static void psy_ui_slider_onalign(psy_ui_Slider*);
static void psy_ui_slider_onpreferredsize(psy_ui_Slider*, psy_ui_Size* limit, psy_ui_Size* rv);
static void psy_ui_slider_onlanguagechanged(psy_ui_Slider*);

static psy_ui_ComponentVtable vtable;
static bool vtable_initialized = FALSE;

static void vtable_init(psy_ui_Slider* self)
{
	if (!vtable_initialized) {
		vtable = *(self->component.vtable);
		vtable.ondraw = (psy_ui_fp_component_ondraw) psy_ui_slider_ondraw;
		vtable.onalign = (psy_ui_fp_component_onalign) psy_ui_slider_onalign;
		vtable.onpreferredsize = (psy_ui_fp_component_onpreferredsize)
			psy_ui_slider_onpreferredsize;
		vtable.onmousedown = (psy_ui_fp_component_onmousedown)
			psy_ui_slider_onmousedown;
		vtable.onmousemove = (psy_ui_fp_component_onmousemove)
			psy_ui_slider_onmousemove;
		vtable.onmousewheel = (psy_ui_fp_component_onmousewheel)
			psy_ui_slider_onmousewheel;
		vtable.onmouseup = (psy_ui_fp_component_onmouseup)
			psy_ui_slider_onmouseup;
		vtable.onlanguagechanged = (psy_ui_fp_component_onlanguagechanged)
			psy_ui_slider_onlanguagechanged;
		vtable_initialized = TRUE;
	}
}

void psy_ui_slider_init(psy_ui_Slider* self, psy_ui_Component* parent)
{	
	psy_ui_component_init(&self->component, parent);
	vtable_init(self);
	self->component.vtable = &vtable;
	psy_ui_component_doublebuffer(&self->component);
	self->tweakbase = -1;
	self->orientation = psy_ui_HORIZONTAL;
	self->value = 0.0;
	self->labelsize = 100;
	self->label = NULL;
	self->slidersize = 6;
	self->valuelabelsize = 40;	
	self->valuedescription[0] = '\0';
	self->translation = NULL;
	self->margin = 5;
	self->rulerstep = 0.1;
	self->charnumber = 0;
	self->valuecharnumber = 8;
	self->labelvisible = TRUE;
	self->valuelabelvisible = TRUE;
	psy_ui_slider_initsignals(self);	
	psy_signal_connect(&self->component.signal_destroy, self, 
		psy_ui_slider_ondestroy);	
	psy_signal_connect(&self->component.signal_timer, self,
		psy_ui_slider_ontimer);
	psy_ui_component_starttimer(&self->component, 0, 50);
}

void psy_ui_slider_init_text(psy_ui_Slider* self, psy_ui_Component* parent,
	const char* text)
{
	psy_ui_slider_init(self, parent);
	psy_ui_slider_settext(self, text);
}

void psy_ui_slider_initsignals(psy_ui_Slider* self)
{
	psy_signal_init(&self->signal_clicked);
	psy_signal_init(&self->signal_changed);
	psy_signal_init(&self->signal_describevalue);
	psy_signal_init(&self->signal_tweakvalue);
	psy_signal_init(&self->signal_value);	
}

void psy_ui_slider_ondestroy(psy_ui_Slider* self, psy_ui_Component* sender)
{
	free(self->translation);		
	free(self->label);
	psy_ui_slider_disposesignals(self);
}

void psy_ui_slider_disposesignals(psy_ui_Slider* self)
{
	psy_signal_dispose(&self->signal_clicked);
	psy_signal_dispose(&self->signal_changed);
	psy_signal_dispose(&self->signal_describevalue);
	psy_signal_dispose(&self->signal_tweakvalue);
	psy_signal_dispose(&self->signal_value);
}

void psy_ui_slider_settext(psy_ui_Slider* self, const char* text)
{	
	psy_strreset(&self->label, text);
	psy_strreset(&self->translation, psy_ui_translate(text));
	psy_ui_component_invalidate(&self->component);
}

void psy_ui_slider_setcharnumber(psy_ui_Slider* self, double charnumber)
{
	self->charnumber = charnumber;
}

void psy_ui_slider_setvaluecharnumber(psy_ui_Slider* self, double charnumber)
{
	self->valuecharnumber = charnumber;
}

void psy_ui_slider_setvalue(psy_ui_Slider* self, double value)
{
	self->value = value;
}

double psy_ui_slider_value(psy_ui_Slider* self)
{
	return self->value;
}

void psy_ui_slider_ondraw(psy_ui_Slider* self, psy_ui_Graphics* g)
{
	psy_ui_Rectangle r;
	extern psy_ui_App app;
	psy_ui_TextMetric tm;
	
	tm = psy_ui_component_textmetric(&self->component);
	psy_ui_setcolour(g, app.defaults.style_common.border.colour_top);	
	if (self->orientation == psy_ui_HORIZONTAL) {
		psy_ui_Size size;	

		size = psy_ui_component_size(&self->component);
		if (self->labelvisible) {
			psy_ui_setrectangle(&r, 0, 0,
				self->labelsize, psy_ui_value_px(&size.height, &tm));
			psy_ui_settextcolour(g, app.defaults.style_common.colour);
			psy_ui_setbackgroundmode(g, psy_ui_TRANSPARENT);
			if (self->translation) {
				psy_ui_textoutrectangle(g, 0, 0, 0, r,
					self->translation, psy_strlen(self->translation));
			}
			size.width = psy_ui_value_makepx(
				psy_ui_value_px(&size.width, &tm) - (self->valuelabelsize + self->labelsize + 2 * self->margin));

			psy_ui_setrectangle(&r, self->labelsize + self->margin, 0,
				psy_ui_value_px(&size.width, &tm),
				psy_ui_value_px(&size.height, &tm));
			psy_ui_drawrectangle(g, r);
		}
		psy_ui_setrectangle(&r,
			self->labelsize + self->margin + ((psy_ui_value_px(&size.width, &tm) - self->slidersize) * self->value),
			2, self->slidersize, psy_ui_value_px(&size.height, &tm) - 4);
		psy_ui_drawsolidrectangle(g, r, app.defaults.style_common.colour);
		if (self->valuelabelvisible) {
			size = psy_ui_component_size(&self->component);
			psy_ui_setrectangle(&r, psy_ui_value_px(&size.width, &tm) - self->valuelabelsize, 0,
				self->valuelabelsize, psy_ui_value_px(&size.height, &tm));
			psy_ui_settextcolour(g, app.defaults.style_common.colour);
			psy_ui_setbackgroundmode(g, psy_ui_TRANSPARENT);
			psy_ui_textoutrectangle(g, psy_ui_value_px(&size.width, &tm) -
				self->valuelabelsize, 0, 0, r,
				self->valuedescription, strlen(self->valuedescription));
		}
	} else {
		double sliderheight = 8;
		psy_ui_Size size;
		psy_ui_Size slidersize;
		double centerx = 0;
		psy_ui_TextMetric tm;

		tm = psy_ui_component_textmetric(&self->component);
		size = psy_ui_component_size(&self->component);
		if (self->charnumber != 0) {
			
			slidersize.width = psy_ui_value_makeew((double)self->charnumber);
			slidersize.height = size.height;
			centerx = (psy_ui_value_px(&size.width, &tm) -
				psy_ui_value_px(&slidersize.width, &tm)) / 2;
		} else {
			slidersize = size;
		}
		
		psy_ui_slider_drawverticalruler(self, g);
		psy_ui_setrectangle(&r, centerx, 0,
			psy_ui_value_px(&slidersize.width, &tm),
			psy_ui_value_px(&size.height, &tm));
		psy_ui_drawrectangle(g, r);
		psy_ui_setrectangle(&r, 2 + centerx,
			((psy_ui_value_px(&size.height, &tm) - sliderheight) * (1 - self->value)),
			psy_ui_value_px(&slidersize.width, &tm) - 4, sliderheight);
		psy_ui_drawsolidrectangle(g, r, psy_ui_colour_make(0x00CACACA));
	}	
}

void psy_ui_slider_drawverticalruler(psy_ui_Slider* self, psy_ui_Graphics* g)
{
	double step = 0;
	double markwidth = 5;
	psy_ui_Size size;

	size = psy_ui_component_size(&self->component);
	for (step = 0; step <= 1.0; step += self->rulerstep) {
		double cpy;
		psy_ui_TextMetric tm;

		tm = psy_ui_component_textmetric(&self->component);
		cpy = step * psy_ui_value_px(&size.height, &tm);
		psy_ui_drawline(g, 0, cpy, markwidth, cpy);
		psy_ui_drawline(g, psy_ui_value_px(&size.width, &tm) - markwidth, cpy,
			psy_ui_value_px(&size.width, &tm), cpy);
	}
}

void psy_ui_slider_onmousedown(psy_ui_Slider* self, psy_ui_MouseEvent* ev)
{
	psy_ui_TextMetric tm;
	psy_ui_Size size;
	double width;
	double height;

	size = psy_ui_component_size(&self->component);
	tm = psy_ui_component_textmetric(&self->component);
	width = psy_ui_value_px(&size.width, &tm);
	height = psy_ui_value_px(&size.height, &tm);	
	width -= (self->valuelabelsize + self->labelsize + 2 * self->margin);
	if (self->orientation == psy_ui_HORIZONTAL) {
		self->tweakbase = (ev->x - self->labelsize - self->margin) -
			self->value * (width - self->slidersize);
	} else
	if (self->orientation == psy_ui_VERTICAL) {
		self->tweakbase = ev->y - ((1.0 - self->value) * (height - self->slidersize));
	}
	psy_ui_component_capture(&self->component);
}

void psy_ui_slider_onmousemove(psy_ui_Slider* self, psy_ui_MouseEvent* ev)
{
	if (self->tweakbase != -1) {
		psy_ui_Size size;
		psy_ui_TextMetric tm;
		float value;
		float* pvalue;

		tm = psy_ui_component_textmetric(&self->component);
		size = psy_ui_component_size(&self->component);
		size.width = psy_ui_value_makepx(
			psy_ui_value_px(&size.width, &tm) - (self->valuelabelsize + self->labelsize + 2 * self->margin));
		if (self->orientation == psy_ui_HORIZONTAL) {
			self->value = max(0.f,
				min(1.f, (ev->x - self->tweakbase - self->labelsize - self->margin) 
					/ (float)(psy_ui_value_px(&size.width, &tm) - self->slidersize)));
		}
		else {
			self->value = max(0.f,
				min(1.f, 1 - (ev->y - self->tweakbase) / (float)(
					psy_ui_value_px(&size.height, &tm) - self->slidersize)));
		}
		psy_signal_emit_float(&self->signal_tweakvalue, self, (float)self->value);		
		pvalue = &value;
		value = 0.f;
		psy_signal_emit(&self->signal_value, self, 1, pvalue);
		if (self->value != value) {
			self->value = value;
			psy_signal_emit(&self->signal_changed, self, 0);			
		}
		psy_ui_component_invalidate(&self->component);
	}
}

void psy_ui_slider_onmouseup(psy_ui_Slider* self, psy_ui_MouseEvent* ev)
{
	self->tweakbase = -1;
	psy_ui_component_releasecapture(&self->component);
}

void psy_ui_slider_onmousewheel(psy_ui_Slider* self, psy_ui_MouseEvent* ev)
{
	if (ev->delta != 0) {
		if (ev->delta > 0) {
			self->value += 0.1;
		} else {
			self->value -= 0.1;
		}
		self->value = max(0.0, min(1.0, self->value));
		psy_signal_emit_float(&self->signal_tweakvalue, self, (float)self->value);
		psy_signal_emit(&self->signal_changed, self, 0);
		psy_ui_component_invalidate(&self->component);
	}
}

void psy_ui_slider_showvertical(psy_ui_Slider* self)
{
	self->orientation = psy_ui_VERTICAL;
}

void psy_ui_slider_showhorizontal(psy_ui_Slider* self)
{
	self->orientation = psy_ui_HORIZONTAL;
}

void psy_ui_slider_showlabel(psy_ui_Slider* self)
{
	self->labelvisible = TRUE;
}

void psy_ui_slider_hidelabel(psy_ui_Slider* self)
{
	self->labelvisible = FALSE;
}

void psy_ui_slider_showvaluelabel(psy_ui_Slider* self)
{
	self->valuelabelvisible = TRUE;
}

void psy_ui_slider_hidevaluelabel(psy_ui_Slider* self)
{
	self->valuelabelvisible = FALSE;
}

psy_ui_Orientation psy_ui_slider_orientation(psy_ui_Slider* self)
{
	return self->orientation;
}

void psy_ui_slider_ontimer(psy_ui_Slider* self, psy_ui_Component* sender,
	uintptr_t timerid)
{	
	if (psy_ui_component_drawvisible(psy_ui_slider_base(self))) {
		psy_ui_slider_updatevalue(self);
		psy_ui_slider_describevalue(self);
	}
}

void psy_ui_slider_updatevalue(psy_ui_Slider* self)
{
	float value = 0;
	float* pvalue;

	pvalue = &value;
	psy_signal_emit(&self->signal_value, self, 1, pvalue);
	if (self->value != value) {
		self->value = value;
		psy_ui_component_invalidate(&self->component);
	}
}

void psy_ui_slider_describevalue(psy_ui_Slider* self)
{	
	self->valuedescription[0] = '\0';
	psy_signal_emit(&self->signal_describevalue, self, 1,
		self->valuedescription);
	if (self->valuedescription[0] == '\0') {
		psy_snprintf(self->valuedescription, 128, "%f", self->value);
	}
	psy_ui_component_invalidate(&self->component);
}

void psy_ui_slider_connect(psy_ui_Slider* self, void* context,
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

void psy_ui_slider_onalign(psy_ui_Slider* self)
{
	if (self->orientation == psy_ui_HORIZONTAL) {
		psy_ui_TextMetric tm;

		tm = psy_ui_component_textmetric(&self->component);
		if (self->labelvisible) {
			self->labelsize = tm.tmAveCharWidth *
				((self->charnumber == 0)
					? psy_strlen(self->translation) + 2
					: self->charnumber);
		} else {
			self->labelsize = 0;
		}
		if (self->valuelabelvisible) {
			self->valuelabelsize = tm.tmAveCharWidth *
				((self->valuecharnumber == 0)
					? psy_strlen(self->translation) + 2
					: self->valuecharnumber);
		} else {
			self->valuelabelsize = 0;
		}
	}
}

void psy_ui_slider_onpreferredsize(psy_ui_Slider* self, psy_ui_Size* limit,
	psy_ui_Size* rv)
{	
	if (self->orientation == psy_ui_HORIZONTAL) {
		rv->width = psy_ui_value_makeew(20);
		rv->height = psy_ui_value_makeeh(1.3);
	} else {
		rv->width = psy_ui_value_makeew((double)self->charnumber);
		rv->height = psy_ui_value_makeeh(20.0);
	}
}

void psy_ui_slider_onlanguagechanged(psy_ui_Slider* self)
{
	assert(self);

	psy_strreset(&self->translation, psy_ui_translate(self->label));
	psy_ui_component_invalidate(&self->component);
}

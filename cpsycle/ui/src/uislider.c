// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "uislider.h"
#include <commctrl.h>
#include <stdio.h>
#include "uiapp.h"
#include "../../detail/portable.h"

static void psy_ui_slider_initsignals(psy_ui_Slider*);
static void psy_ui_slider_disposesignals(psy_ui_Slider*);
static void psy_ui_slider_ondraw(psy_ui_Slider*, psy_ui_Graphics*);
static void psy_ui_slider_drawverticalruler(psy_ui_Slider*, psy_ui_Graphics*);
static void psy_ui_slider_onmousedown(psy_ui_Slider*, psy_ui_MouseEvent* ev);
static void psy_ui_slider_onmouseup(psy_ui_Slider*, psy_ui_MouseEvent* ev);
static void psy_ui_slider_onmousemove(psy_ui_Slider*, psy_ui_MouseEvent* ev);
static void psy_ui_slider_ondestroy(psy_ui_Slider*, psy_ui_Component* sender);
static void psy_ui_slider_ontimer(psy_ui_Slider*, psy_ui_Component* sender,
	int timerid);
static void psy_ui_slider_updatevalue(psy_ui_Slider*);
static void psy_ui_slider_describevalue(psy_ui_Slider* self);

static const int UI_TIMERID_SLIDER = 600;

static psy_ui_ComponentVtable vtable;
static int vtable_initialized = 0;

static void vtable_init(psy_ui_Slider* self)
{
	if (!vtable_initialized) {
		vtable = *(self->component.vtable);
		vtable.ondraw = (psy_ui_fp_ondraw) psy_ui_slider_ondraw;
		vtable.onmousedown = (psy_ui_fp_onmousedown) psy_ui_slider_onmousedown;
		vtable.onmousemove = (psy_ui_fp_onmousemove) psy_ui_slider_onmousemove;
		vtable.onmouseup = (psy_ui_fp_onmouseup) psy_ui_slider_onmouseup;
		vtable_initialized = 1;
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
	self->value = 0.0f;
	self->labelsize = 100;
	self->valuelabelsize = 40;	
	self->valuedescription[0] = '\0';
	self->label[0] = '\0';
	self->margin = 5;
	self->rulerstep = 0.1;
	self->charnumber = 0;
	psy_ui_slider_initsignals(self);	
	psy_signal_connect(&self->component.signal_destroy, self, 
		psy_ui_slider_ondestroy);	
	psy_signal_connect(&self->component.signal_timer, self,
		psy_ui_slider_ontimer);
	psy_ui_component_starttimer(&self->component, UI_TIMERID_SLIDER, 50);
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
	strcpy(self->label, text);
}

void psy_ui_slider_setcharnumber(psy_ui_Slider* self, int charnumber)
{
	self->charnumber = charnumber;
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
	
	psy_ui_setcolor(g, psy_ui_defaults_bordercolor(&app.defaults));	
	if (self->orientation == psy_ui_HORIZONTAL) {
		int sliderwidth = 6;
		psy_ui_Size size;	

		size = psy_ui_component_size(&self->component);
		psy_ui_setrectangle(&r, 0, 0,
			self->labelsize, size.height);
		psy_ui_settextcolor(g, psy_ui_defaults_color(&app.defaults));
		psy_ui_setbackgroundmode(g, TRANSPARENT);
		psy_ui_textoutrectangle(g, 0, 0, 0, r,
			self->label, strlen(self->label));
		size.width -= (self->valuelabelsize + self->labelsize + 2 * self->margin);
		
		psy_ui_setrectangle(&r, self->labelsize + self->margin, 0, size.width, size.height);
		psy_ui_drawrectangle(g, r);
		psy_ui_setrectangle(&r,
			self->labelsize + self->margin + (int)((size.width - sliderwidth) * self->value),
			2, sliderwidth, size.height - 4);
		psy_ui_drawsolidrectangle(g, r, psy_ui_defaults_color(&app.defaults));
		{
			size = psy_ui_component_size(&self->component);
			psy_ui_setrectangle(&r, size.width - self->valuelabelsize, 0,
				self->valuelabelsize, size.height);
			psy_ui_settextcolor(g, psy_ui_defaults_color(&app.defaults));
			psy_ui_setbackgroundmode(g, TRANSPARENT);
			psy_ui_textoutrectangle(g, size.width - self->valuelabelsize, 0, 0, r,
				self->valuedescription, strlen(self->valuedescription));
		}
	} else {
		int sliderheight = 8;
		psy_ui_Size size;
		psy_ui_Size slidersize;
		int centerx = 0;
		size = psy_ui_component_size(&self->component);

		if (self->charnumber != 0) {
			psy_ui_TextMetric tm;			
		
			tm = psy_ui_component_textmetric(&self->component);
			slidersize.width = tm.tmAveCharWidth * self->charnumber;
			slidersize.height = size.height;
			centerx = (size.width - slidersize.width) / 2;
		} else {
			slidersize = size;
		}
		
		psy_ui_slider_drawverticalruler(self, g);
		psy_ui_setrectangle(&r, centerx, 0, slidersize.width, size.height);
		psy_ui_drawrectangle(g, r);
		psy_ui_setrectangle(&r, 2 + centerx,
			(int)((size.height - sliderheight) * (1 - self->value)),
			slidersize.width - 4, sliderheight);
		psy_ui_drawsolidrectangle(g, r, 0x00CACACA);
	}	
}

void psy_ui_slider_drawverticalruler(psy_ui_Slider* self, psy_ui_Graphics* g)
{
	double step = 0;
	int markwidth = 5;
	psy_ui_Size size;

	size = psy_ui_component_size(&self->component);
	for (step = 0; step <= 1.0; step += self->rulerstep) {
		int cpy;

		cpy = (int)(step * size.height);
		psy_ui_drawline(g, 0, cpy, markwidth, cpy);
		psy_ui_drawline(g, size.width - markwidth, cpy, size.width, cpy);
	}
}

void psy_ui_slider_onmousedown(psy_ui_Slider* self, psy_ui_MouseEvent* ev)
{
	psy_ui_Size size;
	size = psy_ui_component_size(&self->component);
	size.width -= (self->valuelabelsize + self->labelsize + 2 * self->margin);
	if (self->orientation == psy_ui_HORIZONTAL) {
		self->tweakbase = (ev->x - self->labelsize - self->margin) -
			(int)(self->value * (size.width - 6));
	} else
	if (self->orientation == psy_ui_VERTICAL) {
		self->tweakbase = ev->y - (int)(self->value * (size.height - 6));
	}
	psy_ui_component_capture(&self->component);
}

void psy_ui_slider_onmousemove(psy_ui_Slider* self, psy_ui_MouseEvent* ev)
{
	if (self->tweakbase != -1) {
		psy_ui_Size size;

		size = psy_ui_component_size(&self->component);
		size.width -= (self->valuelabelsize + self->labelsize + 2 * self->margin);
		if (self->orientation == psy_ui_HORIZONTAL) {
			self->value = max(0.f,
				min(1.f, (ev->x - self->tweakbase - self->labelsize - self->margin) 
					/ (float)(size.width - 6)));
		}
		else {
			self->value = max(0.f,
				min(1.f, 1 - (ev->y - self->tweakbase) / (float)(size.height - 6)));
		}
		psy_signal_emit_float(&self->signal_tweakvalue, self, (float)self->value);
		psy_signal_emit(&self->signal_changed, self, 0);
		psy_ui_component_invalidate(&self->component);
	}
}

void psy_ui_slider_onmouseup(psy_ui_Slider* self, psy_ui_MouseEvent* ev)
{
	self->tweakbase = -1;
	psy_ui_component_releasecapture(&self->component);
}

void psy_ui_slider_showvertical(psy_ui_Slider* self)
{
	self->orientation = psy_ui_VERTICAL;
}

void psy_ui_slider_showhorizontal(psy_ui_Slider* self)
{
	self->orientation = psy_ui_HORIZONTAL;
}

psy_ui_Orientation psy_ui_slider_orientation(psy_ui_Slider* self)
{
	return self->orientation;
}

void psy_ui_slider_ontimer(psy_ui_Slider* self, psy_ui_Component* sender,
	int timerid)
{	
	psy_ui_slider_updatevalue(self);
	psy_ui_slider_describevalue(self);
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
	ui_slider_fpdescribe describe, ui_slider_fptweak tweak,
	ui_slider_fpvalue value)
{
	psy_signal_connect(&self->signal_describevalue, context, describe);
	psy_signal_connect(&self->signal_tweakvalue, context, tweak);
	psy_signal_connect(&self->signal_value, context, value);
}

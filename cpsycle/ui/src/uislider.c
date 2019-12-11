// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "uislider.h"
#include <commctrl.h>
#include <stdio.h>
#include <portable.h>

static void ui_slider_initsignals(ui_slider*);
static void ui_slider_disposesignals(ui_slider*);
static void ui_slider_ondraw(ui_slider*, ui_component* sender, ui_graphics*);
static void ui_slider_drawverticalruler(ui_slider*, ui_graphics* g);
static void ui_slider_onmousedown(ui_slider*, ui_component* sender, MouseEvent* ev);
static void ui_slider_onmouseup(ui_slider*, ui_component* sender, MouseEvent* ev);
static void ui_slider_onmousemove(ui_slider*, ui_component* sender, MouseEvent* ev);
static void ui_slider_ondestroy(ui_slider*, ui_component* sender);
static void ui_slider_ontimer(ui_slider*, ui_component* sender, int timerid);
static void ui_slider_updatevalue(ui_slider*);
static void ui_slider_describevalue(self);

static int UI_TIMERID_SLIDER = 600;

void ui_slider_init(ui_slider* self, ui_component* parent)
{	
	ui_component_init(&self->component, parent);
	self->component.doublebuffered = 1;
	self->tweakbase = -1;
	self->orientation = UI_HORIZONTAL;
	self->value = 0.0f;
	self->labelsize = 100;
	self->valuelabelsize = 40;	
	self->valuedescription[0] = '\0';
	self->label[0] = '\0';
	self->margin = 5;
	self->rulerstep = 0.1;
	self->charnumber = 0;
	ui_slider_initsignals(self);
	psy_signal_connect(&self->component.signal_draw, self, 
		ui_slider_ondraw);
	psy_signal_connect(&self->component.signal_destroy, self, 
		ui_slider_ondestroy);
	psy_signal_connect(&self->component.signal_mousedown, self,
		ui_slider_onmousedown);
	psy_signal_connect(&self->component.signal_mousemove, self,
		ui_slider_onmousemove);
	psy_signal_connect(&self->component.signal_mouseup, self,
		ui_slider_onmouseup);
	psy_signal_connect(&self->component.signal_timer, self,
		ui_slider_ontimer);
	ui_component_starttimer(&self->component, UI_TIMERID_SLIDER, 50);	
}

void ui_slider_initsignals(ui_slider* self)
{
	psy_signal_init(&self->signal_clicked);
	psy_signal_init(&self->signal_changed);
	psy_signal_init(&self->signal_describevalue);
	psy_signal_init(&self->signal_tweakvalue);
	psy_signal_init(&self->signal_value);	
}

void ui_slider_ondestroy(ui_slider* self, ui_component* sender)
{
	ui_slider_disposesignals(self);
}

void ui_slider_disposesignals(ui_slider* self)
{
	psy_signal_dispose(&self->signal_clicked);
	psy_signal_dispose(&self->signal_changed);
	psy_signal_dispose(&self->signal_describevalue);
	psy_signal_dispose(&self->signal_tweakvalue);
	psy_signal_dispose(&self->signal_value);
}

void ui_slider_settext(ui_slider* self, const char* text)
{
	strcpy(self->label, text);
}

void ui_slider_setcharnumber(ui_slider* self, int charnumber)
{
	self->charnumber = charnumber;
}

void ui_slider_setvalue(ui_slider* self, double value)
{
	self->value = value;
}

double ui_slider_value(ui_slider* self)
{
	return self->value;
}

void ui_slider_ondraw(ui_slider* self, ui_component* sender, ui_graphics* g)
{
	ui_rectangle r;
	
	ui_setcolor(g, 0x00333333);	
	if (self->orientation == UI_HORIZONTAL) {
		int sliderwidth = 6;
		ui_size size;	

		size = ui_component_size(&self->component);
		ui_setrectangle(&r, 0, 0,
			self->labelsize, size.height);
		ui_settextcolor(g, 0x00CACACA);
		ui_setbackgroundmode(g, TRANSPARENT);
		ui_textoutrectangle(g, 0, 0, 0, r,
			self->label, strlen(self->label));
		size.width -= (self->valuelabelsize + self->labelsize + 2 * self->margin);

		ui_setrectangle(&r, self->labelsize + self->margin, 0, size.width, size.height);
		ui_drawrectangle(g, r);
		ui_setrectangle(&r,
			self->labelsize + self->margin + (int)((size.width - sliderwidth) * self->value),
			2, sliderwidth, size.height - 4);
		ui_drawsolidrectangle(g, r, 0x00CACACA);
		{
			size = ui_component_size(&self->component);
			ui_setrectangle(&r, size.width - self->valuelabelsize, 0,
				self->valuelabelsize, size.height);
			ui_settextcolor(g, 0x00CACACA);
			ui_setbackgroundmode(g, TRANSPARENT);
			ui_textoutrectangle(g, size.width - self->valuelabelsize, 0, 0, r,
				self->valuedescription, strlen(self->valuedescription));
		}
	} else {
		int sliderheight = 8;
		ui_size size;
		ui_size slidersize;
		int centerx = 0;
		size = ui_component_size(&self->component);

		if (self->charnumber != 0) {
			ui_textmetric tm;			
		
			tm = ui_component_textmetric(&self->component);
			slidersize.width = tm.tmAveCharWidth * self->charnumber;
			slidersize.height = size.height;
			centerx = (size.width - slidersize.width) / 2;
		} else {
			slidersize = size;
		}
		
		ui_slider_drawverticalruler(self, g);
		ui_setrectangle(&r, centerx, 0, slidersize.width, size.height);
		ui_drawrectangle(g, r);
		ui_setrectangle(&r, 2 + centerx, (int)((size.height - sliderheight) * (1 - self->value)),
			slidersize.width - 4, sliderheight);
		ui_drawsolidrectangle(g, r, 0x00CACACA);
	}	
}

void ui_slider_drawverticalruler(ui_slider* self, ui_graphics* g)
{
	double step = 0;
	int markwidth = 5;
	ui_size size;

	size = ui_component_size(&self->component);
	for (step = 0; step <= 1.0; step += self->rulerstep) {
		int cpy;

		cpy = (int)(step * size.height);
		ui_drawline(g, 0, cpy, markwidth, cpy);
		ui_drawline(g, size.width - markwidth, cpy, size.width, cpy);
	}
}

void ui_slider_onmousedown(ui_slider* self, ui_component* sender,
	MouseEvent* ev)
{
	ui_size size;
	size = ui_component_size(&self->component);
	size.width -= (self->valuelabelsize + self->labelsize + 2 * self->margin);
	if (self->orientation == UI_HORIZONTAL) {
		self->tweakbase = (ev->x - self->labelsize - self->margin) -
			(int)(self->value * (size.width - 6));
	} else
	if (self->orientation == UI_VERTICAL) {
		self->tweakbase = ev->y - (int)(self->value * (size.height - 6));
	}
	ui_component_capture(&self->component);
}

void ui_slider_onmousemove(ui_slider* self, ui_component* sender,
	MouseEvent* ev)
{
	if (self->tweakbase != -1) {
		ui_size size;

		size = ui_component_size(&self->component);
		size.width -= (self->valuelabelsize + self->labelsize + 2 * self->margin);
		if (self->orientation == UI_HORIZONTAL) {
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
		ui_component_invalidate(&self->component);
	}
}

void ui_slider_onmouseup(ui_slider* self, ui_component* sender, MouseEvent* ev)
{
	self->tweakbase = -1;
	ui_component_releasecapture(&self->component);
}

void ui_slider_showvertical(ui_slider* self)
{
	self->orientation = UI_VERTICAL;
}

void ui_slider_showhorizontal(ui_slider* self)
{
	self->orientation = UI_HORIZONTAL;
}

UiOrientation ui_slider_orientation(ui_slider* self)
{
	return self->orientation;
}

void ui_slider_ontimer(ui_slider* self, ui_component* sender, int timerid)
{	
	ui_slider_updatevalue(self);
	ui_slider_describevalue(self);
}

void ui_slider_updatevalue(ui_slider* self)
{
	float value = 0;
	float* pvalue;

	pvalue = &value;
	psy_signal_emit(&self->signal_value, self, 1, pvalue);
	if (self->value != value) {
		self->value = value;
		ui_component_invalidate(&self->component);
	}
}

void ui_slider_describevalue(ui_slider* self)
{	
	self->valuedescription[0] = '\0';
	psy_signal_emit(&self->signal_describevalue, self, 1,
		self->valuedescription);
	if (self->valuedescription[0] == '\0') {
		psy_snprintf(self->valuedescription, 128, "%f", self->value);
	}
	ui_component_invalidate(&self->component);
}

void ui_slider_connect(ui_slider* self, void* context, ui_slider_fpdescribe describe,
	ui_slider_fptweak tweak, ui_slider_fpvalue value)
{
	psy_signal_connect(&self->signal_describevalue, context, describe);
	psy_signal_connect(&self->signal_tweakvalue, context, tweak);
	psy_signal_connect(&self->signal_value, context, value);
}

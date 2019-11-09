// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "uislider.h"
#include <commctrl.h>
#include <stdio.h>

static void ui_slider_ondraw(ui_slider*, ui_component* sender, ui_graphics*);
static void ui_slider_onmousedown(ui_slider*, ui_component* sender, int x, int y, int button);
static void ui_slider_onmouseup(ui_slider*, ui_component* sender, int x, int y, int button);
static void ui_slider_onmousemove(ui_slider*, ui_component* sender, int x, int y, int button);
static void ui_slider_ondestroy(ui_slider*, ui_component* sender);

void ui_slider_init(ui_slider* self, ui_component* parent)
{	
	ui_component_init(&self->component, parent);
	self->component.doublebuffered = 1;
	self->tweakbase = -1;
	self->orientation = UI_HORIZONTAL;
	self->value = 0.0f;
	signal_init(&self->signal_clicked);
	signal_init(&self->signal_changed);	
	signal_connect(&self->component.signal_draw, self, ui_slider_ondraw);
	signal_connect(&self->component.signal_destroy, self, ui_slider_ondestroy);
	signal_connect(&self->component.signal_mousedown, self, ui_slider_onmousedown);
	signal_connect(&self->component.signal_mousemove, self, ui_slider_onmousemove);
	signal_connect(&self->component.signal_mouseup, self, ui_slider_onmouseup);
}

void ui_slider_ondestroy(ui_slider* self, ui_component* sender)
{
	signal_dispose(&self->signal_clicked);
	signal_dispose(&self->signal_changed);
}

void ui_slider_settext(ui_slider* slider, const char* text)
{	
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
	ui_size size;

	size = ui_component_size(&self->component);
	ui_setcolor(g, 0x00333333);
	ui_setrectangle(&r, 0, 0, size.width, size.height);
	ui_drawrectangle(g, r);
	if (self->orientation == UI_HORIZONTAL) {
		int sliderwidth = 6;

		ui_setrectangle(&r, (int)((size.width - sliderwidth) * self->value),
			2, sliderwidth, size.height - 4);
		ui_drawsolidrectangle(g, r, 0x00CACACA);
	} else {
		int sliderheight = 8;

		ui_setrectangle(&r, 2, (int)((size.height - sliderheight) * (1 - self->value)),
			size.width - 4, sliderheight);
		ui_drawsolidrectangle(g, r, 0x00CACACA);
	}
}

void ui_slider_onmousedown(ui_slider* self, ui_component* sender, int x, int y,
	int button)
{
	ui_size size;
	size = ui_component_size(&self->component);
	if (self->orientation == UI_HORIZONTAL) {
		self->tweakbase = x - (int)(self->value * (size.width - 6));
	} else
	if (self->orientation == UI_VERTICAL) {
		self->tweakbase = y - (int)(self->value * (size.height - 6));
	}
	ui_component_capture(&self->component);
}

void ui_slider_onmousemove(ui_slider* self, ui_component* sender, int x, int y,
	int button)
{
	if (self->tweakbase != -1) {
		ui_size size;

		size = ui_component_size(&self->component);
		if (self->orientation == UI_HORIZONTAL) {
			self->value = max(0.f,
				min(1.f, (x - self->tweakbase) / (float)(size.width - 6)));
		}
		else {
			self->value = max(0.f,
				min(1.f, 1 - (y - self->tweakbase) / (float)(size.height - 6)));
		}
		signal_emit(&self->signal_changed, self, 0);		
		ui_invalidate(&self->component);
	}
}

void ui_slider_onmouseup(ui_slider* self, ui_component* sender, int x, int y,
	int button)
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

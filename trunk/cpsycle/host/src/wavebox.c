// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "wavebox.h"

static void wavebox_ondraw(WaveBox*, ui_component* sender, ui_graphics*);
static void wavebox_ondestroy(WaveBox*, ui_component* sender);
static void wavebox_onmousedown(WaveBox*, ui_component* sender, MouseEvent*);

void wavebox_init(WaveBox* self, ui_component* parent)
{			
	self->sample = 0;
	self->hasselection = 0;
	self->selectionstart = 0;
	self->selectionend = 0;
	self->zoomleft = 0.f;
	self->zoomright = 1.f;
	ui_component_init(&self->component, parent);
	self->component.doublebuffered = 1;
	signal_connect(&self->component.signal_draw, self, wavebox_ondraw);
	signal_connect(&self->component.signal_destroy, self, wavebox_ondestroy);
	signal_connect(&self->component.signal_mousedown, self,
		wavebox_onmousedown);
}

void wavebox_ondestroy(WaveBox* self, ui_component* sender)
{		
}

void wavebox_setsample(WaveBox* self, Sample* sample)
{
	self->sample = sample;
	ui_component_invalidate(&self->component);
}

void wavebox_ondraw(WaveBox* self, ui_component* sender, ui_graphics* g)
{	
	ui_rectangle r;
	ui_size size = ui_component_size(&self->component);	
	ui_setrectangle(&r, 0, 0, size.width, size.height);
	ui_setcolor(g, 0x00B1C8B0);
	if (!self->sample) {
		ui_textmetric tm;
		static const char* txt = "No wave loaded";

		tm = ui_component_textmetric(&self->component);
		ui_setbackgroundmode(g, TRANSPARENT);
		ui_settextcolor(g, 0x00D1C5B6);
		ui_textout(g, (size.width - tm.tmAveCharWidth * strlen(txt)) / 2,
			(size.height - tm.tmHeight) / 2, txt, strlen(txt));
	} else {
		int x;
		int centery = size.height / 2;
		float offsetstep;
		amp_t scaley;

		scaley = (size.height / 2) / (amp_t)32768;		
		offsetstep = (float) self->sample->numframes / size.width *
			(self->zoomright - self->zoomleft);
		if (self->hasselection) {
			ui_rectangle r;

			ui_setrectangle(&r,
				(int)(1 / offsetstep * self->selectionstart),
				0,
				(int)(1 / offsetstep * self->selectionend) -
				(int)(1 / offsetstep * self->selectionstart),
				size.height);
			ui_drawsolidrectangle(g, r, 0x00B1C8B0);
		} 
		for (x = 0; x < size.width; ++x) {			
			uintptr_t frame = (int)(offsetstep * x + 
				(int)(self->sample->numframes * self->zoomleft));
			float framevalue;
			
			if (frame >= self->sample->numframes) {
				break;
			}
			framevalue = self->sample->channels.samples[0][frame];
			if (self->hasselection &&
				frame >= self->selectionstart &&
				frame < self->selectionend) {				
				ui_setcolor(g, 0x00232323);
			} else {
				ui_setcolor(g, 0x00B1C8B0);
			}
			ui_drawline(g, x, centery, x, centery + (int)(framevalue * scaley));
		}
	}
}

void wavebox_onmousedown(WaveBox* self, ui_component* sender, MouseEvent* ev)
{
}

void wavebox_setzoom(WaveBox* self, float zoomleft, float zoomright)
{
	self->zoomleft = zoomleft;
	self->zoomright = zoomright;
	ui_component_invalidate(&self->component);
}

// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "wavebox.h"

static void OnDraw(WaveBox*, ui_component* sender, ui_graphics*);
static void OnDestroy(WaveBox*, ui_component* sender);
static void OnMouseDown(WaveBox*, ui_component* sender, MouseEvent*);


void InitWaveBox(WaveBox* self, ui_component* parent)
{			
	self->sample = 0;
	ui_component_init(&self->component, parent);	
	signal_connect(&self->component.signal_draw, self, OnDraw);
	signal_connect(&self->component.signal_destroy, self, OnDestroy);	
	signal_connect(&self->component.signal_mousedown, self, OnMouseDown);
}

void OnDestroy(WaveBox* self, ui_component* sender)
{		
}

void OnDraw(WaveBox* self, ui_component* sender, ui_graphics* g)
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

		offsetstep = (float) self->sample->numframes / size.width;
		for (x = 0; x < size.width; ++x) {			
			int frame = (int)(offsetstep * x);
			float framevalue = self->sample->channels.samples[0][frame];
			ui_drawline(g, x, centery, x, (int) (centery + framevalue / 1000));
		}
	}
}

void OnMouseDown(WaveBox* self, ui_component* sender, MouseEvent* mouseevent)
{
}


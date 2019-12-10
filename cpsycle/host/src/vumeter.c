// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "vumeter.h"

#include <math.h>
#include <rms.h>

#define TIMERID_MASTERVU 400

static void vumeter_ondraw(Vumeter*, ui_component* sender, ui_graphics*);
static void vumeter_ontimer(Vumeter*, ui_component* sender, int timerid);
static void vumeter_onmasterworked(Vumeter*, Machine*, unsigned int slot, BufferContext*);
static void vumeter_onsongchanged(Vumeter*, Workspace*);
static void vumeter_connectmachinessignals(Vumeter*, Workspace*);

void vumeter_init(Vumeter* self, ui_component* parent, Workspace* workspace)
{					
	ui_component_init(&self->component, parent);
	self->leftavg = 0;
	self->rightavg = 0;
	self->component.doublebuffered = 1;
	signal_connect(&self->component.signal_draw, self, vumeter_ondraw);	
	signal_connect(&self->component.signal_timer, self, vumeter_ontimer);	
	signal_connect(&workspace->signal_songchanged, self,
		vumeter_onsongchanged);
	vumeter_connectmachinessignals(self, workspace);
	ui_component_starttimer(&self->component, TIMERID_MASTERVU, 50);
}

void vumeter_ondraw(Vumeter* self, ui_component* sender, ui_graphics* g)
{	
	ui_rectangle left;
	ui_rectangle right;
	ui_size size = ui_component_size(&self->component);
	ui_setrectangle(&left, 0, 5, size.width, 5);
	right = left;
	right.top += 6;
	right.bottom += 6;
	ui_drawsolidrectangle(g, left, 0x00000000);
	ui_drawsolidrectangle(g, right, 0x00000000);
	
	left.right = (int) (self->leftavg * size.width);
	right.right = (int) (self->rightavg * size.width);
	ui_drawsolidrectangle(g, left, 0x0000FF00);
	ui_drawsolidrectangle(g, right, 0x0000FF00);

	ui_setrectangle(&left, left.right, left.top, size.width - left.right, 5);
	ui_setrectangle(&right, right.right, right.top, size.width - right.right, 5);
	ui_drawsolidrectangle(g, left, 0x003E3E3E);
	ui_drawsolidrectangle(g, right, 0x003E3E3E);
}

void vumeter_ontimer(Vumeter* self, ui_component* sender, int timerid)
{	
	if (timerid == TIMERID_MASTERVU) {
		ui_component_invalidate(&self->component);
	}
}

void vumeter_onmasterworked(Vumeter* self, Machine* master, unsigned int slot,
	BufferContext* bc)
{	
	if (bc->rmsvol) {
		self->leftavg = bc->rmsvol->data.previousLeft / 32768;
		self->rightavg = bc->rmsvol->data.previousRight / 32768;
	}
}

void vumeter_onsongchanged(Vumeter* self, Workspace* workspace)
{	
	self->leftavg = 0;
	self->rightavg = 0;
	vumeter_connectmachinessignals(self, workspace);
}

void vumeter_connectmachinessignals(Vumeter* self, Workspace* workspace)
{
	if (workspace && workspace->song &&
			machines_master(&workspace->song->machines)) {
		signal_connect(
			&machines_master(&workspace->song->machines)->signal_worked, self,
			vumeter_onmasterworked);
	}
}

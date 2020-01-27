// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "vumeter.h"

#include <math.h>
#include <rms.h>

#define TIMERID_MASTERVU 400

static void vumeter_ondraw(Vumeter*, psy_ui_Graphics*);
static void vumeter_ontimer(Vumeter*, psy_ui_Component* sender, int timerid);
static void vumeter_onmasterworked(Vumeter*, psy_audio_Machine*,
	uintptr_t slot, psy_audio_BufferContext*);
static void vumeter_onsongchanged(Vumeter*, Workspace*);
static void vumeter_connectmachinessignals(Vumeter*, Workspace*);
static void vumeter_onpreferredsize(Vumeter*, psy_ui_Size* limit, psy_ui_Size* rv);

static psy_ui_ComponentVtable vtable;
static int vtable_initialized = 0;

static void vtable_init(Vumeter* self)
{
	if (!vtable_initialized) {
		vtable = *(self->component.vtable);
		vtable.onpreferredsize = (psy_ui_fp_onpreferredsize)
			vumeter_onpreferredsize;
		vtable.ondraw = (psy_ui_fp_ondraw) vumeter_ondraw;
	}
}

void vumeter_init(Vumeter* self, psy_ui_Component* parent, Workspace* workspace)
{					
	psy_ui_component_init(&self->component, parent);
	vtable_init(self);
	self->component.vtable = &vtable;
	self->leftavg = 0;
	self->rightavg = 0;
	psy_ui_component_doublebuffer(&self->component);
	psy_signal_connect(&self->component.signal_timer, self, vumeter_ontimer);	
	psy_signal_connect(&workspace->signal_songchanged, self,
		vumeter_onsongchanged);
	vumeter_connectmachinessignals(self, workspace);
	psy_ui_component_starttimer(&self->component, TIMERID_MASTERVU, 50);
}

void vumeter_ondraw(Vumeter* self, psy_ui_Graphics* g)
{	
	psy_ui_Rectangle left;
	psy_ui_Rectangle right;
	psy_ui_Size size = psy_ui_component_size(&self->component);
	psy_ui_setrectangle(&left, 0, 5, size.width, 5);
	right = left;
	right.top += 6;
	right.bottom += 6;
	psy_ui_drawsolidrectangle(g, left, 0x00000000);
	psy_ui_drawsolidrectangle(g, right, 0x00000000);
	
	left.right = (int) (self->leftavg * size.width);
	right.right = (int) (self->rightavg * size.width);
	psy_ui_drawsolidrectangle(g, left, 0x0000FF00);
	psy_ui_drawsolidrectangle(g, right, 0x0000FF00);

	psy_ui_setrectangle(&left, left.right, left.top, size.width - left.right, 5);
	psy_ui_setrectangle(&right, right.right, right.top, size.width - right.right, 5);
	psy_ui_drawsolidrectangle(g, left, 0x003E3E3E);
	psy_ui_drawsolidrectangle(g, right, 0x003E3E3E);
}

void vumeter_ontimer(Vumeter* self, psy_ui_Component* sender, int timerid)
{	
	if (timerid == TIMERID_MASTERVU) {
		psy_ui_component_invalidate(&self->component);
	}
}

void vumeter_onmasterworked(Vumeter* self, psy_audio_Machine* master,
	uintptr_t slot,
	psy_audio_BufferContext* bc)
{	
	if (bc->rmsvol) {
		self->leftavg = psy_audio_buffercontext_rmsscale(bc,
			bc->rmsvol->data.previousLeft);
		self->rightavg = psy_audio_buffercontext_rmsscale(bc,
			bc->rmsvol->data.previousRight);
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
		psy_signal_connect(
			&machines_master(&workspace->song->machines)->signal_worked, self,
			vumeter_onmasterworked);
	}
}

void vumeter_onpreferredsize(Vumeter* self, psy_ui_Size* limit, psy_ui_Size* rv)
{	
	rv->width = 180;
	rv->height = 20;
}

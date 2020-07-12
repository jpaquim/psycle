// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "vumeter.h"

#include <math.h>
#include <rms.h>

#define TIMERID_MASTERVU 400

static void vumeter_ondestroy(Vumeter*, psy_ui_Component* sender);
static void vumeter_ondraw(Vumeter*, psy_ui_Graphics*);
static void vumeter_ontimer(Vumeter*, psy_ui_Component* sender, uintptr_t timerid);

static VumeterSkin vumeterdefaultskin;
static int vumeterdefaultskin_initialized = 0;

static void vumeterskin_init(Vumeter* self)
{
	if (!vumeterdefaultskin_initialized) {
		vumeterdefaultskin.background = psy_ui_color_make(0x00000000);
		vumeterdefaultskin.rms = psy_ui_color_make(0x0000FF00);
		vumeterdefaultskin.peak = psy_ui_color_make(0x00333333);
		vumeterdefaultskin.border = psy_ui_color_make(0x003E3E3E);
		vumeterdefaultskin_initialized = 1;
	}
	self->skin = vumeterdefaultskin;
}

static psy_ui_ComponentVtable vumeter_vtable;
static int vumeter_vtable_initialized = 0;

static void vumeter_vtable_init(Vumeter* self)
{
	if (!vumeter_vtable_initialized) {
		vumeter_vtable = *(self->component.vtable);
		vumeter_vtable.ondraw = (psy_ui_fp_ondraw)vumeter_ondraw;
		vumeter_vtable_initialized = 1;
	}
}

void vumeter_init(Vumeter* self, psy_ui_Component* parent,
	Workspace* workspace)
{					
	psy_ui_component_init(&self->component, parent);
	vumeter_vtable_init(self);
	self->component.vtable = &vumeter_vtable;	
	self->leftavg = 0;
	self->rightavg = 0;
	self->workspace = workspace;
	vumeterskin_init(self);
	psy_ui_component_setpreferredsize(&self->component,
		psy_ui_size_make(psy_ui_value_makeew(25),
			psy_ui_value_makeeh(1)));
	psy_ui_component_doublebuffer(&self->component);
	psy_signal_connect(&self->component.signal_timer, self, vumeter_ontimer);	
	psy_signal_connect(&self->component.signal_destroy, self,
		vumeter_ondestroy);
	psy_ui_component_starttimer(&self->component, TIMERID_MASTERVU, 50);
}

void vumeter_ondestroy(Vumeter* self, psy_ui_Component* sender)
{	
	psy_ui_component_stoptimer(&self->component, TIMERID_MASTERVU);
}

void vumeter_ondraw(Vumeter* self, psy_ui_Graphics* g)
{	
	psy_ui_Rectangle left;
	psy_ui_Rectangle right;
	psy_ui_IntSize size;
	psy_ui_TextMetric tm;

	tm = psy_ui_component_textmetric(&self->component);
	size = psy_ui_intsize_init_size(
		psy_ui_component_size(&self->component), &tm);
	psy_ui_setrectangle(&left, 0, 5, size.width, 5);
	right = left;
	right.top += 6;
	right.bottom += 6;
	psy_ui_drawsolidrectangle(g, left, self->skin.background);
	psy_ui_drawsolidrectangle(g, right, self->skin.background);	
	left.right = (int) (self->leftavg * size.width);
	right.right = (int) (self->rightavg * size.width);
	psy_ui_drawsolidrectangle(g, left, self->skin.rms);
	psy_ui_drawsolidrectangle(g, right, self->skin.rms);
	psy_ui_setrectangle(&left, left.right, left.top,
		size.width - left.right, 5);
	psy_ui_setrectangle(&right, right.right, right.top,
		size.width - right.right, 5);
	psy_ui_drawsolidrectangle(g, left, self->skin.border);
	psy_ui_drawsolidrectangle(g, right, self->skin.border);
}

void vumeter_ontimer(Vumeter* self, psy_ui_Component* sender, uintptr_t timerid)
{	
	if (timerid == TIMERID_MASTERVU && self->workspace->song) {
		psy_audio_Machine* master;
		psy_audio_Buffer* memory;
		psy_dsp_amp_t leftavg;
		psy_dsp_amp_t rightavg;

		master = psy_audio_machines_master(&self->workspace->song->machines);
		if (master) {
			memory = psy_audio_machine_buffermemory(master);
			if (memory && memory->rms) {
				leftavg = memory->rms->data.previousLeft / 32768;
				rightavg = memory->rms->data.previousRight / 32768;
				if (leftavg != self->leftavg || rightavg != self->rightavg) {
					self->leftavg = memory->rms->data.previousLeft / 32768;
					self->rightavg = memory->rms->data.previousRight / 32768;
					psy_ui_component_invalidate(&self->component);
				}
			}
		} else if (self->leftavg != 0.f || self->rightavg != 0.f) {
			self->leftavg = 0;
			self->rightavg = 0;
			psy_ui_component_invalidate(&self->component);
		}
	}
}

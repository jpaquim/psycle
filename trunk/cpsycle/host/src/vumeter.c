/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "vumeter.h"
/* dsp */
#include <convert.h>
#include <operations.h>
#include <rms.h>
/* std */
#include <math.h>

static VumeterSkin vumeterdefaultskin;
static int vumeterdefaultskin_initialized = 0;

static void vumeterskin_init(Vumeter* self)
{	
	if (!vumeterdefaultskin_initialized) {
		vumeterdefaultskin.background = psy_ui_colour_make(0x00282828);
		vumeterdefaultskin.rms = psy_ui_colour_make(0x004DFFA6);
		vumeterdefaultskin.peak = psy_ui_colour_make(0x004D4DFF);
		vumeterdefaultskin.border = psy_ui_colour_make(0x003E3E3E);
		vumeterdefaultskin_initialized = 1;
	}
	self->skin = vumeterdefaultskin;
}

/* prototypes */
static void vumeter_ondraw(Vumeter*, psy_ui_Graphics*);

/* vtable */
static psy_ui_ComponentVtable vumeter_vtable;
static bool vumeter_vtable_initialized = FALSE;

static void vumeter_vtable_init(Vumeter* self)
{
	if (!vumeter_vtable_initialized) {
		vumeter_vtable = *(self->component.vtable);
		vumeter_vtable.ondraw =
			(psy_ui_fp_component_ondraw)
			vumeter_ondraw;
		vumeter_vtable_initialized = TRUE;
	}
	psy_ui_component_setvtable(&self->component, &vumeter_vtable);
}

/* implementation */
void vumeter_init(Vumeter* self, psy_ui_Component* parent,
	Workspace* workspace)
{					
	psy_ui_component_init(&self->component, parent, NULL);
	vumeter_vtable_init(self);
	psy_ui_component_setpreferredsize(&self->component,
		psy_ui_size_make_em(25.0, 1.0));
	self->leftavg = 0;
	self->rightavg = 0;
	self->l_log = -10000;
	self->r_log = -10000;
	self->workspace = workspace;
	vumeterskin_init(self);	
}

void vumeter_ondraw(Vumeter* self, psy_ui_Graphics* g)
{	
	psy_ui_RealRectangle left;
	psy_ui_RealRectangle right;	
	psy_ui_RealSize size;	
	double vuprevL;
	double vuprevR;
	
	size = psy_ui_component_size_px(&self->component);
	left = psy_ui_realrectangle_make(
		psy_ui_realpoint_make(0, 5),
		psy_ui_realsize_make(size.width, 5));
	right = left;
	right.top += 6;
	right.bottom += 6;
	psy_ui_drawsolidrectangle(g, left, self->skin.background);
	psy_ui_drawsolidrectangle(g, right, self->skin.background);		
	left.right = (int) (self->leftavg * size.width); 
	right.right = (int) (self->rightavg * size.width);
	psy_ui_drawsolidrectangle(g, left, self->skin.rms);
	psy_ui_drawsolidrectangle(g, right, self->skin.rms);
	vuprevL = (40.0 + self->l_log) * size.width / 40.f;
	vuprevR = (40.0 + self->r_log) * size.width / 40.f;
	if (vuprevL > size.width) vuprevL = size.width;
	if (vuprevR > size.width) vuprevR = size.width;
	if (vuprevL > left.left) {		
		left.left = left.right;
		left.right = vuprevL;
		psy_ui_drawsolidrectangle(g, left, self->skin.peak);
	}
	if (vuprevR > right.left) {
		right.left = right.right;
		right.right = vuprevR;
		psy_ui_drawsolidrectangle(g, right, self->skin.peak);
	}
	left = psy_ui_realrectangle_make(
		psy_ui_realpoint_make(left.right, left.top),
		psy_ui_realsize_make(size.width - left.right, 5));
	right = psy_ui_realrectangle_make(
		psy_ui_realpoint_make(right.right, right.top),
		psy_ui_realsize_make(size.width - right.right, 5));
	psy_ui_drawsolidrectangle(g, left, self->skin.border);
	psy_ui_drawsolidrectangle(g, right, self->skin.border);
}

void vumeter_idle(Vumeter* self)
{			
	if (workspace_song(self->workspace)) {
		psy_audio_Machine* master;
		psy_audio_Buffer* memory;
		psy_dsp_amp_t leftavg;
		psy_dsp_amp_t rightavg;

		master = psy_audio_machines_master(
			&workspace_song(self->workspace)->machines);
		if (master) {
			memory = psy_audio_machine_buffermemory(master);
			if (memory && memory->rms) {
				leftavg = memory->rms->data.previousLeft / 32768.f;
				rightavg = memory->rms->data.previousRight / 32768.f;
				if (leftavg != self->leftavg || rightavg != self->rightavg) {					
					self->leftavg = psy_audio_buffer_rmsscale(memory,
						memory->rms->data.previousLeft);
					self->rightavg = psy_audio_buffer_rmsscale(memory,
						memory->rms->data.previousRight);
					self->l_log = psy_dsp_convert_amp_to_db(
						psy_audio_buffer_rangefactor(memory,
							PSY_DSP_AMP_RANGE_VST) *
						(1.f + dsp.maxvol(memory->samples[0],
							memory->numsamples)));
					self->r_log = psy_dsp_convert_amp_to_db(
						(1 + dsp.maxvol(memory->samples[1],
							memory->numsamples)) *
						psy_audio_buffer_rangefactor(memory,
							PSY_DSP_AMP_RANGE_VST));
#ifndef PSYCLE_DEBUG_PREVENT_TIMER_DRAW
					psy_ui_component_invalidate(&self->component);
#endif
				}				
			}
		} else if (self->leftavg != 0.f || self->rightavg != 0.f) {
			self->leftavg = 0.f;
			self->rightavg = 0.f;
			self->l_log = -10000.f;
			self->r_log = -10000.f;
#ifndef PSYCLE_DEBUG_PREVENT_TIMER_DRAW
			psy_ui_component_invalidate(&self->component);			
#endif
		}
	}	
}

/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "vumeter.h"
/* host */
#include "styles.h"
/* ui */
#include <uiapp.h>
/* dsp */
#include <convert.h>
#include <operations.h>
#include <rms.h>
/* std */
#include <math.h>


/* prototypes */
static void vumeter_on_draw(Vumeter*, psy_ui_Graphics*);
static void vumeter_on_align(Vumeter*);

/* vtable */
static psy_ui_ComponentVtable vumeter_vtable;
static bool vumeter_vtable_initialized = FALSE;

static void vumeter_vtable_init(Vumeter* self)
{
	if (!vumeter_vtable_initialized) {
		vumeter_vtable = *(self->component.vtable);
		vumeter_vtable.ondraw =
			(psy_ui_fp_component_ondraw)
			vumeter_on_draw;
		vumeter_vtable.onalign =
			(psy_ui_fp_component_event)
			vumeter_on_align;
		vumeter_vtable_initialized = TRUE;
	}
	psy_ui_component_set_vtable(&self->component, &vumeter_vtable);
}

/* implementation */
void vumeter_init(Vumeter* self, psy_ui_Component* parent)
{					
	psy_ui_component_init(&self->component, parent, NULL);
	vumeter_vtable_init(self);
	psy_ui_component_set_style_type(&self->component, STYLE_MAIN_VU);
	self->leftavg = self->rightavg = 0;	
	self->l_log = self->r_log = -10000;
	self->machine = NULL;
	self->channel_height = 5.0;
}

void vumeter_on_draw(Vumeter* self, psy_ui_Graphics* g)
{	
	psy_ui_RealRectangle left;
	psy_ui_RealRectangle right;	
	psy_ui_RealSize size;
	double vuprevL;
	double vuprevR;
	psy_ui_Colour bg;	
	psy_ui_Colour peak;
	
	size = psy_ui_component_scroll_size_px(&self->component);
	left = psy_ui_realrectangle_make(
		psy_ui_realpoint_make(0.0, 1.0),
		psy_ui_realsize_make(size.width, self->channel_height));
	right = left;
	right.top += self->channel_height + 1;
	right.bottom += self->channel_height + 1;	
	left.right = floor(self->leftavg * size.width); 
	right.right = floor(self->rightavg * size.width);	
	psy_ui_drawsolidrectangle(g, left, psy_ui_colour_gc());
	psy_ui_drawsolidrectangle(g, right, psy_ui_colour_gc());
	vuprevL = (40.0 + self->l_log) * size.width / 40.f;
	vuprevR = (40.0 + self->r_log) * size.width / 40.f;
	if (vuprevL > size.width) {
		vuprevL = size.width;
	}
	if (vuprevR > size.width) {
		vuprevR = size.width;
	}
	peak = psy_ui_style(STYLE_CLIPBOX_SELECT)->background.colour;
	if (vuprevL > left.left) {		
		left.left = left.right;
		left.right = vuprevL;
		psy_ui_drawsolidrectangle(g, left, peak);
	}
	if (vuprevR > right.left) {
		right.left = right.right;
		right.right = vuprevR;
		psy_ui_drawsolidrectangle(g, right, peak);
	}
	bg = psy_ui_component_background_colour(&self->component);
	left = psy_ui_realrectangle_make(
		psy_ui_realpoint_make(left.right, left.top),
		psy_ui_realsize_make(size.width - left.right, self->channel_height));
	right = psy_ui_realrectangle_make(
		psy_ui_realpoint_make(right.right, right.top),
		psy_ui_realsize_make(size.width - right.right, self->channel_height));
	psy_ui_drawsolidrectangle(g, left, bg);
	psy_ui_drawsolidrectangle(g, right, bg);
}

void vumeter_idle(Vumeter* self)
{			
	if (self->machine) {	
		psy_audio_Buffer* memory;
		psy_dsp_amp_t leftavg;
		psy_dsp_amp_t rightavg;
		
		memory = psy_audio_machine_buffermemory(self->machine);
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

void vumeter_set_machine(Vumeter* self, psy_audio_Machine* machine)
{
	assert(self);
		
	self->machine = machine;
}

void vumeter_on_align(Vumeter* self)
{
	psy_ui_RealSize size;
	
	size = psy_ui_component_scroll_size_px(&self->component);
	self->channel_height = psy_max(1.0, size.height / 2.0 - 2);
}

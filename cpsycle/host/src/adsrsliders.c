/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "adsrsliders.h"
/* dsp */
#include <valuemapper.h>
/* std */
#include <math.h>
/* platform */
#include "../../detail/portable.h"


/* prototypes */
static void adsrsliders_on_destroyed(AdsrSliders*);
static void adsrsliders_on_volume_view_describe(AdsrSliders*,
	psy_ui_Slider*, char* txt);
static void adsrsliders_on_volume_view_tweak(AdsrSliders*,
	psy_ui_Slider*, float value);
static void adsrsliders_on_volume_view_value(AdsrSliders*,
	psy_ui_Slider*, float* value);

/* vtable */
static psy_ui_ComponentVtable vtable;
static bool vtable_initialized = FALSE;

static void vtable_init(AdsrSliders* self)
{
	if (!vtable_initialized) {
		vtable = *(self->component.vtable);
		vtable.on_destroyed =
			(psy_ui_fp_component)
			adsrsliders_on_destroyed;
		vtable_initialized = TRUE;
	}
	psy_ui_component_set_vtable(&self->component, &vtable);
}

/* implementation */
void adsrsliders_init(AdsrSliders* self, psy_ui_Component* parent)
{	
	int i;
	psy_ui_Slider* sliders[] = {
		&self->attack,
		&self->decay,
		&self->sustain,
		&self->release,		
	};
	self->envelope = NULL;	
	psy_ui_component_init(&self->component, parent, NULL);
	vtable_init(self);	
	psy_ui_component_set_default_align(&self->component, psy_ui_ALIGN_TOP,
		psy_ui_defaults_vmargin(psy_ui_defaults()));	
	psy_ui_slider_init(&self->attack, &self->component);
	psy_ui_slider_set_text(&self->attack, "instrumentview.attack");
	psy_ui_slider_init(&self->decay, &self->component);
	psy_ui_slider_set_text(&self->decay, "instrumentview.decay");
	psy_ui_slider_init(&self->sustain, &self->component);
	psy_ui_slider_set_text(&self->sustain, "instrumentview.sustain-level");
	psy_ui_slider_setdefaultvalue(&self->sustain, 1.0);
	psy_ui_slider_init(&self->release, &self->component);
	psy_ui_slider_set_text(&self->release, "instrumentview.release");
	psy_ui_slider_setdefaultvalue(&self->attack, 0.005 /
		adsrsliders_max_time(self));
	psy_ui_slider_setdefaultvalue(&self->decay, 0.005 /
		adsrsliders_max_time(self));
	psy_ui_slider_setdefaultvalue(&self->sustain, 1.0);
	psy_ui_slider_setdefaultvalue(&self->release, 0.005 /
		adsrsliders_max_time(self));
	for (i = 0; i < 4; ++i) {				
		psy_ui_slider_setcharnumber(sliders[i], 21);
		psy_ui_slider_setvaluecharnumber(sliders[i], 15);
		psy_ui_slider_connect(sliders[i], self,
			(ui_slider_fpdescribe)adsrsliders_on_volume_view_describe,
			(ui_slider_fptweak)adsrsliders_on_volume_view_tweak,
			(ui_slider_fpvalue)adsrsliders_on_volume_view_value);
	}
	psy_signal_init(&self->signal_tweaked);
}

void adsrsliders_on_destroyed(AdsrSliders* self)
{
	psy_signal_dispose(&self->signal_tweaked);
}

void adsrsliders_set_envelope(AdsrSliders* self,
	psy_dsp_Envelope* envelope)
{	
	self->envelope = envelope;
	adsrsliders_update(self);
}

void adsrsliders_update(AdsrSliders* self)
{
	psy_ui_slider_update(&self->attack);
	psy_ui_slider_update(&self->decay);
	psy_ui_slider_update(&self->sustain);
	psy_ui_slider_update(&self->release);
}

void adsrsliders_on_volume_view_describe(AdsrSliders* self,
	psy_ui_Slider* slider, char* txt)
{
	if (slider == &self->attack) {
		if (self->envelope) {			
			if (self->envelope->timemode == psy_dsp_ENVELOPETIME_TICK) {
				psy_snprintf(txt, 20, "%dt", (int)
					psy_dsp_envelope_attacktime(self->envelope));
			} else {
				psy_snprintf(txt, 20, "%.4fms", (float)
					psy_dsp_envelope_attacktime(self->envelope) * 1000);
			}
		} else {			
			psy_snprintf(txt, 10, "0ms");
		}		
	} else if (slider == &self->decay) {
		if (self->envelope) {
			if (self->envelope->timemode == psy_dsp_ENVELOPETIME_TICK) {
				psy_snprintf(txt, 20, "%dt", (int)
					psy_dsp_envelope_decaytime(self->envelope));
			} else {
				psy_snprintf(txt, 20, "%.4fms", (float)
					psy_dsp_envelope_decaytime(self->envelope) * 1000);
			}
		} else {			
			psy_snprintf(txt, 10, "0ms");
		}		
	} else if (slider == &self->sustain) {
		if (self->envelope) {
			psy_snprintf(txt, 20, "%d%%", (int)(
				psy_dsp_envelope_sustainvalue(self->envelope) * 100));	
		} else {			
			psy_snprintf(txt, 10, "0%%");
		}		
	} else
	if (slider == &self->release) {
		if (self->envelope) {
			if (self->envelope->timemode == psy_dsp_ENVELOPETIME_TICK) {
				psy_snprintf(txt, 20, "%dt", (int)
					psy_dsp_envelope_releasetime(self->envelope));
			} else {
				psy_snprintf(txt, 20, "%.4fms", (float)
					psy_dsp_envelope_releasetime(self->envelope) * 1000);
			}
		} else {			
			psy_snprintf(txt, 10, "0ms");
		}		
	}
}

void adsrsliders_on_volume_view_tweak(AdsrSliders* self,
	psy_ui_Slider* slider, float value)
{
	if (!self->envelope) {
		return;
	}
	if (slider == &self->attack) {
		psy_dsp_envelope_setattacktime(self->envelope,
			(psy_dsp_seconds_t)(value * adsrsliders_max_time(self)));
		psy_signal_emit(&self->signal_tweaked, self, 1, 1);
	} else if (slider == &self->decay) {
		psy_dsp_envelope_setdecaytime(self->envelope,
			((psy_dsp_seconds_t)(value * adsrsliders_max_time(self))));
		psy_signal_emit(&self->signal_tweaked, self, 1, 2);
	} else if (slider == &self->sustain) {
		psy_dsp_envelope_setsustainvalue(self->envelope, value);
		psy_signal_emit(&self->signal_tweaked, self, 1, 2);
	} else if (slider == &self->release) {
		psy_dsp_envelope_setreleasetime(self->envelope,
			(psy_dsp_seconds_t)(value * adsrsliders_max_time(self)));
		psy_signal_emit(&self->signal_tweaked, self, 1, 3);
	}	
}

void adsrsliders_on_volume_view_value(AdsrSliders* self,
	psy_ui_Slider* slider, float* value)
{
	if (slider == &self->attack) {
		if (self->envelope) {
			*value = (psy_dsp_seconds_t)
				(psy_dsp_envelope_attacktime(self->envelope) /
					adsrsliders_max_time(self));
		} else {
			*value = 0.f;
		}
	} else if (slider == &self->decay) {
		if (self->envelope) {
			*value = (psy_dsp_seconds_t)
				(psy_dsp_envelope_decaytime(self->envelope) /
					adsrsliders_max_time(self));
		} else {
			*value = 0.f;
		}	
	} else if (slider == &self->sustain) {
		if (self->envelope) {
			*value = psy_dsp_envelope_sustainvalue(self->envelope);
		} else {
			*value = 0.5f;
		}	
	} else if (slider == &self->release) {
		if (self->envelope) {
			*value = (psy_dsp_seconds_t)
				(psy_dsp_envelope_releasetime(self->envelope) /
					adsrsliders_max_time(self));
		} else {
			*value = 0.5f;
		}	
	}
}

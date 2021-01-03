// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "adsrsliders.h"
// dsp
#include <valuemapper.h>
// std
#include <math.h>
// platform
#include "../../detail/portable.h"

// prototypes
static void adsrsliders_ondestroy(AdsrSliders*, psy_ui_Component* sender);
static void adsrsliders_onvolumeviewdescribe(AdsrSliders*,
	psy_ui_Slider*, char* txt);
static void adsrsliders_onvolumeviewtweak(AdsrSliders*,
	psy_ui_Slider*, float value);
static void adsrsliders_onvolumeviewvalue(AdsrSliders*,
	psy_ui_Slider*, float* value);
// implementation
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
	psy_ui_component_init(&self->component, parent);
	psy_signal_connect(&self->component.signal_destroyed, self,
		adsrsliders_ondestroy);
	psy_ui_component_setdefaultalign(&self->component, psy_ui_ALIGN_TOP,
		psy_ui_defaults_vmargin(psy_ui_defaults()));	
	psy_ui_slider_init(&self->attack, &self->component);
	psy_ui_slider_settext(&self->attack, "instrumentview.attack");
	psy_ui_slider_init(&self->decay, &self->component);
	psy_ui_slider_settext(&self->decay, "instrumentview.decay");
	psy_ui_slider_init(&self->sustain, &self->component);
	psy_ui_slider_settext(&self->sustain, "instrumentview.sustain-level");
	psy_ui_slider_init(&self->release, &self->component);
	psy_ui_slider_settext(&self->release, "instrumentview.release");	
	for (i = 0; i < 4; ++i) {				
		psy_ui_slider_setcharnumber(sliders[i], 21);
		psy_ui_slider_setvaluecharnumber(sliders[i], 15);
		psy_ui_slider_connect(sliders[i], self,
			(ui_slider_fpdescribe)adsrsliders_onvolumeviewdescribe,
			(ui_slider_fptweak)adsrsliders_onvolumeviewtweak,
			(ui_slider_fpvalue)adsrsliders_onvolumeviewvalue);
	}
	psy_signal_init(&self->signal_tweaked);
}

void adsrsliders_ondestroy(AdsrSliders* self, psy_ui_Component* sender)
{
	psy_signal_dispose(&self->signal_tweaked);
}

void adsrsliders_setenvelope(AdsrSliders* self,
	psy_dsp_Envelope* envelope)
{	
	self->envelope = envelope;
}

void adsrsliders_onvolumeviewdescribe(AdsrSliders* self,
	psy_ui_Slider* slider, char* txt)
{
	if (slider == &self->attack) {
		if (self->envelope) {			
			psy_snprintf(txt, 20, "%.4fms", (float)
				psy_dsp_envelope_attacktime(self->envelope) * 1000);
		} else {			
			psy_snprintf(txt, 10, "0ms");
		}		
	} else if (slider == &self->decay) {
		if (self->envelope) {			
			psy_snprintf(txt, 20, "%.4fms", (float)
				psy_dsp_envelope_decaytime(self->envelope) * 1000);
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
			psy_snprintf(txt, 20, "%.4fms", (float)
				psy_dsp_envelope_releasetime(self->envelope) * 1000);
		} else {			
			psy_snprintf(txt, 10, "0ms");
		}		
	}
}

void adsrsliders_onvolumeviewtweak(AdsrSliders* self,
	psy_ui_Slider* slider, float value)
{
	if (!self->envelope) {
		return;
	}
	if (slider == &self->attack) {
		psy_dsp_envelope_setattacktime(self->envelope, value * 5.f);
		psy_signal_emit(&self->signal_tweaked, self, 1, 1);
	} else if (slider == &self->decay) {
		psy_dsp_envelope_setdecaytime(self->envelope, value * 5.f);
		psy_signal_emit(&self->signal_tweaked, self, 1, 2);
	} else if (slider == &self->sustain) {
		psy_dsp_envelope_setsustainvalue(self->envelope, value);
		psy_signal_emit(&self->signal_tweaked, self, 1, 2);
	} else if (slider == &self->release) {
		psy_dsp_envelope_setreleasetime(self->envelope, value * 5.f);
		psy_signal_emit(&self->signal_tweaked, self, 1, 3);
	}	
}

void adsrsliders_onvolumeviewvalue(AdsrSliders* self,
	psy_ui_Slider* slider, float* value)
{
	if (slider == &self->attack) {
		if (self->envelope) {
			*value = psy_dsp_envelope_attacktime(self->envelope) / 5.f;
		} else {
			*value = 0.f;
		}
	} else if (slider == &self->decay) {
		if (self->envelope) {
			*value = psy_dsp_envelope_decaytime(self->envelope) / 5.f;
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
			*value = psy_dsp_envelope_releasetime(self->envelope) / 5.f;
		} else {
			*value = 0.5f;
		}	
	}
}

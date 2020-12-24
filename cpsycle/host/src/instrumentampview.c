// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "instrumentampview.h"
// dsp
#include <valuemapper.h>
// std
#include <math.h>
// platform
#include "../../detail/portable.h"

// prototypes
static void instrumentvolumeview_onvolumeviewdescribe(InstrumentVolumeView*,
	psy_ui_Slider*, char* txt);
static void instrumentvolumeview_onvolumeviewtweak(InstrumentVolumeView*,
	psy_ui_Slider*, float value);
static void instrumentvolumeview_onvolumeviewvalue(InstrumentVolumeView*,
	psy_ui_Slider*, float* value);
static void instrumentvolumeview_onadsrtweaked(InstrumentVolumeView*,
	AdsrSliders*);
// implementation
void instrumentvolumeview_init(InstrumentVolumeView* self, 
	psy_ui_Component* parent, psy_audio_Instruments* instruments, 
	Workspace* workspace)
{	
	int i;
	psy_ui_Slider* sliders[] = {		
		&self->randomvolume,
		&self->volumefadespeed
	};
	psy_ui_Margin margin;

	self->instruments = instruments;	
	self->instrument = 0;
	psy_ui_component_init(&self->component, parent);	
	psy_ui_component_setdefaultalign(&self->component, psy_ui_ALIGN_TOP,
		psy_ui_defaults_vmargin(psy_ui_defaults()));
	psy_ui_slider_init(&self->randomvolume, &self->component);
	margin = psy_ui_defaults_vmargin(psy_ui_defaults());
	margin.top = psy_ui_value_makeeh(1.0);
	psy_ui_component_setmargin(&self->randomvolume.component, &margin);
	psy_ui_slider_settext(&self->randomvolume, "Swing (Randomize)");
	psy_ui_slider_init(&self->volumefadespeed, &self->component);
	psy_ui_slider_settext(&self->volumefadespeed, "Fadeout");
	envelopeview_init(&self->envelopeview, &self->component, workspace);
	envelopeview_settext(&self->envelopeview, "instrumentview.amplitude-envelope");	
	psy_ui_component_setmargin(&self->envelopeview.component, &margin);
	adsrsliders_init(&self->adsrsliders, &self->component);
	psy_signal_connect(&self->adsrsliders.signal_tweaked, self,
		instrumentvolumeview_onadsrtweaked);
	for (i = 0; i < 2; ++i) {				
		psy_ui_slider_setcharnumber(sliders[i], 21);
		psy_ui_slider_setvaluecharnumber(sliders[i], 15);
		psy_ui_slider_connect(sliders[i], self,
			(ui_slider_fpdescribe)instrumentvolumeview_onvolumeviewdescribe,
			(ui_slider_fptweak)instrumentvolumeview_onvolumeviewtweak,
			(ui_slider_fpvalue)instrumentvolumeview_onvolumeviewvalue);
	}		
}

void instrumentvolumeview_setinstrument(InstrumentVolumeView* self,
	psy_audio_Instrument* instrument)
{	
	self->instrument = instrument;
	adsrsliders_setenvelope(&self->adsrsliders,
		(self->instrument)
		? &self->instrument->volumeenvelope
		: NULL);
	envelopeview_setenvelope(&self->envelopeview,
		(instrument)
			? &instrument->volumeenvelope
			: NULL);
}

void instrumentvolumeview_onvolumeviewdescribe(InstrumentVolumeView* self,
	psy_ui_Slider* slider, char* txt)
{	
	if (slider == &self->randomvolume) {
		if (self->instrument && self->instrument->randomvolume != 0.f) {
			psy_snprintf(txt, 20, "%d%%",
				(int)(self->instrument->randomvolume * 100));				
		} else {
			psy_snprintf(txt, 20, "off");
		}		
	} else if (slider == &self->volumefadespeed) {
		if (self->instrument && self->instrument->volumefadespeed != 0.f) {			
			psy_snprintf(txt, 20, "%d%%",
				(int)(self->instrument->volumefadespeed * 100));
		} else {			
			psy_snprintf(txt, 10, "off");
		}
	}
}

void instrumentvolumeview_onvolumeviewtweak(InstrumentVolumeView* self,
	psy_ui_Slider* slider, float value)
{
	if (!self->instrument) {
		return;
	}
	if (slider == &self->randomvolume) {
		self->instrument->randomvolume = value;
	} else if (slider == &self->volumefadespeed) {
		self->instrument->volumefadespeed = value;
	}
	envelopeview_update(&self->envelopeview);
}

void instrumentvolumeview_onvolumeviewvalue(InstrumentVolumeView* self,
	psy_ui_Slider* slider, float* value)
{
	if (slider == &self->randomvolume) {
		*value = self->instrument
			? self->instrument->randomvolume
			: 0.0f;
	} else if (slider == &self->volumefadespeed) {
		*value = self->instrument
			? self->instrument->volumefadespeed
			: 0.0f;
	}
}

void instrumentvolumeview_onadsrtweaked(InstrumentVolumeView* self,
	AdsrSliders* sender)
{
	envelopeview_update(&self->envelopeview);
}

// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "instrumentampview.h"
// dsp
#include <valuemapper.h>
// std
#include <math.h>
// platform
#include "../../detail/portable.h"

// prototypes
static void instrumentvolumeview_ondestroy(InstrumentVolumeView*,
	psy_ui_Component* sender);
static void instrumentvolumeview_updatesliders(InstrumentVolumeView*);
static void instrumentvolumeview_ondescribe(InstrumentVolumeView*,
	psy_ui_Slider* sender, char* text);
static void instrumentvolumeview_ontweak(InstrumentVolumeView*,
	psy_ui_Slider* sender, float value);
static void instrumentvolumeview_onvalue(InstrumentVolumeView*,
	psy_ui_Slider* sender, float* value);
static void instrumentvolumeview_ontweaked(InstrumentVolumeView*,
	psy_ui_Component*, int pointindex);
static void instrumentvolumeview_onenvelopeviewtweaked(InstrumentVolumeView*,
	psy_ui_Component*, int pointindex);
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
	psy_ui_component_init(&self->component, parent, NULL);
	psy_signal_connect(&self->component.signal_destroy, self,
		instrumentvolumeview_ondestroy);
	psy_signal_init(&self->signal_status);
	psy_ui_component_setdefaultalign(&self->component, psy_ui_ALIGN_TOP,
		psy_ui_defaults_vmargin(psy_ui_defaults()));
	psy_ui_slider_init_text(&self->randomvolume, &self->component, NULL,
		"instrumentview.swing");
	psy_ui_slider_setdefaultvalue(&self->randomvolume, 0.0);
	margin = psy_ui_defaults_vmargin(psy_ui_defaults());
	margin.top = psy_ui_value_make_eh(1.0);
	psy_ui_component_setmargin(&self->randomvolume.component, margin);	
	psy_ui_slider_init_text(&self->volumefadespeed, &self->component, NULL,
		"instrumentview.fadeout");
	psy_ui_slider_setdefaultvalue(&self->volumefadespeed, 0.0);
	envelopeview_init(&self->envelopeview, &self->component);
	envelopeview_settext(&self->envelopeview, "instrumentview.amplitude-envelope");	
	psy_ui_component_setmargin(&self->envelopeview.component, margin);
	adsrsliders_init(&self->adsrsliders, &self->component);
	psy_signal_connect(&self->adsrsliders.signal_tweaked, self,
		instrumentvolumeview_ontweaked);
	psy_signal_connect(&self->envelopeview.signal_tweaked, self,
		instrumentvolumeview_onenvelopeviewtweaked);
	for (i = 0; i < 2; ++i) {				
		psy_ui_slider_setcharnumber(sliders[i], 21);
		psy_ui_slider_setvaluecharnumber(sliders[i], 15);
		psy_ui_slider_connect(sliders[i], self,
			(ui_slider_fpdescribe)instrumentvolumeview_ondescribe,
			(ui_slider_fptweak)instrumentvolumeview_ontweak,
			(ui_slider_fpvalue)instrumentvolumeview_onvalue);
	}
	instrumentvolumeview_updatesliders(self);
}

void instrumentvolumeview_ondestroy(InstrumentVolumeView* self,
	psy_ui_Component* sender)
{
	psy_signal_dispose(&self->signal_status);
}

void instrumentvolumeview_setinstrument(InstrumentVolumeView* self,
	psy_audio_Instrument* instrument)
{	
	self->instrument = instrument;
	if (self->instrument) {
		adsrsliders_setenvelope(&self->adsrsliders,			
			&self->instrument->volumeenvelope);
		envelopeview_setenvelope(&self->envelopeview,
			&instrument->volumeenvelope);
	} else {
		adsrsliders_setenvelope(&self->adsrsliders,	NULL);
		envelopeview_setenvelope(&self->envelopeview, NULL);
	}
	instrumentvolumeview_updatesliders(self);	
}

void instrumentvolumeview_updatesliders(InstrumentVolumeView* self)
{	
	psy_ui_slider_update(&self->randomvolume);
	psy_ui_slider_update(&self->volumefadespeed);
}

void instrumentvolumeview_ondescribe(InstrumentVolumeView* self,
	psy_ui_Slider* sender, char* text)
{	
	if (sender == &self->randomvolume) {
		if (self->instrument &&
				psy_audio_instrument_randomvolume_enabled(self->instrument)) {
			psy_snprintf(text, 20, "%d%%", (int)
				(psy_audio_instrument_randomvolume(self->instrument) * 100));
		} else {
			psy_snprintf(text, 20, "off");
		}		
	} else if (sender == &self->volumefadespeed) {
		if (self->instrument && self->instrument->volumefadespeed != 0.f) {			
			psy_snprintf(text, 20, "%d%%", (int)(
				psy_audio_instrument_volumefadespeed(self->instrument) * 100));
		} else {			
			psy_snprintf(text, 10, "off");
		}
	}	
}

void instrumentvolumeview_ontweak(InstrumentVolumeView* self,
	psy_ui_Slider* sender, float value)
{
	if (!self->instrument) {
		return;
	}
	if (sender == &self->randomvolume) {
		psy_audio_instrument_setrandomvolume(self->instrument, value);
	} else if (sender == &self->volumefadespeed) {
		psy_audio_instrument_setvolumefadespeed(self->instrument, value);		
	}
	envelopeview_update(&self->envelopeview);
}

void instrumentvolumeview_onvalue(InstrumentVolumeView* self,
	psy_ui_Slider* sender, float* value)
{
	if (sender == &self->randomvolume) {
		*value = (self->instrument)
			? psy_audio_instrument_randomvolume(self->instrument)
			: 0.0f;
	} else if (sender == &self->volumefadespeed) {
		*value = (self->instrument)
			? psy_audio_instrument_volumefadespeed(self->instrument)
			: 0.0f;
	}
}

void instrumentvolumeview_ontweaked(InstrumentVolumeView* self,
	psy_ui_Component* sender, int pointindex)
{
	if (self->instrument) {
		char statustext[256];
		psy_dsp_EnvelopePoint pt;

		envelopeview_update(&self->envelopeview);
		pt = psy_dsp_envelope_at(&self->instrument->volumeenvelope, pointindex);
		psy_snprintf(statustext, 256, "Point %d (%f, %f)", pointindex,
			(float)pt.time, (float)pt.value);
		psy_signal_emit(&self->signal_status, self, 1, statustext);		
	}
}

void instrumentvolumeview_onenvelopeviewtweaked(InstrumentVolumeView* self,
	psy_ui_Component* sender, int pointindex)
{
	adsrsliders_update(&self->adsrsliders);
}

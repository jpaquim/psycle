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
static void instrumentvolumeview_updatetext(InstrumentVolumeView*);
static void instrumentvolumeview_onlanguagechanged(InstrumentVolumeView*, 
	psy_ui_Component* sender);
static void instrumentvolumeview_onvolumeviewdescribe(InstrumentVolumeView*,
	psy_ui_Slider*, char* txt);
static void instrumentvolumeview_onvolumeviewtweak(InstrumentVolumeView*,
	psy_ui_Slider*, float value);
static void instrumentvolumeview_onvolumeviewvalue(InstrumentVolumeView*,
	psy_ui_Slider*, float* value);
// implementation
void instrumentvolumeview_init(InstrumentVolumeView* self, 
	psy_ui_Component* parent, psy_audio_Instruments* instruments, 
	Workspace* workspace)
{	
	int i;
	psy_ui_Slider* sliders[] = {
		&self->attack,
		&self->decay,
		&self->sustain,
		&self->release,
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
	envelopeview_settext(&self->envelopeview, "Amplitude envelope");
	psy_ui_component_setmargin(&self->envelopeview.component, &margin);
	psy_ui_slider_init(&self->attack, &self->component);
	psy_ui_slider_settext(&self->attack, "instrumentview.attack");
	psy_ui_slider_init(&self->decay, &self->component);
	psy_ui_slider_settext(&self->decay, "instrumentview.decay");
	psy_ui_slider_init(&self->sustain, &self->component);
	psy_ui_slider_settext(&self->sustain, "instrumentview.sustain-level");
	psy_ui_slider_init(&self->release, &self->component);
	psy_ui_slider_settext(&self->release, "instrumentview.release");	
	for (i = 0; i < 6; ++i) {				
		psy_ui_slider_setcharnumber(sliders[i], 21);
		psy_ui_slider_setvaluecharnumber(sliders[i], 15);
		psy_ui_slider_connect(sliders[i], self,
			(ui_slider_fpdescribe)instrumentvolumeview_onvolumeviewdescribe,
			(ui_slider_fptweak)instrumentvolumeview_onvolumeviewtweak,
			(ui_slider_fpvalue)instrumentvolumeview_onvolumeviewvalue);
	}
	instrumentvolumeview_updatetext(self);
	psy_signal_connect(&self->component.signal_languagechanged, self,
		instrumentvolumeview_onlanguagechanged);
}

void instrumentvolumeview_updatetext(InstrumentVolumeView* self)
{
	envelopeview_settext(&self->envelopeview, psy_ui_translate(
		"instrumentview.amplitude-envelope"));	
}

void instrumentvolumeview_onlanguagechanged(InstrumentVolumeView* self, 
	psy_ui_Component* sender)
{
	instrumentvolumeview_updatetext(self);
}

void instrumentvolumeview_setinstrument(InstrumentVolumeView* self,
	psy_audio_Instrument* instrument)
{	
	self->instrument = instrument;
	envelopeview_setenvelope(&self->envelopeview,
		(instrument)
			? &instrument->volumeenvelope
			: NULL);
}

void instrumentvolumeview_onvolumeviewdescribe(InstrumentVolumeView* self,
	psy_ui_Slider* slider, char* txt)
{
	if (slider == &self->attack) {
		if (!self->instrument) {
			psy_snprintf(txt, 10, "0ms");
		} else {
			psy_dsp_EnvelopePoint pt;

			pt = psy_dsp_envelopesettings_at(&self->instrument->volumeenvelope,
				1);
			psy_snprintf(txt, 20, "%.4fms", pt.time * 1000);				
		}		
	} else if (slider == &self->decay) {
		if (!self->instrument) {
			psy_snprintf(txt, 10, "0ms");
		} else {
			psy_dsp_EnvelopePoint pt_start;
			psy_dsp_EnvelopePoint pt_end;

			pt_start = psy_dsp_envelopesettings_at(
				&self->instrument->volumeenvelope, 1);
			pt_end = psy_dsp_envelopesettings_at(
				&self->instrument->volumeenvelope, 2);
			psy_snprintf(txt, 20, "%.4fms",
				(pt_end.time - pt_start.time) * 1000);
		}		
	} else if (slider == &self->sustain) {
		if (!self->instrument) {
			psy_snprintf(txt, 10, "0%%");
		} else {
			psy_dsp_EnvelopePoint pt_sustain;

			pt_sustain = psy_dsp_envelopesettings_at(
				&self->instrument->volumeenvelope, 2);
			psy_snprintf(txt, 20, "%d%%", (int)(pt_sustain.value * 100));
		}		
	} else
	if (slider == &self->release) {
		if (!self->instrument) {
			psy_snprintf(txt, 10, "0ms");
		} else {
			psy_dsp_EnvelopePoint pt_start;
			psy_dsp_EnvelopePoint pt_end;

			pt_start = psy_dsp_envelopesettings_at(
				&self->instrument->volumeenvelope, 2);
			pt_end = psy_dsp_envelopesettings_at(
				&self->instrument->volumeenvelope, 3);
			psy_snprintf(txt, 20, "%.4fms",
				(pt_end.time - pt_start.time) * 1000);
		}		
	} else if (slider == &self->randomvolume) {
		if (!self->instrument) {
			psy_snprintf(txt, 10, "off");
		} else {
			if (self->instrument->randomvolume == 0.f) {
				psy_snprintf(txt, 20, "off");
			} else {
				psy_snprintf(txt, 20, "%d%%",
					(int)(self->instrument->randomvolume * 100));
			}
		}
	} else if (slider == &self->volumefadespeed) {
		if (!self->instrument || self->instrument->volumefadespeed == 0.f) {
			psy_snprintf(txt, 10, "off");
		} else {			
			psy_snprintf(txt, 20, "%d%%",
				(int)(self->instrument->volumefadespeed * 100));			
		}
	}
}

void instrumentvolumeview_onvolumeviewtweak(InstrumentVolumeView* self,
	psy_ui_Slider* slider, float value)
{
	if (!self->instrument) {
		return;
	}
	if (slider == &self->attack) {
		psy_dsp_envelopesettings_settime(&self->instrument->volumeenvelope,
			1, value * 5.f);
	} else if (slider == &self->decay) {
		psy_dsp_EnvelopePoint pt_start;

		pt_start = psy_dsp_envelopesettings_at(
			&self->instrument->volumeenvelope, 1);
		psy_dsp_envelopesettings_settime(
			&self->instrument->volumeenvelope,
			2, pt_start.time + value * 5.f);
	} else if (slider == &self->sustain) {
		psy_dsp_EnvelopePoint pt_start;

		pt_start = psy_dsp_envelopesettings_at(
			&self->instrument->volumeenvelope, 2);
		psy_dsp_envelopesettings_setvalue(&self->instrument->volumeenvelope,
			2, value);
	} else if (slider == &self->release) {
		psy_dsp_EnvelopePoint pt_start;

		pt_start = psy_dsp_envelopesettings_at(
			&self->instrument->volumeenvelope, 2);
		psy_dsp_envelopesettings_settime(&self->instrument->volumeenvelope,
			3, pt_start.time + value * 5.f);
	} else if (slider == &self->randomvolume) {
		self->instrument->randomvolume = value;
	} else if (slider == &self->volumefadespeed) {
		self->instrument->volumefadespeed = value;
	}
	envelopeview_update(&self->envelopeview);
}

void instrumentvolumeview_onvolumeviewvalue(InstrumentVolumeView* self,
	psy_ui_Slider* slider, float* value)
{
	if (slider == &self->attack) {
		if (self->instrument) {
			psy_dsp_EnvelopePoint pt;

			pt = psy_dsp_envelopesettings_at(&self->instrument->volumeenvelope,
				1);
			*value = pt.time / 5.f;
		} else {
			*value = 0.f;
		}
	} else if (slider == &self->decay) {
		if (self->instrument) {
			psy_dsp_EnvelopePoint pt_start;
			psy_dsp_EnvelopePoint pt_end;

			pt_start = psy_dsp_envelopesettings_at(
				&self->instrument->volumeenvelope, 1);
			pt_end = psy_dsp_envelopesettings_at(
				&self->instrument->volumeenvelope, 2);
			*value = (pt_end.time - pt_start.time) / 5.f;
		} else {
			*value = 0.f;
		}	
	} else if (slider == &self->sustain) {
		if (self->instrument) {
			psy_dsp_EnvelopePoint pt_sustain;			

			pt_sustain = psy_dsp_envelopesettings_at(
				&self->instrument->volumeenvelope, 2);
			*value = pt_sustain.value;
		} else {
			*value = 0.5f;
		}	
	} else if (slider == &self->release) {
		if (self->instrument) {
			psy_dsp_EnvelopePoint pt_start;
			psy_dsp_EnvelopePoint pt_end;

			pt_start = psy_dsp_envelopesettings_at(
				&self->instrument->volumeenvelope, 2);
			pt_end = psy_dsp_envelopesettings_at(
				&self->instrument->volumeenvelope, 3);
			*value = (pt_end.time - pt_start.time) / 5.f;
		} else {
			*value = 0.5f;
		}	
	} else if (slider == &self->randomvolume) {
		*value = self->instrument
			? self->instrument->randomvolume
			: 0.0f;
	} else if (slider == &self->volumefadespeed) {
		*value = self->instrument
			? self->instrument->volumefadespeed
			: 0.0f;
	}
}


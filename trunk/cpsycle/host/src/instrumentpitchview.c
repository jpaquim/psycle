// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "instrumentpitchview.h"
// dsp
#include <valuemapper.h>
// std
#include <math.h>
// platform
#include "../../detail/portable.h"

// prototypes
static void instrumentpitchview_updatetext(InstrumentPitchView*);
static void instrumentpitchview_onlanguagechanged(InstrumentPitchView*,
	psy_ui_Component* sender);
static void instrumentpitchview_onpitchviewdescribe(InstrumentPitchView*,
	psy_ui_Slider*, char* txt);
static void instrumentpitchview_onpitchviewtweak(InstrumentPitchView*,
	psy_ui_Slider*, float value);
static void instrumentpitchview_onpitchviewvalue(InstrumentPitchView*,
	psy_ui_Slider*, float* value);
// implementation
void instrumentpitchview_init(InstrumentPitchView* self,
	psy_ui_Component* parent, psy_audio_Instruments* instruments,
	Workspace* workspace)
{
	int i;
	psy_ui_Slider* sliders[] = {
		&self->attack,
		&self->decay,
		&self->sustain,
		&self->release,		
	};
	psy_ui_Margin margin;

	self->instruments = instruments;
	self->instrument = 0;
	psy_ui_component_init(&self->component, parent);
	psy_ui_component_setdefaultalign(&self->component, psy_ui_ALIGN_TOP,
		psy_ui_defaults_vmargin(psy_ui_defaults()));	
	margin = psy_ui_defaults_vmargin(psy_ui_defaults());
	margin.top = psy_ui_value_makeeh(1.0);		
	envelopeview_init(&self->envelopeview, &self->component, workspace);
	envelopeview_settext(&self->envelopeview, "Pitch envelope");
	psy_ui_component_setmargin(&self->envelopeview.component, &margin);
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
			(ui_slider_fpdescribe)instrumentpitchview_onpitchviewdescribe,
			(ui_slider_fptweak)instrumentpitchview_onpitchviewtweak,
			(ui_slider_fpvalue)instrumentpitchview_onpitchviewvalue);
	}
	instrumentpitchview_updatetext(self);
	psy_signal_connect(&self->component.signal_languagechanged, self,
		instrumentpitchview_onlanguagechanged);
}

void instrumentpitchview_updatetext(InstrumentPitchView* self)
{
	envelopeview_settext(&self->envelopeview, psy_ui_translate(
		"instrumentview.pitch-envelope"));
}

void instrumentpitchview_onlanguagechanged(InstrumentPitchView* self,
	psy_ui_Component* sender)
{
	instrumentpitchview_updatetext(self);
}

void instrumentpitchview_setinstrument(InstrumentPitchView* self,
	psy_audio_Instrument* instrument)
{
	self->instrument = instrument;
	envelopeview_setenvelope(&self->envelopeview,
		(instrument)
		? &instrument->pitchenvelope
		: NULL);
}

void instrumentpitchview_onpitchviewdescribe(InstrumentPitchView* self,
	psy_ui_Slider* slider, char* txt)
{
	if (slider == &self->attack) {
		if (!self->instrument) {
			psy_snprintf(txt, 10, "0ms");
		} else {
			psy_dsp_EnvelopePoint pt;

			pt = psy_dsp_envelopesettings_at(&self->instrument->pitchenvelope,
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
				&self->instrument->pitchenvelope, 1);
			pt_end = psy_dsp_envelopesettings_at(
				&self->instrument->pitchenvelope, 2);
			psy_snprintf(txt, 20, "%.4fms",
				(pt_end.time - pt_start.time) * 1000);
		}
	} else if (slider == &self->sustain) {
		if (!self->instrument) {
			psy_snprintf(txt, 10, "0%%");
		} else {
			psy_dsp_EnvelopePoint pt_sustain;

			pt_sustain = psy_dsp_envelopesettings_at(
				&self->instrument->pitchenvelope, 2);
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
					&self->instrument->pitchenvelope, 2);
				pt_end = psy_dsp_envelopesettings_at(
					&self->instrument->pitchenvelope, 3);
				psy_snprintf(txt, 20, "%.4fms",
					(pt_end.time - pt_start.time) * 1000);
			}
		}
}

void instrumentpitchview_onpitchviewtweak(InstrumentPitchView* self,
	psy_ui_Slider* slider, float value)
{
	if (!self->instrument) {
		return;
	}
	if (slider == &self->attack) {
		psy_dsp_envelopesettings_settime(&self->instrument->pitchenvelope,
			1, value * 5.f);
	} else if (slider == &self->decay) {
		psy_dsp_EnvelopePoint pt_start;

		pt_start = psy_dsp_envelopesettings_at(
			&self->instrument->pitchenvelope, 1);
		psy_dsp_envelopesettings_settime(
			&self->instrument->pitchenvelope,
			2, pt_start.time + value * 5.f);
	} else if (slider == &self->sustain) {
		psy_dsp_EnvelopePoint pt_start;

		pt_start = psy_dsp_envelopesettings_at(
			&self->instrument->pitchenvelope, 2);
		psy_dsp_envelopesettings_setvalue(&self->instrument->pitchenvelope,
			2, value);
	} else if (slider == &self->release) {
		psy_dsp_EnvelopePoint pt_start;

		pt_start = psy_dsp_envelopesettings_at(
			&self->instrument->pitchenvelope, 2);
		psy_dsp_envelopesettings_settime(&self->instrument->pitchenvelope,
			3, pt_start.time + value * 5.f);
	}
	envelopeview_update(&self->envelopeview);
}

void instrumentpitchview_onpitchviewvalue(InstrumentPitchView* self,
	psy_ui_Slider* slider, float* value)
{
	if (slider == &self->attack) {
		if (self->instrument) {
			psy_dsp_EnvelopePoint pt;

			pt = psy_dsp_envelopesettings_at(&self->instrument->pitchenvelope,
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
				&self->instrument->pitchenvelope, 1);
			pt_end = psy_dsp_envelopesettings_at(
				&self->instrument->pitchenvelope, 2);
			*value = (pt_end.time - pt_start.time) / 5.f;
		} else {
			*value = 0.f;
		}
	} else if (slider == &self->sustain) {
		if (self->instrument) {
			psy_dsp_EnvelopePoint pt_sustain;

			pt_sustain = psy_dsp_envelopesettings_at(
				&self->instrument->pitchenvelope, 2);
			*value = pt_sustain.value;
		} else {
			*value = 0.5f;
		}
	} else if (slider == &self->release) {
		if (self->instrument) {
			psy_dsp_EnvelopePoint pt_start;
			psy_dsp_EnvelopePoint pt_end;

			pt_start = psy_dsp_envelopesettings_at(
				&self->instrument->pitchenvelope, 2);
			pt_end = psy_dsp_envelopesettings_at(
				&self->instrument->pitchenvelope, 3);
			*value = (pt_end.time - pt_start.time) / 5.f;
		} else {
			*value = 0.5f;
		}
	}
}

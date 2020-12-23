// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "instrumentpanview.h"
// dsp
#include <valuemapper.h>
// std
#include <math.h>
// platform
#include "../../detail/portable.h"

// prototypes
static void instrumentpanview_oninstpanenabled(InstrumentPanView*,
	psy_ui_CheckBox* sender);
static void instrumentpanview_ondescribe(InstrumentPanView*,
	psy_ui_Slider*, char* text);
static void instrumentpanview_ontweak(InstrumentPanView*,
	psy_ui_Slider*, float value);
static void instrumentpanview_onvalue(InstrumentPanView*,
	psy_ui_Slider*, float* value);
// implementation
void instrumentpanview_init(InstrumentPanView* self, psy_ui_Component* parent,
	psy_audio_Instruments* instruments, Workspace* workspace)
{
	int i;
	psy_ui_Slider* sliders[] = {	
		&self->randompanning,
		&self->notemodcenternote,
		&self->notemodamount
	};
	psy_ui_Margin margin;

	self->instrument = 0;
	self->instruments = instruments;
	self->workspace = workspace;
	psy_ui_component_init(&self->component, parent);
	psy_ui_component_setdefaultalign(&self->component, psy_ui_ALIGN_TOP,
		psy_ui_defaults_vmargin(psy_ui_defaults()));
	psy_ui_component_init(&self->top, &self->component);
	psy_ui_component_setdefaultalign(&self->top, psy_ui_ALIGN_TOP,
		psy_ui_defaults_vmargin(psy_ui_defaults()));
	psy_ui_component_init(&self->instpan, &self->top);
	margin = psy_ui_defaults_vmargin(psy_ui_defaults());
	margin.top = psy_ui_value_makeeh(1.0);
	psy_ui_component_setmargin(&self->instpan, &margin);
	psy_ui_component_setdefaultalign(&self->instpan, psy_ui_ALIGN_LEFT,
		psy_ui_defaults_hmargin(psy_ui_defaults()));
	psy_ui_checkbox_init(&self->instpanenabled, &self->instpan);
	psy_signal_connect(&self->instpanenabled.signal_clicked, self,
		instrumentpanview_oninstpanenabled);
	psy_ui_checkbox_settext(&self->instpanenabled, "Instrument Pan");
	psy_ui_component_setminimumsize(&self->instpanenabled.component, psy_ui_size_makeem(22, 1.0));
	psy_ui_slider_init(&self->instpanning, &self->instpan);
	psy_ui_slider_connect(&self->instpanning, self,
		(ui_slider_fpdescribe)instrumentpanview_ondescribe,
		(ui_slider_fptweak)instrumentpanview_ontweak,
		(ui_slider_fpvalue)instrumentpanview_onvalue);
	psy_ui_component_setminimumsize(&self->instpanning.component,
		psy_ui_size_makeem(50.0, 1.0));
	psy_ui_slider_showhorizontal(&self->instpanning);

	psy_ui_slider_init(&self->randompanning, &self->top);
	psy_ui_slider_settext(&self->randompanning,
		"Swing (Randomize)");
	psy_ui_slider_init(&self->notemodcenternote, &self->top);
	psy_ui_slider_settext(&self->notemodcenternote,
		"Note Mod Center Note");
	psy_ui_slider_init(&self->notemodamount, &self->top);
	psy_ui_slider_settext(&self->notemodamount,
		"Note Mod Amount");

	envelopeview_init(&self->envelopeview, &self->component, workspace);
	envelopeview_settext(&self->envelopeview,
		psy_ui_translate("instrumentview.pan-envelope"));
	psy_ui_component_setalign(&self->envelopeview.component,
		psy_ui_ALIGN_CLIENT);	
	margin = psy_ui_defaults_vmargin(psy_ui_defaults());
	margin.top = psy_ui_value_makeeh(1.0);
	for (i = 0; i < 3; ++i) {
		psy_ui_slider_setcharnumber(sliders[i], 25);
		psy_ui_slider_setvaluecharnumber(sliders[i], 15);
		psy_ui_slider_connect(sliders[i], self,
			(ui_slider_fpdescribe)instrumentpanview_ondescribe,
			(ui_slider_fptweak)instrumentpanview_ontweak,
			(ui_slider_fpvalue)instrumentpanview_onvalue);
	}
}

void instrumentpanview_setinstrument(InstrumentPanView* self,
	psy_audio_Instrument* instrument)
{	
	self->instrument = instrument;
	if (self->instrument) {
		envelopeview_setenvelope(&self->envelopeview,
			&instrument->panenvelope);
		if (instrument->panenabled) {
			psy_ui_checkbox_check(&self->instpanenabled);
		} else {
			psy_ui_checkbox_disablecheck(&self->instpanenabled);
		}
	} else {
		envelopeview_setenvelope(&self->envelopeview, NULL);
		psy_ui_checkbox_disablecheck(&self->instpanenabled);
	}
}

void instrumentpanview_oninstpanenabled(InstrumentPanView* self,
	psy_ui_CheckBox* sender)
{
	if (!self->instrument) {
		return;
	}
	if (psy_ui_checkbox_checked(sender)) {
		self->instrument->panenabled = TRUE;
	} else {
		self->instrument->panenabled = FALSE;
	}
}

void instrumentpanview_ondescribe(InstrumentPanView* self,
	psy_ui_Slider* slider, char* txt)
{	
	if (slider == &self->randompanning) {
		if (!self->instrument || self->instrument->randompanning == 0.f) {
			psy_snprintf(txt, 10, "off");
		} else {
			psy_snprintf(txt, 20, "%d%%",
				(int)(self->instrument->randompanning * 100));			
		}
	} else if (slider == &self->instpanning) {
		int pos;
		
		if (self->instrument) {
			pos = (int)(self->instrument->initpan * 128);
		} else {
			pos = 0;
		}
		if (pos == 0) psy_snprintf(txt, 64, "||%02d  ", pos);
		else if (pos < 32) psy_snprintf(txt, 64, "<<%02d  ", pos);
		else if (pos < 64) psy_snprintf(txt, 64, " <%02d< ", pos);
		else if (pos == 64) psy_snprintf(txt, 64, " |%02d| ", pos);
		else if (pos <= 96) psy_snprintf(txt, 64, " >%02d> ", pos);
		else if (pos < 128) psy_snprintf(txt, 64, "  %02d>>", pos);
		else psy_snprintf(txt, 64, "  %02d||", pos);
	} else if (slider == &self->notemodcenternote) {
		char tmp[40];
		int pos;

		if (self->instrument) {
			pos = self->instrument->notemodpancenter;
		} else {
			pos = psy_audio_NOTECOMMANDS_MIDDLEC;
		}
		char notes[12][3] = { "C-","C#","D-","D#","E-","F-","F#","G-","G#","A-","A#","B-" };
		int offset = (workspace_notetabmode(self->workspace) == psy_dsp_NOTESTAB_A440)  ? -1 : 0;
		psy_snprintf(tmp, 40, "%s", notes[pos % 12]);
		psy_snprintf(txt, 64, "%s%d", tmp, offset + (pos / 12));
	} else if (slider == &self->notemodamount) {
		int pos;

		if (self->instrument) {
			pos = self->instrument->notemodpansep;
		} else {
			pos = 0;
		}		
		psy_snprintf(txt, 64, "%.02f%%", (pos / 2.56f));		
	}
}

void instrumentpanview_ontweak(InstrumentPanView* self, psy_ui_Slider* slider,
	float value)
{
	if (!self->instrument) {
		return;
	}
	if (slider == &self->randompanning) {
		self->instrument->randompanning = value;
	} else if (slider == &self->instpanning) {
		self->instrument->initpan = value;
	} else if (slider == &self->notemodcenternote) {
		self->instrument->notemodpancenter = (uint8_t)(value * 119);
	} else if (slider == &self->notemodamount) {
		self->instrument->notemodpansep = (int8_t)(value * 64) - 32;
	}
}

void instrumentpanview_onvalue(InstrumentPanView* self,
	psy_ui_Slider* slider, float* value)
{	
	if (slider == &self->randompanning) {
		*value = (self->instrument)
			? self->instrument->randompanning
			: 0.f;
	} else if (slider == &self->instpanning) {
		*value = (self->instrument)
			? self->instrument->initpan
			: 0.5f;	
	} else if (slider == &self->notemodcenternote) {
		*value = (self->instrument)
			? self->instrument->notemodpancenter / 119.f
			: psy_audio_NOTECOMMANDS_MIDDLEC / 119.f;
	} else if (slider == &self->notemodamount) {
		*value = (self->instrument)
			? ((self->instrument->notemodpansep) / 64.f) + 0.5f
			: 0.5f;
	}
}


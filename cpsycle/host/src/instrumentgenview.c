/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "instrumentgenview.h"
/* audio */
#include <exclusivelock.h>
#include <instruments.h>
#include <machinefactory.h>
#include <virtualgenerator.h>
/* dsp */
#include <valuemapper.h>
/* std */
#include <math.h>
/* platform */
#include "../../detail/portable.h"

/* prototypes */
static void instrumentgeneralview_onfitrow(InstrumentGeneralView*,
	psy_ui_Component* sender);
/* nna */
static void instrumentgeneralview_onnnacut(InstrumentGeneralView*);
static void instrumentgeneralview_onnnarelease(InstrumentGeneralView*);
static void instrumentgeneralview_onnnafadeout(InstrumentGeneralView*);
static void instrumentgeneralview_onnnanone(InstrumentGeneralView*);
static void instrumentgeneralview_nnahighlight(InstrumentGeneralView*, psy_ui_Button*);
/* dct */
static void instrumentgeneralview_ondctnone(InstrumentGeneralView*, psy_ui_Button* sender);	
static void instrumentgeneralview_ondctnote(InstrumentGeneralView*, psy_ui_Button* sender);
static void instrumentgeneralview_ondctsample(InstrumentGeneralView*, psy_ui_Button* sender);
static void instrumentgeneralview_ondctinstrument(InstrumentGeneralView*, psy_ui_Button* sender);
static void instrumentgeneralview_updatedct(InstrumentGeneralView*);
/* dca */
static void instrumentgeneralview_ondcacut(InstrumentGeneralView*);
static void instrumentgeneralview_ondcarelease(InstrumentGeneralView*);
static void instrumentgeneralview_ondcafadeout(InstrumentGeneralView*);
static void instrumentgeneralview_ondcanone(InstrumentGeneralView*);
static void instrumentgeneralview_updatedca(InstrumentGeneralView*);
/* slider */
static void instrumentgeneralview_updateslider(InstrumentGeneralView*);
static void instrumentgeneralview_ongeneralviewdescribe(InstrumentGeneralView*,
	psy_ui_Slider*, char* text);
static void instrumentgeneralview_ongeneralviewtweak(InstrumentGeneralView*,
	psy_ui_Slider*, float value);
static void instrumentgeneralview_ongeneralviewvalue(InstrumentGeneralView*,
	psy_ui_Slider*, float* value);

/* implementation */
void instrumentgeneralview_init(InstrumentGeneralView* self,
	psy_ui_Component* parent, psy_audio_Instruments* instruments,
	Workspace* workspace)
{
	psy_ui_Margin margin;
	
	psy_ui_component_init(&self->component, parent, NULL);
	psy_ui_component_set_default_align(&self->component, psy_ui_ALIGN_TOP,
		psy_ui_defaults_vmargin(psy_ui_defaults()));
	psy_ui_margin_init_em(&margin, 0.0, 2.0, 1.5, 0.0);
	self->instruments = instruments;
	self->instrument = NULL;
	/* nna */
	psy_ui_component_init(&self->nna, &self->component, NULL);
	psy_ui_component_set_default_align(&self->nna, psy_ui_ALIGN_LEFT,
		psy_ui_defaults_hmargin(psy_ui_defaults()));
	margin = psy_ui_defaults_vmargin(psy_ui_defaults());
	margin.top = psy_ui_value_make_eh(1.0);
	psy_ui_component_set_margin(&self->nna, margin);	
	psy_ui_label_init_text(&self->nnaheader, &self->nna,
		"instrumentview.new-note-action");
	psy_ui_button_init_connect(&self->nnacut, &self->nna,
		self, instrumentgeneralview_onnnacut);
	psy_ui_button_set_text(&self->nnacut, "instrumentview.note-cut");		
	psy_ui_button_init_connect(&self->nnarelease, &self->nna,
		self, instrumentgeneralview_onnnarelease);
	psy_ui_button_set_text(&self->nnarelease, "instrumentview.note-release");
	psy_ui_button_init_connect(&self->nnafadeout, &self->nna,
		self, instrumentgeneralview_onnnafadeout);
	psy_ui_button_set_text(&self->nnafadeout, "instrumentview.note-fadeout");
	psy_ui_button_init_connect(&self->nnanone, &self->nna,
		self, instrumentgeneralview_onnnanone);	
	psy_ui_button_set_text(&self->nnanone, "instrumentview.none");
	/* dct */
	psy_ui_component_init(&self->dct, &self->component, NULL);
	psy_ui_component_set_default_align(&self->dct, psy_ui_ALIGN_LEFT,
		psy_ui_defaults_hmargin(psy_ui_defaults()));	
	psy_ui_label_init_text(&self->dctheader, &self->dct,
		"instrumentview.same");
	psy_ui_button_init_connect(&self->dctnone, &self->dct,
		self, instrumentgeneralview_ondctnone);
	psy_ui_button_set_text(&self->dctnone, "instrumentview.none");
	psy_ui_button_init_connect(&self->dctnote, &self->dct,
		self, instrumentgeneralview_ondctnote);
	psy_ui_button_set_text(&self->dctnote, "Note");
	psy_ui_button_init_connect(&self->dctsample, &self->dct,
		self, instrumentgeneralview_ondctsample);
	psy_ui_button_set_text(&self->dctsample, "Sample");
	psy_ui_button_init_connect(&self->dctinstrument, &self->dct,
		self, instrumentgeneralview_ondctinstrument);
	psy_ui_button_set_text(&self->dctinstrument,
		"instrumentview.instrument");
	/* dca */
	psy_ui_label_init_text(&self->dcaheader, &self->dct,
		"instrumentview.do");
	psy_ui_button_init_connect(&self->dcacut, &self->dct,
		self, instrumentgeneralview_ondcacut);
	psy_ui_button_set_text(&self->dcacut, "instrumentview.note-cut");
	psy_ui_button_init_connect(&self->dcarelease, &self->dct,
		self, instrumentgeneralview_ondcarelease);
	psy_ui_button_set_text(&self->dcarelease, "instrumentview.note-release");
	psy_ui_button_init_connect(&self->dcafadeout, &self->dct,
		self, instrumentgeneralview_ondcafadeout);
	psy_ui_button_set_text(&self->dcafadeout, "instrumentview.note-fadeout");
	psy_ui_button_init_connect(&self->dcanone, &self->dct,
		self, instrumentgeneralview_ondcanone);
	psy_ui_button_set_text(&self->dcanone, "instrumentview.none");	
	/* fitrow */
	psy_ui_component_init(&self->fitrow, &self->component, NULL);
	margin = psy_ui_defaults_vmargin(psy_ui_defaults());
	margin.top = psy_ui_value_make_eh(1.0);
	psy_ui_component_set_margin(&self->fitrow, margin);
	psy_ui_component_set_default_align(&self->fitrow, psy_ui_ALIGN_LEFT,
		psy_ui_defaults_hmargin(psy_ui_defaults()));
	psy_ui_checkbox_init(&self->fitrowcheck, &self->fitrow);
	psy_signal_connect(&self->fitrowcheck.signal_clicked, self,
		instrumentgeneralview_onfitrow);
	psy_ui_textarea_init_single_line(&self->fitrowedit, &self->fitrow);
	psy_ui_textarea_set_char_number(&self->fitrowedit, 4);
	psy_ui_label_init_text(&self->fitrowlabel, &self->fitrow,
		"instrumentview.pattern-rows");
	psy_ui_slider_init(&self->globalvolume, &self->component);
	psy_ui_slider_connect(&self->globalvolume, self,
		(ui_slider_fpdescribe)instrumentgeneralview_ongeneralviewdescribe,
		(ui_slider_fptweak)instrumentgeneralview_ongeneralviewtweak,
		(ui_slider_fpvalue)instrumentgeneralview_ongeneralviewvalue);
	instrumentnotemapview_init(&self->notemapview, &self->component, workspace);
	psy_ui_component_set_align(&self->notemapview.component, psy_ui_ALIGN_CLIENT);
	psy_ui_checkbox_settext(&self->fitrowcheck,
		"instrumentview.play-sample-to-fit");
	psy_ui_slider_set_text(&self->globalvolume,
		"instrumentview.global-volume");
	instrumentgeneralview_updateslider(self);
}

void instrumentgeneralview_setinstrument(InstrumentGeneralView* self,
	psy_audio_Instrument* instrument)
{	
	self->instrument = instrument;		
	instrumentnotemapview_setinstrument(&self->notemapview, instrument);
	if (instrument) {
		char text[128];

		if (self->instrument->loop) {
			psy_ui_checkbox_check(&self->fitrowcheck);
		} else {
			psy_ui_checkbox_disablecheck(&self->fitrowcheck);
		}
		psy_snprintf(text, 128, "%d", (int)self->instrument->lines);
		psy_ui_textarea_set_text(&self->fitrowedit, text);
		switch (self->instrument->nna) {
			case psy_audio_NNA_STOP:
				instrumentgeneralview_nnahighlight(self, &self->nnacut);
			break;
			case psy_audio_NNA_NOTEOFF:
				instrumentgeneralview_nnahighlight(self, &self->nnarelease);
			break;
			case psy_audio_NNA_FADEOUT:
				instrumentgeneralview_nnahighlight(self, &self->nnafadeout);
			break;
			case psy_audio_NNA_CONTINUE:
				instrumentgeneralview_nnahighlight(self, &self->nnanone);
			break;
			default:
				instrumentgeneralview_nnahighlight(self, &self->nnacut);
			break;
		}		
	} else {
		instrumentgeneralview_nnahighlight(self, &self->nnacut);
	}
	instrumentgeneralview_updatedct(self);
	instrumentgeneralview_updatedca(self);
	instrumentgeneralview_updateslider(self);
}

void instrumentgeneralview_updateslider(InstrumentGeneralView* self)
{
	psy_ui_slider_update(&self->globalvolume);
}

void instrumentgeneralview_onfitrow(InstrumentGeneralView* self,
	psy_ui_Component* sender)
{
	if (self->instrument) {
		if (psy_ui_checkbox_checked(&self->fitrowcheck)) {
			self->instrument->loop = TRUE;
		} else {
			self->instrument->loop = FALSE;
		}
	}
}

void instrumentgeneralview_onnnacut(InstrumentGeneralView* self)
{
	if (self->instrument) {
		psy_audio_instrument_setnna(self->instrument, psy_audio_NNA_STOP);
		instrumentgeneralview_nnahighlight(self, &self->nnacut);
	}
}

void instrumentgeneralview_onnnarelease(InstrumentGeneralView* self)
{
	if (self->instrument) {
		psy_audio_instrument_setnna(self->instrument, psy_audio_NNA_NOTEOFF);
		instrumentgeneralview_nnahighlight(self, &self->nnarelease);
	}
}

void instrumentgeneralview_onnnafadeout(InstrumentGeneralView* self)
{
	if (self->instrument) {
		psy_audio_instrument_setnna(self->instrument, psy_audio_NNA_FADEOUT);		
		instrumentgeneralview_nnahighlight(self, &self->nnafadeout);
	}
}

void instrumentgeneralview_onnnanone(InstrumentGeneralView* self)
{
	if (self->instrument) {
		psy_audio_instrument_setnna(self->instrument, psy_audio_NNA_CONTINUE);
		instrumentgeneralview_nnahighlight(self, &self->nnanone);
	}
}

void instrumentgeneralview_nnahighlight(InstrumentGeneralView* self,
	psy_ui_Button* highlight)
{
	psy_ui_button_highlight(highlight);
	if (highlight != &self->nnacut) {
		psy_ui_button_disable_highlight(&self->nnacut);
	}
	if (highlight != &self->nnafadeout) {
		psy_ui_button_disable_highlight(&self->nnafadeout);
	}
	if (highlight != &self->nnarelease) {
		psy_ui_button_disable_highlight(&self->nnarelease);
	}
	if (highlight != &self->nnanone) {
		psy_ui_button_disable_highlight(&self->nnanone);
	}	
}

/* dct */
void instrumentgeneralview_ondctnone(InstrumentGeneralView* self,
	psy_ui_Button* sender)
{
	if (self->instrument) {
		self->instrument->dct = psy_audio_DUPECHECK_NONE;
	}
	instrumentgeneralview_updatedct(self);
}

void instrumentgeneralview_ondctnote(InstrumentGeneralView* self,
	psy_ui_Button* sender)
{
	if (self->instrument) {
		self->instrument->dct = psy_audio_DUPECHECK_NOTE;
	}
	instrumentgeneralview_updatedct(self);
}

void instrumentgeneralview_ondctsample(InstrumentGeneralView* self,
	psy_ui_Button* sender)
{
	if (self->instrument) {
		self->instrument->dct = psy_audio_DUPECHECK_SAMPLE;		
	}
	instrumentgeneralview_updatedct(self);
}

void instrumentgeneralview_ondctinstrument(InstrumentGeneralView* self,
	psy_ui_Button* sender)
{
	if (self->instrument) {
		self->instrument->dct = psy_audio_DUPECHECK_INSTRUMENT;
	}
	instrumentgeneralview_updatedct(self);
}

void instrumentgeneralview_updatedct(InstrumentGeneralView* self)
{	
	psy_ui_button_disable_highlight(&self->dctnone);
	psy_ui_button_disable_highlight(&self->dctnote);
	psy_ui_button_disable_highlight(&self->dctsample);
	psy_ui_button_disable_highlight(&self->dctinstrument);
	if (self->instrument) {
		switch (self->instrument->dct) {
			case psy_audio_DUPECHECK_NONE:
				psy_ui_button_highlight(&self->dctnone);
				break;
			case psy_audio_DUPECHECK_NOTE:
				psy_ui_button_highlight(&self->dctnote);
				break;
			case psy_audio_DUPECHECK_SAMPLE:
				psy_ui_button_highlight(&self->dctsample);
				break;
			case psy_audio_DUPECHECK_INSTRUMENT:
				psy_ui_button_highlight(&self->dctinstrument);
				break;
			default:
				break;
			}
	}
	if (self->instrument) {
		if (self->instrument->dct != psy_audio_DUPECHECK_NONE) {
			psy_ui_component_enableinput(&self->dcacut.component, psy_ui_NONE_RECURSIVE);
			psy_ui_component_enableinput(&self->dcarelease.component, psy_ui_NONE_RECURSIVE);
			psy_ui_component_enableinput(&self->dcafadeout.component, psy_ui_NONE_RECURSIVE);
			psy_ui_component_enableinput(&self->dcanone.component, psy_ui_NONE_RECURSIVE);
		} else {
			psy_ui_component_preventinput(&self->dcacut.component, psy_ui_NONE_RECURSIVE);
			psy_ui_component_preventinput(&self->dcarelease.component, psy_ui_NONE_RECURSIVE);
			psy_ui_component_preventinput(&self->dcafadeout.component, psy_ui_NONE_RECURSIVE);
			psy_ui_component_preventinput(&self->dcanone.component, psy_ui_NONE_RECURSIVE);
		}
	}
}

/* dca */
void instrumentgeneralview_ondcacut(InstrumentGeneralView* self)
{
	if (self->instrument) {
		self->instrument->dca = psy_audio_NNA_STOP;
	}
	instrumentgeneralview_updatedca(self);
}

void instrumentgeneralview_ondcarelease(InstrumentGeneralView* self)
{
	if (self->instrument) {
		self->instrument->dca = psy_audio_NNA_NOTEOFF;
	}
	instrumentgeneralview_updatedca(self);
}

void instrumentgeneralview_ondcafadeout(InstrumentGeneralView* self)
{
	if (self->instrument) {
		self->instrument->dca = psy_audio_NNA_FADEOUT;
	}
	instrumentgeneralview_updatedca(self);
}

void instrumentgeneralview_ondcanone(InstrumentGeneralView* self)
{
	if (self->instrument) {
		self->instrument->dca = psy_audio_NNA_CONTINUE;
	}
	instrumentgeneralview_updatedca(self);
}

void instrumentgeneralview_updatedca(InstrumentGeneralView* self)
{
	psy_ui_button_disable_highlight(&self->dcanone);
	psy_ui_button_disable_highlight(&self->dcacut);
	psy_ui_button_disable_highlight(&self->dcarelease);
	psy_ui_button_disable_highlight(&self->dcafadeout);
	if (self->instrument) {
		switch (self->instrument->dca) {
		case psy_audio_NNA_STOP:
			psy_ui_button_highlight(&self->dcacut);
			break;
		case psy_audio_NNA_NOTEOFF:
			psy_ui_button_highlight(&self->dcarelease);
			break;
		case psy_audio_NNA_FADEOUT:
			psy_ui_button_highlight(&self->dcafadeout);
			break;
		case psy_audio_NNA_CONTINUE:
			psy_ui_button_highlight(&self->dcanone);
			break;
		default:
			break;
		}
	}	
}

void instrumentgeneralview_ongeneralviewtweak(InstrumentGeneralView* self,
	psy_ui_Slider* slider, float value)
{
	if (self->instrument) {				
		if (slider == &self->globalvolume) {		
			self->instrument->globalvolume = (value * value);
		}
	}
}

void instrumentgeneralview_ongeneralviewvalue(InstrumentGeneralView* self,
	psy_ui_Slider* slider, float* value)
{		
	if (slider == &self->globalvolume) {
		if (self->instrument) {			
			*value = (float)sqrt(self->instrument->globalvolume);
		} else {
			*value = 1.f;
		}
	}
}

void instrumentgeneralview_ongeneralviewdescribe(InstrumentGeneralView* self,
	psy_ui_Slider* slider, char* text)
{			
	if (slider == &self->globalvolume) {		
		if (!self->instrument) {
			psy_snprintf(text, 10, "-inf. dB");
		} else if (self->instrument->globalvolume == 0) {
			psy_snprintf(text, 10, "-inf. dB");
		} else {
			float db = (float)(20 * log10(self->instrument->globalvolume));
			psy_snprintf(text, 10, "%.1f dB", db);
		}
	}
}

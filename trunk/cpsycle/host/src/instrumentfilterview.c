// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "instrumentfilterview.h"
// dsp
#include <valuemapper.h>
// std
#include <math.h>
// platform
#include "../../detail/portable.h"

// prototypes
static void instrumentfilterview_updatetext(InstrumentFilterView*);
static void instrumentfilterview_onlanguagechanged(InstrumentFilterView*,
	psy_ui_Component* sender);
static void instrumentfilterview_fillfiltercombobox(InstrumentFilterView*);
static void instrumentfilterview_ondescribe(InstrumentFilterView*,
	psy_ui_Slider*, char* txt);
static void instrumentfilterview_ontweak(InstrumentFilterView*,
	psy_ui_Slider*, float value);
static void instrumentfilterview_onvalue(InstrumentFilterView*,
	psy_ui_Slider*, float* value);
static void instrumentfilterview_onfiltercomboboxchanged(InstrumentFilterView*,
	psy_ui_ComboBox* sender, int index);
// implementation
void instrumentfilterview_init(InstrumentFilterView* self,
	psy_ui_Component* parent, psy_audio_Instruments* instruments,
	Workspace* workspace)
{
	psy_ui_Margin margin;
	int i;
	psy_ui_Slider* sliders[] = {
		&self->attack,
		&self->decay,
		&self->sustain,
		&self->release,
		&self->cutoff,
		&self->res,
		&self->modamount,
		&self->randomcutoff,
		& self->randomresonance
	};

	self->instruments = instruments;
	self->instrument = 0;
	psy_ui_component_init(&self->component, parent);
	psy_ui_component_setdefaultalign(&self->component, psy_ui_ALIGN_TOP,
		psy_ui_defaults_vmargin(psy_ui_defaults()));
	psy_ui_component_init(&self->top, &self->component);	
	psy_ui_component_setdefaultalign(&self->top, psy_ui_ALIGN_TOP,
		psy_ui_defaults_vmargin(psy_ui_defaults()));
	psy_ui_component_init(&self->filter, &self->top);
	margin = psy_ui_defaults_vmargin(psy_ui_defaults());
	margin.top = psy_ui_value_makeeh(1.0);
	psy_ui_component_setmargin(&self->filter, &margin);
	psy_ui_label_init_text(&self->filtertypeheader, &self->filter,
		"Filter type");
	psy_ui_component_setalign(&self->filtertypeheader.component,
		psy_ui_ALIGN_LEFT);
	psy_ui_margin_init_all(&margin,
		psy_ui_value_makepx(0), psy_ui_value_makeew(2),
		psy_ui_value_makepx(0), psy_ui_value_makepx(0));
	psy_ui_component_setmargin(&self->filtertypeheader.component, &margin);
	psy_ui_combobox_init(&self->filtertype, &self->filter);
	psy_ui_combobox_setcharnumber(&self->filtertype, 20);
	psy_ui_component_setalign(&self->filtertype.component, psy_ui_ALIGN_LEFT);
	instrumentfilterview_fillfiltercombobox(self);
	psy_ui_combobox_setcursel(&self->filtertype, (int)F_NONE);
	psy_signal_connect(&self->filtertype.signal_selchanged, self,
		instrumentfilterview_onfiltercomboboxchanged);
	psy_ui_slider_init(&self->randomcutoff, &self->top);
	psy_ui_slider_settext(&self->randomcutoff, "Random Cutoff");
	psy_ui_slider_init(&self->randomresonance, &self->top);
	psy_ui_slider_settext(&self->randomresonance, "Random Res");
	envelopeview_init(&self->envelopeview, &self->component, workspace);
	psy_ui_component_setalign(&self->envelopeview.component,
		psy_ui_ALIGN_CLIENT);
	margin = psy_ui_defaults_vmargin(psy_ui_defaults());
	margin.top = psy_ui_value_makeeh(1.0);
	psy_ui_component_init(&self->bottom, &self->component);
	psy_ui_component_setalign(&self->bottom, psy_ui_ALIGN_BOTTOM);
	margin = psy_ui_defaults_vmargin(psy_ui_defaults());
	margin.top = psy_ui_value_makeeh(1.0);
	psy_ui_component_setmargin(&self->bottom, &margin);
	psy_ui_component_setdefaultalign(&self->bottom, psy_ui_ALIGN_TOP,
		psy_ui_defaults_vmargin(psy_ui_defaults()));	
	envelopeview_settext(&self->envelopeview, "Filter envelope");
	psy_ui_slider_init(&self->attack, &self->bottom);
	psy_ui_slider_settext(&self->attack, "Attack");
	psy_ui_slider_init(&self->decay, &self->bottom);
	psy_ui_slider_settext(&self->decay, "Decay");
	psy_ui_slider_init(&self->sustain, &self->bottom);
	psy_ui_slider_settext(&self->sustain, "Sustain Level");
	psy_ui_slider_init(&self->release, &self->bottom);
	psy_ui_slider_settext(&self->release, "Release");
	psy_ui_slider_init(&self->cutoff, &self->bottom);
	psy_ui_slider_settext(&self->cutoff, "Cut-off");
	psy_ui_slider_init(&self->res, &self->bottom);
	psy_ui_slider_settext(&self->res, "Res/bandw.");
	psy_ui_slider_init(&self->modamount, &self->bottom);
	psy_ui_slider_settext(&self->modamount, "Mod. Amount");
	for (i = 0; i < 9; ++i) {				
		psy_ui_slider_setcharnumber(sliders[i], 18);
		psy_ui_slider_setvaluecharnumber(sliders[i], 15);
		psy_ui_slider_connect(sliders[i], self,
			(ui_slider_fpdescribe)instrumentfilterview_ondescribe,
			(ui_slider_fptweak)instrumentfilterview_ontweak,
			(ui_slider_fpvalue)instrumentfilterview_onvalue);
	}
	instrumentfilterview_updatetext(self);
	psy_signal_connect(&self->component.signal_languagechanged, self,
		instrumentfilterview_onlanguagechanged);
	psy_ui_label_settext(&self->filtertypeheader, "instrumentview.filter-type");
	psy_ui_slider_settext(&self->attack, "instrumentview.attack");
	psy_ui_slider_settext(&self->decay, "instrumentview.decay");
	psy_ui_slider_settext(&self->sustain, "instrumentview.sustain-level");
	psy_ui_slider_settext(&self->release, "instrumentview.release");
	psy_ui_slider_settext(&self->cutoff, "instrumentview.cut-off");
	psy_ui_slider_settext(&self->res, "instrumentview.res");
	psy_ui_slider_settext(&self->modamount, "instrumentview.mod");
}

void instrumentfilterview_updatetext(InstrumentFilterView* self)
{	
	envelopeview_settext(&self->envelopeview,
		psy_ui_translate("instrumentview.filter-envelope"));
}

void instrumentfilterview_onlanguagechanged(InstrumentFilterView* self,
	psy_ui_Component* sender)
{
	instrumentfilterview_updatetext(self);
}

void instrumentfilterview_fillfiltercombobox(InstrumentFilterView* self)
{
	uintptr_t ft;
	for (ft = 0; ft < filter_numfilters(); ++ft) {
		psy_ui_combobox_addtext(&self->filtertype,
			filter_name((psy_dsp_FilterType)ft));
	}
}

void instrumentfilterview_onfiltercomboboxchanged(InstrumentFilterView* self,
	psy_ui_ComboBox* sender, int index)
{
	self->instrument->filtertype = (psy_dsp_FilterType)index;
}

void instrumentfilterview_setinstrument(InstrumentFilterView* self,
	psy_audio_Instrument* instrument)
{	
	self->instrument = instrument;
	if (self->instrument) {
		envelopeview_setenvelope(&self->envelopeview,
			&instrument->filterenvelope);
		psy_ui_combobox_setcursel(&self->filtertype,
			(intptr_t)instrument->filtertype);
	} else {
		envelopeview_setenvelope(&self->envelopeview, NULL);
		psy_ui_combobox_setcursel(&self->filtertype, (intptr_t)F_NONE);
	}	
}

void instrumentfilterview_ondescribe(InstrumentFilterView* self,
	psy_ui_Slider* slider, char* txt)
{
	if (!self->instrument) {
		if (slider == &self->attack) {
			psy_snprintf(txt, 10, "0ms");
		} else if (slider == &self->decay) {
			psy_snprintf(txt, 10, "0ms");
		} else if (slider == &self->sustain) {
			psy_snprintf(txt, 10, "0%%");
		} else if (slider == &self->release) {
			psy_snprintf(txt, 10, "0ms");
		} else if (slider == &self->cutoff) {
			psy_snprintf(txt, 10, "0");		
		} else if (slider == &self->res) {
			psy_snprintf(txt, 10, "50%%");
		} else if (slider == &self->modamount) {
			psy_snprintf(txt, 10, "0%");
		} else if (slider == &self->randomcutoff) {
			psy_snprintf(txt, 10, "0%");
		} else if (slider == &self->randomresonance) {
			psy_snprintf(txt, 10, "off");
		}
		return;
	}
	if (slider == &self->attack) {		
		psy_dsp_EnvelopePoint pt_start;			

		pt_start = psy_dsp_envelopesettings_at(
			&self->instrument->filterenvelope, 1);
		psy_snprintf(txt, 20, "%.4fms",
			pt_start.time * 1000);		
	} else if (slider == &self->decay) {		
		psy_dsp_EnvelopePoint pt_start;
		psy_dsp_EnvelopePoint pt_end;

		pt_start = psy_dsp_envelopesettings_at(
			&self->instrument->filterenvelope, 1);
		pt_end = psy_dsp_envelopesettings_at(
			&self->instrument->filterenvelope, 2);
		psy_snprintf(txt, 20, "%.4fms",
			(pt_end.time - pt_start.time) * 1000);		
	} else if (slider == &self->sustain) {		
		psy_dsp_EnvelopePoint pt_start;			

		pt_start = psy_dsp_envelopesettings_at(
			&self->instrument->filterenvelope, 1);			
		psy_snprintf(txt, 20, "%d%%", (int)(pt_start.value * 100));		
	} else if (slider == &self->release) {		
		psy_dsp_EnvelopePoint pt_start;
		psy_dsp_EnvelopePoint pt_end;

		pt_start = psy_dsp_envelopesettings_at(&self->instrument->filterenvelope, 2);
		pt_end = psy_dsp_envelopesettings_at(&self->instrument->filterenvelope, 3);
		psy_snprintf(txt, 20, "%.4fms", (pt_end.time - pt_start.time) * 1000);		
	} else if (slider == &self->cutoff) {		
		psy_snprintf(txt, 20, "%d", (int)(self->instrument->filtercutoff * 11665 + 2333));				
	} else if (slider == &self->res) {		
		psy_snprintf(txt, 20, "%d%%", (int)(self->instrument->filterres * 100));				
	} else if (slider == &self->modamount) {		
		psy_snprintf(txt, 20, "%d%%", (int)(self->instrument->filtermodamount) * 100);				
	} else if (slider == &self->randomcutoff) {
		if (!self->instrument->randomcutoff == 0.f) {
			psy_snprintf(txt, 10, "off");
		} else {
			psy_snprintf(txt, 20, "%d%%", (int)(self->instrument->randomcutoff * 100));
		}
	} else if (slider == &self->randomresonance) {
		if (!self->instrument->randompanning == 0.f) {
			psy_snprintf(txt, 10, "off");
		} else {
			psy_snprintf(txt, 20, "%d%%", (int)(self->instrument->randompanning * 100));			
		}
	}
}

void instrumentfilterview_ontweak(InstrumentFilterView* self,
	psy_ui_Slider* slidergroup, float value)
{
	if (!self->instrument) {
		return;
	}	

	if (slidergroup == &self->attack) {
		psy_dsp_envelopesettings_settime(&self->instrument->filterenvelope,
			1, value * 5.f);		
	} else if (slidergroup == &self->decay) {
		psy_dsp_EnvelopePoint pt_start;		

		pt_start = psy_dsp_envelopesettings_at(&self->instrument->filterenvelope, 1);		
		psy_dsp_envelopesettings_settime(&self->instrument->filterenvelope,
			2, pt_start.time + value * 5.f);		
	} else if (slidergroup == &self->sustain) {
		psy_dsp_EnvelopePoint pt_start;

		pt_start = psy_dsp_envelopesettings_at(&self->instrument->filterenvelope, 2);
		psy_dsp_envelopesettings_setvalue(&self->instrument->filterenvelope,
			2, value);		
	} else if (slidergroup == &self->release) {
		psy_dsp_EnvelopePoint pt_start;

		pt_start = psy_dsp_envelopesettings_at(&self->instrument->filterenvelope, 2);
		psy_dsp_envelopesettings_settime(&self->instrument->filterenvelope,
			3, pt_start.time + value * 5.f);		
	} else if (slidergroup == &self->cutoff) {
		self->instrument->filtercutoff = value;
	} else if (slidergroup == &self->res) {
		self->instrument->filterres = value;
	} else if (slidergroup == &self->modamount) {
		self->instrument->filtermodamount = value - 0.5f;
	} else if (slidergroup == &self->randomcutoff) {
		self->instrument->randomcutoff = value;
	} else if (slidergroup == &self->randomresonance) {
		self->instrument->randomresonance = value;
	}
	envelopeview_update(&self->envelopeview);
}

void instrumentfilterview_onvalue(InstrumentFilterView* self,
	psy_ui_Slider* slider, float* value)
{
	if (slider == &self->attack) {
		if (self->instrument) {
			psy_dsp_EnvelopePoint pt_start;

			pt_start = psy_dsp_envelopesettings_at(
				&self->instrument->filterenvelope, 1);
			*value = pt_start.time / 5.f;
		} else {
			*value = 0.f;
		}			
	} else if (slider == &self->decay) {
		if (self->instrument) {
			psy_dsp_EnvelopePoint pt_start;
			psy_dsp_EnvelopePoint pt_end;

			pt_start = psy_dsp_envelopesettings_at(
				&self->instrument->filterenvelope, 1);
			pt_end = psy_dsp_envelopesettings_at(
				&self->instrument->filterenvelope, 2);
			*value = (pt_end.time - pt_start.time) / 5.f;
		} else {
			*value = 0.f;
		}
	} else if (slider == &self->sustain) {
		if (self->instrument) {
			psy_dsp_EnvelopePoint pt_start;			

			pt_start = psy_dsp_envelopesettings_at(
				&self->instrument->filterenvelope, 2);			
			*value = pt_start.value;
		} else {
			*value = 0.5f;
		}
	} else if (slider == &self->release) {
		if (self->instrument) {
			psy_dsp_EnvelopePoint pt_start;
			psy_dsp_EnvelopePoint pt_end;

			pt_start = psy_dsp_envelopesettings_at(
				&self->instrument->filterenvelope, 2);
			pt_end = psy_dsp_envelopesettings_at(
				&self->instrument->filterenvelope, 3);
			*value = (pt_end.time - pt_start.time) / 5.f;
		} else {
			*value = 0.5f;
		}
	} else if (slider == &self->cutoff) {
		*value = self->instrument
			? self->instrument->filtercutoff
			: 0.5f;
	} else if (slider == &self->res) {
		*value = self->instrument
			? self->instrument->filterres
			: 0.5f;
	} else if (slider == &self->modamount) {
		*value = self->instrument
			? self->instrument->filtermodamount + 0.5f
			: 0.5f;
	} else if (slider == &self->randomcutoff) {
		*value = self->instrument
			? self->instrument->randomcutoff
			: 0.0f;
	} else if (slider == &self->randomresonance) {
		*value = self->instrument
			? self->instrument->randomresonance
			: 0.0f;
	}
}

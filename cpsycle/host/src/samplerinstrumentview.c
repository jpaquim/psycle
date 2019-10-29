// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "samplerinstrumentview.h"

static void OnSize(SamplerInstrumentView*, ui_component* sender, ui_size* size);
static void AlignInstrumentView(SamplerInstrumentView* self);
static void OnInstrumentInsert(SamplerInstrumentView* self, ui_component* sender, int slot);
static void OnInstrumentSlotChanged(SamplerInstrumentView* self, Instrument* sender, int slot);
static void OnInstrumentListChanged(SamplerInstrumentView* self, ui_component* sender, int slot);
static void SetInstrument(SamplerInstrumentView* self, int slot);
static void OnSongChanged(SamplerInstrumentView* self, Workspace* workspace);
static void InitSamplerInstrumentHeaderView(SamplerInstrumentHeaderView*, ui_component* parent, Instruments*);
static void SetInstrumentInstrumentHeaderView(SamplerInstrumentHeaderView*, Instrument*);
static void OnPrevInstrument(SamplerInstrumentHeaderView*, ui_component* sender);
static void OnNextInstrument(SamplerInstrumentHeaderView*, ui_component* sender);
static void OnDeleteInstrument(SamplerInstrumentHeaderView*, ui_component* sender);
static void InitSamplerInstrumentGeneralView(SamplerInstrumentGeneralView*, ui_component* parent, Instruments*);
static void SetInstrumentInstrumentGeneralView(SamplerInstrumentGeneralView* self, Instrument* instrument);
static void OnNNACut(SamplerInstrumentGeneralView* self);
static void OnNNARelease(SamplerInstrumentGeneralView* self);
static void OnNNANone(SamplerInstrumentGeneralView* self);
static void InitSamplerInstrumentVolumeView(SamplerInstrumentVolumeView*, ui_component* parent, Instruments*);
static void OnVolumeViewDescribe(SamplerInstrumentVolumeView*, SliderGroup*, char* txt);
static void OnVolumeViewTweak(SamplerInstrumentVolumeView*, SliderGroup*, float value);
static void OnVolumeViewValue(SamplerInstrumentVolumeView*, SliderGroup*, float* value);
static void SetInstrumentInstrumentVolumeView(SamplerInstrumentVolumeView* self, Instrument* instrument);
static void InitSamplerInstrumentPanView(SamplerInstrumentPanView*, ui_component* parent, Instruments*);
static void SetInstrumentInstrumentPanView(SamplerInstrumentPanView* self, Instrument* instrument);
static void InitSamplerInstrumentFilterView(SamplerInstrumentFilterView*, ui_component* parent, Instruments*);
static void SetInstrumentInstrumentFilterView(SamplerInstrumentFilterView* self, Instrument* instrument);
static void OnFilterViewDescribe(SamplerInstrumentFilterView*, SliderGroup*, char* txt);
static void OnFilterViewTweak(SamplerInstrumentFilterView*, SliderGroup*, float value);
static void OnFilterViewValue(SamplerInstrumentFilterView*, SliderGroup*, float* value);
static void InitSamplerInstrumentPitchView(SamplerInstrumentPitchView*, ui_component* parent, Instruments*);
static void SetInstrumentInstrumentPitchView(SamplerInstrumentPitchView* self, Instrument* instrument);


void InitSamplerInstrumentView(SamplerInstrumentView* self, ui_component* parent,
							   Workspace* workspace)
{
	self->player = &workspace->player;
	ui_component_init(&self->component, parent);
	ui_component_setbackgroundmode(&self->component, BACKGROUND_NONE);
	ui_notebook_init(&self->notebook, &self->component);
	ui_component_setbackgroundmode(&self->notebook.component, BACKGROUND_SET);
	signal_connect(&self->component.signal_size, self, OnSize);
	InitSamplerInstrumentHeaderView(&self->header, &self->component, &workspace->song->instruments);
	InitSamplerInstrumentGeneralView(&self->general, &self->notebook.component, &workspace->song->instruments);
	InitSamplerInstrumentVolumeView(&self->volume, &self->notebook.component, &workspace->song->instruments);
	InitSamplerInstrumentPanView(&self->pan, &self->notebook.component, &workspace->song->instruments);
	InitSamplerInstrumentFilterView(&self->filter, &self->notebook.component, &workspace->song->instruments);
	InitSamplerInstrumentPitchView(&self->pitch, &self->notebook.component, &workspace->song->instruments);
	InitInstrumentsBox(&self->instrumentsbox, &self->component, &workspace->song->instruments);
	tabbar_init(&self->tabbar, &self->component);	
	tabbar_append(&self->tabbar, "General");
	tabbar_append(&self->tabbar, "Volume");
	tabbar_append(&self->tabbar, "Pan");
	tabbar_append(&self->tabbar, "Filter");
	tabbar_append(&self->tabbar, "Pitch");
	ui_notebook_connectcontroller(&self->notebook, &self->tabbar.signal_change);

	AlignInstrumentView(self);
	signal_connect(&self->player->song->instruments.signal_insert, self, OnInstrumentInsert);
	signal_connect(&self->player->song->instruments.signal_slotchange, self, OnInstrumentSlotChanged);
	signal_connect(&self->instrumentsbox.instrumentlist.signal_selchanged, self, OnInstrumentListChanged);

	ui_notebook_setpage(&self->notebook, 0);
	signal_connect(&workspace->signal_songchanged, self, OnSongChanged);
}

void AlignInstrumentView(SamplerInstrumentView* self)
{
	ui_size size = ui_component_size(&self->component);

	ui_component_setposition(&self->instrumentsbox.instrumentlist.component,
																	5,   5, 210, 400);
	ui_component_setposition(&self->header.component,			   220,   5, 500,  20);
	ui_component_setposition(&self->tabbar.component,			   220,  30, 500,  20);
	ui_component_setposition(&self->notebook.component,			   220,  60, size.width - 220, size.height - 60);	
}

void OnSize(SamplerInstrumentView* self, ui_component* sender, ui_size* size)
{
	AlignInstrumentView(self);
}

void OnInstrumentInsert(SamplerInstrumentView* self, ui_component* sender, int slot)
{
	SetInstrument(self, slot);
}

void OnInstrumentSlotChanged(SamplerInstrumentView* self, Instrument* sender, int slot)
{	
	SetInstrument(self, slot);
}

void OnInstrumentListChanged(SamplerInstrumentView* self, ui_component* sender, int slot)
{
	instruments_changeslot(&self->player->song->instruments, slot);
}

void SetInstrument(SamplerInstrumentView* self, int slot)
{
	Instrument* instrument = instruments_at(&self->player->song->instruments, slot);	
	SetInstrumentInstrumentHeaderView(&self->header, instrument);
	SetInstrumentInstrumentGeneralView(&self->general, instrument);
	SetInstrumentInstrumentVolumeView(&self->volume, instrument);
	SetInstrumentInstrumentPanView(&self->pan, instrument);
	SetInstrumentInstrumentFilterView(&self->filter, instrument);
	SetInstrumentInstrumentPitchView(&self->pitch, instrument);
}

void OnSongChanged(SamplerInstrumentView* self, Workspace* workspace)
{	
	self->header.instruments = &workspace->song->instruments;
	self->general.instruments = &workspace->song->instruments;
	self->volume.instruments = &workspace->song->instruments;
	self->pan.instruments = &workspace->song->instruments;
	self->filter.instruments = &workspace->song->instruments;
	self->pitch.instruments = &workspace->song->instruments;
	signal_connect(&workspace->song->instruments.signal_slotchange, self, OnInstrumentSlotChanged);
	SetInstruments(&self->instrumentsbox, &workspace->song->instruments);
	SetInstrument(self, 0);
}

void InitSamplerInstrumentHeaderView(SamplerInstrumentHeaderView* self, ui_component* parent, Instruments* instruments)
{
	self->instrument = 0;
	self->instruments = instruments;
	ui_component_init(&self->component, parent);	

	ui_label_init(&self->namelabel, &self->component);
	ui_label_settext(&self->namelabel, "Instrument name");
	ui_component_setposition(&self->namelabel.component, 0, 0, 120, 20);

	ui_edit_init(&self->nameedit, &self->component, 0);	
	ui_component_setposition(&self->nameedit.component, 125, 0, 85, 20);

	ui_button_init(&self->previnstrumentbutton, &self->component);
	ui_button_settext(&self->previnstrumentbutton, "<");
	ui_component_setposition(&self->previnstrumentbutton.component, 220, 0, 20, 20);
	signal_connect(&self->previnstrumentbutton.signal_clicked, self, OnPrevInstrument);

	ui_button_init(&self->nextinstrumentbutton, &self->component);
	ui_button_settext(&self->nextinstrumentbutton, ">");
	ui_component_setposition(&self->nextinstrumentbutton.component, 245, 0, 20, 20);
	signal_connect(&self->nextinstrumentbutton.signal_clicked, self, OnNextInstrument);

	ui_button_init(&self->deleteinstrumentbutton, &self->component);
	ui_button_settext(&self->deleteinstrumentbutton, "Delete");
	ui_component_setposition(&self->deleteinstrumentbutton.component, 270, 0, 70, 20);
	signal_connect(&self->deleteinstrumentbutton.signal_clicked, self, OnDeleteInstrument);	
}

void SetInstrumentInstrumentHeaderView(SamplerInstrumentHeaderView* self, Instrument* instrument)
{
//	char buffer[20];
	self->instrument = instrument;
	ui_edit_settext(&self->nameedit, instrument ? instrument->name : "");
}

void OnPrevInstrument(SamplerInstrumentHeaderView* self, ui_component* sender)
{
	instruments_changeslot(self->instruments,
		instruments_slot(self->instruments) > 0 ?
		instruments_slot(self->instruments) - 1 : 0);
}

void OnNextInstrument(SamplerInstrumentHeaderView* self, ui_component* sender)
{
	instruments_changeslot(self->instruments,
		instruments_slot(self->instruments) < 255 ?
		instruments_slot(self->instruments) + 1 : 255);
}

void OnDeleteInstrument(SamplerInstrumentHeaderView* self, ui_component* sender)
{
	
}

void InitSamplerInstrumentGeneralView(SamplerInstrumentGeneralView* self, ui_component* parent, Instruments* instruments)
{
	self->instruments = instruments;
	self->instrument = 0;
	ui_component_init(&self->component, parent);	
	
	ui_label_init(&self->nnaheaderlabel, &self->component);
	ui_label_settext(&self->nnaheaderlabel, "New Note Action");
	ui_component_setposition(&self->nnaheaderlabel.component, 5, 5, 150, 20);

	ui_button_init(&self->nnacutbutton, &self->component);
	ui_button_settext(&self->nnacutbutton, "Note Cut");
	ui_component_setposition(&self->nnacutbutton.component, 5, 30, 70, 20);
	signal_connect(&self->nnacutbutton.signal_clicked, self, OnNNACut);

	ui_button_init(&self->nnareleasebutton, &self->component);
	ui_button_settext(&self->nnareleasebutton, "Note Release");
	ui_component_setposition(&self->nnareleasebutton.component, 80, 30, 100, 20);
	signal_connect(&self->nnareleasebutton.signal_clicked, self, OnNNARelease);

	ui_button_init(&self->nnanonebutton, &self->component);
	ui_button_settext(&self->nnanonebutton, "None");
	ui_component_setposition(&self->nnanonebutton.component, 185, 30, 70, 20);
	signal_connect(&self->nnanonebutton.signal_clicked, self, OnNNANone);

	SendMessage(self->nnacutbutton.component.hwnd, BM_SETSTATE, (WPARAM)1, (LPARAM)0);

}

void SetInstrumentInstrumentGeneralView(SamplerInstrumentGeneralView* self, Instrument* instrument)
{
	// char buffer[20];
	self->instrument = instrument;	
	ui_button_disablehighlight(&self->nnacutbutton);
	ui_button_disablehighlight(&self->nnareleasebutton);
	ui_button_disablehighlight(&self->nnanonebutton);
	if (instrument) {
		switch (self->instrument->nna) {
			case NNA_STOP:
				ui_button_highlight(&self->nnacutbutton);
			break;
			case NNA_NOTEOFF:
				ui_button_highlight(&self->nnareleasebutton);
			break;
			case NNA_CONTINUE:
				ui_button_highlight(&self->nnanonebutton);
			break;
			default:				
			break;
		}
	}
}

void OnNNACut(SamplerInstrumentGeneralView* self)
{
	if (self->instrument) {
		instrument_setnna(self->instrument, NNA_STOP);
		ui_button_highlight(&self->nnacutbutton);
		ui_button_disablehighlight(&self->nnareleasebutton);
		ui_button_disablehighlight(&self->nnanonebutton);
	}
}

void OnNNARelease(SamplerInstrumentGeneralView* self)
{
	if (self->instrument) {
		instrument_setnna(self->instrument, NNA_NOTEOFF);
		ui_button_disablehighlight(&self->nnacutbutton);
		ui_button_highlight(&self->nnareleasebutton);
		ui_button_disablehighlight(&self->nnanonebutton);
	}
}

void OnNNANone(SamplerInstrumentGeneralView* self)
{
	if (self->instrument) {
		instrument_setnna(self->instrument, NNA_CONTINUE);
		ui_button_disablehighlight(&self->nnacutbutton);
		ui_button_disablehighlight(&self->nnareleasebutton);
		ui_button_highlight(&self->nnanonebutton);
	}
}

void InitSamplerInstrumentVolumeView(SamplerInstrumentVolumeView* self, ui_component* parent, Instruments* instruments)
{
	ui_margin margin;
	int i;
	SliderGroup* sliders[] = {
		&self->attack,
		&self->decay,
		&self->sustain,
		&self->release
	};

	self->instruments = instruments;	
	self->instrument = 0;
	ui_component_init(&self->component, parent);	
	ui_component_enablealign(&self->component);
	ui_groupbox_init(&self->groupbox, &self->component);	
	ui_groupbox_settext(&self->groupbox, "Amplitude envelope");	
	ui_component_setalign(&self->groupbox.component, UI_ALIGN_FILL);

	InitEnvelopeView(&self->envelopeview, &self->component);
	ui_component_setalign(&self->envelopeview.component, UI_ALIGN_TOP);
	ui_margin_init(&margin, 20, 5, 5, 5);
	ui_component_setmargin(&self->envelopeview.component, &margin);	
	ui_component_resize(&self->envelopeview.component, 0, 200);	

	InitSliderGroup(&self->attack, &self->component, "Attack");
	InitSliderGroup(&self->decay, &self->component, "Decay");
	InitSliderGroup(&self->sustain, &self->component, "Sustain Level");
	InitSliderGroup(&self->release, &self->component, "Release");
	
	ui_margin_init(&margin, 0, 5, 5, 5);
	for (i = 0; i < 4; ++i) {		
		ui_component_resize(&sliders[i]->component, 100, 20);		
		ui_component_setalign(&sliders[i]->component, UI_ALIGN_TOP);
		ui_component_setmargin(&sliders[i]->component, &margin);
		SliderGroupConnect(sliders[i], self, OnVolumeViewDescribe,
			OnVolumeViewTweak, OnVolumeViewValue);		
	}	
}

void SetInstrumentInstrumentVolumeView(SamplerInstrumentVolumeView* self, Instrument* instrument)
{	
	self->instrument = instrument;
	EnvelopeViewSetAdsrEnvelope(&self->envelopeview, instrument ? &instrument->volumeenvelope : 0);
}

void OnVolumeViewDescribe(SamplerInstrumentVolumeView* self, SliderGroup* slidergroup, char* txt)
{
	if (slidergroup == &self->attack) {		
		if (!self->instrument) {
			_snprintf(txt, 10, "0ms");
		} else {
			_snprintf(txt, 20, "%.4fms", 
				adsr_settings_attack(&self->instrument->volumeenvelope) * 1000);				
		}		
	} else
	if (slidergroup == &self->decay) {		
		if (!self->instrument) {
			_snprintf(txt, 10, "0ms");
		} else {
			_snprintf(txt, 20, "%.4fms",
				adsr_settings_decay(&self->instrument->volumeenvelope) * 1000);
		}		
	} else
	if (slidergroup == &self->sustain) {
		if (!self->instrument) {
			_snprintf(txt, 10, "0%%");
		} else {
			_snprintf(txt, 20, "%d%%", (int)
				(adsr_settings_sustain(&self->instrument->volumeenvelope) * 100));
		}		
	} else
	if (slidergroup == &self->release) {
		if (!self->instrument) {
			_snprintf(txt, 10, "0ms");
		} else {
			_snprintf(txt, 20, "%.4fms",
				adsr_settings_release(&self->instrument->volumeenvelope) * 1000);
		}		
	}
}

void OnVolumeViewTweak(SamplerInstrumentVolumeView* self, SliderGroup* slidergroup, float value)
{
	if (!self->instrument) {
		return;
	}
	if (slidergroup == &self->attack) {
		adsr_settings_setattack(
			&self->instrument->volumeenvelope, value * 1.4f);
	} else
	if (slidergroup == &self->decay) {
		adsr_settings_setdecay(
			&self->instrument->volumeenvelope, value * 1.4f);
	} else
	if (slidergroup == &self->sustain) {
		adsr_settings_setsustain(
			&self->instrument->volumeenvelope, value);
	} else
	if (slidergroup == &self->release) {
		adsr_settings_setrelease(
			&self->instrument->volumeenvelope, value * 1.4f);
	}
	EnvelopeViewUpdate(&self->envelopeview);
}

void OnVolumeViewValue(SamplerInstrumentVolumeView* self, SliderGroup* slidergroup, float* value)
{
	if (slidergroup == &self->attack) {
		*value = self->instrument
			? adsr_settings_attack(&self->instrument->volumeenvelope) / 1.4f
			: 0.f;
	} else 
	if (slidergroup == &self->decay) {
		*value = self->instrument
			? adsr_settings_decay(&self->instrument->volumeenvelope) / 1.4f
			: 0.f;
	} else 	
	if (slidergroup == &self->sustain) {
		*value = self->instrument
			? adsr_settings_sustain(&self->instrument->volumeenvelope)
			: 0.5f;
	} else
	if (slidergroup == &self->release) {
		*value = self->instrument
			? adsr_settings_release(&self->instrument->volumeenvelope) / 1.4f
			: 0.5f;
	}
}

void InitSamplerInstrumentPanView(SamplerInstrumentPanView* self, ui_component* parent, Instruments* instruments)
{
	self->instruments = instruments;	
	ui_component_init(&self->component, parent);	
}

void SetInstrumentInstrumentPanView(SamplerInstrumentPanView* self, Instrument* instrument)
{
	// char buffer[20];
	self->instrument = instrument;	
}

void InitSamplerInstrumentFilterView(SamplerInstrumentFilterView* self, ui_component* parent, Instruments* instruments)
{
	ui_margin margin;
	int i;
	SliderGroup* sliders[] = {
		&self->attack,
		&self->decay,
		&self->sustain,
		&self->release,
		&self->cutoff,
		&self->res,
		&self->modamount
	};

	self->instruments = instruments;
	self->instrument = 0;
	ui_component_init(&self->component, parent);	
	ui_component_enablealign(&self->component);
	ui_groupbox_init(&self->groupbox, &self->component);	
	ui_groupbox_settext(&self->groupbox, "Filter envelope");	
	ui_component_setalign(&self->groupbox.component, UI_ALIGN_FILL);

	InitEnvelopeView(&self->envelopeview, &self->component);
	ui_component_setalign(&self->envelopeview.component, UI_ALIGN_TOP);
	ui_margin_init(&margin, 20, 5, 5, 5);
	ui_component_setmargin(&self->envelopeview.component, &margin);	
	ui_component_resize(&self->envelopeview.component, 0, 100);	

	InitSliderGroup(&self->attack, &self->component, "Attack");
	InitSliderGroup(&self->decay, &self->component, "Decay");
	InitSliderGroup(&self->sustain, &self->component, "Sustain Level");
	InitSliderGroup(&self->release, &self->component, "Release");
	InitSliderGroup(&self->cutoff, &self->component, "Cut-off");
	InitSliderGroup(&self->res, &self->component, "Res/bandw.");
	InitSliderGroup(&self->modamount, &self->component, "Mod. Amount");
	
	ui_margin_init(&margin, 0, 5, 5, 5);
	for (i = 0; i < 7; ++i) {		
		ui_component_resize(&sliders[i]->component, 100, 20);		
		ui_component_setalign(&sliders[i]->component, UI_ALIGN_TOP);
		ui_component_setmargin(&sliders[i]->component, &margin);
		SliderGroupConnect(sliders[i], self, OnFilterViewDescribe,
			OnFilterViewTweak, OnFilterViewValue);		
	}	
}

void SetInstrumentInstrumentFilterView(SamplerInstrumentFilterView* self, Instrument* instrument)
{	
	self->instrument = instrument;
	EnvelopeViewSetAdsrEnvelope(&self->envelopeview,
		instrument ? &instrument->filterenvelope : 0);
}

void OnFilterViewDescribe(SamplerInstrumentFilterView* self, SliderGroup* slidergroup, char* txt)
{
	if (slidergroup == &self->attack) {		
		if (!self->instrument) {
			_snprintf(txt, 10, "0ms");
		} else {
			_snprintf(txt, 20, "%.4fms", 
				adsr_settings_attack(
					&self->instrument->filterenvelope) * 1000);				
		}		
	} else
	if (slidergroup == &self->decay) {		
		if (!self->instrument) {
			_snprintf(txt, 10, "0ms");
		} else {
			_snprintf(txt, 20, "%.4fms",
				adsr_settings_decay(
					&self->instrument->filterenvelope) * 1000);
		}		
	} else
	if (slidergroup == &self->sustain) {
		if (!self->instrument) {
			_snprintf(txt, 10, "0%%");
		} else {
			_snprintf(txt, 20, "%d%%", (int)(
				adsr_settings_sustain(
					&self->instrument->filterenvelope) * 100));
		}		
	} else
	if (slidergroup == &self->release) {
		if (!self->instrument) {
			_snprintf(txt, 10, "0ms");
		} else {
			_snprintf(txt, 20, "%.4fms",
				adsr_settings_release(
					&self->instrument->filterenvelope) * 1000);
		}		
	} else
	if (slidergroup == &self->cutoff) {
		if (!self->instrument) {
			_snprintf(txt, 10, "0");
		} else {
			_snprintf(txt, 20, "%d", (int)(self->instrument->filtercutoff * 11665 + 2333));
		}		
	} else
	if (slidergroup == &self->res) {
		if (!self->instrument) {
			_snprintf(txt, 10, "50%%");
		} else {
			_snprintf(txt, 20, "%d%%", (int)(self->instrument->filterres * 100));
		}		
	} else
	if (slidergroup == &self->modamount) {
		if (!self->instrument) {
			_snprintf(txt, 10, "0ms");
		} else {
			_snprintf(txt, 20, "%d%%", (int)(self->instrument->filtermodamount) * 100);
		}		
	}	
}

void OnFilterViewTweak(SamplerInstrumentFilterView* self,
	SliderGroup* slidergroup, float value)
{
	if (!self->instrument) {
		return;
	}
	if (slidergroup == &self->attack) {
			adsr_settings_setattack(
				&self->instrument->filterenvelope, value * 1.4f);
	} else
	if (slidergroup == &self->decay) {
			adsr_settings_setdecay(
				&self->instrument->filterenvelope, value * 1.4f);
	} else
	if (slidergroup == &self->sustain) {
			adsr_settings_setsustain(
				&self->instrument->filterenvelope, value);
	} else
	if (slidergroup == &self->release) {
		adsr_settings_setrelease(
			&self->instrument->filterenvelope, value * 1.4f);
	} else
	if (slidergroup == &self->cutoff) {
		self->instrument->filtercutoff = value;
	} else
	if (slidergroup == &self->res) {
		self->instrument->filterres = value;
	} else
	if (slidergroup == &self->modamount) {
		self->instrument->filtermodamount = value - 0.5f;
	}
	EnvelopeViewUpdate(&self->envelopeview);
}

void OnFilterViewValue(SamplerInstrumentFilterView* self, SliderGroup* slidergroup, float* value)
{
	if (slidergroup == &self->attack) {
		*value = self->instrument
			? adsr_settings_attack(&self->instrument->filterenvelope) / 1.4f
			: 0.f;
	} else 
	if (slidergroup == &self->decay) {
		*value = self->instrument
			? adsr_settings_decay(&self->instrument->filterenvelope) / 1.4f
			: 0.f;
	} else 	
	if (slidergroup == &self->sustain) {
		*value = self->instrument
			? adsr_settings_sustain(&self->instrument->filterenvelope)
			: 0.5f;
	} else
	if (slidergroup == &self->release) {
		*value = self->instrument
			? adsr_settings_release(&self->instrument->filterenvelope) / 1.4f
			: 0.5f;
	} else
	if (slidergroup == &self->cutoff) {
		*value = self->instrument
			? self->instrument->filtercutoff
			: 0.5f;
	} else
	if (slidergroup == &self->res) {
		*value = self->instrument
			? self->instrument->filterres
			: 0.5f;
	} else
	if (slidergroup == &self->modamount) {
		*value = self->instrument
			? self->instrument->filtermodamount + 0.5f
			: 0.5f;
	}
}

void InitSamplerInstrumentPitchView(SamplerInstrumentPitchView* self,
	ui_component* parent, Instruments* instruments)
{
	self->instruments = instruments;	
	ui_component_init(&self->component, parent);	
}

void SetInstrumentInstrumentPitchView(SamplerInstrumentPitchView* self,
	Instrument* instrument)
{
	// char buffer[20];
	self->instrument = instrument;	
}

// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "instrumentview.h"
#include <portable.h>

static void OnInstrumentInsert(InstrumentView* self, ui_component* sender, int slot);
static void OnInstrumentRemoved(InstrumentView* self, ui_component* sender, int slot);
static void OnInstrumentSlotChanged(InstrumentView* self, Instrument* sender, int slot);
static void OnInstrumentListChanged(InstrumentView* self, ui_component* sender, int slot);
static void SetInstrument(InstrumentView* self, int slot);
static void OnSongChanged(InstrumentView* self, Workspace* workspace);
static void InitInstrumentHeaderView(InstrumentHeaderView*, ui_component* parent, Instruments*);
static void SetInstrumentInstrumentHeaderView(InstrumentHeaderView*, Instrument*);
static void OnPrevInstrument(InstrumentHeaderView*, ui_component* sender);
static void OnNextInstrument(InstrumentHeaderView*, ui_component* sender);
static void OnDeleteInstrument(InstrumentHeaderView*, ui_component* sender);
static void InitInstrumentGeneralView(InstrumentGeneralView*, ui_component* parent, Instruments*);
static void SetInstrumentInstrumentGeneralView(InstrumentGeneralView* self, Instrument* instrument);
static void OnNNACut(InstrumentGeneralView* self);
static void OnNNARelease(InstrumentGeneralView* self);
static void OnNNANone(InstrumentGeneralView* self);
static void InitInstrumentVolumeView(InstrumentVolumeView*, ui_component* parent, Instruments*);
static void OnVolumeViewDescribe(InstrumentVolumeView*, ui_slider*, char* txt);
static void OnVolumeViewTweak(InstrumentVolumeView*, ui_slider*, float value);
static void OnVolumeViewValue(InstrumentVolumeView*, ui_slider*, float* value);
static void SetInstrumentInstrumentVolumeView(InstrumentVolumeView* self, Instrument* instrument);
static void InitInstrumentPanView(InstrumentPanView*, ui_component* parent, Instruments*);
static void SetInstrumentInstrumentPanView(InstrumentPanView* self, Instrument* instrument);
static void InitInstrumentFilterView(InstrumentFilterView*, ui_component* parent, Instruments*);
static void SetInstrumentInstrumentFilterView(InstrumentFilterView* self, Instrument* instrument);
static void OnFilterViewDescribe(InstrumentFilterView*, ui_slider*, char* txt);
static void OnFilterViewTweak(InstrumentFilterView*, ui_slider*, float value);
static void OnFilterViewValue(InstrumentFilterView*, ui_slider*, float* value);
static void InitInstrumentPitchView(InstrumentPitchView*, ui_component* parent, Instruments*);
static void SetInstrumentInstrumentPitchView(InstrumentPitchView*, Instrument*);
static void instrumentkeyboardview_ondraw(InstrumentKeyboardView*, ui_component* sender, ui_graphics* g);
static void instrumententryview_init(InstrumentEntryView*, ui_component* parent);
static void instrumententryview_ondraw(InstrumentEntryView*, ui_component* sender,
	ui_graphics* g);
static void instrumententryview_setinstrument(InstrumentEntryView*, Instrument*);
static void instrumentviewbuttons_init(InstrumentViewButtons*,
	ui_component* parent);
static int isblack(int key);

void instrumentview_init(InstrumentView* self, ui_component* parent,
							   Workspace* workspace)
{
	self->player = &workspace->player;
	ui_component_init(&self->component, parent);
	ui_component_enablealign(&self->component);	
	// header
	InitInstrumentHeaderView(&self->header, &self->component,
		&workspace->song->instruments);
	ui_component_setalign(&self->header.component, UI_ALIGN_TOP);
	// left
	ui_component_init(&self->left, &self->component);
	ui_component_enablealign(&self->left);	
	ui_component_setalign(&self->left, UI_ALIGN_LEFT);
	// ui_label_init(&self->label, &self->left);
	// ui_label_settext(&self->label, "Instruments");
	// ui_label_setcharnumber(&self->label, 25);
	// ui_component_setalign(&self->label.component, UI_ALIGN_TOP);
	instrumentviewbuttons_init(&self->buttons, &self->left);
	ui_component_setalign(&self->buttons.component, UI_ALIGN_TOP);	
	InitInstrumentsBox(&self->instrumentsbox, &self->left,
		&workspace->song->instruments);
	ui_component_setalign(&self->instrumentsbox.instrumentlist.component,
		UI_ALIGN_CLIENT);
	{
		ui_margin margin;

		ui_margin_init(&margin, ui_value_makeeh(1.5), ui_value_makepx(0),
		ui_value_makepx(0), ui_value_makepx(0));
		ui_component_setmargin(&self->instrumentsbox.instrumentlist.component,
			&margin);
	}
	// client
	ui_component_init(&self->client, &self->component);
	ui_component_enablealign(&self->client);	
	ui_component_setalign(&self->client, UI_ALIGN_CLIENT);
	tabbar_init(&self->tabbar, &self->client);
	ui_component_setalign(&self->tabbar.component, UI_ALIGN_TOP);
	tabbar_append(&self->tabbar, "General");
	tabbar_append(&self->tabbar, "Volume");
	tabbar_append(&self->tabbar, "Pan");
	tabbar_append(&self->tabbar, "Filter");
	tabbar_append(&self->tabbar, "Pitch");	
	ui_notebook_init(&self->notebook, &self->client);
	ui_component_setalign(&self->notebook.component, UI_ALIGN_CLIENT);
	//ui_component_setbackgroundmode(&self->notebook.component, BACKGROUND_SET);
	InitInstrumentGeneralView(&self->general, &self->notebook.component, &workspace->song->instruments);
	InitInstrumentVolumeView(&self->volume, &self->notebook.component, &workspace->song->instruments);
	InitInstrumentPanView(&self->pan, &self->notebook.component, &workspace->song->instruments);
	InitInstrumentFilterView(&self->filter, &self->notebook.component, &workspace->song->instruments);
	InitInstrumentPitchView(&self->pitch, &self->notebook.component, &workspace->song->instruments);	
	ui_notebook_connectcontroller(&self->notebook, &self->tabbar.signal_change);	
	psy_signal_connect(&self->player->song->instruments.signal_insert, self,
		OnInstrumentInsert);
	psy_signal_connect(&self->player->song->instruments.signal_removed, self,
		OnInstrumentRemoved);
	psy_signal_connect(&self->player->song->instruments.signal_slotchange, self,
		OnInstrumentSlotChanged);
	psy_signal_connect(&self->instrumentsbox.instrumentlist.signal_selchanged, self,
		OnInstrumentListChanged);
	ui_notebook_setpageindex(&self->notebook, 0);
	psy_signal_connect(&workspace->signal_songchanged, self, OnSongChanged);
}

void OnInstrumentInsert(InstrumentView* self, ui_component* sender, int slot)
{
	SetInstrument(self, slot);
}

void OnInstrumentRemoved(InstrumentView* self, ui_component* sender, int slot)
{
	SetInstrument(self, slot);
}

void OnInstrumentSlotChanged(InstrumentView* self, Instrument* sender, int slot)
{	
	SetInstrument(self, slot);
}

void OnInstrumentListChanged(InstrumentView* self, ui_component* sender, int slot)
{
	instruments_changeslot(&self->player->song->instruments, slot);
}

void SetInstrument(InstrumentView* self, int slot)
{
	Instrument* instrument = instruments_at(&self->player->song->instruments, slot);	
	SetInstrumentInstrumentHeaderView(&self->header, instrument);
	SetInstrumentInstrumentGeneralView(&self->general, instrument);
	SetInstrumentInstrumentVolumeView(&self->volume, instrument);
	SetInstrumentInstrumentPanView(&self->pan, instrument);
	SetInstrumentInstrumentFilterView(&self->filter, instrument);
	SetInstrumentInstrumentPitchView(&self->pitch, instrument);
	
}

void OnSongChanged(InstrumentView* self, Workspace* workspace)
{	
	self->header.instruments = &workspace->song->instruments;
	self->general.instruments = &workspace->song->instruments;
	self->volume.instruments = &workspace->song->instruments;
	self->pan.instruments = &workspace->song->instruments;
	self->filter.instruments = &workspace->song->instruments;
	self->pitch.instruments = &workspace->song->instruments;
	psy_signal_connect(&workspace->song->instruments.signal_slotchange, self, OnInstrumentSlotChanged);
	psy_signal_connect(&workspace->song->instruments.signal_insert, self, OnInstrumentInsert);
	psy_signal_connect(&workspace->song->instruments.signal_removed, self, OnInstrumentRemoved);
	SetInstruments(&self->instrumentsbox, &workspace->song->instruments);
	samplesbox_setsamples(&self->general.samplesbox, &workspace->song->samples,
		&workspace->song->instruments);
	SetInstrument(self, 0);
}

void InitInstrumentHeaderView(InstrumentHeaderView* self, ui_component* parent, Instruments* instruments)
{	
	ui_margin margin;

	ui_margin_init(&margin, ui_value_makepx(0), ui_value_makeew(0.5),
		ui_value_makeeh(0.5), ui_value_makepx(0));
	self->instrument = 0;
	self->instruments = instruments;
	ui_component_init(&self->component, parent);
	ui_component_enablealign(&self->component);
	ui_label_init(&self->namelabel, &self->component);
	ui_label_settext(&self->namelabel, "Instrument Name");
	ui_edit_init(&self->nameedit, &self->component, 0);		
	ui_edit_setcharnumber(&self->nameedit, 20);	
//	psy_signal_connect(&self->nameedit.signal_change, self,
//		OnEditInstrumentName);
	ui_button_init(&self->prevbutton, &self->component);
	ui_button_seticon(&self->prevbutton, UI_ICON_LESS);	
	psy_signal_connect(&self->prevbutton.signal_clicked, self,
		OnPrevInstrument);
	ui_button_init(&self->nextbutton, &self->component);
	ui_button_seticon(&self->nextbutton, UI_ICON_MORE);	
	psy_signal_connect(&self->nextbutton.signal_clicked, self,
		OnNextInstrument);	
	list_free(ui_components_setalign(
		ui_component_children(&self->component, 0),
		UI_ALIGN_LEFT,
			&margin));
}

void SetInstrumentInstrumentHeaderView(InstrumentHeaderView* self, Instrument* instrument)
{
//	char buffer[20];
	self->instrument = instrument;
	ui_edit_settext(&self->nameedit, instrument ? instrument->name : "");
}

void OnPrevInstrument(InstrumentHeaderView* self, ui_component* sender)
{
	instruments_changeslot(self->instruments,
		instruments_slot(self->instruments) > 0 ?
		instruments_slot(self->instruments) - 1 : 0);
}

void OnNextInstrument(InstrumentHeaderView* self, ui_component* sender)
{
	instruments_changeslot(self->instruments,
		instruments_slot(self->instruments) < 255 ?
		instruments_slot(self->instruments) + 1 : 255);
}

void OnDeleteInstrument(InstrumentHeaderView* self, ui_component* sender)
{
	
}

void instrumentviewbuttons_init(InstrumentViewButtons* self,
	ui_component* parent)
{
	ui_margin margin;

	ui_margin_init(&margin, ui_value_makepx(0), ui_value_makeew(0.5),
		ui_value_makeeh(1.0), ui_value_makepx(0));
	ui_component_init(&self->component, parent);
	ui_component_enablealign(&self->component);
	ui_component_setalignexpand(&self->component, UI_HORIZONTALEXPAND);
	ui_button_init(&self->load, &self->component);
	ui_button_settext(&self->load, "Load");	
	ui_button_init(&self->save, &self->component);
	ui_button_settext(&self->save, "Save");	
	ui_button_init(&self->duplicate, &self->component);
	ui_button_settext(&self->duplicate, "Duplicate");
	ui_button_init(&self->del, &self->component);
	ui_button_settext(&self->del, "Delete");
	list_free(ui_components_setalign(
		ui_component_children(&self->component, 0),
		UI_ALIGN_LEFT, &margin));
}

// GeneralView
void InitInstrumentGeneralView(InstrumentGeneralView* self, ui_component* parent, Instruments* instruments)
{
	self->instruments = instruments;
	self->instrument = 0;
	ui_component_init(&self->component, parent);
	ui_component_enablealign(&self->component);	
	samplesbox_init(&self->samplesbox, &self->component, 0, 0);
	ui_component_resize(&self->samplesbox.component, 200, 0);
	ui_component_setalign(&self->samplesbox.component, UI_ALIGN_LEFT);	
	// nna
	ui_component_init(&self->nna, &self->component);
	ui_component_enablealign(&self->nna);
	ui_component_setalign(&self->nna, UI_ALIGN_TOP);
	ui_label_init(&self->nnaheaderlabel, &self->nna);
	ui_label_settext(&self->nnaheaderlabel, "New Note Action");	
	ui_button_init(&self->nnacutbutton, &self->nna);
	ui_button_settext(&self->nnacutbutton, "Note Cut");
	ui_component_setposition(&self->nnacutbutton.component, 5, 30, 70, 20);
	psy_signal_connect(&self->nnacutbutton.signal_clicked, self, OnNNACut);
	ui_button_init(&self->nnareleasebutton, &self->nna);
	ui_button_settext(&self->nnareleasebutton, "Note Release");
	ui_button_init(&self->nnanonebutton, &self->nna);
	ui_button_settext(&self->nnanonebutton, "None");	
	psy_signal_connect(&self->nnanonebutton.signal_clicked, self, OnNNANone);
	psy_signal_connect(&self->nnareleasebutton.signal_clicked, self,
		OnNNARelease);
	list_free(ui_components_setalign(		
		ui_component_children(&self->nna, 0),
		UI_ALIGN_LEFT, 0));			

	instrumentkeyboardview_init(&self->keyboard, &self->component);
	ui_component_setalign(&self->keyboard.component, UI_ALIGN_BOTTOM);
	instrumententryview_init(&self->entryview, &self->component);
	ui_component_setalign(&self->entryview.component, UI_ALIGN_BOTTOM);
	
	SendMessage((HWND)self->nnacutbutton.component.hwnd, BM_SETSTATE, (WPARAM)1, (LPARAM)0);
}

void SetInstrumentInstrumentGeneralView(InstrumentGeneralView* self, Instrument* instrument)
{
	// char buffer[20];
	self->instrument = instrument;	
	instrumententryview_setinstrument(&self->entryview, instrument);
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

void OnNNACut(InstrumentGeneralView* self)
{
	if (self->instrument) {
		instrument_setnna(self->instrument, NNA_STOP);
		ui_button_highlight(&self->nnacutbutton);
		ui_button_disablehighlight(&self->nnareleasebutton);
		ui_button_disablehighlight(&self->nnanonebutton);
	}
}

void OnNNARelease(InstrumentGeneralView* self)
{
	if (self->instrument) {
		instrument_setnna(self->instrument, NNA_NOTEOFF);
		ui_button_disablehighlight(&self->nnacutbutton);
		ui_button_highlight(&self->nnareleasebutton);
		ui_button_disablehighlight(&self->nnanonebutton);
	}
}

void OnNNANone(InstrumentGeneralView* self)
{
	if (self->instrument) {
		instrument_setnna(self->instrument, NNA_CONTINUE);
		ui_button_disablehighlight(&self->nnacutbutton);
		ui_button_disablehighlight(&self->nnareleasebutton);
		ui_button_highlight(&self->nnanonebutton);
	}
}

void InitInstrumentVolumeView(InstrumentVolumeView* self, ui_component* parent, Instruments* instruments)
{
	ui_margin margin;
	int i;
	ui_slider* sliders[] = {
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
	ui_margin_init(&margin,
		ui_value_makepx(20),
		ui_value_makepx(5),
		ui_value_makepx(5),
		ui_value_makepx(5));
	ui_component_setmargin(&self->envelopeview.component, &margin);	
	ui_component_resize(&self->envelopeview.component, 0, 200);	

	ui_slider_init(&self->attack, &self->component);
	ui_slider_settext(&self->attack, "Attack");
	ui_slider_init(&self->decay, &self->component);
	ui_slider_settext(&self->decay, "Decay");
	ui_slider_init(&self->sustain, &self->component);
	ui_slider_settext(&self->sustain, "Sustain Level");
	ui_slider_init(&self->release, &self->component);
	ui_slider_settext(&self->release, "Release");
	
	ui_margin_init(&margin,
		ui_value_makepx(0),
		ui_value_makepx(5),
		ui_value_makepx(5),
		ui_value_makepx(5));
	for (i = 0; i < 4; ++i) {		
		ui_component_resize(&sliders[i]->component, 100, 20);		
		ui_component_setalign(&sliders[i]->component, UI_ALIGN_TOP);
		ui_component_setmargin(&sliders[i]->component, &margin);
		ui_slider_connect(sliders[i], self, OnVolumeViewDescribe,
			OnVolumeViewTweak, OnVolumeViewValue);		
	}	
}

void SetInstrumentInstrumentVolumeView(InstrumentVolumeView* self, Instrument* instrument)
{	
	self->instrument = instrument;
	EnvelopeViewSetAdsrEnvelope(&self->envelopeview, instrument ? &instrument->volumeenvelope : 0);
}

void OnVolumeViewDescribe(InstrumentVolumeView* self, ui_slider* slidergroup, char* txt)
{
	if (slidergroup == &self->attack) {		
		if (!self->instrument) {
			psy_snprintf(txt, 10, "0ms");
		} else {
			psy_snprintf(txt, 20, "%.4fms", 
				adsr_settings_attack(&self->instrument->volumeenvelope) * 1000);				
		}		
	} else
	if (slidergroup == &self->decay) {		
		if (!self->instrument) {
			psy_snprintf(txt, 10, "0ms");
		} else {
			psy_snprintf(txt, 20, "%.4fms",
				adsr_settings_decay(&self->instrument->volumeenvelope) * 1000);
		}		
	} else
	if (slidergroup == &self->sustain) {
		if (!self->instrument) {
			psy_snprintf(txt, 10, "0%%");
		} else {
			psy_snprintf(txt, 20, "%d%%", (int)
				(adsr_settings_sustain(&self->instrument->volumeenvelope) * 100));
		}		
	} else
	if (slidergroup == &self->release) {
		if (!self->instrument) {
			psy_snprintf(txt, 10, "0ms");
		} else {
			psy_snprintf(txt, 20, "%.4fms",
				adsr_settings_release(&self->instrument->volumeenvelope) * 1000);
		}		
	}
}

void OnVolumeViewTweak(InstrumentVolumeView* self, ui_slider* slidergroup, float value)
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

void OnVolumeViewValue(InstrumentVolumeView* self, ui_slider* slidergroup, float* value)
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

void InitInstrumentPanView(InstrumentPanView* self, ui_component* parent, Instruments* instruments)
{
	self->instruments = instruments;	
	ui_component_init(&self->component, parent);	
}

void SetInstrumentInstrumentPanView(InstrumentPanView* self, Instrument* instrument)
{
	// char buffer[20];
	self->instrument = instrument;	
}

void InitInstrumentFilterView(InstrumentFilterView* self, ui_component* parent, Instruments* instruments)
{
	ui_margin margin;
	int i;
	ui_slider* sliders[] = {
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
	ui_margin_init(&margin,
		ui_value_makepx(20),
		ui_value_makepx(5),
		ui_value_makepx(5),
		ui_value_makepx(5));
	ui_component_setmargin(&self->envelopeview.component, &margin);	
	ui_component_resize(&self->envelopeview.component, 0, 100);	

	ui_slider_init(&self->attack, &self->component);
	ui_slider_settext(&self->attack, "Attack");
	ui_slider_init(&self->decay, &self->component);
	ui_slider_settext(&self->decay, "Decay");
	ui_slider_init(&self->sustain, &self->component);
	ui_slider_settext(&self->sustain, "Sustain Level");
	ui_slider_init(&self->release, &self->component);
	ui_slider_settext(&self->release, "Release");
	ui_slider_init(&self->cutoff, &self->component);
	ui_slider_settext(&self->cutoff, "Cut-off");
	ui_slider_init(&self->res, &self->component);
	ui_slider_settext(&self->res, "Res/bandw.");
	ui_slider_init(&self->modamount, &self->component);
	ui_slider_settext(&self->modamount, "Mod. Amount");
	
	ui_margin_init(&margin,
		ui_value_makepx(0),
		ui_value_makepx(5),
		ui_value_makepx(5),
		ui_value_makepx(5));
	for (i = 0; i < 7; ++i) {		
		ui_component_resize(&sliders[i]->component, 100, 20);		
		ui_component_setalign(&sliders[i]->component, UI_ALIGN_TOP);
		ui_component_setmargin(&sliders[i]->component, &margin);
		ui_slider_connect(sliders[i], self, OnFilterViewDescribe,
			OnFilterViewTweak, OnFilterViewValue);		
	}	
}

void SetInstrumentInstrumentFilterView(InstrumentFilterView* self, Instrument* instrument)
{	
	self->instrument = instrument;
	EnvelopeViewSetAdsrEnvelope(&self->envelopeview,
		instrument ? &instrument->filterenvelope : 0);
}

void OnFilterViewDescribe(InstrumentFilterView* self, ui_slider* slidergroup, char* txt)
{
	if (slidergroup == &self->attack) {		
		if (!self->instrument) {
			psy_snprintf(txt, 10, "0ms");
		} else {
			psy_snprintf(txt, 20, "%.4fms", 
				adsr_settings_attack(
					&self->instrument->filterenvelope) * 1000);				
		}		
	} else
	if (slidergroup == &self->decay) {		
		if (!self->instrument) {
			psy_snprintf(txt, 10, "0ms");
		} else {
			psy_snprintf(txt, 20, "%.4fms",
				adsr_settings_decay(
					&self->instrument->filterenvelope) * 1000);
		}		
	} else
	if (slidergroup == &self->sustain) {
		if (!self->instrument) {
			psy_snprintf(txt, 10, "0%%");
		} else {
			psy_snprintf(txt, 20, "%d%%", (int)(
				adsr_settings_sustain(
					&self->instrument->filterenvelope) * 100));
		}		
	} else
	if (slidergroup == &self->release) {
		if (!self->instrument) {
			psy_snprintf(txt, 10, "0ms");
		} else {
			psy_snprintf(txt, 20, "%.4fms",
				adsr_settings_release(
					&self->instrument->filterenvelope) * 1000);
		}		
	} else
	if (slidergroup == &self->cutoff) {
		if (!self->instrument) {
			psy_snprintf(txt, 10, "0");
		} else {
			psy_snprintf(txt, 20, "%d", (int)(self->instrument->filtercutoff * 11665 + 2333));
		}		
	} else
	if (slidergroup == &self->res) {
		if (!self->instrument) {
			psy_snprintf(txt, 10, "50%%");
		} else {
			psy_snprintf(txt, 20, "%d%%", (int)(self->instrument->filterres * 100));
		}		
	} else
	if (slidergroup == &self->modamount) {
		if (!self->instrument) {
			psy_snprintf(txt, 10, "0ms");
		} else {
			psy_snprintf(txt, 20, "%d%%", (int)(self->instrument->filtermodamount) * 100);
		}		
	}	
}

void OnFilterViewTweak(InstrumentFilterView* self,
	ui_slider* slidergroup, float value)
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

void OnFilterViewValue(InstrumentFilterView* self, ui_slider* slidergroup, float* value)
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

void InitInstrumentPitchView(InstrumentPitchView* self,
	ui_component* parent, Instruments* instruments)
{
	self->instruments = instruments;	
	ui_component_init(&self->component, parent);	
}

void SetInstrumentInstrumentPitchView(InstrumentPitchView* self,
	Instrument* instrument)
{
	// char buffer[20];
	self->instrument = instrument;	
}

void instrumentkeyboardview_init(InstrumentKeyboardView* self,
	ui_component* parent)
{
	self->dy = 0;
	ui_component_init(&self->component, parent);
	ui_component_resize(&self->component, 0, 40);
	psy_signal_connect(&self->component.signal_draw, self,
		instrumentkeyboardview_ondraw);
}

int isblack(int key)
{
	int offset = key % 12;

	return (offset == 1 || offset == 3 || offset == 6 || offset == 8 
		|| offset == 10);
	// 0 1 2 3 4 5 6 7 8 9 10 11
	// c   d   e f   g   a    h 
}

void instrumentkeyboardview_ondraw(InstrumentKeyboardView* self, ui_component* sender, ui_graphics* g)
{		
	int keymin = 0;
	int keymax = NOTECOMMANDS_RELEASE;
	int key;	
	int keyboardsize;
	int keysize = 8;
	int cp = 0;
	float top = 0.50;
	float bottom = 1 - top;
	ui_textmetric tm;
	ui_size size;

	tm = ui_component_textmetric(&self->component);
	keyboardsize = (keymax - keymin) * keysize;
	size = ui_component_size(&self->component);
	ui_setcolor(g, 0x00333333);

	ui_setbackgroundmode(g, TRANSPARENT);
	ui_settextcolor(g, 0x00333333);
	for (key = keymin; key < keymax; ++key) {					
		ui_drawline(g, cp, 0, cp, size.height);
		if (!isblack(key)) {
			ui_rectangle r;
			ui_setrectangle(&r, cp, 0, keysize, size.height);
			ui_drawsolidrectangle(g, r, 0x00CACACA);
			ui_drawline(g, cp, 0, cp, size.height);			
			cp += keysize;			
			ui_drawline(g, cp, 0, cp, size.height);			
		}
	}
	ui_settextcolor(g, 0x00CACACA);
	for (cp = 0, key = keymin; key < keymax; ++key) {							
		if (!isblack(key)) {			
			cp += keysize;			
		} else {
			ui_rectangle r;
			int x;
			int width;

			x = cp - (int)(keysize * 0.8 / 2);
			width = (int)(keysize * 0.8);
			ui_setrectangle(&r, x, 0, width, (int)(size.height * top));
			ui_drawsolidrectangle(g, r, 0x00444444);			
		}
	}
}

// entry view
void instrumententryview_init(InstrumentEntryView* self,
	ui_component* parent)
{
	self->instrument = 0;
	self->dy = 0;
	ui_component_init(&self->component, parent);	
	ui_component_resize(&self->component, 0, 40);
	psy_signal_connect(&self->component.signal_draw, self,
		instrumententryview_ondraw);
}

void instrumententryview_setinstrument(InstrumentEntryView* self, Instrument* instrument)
{
	self->instrument = instrument;
	ui_component_invalidate(&self->component);
}

int numwhitekey(int key)
{
	int octave = key / 12;
	int offset = key % 12;
	int c = 0;
	int i;

	for (i = 0; i < offset; ++i) {
		if (!isblack(i)) ++c;
	}
	return octave * 7 + c;
}

void instrumententryview_ondraw(InstrumentEntryView* self, ui_component* sender, ui_graphics* g)
{	
	if (self->instrument) {
		int cpy = 0;
		List* p;
		ui_size size;
		int keysize = 8;
		int width = keysize * numwhitekey(NOTECOMMANDS_RELEASE); 

		size = ui_component_size(&self->component);
		ui_setcolor(g, 0x00CACACA);
		for (p = self->instrument->entries; p != 0; p = p->next) {
			InstrumentEntry* entry;
			int startx;
			int endx;

			entry = (InstrumentEntry*) p->entry;
			startx = (int)(
				(float) numwhitekey(entry->keyrange.low) / 
					numwhitekey(NOTECOMMANDS_RELEASE) * width);
			endx = (int)(
				(float)numwhitekey(entry->keyrange.high) / 
					numwhitekey(NOTECOMMANDS_RELEASE) * width);			
			ui_drawline(g, startx, cpy + 5, endx, cpy + 5);
			ui_drawline(g, startx, cpy, startx, cpy + 10);
			ui_drawline(g, endx, cpy, endx, cpy + 10);
		}
	}
}

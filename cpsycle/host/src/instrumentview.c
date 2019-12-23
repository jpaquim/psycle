// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "instrumentview.h"
#include <portable.h>

static void instrumentview_oncreateinstrument(InstrumentView*,
	ui_component* sender);
static void instrumentview_onaddentry(InstrumentView*, ui_component* sender);
static void instrumentview_onremoveentry(InstrumentView*,
	ui_component* sender);
static void OnInstrumentInsert(InstrumentView*, ui_component* sender,
	int slot);
static void OnInstrumentRemoved(InstrumentView*, ui_component* sender,
	int slot);
static void OnInstrumentSlotChanged(InstrumentView*,
	psy_audio_Instrument* sender, int slot);
static void OnInstrumentListChanged(InstrumentView*, ui_component* sender,
	int slot);
static void instrumentview_setinstrument(InstrumentView*, int slot);
static void OnSongChanged(InstrumentView*, Workspace* workspace);
// InstrumentHeaderView
static void instrumentheaderview_init(InstrumentHeaderView*,
	ui_component* parent, psy_audio_Instruments*, InstrumentView*);
static void instrumentheaderview_setinstrument(InstrumentHeaderView*,
	psy_audio_Instrument*);
static void OnPrevInstrument(InstrumentHeaderView*, ui_component* sender);
static void OnNextInstrument(InstrumentHeaderView*, ui_component* sender);
static void OnDeleteInstrument(InstrumentHeaderView*, ui_component* sender);
static void OnEditInstrumentName(InstrumentHeaderView*, ui_edit* sender);
// InstrumentGeneralView
static void instrumentgeneralview_init(InstrumentGeneralView*,
	ui_component* parent, psy_audio_Instruments*);
static void instrumentgeneralview_setinstrument(InstrumentGeneralView*,
	psy_audio_Instrument* instrument);
static void OnNNACut(InstrumentGeneralView*);
static void OnNNARelease(InstrumentGeneralView*);
static void OnNNANone(InstrumentGeneralView*);
// InstrumentVolumeView
static void instrumentvolumeview_init(InstrumentVolumeView*,
	ui_component* parent, psy_audio_Instruments*);
static void OnVolumeViewDescribe(InstrumentVolumeView*, ui_slider*, char* txt);
static void OnVolumeViewTweak(InstrumentVolumeView*, ui_slider*, float value);
static void OnVolumeViewValue(InstrumentVolumeView*, ui_slider*, float* value);
static void instrumentvolumeview_setinstrument(InstrumentVolumeView* self,
	psy_audio_Instrument* instrument);
// InstrumentPanView
static void instrumentpanview_init(InstrumentPanView*, ui_component* parent,
	psy_audio_Instruments*);
static void instrumentpanview_setinstrument(InstrumentPanView* self,
	psy_audio_Instrument* instrument);
// InstrumentFilterView
static void instrumentfilterview_init(InstrumentFilterView*,
	ui_component* parent, psy_audio_Instruments*);
static void instrumentfilterview_setinstrument(InstrumentFilterView*,
	psy_audio_Instrument*);
static void OnFilterViewDescribe(InstrumentFilterView*, ui_slider*, char* txt);
static void OnFilterViewTweak(InstrumentFilterView*, ui_slider*, float value);
static void OnFilterViewValue(InstrumentFilterView*, ui_slider*, float* value);
// InstrumentPitchView
static void instrumentpitchview_init(InstrumentPitchView*,
	ui_component* parent, psy_audio_Instruments*);
static void instrumentpitchview_setinstrument(InstrumentPitchView*,
	psy_audio_Instrument*);
static void instrumentviewbuttons_init(InstrumentViewButtons*,
	ui_component* parent);

// InstrumentView
void instrumentview_init(InstrumentView* self, ui_component* parent,
	Workspace* workspace)
{
	ui_margin margin;

	ui_margin_init(&margin, ui_value_makepx(0), ui_value_makeew(1),
		ui_value_makepx(0), ui_value_makepx(0));
	self->player = &workspace->player;
	self->workspace = workspace;
	ui_component_init(&self->component, parent);
	ui_component_enablealign(&self->component);
	// header
	instrumentheaderview_init(&self->header, &self->component,
		&workspace->song->instruments, self);
	ui_component_setalign(&self->header.component, UI_ALIGN_TOP);
	// left
	ui_component_init(&self->left, &self->component);
	ui_component_enablealign(&self->left);	
	ui_component_setalign(&self->left, UI_ALIGN_LEFT);
	ui_component_setmargin(&self->left, &margin);
	// ui_label_init(&self->label, &self->left);
	// ui_label_settext(&self->label, "Instruments");
	// ui_label_setcharnumber(&self->label, 25);
	// ui_component_setalign(&self->label.component, UI_ALIGN_TOP);
	instrumentviewbuttons_init(&self->buttons, &self->left);
	ui_component_setalign(&self->buttons.component, UI_ALIGN_TOP);	
	instrumentsbox_init(&self->instrumentsbox, &self->left,
		&workspace->song->instruments);
	ui_component_setalign(&self->instrumentsbox.instrumentlist.component,
		UI_ALIGN_CLIENT);
	{
		ui_margin margin;

		ui_margin_init(&margin, ui_value_makeeh(0.5), ui_value_makepx(0),
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
	instrumentgeneralview_init(&self->general, &self->notebook.component, &workspace->song->instruments);
	instrumentvolumeview_init(&self->volume, &self->notebook.component, &workspace->song->instruments);
	instrumentpanview_init(&self->pan, &self->notebook.component, &workspace->song->instruments);
	instrumentfilterview_init(&self->filter, &self->notebook.component, &workspace->song->instruments);
	instrumentpitchview_init(&self->pitch, &self->notebook.component, &workspace->song->instruments);	
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
	samplesbox_setsamples(&self->general.samplesbox, &workspace->song->samples,
		&workspace->song->instruments);
	psy_signal_connect(&self->buttons.create.signal_clicked, self,
		instrumentview_oncreateinstrument);
	psy_signal_connect(&self->general.notemapview.buttons.add.signal_clicked, self,
		instrumentview_onaddentry);
	psy_signal_connect(&self->general.notemapview.buttons.remove.signal_clicked, self,
		instrumentview_onremoveentry);
}

void OnInstrumentInsert(InstrumentView* self, ui_component* sender, int slot)
{
	instrumentview_setinstrument(self, slot);
}

void OnInstrumentRemoved(InstrumentView* self, ui_component* sender, int slot)
{
	instrumentview_setinstrument(self, slot);
}

void OnInstrumentSlotChanged(InstrumentView* self, psy_audio_Instrument* sender, int slot)
{	
	instrumentview_setinstrument(self, slot);
}

void OnInstrumentListChanged(InstrumentView* self, ui_component* sender, int slot)
{
	instruments_changeslot(&self->player->song->instruments, slot);
}

void instrumentview_setinstrument(InstrumentView* self, int slot)
{
	psy_audio_Instrument* instrument;
	
	instrument = instruments_at(&self->player->song->instruments, slot);
	instrumentheaderview_setinstrument(&self->header, instrument);
	instrumentgeneralview_setinstrument(&self->general, instrument);
	instrumentvolumeview_setinstrument(&self->volume, instrument);
	instrumentpanview_setinstrument(&self->pan, instrument);
	instrumentfilterview_setinstrument(&self->filter, instrument);
	instrumentpitchview_setinstrument(&self->pitch, instrument);	
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
	instrumentsbox_setinstruments(&self->instrumentsbox,
		&workspace->song->instruments);
	self->general.notemapview.entryview.instrument = 0;
	instrumentnotemapview_setinstrument(&self->general.notemapview, 0);	
	samplesbox_setsamples(&self->general.samplesbox, &workspace->song->samples,
		&workspace->song->instruments);
}

void instrumentheaderview_init(InstrumentHeaderView* self, ui_component* parent,
	psy_audio_Instruments* instruments, InstrumentView* view)
{	
	ui_margin margin;

	ui_margin_init(&margin, ui_value_makepx(0), ui_value_makeew(0.5),
		ui_value_makeeh(0.5), ui_value_makepx(0));
	self->view = view;
	self->instrument = 0;
	self->instruments = instruments;
	ui_component_init(&self->component, parent);
	ui_component_enablealign(&self->component);
	ui_label_init(&self->namelabel, &self->component);
	ui_label_settext(&self->namelabel, "Instrument Name");
	ui_edit_init(&self->nameedit, &self->component, 0);		
	ui_edit_setcharnumber(&self->nameedit, 20);	
	psy_signal_connect(&self->nameedit.signal_change, self,
		OnEditInstrumentName);
	ui_button_init(&self->prevbutton, &self->component);
	ui_button_seticon(&self->prevbutton, UI_ICON_LESS);	
	psy_signal_connect(&self->prevbutton.signal_clicked, self,
		OnPrevInstrument);
	ui_button_init(&self->nextbutton, &self->component);
	ui_button_seticon(&self->nextbutton, UI_ICON_MORE);	
	psy_signal_connect(&self->nextbutton.signal_clicked, self,
		OnNextInstrument);	
	psy_list_free(ui_components_setalign(
		ui_component_children(&self->component, 0),
		UI_ALIGN_LEFT,
			&margin));
}

void instrumentheaderview_setinstrument(InstrumentHeaderView* self, psy_audio_Instrument* instrument)
{
	self->instrument = instrument;
	ui_edit_settext(&self->nameedit, instrument ? instrument->name : "");
}

void OnEditInstrumentName(InstrumentHeaderView* self, ui_edit* sender)
{
	if (self->instrument) {
		char text[40];
		int index;
		
		index = instrumentsbox_selected(&self->view->instrumentsbox);
		instrument_setname(self->instrument, ui_edit_text(sender));
		psy_snprintf(text, 20, "%02X:%s", 
			(int) index, instrument_name(self->instrument));
		ui_listbox_setstring(&self->view->instrumentsbox.instrumentlist, text,
			index);		
	}
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
		ui_value_makeeh(0.5), ui_value_makepx(0));
	ui_component_init(&self->component, parent);
	ui_component_enablealign(&self->component);
	ui_component_init(&self->row1, &self->component);
	ui_component_enablealign(&self->row1);
	ui_component_setalign(&self->row1, UI_ALIGN_TOP);
	ui_component_setalignexpand(&self->row1, UI_HORIZONTALEXPAND);
	ui_button_init(&self->create, &self->row1);
	ui_button_settext(&self->create, "New");
	ui_button_init(&self->load, &self->row1);
	ui_button_settext(&self->load, "Load");
	ui_button_init(&self->save, &self->row1);
	ui_button_settext(&self->save, "Save");
	ui_button_init(&self->duplicate, &self->row1);
	ui_button_settext(&self->duplicate, "Duplicate");	
	psy_list_free(ui_components_setalign(
		ui_component_children(&self->row1, 0),
		UI_ALIGN_LEFT, &margin));
	ui_component_init(&self->row2, &self->component);
	ui_component_enablealign(&self->row2);
	ui_component_setalign(&self->row2, UI_ALIGN_TOP);
	ui_component_setalignexpand(&self->row2, UI_HORIZONTALEXPAND);
	ui_button_init(&self->del, &self->row2);
	ui_button_settext(&self->del, "Delete");
	psy_list_free(ui_components_setalign(
		ui_component_children(&self->row2, 0),
		UI_ALIGN_LEFT, &margin));
}

// GeneralView
void instrumentgeneralview_init(InstrumentGeneralView* self, ui_component* parent, psy_audio_Instruments* instruments)
{
	ui_margin margin;

	ui_margin_init(&margin, ui_value_makepx(0), ui_value_makeew(2),
		ui_value_makeeh(1.5), ui_value_makepx(0));
	self->instruments = instruments;
	self->instrument = 0;
	ui_component_init(&self->component, parent);
	ui_component_enablealign(&self->component);
	// left
	// ui_component_init(&self->left, &self->component);
	// ui_component_enablealign(&self->left);	
	// ui_component_setalign(&self->left, UI_ALIGN_LEFT);
	samplesbox_init(&self->samplesbox, &self->component, 0, 0);
	self->samplesbox.changeinstrumentslot = 0;
	ui_component_setalign(&self->samplesbox.component, UI_ALIGN_LEFT);
	ui_component_setmargin(&self->samplesbox.component, &margin);
	ui_component_resize(&self->samplesbox.component, 150, 0);
	// nna
	ui_component_init(&self->nna, &self->component);
	ui_component_enablealign(&self->nna);
	ui_component_setalign(&self->nna, UI_ALIGN_TOP);
	ui_label_init(&self->nnaheaderlabel, &self->nna);
	ui_label_settext(&self->nnaheaderlabel, "New Note Action ");
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
	psy_list_free(ui_components_setalign(		
		ui_component_children(&self->nna, 0),
		UI_ALIGN_LEFT, &margin));			
	instrumentnotemapview_init(&self->notemapview, &self->component);
	ui_component_setalign(&self->notemapview.component, UI_ALIGN_CLIENT);
}

void instrumentgeneralview_setinstrument(InstrumentGeneralView* self, psy_audio_Instrument* instrument)
{	
	self->instrument = instrument;		
	instrumentnotemapview_setinstrument(&self->notemapview, instrument);	
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

// InistrumentVolumeView
void instrumentvolumeview_init(InstrumentVolumeView* self, ui_component* parent, psy_audio_Instruments* instruments)
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

void instrumentvolumeview_setinstrument(InstrumentVolumeView* self, psy_audio_Instrument* instrument)
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

// InstrumentPanView
void instrumentpanview_init(InstrumentPanView* self, ui_component* parent, psy_audio_Instruments* instruments)
{
	self->instruments = instruments;	
	ui_component_init(&self->component, parent);	
}

void instrumentpanview_setinstrument(InstrumentPanView* self, psy_audio_Instrument* instrument)
{
	self->instrument = instrument;	
}

// InstrumentFilterView
void instrumentfilterview_init(InstrumentFilterView* self, ui_component* parent, psy_audio_Instruments* instruments)
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

void instrumentfilterview_setinstrument(InstrumentFilterView* self, psy_audio_Instrument* instrument)
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

// InstrumentPitchView
void instrumentpitchview_init(InstrumentPitchView* self,
	ui_component* parent, psy_audio_Instruments* instruments)
{
	self->instruments = instruments;	
	ui_component_init(&self->component, parent);	
}

void instrumentpitchview_setinstrument(InstrumentPitchView* self,
	psy_audio_Instrument* instrument)
{
	self->instrument = instrument;	
}

void instrumentview_oncreateinstrument(InstrumentView* self, ui_component* sender)
{
	if (self->workspace->song) {
		psy_audio_Instrument* instrument;
		int selected;
		
		selected = instrumentsbox_selected(&self->instrumentsbox);
		if (instruments_at(&self->workspace->song->instruments, selected)) {						
			instruments_remove(&self->workspace->song->instruments, selected);
		}
		instrument = instrument_allocinit();
		instrument_setname(instrument, "Untitled");
		instruments_insert(&self->workspace->song->instruments, instrument,
			selected);
	}
}

void instrumentview_onaddentry(InstrumentView* self, ui_component* sender)
{
	if (self->general.instrument) {
		psy_audio_InstrumentEntry entry;
		
		instrumententry_init(&entry);
		entry.sampleindex = samplesbox_selected(&self->general.samplesbox);
		instrument_addentry(self->general.instrument, &entry);
		instrumentnotemapview_update(&self->general.notemapview);
	}
}

void instrumentview_onremoveentry(InstrumentView* self, ui_component* sender)
{
	if (self->general.instrument) {
		instrument_removeentry(
			self->general.instrument,
				self->general.notemapview.entryview.selected);
		self->general.notemapview.entryview.selected = NOINSTRUMENT_INDEX;
		instrumentnotemapview_update(&self->general.notemapview);
	}
}

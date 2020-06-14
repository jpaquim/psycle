// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "instrumentview.h"
#include <xm.h>

#include <uiopendialog.h>
#include <uisavedialog.h>

#include <songio.h>

#include <math.h>

#include "../../detail/portable.h"

static void instrumentview_oncreateinstrument(InstrumentView*,
	psy_ui_Component* sender);
static void instrumentview_onloadinstrument(InstrumentView*, psy_ui_Component* sender);
static void instrumentview_onaddentry(InstrumentView*, psy_ui_Component* sender);
static void instrumentview_onremoveentry(InstrumentView*,
	psy_ui_Component* sender);
static void OnInstrumentInsert(InstrumentView*, psy_audio_Instruments* sender,
	const psy_audio_InstrumentIndex* index);
static void OnInstrumentRemoved(InstrumentView*, psy_audio_Instruments* sender,
	const psy_audio_InstrumentIndex* index);
static void OnInstrumentSlotChanged(InstrumentView*,
	psy_audio_Instrument* sender, const psy_audio_InstrumentIndex* slot);
static void instrumentview_setinstrument(InstrumentView*,
	psy_audio_InstrumentIndex index);
static void OnMachinesInsert(InstrumentView* self, psy_audio_Machines* sender,
	int slot);
static void OnMachinesRemoved(InstrumentView* self, psy_audio_Machines* sender,
	int slot);
static void OnSongChanged(InstrumentView*, Workspace* workspace, int flag,
	psy_audio_SongFile* songfile);
// InstrumentHeaderView
static void instrumentheaderview_init(InstrumentHeaderView*,
	psy_ui_Component* parent, psy_audio_Instruments*, InstrumentView*);
static void instrumentheaderview_setinstrument(InstrumentHeaderView*,
	psy_audio_Instrument*);
static void OnPrevInstrument(InstrumentHeaderView*, psy_ui_Component* sender);
static void OnNextInstrument(InstrumentHeaderView*, psy_ui_Component* sender);
static void OnDeleteInstrument(InstrumentHeaderView*, psy_ui_Component* sender);
static void OnEditInstrumentName(InstrumentHeaderView*, psy_ui_Edit* sender);
// InstrumentGeneralView
static void instrumentgeneralview_init(InstrumentGeneralView*,
	psy_ui_Component* parent, psy_audio_Instruments*);
static void instrumentgeneralview_setinstrument(InstrumentGeneralView*,
	psy_audio_Instrument* instrument);
static void OnFitRow(InstrumentGeneralView*, psy_ui_Component* sender);
static void OnNNACut(InstrumentGeneralView*);
static void OnNNARelease(InstrumentGeneralView*);
static void OnNNAFadeOut(InstrumentGeneralView*);
static void OnNNANone(InstrumentGeneralView*);
static void NNAHighlight(InstrumentGeneralView*, psy_ui_Button* highlight);
static void OnGeneralViewDescribe(InstrumentGeneralView*, psy_ui_Slider*, char* text);
static void OnGeneralViewTweak(InstrumentGeneralView*, psy_ui_Slider*, float value);
static void OnGeneralViewValue(InstrumentGeneralView*, psy_ui_Slider*, float* value);
// InstrumentVolumeView
static void instrumentvolumeview_init(InstrumentVolumeView*,
	psy_ui_Component* parent, psy_audio_Instruments*);
static void OnVolumeViewDescribe(InstrumentVolumeView*, psy_ui_Slider*, char* txt);
static void OnVolumeViewTweak(InstrumentVolumeView*, psy_ui_Slider*, float value);
static void OnVolumeViewValue(InstrumentVolumeView*, psy_ui_Slider*, float* value);
static void instrumentvolumeview_setinstrument(InstrumentVolumeView* self,
	psy_audio_Instrument* instrument);
// InstrumentPanView
static void instrumentpanview_init(InstrumentPanView*, psy_ui_Component* parent,
	psy_audio_Instruments*);
static void instrumentpanview_setinstrument(InstrumentPanView* self,
	psy_audio_Instrument* instrument);
static void instrumentview_onrandompanning(InstrumentPanView* self,
	psy_ui_CheckBox* sender);

// InstrumentPitchView
static void instrumentpitchview_init(InstrumentPitchView*,
	psy_ui_Component* parent, psy_audio_Instruments*);
static void instrumentpitchview_setinstrument(InstrumentPitchView*,
	psy_audio_Instrument*);
static void instrumentviewbuttons_init(InstrumentViewButtons*,
	psy_ui_Component* parent);

// InstrumentView
void instrumentview_init(InstrumentView* self, psy_ui_Component* parent,
	psy_ui_Component* tabbarparent, Workspace* workspace)
{
	psy_ui_Margin margin;

	psy_ui_component_init(&self->component, parent);
	psy_ui_component_enablealign(&self->component);
	psy_ui_component_init(&self->viewtabbar, tabbarparent);
	self->player = &workspace->player;
	self->workspace = workspace;
	psy_ui_margin_init_all(&margin, psy_ui_value_makepx(0),
		psy_ui_value_makeew(1), psy_ui_value_makepx(0),
		psy_ui_value_makepx(0));	
	// header
	instrumentheaderview_init(&self->header, &self->component,
		&workspace->song->instruments, self);
	psy_ui_component_setalign(&self->header.component, psy_ui_ALIGN_TOP);
	// left
	psy_ui_component_init(&self->left, &self->component);
	psy_ui_component_enablealign(&self->left);	
	psy_ui_component_setalign(&self->left, psy_ui_ALIGN_LEFT);
	instrumentviewbuttons_init(&self->buttons, &self->left);
	psy_ui_component_setalign(&self->buttons.component, psy_ui_ALIGN_TOP);	
	instrumentsbox_init(&self->instrumentsbox, &self->left,
		&workspace->song->instruments);
	psy_ui_component_setalign(&self->instrumentsbox.component,
		psy_ui_ALIGN_CLIENT);
	{
		psy_ui_Margin margin;

		psy_ui_margin_init_all(&margin, psy_ui_value_makepx(0),
			psy_ui_value_makeew(2.0), psy_ui_value_makepx(0),
			psy_ui_value_makepx(0));
		psy_ui_component_setmargin(&self->left,
			&margin);		
	}
	// client
	psy_ui_component_init(&self->client, &self->component);
	psy_ui_component_enablealign(&self->client);	
	psy_ui_component_setalign(&self->client, psy_ui_ALIGN_CLIENT);
	tabbar_init(&self->tabbar, &self->client);
	psy_ui_component_setalign(tabbar_base(&self->tabbar), psy_ui_ALIGN_TOP);
	tabbar_append_tabs(&self->tabbar, "General", "Volume", "Pan", "Filter",
		"Pitch", NULL);	
	psy_ui_notebook_init(&self->notebook, &self->client);
	psy_ui_component_setalign(psy_ui_notebook_base(&self->notebook),
		psy_ui_ALIGN_CLIENT);	
	instrumentgeneralview_init(&self->general, psy_ui_notebook_base(&self->notebook), &workspace->song->instruments);
	instrumentvolumeview_init(&self->volume, psy_ui_notebook_base(&self->notebook), &workspace->song->instruments);
	instrumentpanview_init(&self->pan, psy_ui_notebook_base(&self->notebook), &workspace->song->instruments);
	instrumentfilterview_init(&self->filter, psy_ui_notebook_base(&self->notebook), &workspace->song->instruments);
	instrumentpitchview_init(&self->pitch, psy_ui_notebook_base(&self->notebook), &workspace->song->instruments);	
	psy_ui_notebook_connectcontroller(&self->notebook,
		&self->tabbar.signal_change);	
	psy_signal_connect(&self->player->song->instruments.signal_insert, self,
		OnInstrumentInsert);
	psy_signal_connect(&self->player->song->instruments.signal_removed, self,
		OnInstrumentRemoved);	
	psy_signal_connect(&self->player->song->instruments.signal_slotchange, self,
		OnInstrumentSlotChanged);
	psy_signal_connect(&self->player->song->machines.signal_insert, self,
		OnMachinesInsert);
	psy_signal_connect(&self->player->song->machines.signal_removed, self,
		OnMachinesRemoved);
	psy_ui_notebook_setpageindex(&self->notebook, 0);
	psy_signal_connect(&workspace->signal_songchanged, self, OnSongChanged);
	samplesbox_setsamples(&self->general.notemapview.samplesbox, &workspace->song->samples
		/* ,&workspace->song->instruments */);
	psy_signal_connect(&self->buttons.create.signal_clicked, self,
		instrumentview_oncreateinstrument);
	psy_signal_connect(&self->buttons.load.signal_clicked, self,
		instrumentview_onloadinstrument);
	psy_signal_connect(&self->general.notemapview.buttons.add.signal_clicked, self,
		instrumentview_onaddentry);
	psy_signal_connect(&self->general.notemapview.buttons.remove.signal_clicked, self,
		instrumentview_onremoveentry);
}

void OnInstrumentInsert(InstrumentView* self, psy_audio_Instruments* sender,
	const psy_audio_InstrumentIndex* index)
{
	if (index) {
		instrumentview_setinstrument(self, *index);
	}
}

void OnInstrumentRemoved(InstrumentView* self, psy_audio_Instruments* sender,
	const psy_audio_InstrumentIndex* index)
{
	if (index) {
		instrumentview_setinstrument(self, *index);
	}
}

void OnInstrumentSlotChanged(InstrumentView* self, psy_audio_Instrument* sender,
	const psy_audio_InstrumentIndex* index)
{	
	if (index) {
		instrumentview_setinstrument(self, *index);
	}
}

void OnMachinesInsert(InstrumentView* self, psy_audio_Machines* sender,
	int slot)
{
}

void OnMachinesRemoved(InstrumentView* self, psy_audio_Machines* sender,
	int slot)
{
}

void instrumentview_setinstrument(InstrumentView* self, psy_audio_InstrumentIndex index)
{
	psy_audio_Instrument* instrument;
	
	instrument = instruments_at(&self->player->song->instruments, index);
	instrumentheaderview_setinstrument(&self->header, instrument);
	instrumentgeneralview_setinstrument(&self->general, instrument);
	instrumentvolumeview_setinstrument(&self->volume, instrument);
	instrumentpanview_setinstrument(&self->pan, instrument);
	instrumentfilterview_setinstrument(&self->filter, instrument);
	instrumentpitchview_setinstrument(&self->pitch, instrument);
}

void OnSongChanged(InstrumentView* self, Workspace* workspace, int flag, psy_audio_SongFile* songfile)
{	
	if (workspace->song) {
		self->header.instruments = &workspace->song->instruments;
		self->general.instruments = &workspace->song->instruments;
		self->volume.instruments = &workspace->song->instruments;
		self->pan.instruments = &workspace->song->instruments;
		self->filter.instruments = &workspace->song->instruments;
		self->pitch.instruments = &workspace->song->instruments;
		psy_signal_connect(&workspace->song->instruments.signal_slotchange, self, OnInstrumentSlotChanged);
		psy_signal_connect(&workspace->song->instruments.signal_insert, self, OnInstrumentInsert);
		psy_signal_connect(&workspace->song->instruments.signal_removed, self, OnInstrumentRemoved);
		psy_signal_connect(&self->player->song->machines.signal_insert, self,
			OnMachinesInsert);
		psy_signal_connect(&self->player->song->machines.signal_removed, self,
			OnMachinesRemoved);
		instrumentsbox_setinstruments(&self->instrumentsbox,
			&workspace->song->instruments);	
		samplesbox_setsamples(&self->general.notemapview.samplesbox, &workspace->song->samples
			/*,&workspace->song->instruments*/);
	} else {
		instrumentsbox_setinstruments(&self->instrumentsbox, 0);	
		samplesbox_setsamples(&self->general.notemapview.samplesbox,
			&workspace->song->samples);
	}
	instrumentview_setinstrument(self, instrumentindex_make(0, 0));
}

// instrument header
void instrumentheaderview_init(InstrumentHeaderView* self, psy_ui_Component* parent,
	psy_audio_Instruments* instruments, InstrumentView* view)
{	
	psy_ui_Margin margin;

	psy_ui_margin_init_all(&margin, psy_ui_value_makepx(0),
		psy_ui_value_makeew(0.5), psy_ui_value_makeeh(0.5),
		psy_ui_value_makepx(0));
	self->view = view;
	self->instrument = 0;
	self->instruments = instruments;
	psy_ui_component_init(&self->component, parent);
	psy_ui_component_enablealign(&self->component);
	psy_ui_label_init(&self->namelabel, &self->component);
	psy_ui_label_settext(&self->namelabel, "Instrument Name");
	psy_ui_edit_init(&self->nameedit, &self->component);
	psy_ui_edit_setcharnumber(&self->nameedit, 20);	
	psy_signal_connect(&self->nameedit.signal_change, self,
		OnEditInstrumentName);
	psy_ui_button_init(&self->prevbutton, &self->component);
	psy_ui_button_seticon(&self->prevbutton, psy_ui_ICON_LESS);	
	psy_signal_connect(&self->prevbutton.signal_clicked, self,
		OnPrevInstrument);
	psy_ui_button_init(&self->nextbutton, &self->component);
	psy_ui_button_seticon(&self->nextbutton, psy_ui_ICON_MORE);	
	psy_signal_connect(&self->nextbutton.signal_clicked, self,
		OnNextInstrument);
	psy_list_free(psy_ui_components_setalign(
		psy_ui_component_children(&self->component, psy_ui_NONRECURSIVE),
		psy_ui_ALIGN_LEFT,
			&margin));
}

void instrumentheaderview_setinstrument(InstrumentHeaderView* self,
	psy_audio_Instrument* instrument)
{
	self->instrument = instrument;
	psy_signal_prevent(&self->nameedit.signal_change, self,
		OnEditInstrumentName);
	psy_ui_edit_settext(&self->nameedit, instrument ? instrument->name : "");
	psy_signal_enable(&self->nameedit.signal_change, self,
		OnEditInstrumentName);
}

void OnEditInstrumentName(InstrumentHeaderView* self, psy_ui_Edit* sender)
{
	if (self->instrument) {
		char text[40];
		int index;
		
		index = instrumentsbox_selected(&self->view->instrumentsbox);
		psy_audio_instrument_setname(self->instrument, psy_ui_edit_text(sender));
		psy_snprintf(text, 20, "%02X:%s", 
			(int) index, psy_audio_instrument_name(self->instrument));
		psy_ui_listbox_settext(&self->view->instrumentsbox.instrumentlist, text,
			index);		
	}
}

void OnPrevInstrument(InstrumentHeaderView* self, psy_ui_Component* sender)
{
	/*instruments_changeslot(self->instruments,
		instruments_slot(self->instruments) > 0 ?
		instruments_slot(self->instruments) - 1 : 0);*/
}

void OnNextInstrument(InstrumentHeaderView* self, psy_ui_Component* sender)
{
	/*instruments_changeslot(self->instruments,
		instruments_slot(self->instruments) < 255 ?
		instruments_slot(self->instruments) + 1 : 255);*/
}

void OnDeleteInstrument(InstrumentHeaderView* self, psy_ui_Component* sender)
{	
}

// instrumentviewbuttons
void instrumentviewbuttons_init(InstrumentViewButtons* self,
	psy_ui_Component* parent)
{
	psy_ui_Margin margin;

	psy_ui_margin_init_all(&margin, psy_ui_value_makepx(0),
		psy_ui_value_makeew(0.5), psy_ui_value_makeeh(0.5),
		psy_ui_value_makepx(0));
	psy_ui_component_init(&self->component, parent);
	psy_ui_component_enablealign(&self->component);
	psy_ui_component_init(&self->row1, &self->component);
	psy_ui_component_enablealign(&self->row1);
	psy_ui_component_setalign(&self->row1, psy_ui_ALIGN_TOP);
	psy_ui_component_setalignexpand(&self->row1, psy_ui_HORIZONTALEXPAND);
	psy_ui_button_init(&self->create, &self->row1);
	psy_ui_button_settext(&self->create, "New");
	psy_ui_button_init(&self->load, &self->row1);
	psy_ui_button_settext(&self->load, "Load");
	psy_ui_button_init(&self->save, &self->row1);
	psy_ui_button_settext(&self->save, "Save");
	psy_ui_button_init(&self->duplicate, &self->row1);
	psy_ui_button_settext(&self->duplicate, "Duplicate");	
	psy_list_free(psy_ui_components_setalign(
		psy_ui_component_children(&self->row1, psy_ui_NONRECURSIVE),
		psy_ui_ALIGN_LEFT,
		&margin));
	psy_ui_component_init(&self->row2, &self->component);
	psy_ui_component_enablealign(&self->row2);
	psy_ui_component_setalign(&self->row2, psy_ui_ALIGN_TOP);
	psy_ui_component_setalignexpand(&self->row2, psy_ui_HORIZONTALEXPAND);
	psy_ui_button_init(&self->del, &self->row2);
	psy_ui_button_settext(&self->del, "Delete");
	psy_list_free(psy_ui_components_setalign(
		psy_ui_component_children(&self->row2, psy_ui_NONRECURSIVE),
		psy_ui_ALIGN_LEFT,
		&margin));
}

// GeneralView
void instrumentgeneralview_init(InstrumentGeneralView* self, psy_ui_Component* parent, psy_audio_Instruments* instruments)
{
	psy_ui_Margin margin;

	psy_ui_margin_init_all(&margin, psy_ui_value_makepx(0),
		psy_ui_value_makeew(2), psy_ui_value_makeeh(1.5),
		psy_ui_value_makepx(0));
	self->instruments = instruments;
	self->instrument = 0;
	psy_ui_component_init(&self->component, parent);
	psy_ui_component_enablealign(&self->component);
	// left	
	psy_ui_component_setalign(&self->notemapview.samplesbox.component, psy_ui_ALIGN_LEFT);
	psy_ui_component_setmargin(&self->notemapview.samplesbox.component, &margin);
	// nna
	psy_ui_component_init(&self->nna, &self->component);
	psy_ui_component_enablealign(&self->nna);
	psy_ui_component_setalign(&self->nna, psy_ui_ALIGN_TOP);
	psy_ui_label_init(&self->nnaheader, &self->nna);
	psy_ui_label_settext(&self->nnaheader, "New Note Action ");
	psy_ui_button_init(&self->nnacut, &self->nna);
	psy_ui_button_settext(&self->nnacut, "Note Cut");	
	psy_signal_connect(&self->nnacut.signal_clicked, self, OnNNACut);
	psy_ui_button_init(&self->nnarelease, &self->nna);
	psy_ui_button_settext(&self->nnarelease, "Note Release");
	psy_ui_button_init(&self->nnafadeout, &self->nna);
	psy_ui_button_settext(&self->nnafadeout, "Note FadeOut");
	psy_ui_button_init(&self->nnanone, &self->nna);
	psy_ui_button_settext(&self->nnanone, "None");	
	psy_signal_connect(&self->nnanone.signal_clicked, self, OnNNANone);
	psy_signal_connect(&self->nnarelease.signal_clicked, self,
		OnNNARelease);
	psy_signal_connect(&self->nnafadeout.signal_clicked, self,
		OnNNAFadeOut);
	psy_list_free(psy_ui_components_setalign(
		psy_ui_component_children(&self->nna, psy_ui_NONRECURSIVE),
		psy_ui_ALIGN_LEFT,
		&margin));
	// fitrow
	psy_ui_component_init(&self->fitrow, &self->component);
	psy_ui_component_enablealign(&self->fitrow);
	psy_ui_component_setalign(&self->fitrow, psy_ui_ALIGN_TOP);
	psy_ui_checkbox_init(&self->fitrowcheck, &self->fitrow);
	psy_ui_checkbox_settext(&self->fitrowcheck, "Play sample to fit");
	psy_signal_connect(&self->fitrowcheck.signal_clicked, self, OnFitRow);
	psy_ui_edit_init(&self->fitrowedit, &self->fitrow);
	psy_ui_edit_setcharnumber(&self->fitrowedit, 4);
	psy_ui_label_init(&self->fitrowlabel, &self->fitrow);
	psy_ui_label_settext(&self->fitrowlabel, "pattern rows");
	psy_list_free(psy_ui_components_setalign(
		psy_ui_component_children(&self->fitrow, psy_ui_NONRECURSIVE),
		psy_ui_ALIGN_LEFT,
		&margin));
	psy_ui_slider_init(&self->globalvolume, &self->component);
	psy_ui_slider_settext(&self->globalvolume, "Global Volume");	
	psy_ui_component_setalign(&self->globalvolume.component, psy_ui_ALIGN_TOP);
	psy_ui_slider_connect(&self->globalvolume, self, OnGeneralViewDescribe,
			OnGeneralViewTweak, OnGeneralViewValue);
	instrumentnotemapview_init(&self->notemapview, &self->component);
	psy_ui_component_setalign(&self->notemapview.component, psy_ui_ALIGN_CLIENT);
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
		psy_ui_edit_settext(&self->fitrowedit, text);
		switch (self->instrument->nna) {
			case psy_audio_NNA_STOP:
				NNAHighlight(self, &self->nnacut);
			break;
			case psy_audio_NNA_NOTEOFF:
				NNAHighlight(self, &self->nnarelease);
			break;
			case psy_audio_NNA_FADEOUT:
				NNAHighlight(self, &self->nnafadeout);
			break;
			case psy_audio_NNA_CONTINUE:
				NNAHighlight(self, &self->nnanone);
			break;
			default:
				NNAHighlight(self, &self->nnacut);
			break;
		}
	} else {
		NNAHighlight(self, &self->nnacut);
	}
}

void OnFitRow(InstrumentGeneralView* self, psy_ui_Component* sender)
{
	if (self->instrument) {
		if (psy_ui_checkbox_checked(&self->fitrowcheck)) {
			self->instrument->loop = TRUE;
		} else {
			self->instrument->loop = FALSE;
		}
	}
}

void OnNNACut(InstrumentGeneralView* self)
{
	if (self->instrument) {
		psy_audio_instrument_setnna(self->instrument, psy_audio_NNA_STOP);
		NNAHighlight(self, &self->nnacut);
	}
}

void OnNNARelease(InstrumentGeneralView* self)
{
	if (self->instrument) {
		psy_audio_instrument_setnna(self->instrument, psy_audio_NNA_NOTEOFF);
		NNAHighlight(self, &self->nnarelease);
	}
}

void OnNNAFadeOut(InstrumentGeneralView* self)
{
	if (self->instrument) {
		psy_audio_instrument_setnna(self->instrument, psy_audio_NNA_FADEOUT);		
		NNAHighlight(self, &self->nnafadeout);
	}
}

void OnNNANone(InstrumentGeneralView* self)
{
	if (self->instrument) {
		psy_audio_instrument_setnna(self->instrument, psy_audio_NNA_CONTINUE);
		NNAHighlight(self, &self->nnanone);		
	}
}

void NNAHighlight(InstrumentGeneralView* self, psy_ui_Button* highlight)
{
	psy_ui_button_highlight(highlight);
	if (highlight != &self->nnacut) {
		psy_ui_button_disablehighlight(&self->nnacut);
	}
	if (highlight != &self->nnafadeout) {
		psy_ui_button_disablehighlight(&self->nnafadeout);
	}
	if (highlight != &self->nnarelease) {
		psy_ui_button_disablehighlight(&self->nnarelease);
	}
	if (highlight != &self->nnanone) {
		psy_ui_button_disablehighlight(&self->nnanone);
	}	
}

void OnGeneralViewTweak(InstrumentGeneralView* self, psy_ui_Slider* slider,
	float value)
{
	if (self->instrument) {					
		if (slider == &self->globalvolume) {		
			self->instrument->globalvolume = (value * value);
		}
	}
}

void OnGeneralViewValue(InstrumentGeneralView* self, psy_ui_Slider* slider,
	float* value)
{		
	if (slider == &self->globalvolume) {
		if (self->instrument) {			
			*value = (float)sqrt(self->instrument->globalvolume);
		} else {
			*value = 1.f;
		}
	}
}

void OnGeneralViewDescribe(InstrumentGeneralView* self, psy_ui_Slider* slider,
	char* text)
{			
	if (slider == &self->globalvolume) {		
		if (!self->instrument) {
			psy_snprintf(text, 10, "0.0dB");
		} else
		if (self->instrument->globalvolume == 0) {
			psy_snprintf(text, 10, "-inf. dB");
		} else {
			float db = (float)(20 * log10(self->instrument->globalvolume));
			psy_snprintf(text, 10, "%.1f dB", db);
		}
	}
}

// InstrumentVolumeView
void instrumentvolumeview_init(InstrumentVolumeView* self, psy_ui_Component* parent, psy_audio_Instruments* instruments)
{
	psy_ui_Margin margin;
	int i;
	psy_ui_Slider* sliders[] = {
		&self->attack,
		&self->decay,
		&self->sustain,
		&self->release
	};

	self->instruments = instruments;	
	self->instrument = 0;
	psy_ui_component_init(&self->component, parent);	
	psy_ui_component_enablealign(&self->component);	
	envelopeview_init(&self->envelopeview, &self->component);
	envelopeview_settext(&self->envelopeview, "Amplitude envelope");
	psy_ui_component_setalign(&self->envelopeview.component, psy_ui_ALIGN_TOP);
	psy_ui_margin_init_all(&margin,
		psy_ui_value_makepx(20),
		psy_ui_value_makepx(5),
		psy_ui_value_makepx(5),
		psy_ui_value_makepx(5));
	psy_ui_component_setmargin(&self->envelopeview.component, &margin);		
	psy_ui_slider_init(&self->attack, &self->component);
	psy_ui_slider_settext(&self->attack, "Attack");
	psy_ui_slider_init(&self->decay, &self->component);
	psy_ui_slider_settext(&self->decay, "Decay");
	psy_ui_slider_init(&self->sustain, &self->component);
	psy_ui_slider_settext(&self->sustain, "Sustain Level");
	psy_ui_slider_init(&self->release, &self->component);
	psy_ui_slider_settext(&self->release, "Release");
	
	psy_ui_margin_init_all(&margin,
		psy_ui_value_makepx(0),
		psy_ui_value_makepx(5),
		psy_ui_value_makepx(5),
		psy_ui_value_makepx(5));
	for (i = 0; i < 4; ++i) {				
		psy_ui_component_setalign(&sliders[i]->component, psy_ui_ALIGN_TOP);
		psy_ui_component_setmargin(&sliders[i]->component, &margin);
		psy_ui_slider_connect(sliders[i], self, OnVolumeViewDescribe,
			OnVolumeViewTweak, OnVolumeViewValue);		
	}	
}

void instrumentvolumeview_setinstrument(InstrumentVolumeView* self, psy_audio_Instrument* instrument)
{	
	self->instrument = instrument;
	envelopeview_setadsrenvelope(&self->envelopeview,
		instrument
		? &instrument->volumeenvelope
		: 0);
}

void OnVolumeViewDescribe(InstrumentVolumeView* self, psy_ui_Slider* slidergroup, char* txt)
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

void OnVolumeViewTweak(InstrumentVolumeView* self, psy_ui_Slider* slidergroup, float value)
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
	envelopeview_update(&self->envelopeview);
}

void OnVolumeViewValue(InstrumentVolumeView* self, psy_ui_Slider* slidergroup, float* value)
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
void instrumentpanview_init(InstrumentPanView* self, psy_ui_Component* parent, psy_audio_Instruments* instruments)
{
	psy_ui_Margin margin;

	psy_ui_margin_init_all(&margin, psy_ui_value_makepx(0),
		psy_ui_value_makepx(0), psy_ui_value_makeeh(1.5),
		psy_ui_value_makepx(0));
	self->instrument = 0;
	self->instruments = instruments;	
	psy_ui_component_init(&self->component, parent);
	psy_ui_component_enablealign(&self->component);
	psy_ui_checkbox_init(&self->randompanning, &self->component);
	psy_ui_checkbox_settext(&self->randompanning, "Random panning");
	psy_signal_connect(&self->randompanning.signal_clicked, self,
		instrumentview_onrandompanning);
	psy_list_free(psy_ui_components_setalign(
		psy_ui_component_children(&self->component, psy_ui_NONRECURSIVE),
		psy_ui_ALIGN_TOP,
		&margin));
}

void instrumentpanview_setinstrument(InstrumentPanView* self,
	psy_audio_Instrument* instrument)
{	
	self->instrument = instrument;
	if (self->instrument && self->instrument->randompan) {
		psy_ui_checkbox_check(&self->randompanning);
	} else {
		psy_ui_checkbox_disablecheck(&self->randompanning);	
	}
}

void instrumentview_onrandompanning(InstrumentPanView* self, psy_ui_CheckBox* sender)
{
	if (self->instrument) {
		self->instrument->randompan = psy_ui_checkbox_checked(sender);
	}
}

// InstrumentFilterView
static void fillfiltercombobox(InstrumentFilterView* self);
static void OnFilterViewDescribe(InstrumentFilterView*, psy_ui_Slider*, char* txt);
static void OnFilterViewTweak(InstrumentFilterView*, psy_ui_Slider*, float value);
static void OnFilterViewValue(InstrumentFilterView*, psy_ui_Slider*, float* value);
static void OnFilterComboBoxChanged(InstrumentFilterView*, psy_ui_ComboBox* sender, int index);

void instrumentfilterview_init(InstrumentFilterView* self, psy_ui_Component* parent, psy_audio_Instruments* instruments)
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
		&self->modamount
	};

	self->instruments = instruments;
	self->instrument = 0;
	psy_ui_component_init(&self->component, parent);
	psy_ui_component_enablealign(&self->component);
	psy_ui_component_init(&self->filter, &self->component);
	psy_ui_component_enablealign(&self->filter);
	psy_ui_component_setalign(&self->filter, psy_ui_ALIGN_TOP);	
	psy_ui_label_init(&self->filtertypeheader, &self->filter);
	psy_ui_label_settext(&self->filtertypeheader, "Filter type");	
	psy_ui_component_setalign(&self->filtertypeheader.component, psy_ui_ALIGN_LEFT);
	psy_ui_margin_init_all(&margin,
		psy_ui_value_makepx(0), psy_ui_value_makeew(2),
		psy_ui_value_makepx(0), psy_ui_value_makepx(0));
	psy_ui_component_setmargin(&self->filtertypeheader.component, &margin);
	psy_ui_combobox_init(&self->filtertype, &self->filter);
	psy_ui_combobox_setcharnumber(&self->filtertype, 20);
	psy_ui_component_setalign(&self->filtertype.component, psy_ui_ALIGN_LEFT);	
	fillfiltercombobox(self);
	psy_ui_combobox_setcursel(&self->filtertype, (int)F_NONE);
	psy_signal_connect(&self->filtertype.signal_selchanged, self,
		OnFilterComboBoxChanged);
	envelopeview_init(&self->envelopeview, &self->component);
	envelopeview_settext(&self->envelopeview, "Filter envelope");
	psy_ui_component_setalign(&self->envelopeview.component, psy_ui_ALIGN_TOP);
	psy_ui_margin_init_all(&margin,
		psy_ui_value_makepx(20),
		psy_ui_value_makepx(5),
		psy_ui_value_makepx(5),
		psy_ui_value_makepx(5));
	psy_ui_component_setmargin(&self->envelopeview.component, &margin);	
	//psy_ui_component_resize(&self->envelopeview.component,
		//psy_ui_value_makepx(0),
		//psy_ui_value_makeeh(20));
	psy_ui_slider_init(&self->attack, &self->component);
	psy_ui_slider_settext(&self->attack, "Attack");
	psy_ui_slider_init(&self->decay, &self->component);
	psy_ui_slider_settext(&self->decay, "Decay");
	psy_ui_slider_init(&self->sustain, &self->component);
	psy_ui_slider_settext(&self->sustain, "Sustain Level");
	psy_ui_slider_init(&self->release, &self->component);
	psy_ui_slider_settext(&self->release, "Release");
	psy_ui_slider_init(&self->cutoff, &self->component);
	psy_ui_slider_settext(&self->cutoff, "Cut-off");
	psy_ui_slider_init(&self->res, &self->component);
	psy_ui_slider_settext(&self->res, "Res/bandw.");
	psy_ui_slider_init(&self->modamount, &self->component);
	psy_ui_slider_settext(&self->modamount, "Mod. Amount");
	
	psy_ui_margin_init_all(&margin,
		psy_ui_value_makepx(0),
		psy_ui_value_makepx(5),
		psy_ui_value_makepx(5),
		psy_ui_value_makepx(5));
	for (i = 0; i < 7; ++i) {				
		psy_ui_component_setalign(&sliders[i]->component, psy_ui_ALIGN_TOP);
		psy_ui_component_setmargin(&sliders[i]->component, &margin);
		psy_ui_slider_connect(sliders[i], self, OnFilterViewDescribe,
			OnFilterViewTweak, OnFilterViewValue);		
	}	
}

void fillfiltercombobox(InstrumentFilterView* self)
{
	uintptr_t ft;
	for (ft = 0; ft < filter_numfilters(); ++ft) {
		psy_ui_combobox_addtext(&self->filtertype, filter_name((FilterType)ft));
	}
}

void OnFilterComboBoxChanged(InstrumentFilterView* self, psy_ui_ComboBox* sender, int index)
{
	self->instrument->filtertype = (FilterType)index;
}

void instrumentfilterview_setinstrument(InstrumentFilterView* self, psy_audio_Instrument* instrument)
{	
	self->instrument = instrument;
	if (self->instrument) {
		envelopeview_setadsrenvelope(&self->envelopeview,
			&instrument->filterenvelope);
		psy_ui_combobox_setcursel(&self->filtertype, (intptr_t)instrument->filtertype);
	} else {
		envelopeview_setadsrenvelope(&self->envelopeview, NULL);
		psy_ui_combobox_setcursel(&self->filtertype, (intptr_t)F_NONE);
	}	
}

void OnFilterViewDescribe(InstrumentFilterView* self, psy_ui_Slider* slidergroup, char* txt)
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
	psy_ui_Slider* slidergroup, float value)
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
	envelopeview_update(&self->envelopeview);
}

void OnFilterViewValue(InstrumentFilterView* self, psy_ui_Slider* slidergroup, float* value)
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
	psy_ui_Component* parent, psy_audio_Instruments* instruments)
{
	self->instruments = instruments;	
	psy_ui_component_init(&self->component, parent);
}

void instrumentpitchview_setinstrument(InstrumentPitchView* self,
	psy_audio_Instrument* instrument)
{
	self->instrument = instrument;	
}

void instrumentview_oncreateinstrument(InstrumentView* self, psy_ui_Component* sender)
{
	if (self->workspace->song) {
		psy_audio_Instrument* instrument;
		int selected;
		
		selected = instrumentsbox_selected(&self->instrumentsbox);
		if (instruments_at(&self->workspace->song->instruments, instrumentindex_make(0, selected))) {
			instruments_remove(&self->workspace->song->instruments, instrumentindex_make(0, selected));
		}
		instrument = psy_audio_instrument_allocinit();
		psy_audio_instrument_setname(instrument, "Untitled");
		psy_audio_instrument_setindex(instrument, selected);
		instruments_insert(&self->workspace->song->instruments, instrument,
			instrumentindex_make(0, selected));
	}
}

void instrumentview_onloadinstrument(InstrumentView* self, psy_ui_Component* sender)
{
	if (self->workspace->song) {
		psy_ui_OpenDialog dialog;
		static char filter[] =
			"Instrument (*.xi)|*.xi";		

		psy_ui_opendialog_init_all(&dialog, 0, "Load Instrument", filter, "XI",
			workspace_samples_directory(self->workspace));
		if (psy_ui_opendialog_execute(&dialog)) {
			psy_audio_Sample* sample;
			psy_audio_SampleIndex index;
			psy_audio_SongFile songfile;
			PsyFile file;

			psy_audio_songfile_init(&songfile);
			songfile.song = self->workspace->song;
			songfile.file = &file;
			if (psyfile_open(&file, psy_ui_opendialog_filename(&dialog))) {
				psy_audio_xi_load(&songfile, instruments_slot(
					&self->workspace->song->instruments).subslot);
			}
			psyfile_close(&file);			
			psy_audio_songfile_dispose(&songfile);	
			instrumentsbox_rebuild(&self->instrumentsbox);
		}
		psy_ui_opendialog_dispose(&dialog);
	}
}

void instrumentview_onaddentry(InstrumentView* self, psy_ui_Component* sender)
{
	if (self->general.instrument) {
		psy_audio_InstrumentEntry entry;
		
		psy_audio_instrumententry_init(&entry);
		entry.sampleindex = samplesbox_selected(&self->general.notemapview.samplesbox);
		psy_audio_instrument_addentry(self->general.instrument, &entry);
		instrumentnotemapview_update(&self->general.notemapview);
	}
}

void instrumentview_onremoveentry(InstrumentView* self, psy_ui_Component* sender)
{
	if (self->general.instrument) {
		psy_audio_instrument_removeentry(
			self->general.instrument,
				self->general.notemapview.entryview.selected);
		self->general.notemapview.entryview.selected = UINTPTR_MAX;
		instrumentnotemapview_update(&self->general.notemapview);
	}
}

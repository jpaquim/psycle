// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#include "samplerinstrumentview.h"

static void OnDraw(SamplerInstrumentView* self, ui_component* sender, ui_graphics* g);
static void OnSize(SamplerInstrumentView*, ui_component* sender, int width, int height);
static void AddString(SamplerInstrumentView* self, const char* text);
static void AlignInstrumentView(SamplerInstrumentView* self);
static void OnInstrumentInsert(SamplerInstrumentView* self, ui_component* sender, int slot);
static void OnInstrumentSlotChanged(SamplerInstrumentView* self, Instrument* sender, int slot);
static void OnInstrumentListChanged(SamplerInstrumentView* self, ui_component* sender, int slot);
static void BuildInstrumentList(SamplerInstrumentView* self);
static void SetInstrument(SamplerInstrumentView* self, int slot);
static void OnTabBarChange(SamplerInstrumentView* self, ui_component* sender, int tabindex);
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
static void SetInstrumentInstrumentVolumeView(SamplerInstrumentVolumeView* self, Instrument* instrument);
static void InitSamplerInstrumentPanView(SamplerInstrumentPanView*, ui_component* parent, Instruments*);
static void SetInstrumentInstrumentPanView(SamplerInstrumentPanView* self, Instrument* instrument);
static void InitSamplerInstrumentFilterView(SamplerInstrumentFilterView*, ui_component* parent, Instruments*);
static void SetInstrumentInstrumentFilterView(SamplerInstrumentFilterView* self, Instrument* instrument);
static void InitSamplerInstrumentPitchView(SamplerInstrumentPitchView*, ui_component* parent, Instruments*);
static void SetInstrumentInstrumentPitchView(SamplerInstrumentPitchView* self, Instrument* instrument);


void InitSamplerInstrumentView(SamplerInstrumentView* self, ui_component* parent, Player* player)
{
	self->player = player;
	ui_component_init(&self->component, parent);
	signal_connect(&self->component.signal_draw, self, OnDraw);
	signal_connect(&self->component.signal_size, self, OnSize);
	InitSamplerInstrumentHeaderView(&self->header, &self->component, &player->song->instruments);
	InitSamplerInstrumentGeneralView(&self->general, &self->component, &player->song->instruments);
	InitSamplerInstrumentVolumeView(&self->volume, &self->component, &player->song->instruments);
	InitSamplerInstrumentPanView(&self->pan, &self->component, &player->song->instruments);
	InitSamplerInstrumentFilterView(&self->filter, &self->component, &player->song->instruments);
	InitSamplerInstrumentPitchView(&self->pitch, &self->component, &player->song->instruments);
	ui_listbox_init(&self->instrumentlist, &self->component);
	InitTabBar(&self->tabbar, &self->component);	
	tabbar_append(&self->tabbar, "General");
	tabbar_append(&self->tabbar, "Volume");
	tabbar_append(&self->tabbar, "Pan");
	tabbar_append(&self->tabbar, "Filter");
	tabbar_append(&self->tabbar, "Pitch");
	self->tabbar.tabwidth = 80;
	self->tabbar.selected = 0;
	signal_connect(&self->tabbar.signal_change, self, OnTabBarChange);

	AlignInstrumentView(self);
	BuildInstrumentList(self);
	signal_connect(&self->player->song->instruments.signal_insert, self, OnInstrumentInsert);
	signal_connect(&self->player->song->instruments.signal_slotchange, self, OnInstrumentSlotChanged);
	signal_connect(&self->instrumentlist.signal_selchanged, self, OnInstrumentListChanged);

	ui_component_hide(&self->volume.component);
	ui_component_hide(&self->pan.component);
	ui_component_hide(&self->filter.component);
	ui_component_hide(&self->pitch.component);
}

void AlignInstrumentView(SamplerInstrumentView* self)
{
	ui_size size = ui_component_size(&self->component);

	ui_component_setposition(&self->instrumentlist.component,		 5,   5, 210, 400);
	ui_component_setposition(&self->header.component,			   220,   5, 500,  20);
	ui_component_setposition(&self->tabbar.component,			   220,  30, 500,  20);
	ui_component_setposition(&self->general.component,			   220,  60, size.width - 220, size.height - 60);
	ui_component_setposition(&self->volume.component,			   220,  60, size.width - 220, size.height - 60);
	ui_component_setposition(&self->pan.component,				   220,  60, size.width - 220, size.height - 60);
	ui_component_setposition(&self->filter.component,			   220,  60, size.width - 220, size.height - 60);
	ui_component_setposition(&self->pitch.component,			   220,  60, size.width - 220, size.height - 60);
}

void OnSize(SamplerInstrumentView* self, ui_component* sender, int width, int height)
{
	AlignInstrumentView(self);
}

void OnInstrumentInsert(SamplerInstrumentView* self, ui_component* sender, int slot)
{
	BuildInstrumentList(self);
	ui_listbox_setcursel(&self->instrumentlist, slot);	
	SetInstrument(self, slot);
}

void OnInstrumentSlotChanged(SamplerInstrumentView* self, Instrument* sender, int slot)
{
	ui_listbox_setcursel(&self->instrumentlist, slot);
	SetInstrument(self, slot);
}

void OnInstrumentListChanged(SamplerInstrumentView* self, ui_component* sender, int slot)
{
	instruments_changeslot(&self->player->song->instruments, slot);
}

void SetInstrument(SamplerInstrumentView* self, int slot)
{
	Instrument* instrument = SearchIntHashTable(&self->player->song->instruments.container, slot);	
	SetInstrumentInstrumentHeaderView(&self->header, instrument);
	SetInstrumentInstrumentGeneralView(&self->general, instrument);
	SetInstrumentInstrumentVolumeView(&self->volume, instrument);
	SetInstrumentInstrumentPanView(&self->pan, instrument);
	SetInstrumentInstrumentFilterView(&self->filter, instrument);
	SetInstrumentInstrumentPitchView(&self->pitch, instrument);
}

void BuildInstrumentList(SamplerInstrumentView* self)
{
	Instrument* instrument;
	int slot = 0;
	char buffer[20];

	ui_listbox_clear(&self->instrumentlist);
	for ( ; slot < 256; ++slot) {		
		if (instrument = SearchIntHashTable(&self->player->song->instruments.container, slot)) {
			_snprintf(buffer, 20, "%02X:%s", slot, instrument_name(instrument));
		} else {
			_snprintf(buffer, 20, "%02X:%s", slot, "");
		}
		AddString(self, buffer);
	}
}

void AddString(SamplerInstrumentView* self, const char* text)
{
	ui_listbox_addstring(&self->instrumentlist, text);
}

void OnDraw(SamplerInstrumentView* self, ui_component* sender, ui_graphics* g)
{
	ui_rectangle r;
	ui_size size = ui_component_size(&self->component);	
	ui_setrectangle(&r, 0, 0, size.width, size.height);
	ui_drawsolidrectangle(g, r, 0x009a887c);
}

void OnTabBarChange(SamplerInstrumentView* self, ui_component* sender, int tabindex)
{
	switch (tabindex) {
		case 0:						
			ui_component_show(&self->general.component);
			ui_component_hide(&self->volume.component);
			ui_component_hide(&self->pan.component);
			ui_component_hide(&self->filter.component);
			ui_component_hide(&self->pitch.component);
		break;
		case 1:			
			ui_component_hide(&self->general.component);
			ui_component_show(&self->volume.component);
			ui_component_hide(&self->pan.component);
			ui_component_hide(&self->filter.component);
			ui_component_hide(&self->pitch.component);
		break;
		case 2:
			ui_component_hide(&self->general.component);
			ui_component_hide(&self->volume.component);
			ui_component_show(&self->pan.component);
			ui_component_hide(&self->filter.component);
			ui_component_hide(&self->pitch.component);
		break;
		case 3:
			ui_component_hide(&self->general.component);
			ui_component_hide(&self->volume.component);
			ui_component_hide(&self->pan.component);
			ui_component_show(&self->filter.component);
			ui_component_hide(&self->pitch.component);
		break;
		case 4:
			ui_component_hide(&self->general.component);
			ui_component_hide(&self->volume.component);
			ui_component_hide(&self->pan.component);
			ui_component_hide(&self->filter.component);
			ui_component_show(&self->pitch.component);
		break;
		default:
		break;
	}
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
	ui_component_init(&self->component, parent);
	ui_component_enablealign(&self->component);
	ui_groupbox_init(&self->groupbox, &self->component);	
	ui_groupbox_settext(&self->groupbox, "Amplitude envelope");	
	ui_component_setalign(&self->groupbox.component, UI_ALIGN_FILL);

	InitEnvelopeView(&self->envelopeview, &self->component);
	ui_component_setalign(&self->envelopeview.component, UI_ALIGN_TOP);
	ui_setmargin(&margin, 20, 5, 5, 5);
	ui_component_setmargin(&self->envelopeview.component, &margin);	
	ui_component_resize(&self->envelopeview.component, 0, 200);	

	InitSliderGroup(&self->attack, &self->component, "Attack");
	InitSliderGroup(&self->decay, &self->component, "Decay");
	InitSliderGroup(&self->sustain, &self->component, "Sustain Level");
	InitSliderGroup(&self->release, &self->component, "Release");
	
	ui_setmargin(&margin, 0, 5, 5, 5);
	for (i = 0; i < 4; ++i) {		
		ui_component_resize(&sliders[i]->component, 100, 20);		
		ui_component_setalign(&sliders[i]->component, UI_ALIGN_TOP);
		ui_component_setmargin(&sliders[i]->component, &margin);
	//	SliderGroupConnect(sliders[i], self, OnGeneralViewDescribe,
	//		OnGeneralViewTweak, OnGeneralViewValue);		
	}	
}

void SetInstrumentInstrumentVolumeView(SamplerInstrumentVolumeView* self, Instrument* instrument)
{
	// char buffer[20];
	self->instrument = instrument;	
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
		&self->release
	};

	self->instruments = instruments;	
	ui_component_init(&self->component, parent);
	ui_component_enablealign(&self->component);
	ui_groupbox_init(&self->groupbox, &self->component);	
	ui_groupbox_settext(&self->groupbox, "Filter envelope");	
	ui_component_setalign(&self->groupbox.component, UI_ALIGN_FILL);

	InitEnvelopeView(&self->envelopeview, &self->component);
	ui_component_setalign(&self->envelopeview.component, UI_ALIGN_TOP);
	ui_setmargin(&margin, 20, 5, 5, 5);
	ui_component_setmargin(&self->envelopeview.component, &margin);	
	ui_component_resize(&self->envelopeview.component, 0, 200);	

	InitSliderGroup(&self->attack, &self->component, "Attack");
	InitSliderGroup(&self->decay, &self->component, "Decay");
	InitSliderGroup(&self->sustain, &self->component, "Sustain Level");
	InitSliderGroup(&self->release, &self->component, "Release");
	
	ui_setmargin(&margin, 0, 5, 5, 5);
	for (i = 0; i < 4; ++i) {		
		ui_component_resize(&sliders[i]->component, 100, 20);		
		ui_component_setalign(&sliders[i]->component, UI_ALIGN_TOP);
		ui_component_setmargin(&sliders[i]->component, &margin);
	//	SliderGroupConnect(sliders[i], self, OnGeneralViewDescribe,
	//		OnGeneralViewTweak, OnGeneralViewValue);		
	}	
}

void SetInstrumentInstrumentFilterView(SamplerInstrumentFilterView* self, Instrument* instrument)
{
	// char buffer[20];
	self->instrument = instrument;	
}

void InitSamplerInstrumentPitchView(SamplerInstrumentPitchView* self, ui_component* parent, Instruments* instruments)
{
	self->instruments = instruments;	
	ui_component_init(&self->component, parent);	
}

void SetInstrumentInstrumentPitchView(SamplerInstrumentPitchView* self, Instrument* instrument)
{
	// char buffer[20];
	self->instrument = instrument;	
}


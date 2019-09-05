// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#include "samplesview.h"
#include <instruments.h>
#include <instrument.h>


extern HINSTANCE appInstance;

static void OnDraw(SamplesView* self, ui_component* sender, ui_graphics* g);
static void AddString(SamplesView* self, const char* text);
static void BuildSampleList(SamplesView* self);
static void AlignSamplesView(SamplesView* self);
static void OnSampleListChanged(SamplesView* self, ui_component* sender, int slot);
static void OnTabBarChange(SamplesView* self, ui_component* sender, int tabindex);
static void InitSamplesHeaderView(SamplesHeaderView*, ui_component* parent);
static void InitSamplesGeneralView(SamplesGeneralView*, ui_component* parent);
static void InitSamplesVibratoView(SamplesVibratoView*, ui_component* parent);
static void InitSamplesWaveLoopView(SamplesWaveLoopView*, ui_component* parent);
static void SetSampleSamplesHeaderView(SamplesHeaderView*, Sample* sample);
static void SetSampleSamplesGeneralView(SamplesGeneralView*, Sample* sample);
static void SetSampleSamplesVibratoView(SamplesVibratoView*, Sample* sample);
static void SetSampleSamplesWaveLoopView(SamplesWaveLoopView*, Sample* sample);
static void OnLoadSample(SamplesView*, ui_component* sender);
static void OnSize(SamplesView*, ui_component* sender, int width, int height);
static void OnSizeGeneralView(SamplesGeneralView*, ui_component* sender, int width, int height);
static void OnSizeVibratoView(SamplesVibratoView*, ui_component* sender, int width, int height);
static void OnGeneralViewDescribeValue(SamplesGeneralView*, SliderGroup*, char* txt);

extern char* notes_tab_a440[256];

void InitSamplesView(SamplesView* self, ui_component* parent, Player* player)
{
	self->player = player;
	ui_component_init(&self->component, parent);
	signal_connect(&self->component.signal_draw, self, OnDraw);
	signal_connect(&self->component.signal_size, self, OnSize);
	ui_listbox_init(&self->samplelist, &self->component);
	InitSamplesHeaderView(&self->header, &self->component);
	InitTabBar(&self->tabbar, &self->component);	
	tabbar_append(&self->tabbar, "General");
	tabbar_append(&self->tabbar, "Vibrato");	
	self->tabbar.tabwidth = 80;
	self->tabbar.selected = 0;
	signal_connect(&self->tabbar.signal_change, self, OnTabBarChange);
	InitSamplesGeneralView(&self->general, &self->component);
	InitSamplesVibratoView(&self->vibrato, &self->component);
	InitWaveBox(&self->wavebox, &self->component);
	InitSamplesWaveLoopView(&self->waveloop, &self->component);		
	ui_button_init(&self->loadbutton, &self->component);
	ui_button_settext(&self->loadbutton, "Load");
	signal_connect(&self->loadbutton.signal_clicked, self, OnLoadSample);
	ui_button_init(&self->savebutton, &self->component);
	ui_button_settext(&self->savebutton, "Save");
	ui_button_init(&self->duplicatebutton, &self->component);
	ui_button_settext(&self->duplicatebutton, "Duplicate");
	ui_button_init(&self->deletebutton, &self->component);
	ui_button_settext(&self->deletebutton, "Delete");
	ui_button_init(&self->waveeditorbutton, &self->component);
	ui_button_settext(&self->waveeditorbutton, "Wave Editor");
	AlignSamplesView(self);	
	BuildSampleList(self);
	signal_connect(&self->samplelist.signal_selchanged, self, OnSampleListChanged);	
	
	ui_component_hide(&self->vibrato.component);
}

void OnSize(SamplesView* self, ui_component* sender, int width, int height)
{
	ui_component_setposition(&self->general.component, 220,  55, width - 225, 155);
	ui_component_setposition(&self->vibrato.component, 220,  55, width - 225, 155);
	ui_component_setposition(&self->wavebox.component, 220, 240, width - 225, 130);		
}

void AlignSamplesView(SamplesView* self)
{
	ui_component_setposition(&self->samplelist.component,		 5,   5, 210, 400);
	ui_component_setposition(&self->header.component,		   220,   5, 600,  20);
	ui_component_setposition(&self->tabbar.component,		   220,  30, 420,  20);
	ui_component_setposition(&self->general.component, 		   220,  55, 420, 155);
	ui_component_setposition(&self->vibrato.component, 		   220,  55, 420, 155);
	ui_component_setposition(&self->waveeditorbutton.component, 220, 215, 150,  20);
	ui_component_setposition(&self->wavebox.component,		   220, 240, 420, 130);		
	ui_component_setposition(&self->waveloop.component,		   220, 375, 420,  80);
	ui_component_setposition(&self->loadbutton.component,         5, 400,  40,  20);
	ui_component_setposition(&self->savebutton.component,        50, 400,  40,  20);
	ui_component_setposition(&self->duplicatebutton.component,   95, 400,  65,  20);
	ui_component_setposition(&self->deletebutton.component,     165, 400,  50,  20);
}

void BuildSampleList(SamplesView* self)
{	
	Sample* sample;
	int slot = 0;
	char buffer[20];
	ui_listbox_clear(&self->samplelist);
	for ( ; slot < 256; ++slot) {		
		if (sample = SearchIntHashTable(&self->player->song->samples.container, slot)) {
			_snprintf(buffer, 20, "%02X:%s", slot, sample_name(sample));
		} else {
			_snprintf(buffer, 20, "%02X:%s", slot, "");
		}
		AddString(self, buffer);
	}
}

void AddString(SamplesView* self, const char* text)
{
	ui_listbox_addstring(&self->samplelist, text);
}

void OnSampleListChanged(SamplesView* self, ui_component* sender, int slot)
{
	Sample* sample = SearchIntHashTable(&self->player->song->samples.container, slot);
	self->wavebox.sample = sample;
	SetSampleSamplesHeaderView(&self->header, sample);
	SetSampleSamplesGeneralView(&self->general, sample);
	SetSampleSamplesVibratoView(&self->vibrato, sample);
	SetSampleSamplesWaveLoopView(&self->waveloop, sample);
	ui_invalidate(&self->wavebox.component);	
}

void OnLoadSample(SamplesView* self, ui_component* sender)
{	
	OPENFILENAME ofn; 
	char szOpenName[MAX_PATH]       = "";
	char szOpenTitle[MAX_PATH]      = ""; 
	static char szFilter[]			 = "Wav Files (*.wav)\0*.wav\0" "IFF Samples (*.iff)\0*.iff\0" "All Files (*.*)\0*.*\0";
	char  szDefExtension[]          = "WAV"; 
	*szOpenName = '\0'; 

	ofn.lStructSize= sizeof(OPENFILENAME); 
	ofn.hwndOwner= self->component.hwnd; 
	ofn.lpstrFilter= szFilter; 
	ofn.lpstrCustomFilter= (LPSTR)NULL; 
	ofn.nMaxCustFilter= 0L; 
	ofn.nFilterIndex= 1L; 
	ofn.lpstrFile= szOpenName; 
	ofn.nMaxFile= MAX_PATH; 
	ofn.lpstrFileTitle= szOpenTitle; 
	ofn.nMaxFileTitle= MAX_PATH; 
	ofn.lpstrTitle= (LPSTR)NULL; 
	ofn.lpstrInitialDir= (LPSTR)NULL; 
	ofn.Flags= OFN_HIDEREADONLY|OFN_FILEMUSTEXIST; 
	ofn.nFileOffset= 0; 
	ofn.nFileExtension= 0; 
	ofn.lpstrDefExt= szDefExtension; 

	if (GetOpenFileName(&ofn)) {
		int slot;
		Sample* sample;
		Instrument* instrument;

		sample = (Sample*)malloc(sizeof(Sample));
		sample_init(sample);
		sample_load(sample, szOpenName);
		slot = ui_listbox_cursel(&self->samplelist);
		samples_insert(&self->player->song->samples, sample, slot);
		instrument = (Instrument*)malloc(sizeof(Instrument));		
		instrument_init(instrument);
		instrument_setname(instrument, sample_name(sample));
		instruments_insert(&self->player->song->instruments, instrument, slot);	
		BuildSampleList(self);
		ui_listbox_setcursel(&self->samplelist, slot);	
		OnSampleListChanged(self, &self->samplelist.component, slot);
	}
	return; 
}

void OnTabBarChange(SamplesView* self, ui_component* sender, int tabindex)
{
	switch (tabindex) {
		case 0:						
			ui_component_show(&self->general.component);
			ui_component_hide(&self->vibrato.component);	
			ui_invalidate(&self->component);
		break;
		case 1:			
			ui_component_hide(&self->general.component);
			ui_component_show(&self->vibrato.component);
			ui_invalidate(&self->component);
		break;
		default:
		break;
	}
}

void InitSamplesHeaderView(SamplesHeaderView* self, ui_component* parent)
{
	ui_component_init(&self->component, parent);

	ui_label_init(&self->namelabel, &self->component);
	ui_label_settext(&self->namelabel, "Wave Name");
	ui_component_setposition(&self->namelabel.component, 0, 0, 85, 20);

	ui_edit_init(&self->nameedit, &self->component, 0);	
	ui_component_setposition(&self->nameedit.component, 100, 0, 85, 20);

	ui_label_init(&self->srlabel, &self->component);
	ui_label_settext(&self->srlabel, "Sample Rate");
	ui_component_setposition(&self->srlabel.component, 200, 0, 85, 20);

	ui_edit_init(&self->sredit, &self->component, 0);	
	ui_component_setposition(&self->sredit.component, 300, 0, 50, 20);

	ui_label_init(&self->numsamplesheaderlabel, &self->component);
	ui_label_settext(&self->numsamplesheaderlabel, "Samples");
	ui_component_setposition(&self->numsamplesheaderlabel.component, 360, 0, 60, 20);

	ui_label_init(&self->numsampleslabel, &self->component);
	ui_label_settext(&self->numsampleslabel, "");
	ui_component_setposition(&self->numsampleslabel.component, 430, 0, 60, 20);

	ui_label_init(&self->channellabel, &self->component);
	ui_label_settext(&self->channellabel, "");
	ui_component_setposition(&self->channellabel.component, 495, 0, 50, 20);
}

void SetSampleSamplesHeaderView(SamplesHeaderView* self, Sample* sample)
{
	char buffer[20];
	self->sample = sample;
	ui_edit_settext(&self->nameedit, self->sample ? sample->name : "");	
	_snprintf(buffer, 20, "%d", self->sample ? self->sample->samplerate : 0);
	ui_edit_settext(&self->sredit, buffer);
	_snprintf(buffer, 20, "%d", self->sample ? self->sample->numframes : 0);
	ui_label_settext(&self->numsampleslabel, buffer);	
	if (self->sample) {
		if (self->sample->channels.numchannels == 1) {
			_snprintf(buffer, 20, "Mono");
		} else
		if (self->sample->channels.numchannels == 2) {
			_snprintf(buffer, 20, "Stereo");
		} else
		if (self->sample->channels.numchannels > 3) {
			_snprintf(buffer, 20, "%d Chs", self->sample->channels.numchannels);
		}		
	} else {
		_snprintf(buffer, 20, "");
	}
	ui_label_settext(&self->channellabel, buffer);
}

void InitSamplesGeneralView(SamplesGeneralView* self, ui_component* parent)
{
	ui_component_init(&self->component, parent);
	signal_connect(&self->component.signal_size, self, OnSizeGeneralView);
	self->sample = 0;

	ui_groupbox_init(&self->groupbox, &self->component);
	ui_groupbox_settext(&self->groupbox, "General");
	ui_component_setposition(&self->groupbox.component, 0, 0, 410, 155);

	InitSliderGroup(&self->defaultvolume, &self->component, "Default Volume");
	InitSliderGroup(&self->globalvolume, &self->component, "Global Volume");
	InitSliderGroup(&self->panposition, &self->component, "Pan Position");
	InitSliderGroup(&self->samplednote, &self->component, "Sampled Note"); 
	InitSliderGroup(&self->pitchfinetune, &self->component, "Pitch Finetune");
	
	signal_connect(&self->defaultvolume.signal_describevalue, self, OnGeneralViewDescribeValue);
	signal_connect(&self->globalvolume.signal_describevalue, self, OnGeneralViewDescribeValue);
	signal_connect(&self->panposition.signal_describevalue, self, OnGeneralViewDescribeValue);
	signal_connect(&self->samplednote.signal_describevalue, self, OnGeneralViewDescribeValue);
	signal_connect(&self->pitchfinetune.signal_describevalue, self, OnGeneralViewDescribeValue);
}

void SetSampleSamplesGeneralView(SamplesGeneralView* self, Sample* sample)
{
	self->sample = sample;
}

void OnSizeGeneralView(SamplesGeneralView* self, ui_component* sender, int width, int height)
{
	ui_component_setposition(&self->groupbox.component, 0, 0, width, height);
	ui_component_setposition(&self->defaultvolume.component, 10, 25, width - 10, 20);
	ui_component_setposition(&self->globalvolume.component, 10, 50, width - 10, 20);
	ui_component_setposition(&self->panposition.component, 10, 75, width - 10, 20);
	ui_component_setposition(&self->samplednote.component, 10, 100, width - 10, 20);
	ui_component_setposition(&self->pitchfinetune.component, 10, 125, width - 10, 20);
}

int map_1_128(float value) {
	return (int)(value * 128.f);
}

void OnGeneralViewDescribeValue(SamplesGeneralView* self, SliderGroup* slidergroup, char* txt)
{	
	if (slidergroup == &self->defaultvolume) {

		_snprintf(txt, 10, "C%02X", map_1_128(SliderGroupValue(slidergroup)));
	} else
	if (slidergroup == &self->globalvolume) {
		_snprintf(txt, 10, "%.2f dB", SliderGroupValue(slidergroup));
	} else
	if (slidergroup == &self->panposition) {
		_snprintf(txt, 10, "%.2f", SliderGroupValue(slidergroup));
	} else
	if (slidergroup == &self->samplednote) {		
		_snprintf(txt, 10, "%s",
			notes_tab_a440[(int)(SliderGroupValue(slidergroup) * 119.f)]);
	} else
	if (slidergroup == &self->pitchfinetune) {
		_snprintf(txt, 10, "%d ct.", (int)(SliderGroupValue(slidergroup) * 200) - 100);
	}
}

void InitSamplesVibratoView(SamplesVibratoView* self, ui_component* parent)
{
	ui_component_init(&self->component, parent);
	signal_connect(&self->component.signal_size, self, OnSizeVibratoView);
	self->sample = 0;

	ui_groupbox_init(&self->groupbox, &self->component);
	ui_groupbox_settext(&self->groupbox, "Vibrato");	

	ui_label_init(&self->waveformheaderlabel, &self->component);
	ui_label_settext(&self->waveformheaderlabel, "Waveform");
	ui_component_setposition(&self->waveformheaderlabel.component, 10, 25, 100, 20);

	ui_combobox_init(&self->waveformbox, &self->component);	
	ui_combobox_addstring(&self->waveformbox, "Sinus");
	ui_combobox_addstring(&self->waveformbox, "Square");
	ui_combobox_addstring(&self->waveformbox, "RampUp");
	ui_combobox_addstring(&self->waveformbox, "RampDown");
	ui_combobox_addstring(&self->waveformbox, "Random");	
	ui_component_setposition(&self->waveformbox.component, 115, 25, 100, 20);
	ui_combobox_setcursel(&self->waveformbox, 0);

	InitSliderGroup(&self->attack, &self->component, "Attack");	
	InitSliderGroup(&self->speed, &self->component, "Speed");
	InitSliderGroup(&self->depth, &self->component, "Depth");
	
	ui_component_setposition(&self->attack.component, 10, 50,  310, 20);
	ui_component_setposition(&self->speed.component,  10, 75,  310, 20);
	ui_component_setposition(&self->depth.component,  10, 100, 310, 20);	
}

void SetSampleSamplesVibratoView(SamplesVibratoView* self, Sample* sample)
{
	self->sample = sample;
}

void OnSizeVibratoView(SamplesVibratoView* self, ui_component* sender, int width, int height)
{
	ui_component_setposition(&self->groupbox.component, 0, 0, width, 130);
	ui_component_setposition(&self->attack.component, 10, 50,  width - 10, 20);
	ui_component_setposition(&self->speed.component,  10, 75,  width - 10, 20);
	ui_component_setposition(&self->depth.component,  10, 100, width - 10, 20);	
}

void InitSamplesWaveLoopView(SamplesWaveLoopView* self, ui_component* parent)
{
	ui_component_init(&self->component, parent);
	self->sample = 0;

	ui_groupbox_init(&self->groupbox, &self->component);
	ui_groupbox_settext(&self->groupbox, "Wave Loop");
	ui_component_setposition(&self->groupbox.component, 0, 0, 600, 80);

	ui_label_init(&self->waveloopheaderlabel, &self->component);
	ui_label_settext(&self->waveloopheaderlabel, "Continuous Loop");
	ui_component_setposition(&self->waveloopheaderlabel.component, 10, 25, 140, 20);

	ui_label_init(&self->sustainloopheaderlabel, &self->component);
	ui_label_settext(&self->sustainloopheaderlabel, "Sustain Loop");
	ui_component_setposition(&self->sustainloopheaderlabel.component, 10, 50, 140, 20);
}

void SetSampleSamplesWaveLoopView(SamplesWaveLoopView* self, Sample* sample)
{
	self->sample = sample;
}

void OnDraw(SamplesView* self, ui_component* sender, ui_graphics* g)
{
	ui_rectangle r;
	ui_size size = ui_component_size(&self->component);	
	ui_setrectangle(&r, 0, 0, size.width, size.height);
	ui_drawsolidrectangle(g, r, 0x009a887c);
}

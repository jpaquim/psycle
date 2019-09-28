// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#include "samplesview.h"
#include <instruments.h>
#include <instrument.h>
#include <math.h>

/// Samples Main View
static void OnSize(SamplesView*, ui_component* sender, int width, int height);
static void AlignSamplesView(SamplesView* self);
static void OnSampleListChanged(SamplesView* self, ui_component* sender, int slot);
static void SetSample(SamplesView*, int slot);
static void OnLoadSample(SamplesView*, ui_component* sender);
static void OnSongChanged(SamplesView* self, Workspace* workspace);
/// Header View
static void InitSamplesHeaderView(SamplesHeaderView*, ui_component* parent, Instruments* instruments, ui_listbox* samplelist);
static void SetSampleSamplesHeaderView(SamplesHeaderView*, Sample* sample);
static void OnInstrumentSlotChanged(SamplesView* self, Instrument* sender, int slot);
static void OnPrevSample(SamplesHeaderView*, ui_component* sender);
static void OnNextSample(SamplesHeaderView*, ui_component* sender);
static void OnDeleteSample(SamplesHeaderView*, ui_component* sender);
static void OnEditSampleName(SamplesHeaderView*, ui_edit* sender);
/// General Settings View
static void InitSamplesGeneralView(SamplesGeneralView*, ui_component* parent);
static void SetSampleSamplesGeneralView(SamplesGeneralView*, Sample* sample);
static void OnGeneralViewDescribe(SamplesGeneralView*, SliderGroup*, char* txt);
static void GeneralViewFillPanDescription(SamplesGeneralView*, char* txt);
static void OnGeneralViewTweak(SamplesGeneralView*, SliderGroup*, float value);
static void OnGeneralViewValue(SamplesGeneralView*, SliderGroup*, float* value);
static void InitSamplesVibratoView(SamplesVibratoView*, ui_component* parent, Player* player);
static void SetSampleSamplesVibratoView(SamplesVibratoView*, Sample* sample);
/// Vibrato Settings View
static void OnVibratoViewDescribe(SamplesVibratoView*, SliderGroup*, char* txt);
static void OnVibratoViewTweak(SamplesVibratoView*, SliderGroup*, float value);
static void OnVibratoViewValue(SamplesVibratoView*, SliderGroup*, float* value);
static void InitSamplesWaveLoopView(SamplesWaveLoopView*, ui_component* parent);
static void OnWaveFormChange(SamplesVibratoView*, ui_combobox* sender, int sel);
static WaveForms ComboBoxToWaveForm(int combobox_index);
static int WaveFormToComboBox(WaveForms waveform);
/// Waveloop Setting View
static void SetSampleSamplesWaveLoopView(SamplesWaveLoopView*, Sample* sample);
static int LoopTypeToComboBox(LoopType looptype);
static LoopType ComboBoxToLoopType(int combobox_index);
static void OnLoopTypeChange(SamplesWaveLoopView*, ui_combobox* sender, int sel);
static void OnSustainLoopTypeChange(SamplesWaveLoopView* self, ui_combobox* sender, int sel);
static void LoopTypeEnablePreventInput(SamplesWaveLoopView* self);
static void OnEditChangeLoopstart(SamplesWaveLoopView*, ui_edit* sender);
static void OnEditChangeLoopend(SamplesWaveLoopView*, ui_edit* sender);
static void OnEditChangeSustainstart(SamplesWaveLoopView*, ui_edit* sender);
static void OnEditChangeSustainend(SamplesWaveLoopView*, ui_edit* sender);

extern char* notes_tab_a440[256];

void InitSamplesView(SamplesView* self, ui_component* parent,
	ui_component* tabbarparent, Workspace* workspace)
{
	ui_margin margin = {3, 3, 0, 3};
	self->player = &workspace->player;
	ui_component_init(&self->component, parent);
	ui_component_setbackgroundmode(&self->component, BACKGROUND_SET);	
	signal_connect(&self->component.signal_size, self, OnSize);
	InitSamplesBox(&self->samplesbox, &self->component,
		&workspace->song->samples, &workspace->song->instruments);
	ui_button_init(&self->loadbutton, &self->component);
	ui_button_settext(&self->loadbutton, "Load");
	signal_connect(&self->loadbutton.signal_clicked, self, OnLoadSample);
	ui_button_init(&self->savebutton, &self->component);
	ui_button_settext(&self->savebutton, "Save");
	ui_button_init(&self->duplicatebutton, &self->component);
	ui_button_settext(&self->duplicatebutton, "Duplicate");
	ui_button_init(&self->deletebutton, &self->component);
	ui_button_settext(&self->deletebutton, "Delete");	
	ui_component_init(&self->client, &self->component);
	ui_component_enablealign(&self->client);
	InitSamplesHeaderView(&self->header, &self->component,
		&workspace->song->instruments, &self->samplesbox.samplelist);
	ui_component_resize(&self->header.component, 400, 20);	
	ui_component_setmargin(&self->header.component, &margin);
	InitTabBar(&self->tabbar, &self->client);	
	ui_component_resize(&self->tabbar.component, 0, 20);
	ui_component_setalign(&self->tabbar.component, UI_ALIGN_TOP);
	ui_component_setmargin(&self->tabbar.component, &margin);
	tabbar_append(&self->tabbar, "General");
	tabbar_append(&self->tabbar, "Vibrato");	
	ui_notebook_init(&self->notebook, &self->client);	
	ui_component_setbackgroundmode(&self->notebook.component, BACKGROUND_SET);
	ui_component_resize(&self->notebook.component, 0, 120);
	ui_component_setalign(&self->notebook.component, UI_ALIGN_TOP);	
	ui_component_setmargin(&self->notebook.component, &margin);
	ui_notebook_connectcontroller(&self->notebook, &self->tabbar.signal_change);
	InitSamplesGeneralView(&self->general, &self->notebook.component);			
	InitSamplesVibratoView(&self->vibrato, &self->notebook.component, &workspace->player);
	ui_notebook_setpage(&self->notebook, 0);
	InitWaveBox(&self->wavebox, &self->client);
	ui_component_resize(&self->wavebox.component, 0, 80);
	ui_component_setalign(&self->wavebox.component, UI_ALIGN_TOP);	
	ui_component_setmargin(&self->wavebox.component, &margin);
	ui_button_init(&self->waveeditorbutton, &self->client);
	ui_button_settext(&self->waveeditorbutton, "Wave Editor");
	ui_component_resize(&self->waveeditorbutton.component, 0, 20);
	ui_component_setalign(&self->waveeditorbutton.component, UI_ALIGN_TOP);
	ui_component_setmargin(&self->waveeditorbutton.component, &margin);
	InitSamplesWaveLoopView(&self->waveloop, &self->client);
	ui_component_resize(&self->waveloop.component, 0, 125);	
	ui_component_setalign(&self->waveloop.component, UI_ALIGN_TOP);
	ui_component_setmargin(&self->waveloop.component, &margin);
	AlignSamplesView(self);		
	signal_connect(&self->samplesbox.samplelist.signal_selchanged, self, OnSampleListChanged);	
	signal_connect(&workspace->song->instruments.signal_slotchange, self, OnInstrumentSlotChanged);	
	SetSample(self, 0);
	signal_connect(&workspace->signal_songchanged, self, OnSongChanged);
}

void OnSize(SamplesView* self, ui_component* sender, int width, int height)
{		
	AlignSamplesView(self);
}

void AlignSamplesView(SamplesView* self)
{
	ui_size size;

	size = ui_component_size(&self->component);

	ui_component_setposition(&self->header.component, 0, 0, size.width,  25);
	ui_component_setposition(&self->client, 220, 30, size.width - 220,  size.height - 30);
	ui_component_setposition(&self->loadbutton.component,         5,  30,  40,  20);
	ui_component_setposition(&self->savebutton.component,        50,  30,  40,  20);
	ui_component_setposition(&self->duplicatebutton.component,   95,  30,  65,  20);
	ui_component_setposition(&self->deletebutton.component,     165,  30,  50,  20);
	ui_component_setposition(&self->samplesbox.samplelist.component,
																  5,  50, 210, size.height - 50);
}

void OnSampleListChanged(SamplesView* self, ui_component* sender, int slot)
{	
	SetSample(self, slot);	
	signal_prevent(&self->player->song->instruments.signal_slotchange, self, OnInstrumentSlotChanged);
	instruments_changeslot(&self->player->song->instruments, slot);
	signal_enable(&self->player->song->instruments.signal_slotchange, self, OnInstrumentSlotChanged);
	ui_invalidate(&self->wavebox.component);	
}

void OnInstrumentSlotChanged(SamplesView* self, Instrument* sender, int slot)
{
	SetSample(self, slot);
}

void SetSample(SamplesView* self, int slot)
{	
	Sample* sample;	

	sample = samples_at(&self->player->song->samples, slot);
	self->wavebox.sample = sample;
	SetSampleSamplesHeaderView(&self->header, sample);
	SetSampleSamplesGeneralView(&self->general, sample);
	SetSampleSamplesVibratoView(&self->vibrato, sample);
	SetSampleSamplesWaveLoopView(&self->waveloop, sample);
	ui_listbox_setcursel(&self->samplesbox.samplelist, slot);
}

void OnLoadSample(SamplesView* self, ui_component* sender)
{	
	char path[MAX_PATH]	 = "";
	char title[MAX_PATH]	 = ""; 
	static char filter[]	 = "Wav Files (*.wav)\0*.wav\0" "IFF Samples (*.iff)\0*.iff\0" "All Files (*.*)\0*.*\0";
	char  defaultextension[] = "WAV"; 
	*path = '\0'; 
	if (ui_openfile(&self->component, title, filter, defaultextension, path)) {
		int slot;
		Sample* sample;
		Instrument* instrument;

		sample = (Sample*)malloc(sizeof(Sample));
		sample_init(sample);
		sample_load(sample, path);
		slot = ui_listbox_cursel(&self->samplesbox.samplelist);
		samples_insert(&self->player->song->samples, sample, slot);
		instrument = (Instrument*)malloc(sizeof(Instrument));		
		instrument_init(instrument);
		instrument_setname(instrument, sample_name(sample));
		instruments_insert(&self->player->song->instruments, instrument, slot);
		SetSample(self, slot);
		signal_prevent(&self->player->song->instruments.signal_slotchange, self, OnInstrumentSlotChanged);
		instruments_changeslot(&self->player->song->instruments, slot);
		signal_enable(&self->player->song->instruments.signal_slotchange, self, OnInstrumentSlotChanged);	
		ui_invalidate(&self->component);
	}	
}

void OnSongChanged(SamplesView* self, Workspace* workspace)
{	
	signal_connect(&workspace->song->instruments.signal_slotchange, self, OnInstrumentSlotChanged);	
	SetSamples(&self->samplesbox, &workspace->song->samples,
		&workspace->song->instruments);
	SetSample(self, 0);
}

void InitSamplesHeaderView(SamplesHeaderView* self, ui_component* parent, Instruments* instruments, ui_listbox* samplelist)
{
	ui_margin margin = { 5, 3, 3, 0 };

	self->instruments = instruments;
	self->samplelist = samplelist;
	ui_component_init(&self->component, parent);
	ui_component_enablealign(&self->component);

	ui_label_init(&self->namelabel, &self->component);
	ui_label_settext(&self->namelabel, "Sample Name");
	ui_component_resize(&self->namelabel.component, 90, 20);	

	ui_edit_init(&self->nameedit, &self->component, 0);		
	ui_component_resize(&self->nameedit.component, 80, 20);
	signal_connect(&self->nameedit.signal_change, self, OnEditSampleName);

	ui_button_init(&self->prevbutton, &self->component);
	ui_button_settext(&self->prevbutton, "<");
	ui_component_resize(&self->prevbutton.component, 20, 20);
	signal_connect(&self->prevbutton.signal_clicked, self, OnPrevSample);

	ui_button_init(&self->nextbutton, &self->component);
	ui_button_settext(&self->nextbutton, ">");
	ui_component_resize(&self->nextbutton.component, 20, 20);
	signal_connect(&self->nextbutton.signal_clicked, self, OnNextSample);

	ui_button_init(&self->deletebutton, &self->component);
	ui_button_settext(&self->deletebutton, "Delete");
	ui_component_resize(&self->deletebutton.component, 40, 20);
	signal_connect(&self->deletebutton.signal_clicked, self, OnDeleteSample);	

	ui_label_init(&self->srlabel, &self->component);
	ui_label_settext(&self->srlabel, "Sample Rate");
	ui_component_resize(&self->srlabel.component, 85, 20);

	ui_edit_init(&self->sredit, &self->component, 0);	
	ui_component_resize(&self->sredit.component, 50, 20);

	ui_label_init(&self->numsamplesheaderlabel, &self->component);
	ui_label_settext(&self->numsamplesheaderlabel, "Samples");
	ui_component_resize(&self->numsamplesheaderlabel.component, 60, 20);

	ui_label_init(&self->numsampleslabel, &self->component);
	ui_label_settext(&self->numsampleslabel, "");
	ui_component_resize(&self->numsampleslabel.component, 40, 20);

	ui_label_init(&self->channellabel, &self->component);
	ui_label_settext(&self->channellabel, "");
	ui_component_resize(&self->channellabel.component, 40, 20);

	{
		List* p;
		for (p = ui_component_children(&self->component, 0); p != 0; p = p->next)
		{
			ui_component_setalign((ui_component*)p->entry, UI_ALIGN_LEFT);
			ui_component_setmargin((ui_component*)p->entry, &margin);
		}
	}
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

	if (self->sample) {
		ui_component_enableinput(&self->component, 1);
	} else {
		ui_component_preventinput(&self->component, 1);
	}
}

void OnEditSampleName(SamplesHeaderView* self, ui_edit* sender)
{
	if (self->sample) {
		char txt[40];		
		sample_setname(self->sample, ui_edit_text(sender));
		_snprintf(txt, 20, "%02X:%s", ui_listbox_cursel(self->samplelist),
			sample_name(self->sample));
		ui_listbox_setstring(self->samplelist, txt,
			ui_listbox_cursel(self->samplelist));
	}
}

void OnPrevSample(SamplesHeaderView* self, ui_component* sender)
{
	instruments_changeslot(self->instruments,
		instruments_slot(self->instruments) > 0 ?
		instruments_slot(self->instruments) - 1 : 0);
}

void OnNextSample(SamplesHeaderView* self, ui_component* sender)
{
	instruments_changeslot(self->instruments,
		instruments_slot(self->instruments) < 255 ?
		instruments_slot(self->instruments) + 1 : 255);
}

void OnDeleteSample(SamplesHeaderView* self, ui_component* sender)
{

}

void InitSamplesGeneralView(SamplesGeneralView* self, ui_component* parent)
{
	int i;
	ui_margin margin = { 3, 3, 0, 3 };
	SliderGroup* sliders[] = {
		&self->defaultvolume,
		&self->globalvolume,
		&self->panposition,
		&self->samplednote,
		&self->pitchfinetune
	};	
		
	self->sample = 0;
	ui_component_init(&self->component, parent);
	ui_component_enablealign(&self->component);	
	
	InitSliderGroup(&self->defaultvolume, &self->component, "Default Volume");
	InitSliderGroup(&self->globalvolume, &self->component, "Global Volume");
	InitSliderGroup(&self->panposition, &self->component, "Pan Position");
	InitSliderGroup(&self->samplednote, &self->component, "Sampled Note"); 
	InitSliderGroup(&self->pitchfinetune, &self->component, "Pitch Finetune");	
	for (i = 0; i < 5; ++i) {		
		ui_component_resize(&sliders[i]->component, 0, 20);		
		ui_component_setalign(&sliders[i]->component, UI_ALIGN_TOP);
		ui_component_setmargin(&sliders[i]->component, &margin);
		SliderGroupConnect(sliders[i], self, OnGeneralViewDescribe,
			OnGeneralViewTweak, OnGeneralViewValue);		
	}
}

void SetSampleSamplesGeneralView(SamplesGeneralView* self, Sample* sample)
{
	self->sample = sample;
	if (self->sample) {
		ui_component_enableinput(&self->component, 1);
	} else {
		ui_component_preventinput(&self->component, 1);
	}
}

int map_1_128(float value) {
	return (int)(value * 128.f);
}

void OnGeneralViewTweak(SamplesGeneralView* self, SliderGroup* slidergroup, float value)
{
	if (!self->sample) {
		return;
	}
	if (slidergroup == &self->defaultvolume) {
		self->sample->defaultvolume = value;
	} else
	if (slidergroup == &self->globalvolume) {		
		self->sample->globalvolume = (value * value) * 4.f;
	} else
	if (slidergroup == &self->panposition) {
		self->sample->panfactor = value;
	} else
	if (slidergroup == &self->samplednote) {
		self->sample->tune = (int)(value * 119.f) - 60;
	} else
	if (slidergroup == &self->pitchfinetune) {
		self->sample->finetune = (int)(value * 200.f) - 100;
	}
}

void OnGeneralViewValue(SamplesGeneralView* self, SliderGroup* slidergroup,
	float* value)
{	
	if (slidergroup == &self->defaultvolume) {
		*value = self->sample ? self->sample->defaultvolume : 1.0f;
	} else 
	if (slidergroup == &self->globalvolume) {
		if (self->sample) {			
			*value = (float)sqrt(self->sample->globalvolume) * 0.5f;
		} else {
			*value = 0.5f;
		}
	} else 	
	if (slidergroup == &self->panposition) {
		*value = self->sample ? self->sample->panfactor : 0.5f;
	} else
	if (slidergroup == &self->samplednote) {
		*value = self->sample ? (self->sample->tune + 60) / 119.f : 0.5f;
	} else
	if (slidergroup == &self->pitchfinetune) {
		*value = self->sample ? self->sample->finetune / 200.f + 0.5f : 0.f;
	}
}

void OnGeneralViewDescribe(SamplesGeneralView* self, SliderGroup* slidergroup, char* txt)
{		
	if (slidergroup == &self->defaultvolume) {		
		_snprintf(txt, 10, "C%02X", self->sample 
			? map_1_128(self->sample->defaultvolume)
			: 0x80);
	} else
	if (slidergroup == &self->globalvolume) {		
		if (!self->sample) {
			_snprintf(txt, 10, "-inf. dB");
		} else
		if (self->sample->globalvolume == 0) {
			_snprintf(txt, 10, "-inf. dB");
		} else {
			float db = (float)(20 * log10(self->sample->globalvolume));
			_snprintf(txt, 10, "%.2f dB", db);
		}
	} else
	if (slidergroup == &self->panposition) {		
		GeneralViewFillPanDescription(self, txt);		
	} else
	if (slidergroup == &self->samplednote) {		
		_snprintf(txt, 10, "%s", self->sample
			? notes_tab_a440[self->sample->tune + 60]
			: "C4");		
	} else
	if (slidergroup == &self->pitchfinetune) {
		_snprintf(txt, 10, "%d ct.", self->sample
			? self->sample->finetune
			: 0);
	}
}

void GeneralViewFillPanDescription(SamplesGeneralView* self, char* txt) {	

	if (!self->sample) {
		_snprintf(txt, 10, "|64|");
	} else
	if (self->sample->surround) {
		_snprintf(txt, 10, "SurrounD");
	} else {		
		int pos = (int)(self->sample->panfactor * 128.f);
		if (pos == 0) {
			sprintf(txt,"||%02d  ", pos);
		} else
		if (pos < 32) {
			sprintf(txt,"<<%02d  ", pos);
		} else
		if (pos < 64) {
			sprintf(txt," <%02d< ", pos);
		} else 
		if ( pos == 64) {
			sprintf(txt," |%02d| ", pos); 
		} else
		if ( pos <= 96) {
			sprintf(txt," >%02d> ", pos);
		} else
		if (pos < 128) {
			sprintf(txt,"  %02d>>", pos);
		} else {
			sprintf(txt,"  %02d||", pos);
		}
	}	
}

void InitSamplesVibratoView(SamplesVibratoView* self, ui_component* parent, Player* player)
{
	ui_margin margin;
	int i;
	SliderGroup* sliders[] = {
		&self->attack,
		&self->speed,
		&self->depth,		
	};	

	ui_component_init(&self->component, parent);
	ui_component_enablealign(&self->component);	
	ui_component_setbackgroundmode(&self->component, BACKGROUND_SET);	
	self->sample = 0;
	self->player = player;	

	ui_label_init(&self->waveformheaderlabel, &self->component);
	ui_label_settext(&self->waveformheaderlabel, "Waveform");
	ui_component_setposition(&self->waveformheaderlabel.component, 5, 5, 100, 20);

	ui_combobox_init(&self->waveformbox, &self->component);	
	ui_combobox_addstring(&self->waveformbox, "Sinus");
	ui_combobox_addstring(&self->waveformbox, "Square");
	ui_combobox_addstring(&self->waveformbox, "RampUp");
	ui_combobox_addstring(&self->waveformbox, "RampDown");
	ui_combobox_addstring(&self->waveformbox, "Random");	
	ui_component_setposition(&self->waveformbox.component, 110, 5, 100, 20);
	ui_combobox_setcursel(&self->waveformbox, 0);
	signal_connect(&self->waveformbox.signal_selchanged, self, OnWaveFormChange);

	InitSliderGroup(&self->attack, &self->component, "Attack");	
	InitSliderGroup(&self->speed, &self->component, "Speed");
	InitSliderGroup(&self->depth, &self->component, "Depth");

	ui_setmargin(&margin, 3, 3, 0, 3);
	for (i = 0; i < 3; ++i) {		
		ui_component_resize(&sliders[i]->component, 0, 20);		
		ui_component_setalign(&sliders[i]->component, UI_ALIGN_TOP);
		ui_component_setmargin(&sliders[i]->component, &margin);
		SliderGroupConnect(sliders[i], self, OnVibratoViewDescribe,
			OnVibratoViewTweak, OnVibratoViewValue);		
	}	
	sliders[0]->component.margin.top = 32;		
}

void SetSampleSamplesVibratoView(SamplesVibratoView* self, Sample* sample)
{
	self->sample = sample;
	if (self->sample) {
		ui_component_enableinput(&self->component, 1);
		ui_combobox_setcursel(&self->waveformbox,
			WaveFormToComboBox(self->sample->vibrato.type));
	} else {
		ui_component_preventinput(&self->component, 1);
		ui_combobox_setcursel(&self->waveformbox,
			WaveFormToComboBox(WAVEFORMS_SINUS));
	}
}

void OnVibratoViewTweak(SamplesVibratoView* self, SliderGroup* slidergroup, float value)
{
	if (!self->sample) {
		return;
	}
	if (slidergroup == &self->attack) {
		self->sample->vibrato.attack = (unsigned char)(value * 255.f);
	} else
	if (slidergroup == &self->speed) {
		self->sample->vibrato.speed = (unsigned char)(value * 64.f);
	} else
	if (slidergroup == &self->depth) {
		self->sample->vibrato.depth = (unsigned char)(value * 32.f);
	}
}

void OnVibratoViewValue(SamplesVibratoView* self, SliderGroup* slidergroup,
	float* value)
{	
	if (slidergroup == &self->attack) {
		*value = self->sample ? self->sample->vibrato.attack / 255.f : 0.f;
	} else 
	if (slidergroup == &self->speed) {
		*value = self->sample ? self->sample->vibrato.speed / 64.f : 0.f;
	} else
	if (slidergroup == &self->depth) {
		*value = self->sample ? self->sample->vibrato.depth / 32.f : 0.f;
	}
}

void OnVibratoViewDescribe(SamplesVibratoView* self, SliderGroup* slidergroup, char* txt)
{		
	if (slidergroup == &self->attack) {		
		if (!self->sample) {
			_snprintf(txt, 10, "No Delay");
		} else 
		if (self->sample->vibrato.attack == 0) {
			_snprintf(txt, 10, "No Delay");
		} else {
			_snprintf(txt, 10, "%.0fms", (4096000.0f*256)
				/(self->sample->vibrato.attack*44100.f));
		}		
	} else
	if (slidergroup == &self->speed) {		
		if (!self->sample) {
			_snprintf(txt, 10, "off");
		} else
		if (self->sample->vibrato.speed == 0) {
			_snprintf(txt, 10, "off");			
		} else {		
			_snprintf(txt, 10, "%.0fms", (256000.0f*256) 
				/ (self->sample->vibrato.speed*44100.f));
		}
	} else
	if (slidergroup == &self->depth) {		
		if (!self->sample) {
			_snprintf(txt, 10, "off");
		} else
		if (self->sample->vibrato.depth == 0) {
			_snprintf(txt, 10, "off");			
		} else {
			_snprintf(txt, 10, "%d", self->sample->vibrato.depth);
		}
	}
}

void OnWaveFormChange(SamplesVibratoView* self, ui_combobox* sender, int sel)
{
	if (self->sample) {
		self->sample->vibrato.type = ComboBoxToWaveForm(sel);
	}
}

int WaveFormToComboBox(WaveForms waveform)
{
	int rv = 0;

	switch (waveform) {
		case WAVEFORMS_SINUS: rv = 0; break;		
		case WAVEFORMS_SQUARE: rv = 1; break;					
		case WAVEFORMS_SAWUP: rv = 2; break;					
		case WAVEFORMS_SAWDOWN: rv = 3; break;					
		case WAVEFORMS_RANDOM: rv = 4; break;					
		default:
		break;		
	}
	return rv;
}

WaveForms ComboBoxToWaveForm(int combobox_index)
{
	WaveForms rv = WAVEFORMS_SINUS;			
	
	switch (combobox_index) {					
		case 0: rv = WAVEFORMS_SINUS; break;
		case 1: rv = WAVEFORMS_SQUARE; break;
		case 2: rv = WAVEFORMS_SAWUP; break;
		case 3: rv = WAVEFORMS_SAWDOWN; break;
		case 4: rv = WAVEFORMS_RANDOM; break;
		default:
		break;				
	}
	return rv;
}

void InitSamplesWaveLoopView(SamplesWaveLoopView* self, ui_component* parent)
{
	ui_component_init(&self->component, parent);
	self->sample = 0;

	ui_label_init(&self->loopheaderlabel, &self->component);
	ui_label_settext(&self->loopheaderlabel, "Continuous Loop");
	ui_component_setposition(&self->loopheaderlabel.component, 10, 0, 140, 20);

	ui_combobox_init(&self->loopdir, &self->component);	
	ui_combobox_addstring(&self->loopdir, "Disabled");
	ui_combobox_addstring(&self->loopdir, "Forward");
	ui_combobox_addstring(&self->loopdir, "Bidirection");	
	ui_component_setposition(&self->loopdir.component, 155, 0, 100, 20);
	ui_combobox_setcursel(&self->loopdir, 0);

	ui_label_init(&self->loopstartlabel, &self->component);
	ui_label_settext(&self->loopstartlabel, "Start ");
	ui_component_setposition(&self->loopstartlabel.component, 10, 25, 70, 20);

	ui_edit_init(&self->loopstartedit, &self->component, 0);	
	ui_component_setposition(&self->loopstartedit.component, 85, 25, 100, 20);

	ui_label_init(&self->loopendlabel, &self->component);
	ui_label_settext(&self->loopendlabel, "End ");
	ui_component_setposition(&self->loopendlabel.component, 190, 25, 70, 20);

	ui_edit_init(&self->loopendedit, &self->component, 0);	
	ui_component_setposition(&self->loopendedit.component, 265, 25, 100, 20);

	ui_label_init(&self->sustainloopheaderlabel, &self->component);
	ui_label_settext(&self->sustainloopheaderlabel, "Sustain Loop");
	ui_component_setposition(&self->sustainloopheaderlabel.component, 10, 50, 140, 20);

	ui_combobox_init(&self->sustainloopdir, &self->component);	
	ui_combobox_addstring(&self->sustainloopdir, "Disabled");
	ui_combobox_addstring(&self->sustainloopdir, "Forward");
	ui_combobox_addstring(&self->sustainloopdir, "Bidirection");	
	ui_component_setposition(&self->sustainloopdir.component, 155, 50, 100, 20);
	ui_combobox_setcursel(&self->sustainloopdir, 0);

	ui_label_init(&self->sustainloopstartlabel, &self->component);
	ui_label_settext(&self->sustainloopstartlabel, "Start ");
	ui_component_setposition(&self->sustainloopstartlabel.component, 10, 75, 70, 20);

	ui_edit_init(&self->sustainloopstartedit, &self->component, 0);	
	ui_component_setposition(&self->sustainloopstartedit.component, 85, 75, 100, 20);

	ui_label_init(&self->sustainloopendlabel, &self->component);
	ui_label_settext(&self->sustainloopendlabel, "End ");
	ui_component_setposition(&self->sustainloopendlabel.component, 190, 75, 70, 20);

	ui_edit_init(&self->sustainloopendedit, &self->component, 0);	
	ui_component_setposition(&self->sustainloopendedit.component, 265, 75, 100, 20);

	signal_connect(&self->loopdir.signal_selchanged, self, OnLoopTypeChange);
	signal_connect(&self->sustainloopdir.signal_selchanged, self, OnSustainLoopTypeChange);
	signal_connect(&self->loopstartedit.signal_change, self, OnEditChangeLoopstart);
	signal_connect(&self->loopendedit.signal_change, self, OnEditChangeLoopend);
	signal_connect(&self->sustainloopstartedit.signal_change, self, OnEditChangeSustainstart);
	signal_connect(&self->sustainloopendedit.signal_change, self, OnEditChangeSustainend);
}

void SetSampleSamplesWaveLoopView(SamplesWaveLoopView* self, Sample* sample)
{
	char tmp[40];

	self->sample = sample;
	if (self->sample) {
		ui_component_enableinput(&self->component, 1);
		sprintf(tmp, "%d", sample->loopstart);
		ui_edit_settext(&self->loopstartedit, tmp);
		sprintf(tmp, "%d", sample->loopend);
		ui_edit_settext(&self->loopendedit, tmp);
		sprintf(tmp, "%d", sample->sustainloopstart);
		ui_edit_settext(&self->sustainloopstartedit, tmp);
		sprintf(tmp, "%d", sample->sustainloopend);
		ui_edit_settext(&self->sustainloopendedit, tmp);
		ui_combobox_setcursel(&self->loopdir, LoopTypeToComboBox(self->sample->looptype));
		ui_combobox_setcursel(&self->sustainloopdir, LoopTypeToComboBox(self->sample->sustainlooptype));
	} else {
		ui_component_preventinput(&self->component, 1);
		sprintf(tmp, "%d", 0);
		ui_edit_settext(&self->loopstartedit, tmp);		
		ui_edit_settext(&self->loopendedit, tmp);		
		ui_edit_settext(&self->sustainloopstartedit, tmp);		
		ui_edit_settext(&self->sustainloopendedit, tmp);
		ui_combobox_setcursel(&self->loopdir, LoopTypeToComboBox(LOOP_DO_NOT));
		ui_combobox_setcursel(&self->sustainloopdir, LoopTypeToComboBox(LOOP_DO_NOT));		
	}
	LoopTypeEnablePreventInput(self);
}

int LoopTypeToComboBox(LoopType looptype)
{
	int rv = 0;

	switch (looptype) {
		case LOOP_DO_NOT: rv = 0; break;
		case LOOP_NORMAL: rv = 1; break;
		case LOOP_BIDI: rv = 2; break;
		default:
		break;
	}
	return rv;
}

void OnLoopTypeChange(SamplesWaveLoopView* self, ui_combobox* sender, int sel)
{
	if (self->sample) {
		self->sample->looptype = ComboBoxToLoopType(sel);
		LoopTypeEnablePreventInput(self);
	}
}

void OnSustainLoopTypeChange(SamplesWaveLoopView* self, ui_combobox* sender, int sel)
{
	if (self->sample) {
		self->sample->sustainlooptype = ComboBoxToLoopType(sel);
		LoopTypeEnablePreventInput(self);
	}
}

void LoopTypeEnablePreventInput(SamplesWaveLoopView* self)
{
	if (self->sample) {
		if (self->sample->looptype == LOOP_DO_NOT) {
			ui_component_preventinput(&self->loopstartedit.component, 0);
			ui_component_preventinput(&self->loopendedit.component, 0);
		} else {
			ui_component_enableinput(&self->loopstartedit.component, 0);
			ui_component_enableinput(&self->loopendedit.component, 0);
		}
		if (self->sample->sustainlooptype == LOOP_DO_NOT) {
			ui_component_preventinput(&self->sustainloopstartedit.component, 0);
			ui_component_preventinput(&self->sustainloopendedit.component, 0);
		} else {
			ui_component_enableinput(&self->sustainloopstartedit.component, 0);
			ui_component_enableinput(&self->sustainloopendedit.component, 0);
		}
	} else {
		ui_component_preventinput(&self->loopstartedit.component, 0);
		ui_component_preventinput(&self->loopendedit.component, 0);
		ui_component_preventinput(&self->sustainloopstartedit.component, 0);
		ui_component_preventinput(&self->sustainloopendedit.component, 0);	
	}
}

LoopType ComboBoxToLoopType(int combobox_index)
{
	LoopType rv = 0;
			
	switch (combobox_index) {			
		case 0: rv = LOOP_DO_NOT; break;
		case 1: rv = LOOP_NORMAL; break;
		case 2: rv = LOOP_BIDI; break;
		default:
		break;
	}
	return rv;
}

void OnEditChangeLoopstart(SamplesWaveLoopView* self, ui_edit* sender)
{
	if (self->sample) {
		self->sample->loopstart = atoi(ui_edit_text(sender));
		// DrawScope();
	}
}

void OnEditChangeLoopend(SamplesWaveLoopView* self, ui_edit* sender)
{				
	if (self->sample) {
		self->sample->loopend = atoi(ui_edit_text(sender));
		// DrawScope();
	}
}

void OnEditChangeSustainstart(SamplesWaveLoopView* self, ui_edit* sender)
{
	if (self->sample) {
		self->sample->sustainloopstart = atoi(ui_edit_text(sender));
		// DrawScope();
	}
}

void OnEditChangeSustainend(SamplesWaveLoopView* self, ui_edit* sender)
{
	if (self->sample) {
		self->sample->sustainloopend = atoi(ui_edit_text(sender));
		// DrawScope();
	}
}

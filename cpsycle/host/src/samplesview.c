// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#include "samplesview.h"

static void OnDraw(SamplesView* self, ui_component* sender, ui_graphics* g);
static void AddString(SamplesView* self, const char* text);
static void BuildSampleList(SamplesView* self);
static void AlignSamplesView(SamplesView* self);
static void SetPosition(ui_component* component, int x, int y, int width, int height);
static void OnSampleListChanged(SamplesView* self, ui_component* sender, int slot);
static void InitSamplesHeaderView(SamplesHeaderView*, ui_component* parent);
static void InitSamplesGeneralView(SamplesGeneralView*, ui_component* parent);
static void InitSamplesVibratoView(SamplesVibratoView*, ui_component* parent);
static void InitSamplesWaveLoopView(SamplesWaveLoopView*, ui_component* parent);
static void SetSampleSamplesHeaderView(SamplesHeaderView*, Sample* sample);
static void SetSampleSamplesGeneralView(SamplesGeneralView*, Sample* sample);
static void SetSampleSamplesVibratoView(SamplesVibratoView*, Sample* sample);
static void SetSampleSamplesWaveLoopView(SamplesWaveLoopView*, Sample* sample);


void InitSamplesView(SamplesView* self, ui_component* parent, Player* player)
{
	self->player = player;
	ui_component_init(&self->component, parent);
	signal_connect(&self->component.signal_draw, self, OnDraw);
	ui_listbox_init(&self->samplelist, &self->component);
	InitSamplesHeaderView(&self->header, &self->component);
	InitSamplesGeneralView(&self->general, &self->component);
	InitSamplesVibratoView(&self->vibrato, &self->component);
	InitWaveBox(&self->wavebox, &self->component);
	InitSamplesWaveLoopView(&self->waveloop, &self->component);	
	ui_button_init(&self->loadbutton, &self->component);
	ui_button_settext(&self->loadbutton, "Load");
	ui_button_init(&self->savebutton, &self->component);
	ui_button_settext(&self->savebutton, "Save");
	ui_button_init(&self->duplicatebutton, &self->component);
	ui_button_settext(&self->duplicatebutton, "Duplicate");
	ui_button_init(&self->deletebutton, &self->component);
	ui_button_settext(&self->deletebutton, "Delete");
	ui_button_init(&self->waveeditorbutton, &self->component);
	ui_button_settext(&self->waveeditorbutton, "Wave Editor");
	AlignSamplesView(self);	
	//	ui_button_init(&self->loadbutton, &self->component);
	//	ui_button_settext(&self->loadbutton, "Load");
	//  ui_component_move(&self->loadbutton.component, 5,215);
	//	ui_component_resize(&self->loadbutton.component, 231,15);
	BuildSampleList(self);
	signal_connect(&self->samplelist.signal_selchanged, self, OnSampleListChanged);		
}

void AlignSamplesView(SamplesView* self)
{
	SetPosition(&self->samplelist.component,		 5,   5, 210, 400);
	SetPosition(&self->header.component,		   220,   5, 600,  20);
	SetPosition(&self->general.component, 		   220,  30, 300, 155);
	SetPosition(&self->vibrato.component, 		   525,  30, 300, 130);
	SetPosition(&self->waveeditorbutton.component, 525, 165, 300,  20);
	SetPosition(&self->wavebox.component,		   220, 205, 600, 130);		
	SetPosition(&self->waveloop.component,		   220, 340, 600,  80);
	SetPosition(&self->loadbutton.component,         5, 400,  40,  20);
	SetPosition(&self->savebutton.component,        50, 400,  40,  20);
	SetPosition(&self->duplicatebutton.component,   95, 400,  65,  20);
	SetPosition(&self->deletebutton.component,     165, 400, 50,  20);
}

void SetPosition(ui_component* component, int x, int y, int width, int height)
{
	ui_component_move(component, x, y);
	ui_component_resize(component, width, height);
}

void BuildSampleList(SamplesView* self)
{
	Sample* sample;
	int slot = 0;
	char buffer[20];
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

void InitSamplesHeaderView(SamplesHeaderView* self, ui_component* parent)
{
	ui_component_init(&self->component, parent);

	ui_label_init(&self->namelabel, &self->component);
	ui_label_settext(&self->namelabel, "Wave Name");
	SetPosition(&self->namelabel.component, 0, 0, 85, 20);

	ui_edit_init(&self->nameedit, &self->component, 0);	
	SetPosition(&self->nameedit.component, 100, 0, 85, 20);

	ui_label_init(&self->srlabel, &self->component);
	ui_label_settext(&self->srlabel, "Sample Rate");
	SetPosition(&self->srlabel.component, 200, 0, 85, 20);

	ui_edit_init(&self->sredit, &self->component, 0);	
	SetPosition(&self->sredit.component, 300, 0, 50, 20);

	ui_label_init(&self->numsamplesheaderlabel, &self->component);
	ui_label_settext(&self->numsamplesheaderlabel, "Samples");
	SetPosition(&self->numsamplesheaderlabel.component, 360, 0, 60, 20);

	ui_label_init(&self->numsampleslabel, &self->component);
	ui_label_settext(&self->numsampleslabel, "");
	SetPosition(&self->numsampleslabel.component, 430, 0, 60, 20);

	ui_label_init(&self->channellabel, &self->component);
	ui_label_settext(&self->channellabel, "");
	SetPosition(&self->channellabel.component, 495, 0, 50, 20);
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
	self->sample = 0;

	ui_groupbox_init(&self->groupbox, &self->component);
	ui_groupbox_settext(&self->groupbox, "General");
	SetPosition(&self->groupbox.component, 0, 0, 300, 155);

	ui_label_init(&self->defaultvolumeheaderlabel, &self->component);
	ui_label_settext(&self->defaultvolumeheaderlabel, "Default Volume");
	SetPosition(&self->defaultvolumeheaderlabel.component, 10, 25, 100, 20);

	ui_label_init(&self->globalvolumeheaderlabel, &self->component);
	ui_label_settext(&self->globalvolumeheaderlabel, "Global Volume");
	SetPosition(&self->globalvolumeheaderlabel.component, 10, 50, 100, 20);

	ui_label_init(&self->panpositionheaderlabel, &self->component);
	ui_label_settext(&self->panpositionheaderlabel, "Pan Position");
	SetPosition(&self->panpositionheaderlabel.component, 10, 75, 100, 20);

	ui_label_init(&self->samplednoteheaderlabel, &self->component);
	ui_label_settext(&self->samplednoteheaderlabel, "Sampled Note");
	SetPosition(&self->samplednoteheaderlabel.component, 10, 100, 100, 20);

	ui_label_init(&self->pitchfinetuneheaderslabel, &self->component);
	ui_label_settext(&self->pitchfinetuneheaderslabel, "Pitch Finetune");
	SetPosition(&self->pitchfinetuneheaderslabel.component, 10, 125, 100, 20);
}

void SetSampleSamplesGeneralView(SamplesGeneralView* self, Sample* sample)
{
	self->sample = sample;
}

void InitSamplesVibratoView(SamplesVibratoView* self, ui_component* parent)
{
	ui_component_init(&self->component, parent);
	self->sample = 0;

	ui_groupbox_init(&self->groupbox, &self->component);
	ui_groupbox_settext(&self->groupbox, "Vibrato");
	SetPosition(&self->groupbox.component, 0, 0, 300, 130);

	ui_label_init(&self->waveformheaderlabel, &self->component);
	ui_label_settext(&self->waveformheaderlabel, "Waveform");
	SetPosition(&self->waveformheaderlabel.component, 10, 25, 100, 20);

	ui_label_init(&self->attackheaderlabel, &self->component);
	ui_label_settext(&self->attackheaderlabel, "Attack");
	SetPosition(&self->attackheaderlabel.component, 10, 50, 100, 20);

	ui_label_init(&self->speedheaderlabel, &self->component);
	ui_label_settext(&self->speedheaderlabel, "Speed");
	SetPosition(&self->speedheaderlabel.component, 10, 75, 100, 20);

	ui_label_init(&self->depthheaderlabel, &self->component);
	ui_label_settext(&self->depthheaderlabel, "Depth");
	SetPosition(&self->depthheaderlabel.component, 10, 100, 100, 20);
}

void SetSampleSamplesVibratoView(SamplesVibratoView* self, Sample* sample)
{
	self->sample = sample;
}

void InitSamplesWaveLoopView(SamplesWaveLoopView* self, ui_component* parent)
{
	ui_component_init(&self->component, parent);
	self->sample = 0;

	ui_groupbox_init(&self->groupbox, &self->component);
	ui_groupbox_settext(&self->groupbox, "Wave Loop");
	SetPosition(&self->groupbox.component, 0, 0, 600, 80);

	ui_label_init(&self->waveloopheaderlabel, &self->component);
	ui_label_settext(&self->waveloopheaderlabel, "Continuous Loop");
	SetPosition(&self->waveloopheaderlabel.component, 10, 25, 140, 20);

	ui_label_init(&self->sustainloopheaderlabel, &self->component);
	ui_label_settext(&self->sustainloopheaderlabel, "Sustain Loop");
	SetPosition(&self->sustainloopheaderlabel.component, 10, 50, 140, 20);
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
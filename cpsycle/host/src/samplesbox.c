// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#include "samplesbox.h"
#include <stdio.h>

static void BuildSampleList(SamplesBox* self);
static void AddString(SamplesBox* self, const char* text);
static void OnInstrumentInsert(SamplesBox* self, ui_component* sender, int slot);
static void OnSampleListChanged(SamplesBox* self, ui_component* sender,
	int slot);

void InitSamplesBox(SamplesBox* self, ui_component* parent,
	Samples* samples, Instruments* instruments)
{	
	ui_listbox_init(&self->samplelist, parent);	
	SetSamples(self, samples, instruments);	
	signal_connect(&self->samplelist.signal_selchanged, self,
		OnSampleListChanged);
}

void BuildSampleList(SamplesBox* self)
{
	Sample* sample;
	int slot = 0;
	char buffer[20];

	ui_listbox_clear(&self->samplelist);
	for ( ; slot < 256; ++slot) {		
		if (sample = samples_at(self->samples, slot)) {
			_snprintf(buffer, 20, "%02X:%s", slot, sample_name(sample));
		} else {
			_snprintf(buffer, 20, "%02X:%s", slot, "");
		}
		AddString(self, buffer);
	}
}

void AddString(SamplesBox* self, const char* text)
{
	ui_listbox_addstring(&self->samplelist, text);
}

void OnSampleListChanged(SamplesBox* self, ui_component* sender, int slot)
{
	instruments_changeslot(self->instruments, slot);
}

void OnInstrumentInsert(SamplesBox* self, ui_component* sender, int slot)
{
	BuildSampleList(self);
	ui_listbox_setcursel(&self->samplelist, slot);		
}

void OnInstrumentsSlotChanged(SamplesBox* self, Instrument* sender, int slot)
{
	ui_listbox_setcursel(&self->samplelist, slot);	
}

void SetSamples(SamplesBox* self, Samples* samples, Instruments* instruments)
{
	self->samples = samples;
	self->instruments = instruments;
	BuildSampleList(self);
	signal_connect(&instruments->signal_insert, self, OnInstrumentInsert);	
	signal_connect(&instruments->signal_slotchange, self, OnInstrumentsSlotChanged);
}
// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "waveosc.h"

static int waveosctables_initialized = 0;
static psy_audio_WaveOscTables waveosctables;

void waveosctables_init(void)
{
	if (!waveosctables_initialized) {
		psy_audio_waveosctables_init(&waveosctables);
		waveosctables_initialized = 1;
	}
}

static int wavoscrefcounter = 0;

void psy_audio_waveosc_init(psy_audio_WaveOsc* self, psy_audio_WaveShape shape)
{
	psy_audio_Instrument* table;

	waveosctables_init();
	table = psy_audio_waveosctables_table(&waveosctables, shape);
	if (table) {
		psy_audio_PatternEvent ev;

		psy_audio_samplervoice_init(&self->voice,
			0,
			&waveosctables.container, 
			table,
			1, 44100, 0, // uintptr_t channel, unsigned int samplerate, int resamplingmethod,
			0xFF);

		patternevent_clear(&ev);
		ev.note = 60;
		psy_audio_samplervoice_noteon(&self->voice, &ev, 0);
		self->playing = 1;
	}
	self->frequency = 0;
	self->basefrequency = 0;
	++wavoscrefcounter;
}

void psy_audio_waveosc_dispose(psy_audio_WaveOsc* self)
{
	psy_audio_samplervoice_dispose(&self->voice);
	--wavoscrefcounter;
	if (wavoscrefcounter == 0) {
		psy_audio_waveosctables_dispose(&waveosctables);
		waveosctables_initialized = 0;
	}
}

void psy_audio_waveosc_work(psy_audio_WaveOsc* self, int num, float* data)
{
	if (self->playing) {
		psy_audio_Buffer output;

		psy_audio_buffer_init(&output, 1);
		output.samples[0] = data;
		psy_audio_samplervoice_work(&self->voice, &output, num);
		psy_audio_buffer_dispose(&output);
	}
}

void psy_audio_waveosc_setfrequency(psy_audio_WaveOsc* self, float f)
{
	self->basefrequency = f;
}

void psy_audio_waveosc_start(psy_audio_WaveOsc* self, double phase)
{
}

void psy_audio_waveosc_stop(psy_audio_WaveOsc* self, double phase)
{
}

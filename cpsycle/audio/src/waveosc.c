// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "waveosc.h"

#include "../../detail/portable.h"

static psy_audio_Sample* psy_audio_waveosc_sample(psy_audio_WaveOsc*, double frequency);

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

void psy_audio_waveosc_init(psy_audio_WaveOsc* self, psy_audio_WaveShape shape, int frequency)
{
	waveosctables_init();
	++wavoscrefcounter;
	self->shape = shape;
	self->frequency = frequency;
	self->basefrequency = 261.6255653005986346778499935233;
	self->gain = 1.0;
	self->playing = FALSE;
	self->fm = NULL;
	self->am = NULL;
	self->pm = NULL;
	self->phase = 0;
	psy_audio_sampleiterator_init(&self->sampleiterator,
		psy_audio_waveosc_sample(self, frequency), psy_dsp_RESAMPLERQUALITY_LINEAR);
	psy_audio_sampleiterator_setspeed(&self->sampleiterator, frequency / self->basefrequency);
	psy_audio_sampleiterator_play(&self->sampleiterator);
}

void psy_audio_waveosc_dispose(psy_audio_WaveOsc* self)
{
	psy_audio_sampleiterator_dispose(&self->sampleiterator);
	--wavoscrefcounter;
	if (wavoscrefcounter == 0) {
		psy_audio_waveosctables_dispose(&waveosctables);
		waveosctables_initialized = 0;
	}
}

psy_audio_Sample* psy_audio_waveosc_sample(psy_audio_WaveOsc* self, double frequency)
{
	psy_audio_Instrument* wavetable;
	psy_List* entries;
	psy_audio_Sample* rv = NULL;

	wavetable = psy_audio_waveosctables_table(&waveosctables, self->shape);
	if (wavetable) {
		entries = psy_audio_instrument_entriesintersect(wavetable, 0, 0, frequency);
		if (entries) {
			psy_audio_InstrumentEntry* entry;

			entry = psy_list_entry(entries);
			rv = psy_audio_samples_at(&waveosctables.container, entry->sampleindex);
		}
	}
	return rv;
}

void psy_audio_waveosc_work(psy_audio_WaveOsc* self, int amount, float* data)
{
	if (self->playing && self->sampleiterator.sample) {
		psy_audio_Buffer output;
		uintptr_t numsamples;
		uintptr_t dstpos;
		intptr_t nextsamples;		

		psy_audio_buffer_init(&output, 1);
		output.samples[0] = data;
		numsamples = amount;
		dstpos = 0;
		nextsamples = numsamples;
		while (numsamples) {
			intptr_t nextsamples;
			
			nextsamples = psy_min(psy_audio_sampleiterator_prework(
				&self->sampleiterator, 1, FALSE), 1);
			numsamples -= nextsamples;
			while (nextsamples) {
				intptr_t diff;

				psy_dsp_amp_t output;

				if (self->fm) {					
					double f;
					
					f = (self->frequency + *(self->fm++));
					if (f >= 0) {
						double speed;

						speed = f / self->basefrequency;
						psy_audio_sampleiterator_setspeed(&self->sampleiterator,
							speed);
					}
				}
				output = psy_audio_sampleiterator_work(&self->sampleiterator, 0);
				if (self->am) {
					output *= self->am[dstpos];
				}
				
				data[dstpos] += (float)(output * self->gain);
				nextsamples--;
				++dstpos;				
				diff = psy_audio_sampleiterator_inc(&self->sampleiterator);				
				self->sampleiterator.m_pL += diff;
				self->sampleiterator.m_pR += diff;				
				
			}
			psy_audio_sampleiterator_postwork(&self->sampleiterator);
		}	
		psy_audio_buffer_dispose(&output);
	}
}

void psy_audio_waveosc_setfrequency(psy_audio_WaveOsc* self, double frequency)
{	
	self->frequency = frequency;	
	psy_audio_sampleiterator_setsample(&self->sampleiterator,
		psy_audio_waveosc_sample(self, frequency));
	psy_audio_sampleiterator_setspeed(&self->sampleiterator,
		self->frequency / self->basefrequency);	
}

void psy_audio_waveosc_start(psy_audio_WaveOsc* self, double phase)
{
	if (self->sampleiterator.sample) {
		psy_audio_sampleiterator_setposition(&self->sampleiterator,
			(uintptr_t)(phase * self->sampleiterator.sample->numframes));
		self->playing = TRUE;
	}
}

void psy_audio_waveosc_stop(psy_audio_WaveOsc* self, double phase)
{
	self->playing = FALSE;
}

void psy_audio_waveosc_setquality(psy_audio_WaveOsc* self,
	psy_dsp_ResamplerQuality quality)
{
	self->quality = quality;
	psy_audio_sampleiterator_setquality(&self->sampleiterator, quality);
}

psy_dsp_ResamplerQuality  psy_audio_waveosc_quality(psy_audio_WaveOsc* self)
{
	return self->quality;
}

void psy_audio_waveosc_setshape(psy_audio_WaveOsc* self,
	psy_audio_WaveShape shape)
{
	self->shape = shape;
	psy_audio_sampleiterator_setsample(&self->sampleiterator,
		psy_audio_waveosc_sample(self, self->basefrequency));
}

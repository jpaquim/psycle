// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "samplervoice.h"

// audio
#include "sampler.h"
#include "samples.h"
#include "songio.h"
#include "constants.h"
// dsp
#include <noteperiods.h>
#include <linear.h>
// std
#include <assert.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>

#include "../../detail/portable.h"
#include "../../detail/trace.h"

static const int m_FineSineData[256] = {
	0,  2,  3,  5,  6,  8,  9, 11, 12, 14, 16, 17, 19, 20, 22, 23,
	24, 26, 27, 29, 30, 32, 33, 34, 36, 37, 38, 39, 41, 42, 43, 44,
	45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 56, 57, 58, 59,
	59, 60, 60, 61, 61, 62, 62, 62, 63, 63, 63, 64, 64, 64, 64, 64,
	64, 64, 64, 64, 64, 64, 63, 63, 63, 62, 62, 62, 61, 61, 60, 60,
	59, 59, 58, 57, 56, 56, 55, 54, 53, 52, 51, 50, 49, 48, 47, 46,
	45, 44, 43, 42, 41, 39, 38, 37, 36, 34, 33, 32, 30, 29, 27, 26,
	24, 23, 22, 20, 19, 17, 16, 14, 12, 11,  9,  8,  6,  5,  3,  2,
	0, -2, -3, -5, -6, -8, -9,-11,-12,-14,-16,-17,-19,-20,-22,-23,
	-24,-26,-27,-29,-30,-32,-33,-34,-36,-37,-38,-39,-41,-42,-43,-44,
	-45,-46,-47,-48,-49,-50,-51,-52,-53,-54,-55,-56,-56,-57,-58,-59,
	-59,-60,-60,-61,-61,-62,-62,-62,-63,-63,-63,-64,-64,-64,-64,-64,
	-64,-64,-64,-64,-64,-64,-63,-63,-63,-62,-62,-62,-61,-61,-60,-60,
	-59,-59,-58,-57,-56,-56,-55,-54,-53,-52,-51,-50,-49,-48,-47,-46,
	-45,-44,-43,-42,-41,-39,-38,-37,-36,-34,-33,-32,-30,-29,-27,-26,
	-24,-23,-22,-20,-19,-17,-16,-14,-12,-11, -9, -8, -6, -5, -3, -2
};

static const int m_FineRampDownData[256] = {
	64, 63, 63, 62, 62, 61, 61, 60, 60, 59, 59, 58, 58, 57, 57, 56,
	56, 55, 55, 54, 54, 53, 53, 52, 52, 51, 51, 50, 50, 49, 49, 48,
	48, 47, 47, 46, 46, 45, 45, 44, 44, 43, 43, 42, 42, 41, 41, 40,
	40, 39, 39, 38, 38, 37, 37, 36, 36, 35, 35, 34, 34, 33, 33, 32,
	32, 31, 31, 30, 30, 29, 29, 28, 28, 27, 27, 26, 26, 25, 25, 24,
	24, 23, 23, 22, 22, 21, 21, 20, 20, 19, 19, 18, 18, 17, 17, 16,
	16, 15, 15, 14, 14, 13, 13, 12, 12, 11, 11, 10, 10,  9,  9,  8,
	8,  7,  7,  6,  6,  5,  5,  4,  4,  3,  3,  2,  2,  1,  1,  0,
	0, -1, -1, -2, -2, -3, -3, -4, -4, -5, -5, -6, -6, -7, -7, -8,
	-8, -9, -9,-10,-10,-11,-11,-12,-12,-13,-13,-14,-14,-15,-15,-16,
	-16,-17,-17,-18,-18,-19,-19,-20,-20,-21,-21,-22,-22,-23,-23,-24,
	-24,-25,-25,-26,-26,-27,-27,-28,-28,-29,-29,-30,-30,-31,-31,-32,
	-32,-33,-33,-34,-34,-35,-35,-36,-36,-37,-37,-38,-38,-39,-39,-40,
	-40,-41,-41,-42,-42,-43,-43,-44,-44,-45,-45,-46,-46,-47,-47,-48,
	-48,-49,-49,-50,-50,-51,-51,-52,-52,-53,-53,-54,-54,-55,-55,-56,
	-56,-57,-57,-58,-58,-59,-59,-60,-60,-61,-61,-62,-62,-63,-63,-64
};

static const int m_FineSquareTable[256] = {
	64 ,64 ,64 ,64 ,64 ,64 ,64 ,64 ,64 ,64 ,64 ,64 ,64 ,64 ,64 ,64 ,
	64 ,64 ,64 ,64 ,64 ,64 ,64 ,64 ,64 ,64 ,64 ,64 ,64 ,64 ,64 ,64 ,
	64 ,64 ,64 ,64 ,64 ,64 ,64 ,64 ,64 ,64 ,64 ,64 ,64 ,64 ,64 ,64 ,
	64 ,64 ,64 ,64 ,64 ,64 ,64 ,64 ,64 ,64 ,64 ,64 ,64 ,64 ,64 ,64 ,
	64 ,64 ,64 ,64 ,64 ,64 ,64 ,64 ,64 ,64 ,64 ,64 ,64 ,64 ,64 ,64 ,
	64 ,64 ,64 ,64 ,64 ,64 ,64 ,64 ,64 ,64 ,64 ,64 ,64 ,64 ,64 ,64 ,
	64 ,64 ,64 ,64 ,64 ,64 ,64 ,64 ,64 ,64 ,64 ,64 ,64 ,64 ,64 ,64 ,
	64 ,64 ,64 ,64 ,64 ,64 ,64 ,64 ,64 ,64 ,64 ,64 ,64 ,64 ,64 ,64 ,
	-64 ,-64 ,-64 ,-64 ,-64 ,-64 ,-64 ,-64 ,-64 ,-64 ,-64 ,-64 ,-64 ,-64 ,-64 ,-64 ,
	-64 ,-64 ,-64 ,-64 ,-64 ,-64 ,-64 ,-64 ,-64 ,-64 ,-64 ,-64 ,-64 ,-64 ,-64 ,-64 ,
	-64 ,-64 ,-64 ,-64 ,-64 ,-64 ,-64 ,-64 ,-64 ,-64 ,-64 ,-64 ,-64 ,-64 ,-64 ,-64 ,
	-64 ,-64 ,-64 ,-64 ,-64 ,-64 ,-64 ,-64 ,-64 ,-64 ,-64 ,-64 ,-64 ,-64 ,-64 ,-64 ,
	-64 ,-64 ,-64 ,-64 ,-64 ,-64 ,-64 ,-64 ,-64 ,-64 ,-64 ,-64 ,-64 ,-64 ,-64 ,-64 ,
	-64 ,-64 ,-64 ,-64 ,-64 ,-64 ,-64 ,-64 ,-64 ,-64 ,-64 ,-64 ,-64 ,-64 ,-64 ,-64 ,
	-64 ,-64 ,-64 ,-64 ,-64 ,-64 ,-64 ,-64 ,-64 ,-64 ,-64 ,-64 ,-64 ,-64 ,-64 ,-64 ,
	-64 ,-64 ,-64 ,-64 ,-64 ,-64 ,-64 ,-64 ,-64 ,-64 ,-64 ,-64 ,-64 ,-64 ,-64 ,-64
};

// Random wave table (randomly choosen values. Not official)
static const int m_RandomTable[256] = {
	 48,-64,-21, 45, 51, 20,-32,-57, 62, 13,-35,-43,-33,-16, -8,-48,
	  8, 36, 52, -3, 58,-34,-31,-20,  5,-30, 32, 54, -9,-19, -6,-38,
	-11, 43, 10,-47,  2, 53, 11,-56,  3, 55,  9,-44,-15,  4,-63, 59,
	 21,-17, 44, -2,-25,-36, 12,-14, 56, 61, 42,-50,-46, 49,-27,-45,
	 30, 63,-28, 29, 33, 22,-41, 57, 47, 19,-51,-54,-42,-22, -7,-61,
	 14, 25, 34, -4, 40,-49,-40,-26,  7,-39, 24, 37, -10,-24, -5,-53,
	 -12, 27, 16,-59,  0, 35, 17, 50,  1, 38, 15,-55,-18,  6, 60, 41,
	 23,-23, 28, -1,-29,-52, 18,-13, 39, 46, 26,-62,-58, 31,-37,-59,
	 30, 63,-28, 29, 33, 22,-41, 57, 47, 19,-51,-54,-42,-22, -7,-61,
	 21,-17, 44, -2,-25,-36, 12,-14, 56, 61, 42,-50,-46, 49,-27,-45,
	 14, 25, 34, -4, 40,-49,-40,-26,  7,-39, 24, 37, -10,-24, -5,-53,
	 -11, 43, 10,-47,  2, 53, 11,-56,  3, 55,  9,-44,-15,  4,-63, 59,
	 -12, 27, 16,-59,  0, 35, 17, 50,  1, 38, 15,-55,-18,  6, 60, 41,
	 8, 36, 52, -3, 58,-34,-31,-20,  5,-30, 32, 54, -9,-19, -6,-38,
	 23,-23, 28, -1,-29,-52, 18,-13, 39, 46, 26,-62,-58, 31,-37,-59,
	 48,-64,-21, 45, 51, 20,-32,-57, 62, 13,-35,-43,-33,-16, -8,-48,
};

static int psy_audio_sampler_getdelta(psy_audio_WaveForms wavetype, int wavepos)
{
	switch (wavetype)
	{
	case psy_audio_WAVEFORMS_SAWDOWN:
		return m_FineRampDownData[wavepos];
	case psy_audio_WAVEFORMS_SAWUP:
		return m_FineRampDownData[0xFF - wavepos];
	case psy_audio_WAVEFORMS_SQUARE:
		return m_FineSquareTable[wavepos];
	case psy_audio_WAVEFORMS_RANDOM:
		return m_RandomTable[wavepos];
	case psy_audio_WAVEFORMS_SINUS:
	default:
		return m_FineSineData[wavepos];
	}
}

// psy_audio_SamplerVoice
static int alteRand(int x) { return (x * rand()) / 32768; }
static void psy_audio_samplervoice_updatespeed(psy_audio_SamplerVoice*);
static void psy_audio_samplervoice_updateiteratorspeed(psy_audio_SamplerVoice*,
	psy_audio_SampleIterator*);
static void psy_audio_samplervoice_initfilter(psy_audio_SamplerVoice*,
	psy_audio_Instrument* instrument);
static psy_dsp_amp_t psy_audio_samplervoice_workfilter(psy_audio_SamplerVoice*,
	uintptr_t channel, psy_dsp_amp_t input, psy_dsp_amp_t* filterenv, uintptr_t pos);
static void psy_audio_samplervoice_currvolume(psy_audio_SamplerVoice*,
	psy_audio_Sample*, psy_dsp_amp_t* svol, psy_dsp_amp_t* lvol,
	psy_dsp_amp_t* rvol);
psy_dsp_amp_t psy_audio_samplervoice_unprocessed_wavedata(psy_audio_SamplerVoice*,
	psy_audio_SampleIterator* it, uintptr_t channel);
static psy_dsp_amp_t psy_audio_samplervoice_processenvelopes(psy_audio_SamplerVoice*,
	uintptr_t channel, psy_dsp_amp_t input, uintptr_t pos,
	psy_dsp_amp_t* env, psy_dsp_amp_t* filterenv,
	psy_dsp_amp_t svol, psy_dsp_amp_t lvol, psy_dsp_amp_t rvol);
static void psy_audio_samplervoice_adddatatosamplerbuffer(psy_audio_SamplerVoice*,
	uintptr_t channel, psy_dsp_amp_t input, uintptr_t pos,
	psy_audio_Buffer* output);
static double psy_audio_samplervoice_vibratoamount(psy_audio_SamplerVoice* self)
{
	return self->vibratoamount;
}
// effects
static void psy_audio_samplervoice_addeffect(psy_audio_SamplerVoice*, int cmd);
static void psy_audio_samplervoice_setpanfactor(psy_audio_SamplerVoice*, float pan);
static void psy_audio_samplervoice_setpanningslide(psy_audio_SamplerVoice*,
	int speed);
static void psy_audio_samplervoice_performpanningslide(psy_audio_SamplerVoice*);
static void psy_audio_samplervoice_setvibrato(psy_audio_SamplerVoice*,
	int speed, int depth);
static void psy_audio_samplervoice_performvibrato(psy_audio_SamplerVoice*);
static void psy_audio_samplervoice_setvolumeslide(psy_audio_SamplerVoice*,
	int speed);
static void psy_audio_samplervoice_performvolumeslide(psy_audio_SamplerVoice*);
static void psy_audio_samplervoice_volumeup(psy_audio_SamplerVoice*,
	int value);
static void psy_audio_samplervoice_volumedown(psy_audio_SamplerVoice*,
	int value);
static void psy_audio_samplervoice_settremolo(psy_audio_SamplerVoice*,
	int speed, int depth);
static void psy_audio_samplervoice_performtremolo(psy_audio_SamplerVoice*);
static void psy_audio_samplervoice_setpanbrello(psy_audio_SamplerVoice*,
	int speed, int depth);
static void psy_audio_samplervoice_performpanbrello(psy_audio_SamplerVoice*);
static void psy_audio_samplervoice_settremor(psy_audio_SamplerVoice*,
	int parameter);
static void psy_audio_samplervoice_performtremor(psy_audio_SamplerVoice*);
static void psy_audio_samplervoice_setretrigger(psy_audio_SamplerVoice*);
static void psy_audio_samplervoice_performretrig(psy_audio_SamplerVoice*);

void psy_audio_samplervoice_init(psy_audio_SamplerVoice* self,
	psy_audio_Sampler* sampler,
	psy_audio_Samples* samples,
	psy_audio_Instrument* instrument,
	psy_audio_SamplerChannel* channel,
	uintptr_t channelnum,
	uintptr_t samplerate,
	int resamplingmethod,
	int maxvolume) 
{	
	filter_init_samplerate(&self->_filter, samplerate);
	self->sampler = sampler;
	self->samples = samples;
	self->instrument = instrument;
	self->channelnum = channelnum;
	self->channel = channel;
	self->usedefaultvolume = 1;
	self->vol = 1.f;
	self->positions = 0;
	self->effcmd = psy_audio_samplercmd_make(SAMPLER_CMD_NONE, 0, 0);
	self->effval = 0;
	self->dooffset = 0;
	self->maxvolume = maxvolume;
	self->positions = 0;
	self->stopping = FALSE;
	self->period = 0;
	if (instrument) {
		psy_dsp_adsr_init(&self->env, &instrument->volumeenvelope, samplerate);
		psy_dsp_adsr_init(&self->filterenv, &instrument->filterenvelope, samplerate);	
	} else {
		psy_dsp_adsr_initdefault(&self->env, samplerate);
		psy_dsp_adsr_initdefault(&self->filterenv, samplerate);
	}		
	psy_dsp_multiresampler_init(&self->resampler);
	psy_dsp_multiresampler_settype(&self->resampler,
		resamplingmethod);
	psy_audio_samplervoice_initfilter(self, instrument);
	self->effects = NULL;
	psy_audio_samplervoice_reseteffects(self);
}

void psy_audio_samplervoice_initfilter(psy_audio_SamplerVoice* self,
	psy_audio_Instrument* instrument)
{
	if (instrument) {
		filter_settype(&self->_filter, instrument->filtertype);
		filter_setressonance(&self->_filter,
			(instrument->_RRES)
			? alteRand((int)(instrument->filterres * 127))
			: (int)(instrument->filterres * 127));
		self->_cutoff = (instrument->_RCUT)
			? alteRand((int)(instrument->filtercutoff * 127))
			: (int)instrument->filtercutoff * 127;
		self->_coModify = (instrument->filtermodamount - 0.5f) * 255.f;
		filter_setcutoff(&self->_filter, self->_cutoff);
	}
}

void psy_audio_samplervoice_reset(psy_audio_SamplerVoice* self)
{
	self->effcmd = psy_audio_samplercmd_make(SAMPLER_CMD_NONE, 0, 0);
	self->stopping = FALSE;
	psy_dsp_adsr_reset(&self->env);
	psy_dsp_adsr_reset(&self->filterenv);
	psy_audio_samplervoice_reseteffects(self);	
}

void psy_audio_samplervoice_addeffect(psy_audio_SamplerVoice* self, int cmd)
{
	psy_list_append(&self->effects, (void*)(uintptr_t)cmd);
}

void psy_audio_samplervoice_dispose(psy_audio_SamplerVoice* self)
{
	psy_audio_samplervoice_clearpositions(self);
	self->positions = 0;
	psy_list_free(self->effects);
	self->effects = NULL;
}

psy_audio_SamplerVoice* psy_audio_samplervoice_alloc(void)
{
	return (psy_audio_SamplerVoice*) malloc(sizeof(psy_audio_SamplerVoice));
}

psy_audio_SamplerVoice* psy_audio_samplervoice_allocinit(psy_audio_Sampler* sampler,
	psy_audio_Instrument* instrument,
	psy_audio_SamplerChannel* channel,
	uintptr_t channelnum,
	uintptr_t samplerate)
{
	psy_audio_SamplerVoice* rv;

	rv = psy_audio_samplervoice_alloc();
	if (rv) {
		psy_audio_samplervoice_init(rv,
			sampler,
			psy_audio_machine_samples(psy_audio_sampler_base(sampler)),
			instrument,
			channel,
			channelnum,
			samplerate,
			sampler->resamplerquality,
			sampler->maxvolume);
	}
	return rv;
}

void psy_audio_samplervoice_seqtick(psy_audio_SamplerVoice* self,
	const psy_audio_PatternEvent* ev, double samplesprobeat)
{
	psy_audio_samplervoice_seteffect(self, ev);
	if (ev->note < NOTECOMMANDS_RELEASE) {		
		psy_audio_samplervoice_noteon(self, ev->note, samplesprobeat);
	}
}


void psy_audio_samplervoice_noteon(psy_audio_SamplerVoice* self,
	int note, double samplesprobeat)
{	
	psy_audio_Sample* sample;
	psy_List* entries;
	psy_List* p;

	self->note = note;
	psy_audio_samplervoice_initfilter(self, self->instrument);
	psy_audio_samplervoice_clearpositions(self);
	entries = psy_audio_instrument_entriesintersect(self->instrument,
		note, 127, 0);
	for (p = entries; p != NULL; psy_list_next(&p)) {
		psy_audio_InstrumentEntry* entry;
		
		entry = (psy_audio_InstrumentEntry*) p->entry;
		sample = psy_audio_samples_at(self->samples, entry->sampleindex);
		if (sample) {
			psy_audio_SampleIterator* iterator;			
			
			iterator = psy_audio_sampleiterator_alloc();
			*iterator = psy_audio_sample_begin(sample);
			iterator->resampler_data =
				psy_dsp_multiresampler_base(&self->resampler)->vtable->getresamplerdata(
					psy_dsp_multiresampler_base(&self->resampler));
			psy_list_append(&self->positions, iterator);
			if (self->instrument->loop && self->sampler) {
				psy_dsp_big_beat_t bpl;
				double totalsamples;
				
				bpl = psy_audio_machine_currbeatsperline(
					psy_audio_sampler_base(self->sampler));		
				totalsamples = samplesprobeat * bpl * self->instrument->lines;
				psy_audio_sampleiterator_setspeed(iterator,
					sample->numframes / (double)totalsamples);
			} else {
				if (self->sampler->amigaslides) {
					self->period = (int)psy_dsp_notetoamigaperiod(note,
						sample->samplerate,
						sample->tune +
						NOTECOMMANDS_MIDDLEC - self->sampler->basec,
						sample->finetune);
				} else {
					self->period = (int)psy_dsp_notetoperiod(note, sample->tune +
						NOTECOMMANDS_MIDDLEC - self->sampler->basec,
						sample->finetune);
				}
				psy_audio_samplervoice_updateiteratorspeed(self, iterator);
			}
			psy_audio_sampleiterator_play(iterator);
			psy_dsp_resampler_setspeed(psy_dsp_multiresampler_base(
				&self->resampler),
				iterator->resampler_data,
				iterator->speed * 1/ 4294967296.0f);
		}
	}	
	psy_list_free(entries);	
	if (self->positions) {		
		psy_dsp_adsr_start(&self->env);		
		psy_dsp_adsr_start(&self->filterenv);
	}
	// if (!self->dopan && self->instrument->randompan) {
	//	self->dopan = 1; 
	//	self->pan = rand() / (psy_dsp_amp_t) 32768.f;
	//}	
}

void psy_audio_samplervoice_updatespeed(psy_audio_SamplerVoice* self)
{
	if (self->positions && self->env.stage != ENV_OFF) {
		psy_List* p;

		for (p = self->positions; p != NULL; psy_list_next(&p)) {
			psy_audio_SampleIterator* it;

			it = (psy_audio_SampleIterator*)p->entry;
			psy_audio_samplervoice_updateiteratorspeed(self, it);
		}
	}
}

void psy_audio_samplervoice_updateiteratorspeed(psy_audio_SamplerVoice* self,
	psy_audio_SampleIterator* it)
{
	double period;
	double speed;
	
	period = self->period;
	period += psy_audio_samplervoice_vibratoamount(self);
	speed = self->sampler->amigaslides
		? psy_dsp_amigaperiodtospeed((int)period,
			psy_audio_machine_samplerate(psy_audio_sampler_base(self->sampler)),
			0)
		: psy_dsp_periodtospeed((int)period,
			psy_audio_machine_samplerate(psy_audio_sampler_base(self->sampler)),
			it->sample->samplerate, 0);
	//\todo: Attention, AutoVibrato always use linear slides with IT, but in
	// FT2 it depends on amigaslides switch.	
	// speed *= pow(2.0, ((-AutoVibratoAmount()) / 768.0));	
	psy_audio_sampleiterator_setspeed(it, speed);
}

void psy_audio_samplervoice_noteon_frequency(psy_audio_SamplerVoice* self,
	double frequency)
{
	psy_audio_Sample* sample;
	psy_List* entries;
	psy_List* p;

	psy_audio_samplervoice_clearpositions(self);
	entries = psy_audio_instrument_entriesintersect(self->instrument,
		0, 0, frequency);
	for (p = entries; p != NULL; psy_list_next(&p)) {
		psy_audio_InstrumentEntry* entry;

		entry = (psy_audio_InstrumentEntry*)p->entry;
		sample = psy_audio_samples_at(self->samples, entry->sampleindex);
		if (sample) {
			psy_audio_SampleIterator* iterator;

			iterator = psy_audio_sampleiterator_alloc();
			*iterator = psy_audio_sample_begin(sample);
			iterator->resampler_data =
				psy_dsp_multiresampler_base(&self->resampler)->vtable->getresamplerdata(
					psy_dsp_multiresampler_base(&self->resampler));
			psy_list_append(&self->positions, iterator);
			psy_audio_sampleiterator_setspeed(iterator, frequency / 440);
			psy_audio_sampleiterator_play(iterator);
			psy_dsp_resampler_setspeed(psy_dsp_multiresampler_base(
				&self->resampler),
				iterator->resampler_data,
				iterator->speed * 1 / 4294967296.0f);
		}
	}
	psy_list_free(entries);
	if (self->positions) {
		psy_dsp_adsr_start(&self->env);
		psy_dsp_adsr_start(&self->filterenv);
	}	
}

void psy_audio_samplervoice_clearpositions(psy_audio_SamplerVoice* self)
{
	psy_List* p;

	for (p = self->positions; p != NULL; psy_list_next(&p)) {
		psy_audio_SampleIterator* iterator;

		iterator = (psy_audio_SampleIterator*)p->entry;
		psy_dsp_multiresampler_base(&self->resampler)->vtable->disposeresamplerdata(
			psy_dsp_multiresampler_base(&self->resampler),
			iterator->resampler_data);
		psy_audio_sampleiterator_dispose(iterator);
		free(iterator);
	}
	psy_list_free(self->positions);
	self->positions = 0;
}

void psy_audio_samplervoice_nna(psy_audio_SamplerVoice* self)
{
	if (self->instrument) {
		switch (psy_audio_instrument_nna(self->instrument)) {
			case psy_audio_NNA_STOP:
				psy_audio_samplervoice_fastnoteoff(self);
			break;
			case psy_audio_NNA_NOTEOFF:
				psy_audio_samplervoice_noteoff(self);
			break;
			case psy_audio_NNA_CONTINUE:				
			break;
			default:
				// note cut
				psy_audio_samplervoice_fastnoteoff(self);
			break;
		}
	}
}

void psy_audio_samplervoice_noteoff(psy_audio_SamplerVoice* self)
{
	psy_dsp_adsr_release(&self->env);
	psy_dsp_adsr_release(&self->filterenv);
	self->stopping = TRUE;
}

void psy_audio_samplervoice_fastnoteoff(psy_audio_SamplerVoice* self)
{
	psy_dsp_adsr_fastrelease(&self->env);
	psy_dsp_adsr_fastrelease(&self->filterenv);
	self->stopping = TRUE;
}

void psy_audio_samplervoice_work(psy_audio_SamplerVoice* self,
	psy_audio_Buffer* output, uintptr_t amount)
{
	if (self->positions && self->env.stage != ENV_OFF) {
		psy_List* p;
		psy_dsp_amp_t* env;
		psy_dsp_amp_t* filterenv;
		uintptr_t i;		
		
		env = malloc(amount * sizeof(psy_dsp_amp_t));
		for (i = 0; i < amount; ++i) {
			psy_dsp_adsr_tick(&self->env);
			env[i] = self->env.value;
		}
		filterenv = NULL;
		if (filter_type(&self->_filter) != F_NONE) {
			filterenv = malloc(amount * sizeof(psy_dsp_amp_t));
			for (i = 0; i < amount; ++i) {
				psy_dsp_adsr_tick(&self->filterenv);
				filterenv[i] = self->filterenv.value;
			}
		}

		for (p = self->positions; p != NULL; psy_list_next(&p)) {
			psy_audio_SampleIterator* position;
			psy_dsp_amp_t svol;
			psy_dsp_amp_t rvol;
			psy_dsp_amp_t lvol;
			uintptr_t dstpos;
			uintptr_t numsamples;
			
			position = (psy_audio_SampleIterator*)p->entry;
			if (self->dooffset) {
				psy_audio_sampleiterator_dooffset(position, self->offset);
			}			
			psy_audio_samplervoice_currvolume(self, position->sample, &svol,
				&lvol, &rvol);
			dstpos = 0;
			numsamples = amount;
			while (numsamples) {
				uintptr_t channel;
				intptr_t diff;

				intptr_t nextsamples = min(psy_audio_sampleiterator_prework(position,
					numsamples, FALSE), numsamples);
				numsamples -= nextsamples;
				while (nextsamples)
				{
					for (channel = 0; channel < psy_audio_buffer_numchannels(&position->sample->channels) &&
							channel < psy_audio_buffer_numchannels(output); ++channel) {
						psy_dsp_amp_t val;

						val = psy_audio_samplervoice_unprocessed_wavedata(self,
							position, channel);
						val = psy_audio_samplervoice_processenvelopes(self,
							channel, val, dstpos, env, filterenv, svol, lvol, rvol);
						psy_audio_samplervoice_adddatatosamplerbuffer(self, channel,
							val, dstpos, output);
					}					
					++dstpos;
					nextsamples--;
					diff = psy_audio_sampleiterator_inc(position);
					position->m_pL += diff;
					if (psy_audio_buffer_numchannels(&position->sample->channels) > 1) {
						position->m_pR += diff;
					}
				}
				psy_audio_sampleiterator_postwork(position);
				if (!psy_audio_sampleiterator_playing(position)) {
					psy_audio_samplervoice_reset(self);
					break;
				}
			}
			if (psy_audio_buffer_mono(&position->sample->channels)) {
				psy_audio_buffer_make_monoaureal(output, amount);				
			}			
		}		
		free(env);
		free(filterenv);
	}
	self->dooffset = 0;
}

psy_dsp_amp_t psy_audio_samplervoice_unprocessed_wavedata(psy_audio_SamplerVoice* self,
	psy_audio_SampleIterator* it, uintptr_t channel)
{
	psy_dsp_amp_t* src;

	src = psy_audio_buffer_at(&it->sample->channels, channel);	
	return psy_dsp_resampler_work_float_unchecked(
		psy_dsp_multiresampler_base(&self->resampler),
		(channel == 0) ? it->m_pL : it->m_pR,
		it->pos.LowPart,
		it->resampler_data);
}

void psy_audio_samplervoice_setresamplerquality(psy_audio_SamplerVoice* self, 
	ResamplerType quality)
{
	psy_dsp_multiresampler_settype(&self->resampler, quality);
}

psy_dsp_amp_t psy_audio_samplervoice_processenvelopes(psy_audio_SamplerVoice* self,
	uintptr_t channel, psy_dsp_amp_t input, uintptr_t pos,
	psy_dsp_amp_t* env, psy_dsp_amp_t* filterenv,
	psy_dsp_amp_t svol, psy_dsp_amp_t lvol, psy_dsp_amp_t rvol)
{
	psy_dsp_amp_t rv;
	psy_dsp_amp_t volume;

	volume = env[pos] * self->instrument->globalvolume;
	if (psy_audio_sampler_usefilters(self->sampler)) {
		rv = psy_audio_samplervoice_workfilter(self, channel, input, filterenv, pos);
		if (filterenv) {
			volume *= filterenv[pos];
		}
	} else {
		rv = input;
	}
	if (channel == 0) {
		volume *= lvol;
	} else if (channel == 1) {
		volume *= rvol;
	} else {
		volume *= svol;
	}	
	rv *= volume;
	return rv;
}

void psy_audio_samplervoice_adddatatosamplerbuffer(psy_audio_SamplerVoice* self,
	uintptr_t channel, psy_dsp_amp_t input, uintptr_t pos,
	psy_audio_Buffer* output)
{
	psy_dsp_amp_t* dst;

	dst = psy_audio_buffer_at(output, channel);
	if (dst) {
		dst[pos] += input;
	}
}

void psy_audio_samplervoice_currvolume(psy_audio_SamplerVoice* self,
	psy_audio_Sample* sample, psy_dsp_amp_t* svol, psy_dsp_amp_t* lvol,
	psy_dsp_amp_t* rvol)
{
	psy_dsp_amp_t cvol;

	*svol = psy_audio_samplervoice_volume(self, sample) *
		(self->usedefaultvolume || self->effcmd.id == SAMPLER_CMD_VOLUMESLIDE)
			? sample->defaultvolume 
			: self->vol;
	*svol *= sample->globalvolume;
	cvol = self->channel ? self->channel->volume : (psy_dsp_amp_t)1.f;	
	*rvol = self->panfactor * (*svol) * cvol + self->panbrelloamount;
	// linear pan mode
	*lvol = ((psy_dsp_amp_t)1.f - self->panfactor) * *svol * cvol;
	// FT2 Style (Two slides) mode, but with max amp = 0.5.
	if (*rvol > self->sampler->clipmax) {
		*rvol = (psy_dsp_amp_t)self->sampler->clipmax;
	}
	if (*lvol > self->sampler->clipmax) {
		*lvol = (psy_dsp_amp_t)self->sampler->clipmax;
	}
}

psy_dsp_amp_t psy_audio_samplervoice_workfilter(psy_audio_SamplerVoice* self,
	uintptr_t channel, psy_dsp_amp_t input, psy_dsp_amp_t* filterenv, uintptr_t pos)
{		
	if (filter_type(&self->_filter) != F_NONE)
	{
		int newcutoff = (int)(self->_cutoff + (filterenv[pos] * self->_coModify + 0.5f));
		if (newcutoff < 0) {
			newcutoff = 0;
		} else if (newcutoff > 127) {
			newcutoff = 127;
		}
		filter_setcutoff(&self->_filter, newcutoff);		
		return input = self->_filter.vtable->work(&self->_filter, input);
	}
	return input;
}

void psy_audio_samplervoice_release(psy_audio_SamplerVoice* self)
{
	self->effcmd.id = SAMPLER_CMD_NONE;
	psy_dsp_adsr_release(&self->env);	
	psy_dsp_adsr_release(&self->filterenv);
}

void psy_audio_samplervoice_fastrelease(psy_audio_SamplerVoice* self)
{
	self->effcmd.id = SAMPLER_CMD_NONE;
	psy_dsp_adsr_fastrelease(&self->env);	
	psy_dsp_adsr_fastrelease(&self->filterenv);
}

// Voice Effects
void psy_audio_samplervoice_newline(psy_audio_SamplerVoice* self)
{
	self->effcmd.id = SAMPLER_CMD_NONE;
	psy_list_free(self->effects);
	self->effects = NULL;
}

psy_audio_samplervoice_seteffect(psy_audio_SamplerVoice* self,
	const psy_audio_PatternEvent* ev)
{
	psy_audio_SamplerCmd* cmd;

	cmd = psy_audio_sampler_cmd(self->sampler, ev->cmd);
	if (cmd != NULL) {
		switch (psy_audio_samplercmd_id(cmd)) {
			case SAMPLER_CMD_VOLUME:
				self->usedefaultvolume = 0;
				self->vol = ev->parameter /
					(psy_dsp_amp_t)self->maxvolume;

				break;
			case SAMPLER_CMD_PANNING:
				if (self->channel) {
					psy_audio_samplervoice_setpanfactor(self,
						self->channel->panfactor);
				}
				break;
			case SAMPLER_CMD_OFFSET:
				self->dooffset = 1;
				self->offset = ev->parameter;
				break;
			case SAMPLER_CMD_PORTAMENTO_UP: {
				self->effval = ev->parameter;
				self->effcmd = *cmd;
				break;
			}
			case  SAMPLER_CMD_PORTAMENTO_DOWN: {
				self->effval = ev->parameter;
				self->effcmd = *cmd;
				break;
			}
			case SAMPLER_CMD_PORTA2NOTE: {

			}
			case SAMPLER_CMD_VOLUMESLIDE:
				psy_audio_samplervoice_setvolumeslide(self,
					ev->parameter);
				break;
			case SAMPLER_CMD_PANNINGSLIDE:
				psy_audio_samplervoice_setpanningslide(self,
					ev->parameter);
				break;
			case SAMPLER_CMD_VIBRATO:
				psy_audio_samplervoice_setvibrato(self,
					((ev->parameter >> 4) & 0x0F),
					(ev->parameter & 0x0F) << 2);
				break;
			case SAMPLER_CMD_FINE_VIBRATO:
				psy_audio_samplervoice_setvibrato(self,
					((ev->parameter >> 4) & 0x0F),
					(ev->parameter & 0x0F));
				break;
			case SAMPLER_CMD_TREMOR:
				psy_audio_samplervoice_settremor(self,				
					ev->parameter);
				break;
			case SAMPLER_CMD_TREMOLO:
				psy_audio_samplervoice_settremolo(self,
					((ev->parameter >> 4) & 0x0F),
					(ev->parameter & 0x0F));
				break;
			case SAMPLER_CMD_PANBRELLO:
				psy_audio_samplervoice_setpanbrello(self,
					((ev->parameter >> 4) & 0x0F),
					(ev->parameter & 0x0F));
				break;
			case SAMPLER_CMD_RETRIG:
				psy_audio_samplervoice_setretrigger(self);
				break;
			default:
				break;
		}
	}
}

void psy_audio_samplervoice_performfx(psy_audio_SamplerVoice* self)
{	
	psy_List* p;

	for (p = self->effects; p != NULL; psy_list_next(&p)) {
		int effect = (int)psy_list_entry(p);

		switch (effect) {
			case SAMPLER_EFFECT_VOLUMESLIDE:
				psy_audio_samplervoice_performvolumeslide(self);
			break;
			case SAMPLER_EFFECT_PANSLIDE:
				psy_audio_samplervoice_performpanningslide(self);
			break;
			case SAMPLER_EFFECT_VIBRATO:
				psy_audio_samplervoice_performvibrato(self);
				break;
			case SAMPLER_EFFECT_TREMOR:
				psy_audio_samplervoice_performtremor(self);
				break;
			case SAMPLER_EFFECT_TREMOLO:
				psy_audio_samplervoice_performtremolo(self);
				break;
			case SAMPLER_EFFECT_PANBRELLO:
				psy_audio_samplervoice_performpanbrello(self);
				break;
			case SAMPLER_EFFECT_RETRIG:
				psy_audio_samplervoice_performretrig(self);
				break;
			default:
				break;
		}
	}
	switch (psy_audio_samplercmd_id(&self->effcmd)) {	
		case SAMPLER_CMD_PORTAMENTO_UP: {
			self->period -= self->effval;
			break;
		}
		case SAMPLER_CMD_PORTAMENTO_DOWN: {
			self->period += self->effval;
			break;
		}
		default:
			break;
	}
	psy_audio_samplervoice_updatespeed(self);
}

void psy_audio_samplervoice_effectinit(psy_audio_SamplerVoice* self)
{
}

void psy_audio_samplervoice_reseteffects(psy_audio_SamplerVoice* self)
{
	//m_Slide2NoteDestPeriod = 0;
	//m_PitchSlideSpeed = 0;

	// volume slide
	self->volumefadespeed = 0;
	self->volumefadeamount = 1;
	self->volumeslidespeed = 0;
	// pan
	self->panfactor = 0.5f;
	self->panrange = 0.5f;
	// vibrato
	self->vibratospeed = 0;
	self->vibratodepth = 0;
	self->vibratopos = 0;
	self->vibratoamount = 0.0;
	// tremelo
	self->tremolospeed = 0;
	self->tremolodepth = 0;
	self->tremoloamount = 0.0f;
	self->tremolopos = 0;
	// panbrello
	self->panbrellospeed = 0;
	self->panbrellodepth = 0;
	self->panbrelloamount = 0.0f;
	self->panbrellopos = 0;
	self->panbrellorandomcounter = 0;	
	// tremor
	self->tremoronticks = 0;
	self->tremoroffticks = 0;
	self->tremortickchange = 0;
	self->tremormute = FALSE;
	// autovibrato
	self->autovibratoamount = 0.0;
	self->autovibratopos = 0;
	self->autovibratodepth = 0;
	// retrigs
	self->retrigticks = 0;
	// clear effects
	psy_list_free(self->effects);
	self->effects = NULL;
}

// Effects
void psy_audio_samplervoice_setvolumeslide(psy_audio_SamplerVoice* self, int speed)
{
	if (ISSLIDEUP(speed)) { // Slide Up
		speed = GETSLIDEUPVAL(speed);

		psy_audio_samplervoice_addeffect(self, SAMPLER_EFFECT_VOLUMESLIDE);
		self->volumeslidespeed = speed << 1;
		if (speed == 0xF) {
			psy_audio_samplervoice_performvolumeslide(self);
		}
	} else if (ISSLIDEDOWN(speed)) { // Slide Down
		speed = GETSLIDEDOWNVAL(speed);
		psy_audio_samplervoice_addeffect(self, SAMPLER_EFFECT_VOLUMESLIDE);		
		self->volumeslidespeed = -(speed << 1);
		if (speed == 0xF) {
			psy_audio_samplervoice_performvolumeslide(self);
		}
	} else if (ISFINESLIDEUP(speed)) { // FineSlide Up		
		self->volumeslidespeed = GETSLIDEUPVAL(speed) << 1;
		psy_audio_samplervoice_performvolumeslide(self);
	} else if (ISFINESLIDEDOWN(speed)) { // FineSlide Down		
		self->volumeslidespeed = -(GETSLIDEDOWNVAL(speed) << 1);
		psy_audio_samplervoice_performvolumeslide(self);
	}
}

void psy_audio_samplervoice_performvolumeslide(psy_audio_SamplerVoice * self)
{
	if (self->volumeslidespeed > 0) {
		psy_audio_samplervoice_volumeup(self, self->volumeslidespeed);
	} else {
		psy_audio_samplervoice_volumedown(self, self->volumeslidespeed);
	}
}

void psy_audio_samplervoice_volumedown(psy_audio_SamplerVoice* self, int value)
{	
	int vol = (int)(self->vol * 128) + value;
	if (vol < 0) {
		vol = 0;
	}	
	self->vol = vol / 128.f;
}

void psy_audio_samplervoice_volumeup(psy_audio_SamplerVoice* self, int value)
{
	int vol = (int)(self->vol * 128) + value;
	if (vol > 0x80) {
		vol = 0x80;
	}
	self->vol = vol / 128.f;
}

void psy_audio_samplervoice_setpanningslide(psy_audio_SamplerVoice* self,
	int speed)
{	
	if (ISSLIDEUP(speed)) { // Slide Left
		speed = GETSLIDEUPVAL(speed);
		psy_audio_samplervoice_addeffect(self, SAMPLER_EFFECT_PANSLIDE);
		if (speed == 0xF) {
			psy_audio_samplervoice_performpanningslide(self);
		}
	} else if (ISSLIDEDOWN(speed)) { // Slide Right
		speed = GETSLIDEDOWNVAL(speed);
		psy_audio_samplervoice_addeffect(self, SAMPLER_EFFECT_PANSLIDE);
		if (speed == 0xF) {
			psy_audio_samplervoice_performpanningslide(self);
		}
	} else if (ISFINESLIDEUP(speed)) { // FineSlide left
		psy_audio_samplervoice_performpanningslide(self);
	} else if (ISFINESLIDEDOWN(speed)) { // FineSlide right
		psy_audio_samplervoice_performpanningslide(self);
	}
}

void psy_audio_samplervoice_performpanningslide(psy_audio_SamplerVoice* self)
{
	if (self->channel) {
		psy_audio_samplervoice_setpanfactor(self, self->channel->panfactor);
	}
}

void psy_audio_samplervoice_setpanfactor(psy_audio_SamplerVoice* self, float pan)
{
	self->panfactor = pan;
	self->panfactor = self->panrange =(float)( (0.5 - fabs((double)pan - 0.5)));
}

void psy_audio_samplervoice_setvibrato(psy_audio_SamplerVoice* self, int speed,
	int depth)
{
	self->vibratospeed = speed << 2;
	self->vibratodepth = depth;
	psy_audio_samplervoice_addeffect(self, SAMPLER_EFFECT_VIBRATO);
}

void psy_audio_samplervoice_performvibrato(psy_audio_SamplerVoice* self)
{
	if (self->channel) {
		int vdelta = psy_audio_sampler_getdelta(self->channel->vibratotype,
			self->vibratopos);

		vdelta = vdelta * self->vibratodepth;
		self->vibratoamount = (double)vdelta / 32.0;
		self->vibratopos = (self->vibratopos - self->vibratospeed) & 0xff;		
	}
}// Vibrato() -------------------------------------

void psy_audio_samplervoice_settremolo(psy_audio_SamplerVoice* self, int speed,
	int depth)
{
	self->tremolospeed = speed << 2;
	self->tremolodepth = depth;
	psy_audio_samplervoice_addeffect(self, SAMPLER_EFFECT_TREMOLO);
}

void psy_audio_samplervoice_performtremolo(psy_audio_SamplerVoice* self)
{
	//\todo: verify that final volume doesn't go out of range (Redo RealVolume() ?)
	int vdelta = psy_audio_sampler_getdelta(self->channel->tremolotype,
		self->tremolopos);
	vdelta = (vdelta * self->tremolodepth);
	self->tremoloamount = (float)((double)vdelta / 2048.0);
	self->tremolopos = (self->tremolopos + self->tremolospeed) & 0xFF;


}// Tremolo() -------------------------------------------

void psy_audio_samplervoice_setpanbrello(psy_audio_SamplerVoice* self,
	int speed, int depth)
{
	self->panbrellospeed = speed << 2;
	self->panbrellodepth = depth;
	psy_audio_samplervoice_addeffect(self, SAMPLER_EFFECT_PANBRELLO);
}

void psy_audio_samplervoice_performpanbrello(psy_audio_SamplerVoice* self)
{
	// Yxy   Panbrello with speed x, depth y.
	// The random pan position can be achieved by setting the
	// waveform to 3 (ie. a S53 command). In this case *ONLY*, the
	// speed actually is interpreted as a delay in frames before
	// another random value is found. so Y14 will be a very QUICK
	// panbrello, and Y44 will be a slower panbrello.

	//\todo: verify that final pan doesn't go out of range (make a RealPan()
	// similar to RealVolume() ?)
	int vdelta = psy_audio_sampler_getdelta(self->channel->panbrellotype,
		self->panbrellopos);

	vdelta = vdelta * self->panbrellodepth;
	self->panbrelloamount = vdelta / 2048.0f; // 64*16*2
	if (self->channel->panbrellotype != psy_audio_WAVEFORMS_RANDOM)
	{
		self->panbrellopos = (self->panbrellopos + self->panbrellospeed) & 0xFF;
	} else if (++self->panbrellorandomcounter >= self->panbrellospeed)
	{
		self->panbrellopos++;
		self->panbrellorandomcounter = 0;
	}
}// Panbrello() -------------------------------------------


void psy_audio_samplervoice_settremor(psy_audio_SamplerVoice* self, int parameter)
{
	self->tremoronticks = ((parameter >> 4) & 0xf) + 1;
	self->tremoroffticks = (parameter & 0xf) + 1;
	self->tremortickchange = self->tremoronticks;
	psy_audio_samplervoice_addeffect(self, SAMPLER_EFFECT_TREMOR);
}


void psy_audio_samplervoice_performtremor(psy_audio_SamplerVoice* self)
{
	//\todo: according to Impulse Tracker, this command uses its own counter,
	// so with speed 3, we can specify the command I41 ( x/y > speed), which,
	// with the current implementation, doesn't work,
	if (self->sampler->ticktimer.tickcount == self->tremortickchange)
	{
		if (self->tremormute)
		{
			self->tremortickchange = self->sampler->ticktimer.tickcount +
				self->tremoronticks;
			self->tremormute = FALSE;
		} else
		{
			self->tremortickchange = self->sampler->ticktimer.tickcount +
				self->tremoroffticks;
			self->tremormute = TRUE;
		}
	}
}

void psy_audio_samplervoice_setretrigger(psy_audio_SamplerVoice* self)
{
	if (self->channel) {
		self->retrigticks = (self->channel->ticks != 0)
			? self->channel->ticks
			: 1;
	}
}

void psy_audio_samplervoice_performretrig(psy_audio_SamplerVoice* self)
{
	if (self->sampler->ticktimer.tickcount % self->retrigticks == 0)
	{
		psy_audio_samplervoice_noteon(self, self->note,
			1.0 / psy_audio_machine_beatspersample(
				psy_audio_sampler_base(self->sampler)));
	}
}

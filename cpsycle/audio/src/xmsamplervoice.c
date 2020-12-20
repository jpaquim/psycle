// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "xmsamplervoice.h"

// audio
#include "xmsampler.h"
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

static int psy_audio_xmsampler_getdelta(psy_audio_WaveForms wavetype, int wavepos)
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

// psy_audio_XMSamplerVoice
static int alteRand(int x) { return (x * rand()) / 32768; }
static void psy_audio_xmsamplervoice_updatespeed(psy_audio_XMSamplerVoice*);
static void psy_audio_xmsamplervoice_updateiteratorspeed(psy_audio_XMSamplerVoice*,
	psy_audio_SampleIterator*);
static void psy_audio_xmsamplervoice_initfilter(psy_audio_XMSamplerVoice*,
	psy_audio_Instrument* instrument);
static psy_dsp_amp_t psy_audio_xmsamplervoice_workfilter(psy_audio_XMSamplerVoice*,
	uintptr_t channel, psy_dsp_amp_t input, psy_dsp_amp_t* filterenv, uintptr_t pos);
static void psy_audio_xmsamplervoice_currvolume(psy_audio_XMSamplerVoice*,
	psy_audio_Sample*, psy_dsp_amp_t* svol, psy_dsp_amp_t* lvol,
	psy_dsp_amp_t* rvol);
static void psy_audio_xmsamplervoice_adddatatosamplerbuffer(psy_audio_XMSamplerVoice*,
	uintptr_t channel, psy_dsp_amp_t input, uintptr_t pos,
	psy_audio_Buffer* output);
static double psy_audio_xmsamplervoice_vibratoamount(psy_audio_XMSamplerVoice* self)
{
	return self->vibratoamount;
}
// effects
static void psy_audio_xmsamplervoice_addeffect(psy_audio_XMSamplerVoice*, int cmd);
static void psy_audio_xmsamplervoice_setpanfactor(psy_audio_XMSamplerVoice*, float pan);
static void psy_audio_xmsamplervoice_setpanningslide(psy_audio_XMSamplerVoice*,
	int speed);
static void psy_audio_xmsamplervoice_performpanningslide(psy_audio_XMSamplerVoice*);
static void psy_audio_xmsamplervoice_setvibrato(psy_audio_XMSamplerVoice*,
	int speed, int depth);
static void psy_audio_xmsamplervoice_setpitchslide(psy_audio_XMSamplerVoice*,
	bool bUp, int speed, int note);
static void psy_audio_xmsamplervoice_performvibrato(psy_audio_XMSamplerVoice*);
static void psy_audio_xmsamplervoice_performpitchslide(psy_audio_XMSamplerVoice*);
static void psy_audio_xmsamplervoice_setvolumeslide(psy_audio_XMSamplerVoice*,
	int speed);
static void psy_audio_xmsamplervoice_performvolumeslide(psy_audio_XMSamplerVoice*);
static void psy_audio_xmsamplervoice_volumeup(psy_audio_XMSamplerVoice*,
	int value);
static void psy_audio_xmsamplervoice_volumedown(psy_audio_XMSamplerVoice*,
	int value);
static void psy_audio_xmsamplervoice_settremolo(psy_audio_XMSamplerVoice*,
	int speed, int depth);
static void psy_audio_xmsamplervoice_performtremolo(psy_audio_XMSamplerVoice*);
static void psy_audio_xmsamplervoice_setpanbrello(psy_audio_XMSamplerVoice*,
	int speed, int depth);
static void psy_audio_xmsamplervoice_performpanbrello(psy_audio_XMSamplerVoice*);
static void psy_audio_xmsamplervoice_settremor(psy_audio_XMSamplerVoice*,
	int parameter);
static void psy_audio_xmsamplervoice_performtremor(psy_audio_XMSamplerVoice*);
static void psy_audio_xmsamplervoice_setretrigger(psy_audio_XMSamplerVoice*);
static void psy_audio_xmsamplervoice_performretrig(psy_audio_XMSamplerVoice*);

// implementation
void psy_audio_xmsamplervoice_init(psy_audio_XMSamplerVoice* self,
	struct psy_audio_XMSampler* sampler,
	struct psy_audio_Samples* samples,
	struct psy_audio_Instrument* instrument,
	psy_audio_XMSamplerChannel* channel,
	uintptr_t channelnum,
	uintptr_t samplerate,
	psy_dsp_ResamplerQuality quality,
	int maxvolume) 
{		
	self->sampler = sampler;
	self->samples = samples;
	self->instrument = instrument;
	self->channelnum = channelnum;
	self->channel = channel;
	// Init Envelopes
	psy_dsp_envelope_init(&self->amplitudeenvelope);
	psy_dsp_envelope_init_adsr(&self->amplitudeenvelope);
	psy_dsp_envelope_init_adsr(&self->panenvelope);
	psy_dsp_envelope_init_adsr(&self->pitchenvelope);
	psy_dsp_envelope_init_adsr(&self->filterenvelope);
	psy_dsp_envelope_setsamplerate(&self->amplitudeenvelope, samplerate);
	psy_dsp_envelope_setsamplerate(&self->filterenvelope, samplerate);
	psy_dsp_envelope_setsamplerate(&self->panenvelope, samplerate);
	psy_dsp_envelope_setsamplerate(&self->pitchenvelope, samplerate);
	if (instrument) {
		psy_dsp_envelope_set_settings(&self->amplitudeenvelope, &instrument->volumeenvelope);
		psy_dsp_envelope_set_settings(&self->filterenvelope, &instrument->filterenvelope);		
	}	
	// Init Filter
	filter_init_samplerate(&self->_filter, samplerate);
	psy_audio_xmsamplervoice_initfilter(self, instrument);
	self->usedefaultvolume = 1;
	self->positions = 0;
	self->effcmd = XM_SAMPLER_CMD_NONE;
	self->effval = 0;
	self->dooffset = 0;
	self->maxvolume = maxvolume;
	self->positions = 0;
	self->play = FALSE;
	self->stopping = FALSE;
	self->period = 0;
	self->resamplertype = quality;
	self->currrandvol = 1.f;
	self->volume = 128;
	self->note = psy_audio_NOTECOMMANDS_EMPTY;
	self->_cutoff = 127;
	self->m_Ressonance = 0;
	self->_coModify = 0;
	self->play = FALSE;
	self->stopping = FALSE;
	self->panfactor = 0.5f;
	self->panrange = 0.5f;
	self->surround = FALSE;
	self->effects = NULL;
	// ps1
	self->effcmd = XM_SAMPLER_CMD_NONE, 0, 0;
	psy_audio_xmsamplervoice_reseteffects(self);
	psy_dsp_slider_init(&self->rampl);
	psy_dsp_slider_init(&self->rampr);
	psy_dsp_slider_resetto(&self->rampl, 0.f);
	psy_dsp_slider_resetto(&self->rampr, 0.f);		
}

void psy_audio_xmsamplervoice_initfilter(psy_audio_XMSamplerVoice* self,
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

void psy_audio_xmsamplervoice_addeffect(psy_audio_XMSamplerVoice* self, int cmd)
{
	psy_list_append(&self->effects, (void*)(uintptr_t)cmd);
}

void psy_audio_xmsamplervoice_dispose(psy_audio_XMSamplerVoice* self)
{
	psy_audio_xmsamplervoice_clearpositions(self);
	self->positions = 0;
	psy_list_free(self->effects);
	self->effects = NULL;
	psy_dsp_envelope_dispose(&self->amplitudeenvelope);
	psy_dsp_envelope_dispose(&self->filterenvelope);
	psy_dsp_envelope_dispose(&self->panenvelope);
	psy_dsp_envelope_dispose(&self->pitchenvelope);
}

psy_audio_XMSamplerVoice* psy_audio_xmsamplervoice_alloc(void)
{
	return (psy_audio_XMSamplerVoice*) malloc(sizeof(psy_audio_XMSamplerVoice));
}

psy_audio_XMSamplerVoice* psy_audio_xmsamplervoice_allocinit(psy_audio_XMSampler* sampler,
	psy_audio_Instrument* instrument,
	psy_audio_XMSamplerChannel* channel,
	uintptr_t channelnum,
	uintptr_t samplerate)
{
	psy_audio_XMSamplerVoice* rv;

	rv = psy_audio_xmsamplervoice_alloc();
	if (rv) {
		psy_audio_xmsamplervoice_init(rv,
			sampler,
			psy_audio_machine_samples(psy_audio_xmsampler_base(sampler)),
			instrument,
			channel,
			channelnum,
			samplerate,
			sampler->resamplerquality,
			sampler->maxvolume);
	}
	return rv;
}

void psy_audio_xmsamplervoice_seqtick(psy_audio_XMSamplerVoice* self,
	const psy_audio_PatternEvent* ev)
{
	psy_audio_xmsamplervoice_seteffect(self, ev);
	if (ev->note < psy_audio_NOTECOMMANDS_RELEASE) {		
		psy_audio_xmsamplervoice_noteon(self, ev->note);
	}
}


void psy_audio_xmsamplervoice_noteon(psy_audio_XMSamplerVoice* self,
	int note)
{	
	psy_audio_Sample* sample;
	psy_List* entries;
	psy_List* p;
	double samplesprobeat;

	self->note = note;
	samplesprobeat = 1.0 / psy_audio_machine_beatspersample(
		psy_audio_xmsampler_base(self->sampler));
	psy_audio_xmsamplervoice_initfilter(self, self->instrument);
	psy_audio_xmsamplervoice_clearpositions(self);
	entries = psy_audio_instrument_entriesintersect(self->instrument,
		note, 127, 0);
	for (p = entries; p != NULL; psy_list_next(&p)) {
		psy_audio_InstrumentEntry* entry;
		
		entry = (psy_audio_InstrumentEntry*) p->entry;
		sample = psy_audio_samples_at(self->samples, entry->sampleindex);
		if (sample) {
			psy_audio_SampleIterator* iterator;			
			
			iterator = psy_audio_sample_allociterator(sample, self->resamplertype);
			psy_list_append(&self->positions, iterator);
			if (self->instrument->loop && self->sampler) {
				psy_dsp_big_beat_t bpl;
				double totalsamples;
				
				bpl = psy_audio_machine_currbeatsperline(
					psy_audio_xmsampler_base(self->sampler));		
				totalsamples = samplesprobeat * bpl * self->instrument->lines;
				psy_audio_sampleiterator_setspeed(iterator,
					sample->numframes / (double)totalsamples);
			} else {
				if (self->sampler->amigaslides) {
					self->period = (int)psy_dsp_notetoamigaperiod(note,
						sample->samplerate,
						sample->tune +
						psy_audio_NOTECOMMANDS_MIDDLEC - self->sampler->basec,
						sample->finetune);
				} else {
					self->period = (int)psy_dsp_notetoperiod(note, sample->tune +
						psy_audio_NOTECOMMANDS_MIDDLEC - self->sampler->basec,
						sample->finetune);
				}
				psy_audio_xmsamplervoice_updateiteratorspeed(self, iterator);				
			}
			psy_audio_sampleiterator_play(iterator);			
		}
	}	
	psy_list_free(entries);	
	if (self->positions) {		
		psy_dsp_envelope_start(&self->amplitudeenvelope);
		psy_dsp_envelope_start(&self->filterenvelope);
	}
	psy_audio_xmsamplervoice_setisplaying(self,
		TRUE);
	// if (!self->dopan && self->instrument->randompan) {
	//	self->dopan = 1; 
	//	self->pan = rand() / (psy_dsp_amp_t) 32768.f;
	//}	
}

void psy_audio_xmsamplervoice_updatespeed(psy_audio_XMSamplerVoice* self)
{
	if (self->positions && psy_dsp_envelope_playing(&self->amplitudeenvelope)) {
		psy_List* p;

		for (p = self->positions; p != NULL; psy_list_next(&p)) {
			psy_audio_SampleIterator* it;

			it = (psy_audio_SampleIterator*)p->entry;
			psy_audio_xmsamplervoice_updateiteratorspeed(self, it);
		}
	}
}

void psy_audio_xmsamplervoice_updateiteratorspeed(psy_audio_XMSamplerVoice* self,
	psy_audio_SampleIterator* it)
{
	double period;
	double speed;
	
	period = self->period;
	period += psy_audio_xmsamplervoice_vibratoamount(self);
	speed = self->sampler->amigaslides
		? psy_dsp_amigaperiodtospeed((int)period,
			psy_audio_machine_samplerate(psy_audio_xmsampler_base(self->sampler)),
			0)
		: psy_dsp_periodtospeed((int)period,
			psy_audio_machine_samplerate(psy_audio_xmsampler_base(self->sampler)),
			it->sample->samplerate, 0);
	//\todo: Attention, AutoVibrato always use linear slides with IT, but in
	// FT2 it depends on amigaslides switch.	
	// speed *= pow(2.0, ((-AutoVibratoAmount()) / 768.0));	
	psy_audio_sampleiterator_setspeed(it, speed);
}

void psy_audio_xmsamplervoice_noteon_frequency(psy_audio_XMSamplerVoice* self,
	double frequency)
{
	psy_audio_Sample* sample;
	psy_List* entries;
	psy_List* p;

	psy_audio_xmsamplervoice_clearpositions(self);
	entries = psy_audio_instrument_entriesintersect(self->instrument,
		0, 0, frequency);
	for (p = entries; p != NULL; psy_list_next(&p)) {
		psy_audio_InstrumentEntry* entry;

		entry = (psy_audio_InstrumentEntry*)p->entry;
		sample = psy_audio_samples_at(self->samples, entry->sampleindex);
		if (sample) {
			psy_audio_SampleIterator* iterator;

			iterator = psy_audio_sample_allociterator(sample,
				self->resamplertype);
			psy_list_append(&self->positions, iterator);
			psy_audio_sampleiterator_setspeed(iterator, frequency / 440);
			psy_audio_sampleiterator_play(iterator);			
		}
	}
	psy_list_free(entries);
	if (self->positions) {
		psy_dsp_envelope_start(&self->amplitudeenvelope);
		psy_dsp_envelope_start(&self->filterenvelope);
	}	
}

void psy_audio_xmsamplervoice_clearpositions(psy_audio_XMSamplerVoice* self)
{
	psy_list_deallocate(&self->positions, (psy_fp_disposefunc)
		psy_audio_sampleiterator_dispose);	
}

void psy_audio_xmsamplervoice_nna(psy_audio_XMSamplerVoice* self)
{
	if (self->instrument) {
		switch (psy_audio_instrument_nna(self->instrument)) {
			case psy_audio_NNA_STOP:
				psy_audio_xmsamplervoice_fastnoteoff(self);
			break;
			case psy_audio_NNA_NOTEOFF:
				psy_audio_xmsamplervoice_noteoff(self);
			break;
			case psy_audio_NNA_CONTINUE:				
			break;
			default:
				// note cut
				psy_audio_xmsamplervoice_fastnoteoff(self);
			break;
		}
	}
}

void psy_audio_xmsamplervoice_noteoff(psy_audio_XMSamplerVoice* self)
{
	if (!psy_audio_xmsamplervoice_isplaying(self)) {
		return;
	}
	psy_audio_xmsamplervoice_setstopping(self, TRUE);
	psy_dsp_envelope_release(&self->amplitudeenvelope);
	psy_dsp_envelope_release(&self->filterenvelope);
	self->stopping = TRUE;
}

void psy_audio_xmsamplervoice_fastnoteoff(psy_audio_XMSamplerVoice* self)
{
	if (!psy_audio_xmsamplervoice_isplaying(self)) {
		return;
	}
	psy_audio_xmsamplervoice_setstopping(self, TRUE);
	psy_dsp_envelope_release(&self->amplitudeenvelope);
	// psy_dsp_adsr_fastrelease(&self->amplitudeenvelope);
	psy_dsp_envelope_release(&self->filterenvelope);
	//psy_dsp_adsr_fastrelease(&self->filterenvelope);
	// Fade Out Volume
	self->volumefadespeed = 1000.0f / (3.f * psy_audio_machine_samplerate(
		psy_audio_xmsampler_base(self->sampler))); // 3 milliseconds of samples. (same as volume ramping)
	self->volumefadeamount = 1.0f;
	self->stopping = TRUE;
}

void psy_audio_xmsamplervoice_work(psy_audio_XMSamplerVoice* self,
	psy_audio_Buffer* dstbuffer, uintptr_t amount)
{
	if (!self->instrument) {
		psy_audio_xmsamplervoice_setisplaying(self, FALSE);
		return;
	}
	if (self->positions && psy_dsp_envelope_playing(&self->amplitudeenvelope)) {
		psy_List* p;
		psy_dsp_amp_t* env;
		psy_dsp_amp_t* filterenv;
		uintptr_t i;		
		
		env = malloc(amount * sizeof(psy_dsp_amp_t));
		for (i = 0; i < amount; ++i) {
			psy_dsp_envelope_tick(&self->amplitudeenvelope);
			env[i] = self->amplitudeenvelope.value;
		}
		filterenv = NULL;
		if (filter_type(&self->_filter) != F_NONE) {
			filterenv = malloc(amount * sizeof(psy_dsp_amp_t));
			for (i = 0; i < amount; ++i) {
				psy_dsp_envelope_tick(&self->filterenvelope);
				filterenv[i] = self->filterenvelope.value;
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
			psy_audio_xmsamplervoice_currvolume(self, position->sample, &svol,
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
					//////////////////////////////////////////////////////////////////////////
					//  Step 0 : Process Volume.

					// Amplitude Envelope 
					// Voice::RealVolume() returns the calculated volume out of "WaveData.WaveGlobVol() * Instrument.Volume() * Voice.NoteVolume()"
					float volume = psy_audio_xmsamplervoice_realvolume(self, position->sample) *
						((self->channel)
							? self->channel->volume
							: 1.0f);
					if (env[dstpos] <= 0.0f) {
						psy_audio_xmsamplervoice_setisplaying(self, FALSE);
						if (psy_audio_buffer_mono(&position->sample->channels)) {
							psy_audio_buffer_make_monoaureal(dstbuffer, amount);
						}
						return;
					}
					volume *= env[dstpos];
					// Volume Fade Out
					if (self->volumefadespeed > 0.0f)
					{
						psy_audio_xmsamplervoice_updatefadeout(self, position->sample);
						if (self->volumefadeamount <= 0) {
							psy_audio_xmsamplervoice_setisplaying(self, FALSE);
							if (psy_audio_buffer_mono(&position->sample->channels)) {
								psy_audio_buffer_make_monoaureal(dstbuffer, amount);
							}
							return;
						}
						volume *= self->volumefadeamount;
					}
					float lVolDest = 0.f;
					float rVolDest = 0.f;
					if (self->surround) {
						if (self->sampler->panningmode == psy_audio_PANNING_LINEAR) {
							lVolDest = 0.5f * volume;
							rVolDest = -0.5f * volume;
						} else if (self->sampler->panningmode == psy_audio_PANNING_TWOWAY) {
							lVolDest = volume;
							rVolDest = -1.f * volume;
						} else if (self->sampler->panningmode == psy_audio_PANNING_EQUALPOWER) {
							lVolDest = 0.705f * volume;
							rVolDest = -0.705f * volume;
						}
					} else {
						// Panning Envelope 
						// (actually, the correct word for panning is panoramization. "panning" comes from the diminutive "pan")
						// PanFactor() contains the pan calculated at note start ( pan of note, wave pan, instrument pan, NoteModPan sep, and channel pan)
						float lvol = 0;
						float rvol = self->panfactor + self->panbrelloamount;

						//if (m_PanEnvelope.Stage() & EnvelopeController::EnvelopeStage::DOSTEP) {
							//m_PanEnvelope.Work();
						//}
						// PanRange() is a Range delimiter for the envelope, which is set whenever the pan is changed.
						//rvol += (m_PanEnvelope.ModulationAmount() * PanRange());

						if (self->sampler->panningmode == psy_audio_PANNING_LINEAR) {
							lvol = (1.0f - rvol);
							// PanningMode::Linear is already on rvol, so we omit the case.
						} else if (self->sampler->panningmode == psy_audio_PANNING_TWOWAY) {
							lvol = min(1.0f, (1.0f - rvol) * 2);
							rvol = min(1.0f, rvol * 2.0f);
						} else if (self->sampler->panningmode == psy_audio_PANNING_EQUALPOWER) {
							//lvol = powf((1.0f-rvol),0.5f); // This is the commonly used one
							lvol = log10f(((1.0f - rvol) * 9.0f) + 1.0f); // This is a faster approximation
							//rvol = powf(rvol, 0.5f);// This is the commonly used one
							rvol = log10f((rvol * 9.0f) + 1.0f); // This is a faster approximation.
						}
						lVolDest = lvol * volume;
						rVolDest = rvol * volume;
					}
					//Volume Ramping.
					psy_dsp_slider_settarget(&self->rampl, lVolDest);
					psy_dsp_slider_settarget(&self->rampr, rVolDest);					

					for (channel = 0; channel < psy_audio_buffer_numchannels(&position->sample->channels) &&
							channel < psy_audio_buffer_numchannels(dstbuffer); ++channel) {
						psy_dsp_amp_t output;
						//////////////////////////////////////////////////////////////////////////
						//  Step 1 : Get the unprocessed wave data.

						output = psy_audio_sampleiterator_work(position, channel);					

						//////////////////////////////////////////////////////////////////////////
						//  Step 2 : processed filter

						if (psy_audio_xmsampler_usefilters(self->sampler)) {
							output = psy_audio_xmsamplervoice_workfilter(self, channel,
								output, filterenv, dstpos);							
						}
						//Volume after the filter, like schism/IT.
						//If placed before the filter, 303.IT sounds bad (uncontrolled ressonance, replicable in schism if removing the volume changes).
						if (channel == 0) {
							output *= psy_dsp_slider_getnext(&self->rampl);
						} else
						if (channel == 1) {
							output *= psy_dsp_slider_getnext(&self->rampr);
						}						

						// Pitch Envelope.Currently, the pitch envelope Amount is only updated on NewLine().
						//	if (m_PitchEnvelope.Stage() & EnvelopeController::EnvelopeStage::DOSTEP) {
						//		m_PitchEnvelope.Work();
						//	}

						//////////////////////////////////////////////////////////////////////////
						//  Step 3: Add the processed data to the sampler's buffer.
						psy_audio_xmsamplervoice_adddatatosamplerbuffer(self, channel,
							output, dstpos, dstbuffer);
					}
					// Move sample position
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
					psy_audio_xmsamplervoice_setisplaying(self, FALSE);					
					break;
				}
			}
			if (psy_audio_buffer_mono(&position->sample->channels)) {
				psy_audio_buffer_make_monoaureal(dstbuffer, amount);
			}			
		}		
		free(env);
		free(filterenv);
	}
	self->dooffset = 0;
}

void psy_audio_xmsamplervoice_updatefadeout(psy_audio_XMSamplerVoice* self, psy_audio_Sample* sample)
{
	if (psy_audio_xmsamplervoice_realvolume(self, sample) == 0.0f) {
		//IsPlaying(false);
		self->stopping = TRUE;
	}
	self->volumefadeamount -= self->volumefadespeed;
	if (self->volumefadeamount <= 0) {
		self->stopping = TRUE;
		//IsPlaying(false);
	}
}

/*void psy_audio_xmsamplervoice_notefadeout(psy_audio_XMSamplerVoice* self)
{
	self->stopping = TRUE;
	self->volumefadespeed = self->instrument->volumefadespeed /
		psy_audio_machine_samplespertick(self->sampler);
	self->volumefadeamount = 1.0f;	
	//if (RealVolume() * rChannel().Volume() == 0.0f) IsPlaying(false);
	//The following is incorrect, at least with looped envelopes that also have sustain loops.
	//else if ( m_AmplitudeEnvelope.Envelope().IsEnabled() && m_AmplitudeEnvelope.ModulationAmount() == 0.0f) IsPlaying(false);

	//This might not be correct, but since we are saying "IsStopping(true"), then the controller needs to recalculate the buffers.
	//m_WaveDataController.NoteOff();
}*/

void psy_audio_xmsamplervoice_setresamplerquality(psy_audio_XMSamplerVoice* self, 
	psy_dsp_ResamplerQuality quality)
{
	self->resamplertype = quality;
	if (self->positions && psy_dsp_envelope_playing(&self->amplitudeenvelope)) {
		psy_List* p;
		for (p = self->positions; p != NULL; psy_list_next(&p)) {
			psy_audio_SampleIterator* iterator;

			iterator = (psy_audio_SampleIterator*)p->entry;
			psy_dsp_multiresampler_setquality(&iterator->resampler, quality);
		}
	}
}

void psy_audio_xmsamplervoice_adddatatosamplerbuffer(psy_audio_XMSamplerVoice* self,
	uintptr_t channel, psy_dsp_amp_t input, uintptr_t pos,
	psy_audio_Buffer* output)
{
	psy_dsp_amp_t* dst;

	dst = psy_audio_buffer_at(output, channel);
	if (dst) {
		dst[pos] += input;
	}
}

void psy_audio_xmsamplervoice_currvolume(psy_audio_XMSamplerVoice* self,
	psy_audio_Sample* sample, psy_dsp_amp_t* svol, psy_dsp_amp_t* lvol,
	psy_dsp_amp_t* rvol)
{
	psy_dsp_amp_t cvol;

	*svol = psy_audio_xmsamplervoice_volume(self) *
		(self->usedefaultvolume || self->effcmd == XM_SAMPLER_CMD_VOLUMESLIDE)
			? sample->defaultvolume / (psy_dsp_amp_t)0x80
			: psy_audio_xmsamplervoice_realvolume(self, sample);
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

psy_dsp_amp_t psy_audio_xmsamplervoice_workfilter(psy_audio_XMSamplerVoice* self,
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

void psy_audio_xmsamplervoice_release(psy_audio_XMSamplerVoice* self)
{
	self->effcmd = XM_SAMPLER_CMD_NONE;
	psy_dsp_envelope_release(&self->amplitudeenvelope);
	psy_dsp_envelope_release(&self->filterenvelope);
}

void psy_audio_xmsamplervoice_fastrelease(psy_audio_XMSamplerVoice* self)
{
	self->effcmd = XM_SAMPLER_CMD_NONE;
	//psy_dsp_adsr_fastrelease(&self->amplitudeenvelope);
	psy_dsp_envelope_release(&self->amplitudeenvelope);
	//psy_dsp_envelope_fastrelease(&self->filterenvelope);
	psy_dsp_envelope_release(&self->filterenvelope);
}

// Voice Effects
void psy_audio_xmsamplervoice_newline(psy_audio_XMSamplerVoice* self)
{
	self->effcmd = XM_SAMPLER_CMD_NONE;
	psy_list_free(self->effects);
	self->effects = NULL;
}

void psy_audio_xmsamplervoice_seteffect(psy_audio_XMSamplerVoice* self,
	const psy_audio_PatternEvent* ev)
{	
	int cmd;
	int realset = 0;
	int	realvalue = 0;
	int volcmd = 0;
	int slidval = 0;

	cmd = ev->cmd;

	/*
	switch (volcmd & 0xF0)
	{
		case SAMPLER_CMD_VOL_VOLUME0:
		case SAMPLER_CMD_VOL_VOLUME1:
		case SAMPLER_CMD_VOL_VOLUME2:
		case SAMPLER_CMD_VOL_VOLUME3:
			psy_audio_xmsamplervoice_setvolume(self,
				volcmd << 1);			
			break;
		case SAMPLER_CMD_VOL_VOLSLIDEUP:
			psy_audio_xmsamplervoice_setvolumeslide(self,
				(volcmd & 0x0F) << 4);
			break;
		case SAMPLER_CMD_VOL_VOLSLIDEDOWN:
			psy_audio_xmsamplervoice_setvolumeslide(self,
				volcmd & 0x0F);
			break;
		case SAMPLER_CMD_VOL_FINEVOLSLIDEUP:
			self->volumeslidespeed = (volcmd & 0x0F) << 1;
			psy_audio_xmsamplervoice_performvolumeslide(self);
			break;
		case SAMPLER_CMD_VOL_FINEVOLSLIDEDOWN:
			self->volumeslidespeed = -((volcmd & 0x0F) << 1);
			psy_audio_xmsamplervoice_performvolumeslide(self);
			break;
			//				case CMD_VOL::VOL_VIBRATO_SPEED:
			//					Vibrato(volcmd&0x0F,0); //\todo: vibrato_speed does not activate the vibrato if it isn't running.
			//					break;
			//
		case SAMPLER_CMD_VOL_VIBRATO:
			psy_audio_xmsamplervoice_setvibrato(self,
				0, (volcmd & 0x0F) << 2);
			break;
		case SAMPLER_CMD_VOL_TONEPORTAMENTO:
			// Portamento to (Gx) affects the memory for Gxx and has the equivalent
			// slide given by this table:
			// SlideTable      DB      1, 4, 8, 16, 32, 64, 96, 128, 255
			if ((volcmd & 0x0F) == 0) slidval = 0;
			else if ((volcmd & 0x0F) == 1)  slidval = 1;
			else if ((volcmd & 0x0F) < 9) slidval = powf(2.0f, volcmd & 0x0F);
			else slidval = 255;
			//psy_audio_xmsamplervoice_setpitchslide(self,
			// PitchSlide(voice->Period() > voice->NoteToPeriod(Note()), slidval, Note());
			break;
		case SAMPLER_CMD_VOL_PITCH_SLIDE_DOWN:
			// Pitch slide up/down affect E/F/(G)'s memory - a Pitch slide
			// up/down of x is equivalent to a normal slide by x*4
			//psy_audio_xmsamplervoice_setpitchslide(self,
				//FALSE, (volcmd & 0x0F) << 2));
			break;
		case SAMPLER_CMD_VOL_PITCH_SLIDE_UP:
			//PitchSlide(true, (volcmd & 0x0F) << 2);
			break;
		default:
			break;
	}*/

	switch (cmd) {
		case XM_SAMPLER_CMD_VOLUME:
			self->usedefaultvolume = 0;
			psy_audio_xmsamplervoice_setvolume(self, ev->parameter);
			break;
		case XM_SAMPLER_CMD_PANNING:
			if (self->channel) {
				psy_audio_xmsamplervoice_setpanfactor(self,
					self->channel->panfactor);
			}
			break;
		case XM_SAMPLER_CMD_OFFSET:
			self->dooffset = 1;
			self->offset = ev->parameter;
			break;
		case XM_SAMPLER_CMD_PORTAMENTO_UP: {
			self->effval = ev->parameter;
			self->effcmd = cmd;
			break;
		}
		case XM_SAMPLER_CMD_PORTAMENTO_DOWN: {
			self->effval = ev->parameter;
			self->effcmd = cmd;
			break;
		}
		case XM_SAMPLER_CMD_PORTA2NOTE: {

		}
		case XM_SAMPLER_CMD_VOLUMESLIDE:
			psy_audio_xmsamplervoice_setvolumeslide(self,
				ev->parameter);
			break;
		case XM_SAMPLER_CMD_EXTENDED:
			switch (ev->parameter & 0xF0) {
			case XM_SAMPLER_CMD_E9:
				switch (ev->parameter & 0x0F) {
				case XM_SAMPLER_CMD_E9_SURROUND_OFF:
					self->surround = FALSE;
					break;
				case XM_SAMPLER_CMD_E9_SURROUND_ON:
					self->surround = TRUE;
					break;
				case XM_SAMPLER_CMD_E9_REVERB_OFF:
					break;
				case XM_SAMPLER_CMD_E9_REVERB_FORCE:
					break;
				case XM_SAMPLER_CMD_E9_STANDARD_SURROUND:
					break;
				case XM_SAMPLER_CMD_E9_QUAD_SURROUND:
					break;
				case XM_SAMPLER_CMD_E9_GLOBAL_FILTER:
					break;
				case XM_SAMPLER_CMD_E9_LOCAL_FILTER:
					break;
				default:
					break;
				}
				break;
			case XM_SAMPLER_CMD_E_SET_PAN:
				if (self->channel) {
					psy_audio_xmsamplervoice_setpanfactor(self,
						psy_audio_xmsamplerchannel_panfactor(self->channel));
				}
				break;
			case XM_SAMPLER_CMD_E_SET_MIDI_MACRO:
				break;
			case XM_SAMPLER_CMD_E_GLISSANDO_TYPE:
				break;
			case XM_SAMPLER_CMD_E_VIBRATO_WAVE:
				break;
			case XM_SAMPLER_CMD_E_PANBRELLO_WAVE:
				break;
			case XM_SAMPLER_CMD_E_TREMOLO_WAVE:
				break;
			default:
				break;
			}
		break;
		case XM_SAMPLER_CMD_MIDI_MACRO:
			if (ev->parameter < 0x80) {
				if (self->channel) {
					realset = self->channel->midi_set;
				}
				realvalue = ev->parameter;
			} else {
				//realset = m_pSampler->GetMap(ev->parameter - 0x80).mode;
				//realvalue = m_pSampler->GetMap(ev->parameter - 0x80).value;
			}
			switch (realset) {
			case 0:				
				if (self->channel) {
					if (psy_audio_xmsamplervoice_filtertype(self) == F_NONE) {
						filter_settype(&self->_filter, self->channel->defaultfiltertype);					
					}
					self->_cutoff = self->channel->cutoff;
				}
				break;
			case 1:
				if (self->channel) {
					if (psy_audio_xmsamplervoice_filtertype(self) == F_NONE) {
						filter_settype(&self->_filter, self->channel->defaultfiltertype);
					}
					self->m_Ressonance = self->channel->ressonance;				
				}
				break;
			case 2:
				// Set filter mode. OpenMPT only says 0..F lowpass and 10..1F highpass.
				// It also has a macro default setup where 0 and 8 set the lowpass and 10 an 18 set the highpass
				// From there, I adapted the following table for Psycle.
				if (self->channel) {
					filter_settype(&self->_filter,
						self->channel->m_DefaultFilterType);
				} else
				if (realvalue < 0x20) {
					if (realvalue < 4) { //0..3
						filter_settype(&self->_filter, F_ITLOWPASS);
					} else if (realvalue < 6) { //4..5
						filter_settype(&self->_filter, F_LOWPASS12);
					} else if (realvalue < 8) { //6..7
						filter_settype(&self->_filter, F_BANDPASS12);
					} else if (realvalue < 0xC) { //8..B
						filter_settype(&self->_filter, F_MPTLOWPASSE);
					} else if (realvalue < 0xE) { //C..D
						filter_settype(&self->_filter, F_LOWPASS12E);
					} else if (realvalue < 0x10) { //E..F
						filter_settype(&self->_filter, F_BANDPASS12E);
					} else if (realvalue < 0x14) { //10..13
						filter_settype(&self->_filter, F_MPTHIGHPASSE);
					} else if (realvalue < 0x16) { //14..15
						filter_settype(&self->_filter, F_HIGHPASS12);
					} else if (realvalue < 0x18) { //16..17
						filter_settype(&self->_filter, F_BANDREJECT12);
					} else if (realvalue < 0x1C) { //18..1B
						filter_settype(&self->_filter, F_MPTHIGHPASSE);
					} else if (realvalue < 0x1E) { //1C..1D
						filter_settype(&self->_filter, F_HIGHPASS12E);
					} else { // 1E..1F
						filter_settype(&self->_filter, F_BANDREJECT12E);
					}
				}
				break;
			default:
				break;
			}
			break;
		case XM_SAMPLER_CMD_PANNINGSLIDE:
			psy_audio_xmsamplervoice_setpanningslide(self,
				ev->parameter);
			break;
		case XM_SAMPLER_CMD_VIBRATO:
			psy_audio_xmsamplervoice_setvibrato(self,
				((ev->parameter >> 4) & 0x0F),
				(ev->parameter & 0x0F) << 2);
			break;
		case XM_SAMPLER_CMD_FINE_VIBRATO:
			psy_audio_xmsamplervoice_setvibrato(self,
				((ev->parameter >> 4) & 0x0F),
				(ev->parameter & 0x0F));
			break;
		case XM_SAMPLER_CMD_TREMOR:
			psy_audio_xmsamplervoice_settremor(self,				
				ev->parameter);
			break;
		case XM_SAMPLER_CMD_TREMOLO:
			psy_audio_xmsamplervoice_settremolo(self,
				((ev->parameter >> 4) & 0x0F),
				(ev->parameter & 0x0F));
			break;
		case XM_SAMPLER_CMD_PANBRELLO:
			psy_audio_xmsamplervoice_setpanbrello(self,
				((ev->parameter >> 4) & 0x0F),
				(ev->parameter & 0x0F));
			break;
		case XM_SAMPLER_CMD_RETRIG:
			psy_audio_xmsamplervoice_setretrigger(self);
			break;

		default:
			break;
	}	
}

void psy_audio_xmsamplervoice_performfx(psy_audio_XMSamplerVoice* self)
{	
	psy_List* p;

	for (p = self->effects; p != NULL; psy_list_next(&p)) {
		int effect = (int)(intptr_t)psy_list_entry(p);

		switch (effect) {
			case XM_SAMPLER_EFFECT_VOLUMESLIDE:
				psy_audio_xmsamplervoice_performvolumeslide(self);
			break;
			case XM_SAMPLER_EFFECT_PANSLIDE:
				psy_audio_xmsamplervoice_performpanningslide(self);
			break;
			case XM_SAMPLER_EFFECT_VIBRATO:
				psy_audio_xmsamplervoice_performvibrato(self);
				break;
			case XM_SAMPLER_EFFECT_TREMOR:
				psy_audio_xmsamplervoice_performtremor(self);
				break;
			case XM_SAMPLER_EFFECT_TREMOLO:
				psy_audio_xmsamplervoice_performtremolo(self);
				break;
			case XM_SAMPLER_EFFECT_PANBRELLO:
				psy_audio_xmsamplervoice_performpanbrello(self);
				break;
			case XM_SAMPLER_EFFECT_RETRIG:
				psy_audio_xmsamplervoice_performretrig(self);
				break;
			default:
				break;
		}
	}
	switch (self->effcmd) {	
		case XM_SAMPLER_CMD_PORTAMENTO_UP: {
			self->period -= self->effval;
			break;
		}
		case XM_SAMPLER_CMD_PORTAMENTO_DOWN: {
			self->period += self->effval;
			break;
		}
		default:
			break;
	}
	psy_audio_xmsamplervoice_updatespeed(self);
}

void psy_audio_xmsamplervoice_effectinit(psy_audio_XMSamplerVoice* self)
{
}

void psy_audio_xmsamplervoice_reseteffects(psy_audio_XMSamplerVoice* self)
{
	// pitch slide
	self->slide2notedestperiod = 0;
	self->pitchslidespeed = 0;
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
void psy_audio_xmsamplervoice_setvolumeslide(psy_audio_XMSamplerVoice* self, int speed)
{
	if (ISSLIDEUP(speed)) { // Slide Up
		speed = GETSLIDEUPVAL(speed);

		psy_audio_xmsamplervoice_addeffect(self, XM_SAMPLER_EFFECT_VOLUMESLIDE);
		self->volumeslidespeed = speed << 1;
		if (speed == 0xF) {
			psy_audio_xmsamplervoice_performvolumeslide(self);
		}
	} else if (ISSLIDEDOWN(speed)) { // Slide Down
		speed = GETSLIDEDOWNVAL(speed);
		psy_audio_xmsamplervoice_addeffect(self, XM_SAMPLER_EFFECT_VOLUMESLIDE);
		self->volumeslidespeed = -(speed << 1);
		if (speed == 0xF) {
			psy_audio_xmsamplervoice_performvolumeslide(self);
		}
	} else if (ISFINESLIDEUP(speed)) { // FineSlide Up		
		self->volumeslidespeed = GETSLIDEUPVAL(speed) << 1;
		psy_audio_xmsamplervoice_performvolumeslide(self);
	} else if (ISFINESLIDEDOWN(speed)) { // FineSlide Down		
		self->volumeslidespeed = -(GETSLIDEDOWNVAL(speed) << 1);
		psy_audio_xmsamplervoice_performvolumeslide(self);
	}
}

void psy_audio_xmsamplervoice_performvolumeslide(psy_audio_XMSamplerVoice * self)
{
	if (self->volumeslidespeed > 0) {
		psy_audio_xmsamplervoice_volumeup(self, self->volumeslidespeed);
	} else {
		psy_audio_xmsamplervoice_volumedown(self, self->volumeslidespeed);
	}
}

void psy_audio_xmsamplervoice_volumedown(psy_audio_XMSamplerVoice* self, int value)
{	
	int vol = psy_audio_xmsamplervoice_volume(self) + value;
	if (vol < 0) {
		vol = 0;
	}
	psy_audio_xmsamplervoice_setvolume(self, vol);
}

void psy_audio_xmsamplervoice_volumeup(psy_audio_XMSamplerVoice* self, int value)
{
	int vol = psy_audio_xmsamplervoice_volume(self) + value;
	if (vol > 0x80) {
		vol = 0x80;
	}
	psy_audio_xmsamplervoice_setvolume(self, vol);
}

void psy_audio_xmsamplervoice_setpanningslide(psy_audio_XMSamplerVoice* self,
	int speed)
{	
	if (ISSLIDEUP(speed)) { // Slide Left
		speed = GETSLIDEUPVAL(speed);
		psy_audio_xmsamplervoice_addeffect(self, XM_SAMPLER_EFFECT_PANSLIDE);
		if (speed == 0xF) {
			psy_audio_xmsamplervoice_performpanningslide(self);
		}
	} else if (ISSLIDEDOWN(speed)) { // Slide Right
		speed = GETSLIDEDOWNVAL(speed);
		psy_audio_xmsamplervoice_addeffect(self, XM_SAMPLER_EFFECT_PANSLIDE);
		if (speed == 0xF) {
			psy_audio_xmsamplervoice_performpanningslide(self);
		}
	} else if (ISFINESLIDEUP(speed)) { // FineSlide left
		psy_audio_xmsamplervoice_performpanningslide(self);
	} else if (ISFINESLIDEDOWN(speed)) { // FineSlide right
		psy_audio_xmsamplervoice_performpanningslide(self);
	}
}

void psy_audio_xmsamplervoice_performpanningslide(psy_audio_XMSamplerVoice* self)
{
	if (self->channel) {
		psy_audio_xmsamplervoice_setpanfactor(self, self->channel->panfactor);
	}
}

void psy_audio_xmsamplervoice_setpanfactor(psy_audio_XMSamplerVoice* self, float pan)
{
	self->panfactor = pan;
	self->panfactor = self->panrange =(float)( (0.5 - fabs((double)pan - 0.5)));
}

void psy_audio_xmsamplervoice_setvibrato(psy_audio_XMSamplerVoice* self, int speed,
	int depth)
{
	self->vibratospeed = speed << 2;
	self->vibratodepth = depth;
	psy_audio_xmsamplervoice_addeffect(self, XM_SAMPLER_EFFECT_VIBRATO);
}

void psy_audio_xmsamplervoice_setpitchslide(psy_audio_XMSamplerVoice* self,
	bool bUp, int speed, int note)
{
	return;

	if (!self->channel) {
		return;
	}
	if (speed == 0) {
		if (self->channel->pitchslidemem == 0) return;
		speed = self->channel->pitchslidemem;
	}
	if (speed < 0xE0 || note != psy_audio_NOTECOMMANDS_EMPTY)	// Portamento , Fine porta ("f0", and Extra fine porta "e0" ) (*)
	{									// Porta to note does not have Fine.
		speed <<= 2;
		//if (ForegroundVoice()) { ForegroundVoice()->
		self->pitchslidespeed = bUp ? -speed : speed;
		if (note != psy_audio_NOTECOMMANDS_EMPTY)
		{
			if (note != psy_audio_NOTECOMMANDS_RELEASE) {
				//	if (ForegroundVoice()) { ForegroundVoice()
				// self->slide2notedestperiod = NoteToPeriod(note); }
				psy_audio_xmsamplervoice_addeffect(self, XM_SAMPLER_EFFECT_SLIDE2NOTE);
			}
		} else {
			psy_audio_xmsamplervoice_addeffect(self, XM_SAMPLER_EFFECT_PITCHSLIDE);
		}
	} else if (speed < 0xF0) {
		speed = speed & 0xf;
		// if (ForegroundVoice())
		// {
			//ForegroundVoice()->
			self->pitchslidespeed = bUp ? -speed : speed;			
			////ForegroundVoice()->PitchSlide();
			psy_audio_xmsamplervoice_performpitchslide(self);
		//}
	} else {
		speed = (speed & 0xf) << 2;
		//if (ForegroundVoice())
		//{
			//ForegroundVoice()->m_PitchSlideSpeed = bUp ? -speed : speed;
			self->pitchslidespeed = bUp ? -speed : speed;
			//ForegroundVoice()->PitchSlide();
			psy_audio_xmsamplervoice_performpitchslide(self);
		//}
	}
}

void psy_audio_xmsamplervoice_performvibrato(psy_audio_XMSamplerVoice* self)
{
	if (self->channel) {
		int vdelta = psy_audio_xmsampler_getdelta(self->channel->vibratotype,
			self->vibratopos);

		vdelta = vdelta * self->vibratodepth;
		self->vibratoamount = (double)vdelta / 32.0;
		self->vibratopos = (self->vibratopos - self->vibratospeed) & 0xff;		
	}
}// Vibrato() -------------------------------------

void psy_audio_xmsamplervoice_performpitchslide(psy_audio_XMSamplerVoice* self)
{
	return; 
	self->period += self->pitchslidespeed;
	psy_audio_xmsamplervoice_updatespeed(self);
} // PitchSlide() -------------------------------------

void psy_audio_xmsamplervoice_settremolo(psy_audio_XMSamplerVoice* self, int speed,
	int depth)
{
	self->tremolospeed = speed << 2;
	self->tremolodepth = depth;
	psy_audio_xmsamplervoice_addeffect(self, XM_SAMPLER_EFFECT_TREMOLO);
}

void psy_audio_xmsamplervoice_performtremolo(psy_audio_XMSamplerVoice* self)
{
	//\todo: verify that final volume doesn't go out of range (Redo RealVolume() ?)
	int vdelta = psy_audio_xmsampler_getdelta(self->channel->tremolotype,
		self->tremolopos);
	vdelta = (vdelta * self->tremolodepth);
	self->tremoloamount = (float)((double)vdelta / 2048.0);
	self->tremolopos = (self->tremolopos + self->tremolospeed) & 0xFF;


}// Tremolo() -------------------------------------------

void psy_audio_xmsamplervoice_setpanbrello(psy_audio_XMSamplerVoice* self,
	int speed, int depth)
{
	self->panbrellospeed = speed << 2;
	self->panbrellodepth = depth;
	psy_audio_xmsamplervoice_addeffect(self, XM_SAMPLER_EFFECT_PANBRELLO);
}

void psy_audio_xmsamplervoice_performpanbrello(psy_audio_XMSamplerVoice* self)
{
	// Yxy   Panbrello with speed x, depth y.
	// The random pan position can be achieved by setting the
	// waveform to 3 (ie. a S53 command). In this case *ONLY*, the
	// speed actually is interpreted as a delay in frames before
	// another random value is found. so Y14 will be a very QUICK
	// panbrello, and Y44 will be a slower panbrello.

	//\todo: verify that final pan doesn't go out of range (make a RealPan()
	// similar to RealVolume() ?)
	int vdelta = psy_audio_xmsampler_getdelta(self->channel->panbrellotype,
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


void psy_audio_xmsamplervoice_settremor(psy_audio_XMSamplerVoice* self, int parameter)
{
	self->tremoronticks = ((parameter >> 4) & 0xf) + 1;
	self->tremoroffticks = (parameter & 0xf) + 1;
	self->tremortickchange = self->tremoronticks;
	psy_audio_xmsamplervoice_addeffect(self, XM_SAMPLER_EFFECT_TREMOR);
}


void psy_audio_xmsamplervoice_performtremor(psy_audio_XMSamplerVoice* self)
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

void psy_audio_xmsamplervoice_setretrigger(psy_audio_XMSamplerVoice* self)
{
	if (self->channel) {
		self->retrigticks = (self->channel->ticks != 0)
			? self->channel->ticks
			: 1;
	}
}

void psy_audio_xmsamplervoice_performretrig(psy_audio_XMSamplerVoice* self)
{
	if (self->sampler->ticktimer.tickcount % self->retrigticks == 0)
	{
		psy_audio_xmsamplervoice_noteon(self, self->note);
	}
}

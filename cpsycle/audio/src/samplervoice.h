// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#ifndef psy_audio_SAMPLERVOICE_H
#define psy_audio_SAMPLERVOICE_H

#include "samplerchannel.h"
#include "samplerdefs.h"
// dsp
#include <adsr.h>
#include <filter.h>
#include <dspslide.h>
#include <valuemapper.h>

#ifdef __cplusplus
extern "C" {
#endif

struct psy_audio_Sampler;
struct psy_audio_Samples;
struct psy_audio_Instrument;

typedef struct {
	uintptr_t channelnum;
	psy_audio_SamplerChannel* channel;
	struct psy_audio_Sampler* sampler;
	struct psy_audio_Samples* samples;

	struct psy_audio_Instrument* instrument;
	psy_audio_NewNoteAction nna;

	psy_dsp_ADSR amplitudeenvelope;
	psy_dsp_ADSR panenvelope;
	psy_dsp_ADSR pitchenvelope;
	psy_dsp_ADSR filterenv;

	// SampleIterator (WaveDataController)
	psy_List* positions;

	ResamplerType resamplertype;
	// todo
	//dsp::ITFilter m_FilterIT;
	//dsp::Filter m_FilterClassic;
	Filter _filter;
	int _cutoff;
	int m_Ressonance;
	float _coModify;

	bool play;
	bool stopping;
	int note;
	int period;
	int m_Volume;
	psy_dsp_amp_t realvolume;
	psy_dsp_amp_t currrandvol;
	// Volume/Panning ramping 
	psy_dsp_Slider rampl;
	psy_dsp_Slider rampr;

	// effects		
	psy_List* effects;
	// pan
	float panfactor;
	//float m_CurRandPan;
	float panrange;
	bool surround;
	// pitchslide
	int slide2notedestperiod;
	int pitchslidespeed;
	// volumeslide
	int volumeslidespeed;
	float volumefadespeed;
	float volumefadeamount;	
	// vibrato
	int vibratospeed;
	int vibratodepth;
	int vibratopos;
	double vibratoamount;
	// tremelo
	int tremolospeed;
	int tremolodepth;
	float tremoloamount;
	int tremolopos;
	// panbrello
	int panbrellospeed;
	int panbrellodepth;
	float panbrelloamount;
	int panbrellopos;
	int panbrellorandomcounter;
	/// tremor 
	int tremoronticks;
	int tremoroffticks;
	int tremortickchange;
	bool tremormute;
	// auto vibrato 
	double autovibratoamount;
	int autovibratodepth;
	int autovibratopos;
	// retrig
	int retrigticks;

	// sampulse ps1
	int usedefaultvolume;
	psy_audio_SamplerCmd effcmd;
	int effval;
	int dooffset;
	uint8_t offset;
	// configuration
	int maxvolume;
} psy_audio_SamplerVoice;

void psy_audio_samplervoice_init(psy_audio_SamplerVoice*,
	struct psy_audio_Sampler* sampler,
	psy_audio_Samples*,
	psy_audio_Instrument*,
	psy_audio_SamplerChannel* channel,
	uintptr_t channelnum, uintptr_t samplerate,
	ResamplerType quality,
	int maxvolume);
void psy_audio_samplervoice_dispose(psy_audio_SamplerVoice*);
psy_audio_SamplerVoice* psy_audio_samplervoice_alloc(void);
psy_audio_SamplerVoice* psy_audio_samplervoice_allocinit(struct psy_audio_Sampler*,
	psy_audio_Instrument*,
	psy_audio_SamplerChannel* channel,
	uintptr_t channelnum,
	uintptr_t samplerate);
void psy_audio_samplervoice_seqtick(psy_audio_SamplerVoice*,
	const psy_audio_PatternEvent*, double samplesprobeat);
void psy_audio_samplervoice_nna(psy_audio_SamplerVoice*);
void psy_audio_samplervoice_noteon(psy_audio_SamplerVoice*,
	int note, double samplesprobeat);
void psy_audio_samplervoice_noteon_frequency(psy_audio_SamplerVoice*,
	double frequency);
void psy_audio_samplervoice_noteoff(psy_audio_SamplerVoice*);
void psy_audio_samplervoice_fastnoteoff(psy_audio_SamplerVoice*);
void psy_audio_samplervoice_work(psy_audio_SamplerVoice*, psy_audio_Buffer*,
	uintptr_t numsamples);
void psy_audio_samplervoice_release(psy_audio_SamplerVoice*);
void psy_audio_samplervoice_fastrelease(psy_audio_SamplerVoice*);
void psy_audio_samplervoice_clearpositions(psy_audio_SamplerVoice*);
void psy_audio_samplervoice_setresamplerquality(psy_audio_SamplerVoice*,
	ResamplerType quality);
void psy_audio_samplervoice_newline(psy_audio_SamplerVoice*);
void psy_audio_samplervoice_tick(psy_audio_SamplerVoice*);
psy_audio_samplervoice_seteffect(psy_audio_SamplerVoice*,
	const struct psy_audio_PatternEvent*);
void psy_audio_samplervoice_performfx(psy_audio_SamplerVoice*);
void psy_audio_samplervoice_effectinit(psy_audio_SamplerVoice*);
void psy_audio_samplervoice_reseteffects(psy_audio_SamplerVoice*);

INLINE psy_dsp_amp_t psy_audio_samplervoice_volume(
	psy_audio_SamplerVoice* self, psy_audio_Sample* sample)
{
	psy_dsp_amp_t rv;
	psy_dsp_amp_t currrandvol = (psy_dsp_amp_t)1.f;

	// Since we have top +12dB in waveglobvolume and we have to clip randvol, we use the current globvol as top.
	// This isn't exactly what Impulse tracker did, but it's a reasonable compromise.
	// Instrument Global Volume [0..1.0f] Global volume affecting all samples of the instrument.
	rv = psy_audio_instrument_volume(self->instrument) * currrandvol * psy_audio_sample_volume(sample);
	if (rv > psy_audio_sample_volume(sample)) {
		rv = psy_audio_sample_volume(sample);
	}
	return rv;
}

// Voice.RealVolume() returns the calculated volume out of "WaveData.WaveGlobVol() * Instrument.Volume() * Voice.NoteVolume()"
INLINE float psy_audio_samplervoice_realvolume(psy_audio_SamplerVoice* self)
{
	return (!self->tremormute)
		? (self->realvolume + self->tremoloamount)
		: 0;
}

void psy_audio_samplervoice_updatefadeout(psy_audio_SamplerVoice* self);

INLINE bool psy_audio_isplaying(psy_audio_SamplerVoice* self)
{
	return self->play;

}

INLINE bool psy_audio_samplervoice_isplaying(psy_audio_SamplerVoice* self)
{
	return self->play;
}

INLINE void psy_audio_samplervoice_setisplaying(psy_audio_SamplerVoice* self,
	bool value)
{
	self->play = value;
}

INLINE bool psy_audio_samplervoice_isstopping(psy_audio_SamplerVoice* self)
{
	return self->stopping;
}

INLINE void psy_audio_samplervoice_setstopping(psy_audio_SamplerVoice* self,
	bool stop)
{
	self->stopping = stop;
}

#ifdef __cplusplus
}
#endif

#endif /* psy_audio_SAMPLERVOICE_H */

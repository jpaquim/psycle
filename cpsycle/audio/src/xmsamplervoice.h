// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#ifndef psy_audio_XMSAMPLERVOICE_H
#define psy_audio_XMSAMPLERVOICE_H

#include "xmsamplerchannel.h"
#include "samplerdefs.h"
// dsp
#include <filter.h>
#include <dspslide.h>
#include <valuemapper.h>

#ifdef __cplusplus
extern "C" {
#endif

struct psy_audio_XMSampler;
struct psy_audio_Samples;
struct psy_audio_Instrument;

typedef struct {
	uintptr_t channelnum;
	psy_audio_XMSamplerChannel* channel;
	struct psy_audio_XMSampler* sampler;
	struct psy_audio_Samples* samples;

	struct psy_audio_Instrument* instrument;
	uintptr_t instidx;
	psy_audio_NewNoteAction nna;

	psy_dsp_Envelope amplitudeenvelope;
	psy_dsp_Envelope panenvelope;
	psy_dsp_Envelope pitchenvelope;
	psy_dsp_Envelope filterenvelope;

	// SampleIterator (WaveDataController)
	psy_List* positions;

	psy_dsp_ResamplerQuality resamplertype;
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
	int volume;	
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
	int effcmd;
	int effval;
	int dooffset;
	uint8_t offset;
	// configuration
	int maxvolume;
} psy_audio_XMSamplerVoice; 

void psy_audio_xmsamplervoice_init(psy_audio_XMSamplerVoice*,
	struct psy_audio_XMSampler* sampler,
	struct psy_audio_Samples*,
	struct psy_audio_Instrument*,
	uintptr_t instidx,
	psy_audio_XMSamplerChannel* channel,
	uintptr_t channelnum, uintptr_t samplerate,
	psy_dsp_ResamplerQuality quality,
	int maxvolume);
void psy_audio_xmsamplervoice_dispose(psy_audio_XMSamplerVoice*);
psy_audio_XMSamplerVoice* psy_audio_xmsamplervoice_alloc(void);
psy_audio_XMSamplerVoice* psy_audio_xmsamplervoice_allocinit(struct psy_audio_XMSampler*,
	psy_audio_Instrument*,
	uintptr_t instidx,
	psy_audio_XMSamplerChannel* channel,
	uintptr_t channelnum,
	uintptr_t samplerate);
void psy_audio_xmsamplervoice_seqtick(psy_audio_XMSamplerVoice*,
	const psy_audio_PatternEvent*);
void psy_audio_xmsamplervoice_nna(psy_audio_XMSamplerVoice*);
void psy_audio_xmsamplervoice_noteon(psy_audio_XMSamplerVoice*, int note);
void psy_audio_xmsamplervoice_noteon_frequency(psy_audio_XMSamplerVoice*,
	double frequency);
void psy_audio_xmsamplervoice_noteoff(psy_audio_XMSamplerVoice*);
void psy_audio_xmsamplervoice_fastnoteoff(psy_audio_XMSamplerVoice*);
void psy_audio_xmsamplervoice_work(psy_audio_XMSamplerVoice*, psy_audio_Buffer*,
	uintptr_t numsamples);
void psy_audio_xmsamplervoice_release(psy_audio_XMSamplerVoice*);
void psy_audio_xmsamplervoice_fastrelease(psy_audio_XMSamplerVoice*);
void psy_audio_xmsamplervoice_clearpositions(psy_audio_XMSamplerVoice*);
void psy_audio_xmsamplervoice_setresamplerquality(psy_audio_XMSamplerVoice*,
	psy_dsp_ResamplerQuality quality);
void psy_audio_xmsamplervoice_newline(psy_audio_XMSamplerVoice*);
void psy_audio_xmsamplervoice_tick(psy_audio_XMSamplerVoice*);
void psy_audio_xmsamplervoice_seteffect(psy_audio_XMSamplerVoice*,
	const struct psy_audio_PatternEvent*);
void psy_audio_xmsamplervoice_performfx(psy_audio_XMSamplerVoice*);
void psy_audio_xmsamplervoice_effectinit(psy_audio_XMSamplerVoice*);
void psy_audio_xmsamplervoice_reseteffects(psy_audio_XMSamplerVoice*);

// Volume of the current note.
INLINE uint16_t psy_audio_xmsamplervoice_volume(psy_audio_XMSamplerVoice* self)
{
	return self->volume;
}

INLINE void psy_audio_xmsamplervoice_setvolume(psy_audio_XMSamplerVoice* self, uint16_t vol)
{
	self->volume = vol;	
}

// Voice.RealVolume() returns the calculated volume out of "WaveData.WaveGlobVol() * Instrument.Volume() * Voice.NoteVolume()"
INLINE float psy_audio_xmsamplervoice_realvolume(psy_audio_XMSamplerVoice* self, psy_audio_Sample* sample)
{
	float realvolume;

	//Since we have top +12dB in waveglobvolume and we have to clip randvol, we use the current globvol as top.
	//This isn't exactly what Impulse tracker did, but it's a reasonable compromise.
	float tmp_rand = psy_audio_instrument_volume(self->instrument) * self->currrandvol * psy_audio_sample_globvolume(sample);
	if (tmp_rand > psy_audio_sample_globvolume(sample)) {
		tmp_rand = psy_audio_sample_globvolume(sample);
	}
	realvolume = psy_dsp_map_128_1(self->volume) * tmp_rand;
	return (!self->tremormute)
		? (realvolume + self->tremoloamount)
		: 0;
}

void psy_audio_xmsamplervoice_updatefadeout(psy_audio_XMSamplerVoice*, psy_audio_Sample*);

INLINE bool psy_audio_isplaying(psy_audio_XMSamplerVoice* self)
{
	return self->play;

}

INLINE bool psy_audio_xmsamplervoice_isplaying(psy_audio_XMSamplerVoice* self)
{
	return self->play;
}

INLINE void psy_audio_xmsamplervoice_setisplaying(psy_audio_XMSamplerVoice* self,
	bool value)
{
	self->play = value;
}

INLINE bool psy_audio_xmsamplervoice_isstopping(psy_audio_XMSamplerVoice* self)
{
	return self->stopping;
}

INLINE void psy_audio_xmsamplervoice_setstopping(psy_audio_XMSamplerVoice* self,
	bool stop)
{
	self->stopping = stop;
}

INLINE psy_dsp_FilterType psy_audio_xmsamplervoice_filtertype(psy_audio_XMSamplerVoice* self)
{
	return filter_type(&self->_filter);
}

#ifdef __cplusplus
}
#endif

#endif /* psy_audio_XMSAMPLERVOICE_H */

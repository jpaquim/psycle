// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

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
struct psy_audio_XMSamplerVoice;

/// ValueType is a float value from  0 to 1.0  (or -1.0 1.0, or whatever else) which can be used as a multiplier.
typedef float XMEnvelopeValueType;

typedef enum XMEnvelopeStage {
	XMENVELOPESTAGE_OFF = 0, // Indicates that it is not active (isEnabled() false, 
	XMENVELOPESTAGE_DOSTEP = 1, // Indicates that the envelope is enabled (either isEnabled() or used sent EE8 command).
	XMENVELOPESTAGE_RELEASED = 2,  // Indicates that a Note-Off has been issued.
	XMENVELOPESTAGE_PAUSED = 4 // Indicates that it is paused by loop.	
} XMEnvelopeStage;

typedef struct XMEnvelopeController {
	int m_Samples;
	float m_sRateDeviation;
	int m_PositionIndex;
	int m_NextEventSample;
	XMEnvelopeStage m_Stage;
	XMEnvelopeValueType defaultValue;

	const psy_dsp_EnvelopeSettings* m_pEnvelope;
	XMEnvelopeValueType m_ModulationAmount;
	XMEnvelopeValueType m_Step;
	struct psy_audio_XMSamplerVoice* voice;
} XMEnvelopeController;

void xmenvelopecontroller_init(XMEnvelopeController*,
	struct psy_audio_XMSamplerVoice* invoice,
	XMEnvelopeValueType defValue);
void xmenvelopecontroller_dispose(XMEnvelopeController*);

void xmenvelopecontroller_initcontroller(XMEnvelopeController*);
void xmenvelopecontroller_initcontroller_envelope(XMEnvelopeController*,
	const psy_dsp_EnvelopeSettings*);
/// NoteOn EnvelopeStage
// Explanation:
//	First, the stage is set to off. Then, if the envelope has points, we check if it has 
//  sustain or normal loop points, enabling the corresponding flags in m_Stage.
//  Second, if the envelope is enabled and there are more points, let's enable the joy!
void xmenvelopecontroller_noteon(XMEnvelopeController*);
/// NoteOff EnvelopeStage
// Explanation:
// First we check if the envelope is active. If it is, we release the envelope.
// Then if the envelope state is paused, we check if it was because of a sustain loop and in that case
// we reenable it. The next work call, it will recheck the status and decide what does it need to do.
void xmenvelopecontroller_noteoff(XMEnvelopeController*);
void xmenvelopecontroller_stop(XMEnvelopeController*);
void xmenvelopecontroller_start(XMEnvelopeController*);
void xmenvelopecontroller_pause(XMEnvelopeController*);
void xmenvelopecontroller_continue(XMEnvelopeController*);
void xmenvelopecontroller_work(XMEnvelopeController*);
void xmenvelopecontroller_newstep(XMEnvelopeController*);

void xmenvelopecontroller_calcstep(XMEnvelopeController*, int start, int end);
void xmenvelopecontroller_setpositioninsamples(XMEnvelopeController*, int samplePos);

INLINE int xmenvelopecontroller_getpositioninsamples(const XMEnvelopeController* self)
{
	return self->m_Samples;
}

INLINE XMEnvelopeValueType xmenvelopecontroller_modulationamount(const XMEnvelopeController* self)
{
	return self->m_ModulationAmount;
}

INLINE const psy_dsp_EnvelopeSettings* xmenvelopecontroller_envelope(const XMEnvelopeController* self)
{
	return self->m_pEnvelope;
}

INLINE XMEnvelopeStage xmenvelopecontroller_stage(const XMEnvelopeController* self)
{
	return self->m_Stage;
}

INLINE void xmenvelopecontroller_setstage(XMEnvelopeController* self,
	XMEnvelopeStage stage)
{
	self->m_Stage = stage;
}

void xmenvelopecontroller_setposition(XMEnvelopeController* self, int posi);

INLINE int xmenvelopecontroller_getposition(const XMEnvelopeController* self)
{
	return self->m_PositionIndex;
}

INLINE float xmenvelopecontroller_sratedeviation(const XMEnvelopeController* self)
{
	return self->m_sRateDeviation;
}

void xmenvelopecontroller_recalcdeviation(XMEnvelopeController*);


typedef struct psy_audio_XMSamplerVoice {
	int m_ChannelNum;
	psy_audio_XMSamplerChannel* m_pChannel;
	struct psy_audio_XMSampler* m_pSampler;	

	uintptr_t _instrument;// Instrument
	struct psy_audio_Instrument* m_pInstrument;
	psy_audio_NewNoteAction m_NNA;

	XMEnvelopeController m_AmplitudeEnvelope;
	XMEnvelopeController m_PanEnvelope;
	XMEnvelopeController m_PitchEnvelope;
	XMEnvelopeController m_FilterEnvelope;

	psy_audio_SampleIterator m_WaveDataController;

	psy_dsp_ResamplerQuality resamplertype;
	// todo
	//dsp::ITFilter m_FilterIT;
	//dsp::Filter m_FilterClassic;
	Filter m_Filter;
	int m_CutOff;
	int m_Ressonance;
	float _coModify;

	bool m_bPlay;
	bool m_Background;
	bool m_Stopping;
	int m_Note;
	int m_Period;
	int m_Volume;
	float m_RealVolume;
	float m_CurrRandVol;
	
	// Volume/Panning ramping 
	psy_dsp_Slider rampl;
	psy_dsp_Slider rampr;
	
	float m_PanFactor;
	//float m_CurRandPan;
	float m_PanRange;
	bool m_Surround;

	int m_Slide2NoteDestPeriod;
	int m_PitchSlideSpeed;

	float m_VolumeFadeSpeed;
	float m_VolumeFadeAmount;

	int m_VolumeSlideSpeed;

	int m_VibratoSpeed;
	int m_VibratoDepth;
	int m_VibratoPos;
	double m_VibratoAmount;

	int m_TremoloSpeed;
	int m_TremoloDepth;
	float m_TremoloAmount;
	int m_TremoloPos;

	// Panbrello
	int m_PanbrelloSpeed;
	int m_PanbrelloDepth;
	float m_PanbrelloAmount;
	int m_PanbrelloPos;
	int m_PanbrelloRandomCounter;

	/// Tremor 
	int m_TremorOnTicks;
	int m_TremorOffTicks;
	int m_TremorTickChange;
	bool m_bTremorMute;


	// Auto Vibrato 
	double m_AutoVibratoAmount;
	int m_AutoVibratoDepth;
	int m_AutoVibratoPos;

	int m_RetrigTicks;
} psy_audio_XMSamplerVoice; 

void psy_audio_xmsamplervoice_init(psy_audio_XMSamplerVoice*);
void psy_audio_xmsamplervoice_dispose(psy_audio_XMSamplerVoice*);
// Object Functions
void psy_audio_xmsamplervoice_reset(psy_audio_XMSamplerVoice*);
void psy_audio_xmsamplervoice_reseteffects(psy_audio_XMSamplerVoice*);

void psy_audio_xmsamplervoice_voiceinit(psy_audio_XMSamplerVoice* self,
	struct psy_audio_Instrument*,
	int channelNum,
	int instrumentNum);
void psy_audio_xmsamplervoice_work(psy_audio_XMSamplerVoice*,
	int numSamples, float* pSamplesL, float* pSamplesR);

// This one is Tracker Tick (Mod-tick)
void psy_audio_xmsamplervoice_tick(psy_audio_XMSamplerVoice*);
// This one is Psycle's "NewLine"
void psy_audio_xmsamplervoice_newline(psy_audio_XMSamplerVoice*);

// defaults: const int16_t playvol=-1,bool reset=true);
void psy_audio_xmsamplervoice_noteon(psy_audio_XMSamplerVoice*,
	uint8_t note, int16_t playvol, bool reset);
void psy_audio_xmsamplervoice_noteoff(psy_audio_XMSamplerVoice*);
void psy_audio_xmsamplervoice_noteofffast(psy_audio_XMSamplerVoice*);
void psy_audio_xmsamplervoice_notefadeout(psy_audio_XMSamplerVoice*);
void psy_audio_xmsamplervoice_updatefadeout(psy_audio_XMSamplerVoice*);

INLINE psy_audio_NewNoteAction psy_audio_xmsamplervoice_nna(const psy_audio_XMSamplerVoice* self)
{
	return self->m_NNA;
}

INLINE void psy_audio_xmsamplervoice_setnna(psy_audio_XMSamplerVoice* self, psy_audio_NewNoteAction value)
{
	self->m_NNA = value;
}

void psy_audio_xmsamplervoice_resetvolandpan(psy_audio_XMSamplerVoice* self,
	int16_t playvol, bool reset);
void psy_audio_xmsamplervoice_updatespeed(psy_audio_XMSamplerVoice*);
double psy_audio_xmsamplervoice_periodtospeed(const psy_audio_XMSamplerVoice*,
	int period);

// Effect-Related Object Functions
INLINE void psy_audio_xmsamplervoice_pitchslide(psy_audio_XMSamplerVoice* self)
{
	self->m_Period += self->m_PitchSlideSpeed;
	psy_audio_xmsamplervoice_updatespeed(self);
}

void psy_audio_xmsamplervoice_slide2note(psy_audio_XMSamplerVoice*);
void psy_audio_xmsamplervoice_vibrato(psy_audio_XMSamplerVoice*);
void psy_audio_xmsamplervoice_tremolo(psy_audio_XMSamplerVoice*);
void psy_audio_xmsamplervoice_panbrello(psy_audio_XMSamplerVoice*);
void psy_audio_xmsamplervoice_tremor(psy_audio_XMSamplerVoice*);
void psy_audio_xmsamplervoice_volumeslide(psy_audio_XMSamplerVoice*);
void psy_audio_xmsamplervoice_volumedown(psy_audio_XMSamplerVoice*,
	int value);
void psy_audio_xmsamplervoice_volumeup(psy_audio_XMSamplerVoice*,
	int value);
void psy_audio_xmsamplervoice_retrig(psy_audio_XMSamplerVoice*);

// Do Auto Vibrato
void psy_audio_xmsamplervoice_doautovibrato(psy_audio_XMSamplerVoice*);

INLINE bool psy_audio_xmsamplervoice_isautovibrato(const psy_audio_XMSamplerVoice* self)
{
	return psy_audio_sample_autovibrato(self->m_WaveDataController.sample);	
}

// Get Auto Vibrato Amount
INLINE double psy_audio_xmsamplervoice_autovibratoamount(const psy_audio_XMSamplerVoice* self)
{
	return self->m_AutoVibratoAmount;
}

// Properties

// Properties
INLINE int psy_audio_xmsamplervoice_instrumentnum(const psy_audio_XMSamplerVoice* self)
{
	return (int)self->_instrument;
}

INLINE void psy_audio_xmsamplervoice_setinstrumentnum(psy_audio_XMSamplerVoice* self, int value)
{
	self->_instrument = value;
}

INLINE psy_audio_Instrument* psy_audio_xmsamplervoice_rinstrument(psy_audio_XMSamplerVoice* self)
{
	return self->m_pInstrument;
}

INLINE void psy_audio_xmsamplervoice_setpinstrument(psy_audio_XMSamplerVoice* self,
	psy_audio_Instrument* p)
{
	self->m_pInstrument = p;
}


INLINE int psy_audio_xmsamplervoice_channelnum(const psy_audio_XMSamplerVoice* self)
{
	return self->m_ChannelNum;
}

INLINE void psy_audio_xmsamplervoice_setchannelnum(psy_audio_XMSamplerVoice* self, int value)
{
	self->m_ChannelNum = value;
}

INLINE void psy_audio_xmsamplervoice_setpchannel(psy_audio_XMSamplerVoice* self,
	psy_audio_XMSamplerChannel* p)
{
	self->m_pChannel = p;
}

INLINE psy_audio_XMSamplerChannel* psy_audio_xmsamplervoice_rchannel(psy_audio_XMSamplerVoice* self)
{
	return self->m_pChannel;
}

INLINE const psy_audio_XMSamplerChannel* psy_audio_xmsamplervoice_rchannel_const(const psy_audio_XMSamplerVoice* self)
{
	return self->m_pChannel;
}

INLINE void psy_audio_xmsamplervoice_setpsampler(psy_audio_XMSamplerVoice* self,
	struct psy_audio_XMSampler* p)
{
	self->m_pSampler = p;
}

INLINE struct psy_audio_XMSampler* psy_audio_xmsamplervoice_psampler(psy_audio_XMSamplerVoice* self)
{
	return self->m_pSampler;
}

INLINE XMEnvelopeController* psy_audio_xmsamplervoice_amplitudeenvelope(psy_audio_XMSamplerVoice* self) { return &self->m_AmplitudeEnvelope; }
INLINE XMEnvelopeController* psy_audio_xmsamplervoice_filterenvelope(psy_audio_XMSamplerVoice* self) { return &self->m_FilterEnvelope; }
INLINE XMEnvelopeController* psy_audio_xmsamplervoice_pitchenvelope(psy_audio_XMSamplerVoice* self) { return &self->m_PitchEnvelope; }
INLINE XMEnvelopeController* psy_audio_xmsamplervoice_panenvelope(psy_audio_XMSamplerVoice* self) { return &self->m_PanEnvelope; }

INLINE bool psy_audio_xmsamplervoice_isplaying(const psy_audio_XMSamplerVoice* self)
{
	return self->m_bPlay;
}

INLINE void psy_audio_xmsamplervoice_setisplaying(psy_audio_XMSamplerVoice* self, bool value)
{
	if (value == FALSE)
	{
		if (psy_audio_xmsamplerchannel_foregroundvoice(
				psy_audio_xmsamplervoice_rchannel(self)) == self)			
		{
			psy_audio_xmsamplerchannel_setforegroundvoice(
				psy_audio_xmsamplervoice_rchannel(self), NULL);			
			psy_audio_xmsamplerchannel_setlastvoicepanfactor(
				psy_audio_xmsamplervoice_rchannel(self),
				self->m_PanFactor);
			psy_audio_xmsamplerchannel_setlastvoicevolume(
				psy_audio_xmsamplervoice_rchannel(self),
				self->m_Volume);
			psy_audio_xmsamplerchannel_setlastvoicerandvol(
				psy_audio_xmsamplervoice_rchannel(self),
				self->m_CurrRandVol);
		}
	}
	self->m_bPlay = value;
}

INLINE bool psy_audio_xmsamplervoice_isbackground(const psy_audio_XMSamplerVoice* self)
{
	return self->m_Background;
}

INLINE void psy_audio_xmsamplervoice_setisbackground(psy_audio_XMSamplerVoice* self, bool background)
{
	self->m_Background = background;
}

INLINE bool psy_audio_xmsamplervoice_isstopping(const psy_audio_XMSamplerVoice* self)
{
	return self->m_Stopping;
}

INLINE void psy_audio_xmsamplervoice_setisstopping(psy_audio_XMSamplerVoice* self, bool stop)
{
	self->m_Stopping = stop;
}

// Volume of the current note.
INLINE uint16_t psy_audio_xmsamplervoice_volume(const psy_audio_XMSamplerVoice* self)
{
	return self->m_Volume;
}

INLINE void psy_audio_xmsamplervoice_setvolume(psy_audio_XMSamplerVoice* self, const uint16_t vol)
{
	self->m_Volume = vol;
	//Since we have top +12dB in waveglobvolume and we have to clip randvol, we use the current globvol as top.
	//This isn't exactly what Impulse tracker did, but it's a reasonable compromise.
	float tmp_rand = 
		psy_audio_instrument_volume(psy_audio_xmsamplervoice_rinstrument(self)) *
			 self->m_CurrRandVol * self->m_WaveDataController.sample->globalvolume;
	if (tmp_rand > self->m_WaveDataController.sample->globalvolume) tmp_rand = self->m_WaveDataController.sample->globalvolume;
	self->m_RealVolume = psy_dsp_map_128_1(vol) * tmp_rand;
}

// Voice.RealVolume() returns the calculated volume out of "WaveData.WaveGlobVol() * Instrument.Volume() * Voice.NoteVolume()"
INLINE float psy_audio_xmsamplervoice_realvolume(const psy_audio_XMSamplerVoice* self)
{
	return (!self->m_bTremorMute) ? (self->m_RealVolume + self->m_TremoloAmount) : 0;
}

INLINE float psy_audio_xmsamplervoice_activevolume(const psy_audio_XMSamplerVoice* self)
{
	return psy_audio_xmsamplervoice_realvolume(self) * 
		psy_audio_xmsamplerchannel_volume(psy_audio_xmsamplervoice_rchannel_const(self)) *
		xmenvelopecontroller_modulationamount(&self->m_AmplitudeEnvelope) *
		self->m_VolumeFadeAmount;
}

INLINE float psy_audio_xmsamplervoice_currrandvol(const psy_audio_XMSamplerVoice* self)
{
	return self->m_CurrRandVol;
}

INLINE void psy_audio_xmsamplervoice_setpanfactor(psy_audio_XMSamplerVoice* self, float pan)
{
	self->m_PanFactor = pan;
	self->m_PanRange = (float)(0.5 - fabs((double)pan - 0.5));
}

INLINE float psy_audio_xmsamplervoice_panfactor(const psy_audio_XMSamplerVoice* self)
{
	return self->m_PanFactor;
}

INLINE float psy_audio_xmsamplervoice_panrange(const psy_audio_XMSamplerVoice* self)
{
	return self->m_PanRange;
}

INLINE float psy_audio_xmsamplervoice_activepan(const psy_audio_XMSamplerVoice* self)
{
	return psy_audio_xmsamplervoice_panfactor(self) + self->m_PanbrelloAmount +
		xmenvelopecontroller_modulationamount(&self->m_PanEnvelope) *
		psy_audio_xmsamplervoice_panrange(self);
}

INLINE void psy_audio_xmsamplervoice_setissurround(psy_audio_XMSamplerVoice* self, bool surround)
{
	self->m_Surround = surround;
}

INLINE bool psy_audio_xmsamplervoice_issurround(const psy_audio_XMSamplerVoice* self)
{
	return self->m_Surround;
}

INLINE int psy_audio_xmsamplervoice_activecutoff(const psy_audio_XMSamplerVoice* self)
{
	return filter_cutoff(&self->m_Filter);
}

INLINE int psy_audio_xmsamplervoice_cutoff(const psy_audio_XMSamplerVoice* self)
{
	return self->m_CutOff;
}

INLINE void psy_audio_xmsamplervoice_setcutoff(psy_audio_XMSamplerVoice* self, int co)
{
	self->m_CutOff = co;
	filter_setcutoff(&self->m_Filter, co);
//	m_FilterIT.Cutoff(co);
//	m_FilterClassic.Cutoff(co);
}

INLINE int psy_audio_xmsamplervoice_activeressonance(const psy_audio_XMSamplerVoice* self)
{
	return filter_ressonance(&self->m_Filter);	
}

INLINE int psy_audio_xmsamplervoice_ressonance(const psy_audio_XMSamplerVoice* self)
{
	return self->m_Ressonance;
}

INLINE void psy_audio_xmsamplervoice_setressonance(psy_audio_XMSamplerVoice* self, int res)
{
	self->m_Ressonance = res;
	filter_setressonance(&self->m_Filter, res);
	//m_FilterIT.Ressonance(res);
	//m_FilterClassic.Ressonance(res);
}

INLINE psy_dsp_FilterType psy_audio_xmsamplervoice_filtertype(const psy_audio_XMSamplerVoice* self)
{
	// return (self->m_Filter == NULL) ? dsp::F_NONE : m_Filter->Type();
	return filter_type(&self->m_Filter);	
}

INLINE void psy_audio_xmsamplervoice_setfiltertype(psy_audio_XMSamplerVoice* self,
	psy_dsp_FilterType ftype)
{
	filter_settype(&self->m_Filter, ftype);
}

INLINE void psy_audio_xmsamplervoice_setperiod(psy_audio_XMSamplerVoice* self, int newperiod)
{
	self->m_Period = newperiod;
	psy_audio_xmsamplervoice_updatespeed(self);
}

INLINE int psy_audio_xmsamplervoice_period(const psy_audio_XMSamplerVoice* self)
{
	return self->m_Period;
}
// convert note to period
// correctNote = true
double psy_audio_xmsamplervoice_notetoperiod(const psy_audio_XMSamplerVoice* self,
	int note, bool correctNote);
// convert period to note 
int psy_audio_xmsamplervoice_periodtonote(psy_audio_XMSamplerVoice* self,
	double period);

INLINE double psy_audio_xmsamplervoice_vibratoamount(psy_audio_XMSamplerVoice* self)
{
	return self->m_VibratoAmount;
}

int psy_audio_xmsamplervoice_samplerate(const psy_audio_XMSamplerVoice* self);

// protected:
int psy_audio_xmsamplervoice_getdelta(const psy_audio_XMSamplerVoice*,
	psy_audio_WaveForms wavetype, int wavepos);


INLINE bool psy_audio_xmsamplervoice_istremormute(const psy_audio_XMSamplerVoice* self)
{
	return self->m_bTremorMute;
}

INLINE void psy_audio_xmsamplervoice_setistremormute(psy_audio_XMSamplerVoice* self, bool value)
{
	self->m_bTremorMute = value;
}

#ifdef __cplusplus
}
#endif

#endif /* psy_audio_XMSAMPLERVOICE_H */

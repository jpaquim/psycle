// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#ifndef psy_audio_XMSAMPLER_H
#define psy_audio_XMSAMPLER_H

#include "custommachine.h"
#include "samplerdefs.h"
#include "xmsamplerchannel.h"
#include "xmsamplervoice.h"
#include "ticktimer.h"

// Internal Psycle Sampulse Sampler

// XMSampler (Sampulse)
// status:	currently not working, under construction
//			transforming code from xmsampler
// proposed changes:
//			- intrumententries instead notepairs allowing wavelayers
//			- use psy_dsp_EnvelopeSettings instead an own instrumentclass
//			- use of standard paramview instead own view
// problems: Save intrumententries instead notepairs and maintain psy3
//			 compatibility

#ifdef __cplusplus
extern "C" {
#endif

#define XM_SAMPLER_TWK_AMIGASLIDES 24
#define XM_SAMPLER_TWK_GLOBALVOLUME 0x4E

#define XM_SAMPLER_MAX_POLYPHONY 64///< max polyphony
#define XM_SAMPLER_MAX_TRACKS 64


typedef struct psy_audio_XMSampler {
	psy_audio_CustomMachine custommachine;	
	int _numVoices;

	psy_audio_XMSamplerVoice m_Voices[XM_SAMPLER_MAX_POLYPHONY];
	psy_audio_XMSamplerChannel m_Channel[XM_SAMPLER_MAX_TRACKS];
	//psycle::helpers::dsp::cubic_resampler _resampler;
	ZxxMacro zxxMap[128];	

	bool m_bAmigaSlides;// Using Linear or Amiga Slides.
	bool m_UseFilters;
	int m_GlobalVolume;
	int m_PanningMode;
	int m_TickCount;	// Current Tick number.
	int m_NextSampleTick;// The sample position of the next Tracker Tick
	int _sampleCounter;	// Number of Samples since note start
	int m_sampleRate;
	psy_List* multicmdMem; // std::vector<PatternEntry>
	// ticktimer
	psy_audio_TickTimer ticktimer;
	uintptr_t samplerowcounter;

/*	psy_List* voices;
	uintptr_t numvoices;
	int defaultspeed;	
	psy_Table lastinst;
	// psycle 0CFF, xm 0C80
	int maxvolume;
	// ps1 FALSE, sampulse TRUE
	int panpersistent;
	int xmsamplerload;
	int channelbank;
	psy_audio_CustomMachineParam param_general;
	psy_audio_IntMachineParam param_numvoices;	
	psy_audio_ChoiceMachineParam param_resamplingmethod;
	psy_audio_ChoiceMachineParam param_defaultspeed;
	psy_audio_IntMachineParam param_maxvolume;
	psy_audio_ChoiceMachineParam param_panpersistent;
	psy_audio_IntMachineParam param_instrumentbank;
	psy_audio_CustomMachineParam param_blank;
	psy_audio_InfoMachineParam param_filter_cutoff;
	psy_audio_InfoMachineParam param_filter_res;
	psy_audio_InfoMachineParam param_pan;
	psy_audio_XMSamplerChannel masterchannel;
	psy_audio_CustomMachineParam ignore_param;
	psy_audio_ChoiceMachineParam param_amigaslides;
	psy_audio_ChoiceMachineParam param_usefilters;
	psy_audio_ChoiceMachineParam param_panningmode;
	psy_audio_CustomMachineParam param_channels;
	psy_audio_ChoiceMachineParam param_channelview;	
	psy_Table channels;
	uint8_t basec;
	// Sampler PS1 with max amp = 0.5.
	psy_dsp_amp_t clipmax;
	// Instrument Bank 0: PS1 1: Sampulse
	int32_t instrumentbank;	
	psy_dsp_ResamplerQuality resamplerquality;

	int32_t amigaslides; // using linear or amiga slides.
	int32_t usefilters;
	int32_t panningmode;*/
} psy_audio_XMSampler;

void psy_audio_xmsampler_init(psy_audio_XMSampler*, psy_audio_MachineCallback*);

INLINE psy_audio_XMSampler* psy_audio_xmsampler_alloc(void)
{
	return (psy_audio_XMSampler*)malloc(sizeof(psy_audio_XMSampler));
}

INLINE psy_audio_XMSampler* psy_audio_xmsampler_allocinit(psy_audio_MachineCallback* callback)
{
	psy_audio_XMSampler* rv;

	rv = psy_audio_xmsampler_alloc();
	if (rv) {
		psy_audio_xmsampler_init(rv, callback);
	}
	return rv;
}

const psy_audio_MachineInfo* psy_audio_xmsampler_info(void);

void psy_audio_xmsampler_setsamplerate(psy_audio_XMSampler*, int sr);


psy_audio_XMSamplerVoice* psy_audio_xmsampler_getcurrentvoice(
	psy_audio_XMSampler*, int channelNum);
psy_audio_XMSamplerVoice* psy_audio_xmsampler_getfreevoice(
	psy_audio_XMSampler*, int channelNum);
int psy_audio_xmsampler_getplayingvoices(const psy_audio_XMSampler*);

/// properties
INLINE psy_audio_XMSamplerChannel* psy_audio_xmsampler_rchannel(psy_audio_XMSampler* self, int index)
{
	return &self->m_Channel[index];
}

INLINE psy_audio_XMSamplerVoice* psy_audio_xmsampler_rvoice(psy_audio_XMSampler* self, int index)
{
	return &self->m_Voices[index];
}

INLINE psy_audio_Machine* psy_audio_xmsampler_base(psy_audio_XMSampler* self)
{
	return &(self->custommachine.machine);
}

INLINE bool psy_audio_xmsampler_isamigaslides(const psy_audio_XMSampler* self)
{
	return self->m_bAmigaSlides;
}

INLINE void psy_audio_xmsampler_setisamigaslides(psy_audio_XMSampler* self, bool value)
{
	self->m_bAmigaSlides = value;
}

/// set current voice number
INLINE int psy_audio_xmsampler_numvoices(const psy_audio_XMSampler* self)
{
	return self->_numVoices;
}
/// get current voice number
INLINE void psy_audio_xmsampler_setnumvoices(psy_audio_XMSampler* self, int value)
{
	self->_numVoices = value;
}

INLINE int psy_audio_xmsampler_globalvolume(psy_audio_XMSampler* self)
{
	return self->m_GlobalVolume;
}

INLINE void psy_audio_xmsampler_setglobalvolume(psy_audio_XMSampler* self, int value)
{
	self->m_GlobalVolume = value;
}

INLINE void psy_audio_xmsampler_setslidevolume(psy_audio_XMSampler* self, int value)
{
	self->m_GlobalVolume += value;
	if (self->m_GlobalVolume > 128) self->m_GlobalVolume = 128;
	else if (self->m_GlobalVolume < 0) self->m_GlobalVolume = 0;
}

/// set resampler quality 
INLINE void psy_audio_xmsampler_setresamplerquality(psy_audio_XMSampler* self,
		psy_dsp_ResamplerQuality value)
{
//	for (int i = 0; i < MAX_POLYPHONY; i++) {
//		rVoice(i).DisposeResampleData(_resampler);
//	}
//	_resampler.quality(value);
//	for (int i = 0; i < MAX_POLYPHONY; i++) {
//		rVoice(i).RecreateResampleData(_resampler);
//	}
}

INLINE bool psy_audio_xmsampler_usefilters(const psy_audio_XMSampler* self)
{
	return self->m_UseFilters;
}

INLINE void psy_audio_xmsampler_setusefilters(psy_audio_XMSampler* self, bool usefilters)
{
	self->m_UseFilters = usefilters;
}

INLINE int psy_audio_xmsampler_panningmode(const psy_audio_XMSampler* self)
{
	return self->m_PanningMode;
}

INLINE void psy_audio_xmsampler_setpanningmode(psy_audio_XMSampler* self, int value)
{
	self->m_PanningMode = value;
}

INLINE int psy_audio_xmsampler_currenttick(const psy_audio_XMSampler* self)
{
	return (int)psy_audio_ticktimer_tickcount(&self->ticktimer);
}

void psy_audio_xmsampler_workvoices(psy_audio_XMSampler*,
	psy_audio_BufferContext*);


#ifdef __cplusplus
}
#endif

#endif /* psy_audio_XMSAMPLER_H */

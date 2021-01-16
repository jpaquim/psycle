// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#ifndef psy_audio_XMSAMPLER_H
#define psy_audio_XMSAMPLER_H

// local
#include "custommachine.h"
#include "ticktimer.h"
#include "xmsamplerchannel.h"
#include "xmsamplervoice.h"

#ifdef __cplusplus
extern "C" {
#endif

// Internal Psycle Sampulse Sampler

// XMSampler (Sampulse)
//
// todo: nodeoff doesn't set play voice to false if sample has continuous loop
//       SampleIterator(Wavedatacontroller)?, xmenvelopecontroller?,
//       samplervoice_work?

#define XM_SAMPLER_TWK_CHANNEL_START 8

#define XM_SAMPLER_TWK_AMIGASLIDES 0
#define XM_SAMPLER_TWK_GLOBALVOLUME 1
#define XM_SAMPLER_TWK_PANNINGMODE 2
#define XM_SAMPLER_TWK_SETZXXMACRO_INDEX 3
#define XM_SAMPLER_TWK_SETZXXMACRO_MODE 4
#define XM_SAMPLER_TWK_SETZXXMACRO_VALUE 5

#define XM_SAMPLER_MAX_POLYPHONY 64///< max polyphony
#define XM_SAMPLER_MAX_TRACKS 64

#define XM_SAMPLER_TWK_CHANNEL_PANNING 0x1

#define XM_CHANNELROW 2


typedef struct psy_audio_XMSampler {
	// inherits
	psy_audio_CustomMachine custommachine;
	// internal data
	int _numVoices;

	psy_audio_XMSamplerVoice m_Voices[XM_SAMPLER_MAX_POLYPHONY];
	psy_audio_XMSamplerChannel m_Channel[XM_SAMPLER_MAX_TRACKS];	
	ZxxMacro zxxMap[128];	

	int32_t m_bAmigaSlides; // Using Linear or Amiga Slides.
	int32_t m_UseFilters;
	int m_GlobalVolume;
	int m_PanningMode;	
	int m_sampleRate;
	psy_List* multicmdMem; // entry: PatternEvent*
	// ticktimer: current tick number, sample position of the next tracker tick
	//            functionpointers to buffer work and timer tick
	psy_audio_TickTimer ticktimer;	
	psy_dsp_ResamplerQuality resamplerquality;
	int32_t instrumentbank;
	// Parameters shown in the paramview
	// channelbank used as scrollbar
	// channelbank 0: [0..7] 1 : [8..15] 2 : [16..31] ..
	int channelbank;
	psy_audio_CustomMachineParam param_general;
	psy_audio_IntMachineParam param_numvoices;	
	psy_audio_ChoiceMachineParam param_resamplingmethod;
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
	// tweak only
	psy_audio_IntMachineParam tweakparam_zxxindex;
	psy_audio_IntMachineParam tweakparam_zxxmode;	
	psy_audio_IntMachineParam tweakparam_zxxvalue;
	int32_t tweak_zxxindex;
	int32_t tweak_zxxmode;	
	int32_t tweak_zxxvalue;
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

void psy_audio_xmsampler_tick(psy_audio_XMSampler*,
	uintptr_t channelNum, const psy_audio_PatternEvent*);

void psy_audio_xmsampler_setsamplerate(psy_audio_XMSampler*, int sr);


psy_audio_XMSamplerVoice* psy_audio_xmsampler_getcurrentvoice(
	psy_audio_XMSampler*, int channelNum);
psy_audio_XMSamplerVoice* psy_audio_xmsampler_getfreevoice(
	psy_audio_XMSampler*, int channelNum);
int psy_audio_xmsampler_getplayingvoices(const psy_audio_XMSampler*);

/// properties

INLINE void psy_audio_xmsampler_setzxxmacro(psy_audio_XMSampler* self, int index, int mode, int val)
{
	self->zxxMap[index].mode = mode;
	self->zxxMap[index].value = val;
}

INLINE ZxxMacro psy_audio_xmsampler_getmap(const psy_audio_XMSampler* self, int index)
{
	return self->zxxMap[index];
}

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

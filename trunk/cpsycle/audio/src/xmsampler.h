// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#ifndef psy_audio_XMSAMPLER_H
#define psy_audio_XMSAMPLER_H

#include "custommachine.h"
#include "samplerdefs.h"
#include "xmsamplerchannel.h"
#include "xmsamplervoice.h"
#include "ticktimer.h"

// Internal Psycle Sampler

// aim: make XMSampler configurable for SamplerPS1 to avoid two internal
//		samplers

#ifdef __cplusplus
extern "C" {
#endif

#define XM_SAMPLER_TWK_AMIGASLIDES 24
#define XM_SAMPLER_TWK_GLOBALVOLUME 0x4E


typedef struct psy_audio_XMSampler {
	psy_audio_CustomMachine custommachine;		
	psy_List* voices;
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
	psy_audio_TickTimer ticktimer;
	uintptr_t samplerowcounter;
	int32_t amigaslides; // using linear or amiga slides.
	int32_t usefilters;
	int32_t panningmode;
} psy_audio_XMSampler;

void psy_audio_xmsampler_init(psy_audio_XMSampler*, psy_audio_MachineCallback*);
psy_audio_XMSampler* psy_audio_xmsampler_alloc(void);
psy_audio_XMSampler* psy_audio_xmsampler_allocinit(psy_audio_MachineCallback*);
const psy_audio_MachineInfo* psy_audio_xmsampler_info(void);

INLINE psy_audio_Machine* psy_audio_xmsampler_base(psy_audio_XMSampler* self)
{
	return &(self->custommachine.machine);
}

INLINE void psy_audio_xmsampler_defaultC4(psy_audio_XMSampler* self, bool correct)
{
	self->basec = correct ? psy_audio_NOTECOMMANDS_MIDDLEC : 48;
}
			
INLINE bool psy_audio_xmsampler_isdefaultC4(psy_audio_XMSampler* self)
{
	return self->basec == psy_audio_NOTECOMMANDS_MIDDLEC;
}

void psy_audio_xmsampler_setresamplerquality(psy_audio_XMSampler* self,
	psy_dsp_ResamplerQuality quality);

INLINE psy_dsp_ResamplerQuality psy_audio_xmsampler_resamplerquality(psy_audio_XMSampler* self)
{
	return self->resamplerquality;
}

INLINE bool psy_audio_xmsampler_usefilters(psy_audio_XMSampler* self)
{
	return self->usefilters != FALSE;
}

#ifdef __cplusplus
}
#endif

#endif /* psy_audio_XMSAMPLER_H */

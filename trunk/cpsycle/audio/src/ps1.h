// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#ifndef psy_audio_PS1_H
#define psy_audio_PS1_H

#include "constants.h"
#include "custommachine.h"
#include "sampleiterator.h"
#include "samplerdefs.h"
#include "ticktimer.h"
#include "instrument.h"

#include <adsr.h>
#include <filter.h>

// Internal Psycle Sampler

// aim: make XMSampler configurable for SamplerPS1 to avoid two internal
//		samplers

#ifdef __cplusplus
extern "C" {
#endif

#define PS1_SAMPLER_MAX_POLYPHONY		16
#define PS1_SAMPLER_DEFAULT_POLYPHONY	8

#define PS1_SAMPLER_CMD_NONE			0x00
#define PS1_SAMPLER_CMD_PORTAUP			0x01
#define PS1_SAMPLER_CMD_PORTADOWN		0x02
#define PS1_SAMPLER_CMD_PORTA2NOTE		0x03
#define PS1_SAMPLER_CMD_PANNING			0x08
#define PS1_SAMPLER_CMD_OFFSET			0x09
#define PS1_SAMPLER_CMD_VOLUME			0x0C
#define PS1_SAMPLER_CMD_RETRIG			0x15
#define PS1_SAMPLER_CMD_EXTENDED		0x0E
#define PS1_SAMPLER_CMD_EXT_NOTEOFF		0xC0
#define PS1_SAMPLER_CMD_EXT_NOTEDELAY	0xD0

struct psy_audio_SamplerPS1;

typedef struct SamplerPS1Voice
{		
	struct psy_audio_SamplerPS1* sampler;
	psy_audio_SampleIterator controller;	
	psy_dsp_ADSR _envelope;
	psy_dsp_ADSRSettings _envelopesettings;
	psy_dsp_ADSR _filterEnv;
	psy_dsp_ADSRSettings _filterenvsettings;
	Filter _filter;
	psy_audio_Instrument* inst;
	int _instrument;
	int _channel;

	int _sampleCounter;		//Amount of samples since line Tick on this voice.
	int _triggerNoteOff;   //Amount of samples previous to do a delayed noteoff
	int _triggerNoteDelay;  //Amount of samples previous to do a delayed noteon (Also used for retrig)
	int _cutoff;
	float _coModify;
	int64_t _effPortaSpeed;
	// Line memory for command being executed{
	int effCmd;  //running command (like porta or retrig).
	int effVal;  //value related to the running command (like porta or retrig)
	//}
	// retrig {
	int effretTicks; // Number of ticks remaining for retrig
	float effretVol; // volume change amount
	int effretMode;  // volume change mode (multipler or sum)
	// } retrig
	// WaveDataController
	float _vol; // 0..1 value of this voice volume,
	float _pan;
	float _lVolDest;
	float _rVolDest;
	float _lVolCurr;
	float _rVolCurr;
} PS1SamplerVoice;

void ps1samplervoice_init(PS1SamplerVoice*, struct psy_audio_SamplerPS1*);
void ps1samplervoice_setup(PS1SamplerVoice*);
void ps1samplervoice_dispose(PS1SamplerVoice*);

void ps1samplervoice_noteoff(PS1SamplerVoice*);
void ps1samplervoice_noteofffast(PS1SamplerVoice*);
void ps1samplervoice_newline(PS1SamplerVoice*);
void ps1samplervoice_work(PS1SamplerVoice*, int numsamples, float* pSamplesL, float* pSamplesR);
int ps1samplervoice_tick(PS1SamplerVoice*, psy_audio_PatternEvent* pData, int channelNum, int baseC, psy_List* multicmdMem);
void ps1samplervoice_performfxold(PS1SamplerVoice*);
void ps1samplervoice_performfxnew(PS1SamplerVoice*);

typedef struct psy_audio_SamplerPS1 {
	psy_audio_CustomMachine custommachine;		
	psy_List* voices;
	uintptr_t numvoices;
	int defaultspeed;	
	psy_audio_IntMachineParam param_numvoices;	
	psy_audio_ChoiceMachineParam param_resamplingmethod;
	psy_audio_ChoiceMachineParam param_defaultspeed;
	psy_audio_IntMachineParam param_instrumentbank;	
	// Instrument Bank 0: PS1 1: Sampulse
	int32_t instrumentbank;	
	ResamplerType resamplerquality;
	psy_audio_SamplerTickTimer ticktimer;
	uintptr_t samplerowcounter;
	int32_t usefilters;
	int32_t panningmode;
	
	uint16_t lastInstrument[MAX_TRACKS];
	int _numVoices;
	PS1SamplerVoice _voices[PS1_SAMPLER_MAX_POLYPHONY];
	// psycle::helpers::dsp::cubic_resampler _resampler;
	psy_List* multicmdMem; // PatternEvent
	bool linearslide;
} psy_audio_SamplerPS1;

void psy_audio_samplerps1_init(psy_audio_SamplerPS1*, psy_audio_MachineCallback);
psy_audio_SamplerPS1* psy_audio_samplerps1_alloc(void);
psy_audio_SamplerPS1* psy_audio_samplerps1_allocinit(psy_audio_MachineCallback);
const psy_audio_MachineInfo* psy_audio_samplerps1_info(void);

INLINE psy_audio_Machine* psy_audio_samplerps1_base(psy_audio_SamplerPS1* self)
{
	return &(self->custommachine.machine);
}

INLINE void psy_audio_samplerps1_defaultC4(psy_audio_SamplerPS1* self, bool correct)
{
	self->defaultspeed = correct;
}
			
INLINE bool psy_audio_samplerps1_isdefaultC4(psy_audio_SamplerPS1* self)
{	
	return self->defaultspeed;
}

void psy_audio_samplerps1_setresamplerquality(psy_audio_SamplerPS1* self,
	ResamplerType quality);

INLINE ResamplerType psy_audio_samplerps1_resamplerquality(psy_audio_SamplerPS1* self)
{
	return self->resamplerquality;
}

INLINE bool psy_audio_samplerps1_usefilters(psy_audio_SamplerPS1* self)
{
	return self->usefilters != FALSE;
}

#ifdef __cplusplus
}
#endif

#endif /* psy_audio_PS1_H */

// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#ifndef psy_audio_PS1_H
#define psy_audio_PS1_H

#include "constants.h"
#include "custommachine.h"
#include "sampleiterator.h"
#include "ticktimer.h"
#include "instrument.h"

#include <adsr.h>
#include <filter.h>

// Internal PS1 Psycle Sampler

#ifdef __cplusplus
extern "C" {
#endif

#define PS1SAMPLERHELP\
	"Sampler Commands:""\n"\
	"\n"\
	"01xx - Portamento Up  	value FA = increases the speed in 1 / 4th of the original(wave) speed each PerformFx call(256 samples. 5ms at 44Khz).""\n"\
	"Command does not have memory(no special meaning for 00)""\n"\
	"02xx - Portamento Down  value FA = decreases the speed in 1 / 4th of the original(wave) speed each PerformFx call(256 samples. 5ms at 44Khz).""\n"\
	"Command does not have memory(no special meaning for 00)""\n"\
	"03xx - Portamento to note.value FA = increases or decreases the speed in 1 / 4th of the original(wave) speed each PerformFx call(256 samples.""\n"\
	"	5ms at 44Khz), until the corresponding note speed is reached.Command does not have memory(no special meaning for 00)""\n"\
	"	All three portamento commands aren't really song samplerate aware, since at higher song samplerates, it will slide faster.""\n"\
	"	08xx - Set panning position  0 full left, FF full right. 80 center""\n"\
	"	09xx - Set sample offset  x = 1 / 256th of the sample length.""\n"\
	"	0Cxx - Set volume  0 = silent.FF = max volume.""\n"\
	"	If using virtual instruments, the aux column can be used for the set volume command.""\n"\
	"	0ECx - Note cut in x frames(fixed 6 frames per line)""\n"\
	"	0EDx - Note delay in x frames(fixed 6 frames per line)""\n"\
	"	15xy - Retrig Command.y = Number of retrigs to do in one line(1 = do one retrig).""\n"\
	"	- x changes the volume in the following way :""\n"\
	"0 or 8 : No change""\n"\
	"1 : +1		9 : -1""\n"\
	"2 : +2		A : -2""\n"\
	"3 : +4		B : -4""\n"\
	"4 : +8		C : -8""\n"\
	"5 : +16		D : -16""\n"\
	"6 : *2 / 3		E : *3 / 2""\n"\
	"7 : *1 / 2		F : *2""\n"\
	"Note : Due to the way it is implemented, the last retrig happens almost at the next line."

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

struct psy_audio_PS1Sampler;

typedef struct psy_audio_PS1SamplerVoice
{		
	struct psy_audio_PS1Sampler* sampler;
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
} psy_audio_PS1SamplerVoice;

void psy_audio_ps1samplervoice_init(psy_audio_PS1SamplerVoice*, struct psy_audio_PS1Sampler*);
void psy_audio_ps1samplervoice_setup(psy_audio_PS1SamplerVoice*);
void psy_audio_ps1samplervoice_dispose(psy_audio_PS1SamplerVoice*);

void psy_audio_ps1samplervoice_noteoff(psy_audio_PS1SamplerVoice*);
void psy_audio_ps1samplervoice_noteofffast(psy_audio_PS1SamplerVoice*);
void psy_audio_ps1samplervoice_newline(psy_audio_PS1SamplerVoice*);
void psy_audio_ps1samplervoice_work(psy_audio_PS1SamplerVoice*, int numsamples, float* pSamplesL, float* pSamplesR);
int psy_audio_ps1samplervoice_tick(psy_audio_PS1SamplerVoice*, psy_audio_PatternEvent* pData, int channelNum, int baseC, psy_List* multicmdMem);
void psy_audio_ps1samplervoice_performfxold(psy_audio_PS1SamplerVoice*);
void psy_audio_ps1samplervoice_performfxnew(psy_audio_PS1SamplerVoice*);

typedef struct psy_audio_PS1Sampler {
	// base class
	psy_audio_CustomMachine custommachine;	
	psy_audio_IntMachineParam param_numvoices;	
	psy_audio_ChoiceMachineParam param_resamplingmethod;
	psy_audio_ChoiceMachineParam param_defaultspeed;
	psy_audio_IntMachineParam param_instrumentbank;
	// 0: basec = C3, 1: basec = C4
	int defaultspeed;
	// Instrument Bank 0: PS1 1: Sampulse
	int32_t instrumentbank;	
	ResamplerType resamplerquality;
	psy_audio_SamplerTickTimer ticktimer;
	uintptr_t samplerowcounter;
	int32_t usefilters;
	int32_t panningmode;
	
	uint16_t lastInstrument[MAX_TRACKS];
	int _numVoices;
	psy_audio_PS1SamplerVoice _voices[PS1_SAMPLER_MAX_POLYPHONY];
	// psycle::helpers::dsp::cubic_resampler _resampler;
	psy_List* multicmdMem; // PatternEvent
	bool linearslide;
} psy_audio_PS1Sampler;

void psy_audio_ps1sampler_init(psy_audio_PS1Sampler*, psy_audio_MachineCallback);
psy_audio_PS1Sampler* psy_audio_ps1sampler_alloc(void);
psy_audio_PS1Sampler* psy_audio_ps1sampler_allocinit(psy_audio_MachineCallback);
const psy_audio_MachineInfo* psy_audio_ps1sampler_info(void);

INLINE psy_audio_Machine* psy_audio_ps1sampler_base(psy_audio_PS1Sampler* self)
{
	return &(self->custommachine.machine);
}

INLINE void psy_audio_ps1sampler_defaultC4(psy_audio_PS1Sampler* self, bool correct)
{
	self->defaultspeed = correct;
}
			
INLINE bool psy_audio_ps1sampler_isdefaultC4(psy_audio_PS1Sampler* self)
{	
	return self->defaultspeed;
}

void psy_audio_ps1sampler_setresamplerquality(psy_audio_PS1Sampler* self,
	ResamplerType quality);

INLINE ResamplerType psy_audio_ps1sampler_resamplerquality(psy_audio_PS1Sampler* self)
{
	return self->resamplerquality;
}

INLINE bool psy_audio_ps1sampler_usefilters(psy_audio_PS1Sampler* self)
{
	return self->usefilters != FALSE;
}

#ifdef __cplusplus
}
#endif

#endif /* psy_audio_PS1_H */

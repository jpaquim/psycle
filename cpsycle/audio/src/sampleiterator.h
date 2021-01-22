// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#ifndef psy_audio_SAMPLEITERATOR_H
#define psy_audio_SAMPLEITERATOR_H

#include "buffer.h"
#include <multiresampler.h>

#ifdef __cplusplus
extern "C" {
#endif

/// psy_audio_Sample Loop Types
typedef enum {
	/// < Do Nothing
	psy_audio_SAMPLE_LOOP_DO_NOT = 0x0,
	/// < normal Start --> End ,Start --> End ...
	psy_audio_SAMPLE_LOOP_NORMAL = 0x1,
	/// < bidirectional Start --> End, End --> Start ...
	psy_audio_SAMPLE_LOOP_BIDI = 0x2
} psy_audio_SampleLoopType;

typedef	enum {
	psy_audio_LOOPDIRECTION_FORWARD = 0,
	psy_audio_LOOPDIRECTION_BACKWARD
} psy_audio_LoopDirection;

typedef union _Double { 
    struct {
        uint32_t LowPart;
        uint32_t HighPart; 
    };	
    uint64_t QuadPart;
} Double;


INLINE void double_setvalue(Double* self, double value)
{
	self->QuadPart = (uint64_t)(value * 4294967296.0f);
}

INLINE double double_real(Double* self)
{
	return self->QuadPart * 1 / 4294967296.0;
}

typedef psy_dsp_amp_t wavedata_t;

typedef struct {
	struct psy_audio_Sample* sample;
	Double pos;
	int64_t speed;
	// difference to speed is, that it contains direction sign
	// + forward - backward
	int64_t speedinternal;
	int forward;
	psy_dsp_MultiResampler resampler;
	psy_audio_SampleLoopType currentlooptype;
	int32_t currentloopend;
	int32_t currentloopstart;
	psy_audio_LoopDirection currentloopdirection;
	// Indicates if the playback has already looped. This is necessary for
	// properly identifying which buffer to use.
	bool looped;
	bool playing;

	wavedata_t* m_pL;
	wavedata_t* m_pR;

	wavedata_t lBuffer[64 * 3];
	wavedata_t rBuffer[64 * 3];
	//int requiredpre;  //Currently assumed to be the highest one, i.e. SINC sizes.
	//int requiredpost;
} psy_audio_WaveDataController;

void psy_audio_wavedatacontroller_init(psy_audio_WaveDataController*);
void psy_audio_wavedatacontroller_initcontroller(psy_audio_WaveDataController*, struct psy_audio_Sample*,
	psy_dsp_ResamplerQuality);
void psy_audio_wavedatacontroller_dispose(psy_audio_WaveDataController*);
psy_audio_WaveDataController* psy_audio_wavedatacontroller_alloc(void);
psy_audio_WaveDataController* psy_audio_wavedatacontroller_allocinit(
	struct psy_audio_Sample*);
intptr_t psy_audio_wavedatacontroller_inc(psy_audio_WaveDataController*);
void psy_audio_wavedatacontroller_noteoff(psy_audio_WaveDataController*);
int psy_audio_wavedatacontroller_prework(psy_audio_WaveDataController*,
	int numSamples, bool released);
wavedata_t psy_audio_sampleiterator_work(psy_audio_WaveDataController*,
	uintptr_t channel);
void psy_audio_sampleiterator_workstereo(psy_audio_WaveDataController* self,
	float* pLeftw, float* pRightw);
void psy_audio_wavedatacontroller_postwork(psy_audio_WaveDataController*);
void psy_audio_wavedatacontroller_changeloopdirection(psy_audio_WaveDataController*,
	psy_audio_LoopDirection dir);

// Properties

INLINE bool psy_audio_wavedatacontroller_playing(psy_audio_WaveDataController* self)
{
	return self->playing;
}

INLINE void psy_audio_wavedatacontroller_setplaying(
	psy_audio_WaveDataController* self, bool value)
{
	self->playing = value;
}

INLINE void psy_audio_wavedatacontroller_play(
	psy_audio_WaveDataController* self)
{
	self->playing = TRUE;
}

INLINE void psy_audio_wavedatacontroller_stop(
	psy_audio_WaveDataController* self)
{
	self->playing = FALSE;
}

// Current sample position 
INLINE uint32_t psy_audio_wavedatacontroller_position(
	psy_audio_WaveDataController* self)
{
	return self->pos.HighPart;
}

void psy_audio_wavedatacontroller_setposition(psy_audio_WaveDataController*,
	uintptr_t value);
// Current sample Speed
INLINE int64_t psy_audio_wavedatacontroller_speed(
	psy_audio_WaveDataController* self)
{
	return self->speed;
}

INLINE psy_audio_LoopDirection psy_audio_wavedatacontroller_currentloopdirection(
	psy_audio_WaveDataController* self)
{
	return self->currentloopdirection;
}

// Sets Current sample Speed
INLINE void psy_audio_wavedatacontroller_setspeed(
	psy_audio_WaveDataController* self, double value)
{
	// 4294967296 is a left shift of 32bits
	self->speed = (int64_t)(value * 4294967296.0);
	psy_dsp_resampler_setspeed(&self->resampler.resampler, value);
	self->speedinternal =
		(!self->sample ||
			psy_audio_wavedatacontroller_currentloopdirection(self) ==
			psy_audio_LOOPDIRECTION_FORWARD)
		? self->speed
		: -1 * self->speed;
}

INLINE psy_audio_LoopDirection psy_audio_wavedatacontroller_setcurrentloopdirection(
	psy_audio_WaveDataController* self, psy_audio_LoopDirection dir)
{
	self->currentloopdirection = dir;
}

void psy_audio_wavedatacontroller_dooffset(psy_audio_WaveDataController*,
	uint8_t offset);

void psy_audio_wavedatacontroller_setquality(psy_audio_WaveDataController* self, psy_dsp_ResamplerQuality quality);

void psy_audio_wavedatacontroller_setsample(psy_audio_WaveDataController*, struct psy_audio_Sample*);

uintptr_t psy_audio_wavedatacontroller_length(const
	psy_audio_WaveDataController*);

#ifdef __cplusplus
}
#endif

#endif /* psy_audio_SAMPLEITERATOR_H */

// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

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
	// psy_audio_Buffer buffer;
	psy_dsp_amp_t* lBuffer;
	psy_dsp_amp_t* rBuffer;
	psy_dsp_amp_t* m_pL;
	psy_dsp_amp_t* m_pR;
} psy_audio_SampleIterator;

void psy_audio_sampleiterator_init(psy_audio_SampleIterator*, struct psy_audio_Sample*,
	psy_dsp_ResamplerQuality);
void psy_audio_sampleiterator_dispose(psy_audio_SampleIterator*);
psy_audio_SampleIterator* psy_audio_sampleiterator_alloc(void);
psy_audio_SampleIterator* psy_audio_sampleiterator_allocinit(struct psy_audio_Sample*);
intptr_t psy_audio_sampleiterator_inc(psy_audio_SampleIterator*);
int psy_audio_sampleiterator_prework(psy_audio_SampleIterator* self, int numSamples,
	bool released);
psy_dsp_amp_t psy_audio_sampleiterator_work(psy_audio_SampleIterator*, uintptr_t channel);
void psy_audio_sampleiterator_postwork(psy_audio_SampleIterator*);

INLINE int psy_audio_sampleiterator_currentloopdirection(psy_audio_SampleIterator* self)
{
	return self->currentloopdirection;
}
// Current sample position 
INLINE uint32_t psy_audio_sampleiterator_position(psy_audio_SampleIterator* self)
{
	return self->pos.HighPart;
}

// Set Current sample position 
void psy_audio_sampleiterator_setposition(psy_audio_SampleIterator*, uintptr_t value);

// Current sample Speed
INLINE int64_t psy_audio_sampleiterator_speed(psy_audio_SampleIterator* self)
{
	return self->speed;
}
// Sets Current sample Speed
INLINE void psy_audio_sampleiterator_setspeed(psy_audio_SampleIterator* self, double value)
{
	// 4294967296 is a left shift of 32bits
	self->speed = (int64_t)(value * 4294967296.0);
	self->speedinternal =
		(!self->sample || psy_audio_sampleiterator_currentloopdirection(self) ==
				psy_audio_LOOPDIRECTION_FORWARD)
		? self->speed
		: -1 * self->speed;
	psy_dsp_resampler_setspeed(&self->resampler.resampler, value);
}

INLINE bool psy_audio_sampleiterator_playing(psy_audio_SampleIterator* self)
{
	return self->playing;
}

INLINE void psy_audio_sampleiterator_play(psy_audio_SampleIterator* self)
{
	self->playing = TRUE;
}

INLINE void psy_audio_sampleiterator_stop(psy_audio_SampleIterator* self)
{
	self->playing = FALSE;
}

void psy_audio_sampleiterator_dooffset(psy_audio_SampleIterator*,
	uint8_t offset);

void psy_audio_sampleiterator_setquality(psy_audio_SampleIterator* self, psy_dsp_ResamplerQuality quality);

void psy_audio_sampleiterator_setsample(psy_audio_SampleIterator*, struct psy_audio_Sample*);


#ifdef __cplusplus
}
#endif

#endif /* psy_audio_SAMPLEITERATOR_H */

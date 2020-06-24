// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#ifndef psy_audio_SAMPLE_H
#define psy_audio_SAMPLE_H

#include "buffer.h"
#include "sampleiterator.h"

#ifdef __cplusplus
extern "C" {
#endif

/// Wave Form Types
typedef enum {
	psy_audio_WAVEFORMS_SINUS   = 0x0,
	psy_audio_WAVEFORMS_SQUARE  = 0x1,
	psy_audio_WAVEFORMS_SAWUP   = 0x2,
	psy_audio_WAVEFORMS_SAWDOWN = 0x3,
	psy_audio_WAVEFORMS_RANDOM  = 0x4
} psy_audio_WaveForms;

typedef struct {
	// 0..255   0 means autovibrato is disabled.
	//			1 means shortest attack.
	//			255 means longest attack.
	unsigned char attack;
	// 0..64	0 no vibrato. 64 fastest vibrato
	unsigned char speed;
	// 0..32	0 no pitch change. 32 highest pitch change.
	unsigned char depth;		
	psy_audio_WaveForms type;				
} psy_audio_Vibrato;

void vibrato_init(psy_audio_Vibrato*);


typedef struct psy_audio_SampleLoop {
	uintptr_t start;
	uintptr_t end;
	psy_audio_SampleLoopType type;	
} psy_audio_SampleLoop;

INLINE void psy_audio_sampleloop_init(psy_audio_SampleLoop* self)
{
	self->start = 0;
	self->end = 0;
	self->type = psy_audio_SAMPLE_LOOP_DO_NOT;	
}

INLINE void psy_audio_sampleloop_init_all(psy_audio_SampleLoop* self,
	psy_audio_SampleLoopType type, uintptr_t loopstart, uintptr_t loopend)
{
	self->start = loopstart;
	self->end = loopend;
	self->type = type;
}

INLINE bool psy_audio_sampleloop_equal(psy_audio_SampleLoop* self,
	psy_audio_SampleLoop* other)
{
	return (self->type == other->type &&
		self->start == other->start &&
		self->end == other->end);
}

typedef struct psy_audio_Sample {
	psy_audio_Buffer channels;
	uintptr_t numframes;
	uintptr_t samplerate;
	char* name;
	/// Difference between Glob volume and defVolume is that defVolume 
	///	determines the volume if no volume is specified in the pattern, while
	/// globVolume is an attenuator for all notes of this sample.	
	psy_dsp_amp_t defaultvolume;
	/// range ( 0..4 ) (-inf to +12dB)
	psy_dsp_amp_t globalvolume;
	psy_audio_SampleLoop loop;
	psy_audio_SampleLoop sustainloop;
	/// Tuning for the center note (value that is added to the note received).
	/// values from -60 to 59.
	/// 0 = C-5 (middle C, i.e. play at original speed with note C-5);
	short tune;
	short finetune;
	float panfactor;
	int panenabled;
	unsigned char surround;
	unsigned char stereo;
	psy_audio_Vibrato vibrato;
} psy_audio_Sample;

void psy_audio_sample_init(psy_audio_Sample*, uintptr_t numchannels);
psy_audio_Sample* psy_audio_sample_alloc(void);
psy_audio_Sample* psy_audio_sample_allocinit(uintptr_t numchannels);
psy_audio_Sample* psy_audio_sample_clone(psy_audio_Sample*);
void psy_audio_sample_dispose(psy_audio_Sample*);
void psy_audio_sample_load(psy_audio_Sample*, const char* path);
void psy_audio_sample_save(psy_audio_Sample*, const char* path);
void psy_audio_sample_setname(psy_audio_Sample*, const char* name);
psy_audio_SampleIterator* psy_audio_sample_allociterator(psy_audio_Sample* self, ResamplerType quality);
const char* psy_audio_sample_name(psy_audio_Sample*);
void psy_audio_sample_setcontloop(psy_audio_Sample*, psy_audio_SampleLoopType, uintptr_t loopstart,
	uintptr_t loopend);
void psy_audio_sample_setsustainloop(psy_audio_Sample*, psy_audio_SampleLoopType, uintptr_t loopstart,
	uintptr_t loopend);
/// return attenuator for all notes of this sample. range (0..4)(-inf to +12dB)
INLINE psy_dsp_amp_t psy_audio_sample_volume(psy_audio_Sample* self)
{
	return self->globalvolume;
}

INLINE uintptr_t psy_audio_sample_numchannels(psy_audio_Sample* self)
{
	return psy_audio_buffer_numchannels(&self->channels);
}

#ifdef __cplusplus
}
#endif

#endif /* psy_audio_SAMPLE_H */

// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#ifndef psy_audio_SAMPLE_H
#define psy_audio_SAMPLE_H

#include "buffer.h"

#ifdef __cplusplus
extern "C" {
#endif

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

/// psy_audio_Sample Loop Types
typedef enum {
	LOOP_DO_NOT	= 0x0,	/// < Do Nothing
	LOOP_NORMAL	= 0x1,	/// < normal Start --> End ,Start --> End ...
	LOOP_BIDI	= 0x2	/// < bidirectional Start --> End, End --> Start ...
} LoopType;

/// Wave Form Types
typedef enum {
	WAVEFORMS_SINUS   = 0x0,
	WAVEFORMS_SQUARE  = 0x1,
	WAVEFORMS_SAWUP   = 0x2,
	WAVEFORMS_SAWDOWN = 0x3,
	WAVEFORMS_RANDOM  = 0x4
} WaveForms;

typedef struct {
	// 0..255   0 means autovibrato is disabled.
	//			1 means shortest attack.
	//			255 means longest attack.
	unsigned char attack;
	// 0..64	0 no vibrato. 64 fastest vibrato
	unsigned char speed;
	// 0..32	0 no pitch change. 32 highest pitch change.
	unsigned char depth;		
	WaveForms type;				
} Vibrato;

void vibrato_init(Vibrato*);

typedef struct {
	struct psy_audio_Sample* sample;
	Double pos;
	int64_t speed;	
	int forward;
	void* resampler_data;
} SampleIterator;

void sampleiterator_init(SampleIterator*, struct psy_audio_Sample*);
SampleIterator* sampleiterator_alloc(void);
SampleIterator* sampleiterator_allocinit(struct psy_audio_Sample*);
int sampleiterator_inc(SampleIterator*);
unsigned int sampleiterator_frameposition(SampleIterator*);

typedef struct psy_audio_SampleLoop {
	uintptr_t start;
	uintptr_t end;
	LoopType type;	
} psy_audio_SampleLoop;

INLINE void psy_audio_sampleloop_init(psy_audio_SampleLoop* self)
{
	self->start = 0;
	self->end = 0;
	self->type = LOOP_DO_NOT;	
}

INLINE void psy_audio_sampleloop_init_all(psy_audio_SampleLoop* self,
	LoopType type, uintptr_t loopstart, uintptr_t loopend)
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
	Vibrato vibrato;
} psy_audio_Sample;

void sample_init(psy_audio_Sample*, uintptr_t numchannels);
psy_audio_Sample* sample_alloc(void);
psy_audio_Sample* sample_allocinit(uintptr_t numchannels);
psy_audio_Sample* sample_clone(psy_audio_Sample*);
void sample_dispose(psy_audio_Sample*);
void sample_load(psy_audio_Sample*, const char* path);
void sample_save(psy_audio_Sample*, const char* path);
void sample_setname(psy_audio_Sample*, const char* name);
SampleIterator sample_begin(psy_audio_Sample*);
const char* sample_name(psy_audio_Sample*);
void sample_setcontloop(psy_audio_Sample*, LoopType, uintptr_t loopstart,
	uintptr_t loopend);
void sample_setsustainloop(psy_audio_Sample*, LoopType, uintptr_t loopstart,
	uintptr_t loopend);

#ifdef __cplusplus
}
#endif

#endif /* psy_audio_SAMPLE_H */

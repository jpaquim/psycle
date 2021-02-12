// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#ifndef psy_audio_WAVEIO_H
#define psy_audio_WAVEIO_H

#include "../../detail/stdint.h"
#include "../../detail/psydef.h"

#ifdef __cplusplus
extern "C" {
#endif

#define psy_audio_WAVE_FORMAT_PCM			0x00001
#define psy_audio_WAVE_FORMAT_FLOAT			0x00003
#define psy_audio_WAVE_FORMAT_EXTENSIBLE	0xFFFEU;

typedef struct { 
    uint16_t wFormatTag; 
    uint16_t nChannels; 
    uint32_t nSamplesPerSec; 
    uint32_t nAvgBytesPerSec; 
    uint16_t nBlockAlign; 
    uint16_t wBitsPerSample; 
    uint16_t cbSize; 
} psy_audio_WaveFormatChunk; 

INLINE void psy_audio_waveformatchunk_config(psy_audio_WaveFormatChunk* self,
		uint32_t NewSamplingRate,
		uint16_t NewBitsPerSample,
		uint16_t NewNumChannels,
		bool isFloat)
{	
	if (isFloat) {
		self->wFormatTag = psy_audio_WAVE_FORMAT_FLOAT;
	} else {
		self->wFormatTag = psy_audio_WAVE_FORMAT_PCM;
	}
	self->nSamplesPerSec = NewSamplingRate;
	self->nChannels = NewNumChannels;
	self->wBitsPerSample = NewBitsPerSample;
	self->nAvgBytesPerSec = self->nChannels * self->nSamplesPerSec * self->wBitsPerSample / 8;
	self->nBlockAlign = self->nChannels * self->wBitsPerSample / 8;
	self->cbSize = 0;
}


struct psy_audio_Sample;

int psy_audio_wave_load(struct psy_audio_Sample*, const char* path);
void psy_audio_wave_save(struct psy_audio_Sample*, const char* path);

#ifdef __cplusplus
}
#endif

#endif /* psy_audio_WAVEIO_H */

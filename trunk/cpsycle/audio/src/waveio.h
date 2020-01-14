// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#if !defined(PSY_AUDIO_WAVEIO_H)
#define PSY_AUDIO_WAVEIO_H

#include "sample.h"

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

void psy_audio_wave_load(psy_audio_Sample*, const char* path);
void psy_audio_wave_save(psy_audio_Sample*, const char* path);

#endif

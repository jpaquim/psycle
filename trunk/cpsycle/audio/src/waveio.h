// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#if !defined(WAVEIO_H)
#define WAVEIO_H

#include "sample.h"

typedef struct { 
    unsigned short wFormatTag; 
    unsigned short nChannels; 
    unsigned long nSamplesPerSec; 
    unsigned long nAvgBytesPerSec; 
    unsigned short nBlockAlign; 
    unsigned short wBitsPerSample; 
    unsigned short cbSize; 
} WaveFormatChunk; 

void wave_load(Sample* sample, const char* path);

#endif

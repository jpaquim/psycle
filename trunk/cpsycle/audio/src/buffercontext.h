// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#if !defined(BUFFERCONTEXT_H)
#define BUFFERCONTEXT_H

#include "buffer.h"
#include <list.h>

typedef struct {
	psy_audio_Buffer* input;
	psy_audio_Buffer* output;
	psy_List* events;
	uintptr_t numsamples;
	uintptr_t numtracks;
	struct psy_dsp_RMSVol* rmsvol;
} psy_audio_BufferContext;

void buffercontext_init(psy_audio_BufferContext*,
	psy_List* events,
	psy_audio_Buffer* input,
	psy_audio_Buffer* output,
	uintptr_t numsamples,
	uintptr_t numtracks,
	struct psy_dsp_RMSVol* rmsvol);
void buffercontext_dispose(psy_audio_BufferContext*);
uintptr_t buffercontext_numsamples(psy_audio_BufferContext*);
uintptr_t buffercontext_numtracks(psy_audio_BufferContext*);
float buffercontext_rmsvolume(psy_audio_BufferContext*);


#endif

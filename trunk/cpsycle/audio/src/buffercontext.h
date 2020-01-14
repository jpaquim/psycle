// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#if !defined(PSY_AUDIO_BUFFERCONTEXT_H)
#define PSY_AUDIO_BUFFERCONTEXT_H

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

void psy_audio_buffercontext_init(psy_audio_BufferContext*,
	psy_List* events,
	psy_audio_Buffer* input,
	psy_audio_Buffer* output,
	uintptr_t numsamples,
	uintptr_t numtracks,
	struct psy_dsp_RMSVol* rmsvol);
void psy_audio_buffercontext_dispose(psy_audio_BufferContext*);
uintptr_t psy_audio_buffercontext_numsamples(psy_audio_BufferContext*);
uintptr_t psy_audio_buffercontext_numtracks(psy_audio_BufferContext*);
float psy_audio_buffercontext_rmsvolume(psy_audio_BufferContext*);


#endif

// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#if !defined(BUFFER_H)
#define BUFFER_H

#include <dsptypes.h>
#include "../../detail/stdint.h"

typedef struct psy_audio_Buffer {
	uintptr_t numchannels;	
	psy_dsp_amp_t** samples;
	unsigned int numsamples;
	uintptr_t offset;
} psy_audio_Buffer;

void buffer_init(psy_audio_Buffer*, uintptr_t channels);
void buffer_init_shared(psy_audio_Buffer*, psy_audio_Buffer* src, uintptr_t offset);
void buffer_dispose(psy_audio_Buffer*);
psy_audio_Buffer* buffer_alloc(void);
psy_audio_Buffer* buffer_allocinit(uintptr_t channels);
void buffer_resize(psy_audio_Buffer*, uintptr_t channels);
void buffer_move(psy_audio_Buffer*, uintptr_t offset);
void buffer_setoffset(psy_audio_Buffer*, uintptr_t offset);
uintptr_t buffer_offset(psy_audio_Buffer*);
psy_dsp_amp_t* buffer_at(psy_audio_Buffer*, uintptr_t channel);
void buffer_clearsamples(psy_audio_Buffer*, uintptr_t numsamples);
void buffer_addsamples(psy_audio_Buffer*, psy_audio_Buffer* source, uintptr_t numsamples,
	float vol);
void buffer_mulsamples(psy_audio_Buffer*, uintptr_t numsamples, psy_dsp_amp_t mul);
uintptr_t buffer_numchannels(psy_audio_Buffer*);
void buffer_pan(psy_audio_Buffer* self, psy_dsp_amp_t pan, uintptr_t amount);
int buffer_mono(psy_audio_Buffer*);
void buffer_insertsamples(psy_audio_Buffer*, psy_audio_Buffer* source, uintptr_t numsamples,
	uintptr_t numsourcesamples);

#endif

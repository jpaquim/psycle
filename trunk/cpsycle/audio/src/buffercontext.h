/*
** This source is free software; you can redistribute itand /or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2, or (at your option) any later version.
** copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net
*/

#ifndef psy_audio_BUFFERCONTEXT_H
#define psy_audio_BUFFERCONTEXT_H

/* audio */
#include "buffer.h"
/* container */
#include <list.h>

#ifdef __cplusplus
extern "C" {
#endif

/* psy_audio_BufferContext
**
** aim: merges all parameters a machine needs to perform a work or similiar
**      calls. For now a pure parameter object with no additional operations.
*/

typedef struct psy_audio_BufferContext {
	psy_audio_Buffer* input;
	psy_audio_Buffer* output;
	psy_List* events;
	psy_List* outevents;
	uintptr_t numsamples;
	uintptr_t numsongtracks;
} psy_audio_BufferContext;

void psy_audio_buffercontext_init(psy_audio_BufferContext*,
	psy_List* events,
	psy_audio_Buffer* input,
	psy_audio_Buffer* output,
	uintptr_t numsamples,
	uintptr_t numsongtracks);
void psy_audio_buffercontext_dispose(psy_audio_BufferContext*);
void psy_audio_buffercontext_setoffset(psy_audio_BufferContext*,
	uintptr_t offset);

INLINE void psy_audio_buffercontext_setnumsamples(
	psy_audio_BufferContext* self, uintptr_t amount)
{
	self->numsamples = amount;
}

INLINE uintptr_t psy_audio_buffercontext_numsamples(psy_audio_BufferContext*
	self)
{
	return self->numsamples;
}

#ifdef __cplusplus
}
#endif

#endif /* psy_audio_BUFFERCONTEXT_H */

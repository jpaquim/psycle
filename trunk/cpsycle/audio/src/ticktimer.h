// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#ifndef psy_audio_TICKTIMER_H
#define psy_audio_TICKTIMER_H

#include "../../detail/psydef.h"

#ifdef __cplusplus
extern "C" {
#endif

struct psy_audio_BufferContext;

typedef void (*fp_samplerticktimer_ontick)(void* context);
typedef void (*fp_samplerticktimer_onwork)(void* context, struct psy_audio_BufferContext*);

typedef struct psy_audio_TickTimer {
	// read-only
	uintptr_t counter;
	uintptr_t samplesprotick;
	uintptr_t tickcount;
	fp_samplerticktimer_ontick tick;
	fp_samplerticktimer_onwork work;
	void* context;
} psy_audio_TickTimer;

void psy_audio_ticktimer_init(psy_audio_TickTimer*,
	void* context,
	fp_samplerticktimer_ontick,
	fp_samplerticktimer_onwork);
void psy_audio_ticktimer_reset(psy_audio_TickTimer*, uintptr_t samplesprotick);
void psy_audio_ticktimer_update(psy_audio_TickTimer*, uintptr_t amount,
	struct psy_audio_BufferContext*);

INLINE uintptr_t psy_audio_ticktimer_tickcount(const psy_audio_TickTimer* self)
{
	return self->tickcount;
}

#ifdef __cplusplus
}
#endif

#endif /* psy_audio_TICKTIMER_H */

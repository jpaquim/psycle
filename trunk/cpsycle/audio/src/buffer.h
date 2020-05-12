// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#ifndef psy_audio_BUFFER_H
#define psy_audio_BUFFER_H

#include <dsptypes.h>
#include "../../detail/psydef.h"

#include "array.h"

#ifdef __cplusplus
extern "C" {
#endif

struct psy_dsp_RMSVol;

typedef struct psy_audio_Buffer {
	psy_dsp_amp_t** samples;
	uintptr_t numchannels;
	uintptr_t numsamples;
	uintptr_t offset;
	psy_dsp_amp_range_t range;
	psy_dsp_amp_t volumedisplay;
	bool preventmixclear;
	struct psy_dsp_RMSVol* rms;
	uintptr_t writepos;
} psy_audio_Buffer;

void psy_audio_buffer_init(psy_audio_Buffer*, uintptr_t channels);
void psy_audio_buffer_init_shared(psy_audio_Buffer*, psy_audio_Buffer* src,
	uintptr_t offset);
void psy_audio_buffer_dispose(psy_audio_Buffer*);
psy_audio_Buffer* psy_audio_buffer_alloc(void);
psy_audio_Buffer* psy_audio_buffer_allocinit(uintptr_t channels);
void psy_audio_buffer_resize(psy_audio_Buffer*, uintptr_t channels);
void psy_audio_buffer_move(psy_audio_Buffer*, uintptr_t offset);
void psy_audio_buffer_setoffset(psy_audio_Buffer*, uintptr_t offset);
uintptr_t psy_audio_buffer_offset(psy_audio_Buffer*);
psy_dsp_amp_t* psy_audio_buffer_at(psy_audio_Buffer*, uintptr_t channel);
void psy_audio_buffer_clearsamples(psy_audio_Buffer*, uintptr_t numsamples);
void psy_audio_buffer_addsamples(psy_audio_Buffer*, psy_audio_Buffer* source,
	uintptr_t numsamples, float vol);
void psy_audio_buffer_mulsamples(psy_audio_Buffer*, uintptr_t numsamples,
	psy_dsp_amp_t mul);
	uintptr_t psy_audio_buffer_numchannels(psy_audio_Buffer*);
void psy_audio_buffer_pan(psy_audio_Buffer* self, psy_dsp_amp_t pan,
	uintptr_t amount);
int psy_audio_buffer_mono(psy_audio_Buffer*);
void psy_audio_buffer_insertsamples(psy_audio_Buffer*, psy_audio_Buffer* source,
	uintptr_t numsamples, uintptr_t numsourcesamples);
void psy_audio_buffer_writesamples(psy_audio_Buffer*,
	psy_audio_Buffer* source, uintptr_t numsamples, uintptr_t numsourcesamples);
void psy_audio_buffer_scale(psy_audio_Buffer*, psy_dsp_amp_range_t,
	uintptr_t numsamples);
void psy_audio_buffer_trace(psy_audio_Buffer* self, uintptr_t channel,
	uintptr_t numsamples);
void psy_audio_buffer_enablerms(psy_audio_Buffer*);
void psy_audio_buffer_disablerms(psy_audio_Buffer*);
psy_dsp_amp_t psy_audio_buffer_rmsvolume(psy_audio_Buffer*);
psy_dsp_amp_t psy_audio_buffer_rmsdisplay(psy_audio_Buffer*);
psy_dsp_amp_t psy_audio_buffer_rmsscale(psy_audio_Buffer*,
	psy_dsp_amp_t rms_volume);
psy_dsp_amp_t psy_audio_buffer_rangefactor(psy_audio_Buffer*,
	psy_dsp_amp_range_t);


#ifdef __cplusplus
}
#endif

#endif /* psy_audio_BUFFER_H */

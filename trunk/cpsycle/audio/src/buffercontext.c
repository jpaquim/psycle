// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "buffercontext.h"
#include <rms.h>
#include <math.h>

void psy_audio_buffercontext_init(psy_audio_BufferContext* self,
	psy_List* events,
	psy_audio_Buffer* input,
	psy_audio_Buffer* output,
	uintptr_t numsamples,
	uintptr_t numtracks,
	psy_dsp_RMSVol* rmsvol)
{	
	self->events = events;
	self->input = input;
	self->output = output;
	self->numsamples = numsamples;
	self->numtracks = numtracks;
	self->rmsvol = rmsvol;
}

void psy_audio_buffercontext_dispose(psy_audio_BufferContext* self)
{	
}

uintptr_t psy_audio_buffercontext_numsamples(psy_audio_BufferContext* self)
{
	return self->numsamples;
}

uintptr_t psy_audio_buffercontext_numtracks(psy_audio_BufferContext* self)
{
	return self->numtracks;
}

psy_dsp_amp_t psy_audio_buffercontext_rmsvolume(psy_audio_BufferContext* self)
{
	return self->rmsvol ? psy_dsp_rmsvol_value(self->rmsvol) : 0.f;
}

psy_dsp_amp_t psy_audio_buffercontext_volumedisplay(psy_audio_BufferContext* self)
{
	return psy_audio_buffercontext_rmsscale(self,
		psy_audio_buffercontext_rmsvolume(self));
}

psy_dsp_amp_t psy_audio_buffercontext_rmsscale(psy_audio_BufferContext* self,
	psy_dsp_amp_t rms_volume)
{
	psy_dsp_amp_t temp;

	temp = rms_volume;
	if (temp == 0.f) {
		return 0.f;
	}
	if (self->output->range == PSY_DSP_AMP_RANGE_NATIVE) {
		temp /= 32767;
	}
	temp = 50.0f * (float) log10(temp) + 100.f;
	if (temp > 97) {
		temp = 97;
	}
	return (temp > 0) ? temp / (psy_dsp_amp_t)97.f : (psy_dsp_amp_t)0.f;
}
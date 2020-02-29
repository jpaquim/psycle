// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "waveosc.h"

void psy_audio_waveosc_init(psy_audio_WaveOsc* self)
{
	psy_audio_sampler_init(&self->sampler, self->mcb);
	self->frequency = 0;
	self->basefrequency = 0;
}

void psy_audio_waveosc_dispose(psy_audio_WaveOsc* self)
{
	psy_audio_machine_dispose(psy_audio_sampler_base(
		&self->sampler));
}

void psy_audio_waveosc_work(psy_audio_WaveOsc* self, int num, float* data)
{
	psy_audio_BufferContext bc;
	psy_audio_Buffer output;

	psy_audio_buffer_init(&output, 1);
	psy_audio_buffercontext_init(&bc, 0, 0, &output, num, 0, 0);
	psy_audio_machine_work(psy_audio_sampler_base(
		&self->sampler), &bc);
	psy_audio_buffer_dispose(&output);
	psy_audio_buffercontext_dispose(&bc);
}

void psy_audio_waveosc_setfrequency(psy_audio_WaveOsc* self, float f)
{
	self->basefrequency = f;
}

void psy_audio_waveosc_start(psy_audio_WaveOsc* self, double phase)
{
}

void psy_audio_waveosc_stop(psy_audio_WaveOsc* self, double phase)
{
}

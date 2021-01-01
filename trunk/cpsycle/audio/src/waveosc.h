// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#ifndef psy_audio_WAVEOSC_H
#define psy_audio_WAVEOSC_H

#include "waveosctables.h"
#include "sampler.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct psy_audio_WaveOsc {
	psy_audio_SampleIterator sampleiterator;
	double basefrequency;
	double frequency;
	double gain;
	bool playing;
	psy_audio_WaveShape shape;
	psy_dsp_ResamplerQuality quality;
	float *pm, *fm, *am;
	float phase;
} psy_audio_WaveOsc;

void psy_audio_waveosc_init(psy_audio_WaveOsc*, psy_audio_WaveShape,
	int frequency);
void psy_audio_waveosc_dispose(psy_audio_WaveOsc*);
void psy_audio_waveosc_setfrequency(psy_audio_WaveOsc*, double frequency);

INLINE double psy_audio_waveosc_basefrequency(psy_audio_WaveOsc* self)
{
	return self->basefrequency;
}

void psy_audio_waveosc_work(psy_audio_WaveOsc*, int num, float* data);
void psy_audio_waveosc_start(psy_audio_WaveOsc*, double phase);
void psy_audio_waveosc_stop(psy_audio_WaveOsc*, double phase);

INLINE bool psy_audio_waveosc_playing(psy_audio_WaveOsc* self)
{
	return self->playing;
}

void psy_audio_waveosc_setquality(psy_audio_WaveOsc*, psy_dsp_ResamplerQuality);
psy_dsp_ResamplerQuality  psy_audio_waveosc_quality(psy_audio_WaveOsc*);

void psy_audio_waveosc_setshape(psy_audio_WaveOsc*, psy_audio_WaveShape);

INLINE psy_audio_WaveShape psy_audio_waveosc_shape(psy_audio_WaveOsc* self)
{
	return self->shape;
}

INLINE void psy_audio_waveosc_setpm(psy_audio_WaveOsc* self, float* ptr)
{
	self->pm = ptr;
}

INLINE void psy_audio_waveosc_setfm(psy_audio_WaveOsc* self, float* ptr)
{
	self->fm = ptr;
}

INLINE void psy_audio_waveosc_setam(psy_audio_WaveOsc* self, float* ptr)
{
	self->am = ptr;
}

INLINE void psy_audio_waveosc_setgain(psy_audio_WaveOsc* self, double gain)
{
	self->gain = gain;
}

INLINE double psy_audio_waveosc_gain(psy_audio_WaveOsc* self)
{
	return self->gain;
}

#ifdef __cplusplus
}
#endif

#endif /* psy_audio_WAVEOSCTABLES_H */

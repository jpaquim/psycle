// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#ifndef psy_audio_WAVEOSC_H
#define psy_audio_WAVEOSC_H

#include "waveosctables.h"
#include "sampler.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct psy_audio_WaveOsc {
	psy_audio_SamplerVoice voice;
	double basefrequency;
	double frequency;
	int playing;
} psy_audio_WaveOsc;

void psy_audio_waveosc_init(psy_audio_WaveOsc*, psy_audio_WaveShape);
void psy_audio_waveosc_dispose(psy_audio_WaveOsc*);
void psy_audio_waveosc_setfrequency(psy_audio_WaveOsc*, float f);
void psy_audio_waveosc_work(psy_audio_WaveOsc*, int num, float* data);
void psy_audio_waveosc_start(psy_audio_WaveOsc*, double phase);
void psy_audio_waveosc_stop(psy_audio_WaveOsc*, double phase);

#ifdef __cplusplus
}
#endif

#endif /* psy_audio_WAVEOSCTABLES_H */

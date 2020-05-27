// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "waveosctables.h"
#include "dsptypes.h"
#include "sample.h"

#include <math.h>
#include <operations.h>

#define MIDDLEA 69

static void psy_audio_waveosctables_makesaw(float* data, int num, int maxharmonic);
static void psy_audio_waveosctables_makesqr(float* data, int num, int maxharmonic);
static void psy_audio_waveosctables_maketri(float* data, int num, int maxharmonic);
static void psy_audio_waveosctables_makesin(float* data, int num, int maxharmonic);
static void constructtable(psy_audio_WaveOscTables*,
    psy_audio_Instrument*,
    uintptr_t i,
    psy_audio_WaveShape,
    double f_lo,
    double f_hi,
    void (*func)(float*, int, int),
    int sr);

static void constructwave(double fh, psy_audio_Sample* wave,
    void (*func)(float*, int, int),
    int sr);

void psy_audio_waveosctables_init(psy_audio_WaveOscTables* self)
{
    psy_audio_samples_init(&self->container);
    instrument_init(&self->sin_tbl);
    instrument_init(&self->saw_tbl);
    instrument_init(&self->sqr_tbl);
    instrument_init(&self->tri_tbl);
    instrument_init(&self->rnd_tbl);
    psy_audio_waveosctables_makeall(self, 44100);
}

void psy_audio_waveosctables_dispose(psy_audio_WaveOscTables* self)
{
    psy_audio_samples_dispose(&self->container);
    instrument_dispose(&self->sin_tbl);
    instrument_dispose(&self->saw_tbl);
    instrument_dispose(&self->sqr_tbl);
    instrument_dispose(&self->tri_tbl);
    instrument_dispose(&self->rnd_tbl);
}

void psy_audio_waveosctables_clear(psy_audio_WaveOscTables* self)
{
    psy_audio_waveosctables_dispose(self);
    psy_audio_waveosctables_init(self);
}

void psy_audio_waveosctables_makeall(psy_audio_WaveOscTables* self, uintptr_t sr)
{
    double f_lo;
    int i;    

    f_lo = 440 * pow(2.0, (0 - MIDDLEA) / 12.0);
    for (i = 0; i < 10; ++i) {        
        double f_hi = 2 * f_lo;
        if (i == 0) {
            f_lo = 0;
        }
        constructtable(self, &self->sqr_tbl, i, psy_audio_WAVESHAPE_SQR, f_lo, f_hi,
            psy_audio_waveosctables_makesqr, sr);
        constructtable(self, &self->saw_tbl, i, psy_audio_WAVESHAPE_SAW, f_lo, f_hi,
            psy_audio_waveosctables_makesaw, sr);
        constructtable(self, &self->sin_tbl, i, psy_audio_WAVESHAPE_SIN, f_lo, f_hi,
            psy_audio_waveosctables_makesin, sr);
        constructtable(self, &self->tri_tbl, i, psy_audio_WAVESHAPE_TRI, f_lo, f_hi,
            psy_audio_waveosctables_maketri, sr);
        f_lo = f_hi;
    }
}

void constructtable(psy_audio_WaveOscTables* self,
    psy_audio_Instrument* table,
    uintptr_t i,
    psy_audio_WaveShape shape,
    double f_lo,
    double f_hi,
    void (*func)(float*, int, int),
    int sr)
{
    psy_audio_SampleIndex index;
    psy_audio_Sample* w;
    psy_audio_InstrumentEntry* entry;

    index = sampleindex_make(shape, i);
    w = sample_allocinit(1);
    constructwave(f_hi, w, func, sr);
    psy_audio_samples_insert(&self->container, w, index);
    entry = instrumententry_allocinit();
    entry->use_keyrange = 0;
    entry->use_freqrange = 1;
    entry->freqrange.low = f_lo;
    entry->freqrange.high = f_hi;
    entry->sampleindex = index;
    instrument_addentry(table, entry);
}

void constructwave(double fh, psy_audio_Sample* wave,
    void (*func)(float*, int, int),
    int sr)
{
    uintptr_t c;
    double f = 261.6255653005986346778499935233; // C4
    int num = (int)(sr / f + 0.5);
    int hmax = (int)(sr / 2 / fh);

    for (c = 0; c < wave->channels.numchannels; ++c) {
        wave->channels.samples[c] = dsp.memory_alloc(num, sizeof(float));
        dsp.clear(wave->channels.samples[c], num);
    }
    wave->samplerate = sr;    
    wave->loop.start = 0;
    wave->loop.end = num;
    wave->loop.type = psy_audio_SAMPLE_LOOP_NORMAL;    
    func(wave->channels.samples[0], num, hmax);
}

void psy_audio_waveosctables_makesaw(float* data, int num, int maxharmonic)
{
    double gain = 0.5 / 0.777;
	int h;
	int i;

    for (h = 1; h <= maxharmonic; ++h) {
        double amplitude = gain / h;
        double to_angle = 2 * psy_dsp_PI / num * h;
        for (i = 0; i < num; ++i) {
            data[i] += (float)(sin(pow(1.0, h + 1) * i * to_angle) * amplitude);
        }
    }
}

void psy_audio_waveosctables_makesqr(float* data, int num, int maxharmonic)
{
    double gain = 0.5 / 0.777;
	int h;
	int i;

    for (h = 1; h <= maxharmonic; h = h + 2) {
        double amplitude = gain / h;
        double to_angle = 2 * psy_dsp_PI / num * h;
        for (i = 0; i < num; ++i) {
            data[i] += (float)(sin(i * to_angle) * amplitude);
        }
    }
}

void psy_audio_waveosctables_maketri(float* data, int num, int maxharmonic)
{
	int h;
    // double gain = 0.5 / 0.777;
    for (h = 1; h <= maxharmonic; h = h + 2) {
		int i;
        double to_angle = 2 * psy_dsp_PI / num * h;
        for (i = 0; i < num; ++i) {
            data[i] += (float)(pow(-1.0, (h - 1) / 2.0) / (h * h) * sin(i * to_angle));
        }
    }
}

void psy_audio_waveosctables_makesin(float* data, int num, int maxharmonic)
{
    double to_angle = 2 * psy_dsp_PI / num;
	int i;

    for (i = 0; i < num; ++i) {
        data[i] = (float)(sin(i * to_angle));
    }
}

psy_audio_Instrument* psy_audio_waveosctables_table(psy_audio_WaveOscTables* self,
    psy_audio_WaveShape shape)
{
    psy_audio_Instrument* rv;

    switch (shape) {
        case psy_audio_WAVESHAPE_SIN:
            rv = &self->sin_tbl;
        break;
        case psy_audio_WAVESHAPE_SAW:
            rv = &self->saw_tbl;
        break;
        case psy_audio_WAVESHAPE_SQR:
            rv = &self->sqr_tbl;
        break;
        case psy_audio_WAVESHAPE_TRI:
            rv = &self->sin_tbl;
        break;
        case psy_audio_WAVESHAPE_PWM:
            rv = 0;
        break;
        case psy_audio_WAVESHAPE_RND:
            rv = &self->rnd_tbl;
        break;
        default:
            rv = 0;
        break;
    }
    return rv;
}

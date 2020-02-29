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

    psy_audio_waveosctables_clear(self);
    f_lo = 440 * pow(2.0, (0 - MIDDLEA) / 12.0);
    for (i = 0; i < 10; ++i) {
        psy_audio_Sample* w;
        psy_audio_InstrumentEntry* entry;
        SampleIndex index;
        
        double f_hi = 2 * f_lo;
        if (i == 0) {
            f_lo = 0;
        }        

        // sqr
        index = sampleindex_make(SQR, i);
        w = sample_allocinit();        
        constructwave(f_hi, w, psy_audio_waveosctables_makesqr, sr);
        psy_audio_samples_insert(&self->container, w, index);
        entry = instrumententry_allocinit();        
        entry->use_freqrange = 1;
        entry->freqrange.low = f_lo;
        entry->freqrange.high = f_hi;
        entry->sampleindex = index;
        instrument_addentry(&self->sqr_tbl, entry);

        // saw
        index = sampleindex_make(SAW, i);
        w = sample_allocinit();
        constructwave(f_hi, w, psy_audio_waveosctables_makesqr, sr);
        psy_audio_samples_insert(&self->container, w, index);
        entry = instrumententry_allocinit();
        entry->use_freqrange = 1;
        entry->freqrange.low = f_lo;
        entry->freqrange.high = f_hi;
        entry->sampleindex = index;
        instrument_addentry(&self->saw_tbl, entry);
       
        // sin
        index = sampleindex_make(SIN, i);
        w = sample_allocinit();
        constructwave(f_hi, w, psy_audio_waveosctables_makesqr, sr);
        psy_audio_samples_insert(&self->container, w, index);
        entry = instrumententry_allocinit();
        entry->use_freqrange = 1;
        entry->freqrange.low = f_lo;
        entry->freqrange.high = f_hi;
        entry->sampleindex = index;
        instrument_addentry(&self->sin_tbl, entry);
        
        // tri
        index = sampleindex_make(TRI, i);
        w = sample_allocinit();
        constructwave(f_hi, w, psy_audio_waveosctables_makesqr, sr);
        psy_audio_samples_insert(&self->container, w, index);
        entry = instrumententry_allocinit();
        entry->use_freqrange = 1;
        entry->freqrange.low = f_lo;
        entry->freqrange.high = f_hi;
        entry->sampleindex = index;
        instrument_addentry(&self->tri_tbl, entry);

        f_lo = f_hi;
    }
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
    wave->loopstart = 0;
    wave->loopend = num;
    wave->looptype = LOOP_NORMAL;    
    func(wave->channels.samples[c], num, hmax);
}

void psy_audio_waveosctables_makesaw(float* data, int num, int maxharmonic)
{
    double gain = 0.5 / 0.777;
    for (int h = 1; h <= maxharmonic; ++h) {
        double amplitude = gain / h;
        double to_angle = 2 * psy_dsp_PI / num * h;
        for (int i = 0; i < num; ++i) {
            data[i] += sin(pow(1.0, h + 1) * i * to_angle) * amplitude;
        }
    }
}

void psy_audio_waveosctables_makesqr(float* data, int num, int maxharmonic)
{
    double gain = 0.5 / 0.777;
    for (int h = 1; h <= maxharmonic; h = h + 2) {
        double amplitude = gain / h;
        double to_angle = 2 * psy_dsp_PI / num * h;
        for (int i = 0; i < num; ++i) {
            data[i] += sin(i * to_angle) * amplitude;
        }
    }
}

void psy_audio_waveosctables_maketri(float* data, int num, int maxharmonic)
{
    // double gain = 0.5 / 0.777;
    for (int h = 1; h <= maxharmonic; h = h + 2) {
        double to_angle = 2 * psy_dsp_PI / num * h;
        for (int i = 0; i < num; ++i) {
            data[i] += pow(-1.0, (h - 1) / 2.0) / (h * h) * sin(i * to_angle);
        }
    }
}

void psy_audio_waveosctables_makesin(float* data, int num, int maxharmonic)
{
    double to_angle = 2 * psy_dsp_PI / num;
    for (int i = 0; i < num; ++i) {
        data[i] = sin(i * to_angle);
    }
}

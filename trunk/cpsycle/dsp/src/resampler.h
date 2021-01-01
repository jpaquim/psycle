// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#ifndef psy_dsp_RESAMPLER_H
#define psy_dsp_RESAMPLER_H

#include "dsptypes.h"

#include "../../detail/stdint.h"
#include "../../detail/psydef.h"

#ifdef __cplusplus
extern "C" {
#endif

#define CUBIC_RESOLUTION_LOG 10
#define CUBIC_RESOLUTION 2048 // 1 << CUBIC_RESOLUTION_LOG

struct psy_dsp_Resampler;

typedef void(*psy_dsp_fp_resampler_dispose)(struct psy_dsp_Resampler*);
typedef void(*psy_dsp_fp_resampler_setspeed)(struct psy_dsp_Resampler*, double speed);
/// interpolator work function types

// data = input signal to be resampled already pointing at the offset indicated by offset.
// offset = sample offset (integer) [info to avoid go out of bounds on sample reading]
// res = decimal part of the offset (between point y0 and y1) to get, as a 32bit int.
// length = sample length [info to avoid go out of bounds on sample reading]
// resampler_data = resampler specific data. Needed for sinc and sox resamplers. 
// Obtain it by calling at GetResamplerData(speed), and call at UpdateSpeed(speed) when the same sample changes speed.
// when done with the sample, call DispodeResamplerData(resampler_data)
typedef psy_dsp_amp_t (*psy_dsp_fp_resampler_work)(struct psy_dsp_Resampler*,
	const int16_t* data,
	uint64_t offset,
	uint32_t res,
	uint64_t length);
// Version without checks in data limits. Use only when you guarantee that data has enough samples for the resampling algorithm.
// data = input signal to be resampled already pointing at the integer offset.
// res = decimal part of the offset (between point y0 and y1) to get, as a 32bit int.
typedef float (*psy_dsp_fp_resampler_work_unchecked)(struct psy_dsp_Resampler*,
	int16_t const* data, uint32_t res);
// Float version with limits
// data = input signal to be resampled (pointing at the start of data)
// offset = exact sample offset including decimal part of the offset (between point y0 and y1) to get (i.e. 3.41).
// length = sample length [info to avoid go out of bounds on sample reading]
// loopBeg pointer to the start of the loop (if no loop, use same pointer than data)
// loopEnd pointer to the end of the loop (i.e. if 10 positions, then (array+9)) (if no loop use data[length-1])
typedef float (*psy_dsp_fp_resampler_work_float)(struct psy_dsp_Resampler*,
	const float* data, float offset, uint64_t length,
	const float* loopBeg, const float* loopEnd);
// Float version without checks in data limits. Use only when you guarantee that data has enough samples for the resampling algorithm.
// data = input signal to be resampled already pointing at the integer offset.
// res = decimal part of the offset (between point y0 and y1) to get, as a 32bit int.
typedef float (*psy_dsp_fp_resampler_work_float_unchecked)(struct psy_dsp_Resampler*,
	float const* data, uint32_t res);

typedef struct resampler_vtable {
	psy_dsp_fp_resampler_dispose dispose;
	psy_dsp_fp_resampler_work work;
	psy_dsp_fp_resampler_work_unchecked work_unchecked;
	psy_dsp_fp_resampler_work_float work_float;
	psy_dsp_fp_resampler_work_float_unchecked work_float_unchecked;
	psy_dsp_fp_resampler_setspeed setspeed;	
} resampler_vtable;

typedef struct psy_dsp_Resampler {
	struct resampler_vtable* vtable;
} psy_dsp_Resampler;

void psy_dsp_resampler_init(psy_dsp_Resampler*);


INLINE void psy_dsp_resampler_dispose(psy_dsp_Resampler* self)
{
	self->vtable->dispose(self);
}

INLINE void psy_dsp_resampler_setspeed(psy_dsp_Resampler* self, double speed)
{
	self->vtable->setspeed(self, speed);
}

INLINE psy_dsp_amp_t psy_dsp_resampler_work(psy_dsp_Resampler* self,
	const int16_t* data,
	uint64_t offset,
	uint32_t res,
	uint64_t length)
{
	return self->vtable->work(self, data, offset, res, length);
}

INLINE psy_dsp_amp_t psy_dsp_resampler_work_unchecked(psy_dsp_Resampler* self,
	const int16_t* data,
	uint32_t res,
	void* resampler_data)
{
	return self->vtable->work_unchecked(self, data, res);
}

INLINE float psy_dsp_resampler_work_float(psy_dsp_Resampler* self,
	const float* data,
	float offset,
	uint64_t length,
	const float* loopBeg,
	const float* loopEnd)
{
	return self->vtable->work_float(self, data, offset, length, loopBeg,
		loopEnd);
}

INLINE float psy_dsp_resampler_work_float_unchecked(psy_dsp_Resampler* self,
	float const* data, uint32_t res)
{
	return self->vtable->work_float_unchecked(self, data, res);
}

#ifdef __cplusplus
}
#endif

#endif /* psy_dsp_RESAMPLER_H */

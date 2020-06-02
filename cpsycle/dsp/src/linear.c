// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "linear.h"

#include <math.h>

extern float cubic_table[CUBIC_RESOLUTION * 4];
extern float l_table[CUBIC_RESOLUTION];

static psy_dsp_amp_t work(psy_dsp_LinearResampler*,
	int16_t const* data,
	uint64_t offset,
	uint32_t res,
	uint64_t length,
	void* /*resampler_data*/ );
static psy_dsp_amp_t work_unchecked(psy_dsp_LinearResampler*,
	int16_t const* data,
	uint32_t res,
	void* resampler_data);
static float work_float(psy_dsp_LinearResampler*,
	float const* data,
	float offset,
	uint64_t length,
	void* resampler_data,
	float const* loopBeg,
	float const* loopEnd);
static float work_float_unchecked(psy_dsp_LinearResampler*,
	float const* data, uint32_t res, void* resampler_data);

static resampler_vtable vtable;
static int vtable_initialized = 0;

static void vtable_init(psy_dsp_LinearResampler* self)
{
	if (!vtable_initialized) {
		vtable = *self->resampler.vtable;
		vtable.work = (psy_dsp_fp_resampler_work) work;		
		vtable.work_unchecked = (psy_dsp_fp_resampler_work_unchecked)
			work_unchecked;
		vtable.work_float = (psy_dsp_fp_resampler_work_float) work_float;
		vtable.work_float_unchecked =
			(psy_dsp_fp_resampler_work_float_unchecked)work_float_unchecked;
		vtable_initialized = 1;
	}
}

void psy_dsp_linearresampler_init(psy_dsp_LinearResampler* self)
{
	psy_dsp_resampler_init(&self->resampler);
	vtable_init(self);
	self->resampler.vtable = &vtable;	
}

psy_dsp_amp_t work(psy_dsp_LinearResampler* self,
	int16_t const* data,
	uint64_t offset,
	uint32_t res,
	uint64_t length,
	void* resampler_data)
{
	const float y0 = (float)*data;
	const float y1 = (float)((offset >= length-1) ? 0 : *(data + 1));
	return y0 + (y1 - y0) * l_table[res >> (32-CUBIC_RESOLUTION_LOG)];
}

psy_dsp_amp_t work_unchecked(psy_dsp_LinearResampler* self,
	int16_t const* data,
	uint32_t res,
	void* resampler_data)
{
	const float y0 = (float)*data;
	const float y1 = (float)(*(data + 1));
	return y0 + (y1 - y0) * l_table[res >> (32 - CUBIC_RESOLUTION_LOG)];
}

float work_float(psy_dsp_LinearResampler* self,
	float const* data,
	float offset,
	uint64_t length,
	void* resampler_data,
	float const* loopBeg,
	float const* loopEnd)
{
	float y0;
	float y1;
	const float foffset = (float) floor(offset);
	uint32_t res = (uint32_t)((offset - foffset) * CUBIC_RESOLUTION);
	const uint64_t ioffset = (uint64_t) foffset;
	data += ioffset;
	y0 = *data;
	y1 = (ioffset == length - 1) ? *loopBeg : *(data + 1);
	return y0 + (y1 - y0) * l_table[res];
}

float work_float_unchecked(psy_dsp_LinearResampler* self,
	float const* data, uint32_t res, void* resampler_data)
{
	const float y0 = *data;
	const float y1 = *(data + 1);
	uintptr_t debug;

	debug = res >> (32 - CUBIC_RESOLUTION_LOG);
	return y0 + (y1 - y0) * l_table[res >> (32 - CUBIC_RESOLUTION_LOG)];
}

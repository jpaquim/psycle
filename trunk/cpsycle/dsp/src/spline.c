// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "spline.h"

#include "math.h"

extern float cubic_table[CUBIC_RESOLUTION * 4];
extern float l_table[CUBIC_RESOLUTION];

static psy_dsp_amp_t work(psy_dsp_SplineResampler*,
	const int16_t* data,
	uint64_t offset,
	uint32_t res,
	uint64_t length,
	void* /*resampler_data*/ );
static psy_dsp_amp_t work_unchecked(psy_dsp_SplineResampler*,
	const int16_t* data,
	uint32_t res,
	void* resampler_data);
static float work_float(psy_dsp_SplineResampler*,
	const float* data,
	float offset,
	uint64_t length,
	void* resampler_data,
	float const* loopBeg,
	float const* loopEnd);
static float work_float_unchecked(psy_dsp_SplineResampler*,
	const float* data, uint32_t res, void* resampler_data);

static resampler_vtable vtable;
static int vtable_initialized = 0;

static void vtable_init(psy_dsp_SplineResampler* self)
{
	if (!vtable_initialized) {
		vtable = *self->resampler.vtable;
		vtable.work = (psy_dsp_fp_resampler_work)work;
		vtable.work_unchecked = (psy_dsp_fp_resampler_work_unchecked)
			work_unchecked;
		vtable.work_float = (psy_dsp_fp_resampler_work_float)work_float;
		vtable.work_float_unchecked =
			(psy_dsp_fp_resampler_work_float_unchecked)work_float_unchecked;
		vtable_initialized = 1;
	}
}

void psy_dsp_spline_resampler_init(psy_dsp_SplineResampler* self)
{
	psy_dsp_resampler_init(&self->resampler);
	vtable_init(self);
	self->resampler.vtable = &vtable;	
}

psy_dsp_amp_t work(psy_dsp_SplineResampler* self,
	const int16_t* data,
	uint64_t offset,
	uint32_t res,
	uint64_t length,
	void* resampler_data)
{		
	float yo = (float)((offset == 0) ? 0 : *(data - 1));
	float y0 = (float)(*data);
	float y1, y2;
	float* table;

	res >>= 32 - CUBIC_RESOLUTION_LOG;
	res <<= 2; // Since we have four floats in the table, the position is 16byte aligned.
	if (offset < length - 2) { y1 = data[1]; y2 = data[2]; } else if (offset < length - 1) { y1 = data[1]; y2 = 0; } else { y1 = 0; y2 = 0; }
	table = &cubic_table[res];
	return table[0] * yo + table[1] * y0 + table[2] * y1 + table[3] * y2;		
}

psy_dsp_amp_t work_unchecked(psy_dsp_SplineResampler* self,
	const int16_t* data,	
	uint32_t res,
	void* resampler_data)
{
	const float yo = (float)(*(data - 1));
	const float y0 = (float)*data;
	const float y1 = (float)(*(data + 1));
	const float y2 = (float)(*(data + 2));
	float* table;
	
	res >>= 32 - CUBIC_RESOLUTION_LOG;
	res <<= 2;//Since we have four floats in the table, the position is 16byte aligned.
	table = &cubic_table[res];
	return table[0] * yo + table[1] * y0 + table[2] * y1 + table[3] * y2;
}

float work_float(psy_dsp_SplineResampler* self,
	const float* data,
	float offset,
	uint64_t length,
	void* resampler_data,
	const float* loopBeg,
	const float* loopEnd)
{
	float yo;
	float y0;
	float y1, y2;
	float* table;
	const float foffset = (float)floor(offset);
	const int ioffset = (int)(foffset);
	uint32_t res = (uint32_t)((offset - foffset) * CUBIC_RESOLUTION);
	res <<= 2;//Since we have four floats in the table, the position is 16byte aligned.
	data += ioffset;
	yo = (ioffset == 0) ? *loopEnd : *(data - 1);
	y0 = *data;	
	if (ioffset < length - 2) { y1 = data[1]; y2 = data[2]; } else if (ioffset < length - 1) { y1 = data[1]; y2 = *loopBeg; } else { y1 = *loopBeg; y2 = *(loopBeg + 1); }
	table = &cubic_table[res];
	return table[0] * yo + table[1] * y0 + table[2] * y1 + table[3] * y2;
}

float work_float_unchecked(psy_dsp_SplineResampler* self,
	const float* data, uint32_t res, void* resampler_data)
{
	const float yo = *(data - 1);
	const float y0 = *data;
	const float y1 = *(data + 1);
	const float y2 = *(data + 2);
	float* table;
	res >>= 32 - CUBIC_RESOLUTION_LOG;
	res <<= 2;//Since we have four floats in the table, the position is 16byte aligned.	
	table = &cubic_table[res];
	return table[0] * yo + table[1] * y0 + table[2] * y1 + table[3] * y2;
}

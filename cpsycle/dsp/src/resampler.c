// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "resampler.h"

#include <math.h>

// cubic resampler tables


static int cubic_table_initialized = 0;

float cubic_table[CUBIC_RESOLUTION * 4];
float l_table[CUBIC_RESOLUTION];

static void cubic_table_initialize(void)
{
	if (!cubic_table_initialized) {
		// Cubic Resampling  ( 4point cubic spline)	
		// Initialize tables
		double const resdouble = 1.0/(double)CUBIC_RESOLUTION;
		int i;

		for(i = 0; i < CUBIC_RESOLUTION; ++i) {
			double x = (double)i * resdouble;
			//Cubic resolution is made of four table, but I've put them all in one table to optimize memory access.
			cubic_table[i*4]   = (float)(-0.5 * x * x * x +       x * x - 0.5 * x);
			cubic_table[i*4+1] = (float)( 1.5 * x * x * x - 2.5 * x * x           + 1.0);
			cubic_table[i*4+2] = (float)(-1.5 * x * x * x + 2.0 * x * x + 0.5 * x);
			cubic_table[i*4+3] = (float)( 0.5 * x * x * x - 0.5 * x * x);

			l_table[i] = (float)(x);
		}
		cubic_table_initialized = 1;
	}
}

// vtable
static void setspeed(psy_dsp_Resampler* self, void* resampler_data, double speed) { }
static void* getresamplerdata(psy_dsp_Resampler* self) { return 0; }
static void disposeresamplerdata(psy_dsp_Resampler* self, void* resampler_data) { }

static psy_dsp_amp_t work(psy_dsp_Resampler* self, 
	int16_t const* data,
	uint64_t offset,
	uint32_t res,
	uint64_t length,
	void* resampler_data)
{
	return *data;
}

static psy_dsp_amp_t work_unchecked(psy_dsp_Resampler* self,
	int16_t const* data,
	uint32_t res,
	void* resampler_data)
{
	return *data;
}


static float work_float(psy_dsp_Resampler* self,
	float const* data,
	float offset,
	uint64_t length,
	void* resampler_data,
	float const* loopBeg,
	float const* loopEnd)
{
	return data[(uintptr_t)floor(offset)];
}

static float work_float_unchecked(psy_dsp_Resampler* self,
	float const* data, uint32_t res, void* resampler_data)
{
	return *data;
}

static resampler_vtable vtable;
static int vtable_initialized = 0;

static void vtable_init(void)
{
	if (!vtable_initialized) {		
		vtable.work = work;
		vtable.work_unchecked = work_unchecked;
		vtable.work_float = work_float;
		vtable.work_float_unchecked = work_float_unchecked;
		vtable.setspeed = setspeed;
		vtable.getresamplerdata = getresamplerdata;
		vtable.disposeresamplerdata = disposeresamplerdata;
		vtable_initialized = 1;
	}
}

void psy_dsp_resampler_init(psy_dsp_Resampler* self)		
{
	vtable_init();
	self->vtable = &vtable;
	cubic_table_initialize();
}

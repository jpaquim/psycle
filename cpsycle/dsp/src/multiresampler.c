// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "multiresampler.h"

#include <math.h>

static const char* names[] = {
	"Zero Hold",
	"Linear",
	"Spline",
	"Sinc",
	"soxr"
};

static void psy_dsp_multiresampler_initresamplers(psy_dsp_MultiResampler*);

static void setspeed(psy_dsp_MultiResampler*, void* resampler_data, double speed);
static void* getresamplerdata(psy_dsp_MultiResampler*);
static void disposeresamplerdata(psy_dsp_MultiResampler*, void* resampler_data);
static psy_dsp_amp_t work(psy_dsp_MultiResampler*, 
	const int16_t* data,
	uint64_t offset,
	uint32_t res,
	uint64_t length,
	void* resampler_data);
static psy_dsp_amp_t work_unchecked(psy_dsp_MultiResampler*,
	int16_t const* data,
	uint32_t res,
	void* resampler_data);
static float work_float(psy_dsp_MultiResampler*,
	const float* data,
	float offset,
	uint64_t length,
	void* resampler_data,
	const float* loopBeg,
	const float* loopEnd);
static float work_float_unchecked(psy_dsp_MultiResampler*,
	float const* data, uint32_t res, void* resampler_data);

static resampler_vtable vtable;
static int vtable_initialized = 0;

static void vtable_init(psy_dsp_MultiResampler* self)
{
	if (!vtable_initialized) {
		vtable = *self->resampler.vtable;
		vtable.setspeed = (psy_dsp_fp_resampler_setspeed)setspeed;
		vtable.getresamplerdata = (psy_dsp_fp_resampler_getresamplerdata)getresamplerdata;
		vtable.disposeresamplerdata = (psy_dsp_fp_resampler_disposeresamplerdata)disposeresamplerdata;
		vtable.work = (psy_dsp_fp_resampler_work)work;		
		vtable.work_unchecked = (psy_dsp_fp_resampler_work_unchecked)work_unchecked;
		vtable.work_float = (psy_dsp_fp_resampler_work_float)work_float;
		vtable.work_float_unchecked = (psy_dsp_fp_resampler_work_float_unchecked)work_float_unchecked;
		vtable_initialized = 1;
	}	
}

void psy_dsp_multiresampler_init(psy_dsp_MultiResampler* self)
{
	psy_dsp_resampler_init(&self->resampler);
	vtable_init(self);
	self->resampler.vtable = &vtable;
	psy_dsp_multiresampler_initresamplers(self);
	self->selected = RESAMPLERTYPE_LINEAR;	
	psy_dsp_multiresampler_settype(self, self->selected);
}

void psy_dsp_multiresampler_initresamplers(psy_dsp_MultiResampler* self)
{
	psy_dsp_linearresampler_init(&self->linear);
#ifdef PSYCLE_USE_SSE
	psy_dsp_spline_sse2_resampler_init(&self->spline);
	psy_dsp_sinc_sse2_resampler_init(&self->sinc);
#else
	psy_dsp_splineresampler_init(&self->spline);
	psy_dsp_sincresampler_init(&self->sinc);
#endif	
}

void psy_dsp_multiresampler_settype(psy_dsp_MultiResampler* self,
	ResamplerType type)
{
	psy_dsp_Resampler* base;

	self->selected = type;
	switch (self->selected) {
		case RESAMPLERTYPE_LINEAR:
			base = psy_dsp_linearresampler_base(&self->linear);
		break;
#ifdef PSYCLE_USE_SSE	
		case RESAMPLERTYPE_SPLINE:	
			base = psy_dsp_spline_sse2_resampler_base(&self->spline);
		break;
		case RESAMPLERTYPE_SINC:
			base = psy_dsp_sinc_sse2_resampler_base(&self->sinc);
		break;
		break;
#else
		case RESAMPLERTYPE_SPLINE:
			base = psy_dsp_spline_resampler_base(&self->spline);
		break;
		case RESAMPLERTYPE_SINC:
			base = psy_dsp_sincresampler_base(&self->sinc);
		break;
#endif
		default:
			base = NULL;
		break;
	}	
	self->selectedresampler = base;
}

ResamplerType psy_dsp_multiresampler_type(psy_dsp_MultiResampler* self)
{
	return self->selected;
}

const char* psy_dsp_multiresampler_name(ResamplerType type)
{
	return names[(int)type];
}

uintptr_t psy_dsp_multiresampler_count(void)
{
	return RESAMPLERTYPE_NUMRESAMPLERS;
}

psy_dsp_amp_t work(psy_dsp_MultiResampler* self,
	int16_t const* data,
	uint64_t offset,
	uint32_t res,
	uint64_t length,
	void* resampler_data)
{
		return self->selectedresampler 
		? self->selectedresampler->vtable->work(self->selectedresampler,
		data,
		offset,
		res,
		length,
		resampler_data)
		: *data;
}

psy_dsp_amp_t work_unchecked(psy_dsp_MultiResampler* self,
	const int16_t* data,
	uint32_t res,
	void* resampler_data)
{
	return self->selectedresampler
		? self->selectedresampler->vtable->work_unchecked(self->selectedresampler,
			data,
			res,
			resampler_data)
		: *data;
}

float work_float(psy_dsp_MultiResampler* self,
	const float* data,
	float offset,
	uint64_t length,
	void* resampler_data,
	const float* loopBeg,
	const float* loopEnd)
{
	return self->selectedresampler
		? self->selectedresampler->vtable->work_float(self->selectedresampler,
			data,
			offset,
			length,
			resampler_data,
			loopBeg,
			loopEnd)
		: data[(uint64_t)floor(offset)];
}

float work_float_unchecked(psy_dsp_MultiResampler* self,
	float const* data, uint32_t res, void* resampler_data)
{
	return self->selectedresampler
		? self->selectedresampler->vtable->work_float_unchecked(self->selectedresampler,
			data,
			res,
			resampler_data)
		: *data;
}

void setspeed(psy_dsp_MultiResampler* self, void* resampler_data, double speed)
{ 
	if (self->selectedresampler) {
		self->selectedresampler->vtable->setspeed(self->selectedresampler,
			resampler_data,
			speed);
	}	
}

void* getresamplerdata(psy_dsp_MultiResampler* self)
{
	if (self->selectedresampler) {
		return self->selectedresampler->vtable->getresamplerdata(self->selectedresampler);
	}
	return 0;
}

void disposeresamplerdata(psy_dsp_MultiResampler* self, void* resampler_data)
{
	if (self->selectedresampler) {
		self->selectedresampler->vtable->disposeresamplerdata(self->selectedresampler,
			resampler_data);
	}
}

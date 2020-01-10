// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "multiresampler.h"

static const char* names[] = {
	"Zero Hold",
	"Linear",
	"Spline",
	"Sinc",
	"soxr"
};

static void setspeed(psy_dsp_MultiResampler*, int64_t speed);
static psy_dsp_amp_t work(psy_dsp_MultiResampler*, 
	float const * data,
	uint64_t offset,
	uint32_t res,
	uint64_t length,
	void* resampler_data);

static resampler_vtable vtable;
static int vtable_initialized = 0;

static void vtable_init(psy_dsp_MultiResampler* self)
{
	if (!vtable_initialized) {
		vtable = *self->resampler.vtable;
		vtable.setspeed = (psy_dsp_fp_resampler_setspeed) setspeed;
		vtable.work = (psy_dsp_fp_resampler_work) work;		
		vtable_initialized = 1;
	}	
}

void psy_dsp_multiresampler_init(psy_dsp_MultiResampler* self)
{
	psy_dsp_resampler_init(&self->resampler);
	vtable_init(self);
	self->resampler.vtable = &vtable;
	psy_dsp_linearresampler_init(&self->linear);
	self->selected = RESAMPLERTYPE_LINEAR;
	psy_dsp_multiresampler_settype(self, self->selected);
}

void psy_dsp_multiresampler_settype(psy_dsp_MultiResampler* self,
	ResamplerType type)
{
	psy_dsp_Resampler* base;

	self->selected = type;
	switch (self->selected) {
		case RESAMPLERTYPE_LINEAR:
			base = &self->linear.resampler;

		break;
		default:
			base = 0;
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
	float const * data,
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

void setspeed(psy_dsp_MultiResampler* self, int64_t speed)
{ 
	if (self->selectedresampler) {
		self->selectedresampler->vtable->setspeed(self->selectedresampler,
			speed);
	}	
}

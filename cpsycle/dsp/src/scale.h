// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#ifndef psy_dsp_SCALE_H
#define psy_dsp_SCALE_H

#include "dsptypes.h"
#include "../../detail/psydef.h"
#include <math.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct psy_dsp_Scale {
	double input_maximum_;
	double output_minimum_;
	double output_maximum_;
	double output_minimum_maximum_sum_;
	double offset_plus_input_maximum_;
	double offset_;
	double ratio_;
} psy_dsp_Scale;

INLINE psy_dsp_Scale* psy_dsp_scale_init_linear(psy_dsp_Scale* self,
	double input_maximum,
	double output_minimum,
	double output_maximum)
{
	self->input_maximum_ = input_maximum;
	self->output_minimum_ = output_minimum;
	self->output_maximum_ = output_maximum;
	self->ratio_ = (output_maximum - output_minimum) / input_maximum;
	self->offset_ = output_minimum;
	return self;
}

INLINE psy_dsp_Scale* psy_dsp_scale_init_exp(psy_dsp_Scale* self,
	double input_maximum,
	double output_minimum,
	double output_maximum)
{
	self->input_maximum_ = input_maximum;
	self->output_minimum_ = log(output_minimum);
	self->output_maximum_ = log(output_maximum);
	self->ratio_ = (self->output_maximum_ - self->output_minimum_) / input_maximum;
	self->offset_ = self->output_minimum_ / self->ratio_;
	return self;
}

INLINE psy_dsp_Scale* psy_dsp_scale_init_log(psy_dsp_Scale* self,
	double input_maximum,
	double output_minimum,
	double output_maximum)
{
	self->input_maximum_ = input_maximum;
	self->output_minimum_ = log(output_minimum);
	self->output_maximum_ = log(output_maximum);
	self->ratio_ = (self->output_maximum_ - self->output_minimum_) / input_maximum;
	self->offset_ = self->output_minimum_ / self->ratio_;
	self->offset_plus_input_maximum_ = self->offset_ + input_maximum;
	self->output_minimum_maximum_sum_ = output_minimum + output_maximum;
	return self;
}

INLINE double psy_dsp_scale_linear_apply(psy_dsp_Scale* self, double sample)
{
	return sample * self->ratio_ + self->offset_;
}

INLINE double psy_dsp_scale_linear_apply_inverse(psy_dsp_Scale* self,double sample)
{
	return (sample - self->offset_) / self->ratio_;
}

INLINE double psy_dsp_scale_exp_apply(psy_dsp_Scale* self, double sample)
{
	return exp((sample + self->offset_) * self->ratio_);
}

INLINE double psy_dsp_scale_exp_apply_inverse(psy_dsp_Scale* self, double sample)
{
	return log(sample) / self->ratio_ - self->offset_;
}

INLINE double psy_dsp_scale_log_apply(psy_dsp_Scale* self, double sample)
{
	return (self->output_minimum_maximum_sum_ - exp((self->offset_plus_input_maximum_ - sample) * self->ratio_));
}

INLINE double psy_dsp_scale_log_apply_inverse(psy_dsp_Scale* self, double sample)
{
	return (self->offset_plus_input_maximum_ - log(self->output_minimum_maximum_sum_ - sample) / self->ratio_);
}

#ifdef __cplusplus
}
#endif

#endif /* psy_dsp_SCALE_H */

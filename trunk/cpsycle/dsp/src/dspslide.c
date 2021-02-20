// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#include "dspslide.h"
// platform
#include "../../detail/portable.h"

static uintptr_t m_length = 0;
static float l_table_[2048];

void psy_dsp_slider_init(psy_dsp_Slider* self)
{
	self->m_source = 0.0f;
	self->m_target = 0.f;
	self->m_diff = 0.f;
	self->m_position = 0;
}

void psy_dsp_slider_dispose(psy_dsp_Slider* self)
{
}

uintptr_t psy_dsp_slider_getlength(psy_dsp_Slider* self)
{
	return m_length;
}

void psy_dsp_slider_setlength(psy_dsp_Slider* self, uintptr_t length)
{
	length= psy_min((uintptr_t)2048, length);
	if (m_length != length) {
		double const resdouble = 1.0 / (double)length;
		uintptr_t i;
        for(i = 0; i <= length; ++i) {
			l_table_[i] = (float)((double)i * resdouble);
		}
		m_length=length;
	}
}
	
void psy_dsp_slider_resetto(psy_dsp_Slider* self, float target)
{
	self->m_source= self->m_target=target;
	self->m_diff=0.f;
	self->m_position=0;
}

void psy_dsp_slider_settarget(psy_dsp_Slider* self, float target)
{
	if (self->m_target != target) {
		if (self->m_position >= m_length) {
			self->m_position=0;
			self->m_source= self->m_target;
		}
		self->m_target=target;
		self->m_diff= self->m_target- self->m_source;
	}
}

float psy_dsp_slider_getnext(psy_dsp_Slider* self)
{
	if (self->m_position < m_length) {
		const float m_lastVal = self->m_source + self->m_diff * l_table_[self->m_position];
		self->m_position++;
		return m_lastVal;
	} else {
		return self->m_target;
	}
}

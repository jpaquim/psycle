// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#ifndef psy_dsp_SLIDE_H
#define psy_dsp_SLIDE_H

#include "../../detail/psydef.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct psy_dsp_Slider {
	float m_source;
	float m_target;
	float m_diff;
	uintptr_t m_position;	
} psy_dsp_Slider;

void psy_dsp_slider_init(psy_dsp_Slider*);
void psy_dsp_slider_dispose(psy_dsp_Slider*);
//////////////////////////////////////////////////////////////////
//				Get / Set Methods
//////////////////////////////////////////////////////////////////
uintptr_t psy_dsp_slider_getlength(psy_dsp_Slider* self);
void psy_dsp_slider_setlength(psy_dsp_Slider*, uintptr_t length);

INLINE float psy_dsp_slider_gettarget(psy_dsp_Slider* self)
{
	return self->m_target;
}

void psy_dsp_slider_settarget(psy_dsp_Slider*, float target);
//////////////////////////////////////////////////////////////////
//				Methods
//////////////////////////////////////////////////////////////////
void psy_dsp_slider_resetto(psy_dsp_Slider* self, float target);

float psy_dsp_slider_getnext(psy_dsp_Slider* self);

#ifdef __cplusplus
}
#endif

#endif /* psy_dsp_SLIDE_H */

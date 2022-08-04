/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#if !defined(PIANOKEYBOARDSTATE_H)
#define PIANOKEYBOARDSTATE_H

/* audio */
#include <pattern.h>
/* ui */
#include <uidef.h>
#include <uigeometry.h>
/* dsp */
#include <notestab.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct KeyboardState {
	uint8_t keymin;
	uint8_t keymax;
	psy_dsp_NotesTabMode notemode;
	bool drawpianokeys;
	psy_ui_Value key_extent;
	double key_extent_px;
	double keyboard_extent_px;
	psy_ui_Value default_key_extent;
	psy_ui_Orientation orientation;	
} KeyboardState;

void keyboardstate_init(KeyboardState*, psy_ui_Orientation);

INLINE intptr_t keyboardstate_num_keys(const KeyboardState* self)
{
	assert(self);

	return self->keymax - self->keymin;
}

INLINE double keyboardstate_extent(const KeyboardState* self,
	const psy_ui_TextMetric* tm)
{
	assert(self);

	return keyboardstate_num_keys(self) * psy_ui_value_px(&self->key_extent, tm,
		NULL);
}

INLINE double keyboardstate_key_to_px(KeyboardState* self, intptr_t key)
{
	assert(self);

	if (self->orientation == psy_ui_VERTICAL) {
		return self->keyboard_extent_px - (key + 1) * self->key_extent_px;
	}
	return (key + 1) * self->key_extent_px;
}

INLINE uint8_t keyboardstate_screen_to_key(KeyboardState* self,
	psy_ui_RealPoint pt, double extent)
{
	uint8_t rv;
	assert(self);

	rv = 0;
	if (self->orientation == psy_ui_HORIZONTAL) {
		double swap;
		
		swap = pt.x;
		pt.x = pt.y;
		pt.y = swap;
	}
	if (self->keymax - 1 >= pt.y / self->key_extent_px) {
		if (self->orientation == psy_ui_VERTICAL) {
			rv = (uint8_t)(self->keymax - pt.y / self->key_extent_px);
		} else {
			rv = (uint8_t)(pt.y / self->key_extent_px) - 1;
		}
		rv = psy_min(self->keymax, rv);
		rv = psy_max(self->keymin, rv);
	}
	if (rv != psy_audio_NOTECOMMANDS_EMPTY && psy_dsp_isblack(rv)) {				
		if (pt.x > (extent * 0.60)) {
			double cp;

			cp = keyboardstate_key_to_px(self, rv) + (self->key_extent_px / 2);
			if (pt.y > cp) {
				if (rv > self->keymin) {
					rv--;
				}
			} else {
				if (rv < self->keymax) {
					rv++;
				}
			}
		}
	}
	return rv;
}

INLINE void pianokeyboardstate_clip(KeyboardState* self,
	double clip_top_px, double clip_bottom_px,
	uint8_t* rv_keymin, uint8_t* rv_keymax)
{
	assert(self);	
	
	*rv_keymin = (uint8_t)psy_max((int16_t)self->keymin, (int16_t)
		((self->keyboard_extent_px - clip_bottom_px) / self->key_extent_px - 1));
	*rv_keymax = (uint8_t)psy_max(0, psy_min((int16_t)self->keymax, (int16_t)
		((self->keyboard_extent_px - clip_top_px + self->key_extent_px) /
		self->key_extent_px)));	
}

INLINE void keyboardstate_update_metrics(KeyboardState* self,
	const psy_ui_TextMetric* tm)
{
	assert(self);

	self->key_extent_px = psy_ui_value_px(&self->key_extent, tm, NULL);
	self->keyboard_extent_px = keyboardstate_extent(self, tm);
}

#ifdef __cplusplus
}
#endif

#endif /* PIANOKEYBOARDSTATE_H */

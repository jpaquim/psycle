/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#if !defined(PIANOKEYBOARDSTATE_H)
#define PIANOKEYBOARDSTATE_H

/* audio */
#include <pattern.h>
/* ui */
#include <uivalue.h>
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
	psy_ui_Value keyheight;
	double keyheightpx;
	double keyboardheightpx;
	psy_ui_Value defaultkeyheight;	
} KeyboardState;

void keyboardstate_init(KeyboardState*);

INLINE intptr_t keyboardstate_numkeys(KeyboardState* self)
{
	assert(self);

	return self->keymax - self->keymin;
}

INLINE double keyboardstate_height(KeyboardState* self,
	const psy_ui_TextMetric* tm)
{
	assert(self);

	return keyboardstate_numkeys(self) *
		psy_ui_value_px(&self->keyheight, tm, NULL);
}

INLINE double keyboardstate_keytopx(KeyboardState* self, intptr_t key)
{
	assert(self);

	return self->keyboardheightpx - (key + 1) * self->keyheightpx;
}

INLINE uint8_t keyboardstate_screen_to_key(KeyboardState* self, psy_ui_RealPoint pt,
	double width)
{
	uint8_t rv;
	assert(self);

	rv = 0;
	if (self->keymax - 1 >= pt.y / self->keyheightpx) {		
		rv = (uint8_t)(self->keymax - pt.y / self->keyheightpx);
	}
	if (rv != psy_audio_NOTECOMMANDS_EMPTY && psy_dsp_isblack(rv)) {				
		if (pt.x > (width * 0.60)) {
			double cpy;

			cpy = keyboardstate_keytopx(self, rv) + (self->keyheightpx / 2);
			if (pt.y > cpy) {
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
		((self->keyboardheightpx - clip_bottom_px) / self->keyheightpx - 1));
	*rv_keymax = (uint8_t)psy_max(0, psy_min((int16_t)self->keymax, (int16_t)
		((self->keyboardheightpx - clip_top_px + self->keyheightpx) /
		self->keyheightpx)));	
}

INLINE void keyboardstate_update_metrics(KeyboardState* self,
	const psy_ui_TextMetric* tm)
{
	assert(self);

	self->keyheightpx = psy_ui_value_px(
		&self->keyheight, tm, NULL);
	self->keyboardheightpx = keyboardstate_height(self, tm);
}

#ifdef __cplusplus
}
#endif

#endif /* PIANOKEYBOARDSTATE_H */

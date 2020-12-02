// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#if !defined(ZOOMBOX_H)
#define ZOOMBOX_H

// ui
#include <uibutton.h>
#include <uilabel.h>

// ZoomBox
//
// Displays and sets a zoom factor with buttons or wheel scroll

#ifdef __cplusplus
extern "C" {
#endif

typedef struct ZoomBox {
	// inherits
	psy_ui_Component component;
	// ui elements
	psy_ui_Button zoomin;
	psy_ui_Label label;
	psy_ui_Button zoomout;
	// data members
	double zoomrate;
	double zoomstep;
	double minrate;
	double maxrate;
	// Signal
	psy_Signal signal_changed;
} ZoomBox;

void zoombox_init(ZoomBox*, psy_ui_Component* parent);

void zoombox_setrate(ZoomBox*, double);

INLINE double zoombox_rate(const ZoomBox* self)
{
	assert(self);

	return self->zoomrate;
}

void zoombox_setstep(ZoomBox*, double);

// sets the step, the rate is inc-/decremented
INLINE double zoombox_step(const ZoomBox* self)
{
	assert(self);

	return self->zoomstep;
}

// sets a min and maxrange (including maxval)
INLINE void zoombox_setrange(ZoomBox* self, double minval, double maxval)
{
	assert(self);

	self->minrate = minval;
	self->maxrate = psy_max(maxval, minval);
}

INLINE void zoombox_range(const ZoomBox* self, double* rv_minval, double* rv_maxval)
{
	assert(self);

	*rv_minval = self->minrate;
	*rv_maxval = self->maxrate;
}

INLINE psy_ui_Component* zoombox_base(ZoomBox* self)
{
	assert(self);

	return &self->component;
}

#ifdef __cplusplus
}
#endif

#endif /* ZOOMBOX_H */

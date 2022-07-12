/*
** This source is free software; you can redistribute itand /or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#if !defined(ZOOMBOX_H)
#define ZOOMBOX_H

/* ui */
#include <uibutton.h>
#include <uitextarea.h>

/*
** ZoomBox
**
** Displays and sets a zoom factor with buttons or wheel scroll
*/

#ifdef __cplusplus
extern "C" {
#endif

typedef struct ZoomBox {
	/* inherits */
	psy_ui_Component component;
	/* Signal */
	psy_Signal signal_changed;
	/* internal */
	psy_ui_Button zoomin;
	psy_ui_TextArea zoom;
	psy_ui_Button zoomout;	
	double zoomrate;
	double zoomstep;
	psy_RealPair range;
	/* references */
	psy_Property* property;
} ZoomBox;

void zoombox_init(ZoomBox*, psy_ui_Component* parent);
void zoombox_init_connect(ZoomBox*, psy_ui_Component* parent,
	void* context, void* fp);
void zoombox_init_exchange(ZoomBox*, psy_ui_Component* parent,
	psy_Property*);

ZoomBox* zoombox_alloc(void);
ZoomBox* zoombox_allocinit(psy_ui_Component* parent);
ZoomBox* zoombox_allocinit_exchange(psy_ui_Component* parent, psy_Property*);

void zoombox_data_exchange(ZoomBox*, psy_Property*);

void zoombox_set_rate(ZoomBox*, double);

INLINE double zoombox_rate(const ZoomBox* self)
{
	assert(self);

	return self->zoomrate;
}

/* sets the step, the rate is inc-/decremented */
void zoombox_set_step(ZoomBox*, double);
INLINE double zoombox_step(const ZoomBox* self)
{
	assert(self);

	return self->zoomstep;
}

/* sets min/max-range (including maxval) */
INLINE void zoombox_set_range(ZoomBox* self, psy_RealPair range)
{
	assert(self);

	self->range = range;	
	self->range.second = psy_max(self->range.first, self->range.second);
}

INLINE psy_RealPair zoombox_range(const ZoomBox* self)
{
	assert(self);

	return self->range;
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

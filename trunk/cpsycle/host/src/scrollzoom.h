/*
** This source is free software; you can redistribute itand /or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#if !defined(SCROLLZOOM_H)
#define SCROLLZOOM_H

/* ui */
#include <uicomponent.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
** ScrollZoom
**
** Scrollbar with zoom
*/

typedef struct ScrollZoom {
	/* inherits */
	psy_ui_Component component;
	/* signals */
	psy_Signal signal_zoom;
	psy_Signal signal_customdraw;
	/* internal */
	double start;
	double end;
	int dragmode;
	double dragoffset;	
} ScrollZoom;

void scrollzoom_init(ScrollZoom*, psy_ui_Component* parent);

INLINE double scrollzoom_start(ScrollZoom* self)
{
	return self->start;
}

INLINE double scrollzoom_end(ScrollZoom* self)
{
	return self->end;
}

INLINE psy_ui_Component* scrollzoom_base(ScrollZoom* self)
{
	return &self->component;
}

#ifdef __cplusplus
}
#endif

#endif /* SCROLLZOOM_H */

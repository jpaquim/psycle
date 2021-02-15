// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#if !defined(MINIVIEW_H)
#define MINIVIEW_H

//ui
#include <uicomponent.h>

#ifdef __cplusplus
extern "C" {
#endif

// Scrollbar with zoom

typedef struct MiniView {
	// inherits
	psy_ui_Component component;
	// signals
	psy_Signal signal_zoom;
	psy_Signal signal_customdraw;
	// internal data
	double start;
	double end;
	int dragmode;
	double dragoffset;
	uintptr_t opcount;	
	psy_ui_Bitmap bitmap;
	psy_ui_Bitmap scaledbitmap;
	volatile bool threadrunning;
	// references
	psy_ui_Component* view;
} MiniView;

void miniview_init(MiniView*, psy_ui_Component* parent);

INLINE double miniview_start(MiniView* self)
{
	return self->start;
}

INLINE double miniview_end(MiniView* self)
{
	return self->end;
}

void miniview_setview(MiniView* self, psy_ui_Component* view);

INLINE psy_ui_Component* miniview_base(MiniView* self)
{
	return &self->component;
}

#ifdef __cplusplus
}
#endif

#endif /* MINIVIEW_H */

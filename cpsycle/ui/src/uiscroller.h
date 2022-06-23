/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
**  copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net
*/

#ifndef psy_ui_SCROLLER_H
#define psy_ui_SCROLLER_H

/* local */
#include "uiscrollbar.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {	
	double speed;
	double targetpx;	
	double steppx;
	uintptr_t counter;	
} psy_ui_ScrollAnimate;

typedef struct psy_ui_Scroller {
	/* inherits */
	psy_ui_Component component;	
	/* internal */
	psy_ui_ScrollBar vscroll_intern;
	psy_ui_ScrollBar* vscroll;	
	psy_ui_ScrollBar hscroll_intern;
	psy_ui_ScrollBar* hscroll;	
	psy_ui_Component pane;
	psy_ui_Component* client;	
	psy_ui_ScrollAnimate hanimate;
	psy_ui_ScrollAnimate vanimate;
	bool smooth;
	bool thumbmove;	
	bool vscroll_autohide;
	bool hscroll_autohide;
	bool prevent_mouse_down_propagation;
} psy_ui_Scroller;

void psy_ui_scroller_init(psy_ui_Scroller*, psy_ui_Component* parent,
	psy_ui_ScrollBar* hscroll, psy_ui_ScrollBar* vscroll);

void psy_ui_scroller_set_client(psy_ui_Scroller*, psy_ui_Component*);

INLINE void psy_ui_scroller_scroll_smooth(psy_ui_Scroller* self)
{
	self->smooth = TRUE;
}

INLINE void psy_ui_scroller_scroll_fast(psy_ui_Scroller* self)
{
	self->smooth = FALSE;
}

INLINE psy_ui_Component* psy_ui_scroller_pane(psy_ui_Scroller* self)
{
	return &self->pane;
}

INLINE psy_ui_Component* psy_ui_scroller_base(psy_ui_Scroller* self)
{
	return &self->component;
}

#ifdef __cplusplus
}
#endif

#endif /* psy_ui_SCROLLBAR_H */

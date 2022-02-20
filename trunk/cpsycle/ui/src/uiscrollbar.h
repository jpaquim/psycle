/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http:/*psycle.sourceforge.net
*/

#ifndef psy_ui_SCROLLBAR_H
#define psy_ui_SCROLLBAR_H

/* local */
#include "uibutton.h"

/* Scrollbar
**
** displays a scrollbar
**
**                              psy_ui_Component
**                                     ^
**                                     |
**      -------------------------------------------------------------
**      |                              |                            |
** psy_ui_Scrollbar <>----- psy_ui_ScrollBarPane <>----- psy_ui_Component (Thumb)
**                  <>----- psy_ui_Button (less)
**                  <>----- psy_ui_Button (more)
*/

#ifdef __cplusplus
extern "C" {
#endif

/* psy_ui_ScrollBarPane */
/* todo combine with sliderpane */
typedef struct psy_ui_ScrollBarPane {
    /* inherits */
    psy_ui_Component component;
    /* signals */
    psy_Signal signal_changed;
    /* internal */
    psy_ui_Component thumb;
    double position;
    double screenpos;    
    psy_ui_IntPoint scrollrange;
    double dragoffset;    
    int repeat;
    int repeatdelaycounter;
    psy_ui_Orientation orientation;     
} psy_ui_ScrollBarPane;

void psy_ui_scrollbarpane_init(psy_ui_ScrollBarPane*,
    psy_ui_Component* parent);

void psy_ui_scrollbarpane_setorientation(psy_ui_ScrollBarPane*,
    psy_ui_Orientation);

INLINE double psy_ui_scrollbarpane_position(const
    psy_ui_ScrollBarPane* self)
{
    assert(self);

    return self->position;
}

INLINE void psy_ui_scrollbarpane_setscrollrange(psy_ui_ScrollBarPane* self,
    psy_ui_IntPoint range)
{
    assert(self);

    self->scrollrange = range;
    self->position = (double)range.x;
}

INLINE psy_ui_IntPoint psy_ui_scrollbarpane_scrollrange(const
    psy_ui_ScrollBarPane* self)
{
    assert(self);

    return self->scrollrange;
}

void psy_ui_scrollbarpane_setthumbposition(psy_ui_ScrollBarPane*, double pos);

INLINE psy_ui_Component* psy_ui_scrollbarpane_base(
    psy_ui_ScrollBarPane* self)
{
    return &self->component;
}

/* psy_ui_ScrollBar */
typedef struct psy_ui_ScrollBar {
    /* inherits */
    psy_ui_Component component;
    /* signals */
    psy_Signal signal_changed;    
    /* internal */    
    psy_ui_Button less;
    psy_ui_Button more;
    psy_ui_ScrollBarPane pane;    
} psy_ui_ScrollBar;

void psy_ui_scrollbar_init(psy_ui_ScrollBar*, psy_ui_Component* parent);

void psy_ui_scrollbar_setorientation(psy_ui_ScrollBar*, psy_ui_Orientation);

INLINE void psy_ui_scrollbar_setthumbposition(psy_ui_ScrollBar* self,
    double pos)
{
    assert(self);

    psy_ui_scrollbarpane_setthumbposition(&self->pane, pos);
}

INLINE double psy_ui_scrollbar_position(psy_ui_ScrollBar* self)
{
    assert(self);

    return psy_ui_scrollbarpane_position(&self->pane);
}

INLINE void psy_ui_scrollbar_setscrollrange(psy_ui_ScrollBar* self,
    psy_ui_IntPoint range)
{
    assert(self);

    psy_ui_scrollbarpane_setscrollrange(&self->pane, range);
}

INLINE psy_ui_IntPoint psy_ui_scrollbar_scrollrange(
    const psy_ui_ScrollBar* self)
{
    assert(self);

    return psy_ui_scrollbarpane_scrollrange(&self->pane);
}

INLINE psy_ui_Component* psy_ui_scrollbar_base(psy_ui_ScrollBar* self)
{
    return &self->component;
}

#ifdef __cplusplus
}
#endif

#endif /* psy_ui_SCROLLBAR_H */

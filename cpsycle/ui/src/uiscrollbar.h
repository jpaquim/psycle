// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#ifndef psy_ui_SCROLLBAR_H
#define psy_ui_SCROLLBAR_H

#include "uibutton.h"

// Scrollbar
//
// displays a scrollbar
//
//                              psy_ui_Component 
//                                     ^
//                                     |
//      -------------------------------------------------------------
//      |                              |                            |
// psy_ui_Scrollbar <>----- psy_ui_ScrollBarPane <>----- psy_ui_ScrollBarThumb

#ifdef __cplusplus
extern "C" {
#endif

// psy_ui_ScrollBarState
typedef struct psy_ui_ScrollBarState {
    bool dragthumb;
} psy_ui_ScrollBarState;

void psy_ui_scrollbarstate_init(psy_ui_ScrollBarState*);

// psy_ui_ScrollBarThumb
typedef struct psy_ui_ScrollBarThumb {
    // inherits
    psy_ui_Component component;
    // internal
    // references
    psy_ui_ScrollBarState* state;
} psy_ui_ScrollBarThumb;

void psy_ui_scrollbarthumb_init(psy_ui_ScrollBarThumb*,
    psy_ui_Component* parent, psy_ui_Component* view, psy_ui_ScrollBarState*);

INLINE psy_ui_Component* psy_ui_scrollbarthumb_base(
    psy_ui_ScrollBarThumb* self)
{
    return &self->component;
}

// todo combine with sliderpane
typedef struct psy_ui_ScrollBarPane {
    // inherits
    psy_ui_Component component;
    // signals
    psy_Signal signal_changed;
    psy_Signal signal_clicked;
    // internal
    psy_ui_ScrollBarThumb thumb;
    double pos;
    double screenpos;
    psy_ui_IntPoint scrollrange;
    double dragoffset;    
    bool enabled;    
    int repeat;
    int repeatdelaycounter;
    psy_ui_Orientation orientation; 
    // references
    psy_ui_ScrollBarState* state;
} psy_ui_ScrollBarPane;

void psy_ui_scrollbarpane_init(psy_ui_ScrollBarPane*,
    psy_ui_Component* parent, psy_ui_Component* view, psy_ui_ScrollBarState*);
void psy_ui_scrollbarpane_setorientation(psy_ui_ScrollBarPane*,
    psy_ui_Orientation);

INLINE psy_ui_Component* psy_ui_scrollbarpane_base(
    psy_ui_ScrollBarPane* self)
{
    return &self->component;
}

typedef struct psy_ui_ScrollBar {
    // inherits
    psy_ui_Component component;
    // signals
    psy_Signal signal_changed;
    psy_Signal signal_clicked;
    /// internal
    // ui elements
    psy_ui_Button less;
    psy_ui_Button more;
    psy_ui_ScrollBarPane pane;    
    psy_ui_ScrollBarState state;
} psy_ui_ScrollBar;

void psy_ui_scrollbar_init(psy_ui_ScrollBar*, psy_ui_Component* parent,
    psy_ui_Component* view);
void psy_ui_scrollbar_setorientation(psy_ui_ScrollBar*, psy_ui_Orientation);
double psy_ui_scrollbar_position(psy_ui_ScrollBar*);
void psy_ui_scrollbar_setscrollrange(psy_ui_ScrollBar*, psy_ui_IntPoint range);
psy_ui_IntPoint psy_ui_scrollbar_scrollrange(const psy_ui_ScrollBar*);
void psy_ui_scrollbar_setthumbposition(psy_ui_ScrollBar*, double pos);

INLINE psy_ui_Component* psy_ui_scrollbar_base(psy_ui_ScrollBar* self)
{
    return &self->component;
}

#ifdef __cplusplus
}
#endif

#endif /* psy_ui_SCROLLBAR_H */

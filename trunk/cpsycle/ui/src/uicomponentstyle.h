/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#ifndef psy_ui_COMPONENTSTYLE_H
#define psy_ui_COMPONENTSTYLE_H

#include "uistyles.h"
#include "uicomponentsizehints.h"

/*
** psy_ui_ComponentStyle
**
** Assigns styles to special states
**
** todo: add specificity for mixing several state styles
**       the states flag stores combinations of states but only one style can
**       be selected at the moment
*/

#ifdef __cplusplus
extern "C" {
#endif

typedef enum psy_ui_StyleState {
	psy_ui_STYLESTATE_NONE     = 0,
	psy_ui_STYLESTATE_HOVER    = 1,
	psy_ui_STYLESTATE_SELECT   = 2,
	psy_ui_STYLESTATE_FOCUS    = 4,
	psy_ui_STYLESTATE_DISABLED = 8,
	psy_ui_STYLESTATE_ACTIVE   = 16,
	psy_ui_STYLESTATE_NUM      = 32
} psy_ui_StyleState;


/* psy_ui_ComponentStyleStates */
typedef struct psy_ui_ComponentStyleStates {
	psy_Table* styles;	/* StyleState X StyleId */	
} psy_ui_ComponentStyleStates;

void psy_ui_componentstylestates_init(psy_ui_ComponentStyleStates*);
void psy_ui_componentstylestates_dispose(psy_ui_ComponentStyleStates*);

uintptr_t psy_ui_componentstylestates_style_id(psy_ui_ComponentStyleStates*,
	psy_ui_StyleState);
void psy_ui_componentstylestates_setstyle(psy_ui_ComponentStyleStates*,
	psy_ui_StyleState, uintptr_t style_id);


/* psy_ui_ComponentStyle */
typedef struct psy_ui_ComponentStyle {	
	psy_ui_ComponentStyleStates styles;
	uintptr_t currstyle;	    /* 
								** StyleId of the current style state 
								** - if no state available points to the style
								**   for STYLESTATE_NONE
								** - if no style at all available points to
								**   inlinestyle
								*/
	psy_ui_Style inlinestyle;	/* overrides currstyle for component setter */	
	psy_ui_StyleState states;	/* bit flag holding the activated states */	
	psy_ui_SizeHints* sizehints;
	intptr_t debugflag;
} psy_ui_ComponentStyle;

void psy_ui_componentstyle_init(psy_ui_ComponentStyle*);
void psy_ui_componentstyle_dispose(psy_ui_ComponentStyle*);

/* assigns a style to a state */
void psy_ui_componentstyle_set_style(psy_ui_ComponentStyle*,
	psy_ui_StyleState, uintptr_t style_id);
/* activeates a state */
bool psy_ui_componentstyle_add_state(psy_ui_ComponentStyle*, psy_ui_StyleState);
/* deactivates a state */
bool psy_ui_componentstyle_remove_state(psy_ui_ComponentStyle*,
	psy_ui_StyleState);
/* sets the currstate according to the style state */
bool psy_ui_componentstyle_setcurrstate(psy_ui_ComponentStyle*,
	psy_ui_StyleState);
/* returns state flags */
INLINE psy_ui_StyleState psy_ui_componentstyle_state(
	const psy_ui_ComponentStyle* self)
{
	return self->states;
}
psy_ui_Style* psy_ui_componentstyle_currstyle(psy_ui_ComponentStyle*);
const psy_ui_Style* psy_ui_componentstyle_currstyle_const(
	const psy_ui_ComponentStyle*);

/*
** Properties
*/
INLINE void psy_ui_componentstyle_set_margin(psy_ui_ComponentStyle* self,
		psy_ui_Margin margin)
{
	psy_ui_style_setmargin(&self->inlinestyle, margin);	
}

INLINE psy_ui_Margin psy_ui_componentstyle_margin(
	const psy_ui_ComponentStyle* self)
{	
	if (self->inlinestyle.margin_set) {
		return self->inlinestyle.margin;
	} 
	return psy_ui_componentstyle_currstyle_const(self)->margin;
}

INLINE void psy_ui_componentstyle_set_padding(psy_ui_ComponentStyle* self,
	psy_ui_Margin padding)
{
	psy_ui_style_setpadding(&self->inlinestyle, padding);	
}

INLINE psy_ui_Margin psy_ui_componentstyle_padding(
	const psy_ui_ComponentStyle * self)
{
	if (self->inlinestyle.padding_set) {
		return self->inlinestyle.padding;
	}
	return psy_ui_componentstyle_currstyle_const(self)->padding;
}

INLINE psy_ui_Colour psy_ui_componentstyle_colour(
	const psy_ui_ComponentStyle* self)
{
	if (!self->inlinestyle.colour.mode.transparent) {
		return self->inlinestyle.colour;
	}
	return psy_ui_componentstyle_currstyle_const(self)->colour;
}

INLINE void psy_ui_componentstyle_set_colour(psy_ui_ComponentStyle* self,
	psy_ui_Colour colour)
{
	self->inlinestyle.colour = colour;
}

INLINE psy_ui_Colour psy_ui_componentstyle_background_colour(
	const psy_ui_ComponentStyle* self)
{
	if (!self->inlinestyle.background.colour.mode.transparent) {
		return self->inlinestyle.background.colour;
	}
	return psy_ui_componentstyle_currstyle_const(self)->background.colour;
}

INLINE void psy_ui_componentstyle_set_background_colour(
	psy_ui_ComponentStyle* self, psy_ui_Colour colour)
{
	self->inlinestyle.background.colour = colour;
}

INLINE const psy_ui_Border* psy_ui_componentstyle_border(
	const psy_ui_ComponentStyle* self)
{
	if (self->inlinestyle.border.mode.set) {
		return &self->inlinestyle.border;
	}
	return &psy_ui_componentstyle_currstyle_const(self)->border;
}

INLINE void psy_ui_componentstyle_set_border(psy_ui_ComponentStyle* self,
	const psy_ui_Border* border)
{
	if (!border) {
		self->inlinestyle.border.mode.set = 0;
		return;
	}
	self->inlinestyle.border = *border;
}

void psy_ui_componentstyle_usesizehints(psy_ui_ComponentStyle*);
void psy_ui_componentstyle_setpreferredsize(psy_ui_ComponentStyle*, psy_ui_Size);
psy_ui_Size psy_ui_componentstyle_preferredsize(const psy_ui_ComponentStyle*);
void psy_ui_componentstyle_setpreferredheight(psy_ui_ComponentStyle*, psy_ui_Value);
void psy_ui_componentstyle_setpreferredwidth(psy_ui_ComponentStyle*, psy_ui_Value);
void psy_ui_componentstyle_setmaximumsize(psy_ui_ComponentStyle*, psy_ui_Size);
const psy_ui_Size psy_ui_componentstyle_maximumsize(const psy_ui_ComponentStyle*);
void psy_ui_componentstyle_setminimumsize(psy_ui_ComponentStyle*, psy_ui_Size);
const psy_ui_Size psy_ui_componentstyle_minimumsize(const psy_ui_ComponentStyle*);

bool psy_ui_componentstyle_background_animation_enabled(
	const psy_ui_ComponentStyle*);


#ifdef __cplusplus
}
#endif

#endif /* psy_ui_COMPONENTSTYLE_H */

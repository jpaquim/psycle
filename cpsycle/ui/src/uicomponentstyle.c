/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "uicomponentstyle.h"
/* local */
#include "uiapp.h"
/* std */
#include <stdlib.h>

static bool sizehints_initialized = FALSE;
static psy_ui_SizeHints sizehints;

/* psy_ui_ComponentStyleStates */

/* implementation */
void psy_ui_componentstylestates_init(psy_ui_ComponentStyleStates* self)
{
	assert(self);
	
	self->styles = NULL;
}

void psy_ui_componentstylestates_dispose(psy_ui_ComponentStyleStates* self)
{
	assert(self);
	
	if (self->styles) {
		psy_table_dispose(self->styles);
		free(self->styles);
	}
}

uintptr_t psy_ui_componentstylestates_style_id(psy_ui_ComponentStyleStates* self,
	psy_ui_StyleState state)
{
	uintptr_t styleid;	

	assert(self);
		
	if (!self->styles) {
		return psy_INDEX_INVALID;
	}
	styleid = psy_INDEX_INVALID;
	if (psy_table_exists(self->styles, state)) {
		styleid = (uintptr_t)psy_table_at_const(self->styles, (uintptr_t)state);
	}
	if (styleid != psy_INDEX_INVALID) {
		return styleid;
	}	
	if (psy_table_exists(self->styles, (uintptr_t)psy_ui_STYLESTATE_NONE)) {
		return (uintptr_t)psy_table_at_const(self->styles,
			(uintptr_t)psy_ui_STYLESTATE_NONE);		
	}		
	return psy_INDEX_INVALID;
}

void psy_ui_componentstylestates_setstyle(psy_ui_ComponentStyleStates* self,
	psy_ui_StyleState state, uintptr_t style_id)
{
	assert(self);

	if (style_id != psy_INDEX_INVALID) {		
		if (!self->styles) {
			self->styles = (psy_Table*)malloc(sizeof(psy_Table));			
			psy_table_init_keysize(self->styles, 11);
		}
		psy_table_insert(self->styles, state, (void*)(uintptr_t)style_id);
	} else if (self->styles) {
		psy_table_remove(self->styles, state);
	}	
}

/* psy_ui_ComponentStyle */

/* prototypes */
static uintptr_t psy_ui_componentstyle_style_id(psy_ui_ComponentStyle*,
	psy_ui_StyleState);
static bool psy_ui_componentstyle_has_state(const psy_ui_ComponentStyle*,
	psy_ui_StyleState);
/* sets the currstate according to current style state */
static void psy_ui_componentstyle_updatecurrstate(psy_ui_ComponentStyle*);
/* selects a state from the states flag (see todo) */
static bool psy_ui_componentstyle_updatestate(psy_ui_ComponentStyle*);

/* implementation */
void psy_ui_componentstyle_init(psy_ui_ComponentStyle* self)
{
	assert(self);

	psy_ui_componentstylestates_init(&self->styles);
	psy_ui_style_init(&self->inlinestyle);	
	self->currstyle = psy_INDEX_INVALID;
	self->states = psy_ui_STYLESTATE_NONE;		
	if (!sizehints_initialized) {
		psy_ui_sizehints_init(&sizehints);
		sizehints_initialized = TRUE;
	}
	self->sizehints = &sizehints;
	self->debugflag = 0;
}

void psy_ui_componentstyle_dispose(psy_ui_ComponentStyle* self)
{
	assert(self);
	
	self->currstyle = psy_INDEX_INVALID;
	psy_ui_componentstylestates_dispose(&self->styles);	
	psy_ui_style_dispose(&self->inlinestyle);
	if (self->sizehints != &sizehints) {
		free(self->sizehints);
		self->sizehints = NULL;
	}
}

uintptr_t psy_ui_componentstyle_style_id(psy_ui_ComponentStyle* self,
	psy_ui_StyleState state)
{
	return psy_ui_componentstylestates_style_id(&self->styles, state);
}

bool psy_ui_componentstyle_has_state(const psy_ui_ComponentStyle* self,
	psy_ui_StyleState state)
{
	assert(self);

	return ((self->states & state) == state);
}

bool psy_ui_componentstyle_updatestate(psy_ui_ComponentStyle* self)
{
	uintptr_t state;

	assert(self);
	
	if (psy_ui_componentstyle_has_state(self, psy_ui_STYLESTATE_DISABLED)) {
		state = psy_ui_STYLESTATE_DISABLED;
	} else if (psy_ui_componentstyle_has_state(self, psy_ui_STYLESTATE_ACTIVE)) {
		state = psy_ui_STYLESTATE_ACTIVE;	
	} else if (psy_ui_componentstyle_has_state(self, psy_ui_STYLESTATE_SELECT)) {
		state = psy_ui_STYLESTATE_SELECT;	
	} else if (psy_ui_componentstyle_has_state(self, psy_ui_STYLESTATE_FOCUS)) {
		state = psy_ui_STYLESTATE_FOCUS;
	} else if (psy_ui_componentstyle_has_state(self, psy_ui_STYLESTATE_HOVER)) {
		state = psy_ui_STYLESTATE_HOVER;
	} else {
		state = psy_ui_STYLESTATE_NONE;
	}	
	return psy_ui_componentstyle_setcurrstate(self, state);
}

bool psy_ui_componentstyle_setcurrstate(psy_ui_ComponentStyle* self,
	psy_ui_StyleState state)
{
	uintptr_t oldstyle;

	assert(self);

	oldstyle = self->currstyle;
	self->currstyle = psy_ui_componentstyle_style_id(self, state);	
	return self->currstyle != oldstyle;
}

void psy_ui_componentstyle_set_style(psy_ui_ComponentStyle* self,
	psy_ui_StyleState state, uintptr_t style_id)
{
	assert(self);

	psy_ui_componentstylestates_setstyle(&self->styles, state, style_id);	
	psy_ui_componentstyle_setcurrstate(self, self->states);
}

bool psy_ui_componentstyle_add_state(psy_ui_ComponentStyle* self,
	psy_ui_StyleState state)
{
	uintptr_t newstate;

	assert(self);

	newstate = self->states | state;
	if (newstate != self->states) {
		self->states = newstate;
		return psy_ui_componentstyle_updatestate(self);
	}
	return FALSE;
}

bool psy_ui_componentstyle_remove_state(psy_ui_ComponentStyle* self,
	psy_ui_StyleState state)
{
	uintptr_t newstate;

	assert(self);
	
	newstate = self->states & (~state);
	if (newstate != self->states) {
		self->states = newstate;
		return psy_ui_componentstyle_updatestate(self);
	}
	return FALSE;
}

psy_ui_Style* psy_ui_componentstyle_currstyle(psy_ui_ComponentStyle* self)
{	
	if (self->currstyle == psy_INDEX_INVALID) {
		return &self->inlinestyle;
	}
	return psy_ui_style(self->currstyle);
}

const psy_ui_Style* psy_ui_componentstyle_currstyle_const(const psy_ui_ComponentStyle* self)
{
	return psy_ui_componentstyle_currstyle((psy_ui_ComponentStyle*)self);
}

void psy_ui_componentstyle_updatecurrstate(psy_ui_ComponentStyle* self)
{
	psy_ui_componentstyle_setcurrstate(self, self->states);
}

psy_ui_Size psy_ui_componentstyle_preferredsize(const psy_ui_ComponentStyle* self)
{	
	psy_ui_Size rv;	
	const psy_ui_Style* style;

	rv = self->sizehints->preferredsize;
	if (rv.height.set && rv.width.set) {
		return rv;
	}
	style = psy_ui_componentstyle_currstyle_const(self);
	if (style) {
		if (!rv.width.set) {
			rv.width = style->position.rectangle->size.width;
		}
		if (!rv.height.set) {
			rv.height = style->position.rectangle->size.height;
		}
	}
	return rv;
}

void psy_ui_componentstyle_setpreferredsize(psy_ui_ComponentStyle* self, psy_ui_Size size)
{
	psy_ui_componentstyle_usesizehints(self);
	self->sizehints->preferredsize = size;	
}

void psy_ui_componentstyle_setpreferredheight(psy_ui_ComponentStyle* self, psy_ui_Value height)
{
	psy_ui_componentstyle_usesizehints(self);
	self->sizehints->preferredsize.height = height;	
}

void psy_ui_componentstyle_setpreferredwidth(psy_ui_ComponentStyle* self, psy_ui_Value width)
{
	psy_ui_componentstyle_usesizehints(self);
	self->sizehints->preferredsize.width = width;
}

void psy_ui_componentstyle_setmaximumsize(psy_ui_ComponentStyle* self, psy_ui_Size size)
{
	psy_ui_componentstyle_usesizehints(self);
	self->sizehints->maxsize = size;
}

const psy_ui_Size psy_ui_componentstyle_maximumsize(const psy_ui_ComponentStyle* self)
{
	return self->sizehints->maxsize;
}

void psy_ui_componentstyle_setminimumsize(psy_ui_ComponentStyle* self, psy_ui_Size size)
{
	psy_ui_componentstyle_usesizehints(self);
	self->sizehints->minsize = size;
}

const psy_ui_Size psy_ui_componentstyle_minimumsize(const psy_ui_ComponentStyle* self)
{
	return self->sizehints->minsize;
}

void psy_ui_componentstyle_usesizehints(psy_ui_ComponentStyle* self)
{
	if (self->sizehints == &sizehints) {
		self->sizehints = psy_ui_sizehints_allocinit();
	}
}

bool psy_ui_componentstyle_background_animation_enabled(
	const psy_ui_ComponentStyle* self)
{
	return (psy_ui_componentstyle_currstyle_const(self)->background.animation.enabled);
}

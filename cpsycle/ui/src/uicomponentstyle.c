/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "uicomponentstyle.h"
/* local */
#include "uiapp.h"
/* std */
#include <stdlib.h>

void psy_ui_componentstyle_init(psy_ui_ComponentStyle* self)
{
	assert(self);

	self->styles = NULL;	
	psy_ui_style_init(&self->overridestyle);
	self->currstyle = &self->overridestyle;	
	self->states = psy_ui_STYLESTATE_NONE;	
	self->debugflag = 0;
}

void psy_ui_componentstyle_dispose(psy_ui_ComponentStyle* self)
{
	assert(self);
	
	self->currstyle = NULL;	
	if (self->styles) {
		psy_table_dispose(self->styles);
		free(self->styles);
	}
	psy_ui_style_dispose(&self->overridestyle);
}

bool psy_ui_componentstyle_hasstyle(const psy_ui_ComponentStyle* self,
	psy_ui_StyleState state)
{
	assert(self);

	if (!self->styles) {
		FALSE;
	}
	return psy_table_exists(self->styles, (uintptr_t)state);
}

psy_ui_Style* psy_ui_componentstyle_style(psy_ui_ComponentStyle* self,
	psy_ui_StyleState state)
{
	uintptr_t styleid;
	psy_ui_Style* rv;

	assert(self);
	
	rv = NULL;
	if (!self->styles) {
		return &self->overridestyle;
	}
	styleid = psy_INDEX_INVALID;
	if (psy_table_exists(self->styles, state)) {
		styleid = (uintptr_t)psy_table_at_const(self->styles, (uintptr_t)state);
	}
	if (styleid != psy_INDEX_INVALID) {
		rv = psy_ui_style(styleid);
	}
	if (!rv) {
		if (psy_table_exists(self->styles, (uintptr_t)psy_ui_STYLESTATE_NONE)) {
			styleid = (uintptr_t)psy_table_at_const(self->styles,
				(uintptr_t)psy_ui_STYLESTATE_NONE);
			rv = psy_ui_style(styleid);
		}
	}
	if (!rv) {
		rv = &self->overridestyle;	
	}	
	return rv;
}

bool psy_ui_componentstyle_hasstate(const psy_ui_ComponentStyle* self,
	psy_ui_StyleState state)
{
	assert(self);

	return ((self->states & state) == state);
}

bool psy_ui_componentstyle_updatestate(psy_ui_ComponentStyle* self)
{
	uintptr_t state;

	assert(self);
	
	if (psy_ui_componentstyle_hasstate(self, psy_ui_STYLESTATE_DISABLED)) {
		state = psy_ui_STYLESTATE_DISABLED;
	} else if (psy_ui_componentstyle_hasstate(self,
			psy_ui_STYLESTATE_SELECT)) {
		state = psy_ui_STYLESTATE_SELECT;
	} else if (psy_ui_componentstyle_hasstate(self,
			psy_ui_STYLESTATE_ACTIVE)) {
		state = psy_ui_STYLESTATE_ACTIVE;	
	} else if (psy_ui_componentstyle_hasstate(self, psy_ui_STYLESTATE_FOCUS)) {
		state = psy_ui_STYLESTATE_FOCUS;
	} else if (psy_ui_componentstyle_hasstate(self, psy_ui_STYLESTATE_HOVER)) {
		state = psy_ui_STYLESTATE_HOVER;
	} else {
		state = psy_ui_STYLESTATE_NONE;
	}	
	return psy_ui_componentstyle_setcurrstate(self, state);
}

bool psy_ui_componentstyle_setcurrstate(psy_ui_ComponentStyle* self,
	psy_ui_StyleState state)
{
	psy_ui_Style* oldstyle;

	assert(self);

	oldstyle = self->currstyle;	
	self->currstyle = psy_ui_componentstyle_style(self, state);	
	return self->currstyle != oldstyle;
}

void psy_ui_componentstyle_setstyle(psy_ui_ComponentStyle* self,
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
	psy_ui_componentstyle_setcurrstate(self, self->states);
}

bool psy_ui_componentstyle_addstate(psy_ui_ComponentStyle* self,
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

bool psy_ui_componentstyle_removestate(psy_ui_ComponentStyle* self,
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

uintptr_t psy_ui_componentstyle_currstyleid(const psy_ui_ComponentStyle* self)
{
	/* todo works only if one state is set */
	if (!self->styles) {
		return psy_INDEX_INVALID;
	}
	if (psy_table_exists(self->styles, self->states)) {
		return (uintptr_t)psy_table_at_const(self->styles, self->states);
	}
	return psy_INDEX_INVALID;
}

void psy_ui_componentstyle_updatecurrstate(psy_ui_ComponentStyle* self)
{
	psy_ui_componentstyle_setcurrstate(self, self->states);
}

// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "uicomponentstyle.h"
// local
#include "uiapp.h"
// platform
#include "../../detail/portable.h"

void psy_ui_componentstyle_init(psy_ui_ComponentStyle* self)
{
	assert(self);

	psy_table_init(&self->styles);
	psy_ui_style_init(&self->style);
	psy_table_init(&self->styleids);
	self->state = psy_ui_STYLESTATE_NONE;
	self->currstyle = &self->style;
	psy_ui_margin_init(&self->margin);
	psy_ui_margin_init(&self->spacing);
}

void psy_ui_componentstyle_dispose(psy_ui_ComponentStyle* self)
{
	assert(self);

	psy_table_dispose(&self->styleids);
	psy_table_disposeall(&self->styles, (psy_fp_disposefunc)
		psy_ui_style_dispose);
	psy_ui_style_dispose(&self->style);
}

bool psy_ui_componentstyle_hasstyle(const psy_ui_ComponentStyle* self,
	psy_ui_StyleState state)
{
	assert(self);

	return psy_table_exists(&self->styles, (uintptr_t)state);
}

psy_ui_Style* psy_ui_componentstyle_style(psy_ui_ComponentStyle* self,
	psy_ui_StyleState state)
{
	assert(self);

	if (state == psy_ui_STYLESTATE_NONE) {
		return &self->style;
	}
	return psy_table_at(&self->styles, (uintptr_t)state);
}

bool psy_ui_componentstyle_hasstate(const psy_ui_ComponentStyle* self,
	psy_ui_StyleState state)
{
	assert(self);

	return ((self->state & state) == state);
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
	if (psy_ui_componentstyle_hasstyle(self, state)) {
		self->currstyle = psy_ui_componentstyle_style(self, state);
	} else {			
		self->currstyle = &self->style;		
	}
	return self->currstyle != oldstyle;
}

void psy_ui_componentstyle_setstyle(psy_ui_ComponentStyle* self,
	psy_ui_StyleState state, uintptr_t style_id)
{
	assert(self);

	if (style_id != psy_INDEX_INVALID) {
		psy_table_insert(&self->styleids, state, (void*)(uintptr_t)style_id);
	} else {
		psy_table_remove(&self->styleids, state);
	}
}

void psy_ui_componentstyle_readstyle(psy_ui_ComponentStyle* self,
	psy_ui_StyleState state, uintptr_t style_id)
{
	assert(self);

	if (style_id != psy_INDEX_INVALID) {
		psy_ui_Style* style;

		if (state == psy_ui_STYLESTATE_NONE) {
			style = &self->style;
		} else {
			style = psy_ui_componentstyle_style(self, state);
			if (!style) {
				style = psy_ui_style_allocinit();
				psy_table_insert(&self->styles, state, (void*)style);
			}			
		}
		psy_ui_style_copy(style, psy_ui_style(style_id));
	}
}

void psy_ui_componentstyle_readstyles(psy_ui_ComponentStyle* self)
{
	psy_TableIterator it;	

	assert(self);
	
	for (it = psy_table_begin(&self->styleids);
			!psy_tableiterator_equal(&it, psy_table_end());
			psy_tableiterator_inc(&it)) {
		psy_ui_StyleState state;
		uintptr_t id;
		
		state = (psy_ui_StyleState)psy_tableiterator_key(&it);
		id = (uintptr_t)psy_tableiterator_value(&it);
		psy_ui_componentstyle_readstyle(self, state, id);
	}
}

bool psy_ui_componentstyle_addstate(psy_ui_ComponentStyle* self,
	psy_ui_StyleState state)
{
	uintptr_t newstate;

	assert(self);

	newstate = self->state | state;
	if (newstate != self->state) {
		self->state = newstate;
		return psy_ui_componentstyle_updatestate(self);
	}
	return FALSE;
}

bool psy_ui_componentstyle_removestate(psy_ui_ComponentStyle* self,
	psy_ui_StyleState state)
{
	uintptr_t newstate;

	assert(self);

	if (self->state & psy_ui_STYLESTATE_SELECT) {
		self = self;
	}
	newstate = self->state & (~state);
	if (newstate != self->state) {
		self->state = newstate;
		return psy_ui_componentstyle_updatestate(self);
	}
	return FALSE;
}

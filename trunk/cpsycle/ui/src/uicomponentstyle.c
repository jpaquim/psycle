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
	psy_ui_style_init(&self->style);
	psy_ui_style_init(&self->focus);
	psy_ui_style_init(&self->hover);
	psy_ui_style_init(&self->select);
	psy_ui_style_init(&self->disabled);
	psy_ui_style_init(&self->active);
	self->currstyle = &self->style;
	self->style_id = psy_INDEX_INVALID;
	self->focus_id = psy_INDEX_INVALID;
	self->hover_id = psy_INDEX_INVALID;
	self->select_id = psy_INDEX_INVALID;
	self->disabled_id = psy_INDEX_INVALID;
	self->active_id = psy_INDEX_INVALID;
	self->state = psy_ui_STYLESTATE_NONE;
}

void psy_ui_componentstyle_dispose(psy_ui_ComponentStyle* self)
{
	psy_ui_style_dispose(&self->style);
	psy_ui_style_dispose(&self->focus);
	psy_ui_style_dispose(&self->hover);
	psy_ui_style_dispose(&self->select);	
	psy_ui_style_dispose(&self->disabled);
	psy_ui_style_dispose(&self->active);
}

bool psy_ui_componentstyle_hasstate(const psy_ui_ComponentStyle* self,
	psy_ui_StyleState state)
{
	return ((self->state & state) == state);
}

bool psy_ui_componentstyle_updatestate(psy_ui_ComponentStyle* self)
{
	uintptr_t state;

	if (psy_ui_componentstyle_hasstate(self, psy_ui_STYLESTATE_DISABLED)) {
		state = psy_ui_STYLESTATE_DISABLED;
	} else if (psy_ui_componentstyle_hasstate(self, psy_ui_STYLESTATE_SELECT)) {
		state = psy_ui_STYLESTATE_SELECT;
	} else if (psy_ui_componentstyle_hasstate(self, psy_ui_STYLESTATE_ACTIVE)) {
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

	oldstyle = self->currstyle;
	switch (state) {
	case psy_ui_STYLESTATE_NONE:
		if (self->currstyle != &self->style) {
			self->currstyle = &self->style;			
		}
		break;
	case psy_ui_STYLESTATE_FOCUS:
		if (self->focus_id != psy_INDEX_INVALID) {
			self->currstyle = &self->focus;

		}
		break;
	case psy_ui_STYLESTATE_HOVER:
		if (self->hover_id != psy_INDEX_INVALID) {
			self->currstyle = &self->hover;		
		}
		break;
	case psy_ui_STYLESTATE_SELECT:
		if (self->select_id != psy_INDEX_INVALID) {
			self->currstyle = &self->select;		
		}
		break;
	case psy_ui_STYLESTATE_DISABLED:
		if (self->disabled_id != psy_INDEX_INVALID) {
			self->currstyle = &self->disabled;		
		}
		break;
	case psy_ui_STYLESTATE_ACTIVE:
		if (self->active_id != psy_INDEX_INVALID) {
			self->currstyle = &self->active;
		}
		break;
	default:
		if (self->currstyle != &self->style) {
			self->currstyle = &self->style;		
		}
		break;
	}
	return self->currstyle != oldstyle;
}

void psy_ui_componentstyle_readstyles(psy_ui_ComponentStyle* self)
{
	if (self->style_id != psy_INDEX_INVALID) {
		psy_ui_style_copy(&self->style,
			psy_ui_style(self->style_id));
	}
	if (self->focus_id != psy_INDEX_INVALID) {
		psy_ui_style_copy(&self->focus,
			psy_ui_style(self->focus_id));
	}
	if (self->hover_id != psy_INDEX_INVALID) {
		psy_ui_style_copy(&self->hover,
			psy_ui_style(self->hover_id));
	}
	if (self->select_id != psy_INDEX_INVALID) {
		psy_ui_style_copy(&self->select,
			psy_ui_style(self->select_id));
	}
	if (self->disabled_id != psy_INDEX_INVALID) {
		psy_ui_style_copy(&self->disabled,
			psy_ui_style(self->disabled_id));
	}
	if (self->active_id != psy_INDEX_INVALID) {
		psy_ui_style_copy(&self->active,
			psy_ui_style(self->active_id));
	}
}

bool psy_ui_componentstyle_addstate(psy_ui_ComponentStyle* self,
	psy_ui_StyleState state)
{
	uintptr_t newstate;

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

	newstate = self->state & (~state);
	if (newstate != self->state) {
		self->state = newstate;
		return psy_ui_componentstyle_updatestate(self);
	}
	return FALSE;
}
/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "uievents.h"
/* local */
#include "uicomponent.h"
/* container */
#include <properties.h>

/* psy_ui_Event */
void psy_ui_event_init(psy_ui_Event* self, psy_ui_EventType type)
{
	self->type = type;
	self->bubbles = TRUE;
	self->default_prevented = FALSE;
	self->target = NULL;
	self->currenttarget = NULL;
	self->timestamp = 0;
}

void psy_ui_event_init_stop_propagation(psy_ui_Event* self, psy_ui_EventType type)
{
	psy_ui_event_init(self, type);
	psy_ui_event_stop_propagation(self);
}

/* psy_ui_KeyboardEvent */
void psy_ui_keyboardevent_init(psy_ui_KeyboardEvent* self)
{
	psy_ui_keyboardevent_init_all(self, 0, 0, 0, 0, 0, 0);
}

void psy_ui_keyboardevent_init_all(psy_ui_KeyboardEvent* self, uint32_t keycode,
	intptr_t keydata, bool shift, bool ctrl, bool alt, bool repeat)
{
	psy_ui_event_init(&self->event, psy_ui_KEYDOWN);
	self->keycode = keycode;
	self->keydata = keydata;
	self->shift_key = shift;
	self->ctrl_key = ctrl;
	self->alt_key = alt;
	self->repeat = repeat;	
}

/* psy_ui_MouseEvent */
void psy_ui_mouseevent_init(psy_ui_MouseEvent* self)
{
	psy_ui_mouseevent_init_all(self, psy_ui_realpoint_zero(),		
		0, 0, 0, 0);
}

void psy_ui_mouseevent_init_all(psy_ui_MouseEvent* self, psy_ui_RealPoint offset,
	uintptr_t button, intptr_t delta, bool shift, bool ctrl)
{
	psy_ui_event_init(&self->event, psy_ui_MOUSEDOWN);	
	self->offset_ = offset;
	self->button_ = button;
	self->delta_ = delta;	
	self->shift_key_ = shift;
	self->ctrl_key_ = ctrl;
}

psy_ui_RealPoint psy_ui_mouseevent_pt(const psy_ui_MouseEvent* self)
{
	psy_ui_RealPoint rv;

	rv = psy_ui_mouseevent_offset(self); /* relative padding edge */
	if (self->event.target) {
		psy_ui_RealMargin padding;

		padding = psy_ui_component_spacing_px(self->event.currenttarget);
		if (!psy_ui_realmargin_iszero(&padding)) {
			/* subtract padding edge */
			rv.x -= padding.left;
			rv.y -= padding.top;
		}
	}
	return rv;
}

/* psy_ui_DragEvent */
void psy_ui_dragevent_init(psy_ui_DragEvent* self)
{
	psy_ui_mouseevent_init(&self->mouse);
	self->mouse.event.type = psy_ui_DRAG;
	self->mouse.event.default_prevented = TRUE;	
	self->active = FALSE;	
	self->dataTransfer = NULL;
}

void psy_ui_dragevent_dispose(psy_ui_DragEvent* self)
{
	if (self->dataTransfer) {
		psy_property_deallocate(self->dataTransfer);
		self->dataTransfer = NULL;
	}
}

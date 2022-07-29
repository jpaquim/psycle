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
	self->type_ = type;
	self->bubbles_ = TRUE;
	self->default_prevented_ = FALSE;
	self->prevent_focus = FALSE;
	self->target_ = NULL;
	self->currenttarget_ = NULL;
	self->timestamp_ = 0;
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
	self->keycode_ = keycode;
	self->keydata_ = keydata;
	self->shift_key_ = shift;
	self->ctrl_key_ = ctrl;
	self->alt_key_ = alt;
	self->repeat_ = repeat;
}

unsigned char psy_ui_keyboardevent_printable_char(psy_ui_KeyboardEvent* self)
{	
	int lower;
	
	lower = 'a' - 0x41;
	switch (self->keycode_) {
	case psy_ui_KEY_SPACE: return 0x20;
	case psy_ui_KEY_DIGIT0: return 0x30;
	case psy_ui_KEY_DIGIT1: return 0x31;
	case psy_ui_KEY_DIGIT2: return 0x32;
	case psy_ui_KEY_DIGIT3: return 0x33;
	case psy_ui_KEY_DIGIT4: return 0x34;
	case psy_ui_KEY_DIGIT5: return 0x35;
	case psy_ui_KEY_DIGIT6: return 0x36;
	case psy_ui_KEY_DIGIT7: return 0x37;
	case psy_ui_KEY_DIGIT8: return 0x38;
	case psy_ui_KEY_DIGIT9: return 0x39;
	case psy_ui_KEY_A: return self->shift_key_ ? 0x41 : lower + 0x41;
	case psy_ui_KEY_B: return self->shift_key_ ? 0x42 : lower + 0x42;
	case psy_ui_KEY_C: return self->shift_key_ ? 0x43 : lower + 0x43;
	case psy_ui_KEY_D: return self->shift_key_ ? 0x44 : lower + 0x44;
	case psy_ui_KEY_E: return self->shift_key_ ? 0x45 : lower + 0x45;
	case psy_ui_KEY_F: return self->shift_key_ ? 0x46 : lower + 0x46;
	case psy_ui_KEY_G: return self->shift_key_ ? 0x47 : lower + 0x47;
	case psy_ui_KEY_H: return self->shift_key_ ? 0x48 : lower + 0x48;
	case psy_ui_KEY_I: return self->shift_key_ ? 0x49 : lower + 0x49;
	case psy_ui_KEY_J: return self->shift_key_ ? 0x4A : lower + 0x4A;
	case psy_ui_KEY_K: return self->shift_key_ ? 0x4B : lower + 0x4B;
	case psy_ui_KEY_L: return self->shift_key_ ? 0x4C : lower + 0x4C;
	case psy_ui_KEY_M: return self->shift_key_ ? 0x4D : lower + 0x4D;
	case psy_ui_KEY_N: return self->shift_key_ ? 0x4E : lower + 0x4E;
	case psy_ui_KEY_O: return self->shift_key_ ? 0x4F : lower + 0x4F;
	case psy_ui_KEY_P: return self->shift_key_ ? 0x50 : lower + 0x50;
	case psy_ui_KEY_Q: return self->shift_key_ ? 0x51 : lower + 0x51;
	case psy_ui_KEY_R: return self->shift_key_ ? 0x52 : lower + 0x52;
	case psy_ui_KEY_S: return self->shift_key_ ? 0x53 : lower + 0x53;
	case psy_ui_KEY_T: return self->shift_key_ ? 0x54 : lower + 0x54;
	case psy_ui_KEY_U: return self->shift_key_ ? 0x55 : lower + 0x55;
	case psy_ui_KEY_V: return self->shift_key_ ? 0x56 : lower + 0x56;
	case psy_ui_KEY_W: return self->shift_key_ ? 0x57 : lower + 0x57;
	case psy_ui_KEY_X: return self->shift_key_ ? 0x58 : lower + 0x58;
	case psy_ui_KEY_Y: return self->shift_key_ ? 0x59 : lower + 0x59;
	case psy_ui_KEY_Z: return self->shift_key_ ? 0x5A : lower + 0x5A;
	case psy_ui_KEY_MULTIPLY:return '*';
	case psy_ui_KEY_ADD: return '+';
	case psy_ui_KEY_SEPARATOR: return '|';
	case psy_ui_KEY_SUBTRACT: return '-';
	case psy_ui_KEY_DIVIDE: return ':';
	case psy_ui_KEY_SEMICOLON: return ';';
	case psy_ui_KEY_EQUAL: return '=';
	case psy_ui_KEY_COMMA: return ',';
	case psy_ui_KEY_MINUS: return '-';
	case psy_ui_KEY_PERIOD: return '.';
	case psy_ui_KEY_SLASH: return '/';
	case psy_ui_KEY_BACKQUOTE: return '`';
	case psy_ui_KEY_BRACKETLEFT: return '[';
	case psy_ui_KEY_BACKSLASH: return '\\';
	case psy_ui_KEY_QUOTE: return '\'';
	case psy_ui_KEY_BRACKETRIGHT: return ']';
	default:
		return 0;	
	}
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
	if (psy_ui_event_currenttarget_const(&self->event)) {
		psy_ui_RealMargin padding;

		padding = psy_ui_component_spacing_px(psy_ui_event_currenttarget_const(
			&self->event));
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
	psy_ui_mouseevent_settype(&self->mouse, psy_ui_DRAG);	
	self->mouse.event.default_prevented_ = TRUE;
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

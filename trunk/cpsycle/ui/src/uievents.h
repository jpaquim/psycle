/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#ifndef psy_ui_EVENTS_H
#define psy_ui_EVENTS_H

/* local */
#include "uidef.h"

#ifdef __cplusplus
extern "C" {
#endif

#define psy_ui_CURRENT_TIME 0

	typedef enum psy_ui_EventType {
		psy_ui_UNKNOWNEVENT = 1,
		psy_ui_KEYDOWN = 2,
		psy_ui_KEYUP = 3,
		psy_ui_MOUSEDOWN = 4,
		psy_ui_MOUSEUP = 5,
		psy_ui_MOUSEMOVE = 6,
		psy_ui_DBLCLICK = 7,
		psy_ui_MOUSEENTER = 8,
		psy_ui_MOUSELEAVE = 9,
		psy_ui_DRAG = 10,
		psy_ui_FOCUS = 11,
		psy_ui_FOCUSOUT = 12,
		psy_ui_FOCUSIN = 13,
		psy_ui_RESIZE = 14,
		psy_ui_WHEEL = 15
} psy_ui_EventType;

/* Forward Handler for Event target */
struct psy_ui_Component;

/*
** psy_ui_Event base class
** An event gives additional data to an event method triggered by the ui imp
*/
typedef struct psy_ui_Event {
	psy_ui_EventType type_;
	bool bubbles_;
	bool default_prevented_;
	bool prevent_focus;
	struct psy_ui_Component* target_;
	struct psy_ui_Component* currenttarget_;
	uintptr_t timestamp_;
} psy_ui_Event;

void psy_ui_event_init(psy_ui_Event*, psy_ui_EventType);
void psy_ui_event_init_stop_propagation(psy_ui_Event*, psy_ui_EventType);

INLINE psy_ui_EventType psy_ui_event_type(const psy_ui_Event* self)
{
	return self->type_;
}

INLINE void psy_ui_event_settype(psy_ui_Event* self,
	psy_ui_EventType type)
{
	self->type_ = type;
}

INLINE bool psy_ui_event_default_prevented(const psy_ui_Event* self)
{
	return self->default_prevented_;
}

INLINE void psy_ui_event_prevent_default(psy_ui_Event* self)
{
	self->default_prevented_ = TRUE;
}

INLINE bool psy_ui_event_bubbles(const psy_ui_Event* self)
{
	return self->bubbles_;
}

INLINE void psy_ui_event_stop_propagation(psy_ui_Event* self)
{
	self->bubbles_ = FALSE;
}

INLINE void psy_ui_event_settarget(psy_ui_Event* self,
	struct psy_ui_Component* component)
{
	self->target_ = component;
}

INLINE void psy_ui_event_setcurrenttarget(psy_ui_Event* self,
	struct psy_ui_Component* component)
{
	self->currenttarget_ = component;
}

/*
** The component that produced the event (the top of the bubble list).
** (opposed to currenttarget which is the current bubble component)
*/
INLINE struct psy_ui_Component* psy_ui_event_target(psy_ui_Event* self)
{
	return self->target_;
}

/*
** The current component as the event is bubbling (self of event method).
** (opposed to target which produced the event)
*/
INLINE struct psy_ui_Component* psy_ui_event_currenttarget(psy_ui_Event* self)
{
	return self->currenttarget_;
}

INLINE const struct psy_ui_Component* psy_ui_event_currenttarget_const(
	const psy_ui_Event* self)
{
	return self->currenttarget_;
}

INLINE uintptr_t psy_ui_event_timestamp(const psy_ui_Event* self)
{
	return self->timestamp_;
}

/* psy_ui_KeyboardEvent */
typedef struct psy_ui_KeyboardEvent {
	/* inherits */
	psy_ui_Event event;
	/* internal */
	uint32_t keycode_;
	intptr_t keydata_;
	bool shift_key_;
	bool ctrl_key_;
	bool alt_key_;
	bool repeat_;
} psy_ui_KeyboardEvent;

void psy_ui_keyboardevent_init(psy_ui_KeyboardEvent*);
void psy_ui_keyboardevent_init_all(psy_ui_KeyboardEvent*, uint32_t keycode,
	intptr_t keydata, bool shift, bool ctrl, bool alt, bool repeat);

INLINE void psy_ui_keyboardevent_prevent_default(psy_ui_KeyboardEvent* self)
{
	psy_ui_event_prevent_default(&self->event);
}

INLINE void psy_ui_keyboardevent_stop_propagation(psy_ui_KeyboardEvent* self)
{
	psy_ui_event_stop_propagation(&self->event);
}

INLINE struct psy_ui_Component* psy_ui_keyboardevent_target(
	psy_ui_KeyboardEvent* self)
{
	return psy_ui_event_target(&self->event);
}

INLINE void psy_ui_keyboardevent_settype(psy_ui_KeyboardEvent* self,
	psy_ui_EventType type)
{
	psy_ui_event_settype(&self->event, type);	
}

INLINE uint32_t psy_ui_keyboardevent_keycode(const psy_ui_KeyboardEvent* self)
{
	return self->keycode_;
}

INLINE intptr_t psy_ui_keyboardevent_keydata(const psy_ui_KeyboardEvent* self)
{
	return self->keydata_;
}

INLINE bool psy_ui_keyboardevent_repeat(const psy_ui_KeyboardEvent* self)
{
	return self->repeat_;
}

/* returns TRUE if the Ctrl key was pressed down when the event occurred. */
INLINE bool psy_ui_keyboardevent_ctrlkey(const psy_ui_KeyboardEvent* self)
{
	return self->ctrl_key_;
}

/* returns TRUE if the Shift key was pressed down when the event occurred */
INLINE bool psy_ui_keyboardevent_shiftkey(const psy_ui_KeyboardEvent* self)
{
	return self->shift_key_;
}

/* returns TRUE if the Shift key was pressed down when the event occurred */
INLINE bool psy_ui_keyboardevent_altkey(const psy_ui_KeyboardEvent* self)
{
	return self->alt_key_;
}

INLINE uint32_t psy_ui_keyboardevent_encode(const psy_ui_KeyboardEvent* self, bool up)
{
	return (self->keycode_ |
		((uintptr_t)self->shift_key_ << 8) |
		((uintptr_t)self->ctrl_key_ << 9) |
		((uintptr_t) self->alt_key_ << 10) |
		((uintptr_t)up << 11));
}

INLINE psy_ui_Event* psy_ui_keyboardevent_base(psy_ui_KeyboardEvent* self)
{
	return &self->event;
}


/* psy_ui_MouseEvent */
typedef struct psy_ui_MouseEvent {
	/* inherits */
	psy_ui_Event event;	
	/* internal */
	psy_ui_RealPoint offset_;
	uintptr_t button_;
	intptr_t delta_;
	bool shift_key_;
	bool ctrl_key_;
} psy_ui_MouseEvent;

void psy_ui_mouseevent_init(psy_ui_MouseEvent*);
void psy_ui_mouseevent_init_all(psy_ui_MouseEvent*, psy_ui_RealPoint,
	uintptr_t button, intptr_t delta,
	bool shift, bool ctrl);

INLINE void psy_ui_mouseevent_stop_propagation(psy_ui_MouseEvent* self)
{
	psy_ui_event_stop_propagation(&self->event);
}

INLINE void psy_ui_mouseevent_prevent_default(psy_ui_MouseEvent* self)
{
	psy_ui_event_prevent_default(&self->event);
}

INLINE struct psy_ui_Component* psy_ui_mouseevent_target(
	psy_ui_MouseEvent* self)
{
	return psy_ui_event_target(&self->event);
}

INLINE bool psy_ui_mouseevent_type(const psy_ui_MouseEvent* self)	
{
	return psy_ui_event_type(&self->event);
}

INLINE void psy_ui_mouseevent_settype(psy_ui_MouseEvent* self,
	psy_ui_EventType type)
{
	psy_ui_event_settype(&self->event, type);
}

INLINE psy_ui_Event* psy_ui_mouseevent_base(psy_ui_MouseEvent* self)
{
	return &self->event;
}


/*
** X/Y offset coordinate of the mouse pointer relative to the padding edge of
** the current target
*/
INLINE psy_ui_RealPoint psy_ui_mouseevent_offset(const psy_ui_MouseEvent* self)
{
	return self->offset_;
}

INLINE void psy_ui_mouseevent_set_offset(psy_ui_MouseEvent* self,
	psy_ui_RealPoint offset)
{
	self->offset_ = offset;
}

INLINE uintptr_t psy_ui_mouseevent_button(const psy_ui_MouseEvent* self)
{
	return self->button_;
}

INLINE intptr_t psy_ui_mouseevent_delta(const psy_ui_MouseEvent* self)
{
	return self->delta_;
}

/*
** X/Y offset coordinate of the mouse pointer relative to the content edge of
** the current target
*/
psy_ui_RealPoint psy_ui_mouseevent_pt(const psy_ui_MouseEvent* self);

/* returns TRUE if the Ctrl key was pressed down when the event occurred. */
INLINE bool psy_ui_mouseevent_ctrl_key(const psy_ui_MouseEvent* self)
{
	return self->ctrl_key_;
}

/* returns TRUE if the Shift key was pressed down when the event occurred */
INLINE bool psy_ui_mouseevent_shift_key(const psy_ui_MouseEvent* self)
{
	return self->shift_key_;
}

/* Forward Handler for dataTransfer */
struct psy_Property;

/* psy_ui_DragEvent */
typedef struct psy_ui_DragEvent {
	psy_ui_MouseEvent mouse;	
	bool active;
	struct psy_Property* dataTransfer;
} psy_ui_DragEvent;

void psy_ui_dragevent_init(psy_ui_DragEvent*);
void psy_ui_dragevent_dispose(psy_ui_DragEvent*);

INLINE void psy_ui_dragevent_prevent_default(psy_ui_DragEvent* self)
{
	psy_ui_mouseevent_prevent_default(&self->mouse);
}

INLINE struct psy_ui_Component* psy_ui_dragevent_target(psy_ui_DragEvent* self)
{
	return psy_ui_mouseevent_target(&self->mouse);
}

INLINE psy_ui_Event* psy_ui_dragevent_base(psy_ui_DragEvent* self)
{
	return psy_ui_mouseevent_base(&self->mouse);
}

#ifdef __cplusplus
}
#endif

#endif /* psy_ui_EVENTS_H */

/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net
*/

#ifndef psy_ui_EVENTS_H
#define psy_ui_EVENTS_H

/* local */
#include "uidef.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum psy_ui_EventType {
	psy_ui_UNKNOWNEVENT  = 1,
	psy_ui_KEYDOWN       = 2,
	psy_ui_KEYUP         = 3,
	psy_ui_MOUSEDOWN     = 4,
	psy_ui_MOUSEUP       = 5,
	psy_ui_MOUSEMOVE     = 6,	
	psy_ui_MOUSEENTER	 = 7,
	psy_ui_MOUSELEAVE    = 8,
	psy_ui_DBLCLICK      = 9,
	psy_ui_DRAG          = 10,
	psy_ui_FOCUS         = 11,
	psy_ui_FOCUSOUT      = 12,
	psy_ui_RESIZE        = 13
} psy_ui_EventType;

/* Forward Handler for Event target */
struct psy_ui_Component;

/*
** psy_ui_Event base class
** An event gives additional data to an event method triggered by the ui imp
*/
typedef struct psy_ui_Event {
	psy_ui_EventType type;
	bool bubbles;
	bool default_prevented;
	struct psy_ui_Component* target;
	struct psy_ui_Component* currenttarget;
	uintptr_t timestamp;
} psy_ui_Event;

void psy_ui_event_init(psy_ui_Event*, psy_ui_EventType);
void psy_ui_event_init_stop_propagation(psy_ui_Event*, psy_ui_EventType);

INLINE bool psy_ui_event_default_prevented(const psy_ui_Event* self)
{
	return self->default_prevented;
}

INLINE void psy_ui_event_prevent_default(psy_ui_Event* self)
{
	self->default_prevented = TRUE;
}

INLINE bool psy_ui_event_bubbles(const psy_ui_Event* self)
{
	return self->bubbles;
}

INLINE void psy_ui_event_stop_propagation(psy_ui_Event* self)
{
	self->bubbles = FALSE;
}

/*
** The component that produced the event (the top of the bubble list).
** (opposed to currenttarget which is the current bubble component)
*/
INLINE struct psy_ui_Component* psy_ui_event_target(psy_ui_Event* self)
{
	return self->target;
}

/*
** The current component as the event is bubbling (self of event method).
** (opposed to target which produced the event)
*/
INLINE struct psy_ui_Component* psy_ui_event_currenttarget(psy_ui_Event* self)
{
	return self->currenttarget;
}

/* psy_ui_KeyboardEvent */
typedef struct psy_ui_KeyboardEvent {
	psy_ui_Event event;
	uint32_t keycode;
	intptr_t keydata;
	bool shift_key;
	bool ctrl_key;
	bool alt_key;
	bool repeat;	
} psy_ui_KeyboardEvent;

void psy_ui_keyboardevent_init(psy_ui_KeyboardEvent*);
void psy_ui_keyboardevent_init_all(psy_ui_KeyboardEvent*, uint32_t keycode, intptr_t keydata,
	bool shift, bool ctrl, bool alt, bool repeat);

INLINE void psy_ui_keyboardevent_prevent_default(psy_ui_KeyboardEvent* self)
{
	psy_ui_event_prevent_default(&self->event);
}

INLINE void psy_ui_keyboardevent_stop_propagation(psy_ui_KeyboardEvent* self)
{
	psy_ui_event_stop_propagation(&self->event);
}

INLINE struct psy_ui_Component* psy_ui_keyboardevent_target(psy_ui_KeyboardEvent* self)
{
	return psy_ui_event_target(&self->event);
}

INLINE psy_ui_Event* psy_ui_keyboardevent_base(psy_ui_KeyboardEvent* self)
{
	return &self->event;
}

INLINE void psy_ui_keyboardevent_settype(psy_ui_KeyboardEvent* self,
	psy_ui_EventType type)
{
	self->event.type = type;
}

/* psy_ui_MouseEvent */
typedef struct psy_ui_MouseEvent {
	psy_ui_Event event;
	psy_ui_RealPoint pt;
	psy_ui_RealPoint offset;
	uintptr_t button;
	intptr_t delta;
	bool shift_key;
	bool ctrl_key;	
} psy_ui_MouseEvent;


void psy_ui_mouseevent_init(psy_ui_MouseEvent*);
void psy_ui_mouseevent_init_all(psy_ui_MouseEvent*, psy_ui_RealPoint,
	psy_ui_RealPoint /* offset */, uintptr_t button, intptr_t delta,
	bool shift, bool ctrl);

INLINE void psy_ui_mouseevent_stop_propagation(psy_ui_MouseEvent* self)
{
	psy_ui_event_stop_propagation(&self->event);
}

INLINE void psy_ui_mouseevent_prevent_default(psy_ui_MouseEvent* self)
{
	psy_ui_event_prevent_default(&self->event);
}

INLINE struct psy_ui_Component* psy_ui_mouseevent_target(psy_ui_MouseEvent* self)
{
	return psy_ui_event_target(&self->event);
}

INLINE psy_ui_Event* psy_ui_mouseevent_base(psy_ui_MouseEvent* self)
{
	return &self->event;
}

INLINE void psy_ui_mouseevent_settype(psy_ui_MouseEvent* self,
	psy_ui_EventType type)
{
	self->event.type = type;
}

/*
** X/Y offset coordinate of the mouse pointer relative to the padding edge of
** the current target
*/
INLINE psy_ui_RealPoint psy_ui_mouseevent_offset(const psy_ui_MouseEvent* self)
{
	return self->offset;
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

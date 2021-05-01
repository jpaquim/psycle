// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#ifndef psy_ui_EVENTS_H
#define psy_ui_EVENTS_H

// local
#include "uigraphics.h"
//container
#include <signal.h>
#include <properties.h>

#include "../../detail/psydef.h"

#ifdef __cplusplus
extern "C" {
#endif

struct psy_ui_Component;

typedef struct {
	uint32_t keycode;
	intptr_t keydata;
	bool shift;
	bool ctrl;
	bool alt;
	int repeat;
	bool bubble;
	bool preventdefault;
	struct psy_ui_Component* target;
} psy_ui_KeyEvent;

void psy_ui_keyevent_init(psy_ui_KeyEvent*, uint32_t keycode, intptr_t keydata,
	bool shift, bool ctrl, bool alt, int repeat);
void psy_ui_keyevent_stoppropagation(psy_ui_KeyEvent*);
void psy_ui_keyevent_preventdefault(psy_ui_KeyEvent*);

INLINE void psy_ui_keyevent_settarget(psy_ui_KeyEvent* self, struct psy_ui_Component* target)
{
	self->target = target;
}

typedef struct psy_ui_MouseEvent {
	psy_ui_RealPoint pt;
	uintptr_t button;
	intptr_t delta;
	bool shift;
	bool ctrl;
	bool bubble;
	bool preventdefault;
	struct psy_ui_Component* target;
} psy_ui_MouseEvent;

void psy_ui_mouseevent_init(psy_ui_MouseEvent*, double x, double y, uintptr_t button,
	intptr_t delta, bool shift, bool ctrl);
void psy_ui_mouseevent_stoppropagation(psy_ui_MouseEvent*);
struct psy_ui_Component* psy_ui_mouseevent_target(psy_ui_MouseEvent*);
void psy_ui_mouseevent_settarget(psy_ui_MouseEvent*, struct psy_ui_Component* target);

INLINE void psy_ui_mouseevent_preventdefault(psy_ui_MouseEvent* self)
{
	self->preventdefault = TRUE;
}

struct psy_ui_Component;

typedef struct psy_ui_DragEvent {
	psy_ui_MouseEvent mouse;
	struct psy_ui_Component* target;
	bool active;
	psy_Property* dataTransfer;
	bool preventdefault;
} psy_ui_DragEvent;

void psy_ui_dragevent_init(psy_ui_DragEvent*);
void psy_ui_dragevent_dispose(psy_ui_DragEvent*);

#ifdef __cplusplus
}
#endif

#endif /* psy_ui_EVENTS_H */

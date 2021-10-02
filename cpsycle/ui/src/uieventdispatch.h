/* This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net
*/

#ifndef psy_ui_EVENTDISPATCH_H
#define psy_ui_EVENTDISPATCH_H

/* local */
#include "uievents.h"
#include "uidefaults.h"

#ifdef __cplusplus
extern "C" {
#endif

struct psy_ui_Component;

typedef struct psy_ui_EventDispatch {
	psy_List* targetids;
	struct psy_ui_Component* eventretarget;
	bool handledoubleclick;
	uintptr_t lastbutton;
	uintptr_t lastbuttontimestamp;		
} psy_ui_EventDispatch;

void psy_ui_eventdispatch_init(psy_ui_EventDispatch*);
void psy_ui_eventdispatch_dispose(psy_ui_EventDispatch*);

void psy_ui_eventdispatch_focuslost(psy_ui_EventDispatch*,
	struct psy_ui_Component*);
void psy_ui_eventdispatch_keydown(psy_ui_EventDispatch*,
	struct psy_ui_Component*, psy_ui_KeyboardEvent*);
void psy_ui_eventdispatch_keyup(psy_ui_EventDispatch*,
	struct psy_ui_Component*, psy_ui_KeyboardEvent*);
void psy_ui_eventdispatch_buttondown(psy_ui_EventDispatch*,
	struct psy_ui_Component*, psy_ui_MouseEvent*);
void psy_ui_eventdispatch_buttonup(psy_ui_EventDispatch*,
	struct psy_ui_Component*, psy_ui_MouseEvent*);
void psy_ui_eventdispatch_mousemove(psy_ui_EventDispatch*,
	struct psy_ui_Component*, psy_ui_MouseEvent*);
void psy_ui_eventdispatch_doubleclick(psy_ui_EventDispatch*,
	struct psy_ui_Component*, psy_ui_MouseEvent*);
void psy_ui_eventdispatch_size(psy_ui_EventDispatch*,
	struct psy_ui_Component*, psy_ui_Size);
void psy_ui_eventdispatch_timer(psy_ui_EventDispatch*,
	struct psy_ui_Component*, uintptr_t timerid);

#ifdef __cplusplus
}
#endif

#endif /* psy_ui_EVENTDISPATCH_H */

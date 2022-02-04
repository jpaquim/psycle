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
	bool handledoubleclick;
	uintptr_t lastbutton;
	uintptr_t lastbuttontimestamp;		
} psy_ui_EventDispatch;

void psy_ui_eventdispatch_init(psy_ui_EventDispatch*);
void psy_ui_eventdispatch_dispose(psy_ui_EventDispatch*);

INLINE void psy_ui_eventdispatch_handle_doubleclick(psy_ui_EventDispatch* self)
{
	self->handledoubleclick = TRUE;
}

INLINE void psy_ui_eventdispatch_disable_handle_doubleclick(
	psy_ui_EventDispatch* self)
{
	self->handledoubleclick = FALSE;
}

void psy_ui_eventdispatch_send(psy_ui_EventDispatch*,
	struct psy_ui_Component*, psy_ui_Event*);

#ifdef __cplusplus
}
#endif

#endif /* psy_ui_EVENTDISPATCH_H */

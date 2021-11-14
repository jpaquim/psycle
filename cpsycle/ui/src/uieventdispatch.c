/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "uieventdispatch.h"
#include "uiapp.h"
#include "timers.h"
/* local */
#include "uicomponent.h" 
/* platform */
#include "../../detail/os.h"

/* prototypes */
static bool psy_ui_eventdispatch_sendtoparent(psy_ui_EventDispatch*,
	psy_ui_Component*, psy_ui_Event*);
static psy_ui_Component* psy_ui_eventdispatch_eventtarget(
	psy_ui_EventDispatch*, psy_ui_Component* component);

/* implementation*/
void psy_ui_eventdispatch_init(psy_ui_EventDispatch* self)
{
	self->targetids = NULL;
	self->eventretarget = NULL;	
	self->lastbutton = 0;
	self->lastbuttontimestamp = 0;
	self->handledoubleclick = TRUE;
}

void psy_ui_eventdispatch_dispose(psy_ui_EventDispatch* self)
{
	psy_list_free(self->targetids);
	self->targetids = NULL;
}

void psy_ui_eventdispatch_send(psy_ui_EventDispatch* self,
	psy_ui_Component* component, psy_ui_Event* ev)
{
	uintptr_t eventtime;
	bool preventbubble;

	assert(component);
	assert(ev);

	ev->target = psy_ui_eventdispatch_eventtarget(self, component);
	ev->currenttarget = component;
	eventtime = 0;
	preventbubble = FALSE;
	switch (ev->type) {
	case psy_ui_RESIZE:
		if (component->containeralign &&
			component->containeralign->containeralign != psy_ui_CONTAINER_ALIGN_NONE) {
			psy_ui_component_align(component);
		}
		component->vtable->onsize(component);
		if (psy_ui_component_overflow(component) != psy_ui_OVERFLOW_HIDDEN) {
			psy_ui_component_updateoverflow(component);
		}
		psy_signal_emit(&component->signal_size, component, 0);
		break;
	case psy_ui_FOCUSOUT:
		component->vtable->onfocuslost(component);
		psy_signal_emit(&component->signal_focuslost, component, 0);
		break;
	case psy_ui_KEYDOWN:
		component->vtable->onkeydown(component, (psy_ui_KeyboardEvent*)ev);
		psy_signal_emit(&component->signal_keydown, component, 1, ev);
		break;
	case psy_ui_KEYUP:
		component->vtable->onkeyup(component, (psy_ui_KeyboardEvent*)ev);
		psy_signal_emit(&component->signal_keyup, component, 1, ev);
		break;	
	case psy_ui_MOUSEDOWN: {			
		if (ev->timestamp == 0) { /* CurrentTime */
			eventtime = self->lastbuttontimestamp;
		} else {
			eventtime = ev->timestamp;
		}
		if (self->handledoubleclick) {
			if (self->lastbutton == ((psy_ui_MouseEvent*)(ev))->button &&
					(eventtime - self->lastbuttontimestamp) < 500) {												
				ev->type = psy_ui_DBLCLICK;
				component->imp->vtable->dev_mousedoubleclick(component->imp,
					(psy_ui_MouseEvent*)ev);
				if (ev->bubbles != FALSE) {
					component->vtable->onmousedoubleclick(component,
						(psy_ui_MouseEvent*)ev);
					psy_signal_emit(&component->signal_mousedoubleclick, component,
						1, ev);
				}				
			} else {
				self->lastbutton = ((psy_ui_MouseEvent*)(ev))->button;
				component->imp->vtable->dev_mousedown(component->imp,
					(psy_ui_MouseEvent*)(ev));
				if (ev->bubbles != FALSE) {
					component->vtable->onmousedown(component,
						(psy_ui_MouseEvent*)(ev));
					psy_signal_emit(&component->signal_mousedown, component, 1,
						(psy_ui_MouseEvent*)(ev));
				}					
			}
		} else {
			component->imp->vtable->dev_mousedown(component->imp,
				(psy_ui_MouseEvent*)(ev));
			if (ev->bubbles != FALSE) {
				component->vtable->onmousedown(component,
					(psy_ui_MouseEvent*)(ev));
				psy_signal_emit(&component->signal_mousedown, component, 1,
					(psy_ui_MouseEvent*)(ev));
			}
		}
		break; }
	case psy_ui_MOUSEUP:
		component->imp->vtable->dev_mouseup(component->imp,
			(psy_ui_MouseEvent*)ev);
		if (ev->bubbles != FALSE) {
			component->vtable->onmouseup(component,
				(psy_ui_MouseEvent*)ev);
			psy_signal_emit(&component->signal_mouseup, component, 1,
				(psy_ui_MouseEvent*)ev);
		}
		break;
	case psy_ui_DBLCLICK:
		component->imp->vtable->dev_mousedoubleclick(component->imp,
			(psy_ui_MouseEvent*)ev);
		if (ev->bubbles != FALSE) {
			component->vtable->onmousedoubleclick(component,
				(psy_ui_MouseEvent*)ev);
			psy_signal_emit(&component->signal_mousedoubleclick, component,
				1, ev);
		}
		break;
	case psy_ui_MOUSEMOVE: {
		psy_ui_App* app;

		app = psy_ui_app();
		component->imp->vtable->dev_mousemove(component->imp,
			(psy_ui_MouseEvent*)ev);
		if (ev->bubbles != FALSE && app->dragevent.active) {
			component->vtable->onmousemove(component, (psy_ui_MouseEvent*)ev);
			psy_signal_emit(&component->signal_mousemove, component, 1,
				(psy_ui_MouseEvent*)ev);
		}
		if (!psy_ui_app()->dragevent.active) {
			return;
		}
		break; }
	case psy_ui_MOUSEENTER:
		component->imp->vtable->dev_mouseenter(component->imp);
		preventbubble = TRUE;
		break;
	case psy_ui_MOUSELEAVE:
		component->imp->vtable->dev_mouseleave(component->imp);
		preventbubble = TRUE;
		break;
	default:
		preventbubble = TRUE;		
	}	
	if (!preventbubble && ev->bubbles != FALSE) {
		psy_ui_eventdispatch_sendtoparent(self, component, ev);
	} else {
		psy_list_free(self->targetids);
		self->targetids = NULL;
	}
	if (ev->type == psy_ui_MOUSEDOWN) {
		self->lastbuttontimestamp = eventtime;
	}
	if (ev->type == psy_ui_DBLCLICK) {
		self->lastbutton = 0;
		self->lastbuttontimestamp = 0;
	}
}

void psy_ui_eventdispatch_timer(psy_ui_EventDispatch* self,
	psy_ui_Component* component, uintptr_t timerid)
{
	assert(component);

	component->vtable->ontimer(component, timerid);
	psy_signal_emit(&component->signal_timer,
		component, 1, timerid);
}

bool psy_ui_eventdispatch_sendtoparent(psy_ui_EventDispatch* self,
	psy_ui_Component* component, psy_ui_Event* ev)
{	
	if (psy_ui_component_parent(component)) {
		psy_list_append(&self->targetids,
			(void*)component->imp->vtable->dev_platform_handle(component->imp));
		self->eventretarget = component;
		psy_ui_app()->imp->vtable->dev_sendevent(
			psy_ui_app()->imp, psy_ui_component_parent(component),
			ev);
		self->eventretarget = 0;
		if (ev->type == psy_ui_MOUSEUP && psy_ui_app()->dragevent.active) {
			psy_ui_app_stopdrag(psy_ui_app());
		}
		return TRUE;
	} else {
		psy_list_free(self->targetids);
		self->targetids = NULL;
	}
	self->eventretarget = 0;	
	return FALSE;
}

psy_ui_Component* psy_ui_eventdispatch_eventtarget(psy_ui_EventDispatch* self,
	psy_ui_Component* component)
{
	if (self->targetids) {
		uintptr_t targethwnd;
		psy_ui_Component* target;
		
		targethwnd =  (self->targetids)
			? (uintptr_t)self->targetids->entry
			: 0;
		target = psy_ui_app()->imp->vtable->dev_component(
			psy_ui_app()->imp, targethwnd);			
		if (target) {
			return target;
		}
	}
	return component;
}

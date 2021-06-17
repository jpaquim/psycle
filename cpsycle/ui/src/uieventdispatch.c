/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "uieventdispatch.h"
#include "uiapp.h"
/* local */
#include "uicomponent.h" 


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
}

void psy_ui_eventdispatch_dispose(psy_ui_EventDispatch* self)
{
	psy_list_free(self->targetids);
	self->targetids = NULL;
}

void psy_ui_eventdispatch_focuslost(psy_ui_EventDispatch* self,
	psy_ui_Component* component)
{
	assert(component);

	component->vtable->onfocuslost(component);
	psy_signal_emit(&component->signal_focuslost, component, 0);
}

void psy_ui_eventdispatch_keydown(psy_ui_EventDispatch* self,
	psy_ui_Component* component, psy_ui_KeyboardEvent* ev)
{
	assert(component);
	assert(ev);
	
	ev->event.target = psy_ui_eventdispatch_eventtarget(self, component);
	component->vtable->onkeydown(component, ev);
	psy_signal_emit(&component->signal_keydown, component, 1, ev);	
	if (ev->event.bubbles != FALSE) {
		psy_ui_eventdispatch_sendtoparent(self, component, &ev->event);		
	} else {
		psy_list_free(self->targetids);
		self->targetids = NULL;
	}
}

void psy_ui_eventdispatch_keyup(psy_ui_EventDispatch* self,
	psy_ui_Component* component, psy_ui_KeyboardEvent* ev)
{
	assert(component);

	ev->event.target = psy_ui_eventdispatch_eventtarget(self, component);
	component->vtable->onkeyup(component, ev);
	psy_signal_emit(&component->signal_keyup, component, 1, ev);
	if (ev->event.bubbles != FALSE) {
		psy_ui_eventdispatch_sendtoparent(self, component, &ev->event);
	} else {
		psy_list_free(self->targetids);
		self->targetids = NULL;
	}
}

void psy_ui_eventdispatch_buttondown(psy_ui_EventDispatch* self,
	struct psy_ui_Component* component, psy_ui_MouseEvent* ev)
{
	assert(component);

	ev->event.type = psy_ui_ButtonPress;
	ev->event.target = psy_ui_eventdispatch_eventtarget(self, component);
	component->imp->vtable->dev_mousedown(component->imp, ev);
	if (ev->event.bubbles != FALSE) {		
		component->vtable->onmousedown(component, ev);
		psy_signal_emit(&component->signal_mousedown, component, 1, ev);
	}		
	if (ev->event.bubbles != FALSE) {
		bool bubble;

		bubble = psy_ui_eventdispatch_sendtoparent(self, component, &ev->event);		
	}
}

void psy_ui_eventdispatch_buttonup(psy_ui_EventDispatch* self,
	struct psy_ui_Component* component, psy_ui_MouseEvent* ev)
{
	assert(component);

	ev->event.type = psy_ui_ButtonRelease;
	ev->event.target = psy_ui_eventdispatch_eventtarget(self, component);
	component->imp->vtable->dev_mouseup(component->imp, ev);	
	if (ev->event.bubbles != FALSE) {
		component->vtable->onmouseup(component, ev);
		psy_signal_emit(&component->signal_mouseup, component, 1, ev);
	}
	if (ev->event.bubbles != FALSE) {
		bool bubble;

		bubble = psy_ui_eventdispatch_sendtoparent(self, component, &ev->event);
		if (!bubble) {
			psy_ui_app_stopdrag(psy_ui_app());
		}
	} else {
		psy_ui_app_stopdrag(psy_ui_app());
	}

}

void psy_ui_eventdispatch_mousemove(psy_ui_EventDispatch* self,
	struct psy_ui_Component* component, psy_ui_MouseEvent* ev)
{
	assert(component);

	ev->event.type = psy_ui_MotionNotify;
	ev->event.target = psy_ui_eventdispatch_eventtarget(self, component);
	component->imp->vtable->dev_mousemove(component->imp, ev);
	if (ev->event.bubbles != FALSE) {
		component->vtable->onmousemove(component, ev);
		psy_signal_emit(&component->signal_mousemove, component, 1, ev);
	}
	if (ev->event.bubbles != FALSE) {
		bool bubble;

		bubble = psy_ui_eventdispatch_sendtoparent(self, component, &ev->event);
	} else {
		psy_list_free(self->targetids);
		self->targetids = NULL;
	}
}

void psy_ui_eventdispatch_doubleclick(psy_ui_EventDispatch* self,
	struct psy_ui_Component* component, psy_ui_MouseEvent* ev)
{
	assert(component);

	ev->event.type = psy_ui_DoubleClick;
	ev->event.target = psy_ui_eventdispatch_eventtarget(self, component);	
	component->imp->vtable->dev_mousedoubleclick(component->imp, ev);
	if (ev->event.bubbles != FALSE) {
		component->vtable->onmousedoubleclick(component, ev);
		psy_signal_emit(&component->signal_mousedoubleclick, component, 1, ev);
	}
	if (ev->event.bubbles != FALSE) {
		bool bubble;

		bubble = psy_ui_eventdispatch_sendtoparent(self, component, &ev->event);
	} else {
		psy_list_free(self->targetids);
		self->targetids = NULL;
	}
}

void psy_ui_eventdispatch_size(psy_ui_EventDispatch* self,
	psy_ui_Component* component, psy_ui_Size size)
{
	assert(component);		
	
	if (component->containeralign &&
			component->containeralign->containeralign != psy_ui_CONTAINER_ALIGN_NONE) {
		psy_ui_component_align(component);
	}		
	component->vtable->onsize(component, &size);
	if (psy_ui_component_overflow(component) != psy_ui_OVERFLOW_HIDDEN) {
		psy_ui_component_updateoverflow(component);
	}
	psy_signal_emit(&component->signal_size, component, 1,
		(void*)&size);	
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

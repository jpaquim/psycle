/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "uieventdispatch.h"
/* local */
#include "uiapp.h"
#include "uicomponent.h" 
#include "timers.h"


/* prototypes */
static void psy_ui_eventdispatch_handlemouseevent(psy_ui_EventDispatch*,
	psy_ui_MouseEvent*, psy_ui_Component*);
static void psy_ui_eventdispatch_handlemouseenter(psy_ui_EventDispatch*,
	psy_ui_Component*);
static void psy_ui_eventdispatch_handlemouseleave(psy_ui_EventDispatch*,
	psy_ui_Component*);
static void psy_ui_eventdispatch_handle_wheel(psy_ui_EventDispatch*,
	psy_ui_Component*, psy_ui_MouseEvent*);
static void psy_ui_eventdispatch_bubble(psy_ui_EventDispatch*,
	psy_ui_Component* component, psy_ui_Event*);
static psy_ui_Component* psy_ui_eventdispatch_target(psy_ui_EventDispatch*,
	psy_ui_Component*, psy_ui_RealPoint*);
static void psy_ui_eventdispatch_notify(psy_ui_EventDispatch*,
	psy_ui_Component*, psy_ui_Event*);

/* implementation*/
void psy_ui_eventdispatch_init(psy_ui_EventDispatch* self)
{	
	self->lastbutton = 0;
	self->lastbuttontimestamp = 0;
	self->handledoubleclick = TRUE;
}

void psy_ui_eventdispatch_dispose(psy_ui_EventDispatch* self)
{
}

void psy_ui_eventdispatch_send(psy_ui_EventDispatch* self,
	psy_ui_Component* component, psy_ui_Event* ev)
{	

	assert(component);
	assert(ev);

	psy_ui_event_settarget(ev, component);
	psy_ui_event_setcurrenttarget(ev, component);
	switch (psy_ui_event_type(ev)) {
	case psy_ui_RESIZE:
		if (psy_ui_component_hasalign(component)) {
			psy_ui_component_align(component);
		}
		psy_ui_eventdispatch_notify(self, component, ev);
		if (psy_ui_component_overflow(component) != psy_ui_OVERFLOW_HIDDEN) {
			psy_ui_component_updateoverflow(component);
		}		
		break;	
	case psy_ui_MOUSEDOWN:
	case psy_ui_MOUSEUP:
	case psy_ui_DBLCLICK:
	case psy_ui_MOUSEMOVE:		
		psy_ui_eventdispatch_handlemouseevent(self, (psy_ui_MouseEvent*)ev,
			component);
		break;
	case psy_ui_MOUSEENTER:
		psy_ui_eventdispatch_handlemouseenter(self, component);				
		break;
	case psy_ui_MOUSELEAVE:
		psy_ui_eventdispatch_handlemouseleave(self, component);		
		break;
	case psy_ui_WHEEL:
		psy_ui_eventdispatch_handle_wheel(self, component,
			(psy_ui_MouseEvent*)ev);		
		break;
	default:
		psy_ui_eventdispatch_bubble(self, component, ev);
		break;
	}	
}

void psy_ui_eventdispatch_handlemouseevent(psy_ui_EventDispatch* self,
	psy_ui_MouseEvent* ev, psy_ui_Component* component)
{	
	uintptr_t eventtime;	
	psy_ui_RealPoint offset;
	
	eventtime = 0;	
	offset = psy_ui_mouseevent_offset(ev);
	component = psy_ui_eventdispatch_target(self, component, &offset);
	psy_ui_mouseevent_setoffset(ev, offset);
	if (psy_ui_event_type(&ev->event) == psy_ui_MOUSEMOVE) {
		psy_ui_eventdispatch_handlemouseenter(self, component);
	} else if (psy_ui_event_type(&ev->event) == psy_ui_MOUSEDOWN) {
		if (psy_ui_event_timestamp(psy_ui_mouseevent_base(ev)) == psy_ui_CURRENT_TIME) {
			eventtime = self->lastbuttontimestamp;
		} else {
			eventtime = psy_ui_event_timestamp(psy_ui_mouseevent_base(ev));
		}
		if (self->handledoubleclick) {
			if (self->lastbutton == psy_ui_mouseevent_button(ev) &&
				(eventtime - self->lastbuttontimestamp) < 500) {
				psy_ui_mouseevent_settype(ev, psy_ui_DBLCLICK);
			} else {
				self->lastbutton = psy_ui_mouseevent_button(ev);
			}
		}
	}	
	psy_ui_eventdispatch_bubble(self, component, &ev->event);
	if (psy_ui_event_type(&ev->event) == psy_ui_MOUSEDOWN) {
		self->lastbuttontimestamp = eventtime;
	} else if (psy_ui_event_type(&ev->event) == psy_ui_DBLCLICK) {
		self->lastbutton = 0;
		self->lastbuttontimestamp = 0;
	}
	if (psy_ui_event_type(&ev->event) == psy_ui_DBLCLICK ||
		psy_ui_event_type(&ev->event) == psy_ui_MOUSEUP) {
		psy_ui_app()->mousetracking = FALSE;
	}		
}

void psy_ui_eventdispatch_handlemouseenter(psy_ui_EventDispatch* self,
	psy_ui_Component* component)
{
	if (psy_ui_app_hover(psy_ui_app()) != component) {
		psy_ui_Event ev;

		if (psy_ui_app_hover(psy_ui_app())) {
			psy_ui_event_init(&ev, psy_ui_MOUSELEAVE);
			psy_ui_eventdispatch_notify(self, psy_ui_app_hover(psy_ui_app()),
				&ev);
		}
		psy_ui_app_sethover(psy_ui_app(), component);
		psy_ui_event_init(&ev, psy_ui_MOUSEENTER);
		psy_ui_eventdispatch_notify(self, psy_ui_app_hover(psy_ui_app()), &ev);
	}
}

void psy_ui_eventdispatch_handlemouseleave(psy_ui_EventDispatch* self,
	psy_ui_Component* component)
{
	psy_ui_app()->mousetracking = FALSE;
	if (psy_ui_app_hover(psy_ui_app())) {
		psy_ui_app_hover(psy_ui_app())->vtable->onmouseleave(
			psy_ui_app_hover(psy_ui_app()));
		psy_signal_emit(&psy_ui_app_hover(psy_ui_app())->signal_mouseleave,
			psy_ui_app_hover(psy_ui_app()), 0);
		psy_ui_app_sethover(psy_ui_app(), NULL);
	}
	component->vtable->onmouseleave(component);
	psy_signal_emit(&component->signal_mouseleave, component, 0);
}

void psy_ui_eventdispatch_handle_wheel(psy_ui_EventDispatch* self,
	psy_ui_Component* component, psy_ui_MouseEvent* ev)
{
	bool preventdefault;
	intptr_t delta;
	psy_ui_RealPoint offset;
	psy_ui_Component* curr;

	delta = psy_ui_mouseevent_delta(ev);
	offset = psy_ui_mouseevent_offset(ev);
	curr = component = psy_ui_eventdispatch_target(self, component, &offset);
	psy_ui_mouseevent_setoffset(ev, offset);
	while (curr) {		
		psy_ui_RealRectangle r;

		psy_ui_event_setcurrenttarget(psy_ui_mouseevent_base(ev),
			curr);
		curr->vtable->onmousewheel(curr, ev);
		psy_signal_emit(&curr->signal_mousewheel, curr, 1, ev);
		preventdefault = psy_ui_event_default_prevented(&ev->event);
		if (!preventdefault && psy_ui_component_wheelscroll(curr) > 0) {
			if (psy_ui_app()->deltaperline != 0) {
				psy_ui_app()->accumwheeldelta += (short)delta; /* 120 or -120 */
				while (psy_ui_app()->accumwheeldelta >= psy_ui_app()->deltaperline) {
					double pos;
					psy_ui_IntPoint scrollrange;
					double scrolltoppx;
					const psy_ui_TextMetric* tm;

					tm = psy_ui_component_textmetric(curr);
					scrollrange = psy_ui_component_verticalscrollrange(curr);
					scrolltoppx = psy_ui_component_scrolltop_px(curr);
					pos = (scrolltoppx / psy_ui_component_scrollstep_height_px(curr)) -
						psy_ui_component_wheelscroll(curr);
					if (pos < (double)scrollrange.x) {
						pos = (double)scrollrange.x;
					}
					psy_ui_component_setscrolltop(curr,
						psy_ui_mul_value_real(
							psy_ui_component_scrollstep_height(curr), pos));
					psy_ui_app()->accumwheeldelta -= psy_ui_app()->deltaperline;
				}
				while (psy_ui_app()->accumwheeldelta <= -psy_ui_app()->deltaperline)
				{
					double pos;
					psy_ui_IntPoint scrollrange;
					double scrolltoppx;
					const psy_ui_TextMetric* tm;

					tm = psy_ui_component_textmetric(curr);
					scrollrange = psy_ui_component_verticalscrollrange(curr);
					scrolltoppx = psy_ui_component_scrolltop_px(curr);
					pos = (scrolltoppx / psy_ui_component_scrollstep_height_px(curr)) +
						psy_ui_component_wheelscroll(curr);
					if (pos > (double)scrollrange.y) {
						pos = (double)scrollrange.y;
					}
					psy_ui_component_setscrolltop(curr,
						psy_ui_mul_value_real(
							psy_ui_component_scrollstep_height(curr), pos));
					psy_ui_app()->accumwheeldelta += psy_ui_app()->deltaperline;
				}
			}
		}
		if (!psy_ui_event_bubbles(&ev->event)) {
			break;
		}
		r = psy_ui_component_position(curr);
		psy_ui_realpoint_add(&offset, psy_ui_realrectangle_topleft(&r));
		psy_ui_mouseevent_setoffset(ev, offset);
		curr = psy_ui_component_parent(curr);
	}
	psy_ui_event_stop_propagation(&ev->event);
}

void psy_ui_eventdispatch_bubble(psy_ui_EventDispatch* self,
	psy_ui_Component* component, psy_ui_Event* ev)
{
	psy_ui_Component* curr;		
	
	curr = component;
	while (curr) {						
		psy_ui_eventdispatch_notify(self, curr, ev);
		if (psy_ui_event_type(ev) >= psy_ui_MOUSEDOWN &&
				psy_ui_event_type(ev) <= psy_ui_DBLCLICK) {
			psy_ui_MouseEvent* mouse_event;
			psy_ui_RealPoint offset;
			psy_ui_RealRectangle r;
			
			mouse_event = (psy_ui_MouseEvent*)ev;
			offset = psy_ui_mouseevent_offset(mouse_event);
			r = psy_ui_component_position(curr);
			psy_ui_realpoint_add(&offset, psy_ui_realrectangle_topleft(&r));
			psy_ui_mouseevent_setoffset(mouse_event, offset);
		}
		if (!psy_ui_event_bubbles(ev)) {
			break;
		}
		curr = psy_ui_component_parent(curr);
	};
	psy_ui_event_stop_propagation(ev);
}

psy_ui_Component* psy_ui_eventdispatch_target(psy_ui_EventDispatch* self,
	psy_ui_Component* component, psy_ui_RealPoint* pt)
{
	psy_ui_Component* curr;
	uintptr_t index;	

	if (psy_ui_app_capture(psy_ui_app())) {
		curr = psy_ui_app_capture(psy_ui_app());
		while (curr && curr != component) {
			psy_ui_RealRectangle r;			

			r = psy_ui_component_position(curr);
			psy_ui_realpoint_sub(pt, psy_ui_realrectangle_topleft(&r));		
			curr = psy_ui_component_parent(curr);			
		}
		return psy_ui_app_capture(psy_ui_app());
	}
	curr = component;
	while (curr) {
		component = curr;
		curr = psy_ui_component_intersect(curr, *pt, &index);
		if (curr && (psy_ui_component_islightweight(curr))) {
			psy_ui_RealRectangle r;

			r = psy_ui_component_position(curr);
			psy_ui_realpoint_sub(pt, psy_ui_realrectangle_topleft(&r));
		} else {
			break;
		}
	}
	return component;
}

void psy_ui_eventdispatch_notify(psy_ui_EventDispatch* self,
	psy_ui_Component* component, psy_ui_Event* ev)
{	
	psy_ui_event_setcurrenttarget(ev, component);	
	switch (psy_ui_event_type(ev)) {
	case psy_ui_DBLCLICK:
		component->vtable->onmousedoubleclick(component, (psy_ui_MouseEvent*)ev);
		psy_signal_emit(&component->signal_mousedoubleclick,
			component, 1, ev);
		break;
	case psy_ui_MOUSEDOWN:
		component->vtable->onmousedown(component, (psy_ui_MouseEvent*)ev);
		psy_signal_emit(&component->signal_mousedown, component, 1, ev);
		break;
	case psy_ui_MOUSEUP:
		component->vtable->onmouseup(component, (psy_ui_MouseEvent*)ev);
		psy_signal_emit(&component->signal_mouseup, component, 1, ev);
		break;
	case psy_ui_MOUSEMOVE:
		component->vtable->onmousemove(component, (psy_ui_MouseEvent*)ev);
		psy_signal_emit(&component->signal_mousemove, component, 1, ev);
		break;
	case psy_ui_KEYDOWN:
		component->vtable->onkeydown(component, (psy_ui_KeyboardEvent*)ev);
		psy_signal_emit(&component->signal_keydown, component, 1, ev);
		break;
	case psy_ui_KEYUP:
		component->vtable->onkeyup(component, (psy_ui_KeyboardEvent*)ev);
		psy_signal_emit(&component->signal_keyup, component, 1, ev);
		break;
	case psy_ui_FOCUS:
		if (component->imp) {
			component->imp->vtable->dev_setfocus(component->imp);
		}
		component->vtable->onfocus(component);
		psy_signal_emit(&component->signal_focus, component, 0);
		break;
	case psy_ui_FOCUSOUT:
		component->vtable->onfocuslost(component);
		psy_signal_emit(&component->signal_focuslost, component, 0);
		break;	
	case psy_ui_MOUSEENTER:
		component->vtable->onmouseenter(component);
		psy_signal_emit(&component->signal_mouseenter, component, 0);
		break;
	case psy_ui_MOUSELEAVE:
		component->vtable->onmouseleave(component);
		psy_signal_emit(&component->signal_mouseleave, component, 0);
		break;
	case psy_ui_RESIZE:		
		component->vtable->onsize(component);
		psy_signal_emit(&component->signal_size, component, 0);		
		break;
	default:
		break;
	}		
}

/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "viewframe.h"
/* host */
#include "viewindex.h"
/* platform */
#include "../../detail/portable.h"


/* prototypes */
static bool viewframe_on_close(ViewFrame*);
static void viewframe_on_key_down(ViewFrame*, psy_ui_KeyboardEvent*);
static void viewframe_on_key_up(ViewFrame*, psy_ui_KeyboardEvent*);
static void viewframe_delegate_keyboard(ViewFrame*, intptr_t message,
	psy_ui_KeyboardEvent*);
static void viewframe_on_language_changed(ViewFrame*);

/* vtable */
static psy_ui_ComponentVtable vtable;
static bool vtable_initialized = FALSE;

static void vtable_init(ViewFrame* self)
{
	if (!vtable_initialized) {
		vtable = *(self->component.vtable);
		vtable.onclose =
			(psy_ui_fp_component_onclose)
			viewframe_on_close;
		vtable.on_key_down =
			(psy_ui_fp_component_on_key_event)
			viewframe_on_key_down;
		vtable.onkeyup =
			(psy_ui_fp_component_on_key_event)
			viewframe_on_key_up;
		vtable.onlanguagechanged =
			(psy_ui_fp_component_onlanguagechanged)
			viewframe_on_language_changed;
		vtable_initialized = TRUE;
	}
	psy_ui_component_set_vtable(viewframe_base(self), &vtable);
}

/* implementation */
void viewframe_init(ViewFrame* self, psy_ui_Component* parent,
	psy_ui_Component* page, psy_ui_Notebook* dock,
	psy_EventDriver* kbd)
{
	psy_ui_frame_init(&self->component, parent);
	vtable_init(self);
	psy_ui_component_doublebuffer(viewframe_base(self));
	self->dock = dock;
	self->kbd = kbd;
	psy_ui_component_init(&self->pane, &self->component, &self->component);
	psy_ui_component_set_align(&self->pane, psy_ui_ALIGN_CLIENT);
	viewframe_float(self, page);	
}

ViewFrame* viewframe_alloc(void)
{
	return (ViewFrame*)malloc(sizeof(ViewFrame));
}

ViewFrame* viewframe_allocinit(psy_ui_Component* parent,
	psy_ui_Component* page, psy_ui_Notebook* dock,
	psy_EventDriver* kbd)
{
	ViewFrame* rv;

	rv = viewframe_alloc();
	if (rv) {
		viewframe_init(rv, parent, page, dock, kbd);
		psy_ui_component_deallocate_after_destroyed(&rv->component);
	}
	return rv;
}

bool viewframe_on_close(ViewFrame* self)
{
	viewframe_dock(self);
	return TRUE;
}

void viewframe_float(ViewFrame* self, psy_ui_Component* component)
{
	if (!component) {
		return;
	}	
	psy_ui_component_parent(component);
	psy_ui_component_set_parent(component, &self->pane);
	psy_ui_component_set_align(component, psy_ui_ALIGN_CLIENT);
	psy_ui_component_set_title(&self->component, 
		psy_ui_translate(psy_ui_component_title(component)));
	psy_ui_component_resize(&self->component,
		psy_ui_size_make_em(120.0, 30.0));
	psy_ui_component_show(&self->component);
	psy_ui_notebook_select_by_component_id(self->dock, VIEW_ID_FLOATED);	
}

void viewframe_dock(ViewFrame* self)
{
	psy_ui_Component* page;

	if (!self->dock) {
		return;
	}
	page = psy_ui_component_at(&self->pane, 0);
	if (page) {
		psy_ui_component_set_parent(page, &self->dock->component);
		psy_ui_component_set_align(page, psy_ui_ALIGN_CLIENT);
		psy_ui_notebook_select_by_component_id(self->dock,
			psy_ui_component_id(page));
	}
}

void viewframe_on_key_down(ViewFrame* self, psy_ui_KeyboardEvent* ev)
{	
	if (psy_ui_keyboardevent_keycode(ev) == psy_ui_KEY_ESCAPE) {
		return;
	}
	viewframe_delegate_keyboard(self, psy_EVENTDRIVER_PRESS, ev);
}

void viewframe_on_key_up(ViewFrame* self, psy_ui_KeyboardEvent* ev)
{
	if (psy_ui_keyboardevent_keycode(ev) == psy_ui_KEY_ESCAPE) {
		return;
	}
	viewframe_delegate_keyboard(self, psy_EVENTDRIVER_RELEASE, ev);
}

/* delegate keyboard events to the keyboard driver */
void viewframe_delegate_keyboard(ViewFrame* self, intptr_t message,
	psy_ui_KeyboardEvent* ev)
{
	if (!self->kbd) {
		return;
	}
	psy_eventdriver_write(self->kbd,
		psy_eventdriverinput_make(message, psy_ui_keyboardevent_encode(ev,
			message == psy_EVENTDRIVER_RELEASE),
			psy_ui_keyboardevent_repeat(ev)));
}

void viewframe_on_language_changed(ViewFrame* self)
{
	psy_ui_Component* page;

	page = psy_ui_component_at(&self->pane, 0);
	if (page) {
		psy_ui_component_set_title(&self->component,
			psy_ui_translate(psy_ui_component_title(page)));
	}
}

/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "mainviews.h"
/* host */
#include "resources/resource.h"
#include "styles.h"
/* ui */
#include <uisplitbar.h>
/* platform */
#include "../../detail/portable.h"

/* ViewFrame */

/* prototypes */
static bool viewframe_on_close(ViewFrame*);
static void viewframe_on_key_down(ViewFrame*, psy_ui_KeyboardEvent*);
static void viewframe_checkplaystartwithrctrl(ViewFrame*,
	psy_ui_KeyboardEvent*);
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
	psy_ui_component_set_vtable(&self->component, &vtable);
}

/* implementation */
void viewframe_init(ViewFrame* self, psy_ui_Component* parent,
	psy_ui_Notebook* dock, Workspace* workspace)
{
	psy_ui_frame_init(&self->component, parent);
	vtable_init(self);
	psy_ui_component_doublebuffer(viewframe_base(self));
	self->dock = dock;
	self->workspace = workspace;
	psy_ui_component_init(&self->pane, &self->component, &self->component);
	psy_ui_component_set_align(&self->pane, psy_ui_ALIGN_CLIENT);
	viewframe_float(self);	
}

ViewFrame* viewframe_alloc(void)
{
	return (ViewFrame*)malloc(sizeof(ViewFrame));
}

ViewFrame* viewframe_allocinit(psy_ui_Component* parent,
	psy_ui_Notebook* dock, Workspace* workspace)
{
	ViewFrame* rv;

	rv = viewframe_alloc();
	if (rv) {
		viewframe_init(rv, parent, dock, workspace);
		psy_ui_component_deallocate_after_destroyed(&rv->component);
	}
	return rv;
}

bool viewframe_on_close(ViewFrame* self)
{
	viewframe_dock(self);
	return TRUE;
}

void viewframe_float(ViewFrame* self)
{	
	psy_ui_Component* page;
	uintptr_t id;

	page = psy_ui_notebook_active_page(self->dock);
	id = psy_ui_notebook_pageindex(self->dock);
	if (page->id != psy_INDEX_INVALID && page->id > 0) {
		id = page->id - 1;
	} else {
		id = 1;
	}
	if (page) {					
		psy_ui_component_set_parent(page, &self->pane);
		psy_ui_component_set_align(page, psy_ui_ALIGN_CLIENT);		
		psy_ui_component_set_title(&self->component, 
			psy_ui_translate(psy_ui_component_title(page)));
		psy_ui_component_show(&self->component);
		psy_ui_notebook_select_by_component_id(self->dock, VIEW_ID_FLOATED);
	}
}

void viewframe_dock(ViewFrame* self)
{
	psy_ui_Component* page;

	page = psy_ui_component_at(&self->pane, 0);
	if (page) {
		psy_ui_component_set_parent(page, &self->dock->component);
		psy_ui_component_set_align(page, psy_ui_ALIGN_CLIENT);		
		psy_ui_notebook_select_by_component_id(self->dock, psy_ui_component_id(page));
	}
}

void viewframe_on_key_down(ViewFrame* self, psy_ui_KeyboardEvent* ev)
{
	/* TODO add immediate mode */	
	viewframe_checkplaystartwithrctrl(self, ev);
	viewframe_delegate_keyboard(self, psy_EVENTDRIVER_PRESS, ev);
}

void viewframe_checkplaystartwithrctrl(ViewFrame* self, psy_ui_KeyboardEvent* ev)
{
	if (keyboardmiscconfig_playstartwithrctrl(
			&self->workspace->config.misc)) {
		if (psy_ui_keyboardevent_keycode(ev) == psy_ui_KEY_CONTROL) {
			/* todo: this win32 detection only */
			int extended = (psy_ui_keyboardevent_keydata(ev) & 0x01000000) != 0;
			if (extended) {
				/* right ctrl */
				psy_audio_player_start_currseqpos(workspace_player(
					self->workspace));
				return;
			}
		} else if (psy_audio_player_playing(&self->workspace->player) &&
			psy_ui_keyboardevent_keycode(ev) == psy_ui_KEY_SPACE) {
			psy_audio_player_stop(&self->workspace->player);
			return;
		}
	}
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
	psy_eventdriver_write(workspace_kbd_driver(self->workspace),
		psy_eventdriverinput_make(message,
			psy_audio_encodeinput(psy_ui_keyboardevent_keycode(ev),
				psy_ui_keyboardevent_shiftkey(ev),
				psy_ui_keyboardevent_ctrlkey(ev),
				psy_ui_keyboardevent_altkey(ev),
				message == psy_EVENTDRIVER_RELEASE),
			psy_ui_keyboardevent_repeat(ev),
			workspace_current_view(self->workspace).id));

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


/* EmptyViewPage */

void emptyviewpage_init(EmptyViewPage* self, psy_ui_Component* parent)
{
	psy_ui_component_init(&self->component, parent, NULL);
	psy_ui_component_set_id(&self->component, VIEW_ID_FLOATED);
	psy_ui_label_init_text(&self->label, &self->component,
		"main.floated");
	psy_ui_component_set_align(psy_ui_label_base(&self->label),
		psy_ui_ALIGN_CENTER);
}

/* MainViews */

/* prototypes */
static void mainviews_onextract(MainViews*, psy_ui_Button* sender);
static void mainviews_extract(MainViews*);
static void mainviews_dock(MainViews*, psy_ui_Component* page);
static void mainviews_on_float(MainViews*, psy_ui_Button* sender);
static void mainviews_float(MainViews*);

/* implementation */
void mainviews_init(MainViews* self, psy_ui_Component* parent, psy_ui_Component* pane, Workspace* workspace)
{	
	psy_ui_component_init(&self->component, parent, NULL);
	self->workspace = workspace;
	mainviewbar_init(&self->mainviewbar, &self->component, pane, workspace);
	psy_signal_connect(&self->mainviewbar.extract.signal_clicked,
		self, mainviews_onextract);
	psy_signal_connect(&self->mainviewbar.view_float.signal_clicked,
		self, mainviews_on_float);
	psy_ui_component_set_align(mainviewbar_base(&self->mainviewbar),
		psy_ui_ALIGN_TOP);
	psy_ui_notebook_init(&self->notebook, &self->component);
	self->notebook.page_not_found_index = VIEW_ID_FLOATED;
	psy_ui_component_set_align(psy_ui_notebook_base(&self->notebook),
		psy_ui_ALIGN_CLIENT);
	emptyviewpage_init(&self->empty_page, 
		psy_ui_notebook_base(&self->notebook));
}

void mainviews_onextract(MainViews* self, psy_ui_Button* sender)
{
	ViewIndex view;
	psy_ui_Component* page;

	view = workspace_current_view(self->workspace);
	page = psy_ui_component_by_id(&self->component, view.id,
		psy_ui_NONE_RECURSIVE);
	if (page) {
		mainviews_dock(self, page);
	} else {
		mainviews_extract(self);
	}
}

void mainviews_extract(MainViews* self)
{
	psy_ui_Component* page;
	uintptr_t id;

	page = psy_ui_notebook_active_page(&self->notebook);
	id = psy_ui_notebook_pageindex(&self->notebook);
	if (page->id != psy_INDEX_INVALID && page->id > 0) {
		id = page->id - 1;
	} else {
		id = 1;
	}
	if (page) {
		psy_ui_Splitter* splitter;

		psy_ui_component_set_parent(page, &self->component);
		psy_ui_component_set_align(page, psy_ui_ALIGN_RIGHT);
		psy_ui_component_set_preferred_size(page, psy_ui_size_make_em(80.0, 0.0));
		splitter = psy_ui_splitter_allocinit(&self->component);
		psy_ui_component_set_align(&splitter->component, psy_ui_ALIGN_RIGHT);
		psy_ui_component_align(&self->component);
		psy_ui_component_invalidate(&self->component);
		psy_ui_notebook_select_by_component_id(&self->notebook,  VIEW_ID_FLOATED);
	}	
}

void mainviews_dock(MainViews* self, psy_ui_Component* page)
{
	psy_ui_Component* splitter;
	uintptr_t index;

	index = psy_ui_component_index(page);
	index++;
	splitter = psy_ui_component_at(&self->component, index);
	if (splitter) {
		psy_ui_component_remove(&self->component, splitter);
	}
	psy_ui_component_set_parent(page, &self->notebook.component);
	psy_ui_component_set_align(page, psy_ui_ALIGN_CLIENT);
	psy_ui_component_align(&self->component);
	psy_ui_component_invalidate(&self->component);
	psy_ui_notebook_select_by_component_id(&self->notebook, page->id);
}

void mainviews_on_float(MainViews* self, psy_ui_Button* sender)
{
	mainviews_float(self);
}

void mainviews_float(MainViews* self)
{
	ViewFrame* frame;

	frame = viewframe_allocinit(self->workspace->main, 
		&self->notebook, self->workspace);
}

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


/* MainViews */

/* prototypes */
static void mainviews_init_view_status_bars(MainViews*);
static void mainviews_on_extract(MainViews*, psy_ui_Button* sender);
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
		self, mainviews_on_extract);
	psy_signal_connect(&self->mainviewbar.view_float.signal_clicked,
		self, mainviews_on_float);
	psy_ui_component_set_align(mainviewbar_base(&self->mainviewbar),
		psy_ui_ALIGN_TOP);
	mainviews_init_view_status_bars(self);
	psy_ui_notebook_init(&self->notebook, &self->component);
	self->notebook.page_not_found_index = VIEW_ID_FLOATED;
	psy_ui_component_set_align(psy_ui_notebook_base(&self->notebook),
		psy_ui_ALIGN_CLIENT);
	emptyviewpage_init(&self->empty_page, 
		psy_ui_notebook_base(&self->notebook));	
}

void mainviews_init_view_status_bars(MainViews* self)
{
	psy_ui_notebook_init(&self->viewstatusbars, &self->component);	
	psy_ui_component_set_align(psy_ui_notebook_base(&self->viewstatusbars),
		psy_ui_ALIGN_BOTTOM);
	psy_ui_component_set_style_type(&self->viewstatusbars.component, 
		STYLE_STATUSBAR);
	psy_ui_component_set_default_align(
		psy_ui_notebook_base(&self->viewstatusbars),
		psy_ui_ALIGN_LEFT, psy_ui_defaults_hmargin(psy_ui_defaults()));	
}


void mainviews_on_extract(MainViews* self, psy_ui_Button* sender)
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
	psy_ui_Component* page;

	page = psy_ui_notebook_active_page(&self->notebook);
	if (page) {
		viewframe_allocinit(self->workspace->main, page,
			NULL, NULL,
			workspace_kbd_driver(self->workspace));
	}
}

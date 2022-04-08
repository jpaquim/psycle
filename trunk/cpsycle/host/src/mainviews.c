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


static void mainviews_onextract(MainViews*, psy_ui_Button* sender);
static void mainviews_extract(MainViews*);
static void mainviews_dock(MainViews*, psy_ui_Component* page);

/* implementation */
void mainviews_init(MainViews* self, psy_ui_Component* parent, psy_ui_Component* pane, Workspace* workspace)
{	
	psy_ui_component_init(&self->component, parent, NULL);
	self->workspace = workspace;
	mainviewbar_init(&self->mainviewbar, &self->component, pane, workspace);
	psy_signal_connect(&self->mainviewbar.extract.signal_clicked,
		self, mainviews_onextract);
	psy_ui_component_set_align(mainviewbar_base(&self->mainviewbar),
		psy_ui_ALIGN_TOP);
	psy_ui_notebook_init(&self->notebook, &self->component);
	psy_ui_component_set_align(psy_ui_notebook_base(&self->notebook),
		psy_ui_ALIGN_CLIENT);	
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

		psy_ui_component_setparent(page, &self->component);
		psy_ui_component_set_align(page, psy_ui_ALIGN_RIGHT);
		psy_ui_component_set_preferred_size(page, psy_ui_size_make_em(80.0, 0.0));
		splitter = psy_ui_splitter_allocinit(&self->component);
		psy_ui_component_set_align(&splitter->component, psy_ui_ALIGN_RIGHT);
		psy_ui_component_align(&self->component);
		psy_ui_component_invalidate(&self->component);
		psy_ui_notebook_select_by_component_id(&self->notebook, id);
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
	psy_ui_component_setparent(page, &self->notebook.component);
	psy_ui_component_set_align(page, psy_ui_ALIGN_CLIENT);
	psy_ui_component_align(&self->component);
	psy_ui_component_invalidate(&self->component);
	psy_ui_notebook_select_by_component_id(&self->notebook, page->id);
}

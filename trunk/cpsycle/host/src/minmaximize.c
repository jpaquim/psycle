/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "minmaximize.h"
/* ui */
#include <uiapp.h>
#include <uicomponent.h>


/* implementation */
void minmaximize_init(MinMaximize* self, psy_ui_Component* view)
{
	assert(self);
	assert(view);
	
	self->minmaximize = NULL;
	self->view = view;
}

void minmaximize_dispose(MinMaximize* self)
{
	psy_list_free(self->minmaximize);
	self->minmaximize = NULL;
}

void minmaximize_add(MinMaximize* self, psy_ui_Component* component)
{
	psy_list_append(&self->minmaximize, component);	
}

void minmaximize_toggle(MinMaximize* self)
{
	if (self->minmaximize) {
		psy_List* p;
		bool show;

		show = TRUE;
		for (p = self->minmaximize; p != NULL; p = p->next) {
			psy_ui_Component* component;

			component = (psy_ui_Component*)p->entry;
			if (p == self->minmaximize) {
				show = !psy_ui_component_visible(component);
			}
			if (show) {
				psy_ui_component_show(component);
			} else {
				psy_ui_component_hide(component);
			}
		}
		psy_ui_component_align(self->view);
		psy_ui_component_invalidate(self->view);
	}
}

/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "viewhistory.h"
/* std */
#include <stdlib.h>

void viewhistory_init(ViewHistory* self)
{
	assert(self);

	self->container = NULL;
	self->prevented = FALSE;
	self->currnavigation = NULL;
}

void viewhistory_dispose(ViewHistory* self)
{
	assert(self);

	psy_list_deallocate(&self->container, (psy_fp_disposefunc)NULL);
	self->prevented = FALSE;
}

void viewhistory_clear(ViewHistory* self)
{
	assert(self);

	psy_list_deallocate(&self->container, (psy_fp_disposefunc)NULL);
	self->prevented = FALSE;
}

void viewhistory_add(ViewHistory* self, ViewIndex view)
{
	assert(self);

	if (!self->prevented) {
		ViewIndex* entry;

		entry = (ViewIndex*)malloc(sizeof(ViewIndex));
		if (entry) {
			*entry = view;			
			self->currnavigation = psy_list_append(&self->container, entry);
		}
	}
}

void viewhistory_add_seq_pos(ViewHistory* self, uintptr_t seqpos)
{
	ViewIndex view;

	view = viewhistory_current(self);
	view.seqpos = seqpos;
	viewhistory_add(self, view);	
}

ViewIndex viewhistory_current(const ViewHistory* self)
{
	ViewIndex rv;

	assert(self);

	if (viewhistory_has_curr_view(self)) {
		const ViewIndex* entry;

		entry = (ViewIndex*)psy_list_entry_const(self->currnavigation);
		assert(entry);
		rv = *entry;
	} else {
		rv.seqpos = psy_INDEX_INVALID;
		rv.id = 0;		
		rv.section = 0;
		rv.option = psy_INDEX_INVALID;
	}
	return rv;
}

bool viewhistory_back(ViewHistory* self)
{
	assert(self);

	if (self->currnavigation && self->currnavigation->prev) {
			self->currnavigation = self->currnavigation->prev;
			return TRUE;		
	}
	return FALSE;
}

bool viewhistory_forward(ViewHistory* self)
{
	assert(self);

	if (self->currnavigation && self->currnavigation->next) {
		self->currnavigation = self->currnavigation->next;
		return TRUE;
	}
	return FALSE;
}

bool viewhistory_equal(const ViewHistory* self, ViewIndex view)
{
	ViewIndex curr;

	assert(self);

	curr = viewhistory_current(self);
	return (view.seqpos == curr.seqpos) && (view.id == curr.id);
}

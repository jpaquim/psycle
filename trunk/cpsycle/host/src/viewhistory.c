// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "viewhistory.h"
// std
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

void viewhistory_add(ViewHistory* self, ViewHistoryEntry view)
{
	assert(self);

	if (!self->prevented) {
		ViewHistoryEntry* entry;

		entry = (ViewHistoryEntry*)malloc(sizeof(ViewHistoryEntry));
		if (entry) {
			*entry = view;			
			psy_list_append(&self->container, entry);
			self->currnavigation = self->container->tail;
		}
	}
}

void viewhistory_addseqpos(ViewHistory* self, uintptr_t seqpos)
{
	ViewHistoryEntry view;

	view = viewhistory_currview(self);
	view.seqpos = seqpos;
	viewhistory_add(self, view);	
}

ViewHistoryEntry viewhistory_currview(const ViewHistory* self)
{
	assert(self);

	ViewHistoryEntry rv;

	if (viewhistory_hascurrview(self)) {
		ViewHistoryEntry* entry;

		entry = (ViewHistoryEntry*)(self->currnavigation->entry);
		assert(entry);
		rv = *entry;
	} else {
		rv.seqpos = UINTPTR_MAX;
		rv.id = 0;
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

bool viewhistory_equal(const ViewHistory* self, ViewHistoryEntry view)
{
	ViewHistoryEntry curr;

	assert(self);

	curr = viewhistory_currview(self);
	return (view.seqpos == curr.seqpos) && (view.id == curr.id);
}
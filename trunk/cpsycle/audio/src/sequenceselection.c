/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "sequenceselection.h"
/* std */
#include <stdlib.h>

psy_audio_OrderIndex* psy_audio_orderindex_alloc(void)
{
	return (psy_audio_OrderIndex*)malloc(sizeof(psy_audio_OrderIndex));
}

/* SequenceSelection */
/* implementation */
void psy_audio_sequenceselection_init(psy_audio_SequenceSelection* self)
{			
	psy_signal_init(&self->signal_clear);
	psy_signal_init(&self->signal_select);
	psy_signal_init(&self->signal_deselect);
	self->entries = NULL;
}

void psy_audio_sequenceselection_dispose(psy_audio_SequenceSelection* self)
{
	psy_list_deallocate(&self->entries, NULL);	
	psy_signal_dispose(&self->signal_clear);
	psy_signal_dispose(&self->signal_select);
	psy_signal_dispose(&self->signal_deselect);	
}

void psy_audio_sequenceselection_clear(psy_audio_SequenceSelection* self)
{
	psy_list_deallocate(&self->entries, NULL);
	psy_signal_emit(&self->signal_clear, self, 0);
}

void psy_audio_sequenceselection_deselect_all(psy_audio_SequenceSelection* self)
{
	psy_List* p;
	psy_List* q;
	
	for (p = q = self->entries; p != NULL; p = q) {
		psy_audio_OrderIndex* currindex;
		psy_audio_OrderIndex index;

		q = p->next;
		currindex = (psy_audio_OrderIndex*)psy_list_entry(p);
		index = *currindex;
		psy_list_remove(&self->entries, p);
		psy_signal_emit(&self->signal_deselect, self, 1, &index);
	}
}

void psy_audio_sequenceselection_copy(psy_audio_SequenceSelection* self,
	psy_audio_SequenceSelection* other)
{
	psy_List* p;

	psy_list_deallocate(&self->entries, NULL);
	for (p = other->entries; p != NULL; p = p->next) {
		psy_audio_OrderIndex* currindex;
		psy_audio_OrderIndex* newindex;

		currindex = (psy_audio_OrderIndex*)psy_list_entry(p);
		newindex = (psy_audio_OrderIndex*)
			malloc(sizeof(psy_audio_OrderIndex));
		if (newindex) {
			*newindex = *currindex;
			psy_list_append(&self->entries, newindex);
		}		
	}
}

psy_audio_OrderIndex psy_audio_sequenceselection_first(
	const psy_audio_SequenceSelection* self)
{
	if (self->entries) {
		psy_audio_OrderIndex* first;

		first = (psy_audio_OrderIndex*)(self->entries->entry);
		return *first;

	}
	return psy_audio_orderindex_make_invalid();
}

void psy_audio_sequenceselection_select(psy_audio_SequenceSelection* self,
	psy_audio_OrderIndex index)
{
	if (!psy_audio_sequenceselection_is_selected(self, index)) {
		psy_audio_OrderIndex* newindex;

		newindex = psy_audio_orderindex_alloc();
		if (newindex) {
			*newindex = index;
			psy_list_append(&self->entries, newindex);
			psy_signal_emit(&self->signal_select, self, 1, &index);			
		}
	}	
}

void psy_audio_sequenceselection_select_first(psy_audio_SequenceSelection* self,
	psy_audio_OrderIndex index)
{	
	if (!psy_audio_sequenceselection_is_selected(self, index)) {
		psy_audio_OrderIndex* newindex;

		newindex = psy_audio_orderindex_alloc();
		if (newindex) {
			*newindex = index;
			psy_list_insert(&self->entries, NULL, newindex);
			psy_signal_emit(&self->signal_select, self, 1, &index);			
		}
	}
}

void psy_audio_sequenceselection_append(psy_audio_SequenceSelection* self,
	psy_audio_OrderIndex index)
{
	psy_audio_OrderIndex* newindex;

	newindex = (psy_audio_OrderIndex*)
		malloc(sizeof(psy_audio_OrderIndex));
	*newindex = index;
	psy_list_append(&self->entries, newindex);
}

void psy_audio_sequenceselection_deselect(psy_audio_SequenceSelection* self,
	psy_audio_OrderIndex index)
{
	if (psy_audio_sequenceselection_is_selected(self, index)) {
		psy_List* p;

		p = self->entries;
		while (p) {
			psy_audio_OrderIndex* currindex;

			currindex = (psy_audio_OrderIndex*)psy_list_entry(p);
			if (psy_audio_orderindex_equal(currindex, index)) {
				free(p->entry);
				psy_list_remove(&self->entries, p);
				psy_signal_emit(&self->signal_deselect, self, 1, &index);				
				break;
			}
			p = p->next;
		}
	}
}

bool psy_audio_sequenceselection_is_selected(const psy_audio_SequenceSelection* self,
	psy_audio_OrderIndex index)
{
	psy_List* p;

	p = self->entries;
	while (p) {
		psy_audio_OrderIndex* currindex;

		currindex = (psy_audio_OrderIndex*)psy_list_entry(p);
		if (psy_audio_orderindex_equal(currindex, index)) {
			break;
		}
		p = p->next;
	}
	return p != NULL;
}

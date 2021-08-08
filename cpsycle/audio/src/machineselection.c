/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "machineselection.h"
/* std */
#include <stdlib.h>

psy_audio_MachineIndex* psy_audio_machineindex_alloc(void)
{
	return (psy_audio_MachineIndex*)malloc(sizeof(psy_audio_MachineIndex));
}

/* MachineSelection */
/* implementation */
void psy_audio_machineselection_init(psy_audio_MachineSelection* self)
{			
	psy_signal_init(&self->signal_clear);
	psy_signal_init(&self->signal_select);
	psy_signal_init(&self->signal_deselect);
	self->entries = NULL;
}

void psy_audio_machineselection_dispose(psy_audio_MachineSelection* self)
{
	psy_list_deallocate(&self->entries, NULL);	
	psy_signal_dispose(&self->signal_clear);
	psy_signal_dispose(&self->signal_select);
	psy_signal_dispose(&self->signal_deselect);	
}

void psy_audio_machineselection_clear(psy_audio_MachineSelection* self)
{
	psy_list_deallocate(&self->entries, NULL);
	psy_signal_emit(&self->signal_clear, self, 0);
}

void psy_audio_machineselection_deselectall(psy_audio_MachineSelection* self)
{
	psy_List* p;
	psy_List* q;
	
	for (p = q = self->entries; p != NULL; p = q) {
		psy_audio_MachineIndex* currindex;
		psy_audio_MachineIndex index;

		q = p->next;
		currindex = (psy_audio_MachineIndex*)psy_list_entry(p);
		index = *currindex;
		psy_list_remove(&self->entries, p);
		psy_signal_emit(&self->signal_deselect, self, 1, &index);
	}
}

void psy_audio_machineselection_copy(psy_audio_MachineSelection* self,
	psy_audio_MachineSelection* other)
{
	psy_List* p;

	psy_list_deallocate(&self->entries, NULL);
	for (p = other->entries; p != NULL; p = p->next) {
		psy_audio_MachineIndex* currindex;
		psy_audio_MachineIndex* newindex;

		currindex = (psy_audio_MachineIndex*)psy_list_entry(p);
		newindex = (psy_audio_MachineIndex*)
			malloc(sizeof(psy_audio_MachineIndex));
		if (newindex) {
			*newindex = *currindex;
			psy_list_append(&self->entries, newindex);
		}		
	}
}

psy_audio_MachineIndex psy_audio_machineselection_first(
	const psy_audio_MachineSelection* self)
{
	if (self->entries) {
		psy_audio_MachineIndex* first;

		first = (psy_audio_MachineIndex*)(self->entries->entry);
		return *first;

	}
	return psy_audio_machineindex_zero();
}

void psy_audio_machineselection_select(psy_audio_MachineSelection* self,
	psy_audio_MachineIndex index)
{
	if (!psy_audio_machineselection_isselected(self, index)) {
		psy_audio_MachineIndex* newindex;

		newindex = psy_audio_machineindex_alloc();
		if (newindex) {
			*newindex = index;
			psy_list_append(&self->entries, newindex);
			psy_signal_emit(&self->signal_select, self, 1, &index);			
		}
	}	
}

void psy_audio_machineselection_select_first(psy_audio_MachineSelection* self,
	psy_audio_MachineIndex index)
{	
	if (!psy_audio_machineselection_isselected(self, index)) {
		psy_audio_MachineIndex* newindex;

		newindex = psy_audio_machineindex_alloc();
		if (newindex) {
			*newindex = index;
			psy_list_insert(&self->entries, NULL, newindex);
			psy_signal_emit(&self->signal_select, self, 1, &index);			
		}
	}
}

void psy_audio_machineselection_append(psy_audio_MachineSelection* self,
	psy_audio_MachineIndex index)
{
	psy_audio_MachineIndex* newindex;

	newindex = (psy_audio_MachineIndex*)
		malloc(sizeof(psy_audio_MachineIndex));
	if (newindex) {
		*newindex = index;
		psy_list_append(&self->entries, newindex);
	}
}

void psy_audio_machineselection_deselect(psy_audio_MachineSelection* self,
	psy_audio_MachineIndex index)
{
	if (psy_audio_machineselection_isselected(self, index)) {
		psy_List* p;

		p = self->entries;
		while (p) {
			psy_audio_MachineIndex* currindex;

			currindex = (psy_audio_MachineIndex*)psy_list_entry(p);
			if (psy_audio_machineindex_equal(currindex, index)) {
				free(p->entry);
				psy_list_remove(&self->entries, p);
				psy_signal_emit(&self->signal_deselect, self, 1, &index);				
				break;
			}
			p = p->next;
		}
	}
}

bool psy_audio_machineselection_isselected(const psy_audio_MachineSelection* self,
	psy_audio_MachineIndex index)
{
	psy_List* p;

	p = self->entries;
	while (p) {
		psy_audio_MachineIndex* currindex;

		currindex = (psy_audio_MachineIndex*)psy_list_entry(p);
		if (psy_audio_machineindex_equal(currindex, index)) {
			break;
		}
		p = p->next;
	}
	return p != NULL;
}

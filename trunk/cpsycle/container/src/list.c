// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "list.h"
#include <stdlib.h>
#include <assert.h>

psy_List* psy_list_create(void* entry)
{
	psy_List* list;

	list = (psy_List*) malloc(sizeof(psy_List));
	list->prev = 0;
	list->next = 0;
	list->tail = list;
	list->size = 1;
	list->entry = entry;
	return list;
}

void psy_list_free(psy_List* list)
{
	if (list) {
		psy_List* ptr;
		psy_List* next;

		ptr = list;
		while (ptr != NULL) {
			next = ptr->next;
			free(ptr);
			ptr = next;
		}
	}
}

psy_List* psy_list_append(psy_List** self, void* entry)
{		
	if (*self == 0) {
		*self = psy_list_create(entry);
	} else {
		(*self)->tail->next = psy_list_create(entry);
		(*self)->tail->next->prev = (*self)->tail;
		(*self)->tail = (*self)->tail->next;
		++((*self)->size);
	}	
	return (*self)->tail;
}

psy_List* psy_list_cat(psy_List** self, psy_List* list)
{
	if (!*self) {
		*self = list;
	} else 
	if (list) {		
		(*self)->tail->next = list;
		(*self)->tail = list->tail;
		(*self)->size += list->size;
	}
	return *self ? (*self)->tail : 0;
}

psy_List* psy_list_insert(psy_List** self, psy_List* ptr, void* entry)
{
	psy_List* next;
	if (ptr == NULL) {
		ptr = psy_list_create(entry);
		(*self)->prev = ptr;
		ptr->next = *self;
		ptr->tail = (*self)->tail;
		(*self)->tail = 0;
		*self = ptr;		
		return ptr;
	}	
	next = ptr->next;	
	if (next == NULL) {
		return psy_list_append(self, entry);
	}
	ptr->next = psy_list_create(entry);
	ptr->next->prev = ptr;	
	next->prev = ptr->next;
	ptr->next->next = next;
	++(*self)->size;
	return ptr->next;
}

psy_List* psy_list_remove(psy_List** self, psy_List* ptr)
{	
	psy_List* rv;

	if (self && *self) {
		if (ptr->prev == NULL && ptr->next == NULL) {
			*self = NULL;
			free(ptr);
			return 0;
		}
		if (ptr->prev != NULL) {
			ptr->prev->next = ptr->next;
		} else {
			*self = ptr->next;		
			(*self)->tail = ptr->tail;
		}
		if (ptr->next != NULL) {
			ptr->next->prev = ptr->prev;
		} else {
			(*self)->tail = ptr->prev;		
		}
		rv = ptr->next;
		free(ptr);
		--(*self)->size;
	} else {
		rv = 0;
	}
	return rv;
}

uintptr_t psy_list_size(const psy_List* self)
{	
	return self? self->size : 0;
	/*
	uintptr_t rv = 0;
	const psy_List* p;

	for (p = self; p != 0; p = p->next, ++rv);
	if (self) {
		assert(self->size == rv);
	}
	return rv;
	*/
}

psy_List* list_last(psy_List* self)
{
	return self ? self->tail : 0;
}

int psy_list_check(psy_List* self, psy_List* node)
{
	psy_List* p = self;
			
	while (p != 0) {
		if (p == node) {
			break;
		}
		p = p->next;		
	}
	return p != 0;
}

psy_List* psy_list_findentry(psy_List* self, void* entry)
{
	psy_List* p = self;
			
	while (p != 0) {
		if (p->entry == entry) {
			break;
		}
		p = p->next;		
	}
	return p;
}

psy_List* psy_list_at(psy_List* self, uintptr_t numentry)
{
	psy_List* p = self;
	uintptr_t c = 0;
			
	while (p != 0) {
		if (c == numentry) {
			break;
		}
		p = p->next;
		++c;
	}
	return p;
}

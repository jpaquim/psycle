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
	list->prev = NULL;
	list->next = NULL;
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
	if (*self == NULL) {
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
		list->prev = (*self)->tail;
		(*self)->tail = list->tail;
		(*self)->size += list->size;
	}
	return *self ? (*self)->tail : NULL;
}

psy_List* psy_list_insert(psy_List** self, psy_List* ptr, void* entry)
{
	psy_List* next;
	if (ptr == NULL) {
		ptr = psy_list_create(entry);
		(*self)->prev = ptr;
		ptr->next = *self;
		ptr->tail = (*self)->tail;
		(*self)->tail = NULL;
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
	psy_List* tail;	

	if (self && *self) {
		tail = (*self)->tail;
		if (ptr->prev == NULL && ptr->next == NULL) {
			*self = NULL;
			free(ptr);
			return 0;
		}
		if (ptr->prev != NULL) {
			ptr->prev->next = ptr->next;
		} else {
			*self = ptr->next;
			if (tail != ptr) {
				(*self)->tail = tail;
			} else {
				(*self)->tail = tail->prev;
			}
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
		rv = NULL;
	}
	return rv;
}

bool psy_list_check(psy_List* self, psy_List* node)
{
	psy_List* p = self;
			
	while (p != NULL) {
		if (p == node) {
			break;
		}
		p = p->next;		
	}
	return p != NULL;
}

psy_List* psy_list_findentry(psy_List* self, void* entry)
{
	psy_List* p = self;
			
	while (p != NULL) {
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
			
	while (p != NULL) {
		if (c == numentry) {
			break;
		}
		p = p->next;
		++c;
	}
	return p;
}

void psy_list_deallocate(psy_List** self, psy_fp_disposefunc disposefunc)
{
	if (*self) {
		psy_List* p = *self;		

		if (disposefunc) {
			for (p = *self; p != NULL; psy_list_next(&p)) {
				disposefunc(psy_list_entry(p));
				free(psy_list_entry(p));
			}
		} else {
			for (p = *self; p != NULL; psy_list_next(&p)) {				
				free(psy_list_entry(p));
			}
		}
		psy_list_free(*self);
		*self = NULL;
	}
}
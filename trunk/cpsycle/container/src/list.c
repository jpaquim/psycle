// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net
#include "list.h"
#include <stdlib.h>

List* list_create(void* node)
{
	List* list;

	list = (List*) malloc(sizeof(List));
	list->prev = 0;
	list->next = 0;
	list->tail = list;
	list->node = node;
	return list;
}

List* list_append(List* self, void* node)
{		
	self->tail->next = list_create(node);
	self->tail->next->prev = self->tail;
	self->tail = self->tail->next;	
	return self->tail;
}

List* list_insert(List* self, List* ptr, void* node)
{
	List* next = ptr->next;	
	if (next == NULL) {
		return list_append(self, node);
	}
	ptr->next = list_create(node);	
	ptr->next->prev = ptr;	
	next->prev = ptr->next;
	ptr->next->next = next;
	return ptr->next;
}

List* list_remove(List** self, List* ptr)
{	
	List* rv;
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
	return rv;
}

void list_free(List* list)
{
	if (list) {
		List* ptr;
		List* next;

		ptr = list;
		while (ptr != NULL) {
			next = ptr->next;
			free(ptr);
			ptr = next;
		}
	}
}


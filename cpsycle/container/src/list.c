// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net
#include "list.h"
#include <stdlib.h>

List* list_create(void)
{
	List* list;

	list = (List*) malloc(sizeof(List));
	list->next = 0;
	list->tail = list;
	list->node = 0;
	return list;
}

List* list_append(List* self, void* node)
{
	self->tail->next = list_create();	
	self->tail = self->tail->next;	
	self->tail->node = node;
	return self->tail;
}


List* list_front(List* self, void* node)
{
	List* first;

	first = list_create();
	first->node = node;
	first->tail = self->tail;
	first->next = self;
	return first;
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


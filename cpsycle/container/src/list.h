// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net
#if !defined(LIST)
#define LIST

struct ListStruct {
	void* entry;	
	struct ListStruct* next;
	struct ListStruct* prev;
	struct ListStruct* tail;
};

typedef struct ListStruct List;

List* list_create(void* node);
void list_free(List* list);
List* list_append(List*, void* node);
List* list_insert(List**, List* ptr, void* node);
List* list_remove(List**, List* ptr);

#endif
// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#if !defined(LIST_H)
#define LIST_H

#include "../../detail/stdint.h"

#ifdef __cplusplus
extern "C" {
#endif

/// a double linked list
typedef struct List {
	void* entry;	
	struct List* next; /// anchor to next node or null
	struct List* prev; /// anchor to previous node or null
	struct List* tail; /// first node only, anchor to the tail node
} List;

/// constructs a list
List* list_create(void* entry);
/// frees the list structure, the entry pointer is not freed
void list_free(List* list);
/// constructs a node or new list and appends it to the tail of the list
///\return constructed node appended at the tail of the list
List* list_append(List**, void* entry);
/// appends a list
///\return tail of concatenated list or null
List* list_cat(List**, List*);
/// constructs a node and inserts it after the given node
/// or if node is null at the beginning
///\return tail of concatenated list or null
List* list_insert(List**, List* node, void* entry);
/// removes the node
///\return next node of the removed node or last node, if next is null
List* list_remove(List**, List* node);
/// returns the tail node or null
List* list_last(List*);
/// counts the nodes of the list
uintptr_t list_size(const List*);
/// checks if the list has the node
int list_check(List*, List* node);
/// returns the node for the entry
List* list_findentry(List*, void* entry);
/// returns the node at numentry position
List* list_at(List*, uintptr_t numentry);

#ifdef __cplusplus
}
#endif

#endif

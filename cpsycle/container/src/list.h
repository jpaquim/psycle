// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#ifndef psy_LIST_H
#define psy_LIST_H

#include "../../detail/psydef.h"

#include <assert.h>

#ifdef __cplusplus
extern "C" {
#endif

/// a double linked list
typedef struct psy_List {
	void* entry;	
	struct psy_List* next; /// anchor to next node or null
	struct psy_List* prev; /// anchor to previous node or null
	struct psy_List* tail; /// first node only, anchor to the tail node
	uintptr_t size;		   /// first node only, keeps track of the size
} psy_List;

/// constructs a list with one node
psy_List* psy_list_create(void* entry);
/// frees the list structure, the entry pointer is not freed
void psy_list_free(psy_List* list);
/// deallocates the list: frees the list structure, calls
/// the dispose func, if not NULL, and frees all entries
void psy_list_deallocate(psy_List**, psy_fp_disposefunc disposefunc);
/// returns the entry of a node (!= NULL)
INLINE void* psy_list_entry(psy_List* self)
{
	assert(self);
	return self->entry;
}
/// returns the next node or NULL
INLINE void psy_list_next(psy_List** self)
{
	assert(self != NULL && (*self) != NULL);
	*self = (*self)->next;
}
/// returns the previous node or NULL
INLINE void psy_list_prev(psy_List** self)
{
	assert(self != NULL && (*self) != NULL);
	*self = (*self)->prev;
}
/// constructs a node or new list and appends it to the tail of the list
///\return constructed node appended at the tail of the list
psy_List* psy_list_append(psy_List**, void* entry);
/// appends a list
///\return tail of concatenated list or null
psy_List* psy_list_cat(psy_List**, psy_List*);
/// constructs a node and inserts it after the given node
/// or if node is null at the beginning
///\return tail of concatenated list or null
psy_List* psy_list_insert(psy_List**, psy_List* node, void* entry);
/// removes the node
///\return next node of the removed node or last node, if next is null
psy_List* psy_list_remove(psy_List**, psy_List* node);
/// returns the tail node or null
INLINE psy_List* psy_list_last(psy_List* self)
{
	return (self)
		? self->tail
		: NULL;
}
/// returns the node number of the list
INLINE uintptr_t psy_list_size(const psy_List* self)
{
	return (self)
		? self->size
		: 0;
}
/// checks if the list has the node
bool psy_list_exists(psy_List*, psy_List* node);
/// returns the node for the entry
psy_List* psy_list_findentry(psy_List*, void* entry);
/// returns the index of the entry or UINTPTR_MAX
uintptr_t psy_list_entry_index(psy_List*, void* entry);
/// returns the node at numentry position
psy_List* psy_list_at(psy_List*, uintptr_t numentry);

#ifdef __cplusplus
}
#endif

#endif /* psy_LIST_H */

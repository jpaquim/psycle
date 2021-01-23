// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "containerconvert.h"
// std
#include <assert.h>
#include <stdlib.h>
#include <string.h>

void** psy_array_alloc(uintptr_t size)
{
	return malloc(sizeof(uintptr_t) * size);
}

void psy_array_clear(void** self, uintptr_t size)
{
	uintptr_t i;
	
	for (i = 0; i < size; ++i) {
		self[i] = NULL;
	}
}

void psy_list_to_array(void** dest, uintptr_t maxsize, psy_List* source)
{		
	psy_List* p;
	uintptr_t i;

	for (i = 0, p = source; p != NULL && i < maxsize; psy_list_next(&p), ++i) {
		dest[i] = p->entry;
	}	
}

psy_List* psy_array_to_list(void** source, uintptr_t size)
{
	psy_List* rv;
	uintptr_t i;

	rv = 0;
	for (rv = NULL, i = 0; i < size; ++i) {
		psy_list_append(&rv, source[i]);
	}
	return rv;
}

void psy_table_to_array(void** dest, uintptr_t maxsize, psy_Table* source)
{	
	psy_TableIterator it;

	for (it = psy_table_begin(source);
			!psy_tableiterator_equal(&it, psy_table_end());
			psy_tableiterator_inc(&it)) {
		uintptr_t i;

		i = psy_tableiterator_key(&it);
		if (i < maxsize) {
			dest[i] = psy_tableiterator_value(&it);
		}		
	}
}

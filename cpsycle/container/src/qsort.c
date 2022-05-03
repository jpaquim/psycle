// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net
// based on qsort Kerninghan & Ritchie The C Programming Language

#include "qsort.h"

static void psy_qsort_swap(void *container,
	psy_fp_set_index_double set,
	psy_fp_index_double get,
	intptr_t i, intptr_t j);

void psy_qsort(void* container,
	psy_fp_set_index_double set,
	psy_fp_index_double get,
	intptr_t left, intptr_t right, psy_fp_comp comp)
{
	intptr_t i, last;
	
	if (left >= right) {
		return;
	}
	psy_qsort_swap(container, set, get, left, (left + right) / 2);
	last = left;
	for (i = left + 1; i <= right; ++i) {
		if ((*comp)(get(container, i), get(container, left)) < 0) {
			psy_qsort_swap(container, set, get, ++last, i);
		}
	}	
	psy_qsort_swap(container, set, get, left, last);	
	psy_qsort(container, set, get, left, last - 1, comp);
	psy_qsort(container, set, get, last+1, right, comp);
}

void psy_qsort_swap(void *container,
	psy_fp_set_index_double set,
	psy_fp_index_double get,
	intptr_t i, intptr_t j)
{
	void* temp;

	temp = get(container, i);
	set(container, i, get(container, j));	
	set(container, j, temp);	
}

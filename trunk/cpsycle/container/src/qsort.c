// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net
// based on qsort Kerninghan & Ritchie The C Programming Language

#include "qsort.h"

static void psy_qsort_swap(void *v[], intptr_t i, intptr_t j);

void psy_qsort(void *v[], intptr_t left, intptr_t right, psy_fp_comp comp)
{
	intptr_t i, last;
	
	if (left >= right) {
		return;
	}
	psy_qsort_swap(v, left, (left + right) / 2);
	last = left;
	for (i = left + 1; i <= right; ++i) {
		if ((*comp)(v[i], v[left]) < 0) {
			psy_qsort_swap(v, ++last, i);
		}
	}	
	psy_qsort_swap(v, left, last);	
	psy_qsort(v, left, last - 1, comp);
	psy_qsort(v, last+1, right, comp);
}

void psy_qsort_swap(void *v[], intptr_t i, intptr_t j)
{
	void *temp;

	temp = v[i];
	v[i] = v[j];
	v[j] = temp;
}

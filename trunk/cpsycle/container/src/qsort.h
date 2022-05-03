// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#ifndef psy_QSORT_H
#define psy_QSORT_H

#include "../../detail/psydef.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*psy_fp_set_index_double)(void*, uintptr_t, void*);
typedef void* (*psy_fp_index_double)(void*, uintptr_t);
typedef int (*psy_fp_comp)(void *, void *);

void psy_qsort(void * container, 
	psy_fp_set_index_double,
	psy_fp_index_double,
	intptr_t left, intptr_t right, psy_fp_comp);

#ifdef __cplusplus
}
#endif

#endif /* psy_QSORT_H */

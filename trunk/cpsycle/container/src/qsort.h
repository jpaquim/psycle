// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#ifndef psy_QSORT_H
#define psy_QSORT_H

#include "../../detail/stdint.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef int (*psy_fp_comp)(void *, void *);

void psy_qsort(void *v[], int left, int right, psy_fp_comp);

#ifdef __cplusplus
}
#endif

#endif /* psy_QSORT_H */

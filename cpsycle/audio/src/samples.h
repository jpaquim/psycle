// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#if !defined(SAMPLES_H)
#define SAMPLES_H

#include "sample.h"
#include <hashtbl.h>

#define NOSAMPLES_INDEX UINTPTR_MAX

typedef struct Samples {
	Table container;
} Samples;

void samples_init(Samples*);
void samples_dispose(Samples*);
void samples_insert(Samples*, Sample* sample, uintptr_t slot);
Sample* samples_at(Samples*, uintptr_t slot);

#endif
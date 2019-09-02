// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#if !defined(SAMPLES_H)
#define SAMPLES_H

#include "sample.h"
#include <hashtbl.h>

typedef struct {
	IntHashTable container;
} Samples;

void samples_init(Samples*);
void samples_dispose(Samples*);
void samples_insert(Samples* samples, Sample* sample, int slot);


#endif
// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#include "samples.h"

void samples_init(Samples* self)
{
	InitIntHashTable(&self->container, 256);
}

void samples_dispose(Samples* self)
{
	DisposeIntHashTable(&self->container);
}

void samples_insert(Samples* self, Sample* sample, int slot)
{
	InsertIntHashTable(&self->container, slot, sample);
}


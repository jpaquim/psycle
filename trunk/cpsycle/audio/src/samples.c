// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "samples.h"
#include <stdlib.h> 

void samples_init(Samples* self)
{
	table_init(&self->container);
}

void samples_dispose(Samples* self)
{	
	TableIterator it;

	for (it = table_begin(&self->container);
			!tableiterator_equal(&it, table_end()); tableiterator_inc(&it)) {
		Sample* sample;
		
		sample = (Sample*)tableiterator_value(&it);
		sample_dispose(sample);
		free(sample);
	}
	table_dispose(&self->container);
}

void samples_insert(Samples* self, Sample* sample, uintptr_t slot)
{
	table_insert(&self->container, slot, sample);
}

Sample* samples_at(Samples* self, uintptr_t slot)
{
	return table_at(&self->container, slot);
}

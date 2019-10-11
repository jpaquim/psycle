// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#include "xminstruments.h"
#include <stdlib.h>

void xminstruments_init(XMInstruments* self)
{
	table_init(&self->container);
}

void xminstruments_dispose(XMInstruments* self)
{
	TableIterator it;

	for (it = table_begin(&self->container);
			!tableiterator_equal(&it, table_end()); tableiterator_inc(&it)) {
		XMInstrument* xminstrument;
		
		xminstrument = (XMInstrument*)tableiterator_value(&it);
		xminstrument_dispose(xminstrument);
		free(xminstrument);
	}
	table_dispose(&self->container);
}

void xminstruments_insert(XMInstruments* self, XMInstrument* instrument, int slot)
{
	table_insert(&self->container, slot, instrument);
}


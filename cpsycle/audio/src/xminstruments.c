// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#include "xminstruments.h"

void xminstruments_init(XMInstruments* self)
{
	InitIntHashTable(&self->container, 256);
}

void xminstruments_dispose(XMInstruments* self)
{
	DisposeIntHashTable(&self->container);
}

void xminstruments_insert(XMInstruments* self, XMInstrument* instrument, int slot)
{
	InsertIntHashTable(&self->container, slot, instrument);
}


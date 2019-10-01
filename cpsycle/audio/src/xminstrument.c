// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#include "xminstrument.h"
#include "waveio.h"
#include <string.h>
#include <stdlib.h>

void xminstrument_init(XMInstrument* self)
{
	self->name = _strdup("");
}

void xminstrument_dispose(XMInstrument* self)
{
	free(self->name);
}

void xminstrument_load(XMInstrument* self, const char* path)
{

}

void xminstrument_setname(XMInstrument* self, const char* name)
{
	free(self->name);
	self->name = _strdup(name);
}

const char* xminstrument_name(XMInstrument* self)
{
	return self->name;
}

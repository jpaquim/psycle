// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#if !defined(XMINSTRUMENTS_H)
#define XMINSTRUMENTS_H

#include "xminstrument.h"
#include <hashtbl.h>

typedef struct {
	Table container;
} XMInstruments;

void xminstruments_init(XMInstruments*);
void xminstruments_dispose(XMInstruments*);
void xminstruments_insert(XMInstruments*, XMInstrument*, int slot);
uintptr_t xminstruments_size(XMInstruments*);

#endif

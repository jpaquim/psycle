// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#if !defined(XMINSTRUMENT_H)
#define XMINSTRUMENT_H

typedef struct {
	char* name;
} XMInstrument;

void xminstrument_init(XMInstrument*);
void xminstrument_dispose(XMInstrument*);
void xminstrument_load(XMInstrument*, const char* path);
void xminstrument_setname(XMInstrument*, const char* name);
const char* xminstrument_name(XMInstrument*);

#endif
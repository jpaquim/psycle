// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net
#if !defined(EVENT_H)
#define EVENT_H

typedef struct {
	unsigned char note;
	unsigned char inst;
	unsigned char mach;
	unsigned char cmd;
	unsigned char parameter;
} PatternEvent;


void pattern_setevent(PatternEvent* self,
	unsigned char note,
	unsigned char inst,
	unsigned char mach,
	unsigned char cmd,
	unsigned char parameter);

#endif
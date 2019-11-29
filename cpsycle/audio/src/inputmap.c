// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "inputmap.h"

void inputs_init(Inputs* self)
{
	table_init(&self->map);
}

void inputs_dispose(Inputs* self)
{
	table_dispose(&self->map);
}

int inputs_cmd(Inputs* self, int input)
{
	return table_exists(&self->map, input) 
		? (int)(uintptr_t) table_at(&self->map, input)
		: -1;	
}

void inputs_define(Inputs* self, int input, int cmd)
{
	table_insert(&self->map, input, (void*)(uintptr_t)cmd);
}

unsigned int encodeinput(unsigned int keycode, int shift, int ctrl)
{
	unsigned int rv;

	rv = keycode | (shift << 8) | (ctrl << 9);

	return rv;
}

void decodeinput(unsigned int input, unsigned int* keycode, int* shift, int* ctrl)
{
	*keycode = input & 0xFF;
	*shift = ((input >> 8) & 0x01) == 0x01;
	*ctrl = ((input >> 9) & 0x01) == 0x01;	
}

// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "preset.h"
#include <string.h>
#include <stdlib.h>

void preset_init(Preset* self)
{
	self->name = strdup("");
	table_init(&self->parameters);
}

void preset_dispose(Preset* self)
{
	free(self->name);
	self->name = 0;
	table_dispose(&self->parameters);
}

Preset* preset_alloc(void)
{
	return malloc(sizeof(Preset));
}

Preset* preset_allocinit(void)
{
	Preset* rv;

	rv = preset_alloc();
	if (rv) {
		preset_init(rv);
	}
	return rv;
}

void preset_setname(Preset* self, const char* name)
{
	free(self->name);
	self->name = strdup(name);
}

const char* preset_name(Preset* self)
{
	return self->name;
}

void preset_setvalue(Preset* self, int numparam, int value)
{
	table_insert(&self->parameters, numparam, (void*)(uintptr_t)value);
}

int preset_value(Preset* self, int numparam)
{
	if (table_exists(&self->parameters, numparam)) {
		return (int)(uintptr_t) table_at(&self->parameters, numparam);
	} else {
		return 0;
	}
}

// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "preset.h"
#include <string.h>
#include <stdlib.h>

void preset_init(psy_audio_Preset* self)
{
	self->name = strdup("");
	psy_table_init(&self->parameters);
}

void preset_dispose(psy_audio_Preset* self)
{
	free(self->name);
	self->name = 0;
	psy_table_dispose(&self->parameters);
}

psy_audio_Preset* preset_alloc(void)
{
	return malloc(sizeof(psy_audio_Preset));
}

psy_audio_Preset* preset_allocinit(void)
{
	psy_audio_Preset* rv;

	rv = preset_alloc();
	if (rv) {
		preset_init(rv);
	}
	return rv;
}

void preset_setname(psy_audio_Preset* self, const char* name)
{
	free(self->name);
	self->name = strdup(name);
}

const char* preset_name(psy_audio_Preset* self)
{
	return self->name;
}

void preset_setvalue(psy_audio_Preset* self, uintptr_t param, int value)
{
	psy_table_insert(&self->parameters, param, (void*)(uintptr_t)value);
}

int preset_value(psy_audio_Preset* self, uintptr_t param)
{
	if (psy_table_exists(&self->parameters, param)) {
		return (int)(uintptr_t) psy_table_at(&self->parameters, param);
	} else {
		return 0;
	}
}

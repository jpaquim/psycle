// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "inputmap.h"
#include <string.h>

void psy_audio_inputs_init(psy_audio_Inputs* self)
{
	psy_table_init(&self->map);
	psy_table_init(&self->cmdnames);
}

void psy_audio_inputs_dispose(psy_audio_Inputs* self)
{
	psy_table_dispose(&self->map);
	psy_table_disposeall(&self->cmdnames, (psy_fp_disposefunc)NULL);
}

int psy_audio_inputs_cmd(psy_audio_Inputs* self, int input)
{
	return psy_table_exists(&self->map, input) 
		? (int)(uintptr_t) psy_table_at(&self->map, input)
		: -1;	
}

const char* psy_audio_inputs_cmdname(psy_audio_Inputs* self, int cmd)
{
	return psy_table_exists(&self->cmdnames, cmd)
		? (const char*)psy_table_at(&self->cmdnames, cmd)
		: "not defined";
}

void psy_audio_inputs_define(psy_audio_Inputs* self, int input, int cmd,
	const char* cmdname)
{
	psy_table_insert(&self->map, input, (void*)(uintptr_t)cmd);
	psy_table_insert(&self->cmdnames, cmd, cmdname ? strdup(cmdname) : "cmd");
}

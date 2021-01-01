// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "presets.h"
#include <stdlib.h>

void psy_audio_presets_init(psy_audio_Presets* self)
{	
	psy_table_init(&self->container);
}

void psy_audio_presets_dispose(psy_audio_Presets* self)
{
	psy_table_disposeall(&self->container, (psy_fp_disposefunc)
		psy_audio_preset_dispose);	
}

psy_audio_Presets* psy_audio_presets_alloc(void)
{
	return malloc(sizeof(psy_audio_Presets));
}

psy_audio_Presets* psy_audio_presets_allocinit(void)
{
	psy_audio_Presets* rv;

	rv = psy_audio_presets_alloc();
	if (rv) {
		psy_audio_presets_init(rv);
	}
	return rv;
}

void psy_audio_presets_append(psy_audio_Presets* self,
	psy_audio_Preset* preset)
{		
	psy_table_insert(&self->container,
		(psy_table_empty(&self->container))
		? 0
		: psy_table_maxkey(&self->container) + 1, preset);	
}

void psy_audio_presets_insert(psy_audio_Presets* self, uintptr_t index,
	psy_audio_Preset* preset)
{
	psy_audio_Preset* oldpreset;

	oldpreset = psy_table_at(&self->container, index);
	if (oldpreset) {
		psy_audio_preset_dispose(oldpreset);
	}
	psy_table_insert(&self->container, index, preset);
}

psy_TableIterator psy_audio_presets_begin(psy_audio_Presets* self)
{
	return psy_table_begin(&self->container);
}

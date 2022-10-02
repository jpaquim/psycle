/*
** This source is free software; you can redistribute itand /or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#ifndef psy_audio_PRESETS_H
#define psy_audio_PRESETS_H

#include "preset.h"
#include <list.h>

#include "../../detail/psydef.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct psy_audio_Presets {
	/* internal */
	psy_Table container;	
} psy_audio_Presets;

void psy_audio_presets_init(psy_audio_Presets*);
void psy_audio_presets_dispose(psy_audio_Presets*);
psy_audio_Presets* psy_audio_presets_alloc(void);
psy_audio_Presets* psy_audio_presets_allocinit(void);
void psy_audio_presets_append(psy_audio_Presets*, psy_audio_Preset*);
void psy_audio_presets_insert(psy_audio_Presets*, uintptr_t index,
	psy_audio_Preset*);
void psy_audio_presets_remove(psy_audio_Presets*, uintptr_t index);
void psy_audio_presets_remove_empty(psy_audio_Presets*);
psy_TableIterator psy_audio_presets_begin(psy_audio_Presets*);
INLINE psy_audio_Preset* psy_audio_presets_at(psy_audio_Presets* self,
	uintptr_t index)
{
	return (psy_audio_Preset*)psy_table_at(&self->container, index);
}

INLINE bool psy_audio_presets_empty(psy_audio_Presets* self)
{
	return psy_table_empty(&self->container);
}

INLINE uintptr_t psy_audio_presets_size(psy_audio_Presets* self)
{
	return psy_table_size(&self->container);
}

#ifdef __cplusplus
}
#endif

#endif /* psy_audio_PRESETS_H */

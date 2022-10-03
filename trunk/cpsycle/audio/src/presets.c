/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "presets.h"
/* container */
#include <qsort.h>
/* platform */
#include "../../detail/portable.h"


/* prototypes */
static int psy_audio_presets_comp_name(psy_audio_Preset* p,
	psy_audio_Preset* q);

/* implementation */
void psy_audio_presets_init(psy_audio_Presets* self)
{	
	assert(self);
	
	psy_table_init(&self->container);
}

void psy_audio_presets_dispose(psy_audio_Presets* self)
{
	assert(self);
	
	psy_table_dispose_all(&self->container, (psy_fp_disposefunc)
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
	assert(self);
	
	psy_table_insert(&self->container,
		(psy_table_empty(&self->container))
		? 0
		: psy_table_maxkey(&self->container) + 1, preset);	
}

void psy_audio_presets_insert(psy_audio_Presets* self, uintptr_t index,
	psy_audio_Preset* preset)
{
	psy_audio_Preset* oldpreset;
	
	assert(self);

	oldpreset = psy_table_at(&self->container, index);
	if (oldpreset) {
		psy_audio_preset_dispose(oldpreset);
	}
	psy_table_insert(&self->container, index, preset);
}

void psy_audio_presets_remove(psy_audio_Presets* self, uintptr_t index)
{
	psy_audio_Preset* oldpreset;
	
	assert(self);

	oldpreset = psy_table_at(&self->container, index);
	psy_table_remove(&self->container, index);
	if (oldpreset) {
		psy_audio_preset_dispose(oldpreset);
	}	
}

void psy_audio_presets_remove_empty(psy_audio_Presets* self)
{
	psy_List* list;
	psy_List* p;
	psy_TableIterator it;
	uintptr_t i;
	
	assert(self);
	
	list = NULL;
	for (it = psy_audio_presets_begin(self);
			!psy_tableiterator_equal(&it, psy_table_end());
			psy_tableiterator_inc(&it)) {			
		psy_audio_Preset* preset;

		preset = (psy_audio_Preset*)psy_tableiterator_value(&it);
		if (preset) {
			psy_list_append(&list, preset);
		}
	}
	psy_table_clear(&self->container);
	for (p = list, i = 0; p != NULL; p = p->next, ++i) {		
		psy_table_insert(&self->container, i, (psy_audio_Preset*)p->entry);
	}
	psy_list_free(list);
	list = NULL;
}

void psy_audio_presets_merge(psy_audio_Presets* self, psy_audio_Presets* src)
{
	psy_List* list;
	psy_List* p;
	uintptr_t i;
	psy_TableIterator it;
	
	assert(self);
	
	if (!src) {
		return;
	}
	list = NULL;
	for (it = psy_audio_presets_begin(self);
			!psy_tableiterator_equal(&it, psy_table_end());
			psy_tableiterator_inc(&it)) {			
		psy_audio_Preset* preset;

		preset = (psy_audio_Preset*)psy_tableiterator_value(&it);
		if (preset) {
			psy_list_append(&list, preset);
		}
	}
	for (it = psy_audio_presets_begin(src);
			!psy_tableiterator_equal(&it, psy_table_end());
			psy_tableiterator_inc(&it)) {			
		psy_audio_Preset* preset;

		preset = (psy_audio_Preset*)psy_tableiterator_value(&it);
		if (preset) {
			psy_list_append(&list, preset);
		}
	}
	psy_table_clear(&self->container);
	psy_table_clear(&src->container);
	for (p = list, i = 0; p != NULL; p = p->next, ++i) {		
		psy_table_insert(&self->container, i, (psy_audio_Preset*)p->entry);
	}
	psy_list_free(list);
	list = NULL;
	psy_audio_presets_sort(self);
}

void psy_audio_presets_sort(psy_audio_Presets* self)
{
	uintptr_t i;
	
	assert(self);
	
	if (psy_table_size(&self->container) <= 1) {
		return;
	}
	i = psy_table_maxkey(&self->container);	
	psy_qsort(&self->container,
		(psy_fp_set_index_double)psy_table_insert,
		(psy_fp_index_double)psy_table_at,
		0, (int)(i), (psy_fp_comp)psy_audio_presets_comp_name);
}

psy_TableIterator psy_audio_presets_begin(psy_audio_Presets* self)
{
	assert(self);
	
	return psy_table_begin(&self->container);
}

int psy_audio_presets_comp_name(psy_audio_Preset* p,
	psy_audio_Preset* q)
{
	const char* left;
	const char* right;
	
	if ((p == NULL) || (q == NULL)) {
		return 0;
	}
	left = psy_audio_preset_name(p);	
	right = psy_audio_preset_name(q);	
	return strcmp(left, right);
}

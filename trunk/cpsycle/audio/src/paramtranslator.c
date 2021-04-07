// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "paramtranslator.h"
// std
#include <stdlib.h>

void psy_audio_paramtranslator_init(psy_audio_ParamTranslator* self)
{
	assert(self);

	psy_table_init(&self->container);	
}

void psy_audio_paramtranslator_dispose(psy_audio_ParamTranslator* self)
{	
	psy_table_dispose(&self->container);
}

psy_audio_ParamTranslator* psy_audio_paramtranslator_alloc(void)
{
	return malloc(sizeof(psy_audio_ParamTranslator));
}

psy_audio_ParamTranslator* psy_audio_paramtranslator_allocinit(void)
{
	psy_audio_ParamTranslator* rv;

	rv = psy_audio_paramtranslator_alloc();
	if (rv) {
		psy_audio_paramtranslator_init(rv);
	}
	return rv;
}

void psy_audio_paramtranslator_clear(psy_audio_ParamTranslator* self)
{
	assert(self);

	psy_table_clear(&self->container);	
}

void psy_audio_paramtranslator_set_virtual_index(
	psy_audio_ParamTranslator* self, uintptr_t virtual_index,
	uintptr_t machine_index)
{
	assert(self);

	if (virtual_index == machine_index) {
		psy_table_remove(&self->container, virtual_index);
	} else {
		psy_table_insert(&self->container, virtual_index, (void*)machine_index);
	}
}

uintptr_t psy_audio_paramtranslator_translate(
	const psy_audio_ParamTranslator* self, uintptr_t virtual_index)
{	
	assert(self);

	if (psy_table_exists(&self->container, virtual_index)) {
		return (uintptr_t)psy_table_at_const(&self->container, virtual_index);
	}
	return virtual_index;
}

uintptr_t psy_audio_paramtranslator_virtual_index(
	const psy_audio_ParamTranslator* self, uintptr_t machine_index)
{
	uintptr_t rv;
	psy_TableIterator it;

	assert(self);

	rv = machine_index;
	for (it = psy_table_begin(&((psy_audio_ParamTranslator*)self)->container);
			!psy_tableiterator_equal(&it, psy_table_end());
			psy_tableiterator_inc(&it)) {
		if ((uintptr_t)psy_tableiterator_value(&it) == machine_index) {
			rv = psy_tableiterator_key(&it);
			break;
		}		
	}
	return rv;
}
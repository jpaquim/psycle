// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include <assert.h>

#include "constants.h"
#include "logicalchannels.h"

void psy_audio_logicalchannels_init(psy_audio_LogicalChannels* self)
{	
	assert(self);

	psy_table_init(&self->physicalmap);
	psy_table_init(&self->logicalmap);
}

void psy_audio_logicalchannels_dispose(psy_audio_LogicalChannels* self)
{
	psy_table_dispose(&self->physicalmap);
	psy_table_dispose(&self->logicalmap);
}

uintptr_t psy_audio_logicalchannels_physical(psy_audio_LogicalChannels* self, uintptr_t logical)
{
	uintptr_t rv;

	if (psy_table_exists(&self->logicalmap, logical)) {
		rv = (uintptr_t)psy_table_at(&self->logicalmap, logical);
	} else {		
		rv = logical % MAX_TRACKS;
		if (!psy_table_exists(&self->physicalmap, rv)) {
			// physical free
			psy_table_insert(&self->physicalmap, rv, (void*)logical);
		} else {
			// physical channel not free
			if (psy_table_size(&self->physicalmap) >= MAX_TRACKS) {
				// no physical channels free
				// todo
			}
			// find free physical channel
			for (rv = 0; rv < 64; ++rv) {
				if (!psy_table_exists(&self->physicalmap, rv)) {
					psy_table_insert(&self->physicalmap, rv, (void*)logical);
					break;
				}
			}			
		}
		psy_table_insert(&self->logicalmap, logical, (void*)rv);		
	}
	return rv;
}

void psy_audio_logicalchannels_reset(psy_audio_LogicalChannels* self)
{
	psy_table_clear(&self->physicalmap);
	psy_table_clear(&self->logicalmap);
}

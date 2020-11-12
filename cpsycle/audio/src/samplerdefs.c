// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "samplerdefs.h"

#include <stdlib.h>

// Sampler Configuration
void psy_audio_xmsamplercmd_init_all(psy_audio_XMSamplerCmd* self,
	int id, int patternslot, int mask)
{
	self->id = id;
	self->patternslot = patternslot;
	self->mode = mask;
}

void psy_audio_xmsamplercmd_dispose(psy_audio_XMSamplerCmd* self)
{	
}

psy_audio_XMSamplerCmd* psy_audio_xmsamplercmd_alloc(void)
{
	return (psy_audio_XMSamplerCmd*)malloc(sizeof(psy_audio_XMSamplerCmd));
}

psy_audio_XMSamplerCmd* psy_audio_xmsamplercmd_allocinit_all(int id,
	int patternslot, int mask)
{
	psy_audio_XMSamplerCmd* rv;
	
	rv = psy_audio_xmsamplercmd_alloc();
	if (rv) {
		psy_audio_xmsamplercmd_init_all(rv, id, patternslot, mask);
	}
	return rv;
}

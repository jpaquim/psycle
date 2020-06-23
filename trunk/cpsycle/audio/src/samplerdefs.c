// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "samplerdefs.h"

#include <stdlib.h>

// Sampler Configuration
void psy_audio_samplercmd_init_all(psy_audio_SamplerCmd* self,
	int id, int patternid, int mask)
{
	self->id = id;
	self->patternid = patternid;
	self->mode = mask;
}

void psy_audio_samplercmd_dispose(psy_audio_SamplerCmd* self)
{	
}

psy_audio_SamplerCmd* psy_audio_samplercmd_alloc(void)
{
	return (psy_audio_SamplerCmd*)malloc(sizeof(psy_audio_SamplerCmd));
}

psy_audio_SamplerCmd* psy_audio_samplercmd_allocinit_all(int id,
	int patternid, int mask)
{
	psy_audio_SamplerCmd* rv;
	
	rv = psy_audio_samplercmd_alloc();
	if (rv) {
		psy_audio_samplercmd_init_all(rv, id, patternid, mask);
	}
	return rv;
}

/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "ladspainterface.h"
/* local */
#include "buffercontext.h"
/* std */
#include <stdlib.h>

static bool psy_audio_ladspainterface_valid(const
	psy_audio_LadspaInterface* self)
{
	assert(self);
	
	return (self->handle && self->descriptor);
}

void psy_audio_ladspainterface_init(psy_audio_LadspaInterface* self,
	const LADSPA_Descriptor* descriptor, LADSPA_Handle handle)
{
	assert(self);
	
	self->descriptor = descriptor;
	self->handle = handle;
}

void psy_audio_ladspainterface_activate(psy_audio_LadspaInterface* self)
{	
	assert(self);
	
	if (!psy_audio_ladspainterface_valid(self)) {
		return;
	}
	if (self->descriptor->activate) {
		self->descriptor->activate(self->handle);					
	}
}

void psy_audio_ladspainterface_deactivate(psy_audio_LadspaInterface*
	self)
{	
	assert(self);
	
	if (!psy_audio_ladspainterface_valid(self)) {
		return;
	}
	if (self->descriptor->deactivate) {
		self->descriptor->deactivate(self->handle);					
	}
}

void psy_audio_ladspainterface_cleanup(psy_audio_LadspaInterface*
	self)
{	
	assert(self);
	
	if (!psy_audio_ladspainterface_valid(self)) {
		return;
	}
	if (self->descriptor->cleanup) {
		self->descriptor->cleanup(self->handle);					
	}
}

void psy_audio_ladspainterface_run(psy_audio_LadspaInterface* self,
	uint32_t num_samples)
{
	assert(self);
	
	if (!psy_audio_ladspainterface_valid(self)) {
		return;
	}
	self->descriptor->run(self->handle, num_samples);
}

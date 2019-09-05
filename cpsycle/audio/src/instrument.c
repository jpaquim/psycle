// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#include "instrument.h"
#include <string.h>
#include <stdlib.h>

void instrument_init(Instrument* self)
{	
	self->name = strdup("");
	self->nna = NNA_STOP;
	adsr_envelopesettings_initdefault(&self->volumeenvelope);
	adsr_envelopesettings_initdefault(&self->filterenvelope);
}

void instrument_dispose(Instrument* self)
{
	free(self->name);
}

void instrument_load(Instrument* self, const char* path)
{	
}

void instrument_setname(Instrument* self, const char* name)
{
	free(self->name);
	self->name = strdup(name);
}

const char* instrument_name(Instrument* self)
{
	return self->name;
}

void instrument_setnna(Instrument* self, NewNoteAction nna)
{
	self->nna = nna;
}

NewNoteAction instrument_nna(Instrument* self)
{
	return self->nna;
}
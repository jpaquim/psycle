// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "instrument.h"
#include <string.h>
#include <stdlib.h>

void instrument_init(Instrument* self)
{	
	self->name = _strdup("");
	self->nna = NNA_STOP;
	self->filtertype = F_NONE;
	adsr_settings_initdefault(&self->volumeenvelope);
	adsr_settings_init(&self->filterenvelope, 0.005f, 0.370f, 0.5f, 0.370f);
	self->filtermodamount = 1.0f;
}

void instrument_dispose(Instrument* self)
{
	free(self->name);
}

Instrument* instrument_alloc(void)
{
	return (Instrument*)malloc(sizeof(Instrument));
}

Instrument* instrument_allocinit(void)
{
	Instrument* rv;

	rv = instrument_alloc();
	if (rv) {
		instrument_init(rv);
	}
	return rv;
}

void instrument_load(Instrument* self, const char* path)
{	
}

void instrument_setname(Instrument* self, const char* name)
{
	free(self->name);
	self->name = _strdup(name);
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
// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "duplicatormap.h"
#include <stdlib.h>

static int duplicatormap_isavail(psy_audio_DuplicatorMap* self, int machine,
	uintptr_t channel);
static void duplicatormap_setavail(psy_audio_DuplicatorMap* self, int machine,
	uintptr_t channel);
static void duplicatormap_setunavail(psy_audio_DuplicatorMap* self, int machine,
	uintptr_t channel);
static void duplicatoroutput_dispose(psy_audio_DuplicatorOutput*);
static void duplicatoroutput_clear(psy_audio_DuplicatorOutput*);

// Output
void duplicatoroutput_dispose(psy_audio_DuplicatorOutput* self)
{
	psy_table_dispose(&self->allochans);
	self->machine = -1;
	self->offset = 0;
	self->lowkey = NOTECOMMANDS_C0;
	self->highkey = NOTECOMMANDS_B9;
}

void psy_audio_duplicatoroutput_setall(psy_audio_DuplicatorOutput* self,
	int machine, int offset, int lowkey, int highkey)
{
	self->machine = machine;
	self->offset = offset;
	self->lowkey = lowkey;
	self->highkey = highkey;
}

void duplicatoroutput_clear(psy_audio_DuplicatorOutput* self)
{
	psy_table_clear(&self->allochans);	
}

// Map
void psy_audio_duplicatormap_init(psy_audio_DuplicatorMap* self,
	uintptr_t numoutputs, uintptr_t maxtracks)
{
	uintptr_t i;

	self->maxtracks = maxtracks;
	psy_table_init(&self->outputs);
	psy_table_init(&self->unavail);
	for (i = 0; i < numoutputs; ++i) {
		psy_audio_DuplicatorOutput* output;

		output = (psy_audio_DuplicatorOutput*) malloc(sizeof(
			psy_audio_DuplicatorOutput));
		output->machine = -1;
		output->offset = 0;
		output->lowkey = 0;
		output->highkey = 119;
		psy_table_init(&output->allochans);
		psy_table_insert(&self->outputs, i, output);
	}	
}

void psy_audio_duplicatormap_dispose(psy_audio_DuplicatorMap* self)
{
	psy_table_disposeall(&self->outputs, (psy_fp_disposefunc)
		duplicatoroutput_dispose);	
	psy_table_dispose(&self->unavail);		
}

void psy_audio_duplicatormap_clear(psy_audio_DuplicatorMap* self)
{	
	psy_TableIterator it;
	
	for (it = psy_table_begin(&self->outputs);
			!psy_tableiterator_equal(&it, psy_table_end());
			psy_tableiterator_inc(&it)) {
		psy_audio_DuplicatorOutput* output;

		output = psy_tableiterator_value(&it);
		duplicatoroutput_clear(output);
	}
	psy_table_clear(&self->unavail);
}

int psy_audio_duplicatormap_channel(psy_audio_DuplicatorMap* self,
	uintptr_t patternchannel,
	psy_audio_DuplicatorOutput* output)
{
	uintptr_t j;
	int repeat = 0;
		
	j = patternchannel;
	if (psy_table_exists(&output->allochans, j)) {
		j = (int) (intptr_t) psy_table_at(&output->allochans, j);
	} else {						
	while (!duplicatormap_isavail(self, output->machine, j)) {
		if (repeat) {
			if (j == patternchannel) {
				break;
			}
		}
		++j;
		if (j >= self->maxtracks) {
			j = 0;
			repeat = 1;
		}
	}	
	psy_table_insert(&output->allochans, patternchannel,
		(void*) (intptr_t) j);
		duplicatormap_setunavail(self, output->machine, j);
	}
	return j;
}

void psy_audio_duplicatormap_release(psy_audio_DuplicatorMap* self,
	uintptr_t patternchannel, uintptr_t duplicatorchannel,
	psy_audio_DuplicatorOutput* output)
{	
	duplicatormap_setavail(self, output->machine, duplicatorchannel);
	psy_table_remove(&output->allochans, patternchannel);
}

psy_TableIterator psy_audio_duplicatormap_begin(psy_audio_DuplicatorMap* self)
{
	return psy_table_begin(&self->outputs);
}

// private
int duplicatormap_isavail(psy_audio_DuplicatorMap* self, int machine,
	uintptr_t channel)
{
	uintptr_t index;
	
	index = self->maxtracks * machine + channel;
	return !psy_table_exists(&self->unavail, index);
}

void duplicatormap_setavail(psy_audio_DuplicatorMap* self, int machine,
	uintptr_t channel)
{
	uintptr_t index;
	
	index = self->maxtracks * machine + channel;
	psy_table_remove(&self->unavail, index);
}

void duplicatormap_setunavail(psy_audio_DuplicatorMap* self, int machine,
	uintptr_t channel)
{
	uintptr_t index;
	
	index = self->maxtracks * machine + channel;
	psy_table_insert(&self->unavail, index, (void*) (intptr_t) 1);
}

psy_audio_DuplicatorOutput* psy_audio_duplicatormap_output(
	psy_audio_DuplicatorMap* self, int output)
{	
	return (psy_audio_DuplicatorOutput*) psy_table_at(
		&self->outputs, output);	
}

uintptr_t psy_audio_duplicatormap_numoutputs(psy_audio_DuplicatorMap* self)
{
	return psy_table_size(&self->outputs);
}

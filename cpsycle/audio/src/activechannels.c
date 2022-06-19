/*
** This source is free software; you can redistribute itand /or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2, or (at your option) any later version.
** copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


/* local */
#include "activechannels.h"
#include "machines.h"
/* std */
#include <assert.h>
#include <stdlib.h>
#include <string.h>

void psy_audio_activechannelmacs_init(psy_audio_ActiveChannelMacs* self)
{
	assert(self);

	psy_table_init(&self->activemacs);
}

void psy_audio_activechannelmacs_dispose(psy_audio_ActiveChannelMacs* self)
{
	assert(self);

	psy_table_dispose(&self->activemacs);
}

psy_audio_ActiveChannelMacs* psy_audio_activechannelmacs_alloc(void)
{
	return (psy_audio_ActiveChannelMacs*)malloc(sizeof(psy_audio_ActiveChannelMacs));
}

psy_audio_ActiveChannelMacs* psy_audio_activechannelmacs_allocinit(void)
{
	psy_audio_ActiveChannelMacs* rv;
	
	rv = psy_audio_activechannelmacs_alloc();
	if (rv) {
		psy_audio_activechannelmacs_init(rv);
	}
	return rv;
}

void psy_audio_activechannelmacs_deallocate(psy_audio_ActiveChannelMacs* self)
{
	assert(self);

	psy_audio_activechannelmacs_dispose(self);
	free(self);
}

void psy_audio_activechannelmacs_set(psy_audio_ActiveChannelMacs* self, uintptr_t mac)
{
	assert(self);

	psy_table_insert(&self->activemacs, mac, (void*)(uintptr_t)TRUE);
}

void psy_audio_activechannelmacs_clear(psy_audio_ActiveChannelMacs* self, uintptr_t mac)
{
	assert(self);

	psy_table_remove(&self->activemacs, mac);
}

bool psy_audio_activechannelmacs_empty(const psy_audio_ActiveChannelMacs* self)
{
	assert(self);

	return psy_table_size(&self->activemacs) == 0;
}

bool psy_audio_activechannelmacs_active(psy_audio_ActiveChannelMacs* self, uintptr_t mac)
{
	assert(self);

	return psy_table_exists(&self->activemacs, mac);
}

/* ActiveChannels */
void psy_audio_activechannels_init(psy_audio_ActiveChannels* self)
{	
	assert(self);

	psy_table_init(&self->channels);
	self->machines = NULL;
}

void psy_audio_activechannels_dispose(psy_audio_ActiveChannels* self)
{	
	assert(self);

	psy_table_dispose_all(&self->channels,
		(psy_fp_disposefunc)psy_audio_activechannelmacs_dispose);
}

void psy_audio_activechannels_setmachines(psy_audio_ActiveChannels* self,
	struct psy_audio_Machines* machines)
{
	assert(self);

	self->machines = machines;
}

void psy_audio_activechannels_reset(psy_audio_ActiveChannels* self)
{
	assert(self);

	psy_table_dispose_all(&self->channels,
		(psy_fp_disposefunc)psy_audio_activechannelmacs_dispose);
	psy_table_init(&self->channels);
}

void psy_audio_activechannels_write(psy_audio_ActiveChannels* self,
	uintptr_t channel , psy_audio_PatternEvent* ev)
{
	psy_audio_ActiveChannelMacs* macs;

	assert(self);

	macs = psy_audio_activechannels_activemacs(self, channel);
	if (!macs) {
		macs = psy_audio_activechannelmacs_allocinit();
		assert(macs);
		psy_table_insert(&self->channels, channel, macs);
	}
	if (ev->note < psy_audio_NOTECOMMANDS_RELEASE) {
		if (!self->machines || psy_audio_machines_at(self->machines, ev->mach)) {
			psy_audio_activechannelmacs_set(macs, ev->mach);
		}
	} else {
		psy_audio_activechannelmacs_clear(macs, ev->mach);
		if (psy_audio_activechannelmacs_empty(macs)) {
			psy_table_remove(&self->channels, channel);
			psy_audio_activechannelmacs_deallocate(macs);
		}
	}
}

bool psy_audio_activechannels_playon(const psy_audio_ActiveChannels* self, uintptr_t channel)
{
	psy_audio_ActiveChannelMacs* macs;

	assert(self);

	macs = psy_audio_activechannels_activemacs((psy_audio_ActiveChannels*)self, channel);
	if (macs) {
		return !psy_audio_activechannelmacs_empty(macs);		
	}
	return FALSE;
}

bool psy_audio_activechannels_playonmac(psy_audio_ActiveChannels* self,
	uintptr_t channel, uintptr_t macid)
{
	psy_audio_ActiveChannelMacs* macs;

	assert(self);

	macs = psy_audio_activechannels_activemacs(self, channel);
	if (macs) {		
		return psy_audio_activechannelmacs_active(macs, macid);		
	}
	return FALSE;
}

psy_audio_ActiveChannelMacs* psy_audio_activechannels_activemacs(
	psy_audio_ActiveChannels* self, uintptr_t channel)
{
	assert(self);

	return (psy_audio_ActiveChannelMacs*)psy_table_at(&self->channels,
		channel);
}

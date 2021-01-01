// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#ifndef psy_audio_ACTIVECHANNELS_H
#define psy_audio_ACTIVECHANNELS_H

#include <hashtbl.h>

#include "patternevent.h"

#ifdef __cplusplus
extern "C" {
#endif

// psy_audio_ActiveChannels

typedef struct psy_audio_ActiveChannelMacs
{
	psy_Table activemacs;
} psy_audio_ActiveChannelMacs;

void psy_audio_activechannelmacs_init(psy_audio_ActiveChannelMacs*);
void psy_audio_activechannelmacs_dispose(psy_audio_ActiveChannelMacs*);
psy_audio_ActiveChannelMacs* psy_audio_activechannelmacs_alloc(void);
psy_audio_ActiveChannelMacs* psy_audio_activechannelmacs_allocinit(void);
void psy_audio_activechannelmacs_deallocate(psy_audio_ActiveChannelMacs*);
void psy_audio_activechannelmacs_set(psy_audio_ActiveChannelMacs*, uintptr_t macid);
void psy_audio_activechannelmacs_clear(psy_audio_ActiveChannelMacs*, uintptr_t macid);
bool psy_audio_activechannelmacs_empty(psy_audio_ActiveChannelMacs*);
bool psy_audio_activechannelmacs_active(psy_audio_ActiveChannelMacs*, uintptr_t macid);

struct psy_audio_Machines;

typedef struct psy_audio_ActiveChannels
{
	psy_Table channels;
	struct psy_audio_Machines* machines;
} psy_audio_ActiveChannels;

void psy_audio_activechannels_init(psy_audio_ActiveChannels*);
void psy_audio_activechannels_dispose(psy_audio_ActiveChannels*);

void psy_audio_activechannels_reset(psy_audio_ActiveChannels*);
void psy_audio_activechannels_setmachines(psy_audio_ActiveChannels*,
	struct psy_audio_Machines* machines);
void psy_audio_activechannels_write(psy_audio_ActiveChannels*,
	uintptr_t channel, psy_audio_PatternEvent*);
bool psy_audio_activechannels_playon(psy_audio_ActiveChannels*,
	uintptr_t channel);
bool psy_audio_activechannels_playonmac(psy_audio_ActiveChannels*,
	uintptr_t channel, uintptr_t mac);
psy_audio_ActiveChannelMacs* psy_audio_activechannels_activemacs(
	psy_audio_ActiveChannels*, uintptr_t channel);


#ifdef __cplusplus
}
#endif

#endif /* psy_audio_ACTIVECHANNELS_H */

/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net
*/

#ifndef psy_audio_EVENTDRIVERS_H
#define psy_audio_EVENTDRIVERS_H

/* driver */
#include "../../driver/eventdriver.h"
/* local */
#include "library.h"
/* container */
#include <list.h>
#include <hashtbl.h>
#include <signal.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
** psy_audio_EventDrivers
**
** container for all active inputdriver
*/

typedef struct {
	psy_EventDriver* eventdriver;
	psy_Library* library;
} psy_audio_EventDriverEntry;

typedef struct {
	psy_EventDriver* kbddriver;
	psy_EventDriver* cmddriver;
	psy_List* eventdrivers;
	void* context;
	void* systemhandle;	
	psy_Signal signal_input;
	psy_Property* cmds;
	psy_Table guids;	
} psy_audio_EventDrivers;

void psy_audio_eventdrivers_init(psy_audio_EventDrivers*, void* systemhandle);
void psy_audio_eventdrivers_dispose(psy_audio_EventDrivers*);
void psy_audio_eventdrivers_initkbd(psy_audio_EventDrivers*);
void psy_audio_eventdrivers_register(psy_audio_EventDrivers*,
	intptr_t guid, const char* path);
psy_EventDriver* psy_audio_eventdrivers_loadbyguid(psy_audio_EventDrivers*,
	intptr_t guid);
psy_EventDriver* psy_audio_eventdrivers_load(psy_audio_EventDrivers*, const char* path);
intptr_t psy_audio_eventdrivers_guid(psy_audio_EventDrivers*,
	const char* path);
void psy_audio_eventdrivers_restart(psy_audio_EventDrivers*, intptr_t id,
	psy_Property* configuration);
void psy_audio_eventdrivers_restart_all(psy_audio_EventDrivers*);
void psy_audio_eventdrivers_remove(psy_audio_EventDrivers*, intptr_t id);
void psy_audio_eventdrivers_setcmds(psy_audio_EventDrivers*, psy_Property* self);
uintptr_t psy_audio_eventdrivers_size(psy_audio_EventDrivers*);
psy_EventDriver* psy_audio_eventdrivers_driver(psy_audio_EventDrivers*, intptr_t id);
psy_audio_EventDriverEntry* psy_audio_eventdrivers_entry(psy_audio_EventDrivers*, intptr_t id);
void psy_audio_eventdrivers_idle(psy_audio_EventDrivers*);
void psy_audio_eventdrivers_sendcmd(psy_audio_EventDrivers*,
	const char* section, psy_EventDriverCmd cmd);

#ifdef __cplusplus
}
#endif

#endif /* psy_audio_EVENTDRIVERS_H */

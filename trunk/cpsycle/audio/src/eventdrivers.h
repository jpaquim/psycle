// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#ifndef psy_audio_EVENTDRIVERS_H
#define psy_audio_EVENTDRIVERS_H

#include "../../driver/eventdriver.h"
#include "library.h"
#include "list.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	psy_EventDriver* eventdriver;
	psy_Library* library;
} EventDriverEntry;

typedef struct {
	psy_EventDriver* kbddriver;
	psy_List* eventdrivers;
	void* context;
	void* systemhandle;	
	psy_Signal signal_input;
	psy_Properties* cmds;
} EventDrivers;

void eventdrivers_init(EventDrivers*, void* systemhandle);
void eventdrivers_dispose(EventDrivers*);
void eventdrivers_initkbd(EventDrivers*);
void eventdrivers_load(EventDrivers*, const char* path);
void eventdrivers_restart(EventDrivers*, int id);
void eventdrivers_restartall(EventDrivers*);
void eventdrivers_remove(EventDrivers*, int id);
void eventdrivers_setcmds(EventDrivers*, psy_Properties* self);
unsigned int eventdrivers_size(EventDrivers*);
psy_EventDriver* eventdrivers_driver(EventDrivers*, int id); 
EventDriverEntry* eventdrivers_entry(EventDrivers*, int id);

#ifdef __cplusplus
}
#endif

#endif /* psy_audio_EVENTDRIVERS_H */

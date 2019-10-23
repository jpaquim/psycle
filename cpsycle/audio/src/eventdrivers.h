// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#if !defined(EVENTDRIVERS_H)
#define EVENTDRIVERS_H

#include "../../driver/eventdriver.h"
#include "library.h"
#include "list.h"

typedef struct {
	EventDriver* eventdriver;
	Library* library;
} EventDriverEntry;

typedef struct {
	EventDriver* kbddriver;
	List* eventdrivers;
	void* context;
	void* systemhandle;
	EVENTDRIVERWORKFN callback;
} EventDrivers;

void eventdrivers_init(EventDrivers*, void* context, EVENTDRIVERWORKFN,
	void* systemhandle);
void eventdrivers_dispose(EventDrivers*);
void eventdrivers_initkbd(EventDrivers*);
void eventdrivers_load(EventDrivers*, const char* path);
void eventdrivers_restart(EventDrivers*, int id);
void eventdrivers_remove(EventDrivers*, int id);
unsigned int eventdrivers_size(EventDrivers*);
EventDriver* eventdrivers_driver(EventDrivers*, int id); 
EventDriverEntry* eventdrivers_entry(EventDrivers*, int id);

#endif

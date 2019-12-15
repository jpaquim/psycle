// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "eventdrivers.h"
#include "kbddriver.h"
#include "inputmap.h"
#include "windows.h"

#include <stdlib.h>
#include <string.h>

static void eventdrivers_ondriverinput(EventDrivers*, EventDriver*);

void eventdrivers_init(EventDrivers* self, void* systemhandle)
{
	self->eventdrivers = 0;	
	self->kbddriver = 0;	
	self->systemhandle = systemhandle;
	self->cmds = 0;
	psy_signal_init(&self->signal_input);
	eventdrivers_initkbd(self);
}

void eventdrivers_initkbd(EventDrivers* self)
{
	EventDriver* eventdriver;
	EventDriverEntry* eventdriverentry;

	eventdriver = create_kbd_driver();	
	self->kbddriver = eventdriver;	
	eventdriverentry = (EventDriverEntry*) malloc(sizeof(EventDriverEntry));
	eventdriverentry->eventdriver = eventdriver;
	eventdriverentry->library = 0;
	list_append(&self->eventdrivers, eventdriverentry);
	psy_signal_connect(&eventdriver->signal_input, self,
		eventdrivers_ondriverinput);
}

void eventdrivers_dispose(EventDrivers* self)
{
	List* p;	

	for (p = self->eventdrivers; p != 0; p = p->next) {
		EventDriverEntry* eventdriverentry;
		EventDriver* eventdriver;
		
		eventdriverentry = (EventDriverEntry*)p->entry;
		eventdriver = eventdriverentry->eventdriver;
		eventdriver->close(eventdriver);
		eventdriver->dispose(eventdriver);
		eventdriver->free(eventdriver);
//		free(eventdriver);
		if (eventdriverentry && eventdriverentry->library) {
			library_unload(eventdriverentry->library);
			library_disposefree(eventdriverentry->library);			
		}
		free(eventdriverentry);
	}
	list_free(self->eventdrivers);
	self->eventdrivers = 0;
	self->cmds = 0;
	psy_signal_dispose(&self->signal_input);
}

void eventdrivers_load(EventDrivers* self, const char* path)
{
	EventDriver* eventdriver = 0;	
	
	if (path) {
		if (strcmp(path, "kbd") == 0) {
			if (!self->kbddriver) {
				eventdrivers_initkbd(self);
			}
		} else {
			Library* library;
			EventDriver* eventdriver = 0;

			library = library_allocinit();			
			library_load(library, path);
			if (!library_empty(library)) {				
				pfneventdriver_create fpeventdrivercreate;

				fpeventdrivercreate = (pfneventdriver_create)
					library_functionpointer(library, "eventdriver_create");
				if (fpeventdrivercreate) {
					EventDriverEntry* eventdriverentry;

					eventdriver = fpeventdrivercreate();
					eventdriver->setcmddef(eventdriver, self->cmds);					
					eventdriverentry = (EventDriverEntry*) malloc(sizeof(EventDriverEntry));
					eventdriverentry->eventdriver = eventdriver;
					eventdriverentry->library = library;
					list_append(&self->eventdrivers, eventdriverentry);				
					eventdriver->open(eventdriver);
					psy_signal_connect(&eventdriver->signal_input, self,
						eventdrivers_ondriverinput);
				}
			}
			if (!eventdriver) {
				library_disposefree(library);
			}
		}
	}
}

void eventdrivers_restart(EventDrivers* self, int id)
{	
	EventDriver* eventdriver;

	eventdriver = eventdrivers_driver(self, id);
	if (eventdriver) {
		eventdriver->close(eventdriver);	
		eventdriver->configure(eventdriver);
		eventdriver->open(eventdriver);	
	}
}

void eventdrivers_restartall(EventDrivers* self)
{
	List* p;	

	for (p = self->eventdrivers; p != 0; p = p->next) {
		EventDriverEntry* eventdriverentry;
		EventDriver* eventdriver;
		
		eventdriverentry = (EventDriverEntry*)p->entry;
		eventdriver = eventdriverentry->eventdriver;
		if (eventdriver) {
			eventdriver->close(eventdriver);	
			eventdriver->configure(eventdriver);
			eventdriver->open(eventdriver);	
		}
	}
}

void eventdrivers_remove(EventDrivers* self, int id)
{
	EventDriver* eventdriver;

	eventdriver = eventdrivers_driver(self, id);	
	if (eventdriver) {
		EventDriverEntry* eventdriverentry;
		List* p;

		eventdriver->close(eventdriver);
		eventdriver->dispose(eventdriver);
		eventdriver->free(eventdriver);
//		free(eventdriver);
		if (eventdriver == self->kbddriver) {
			self->kbddriver = 0;
		}
		eventdriverentry = eventdrivers_entry(self, id);
		if (eventdriverentry && eventdriverentry->library) {
			library_unload(eventdriverentry->library);
			library_disposefree(eventdriverentry->library);			
			eventdriverentry->library = 0;
		}
		for (p = self->eventdrivers; p != 0; p = p->next) {
			if (((EventDriverEntry*)p->entry)->eventdriver == eventdriver) {
				list_remove(&self->eventdrivers, p);
				break;
			}
		}
		free(eventdriverentry);
	}
}

unsigned int eventdrivers_size(EventDrivers* self)
{
	int rv = 0;
	List* p;
	
	for (p = self->eventdrivers; p != 0; p = p->next, ++rv);
	return rv;
}

EventDriverEntry* eventdrivers_entry(EventDrivers* self, int id)
{
	List* p;
	int c = 0;

	for (p = self->eventdrivers; p != 0 && id != c; p = p->next, ++c);
	
	return p ? ((EventDriverEntry*) (p->entry)) : 0;
}

EventDriver* eventdrivers_driver(EventDrivers* self, int id) 
{
	List* p;
	int c = 0;

	for (p = self->eventdrivers; p != 0 && id != c; p = p->next, ++c);
	
	return p ? ((EventDriverEntry*) (p->entry))->eventdriver : 0;
}

void eventdrivers_ondriverinput(EventDrivers* self, EventDriver* sender)
{
	psy_signal_emit(&self->signal_input, sender, 0);
}

void eventdrivers_setcmds(EventDrivers* self, Properties* cmds)
{
	List* p;

	self->cmds = cmds;
	for (p = self->eventdrivers; p != 0; p = p->next) {
		EventDriverEntry* eventdriverentry;
		EventDriver* eventdriver;
		
		eventdriverentry = (EventDriverEntry*)p->entry;
		eventdriver = eventdriverentry->eventdriver;
		if (eventdriver) {
			eventdriver->setcmddef(eventdriver, cmds);
		}
	}
}

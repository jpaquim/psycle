// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "eventdrivers.h"
#include "kbddriver.h"
#include "inputmap.h"

#include <stdlib.h>
#include <string.h>

static void eventdrivers_ondriverinput(EventDrivers*, psy_EventDriver*);

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
	psy_EventDriver* eventdriver;
	EventDriverEntry* eventdriverentry;

	eventdriver = create_kbd_driver();	
	self->kbddriver = eventdriver;	
	eventdriverentry = (EventDriverEntry*) malloc(sizeof(EventDriverEntry));
	eventdriverentry->eventdriver = eventdriver;
	eventdriverentry->library = 0;
	psy_list_append(&self->eventdrivers, eventdriverentry);
	psy_signal_connect(&eventdriver->signal_input, self,
		eventdrivers_ondriverinput);
}

void eventdrivers_dispose(EventDrivers* self)
{
	psy_List* p;	

	for (p = self->eventdrivers; p != NULL; p = p->next) {
		EventDriverEntry* eventdriverentry;
		psy_EventDriver* eventdriver;
		
		eventdriverentry = (EventDriverEntry*)p->entry;
		eventdriver = eventdriverentry->eventdriver;
		eventdriver->close(eventdriver);
		eventdriver->dispose(eventdriver);
		eventdriver->free(eventdriver);
//		free(eventdriver);
		if (eventdriverentry && eventdriverentry->library) {
			psy_library_unload(eventdriverentry->library);
			psy_library_deallocate(eventdriverentry->library);			
		}
		free(eventdriverentry);
	}
	psy_list_free(self->eventdrivers);
	self->eventdrivers = 0;
	self->cmds = 0;
	psy_signal_dispose(&self->signal_input);
}

void eventdrivers_load(EventDrivers* self, const char* path)
{
	psy_EventDriver* eventdriver = 0;	
	
	if (path) {
		if (strcmp(path, "kbd") == 0) {
			if (!self->kbddriver) {
				eventdrivers_initkbd(self);
			}
		} else {
			psy_Library* library;
			psy_EventDriver* eventdriver = 0;

			library = psy_library_allocinit();			
			psy_library_load(library, path);
			if (!psy_library_empty(library)) {				
				pfneventdriver_create fpeventdrivercreate;

				fpeventdrivercreate = (pfneventdriver_create)
					psy_library_functionpointer(library, "eventdriver_create");
				if (fpeventdrivercreate) {
					EventDriverEntry* eventdriverentry;

					eventdriver = fpeventdrivercreate();
					eventdriver->setcmddef(eventdriver, self->cmds);					
					eventdriverentry = (EventDriverEntry*) malloc(sizeof(EventDriverEntry));
					eventdriverentry->eventdriver = eventdriver;
					eventdriverentry->library = library;
					psy_list_append(&self->eventdrivers, eventdriverentry);				
					eventdriver->open(eventdriver);
					psy_signal_connect(&eventdriver->signal_input, self,
						eventdrivers_ondriverinput);
				}
			}
			if (!eventdriver) {
				psy_library_deallocate(library);
			}
		}
	}
}

void eventdrivers_restart(EventDrivers* self, int id)
{	
	psy_EventDriver* eventdriver;

	eventdriver = eventdrivers_driver(self, id);
	if (eventdriver) {
		eventdriver->close(eventdriver);	
		eventdriver->configure(eventdriver);
		eventdriver->open(eventdriver);	
	}
}

void eventdrivers_restartall(EventDrivers* self)
{
	psy_List* p;	

	for (p = self->eventdrivers; p != NULL; p = p->next) {
		EventDriverEntry* eventdriverentry;
		psy_EventDriver* eventdriver;
		
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
	psy_EventDriver* eventdriver;

	eventdriver = eventdrivers_driver(self, id);	
	if (eventdriver) {
		EventDriverEntry* eventdriverentry;
		psy_List* p;

		eventdriver->close(eventdriver);
		eventdriver->dispose(eventdriver);
		eventdriver->free(eventdriver);
//		free(eventdriver);
		if (eventdriver == self->kbddriver) {
			self->kbddriver = 0;
		}
		eventdriverentry = eventdrivers_entry(self, id);
		if (eventdriverentry && eventdriverentry->library) {
			psy_library_unload(eventdriverentry->library);
			psy_library_deallocate(eventdriverentry->library);			
			eventdriverentry->library = 0;
		}
		for (p = self->eventdrivers; p != NULL; p = p->next) {
			if (((EventDriverEntry*)p->entry)->eventdriver == eventdriver) {
				psy_list_remove(&self->eventdrivers, p);
				break;
			}
		}
		free(eventdriverentry);
	}
}

unsigned int eventdrivers_size(EventDrivers* self)
{
	int rv = 0;
	psy_List* p;
	
	for (p = self->eventdrivers; p != NULL; p = p->next, ++rv);
	return rv;
}

EventDriverEntry* eventdrivers_entry(EventDrivers* self, int id)
{
	psy_List* p;
	int c = 0;

	for (p = self->eventdrivers; p != NULL && id != c; p = p->next, ++c);
	
	return p ? ((EventDriverEntry*) (p->entry)) : 0;
}

psy_EventDriver* eventdrivers_driver(EventDrivers* self, int id) 
{
	psy_List* p;
	int c = 0;

	for (p = self->eventdrivers; p != NULL && id != c; p = p->next, ++c);
	
	return p ? ((EventDriverEntry*) (p->entry))->eventdriver : 0;
}

void eventdrivers_ondriverinput(EventDrivers* self, psy_EventDriver* sender)
{
	psy_signal_emit(&self->signal_input, sender, 0);
}

void eventdrivers_setcmds(EventDrivers* self, psy_Properties* cmds)
{
	psy_List* p;

	self->cmds = cmds;
	for (p = self->eventdrivers; p != NULL; p = p->next) {
		EventDriverEntry* eventdriverentry;
		psy_EventDriver* eventdriver;
		
		eventdriverentry = (EventDriverEntry*)p->entry;
		eventdriver = eventdriverentry->eventdriver;
		if (eventdriver) {
			eventdriver->setcmddef(eventdriver, cmds);
		}
	}
}

// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "eventdrivers.h"
#include "kbddriver.h"

#include <stdlib.h>
#include <string.h>

static void eventdrivers_ondriverinput(psy_audio_EventDrivers*, psy_EventDriver*);

void psy_audio_eventdrivers_init(psy_audio_EventDrivers* self, void* systemhandle)
{
	self->eventdrivers = NULL;	
	self->kbddriver = NULL;
	self->cmds = NULL;
	self->systemhandle = systemhandle;	
	psy_signal_init(&self->signal_input);
	psy_audio_eventdrivers_initkbd(self);
}

void psy_audio_eventdrivers_initkbd(psy_audio_EventDrivers* self)
{
	psy_EventDriver* kbd;
	psy_audio_EventDriverEntry* eventdriverentry;

	kbd = create_kbd_driver();
	self->kbddriver = kbd;
	eventdriverentry = (psy_audio_EventDriverEntry*)malloc(sizeof(psy_audio_EventDriverEntry));
	eventdriverentry->eventdriver = kbd;
	eventdriverentry->library = 0;
	psy_list_append(&self->eventdrivers, eventdriverentry);
	psy_signal_connect(&kbd->signal_input, self,
		eventdrivers_ondriverinput);
}

void psy_audio_eventdrivers_dispose(psy_audio_EventDrivers* self)
{
	psy_List* p;	

	for (p = self->eventdrivers; p != NULL; psy_list_next(&p)) {
		psy_audio_EventDriverEntry* eventdriverentry;
		psy_EventDriver* eventdriver;
		
		eventdriverentry = (psy_audio_EventDriverEntry*)psy_list_entry(p);
		eventdriver = eventdriverentry->eventdriver;
		eventdriver->close(eventdriver);
		eventdriver->dispose(eventdriver);
#if defined _CRTDBG_MAP_ALLOC
		free(eventdriver);
#else
		eventdriver->free(eventdriver);
#endif
		if (eventdriverentry && eventdriverentry->library) {
			psy_library_unload(eventdriverentry->library);
			psy_library_deallocate(eventdriverentry->library);			
		}
		free(eventdriverentry);
	}
	psy_list_free(self->eventdrivers);
	self->eventdrivers = NULL;
	self->cmds = NULL;
	psy_signal_dispose(&self->signal_input);
}

psy_EventDriver* psy_audio_eventdrivers_load(psy_audio_EventDrivers* self, const char* path)
{
	psy_EventDriver* eventdriver = 0;	
	
	if (path) {
		if (strcmp(path, "kbd") == 0) {
			if (!self->kbddriver) {
				psy_audio_eventdrivers_initkbd(self);
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
					psy_audio_EventDriverEntry* eventdriverentry;

					eventdriver = fpeventdrivercreate();
					eventdriver->setcmddef(eventdriver, self->cmds);					
					eventdriverentry = (psy_audio_EventDriverEntry*) malloc(sizeof(psy_audio_EventDriverEntry));
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
	return eventdriver;
}

void psy_audio_eventdrivers_restart(psy_audio_EventDrivers* self, int id)
{	
	psy_EventDriver* eventdriver;

	eventdriver = psy_audio_eventdrivers_driver(self, id);
	if (eventdriver) {
		eventdriver->close(eventdriver);	
		eventdriver->configure(eventdriver, eventdriver->properties);
		eventdriver->open(eventdriver);	
	}
}

void psy_audio_eventdrivers_restartall(psy_audio_EventDrivers* self)
{
	psy_List* p;	

	for (p = self->eventdrivers; p != NULL; psy_list_next(&p)) {
		psy_audio_EventDriverEntry* eventdriverentry;
		psy_EventDriver* eventdriver;
		
		eventdriverentry = (psy_audio_EventDriverEntry*)psy_list_entry(p);
		eventdriver = eventdriverentry->eventdriver;
		if (eventdriver) {
			eventdriver->close(eventdriver);	
			eventdriver->configure(eventdriver, eventdriver->properties);
			eventdriver->open(eventdriver);	
		}
	}
}

void psy_audio_eventdrivers_remove(psy_audio_EventDrivers* self, int id)
{
	psy_EventDriver* eventdriver;

	eventdriver = psy_audio_eventdrivers_driver(self, id);	
	if (eventdriver) {
		psy_audio_EventDriverEntry* eventdriverentry;
		psy_List* p;

		eventdriver->close(eventdriver);
		eventdriver->dispose(eventdriver);
#if defined _CRTDBG_MAP_ALLOC
		free(eventdriver);
#else
		eventdriver->free(eventdriver);
#endif
//		free(eventdriver);
		if (eventdriver == self->kbddriver) {
			self->kbddriver = 0;
		}
		eventdriverentry = psy_audio_eventdrivers_entry(self, id);
		if (eventdriverentry && eventdriverentry->library) {
			psy_library_unload(eventdriverentry->library);
			psy_library_deallocate(eventdriverentry->library);			
			eventdriverentry->library = 0;
		}
		for (p = self->eventdrivers; p != NULL; psy_list_next(&p)) {
			if (((psy_audio_EventDriverEntry*)p->entry)->eventdriver == eventdriver) {
				psy_list_remove(&self->eventdrivers, p);
				break;
			}
		}
		free(eventdriverentry);
	}
}

unsigned int psy_audio_eventdrivers_size(psy_audio_EventDrivers* self)
{
	int rv = 0;
	psy_List* p;
	
	for (p = self->eventdrivers; p != NULL; psy_list_next(&p), ++rv);
	return rv;
}

psy_audio_EventDriverEntry* psy_audio_eventdrivers_entry(psy_audio_EventDrivers* self, int id)
{
	psy_List* p;
	int c = 0;

	for (p = self->eventdrivers; p != NULL && id != c; psy_list_next(&p), ++c);
	
	return p ? ((psy_audio_EventDriverEntry*) (p->entry)) : 0;
}

psy_EventDriver* psy_audio_eventdrivers_driver(psy_audio_EventDrivers* self, int id) 
{
	psy_List* p;
	int c = 0;

	for (p = self->eventdrivers; p != NULL && id != c; psy_list_next(&p), ++c);
	
	return p ? ((psy_audio_EventDriverEntry*) (p->entry))->eventdriver : 0;
}

void eventdrivers_ondriverinput(psy_audio_EventDrivers* self, psy_EventDriver* sender)
{
	psy_signal_emit(&self->signal_input, sender, 0);
}

void psy_audio_eventdrivers_setcmds(psy_audio_EventDrivers* self, psy_Properties* cmds)
{
	psy_List* p;

	self->cmds = cmds;
	for (p = self->eventdrivers; p != NULL; psy_list_next(&p)) {
		psy_audio_EventDriverEntry* eventdriverentry;
		psy_EventDriver* eventdriver;
		
		eventdriverentry = (psy_audio_EventDriverEntry*)psy_list_entry(p);
		eventdriver = eventdriverentry->eventdriver;
		if (eventdriver) {
			eventdriver->setcmddef(eventdriver, cmds);
		}
	}
}

void psy_audio_eventdrivers_idle(psy_audio_EventDrivers* self)
{
	psy_List* p;

	for (p = self->eventdrivers; p != NULL; psy_list_next(&p)) {
		psy_audio_EventDriverEntry* eventdriverentry;
		psy_EventDriver* eventdriver;

		eventdriverentry = (psy_audio_EventDriverEntry*)psy_list_entry(p);
		eventdriver = eventdriverentry->eventdriver;
		if (eventdriver) {
			eventdriver->idle(eventdriver);
		}
	}
}

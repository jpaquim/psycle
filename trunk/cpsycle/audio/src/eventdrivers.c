/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "eventdrivers.h"
/* local */
#include "cmddriver.h"
#include "kbddriver.h"
/* std */
#include <stdlib.h>
#include <string.h>


/* prototypes */
static void eventdrivers_ondriverinput(psy_audio_EventDrivers*, psy_EventDriver*);


/* implementation */
void psy_audio_eventdrivers_init(psy_audio_EventDrivers* self, void* systemhandle)
{
	self->eventdrivers = NULL;	
	self->kbddriver = NULL;
	self->cmddriver = psy_audio_cmddriver_create();
	self->cmds = NULL;
	self->systemhandle = systemhandle;
	psy_table_init(&self->guids);
	psy_signal_init(&self->signal_input);	
	psy_audio_eventdrivers_initkbd(self);
}

void psy_audio_eventdrivers_initkbd(psy_audio_EventDrivers* self)
{
	psy_audio_EventDriverEntry* eventdriverentry;

	self->kbddriver = psy_audio_kbddriver_create();	
	if (self->kbddriver) {		
		eventdriverentry = (psy_audio_EventDriverEntry*)malloc(sizeof(psy_audio_EventDriverEntry));
		if (eventdriverentry) {
			eventdriverentry->eventdriver = self->kbddriver;
			eventdriverentry->library = NULL;
			psy_list_append(&self->eventdrivers, eventdriverentry);
			psy_eventdriver_connect(self->kbddriver, self,
				(EVENTDRIVERWORKFN)eventdrivers_ondriverinput);
		}
	}
}

void psy_audio_eventdrivers_dispose(psy_audio_EventDrivers* self)
{
	psy_List* p;	

	for (p = self->eventdrivers; p != NULL; psy_list_next(&p)) {
		psy_audio_EventDriverEntry* eventdriverentry;
		psy_EventDriver* eventdriver;
		
		eventdriverentry = (psy_audio_EventDriverEntry*)psy_list_entry(p);
		eventdriver = eventdriverentry->eventdriver;
		psy_eventdriver_close(eventdriver);
		psy_eventdriver_dispose(eventdriver);
#if defined _CRTDBG_MAP_ALLOC
		free(eventdriver);
#else
		psy_eventdriver_deallocate(eventdriver);
#endif
		if (eventdriverentry && eventdriverentry->library) {
			psy_library_unload(eventdriverentry->library);
			psy_library_deallocate(eventdriverentry->library);			
		}
		free(eventdriverentry);
	}
	psy_list_free(self->eventdrivers);
	self->eventdrivers = NULL;
	psy_property_deallocate(self->cmds);
	self->cmds = NULL;
	psy_table_dispose_all(&self->guids, (psy_fp_disposefunc)0);
	psy_signal_dispose(&self->signal_input);
	psy_eventdriver_release(self->cmddriver);
	self->cmddriver = NULL;
}

psy_EventDriver* psy_audio_eventdrivers_load(psy_audio_EventDrivers* self, const char* path)
{
	psy_EventDriver* eventdriver = 0;	
	
	if (path) {
		if (strcmp(path, "kbd") == 0) {
			if (!self->kbddriver) {
				psy_audio_eventdrivers_initkbd(self);
				eventdriver = self->kbddriver;
			}
		} else {
			psy_Library* library;
			psy_EventDriver* eventdriver = 0;

			library = psy_library_allocinit();			
			psy_library_load(library, path);
			if (!psy_library_empty(library)) {				
				pfneventdriver_create fpeventdrivercreate;

				fpeventdrivercreate = (pfneventdriver_create)
					psy_library_functionpointer(library, "psy_eventdriver_create");
				if (fpeventdrivercreate) {
					psy_audio_EventDriverEntry* eventdriverentry;

					eventdriver = fpeventdrivercreate();
					psy_eventdriver_setcmddef(eventdriver, self->cmds);
					eventdriverentry = (psy_audio_EventDriverEntry*) malloc(sizeof(psy_audio_EventDriverEntry));
					if (eventdriverentry) {
						eventdriverentry->eventdriver = eventdriver;
						eventdriverentry->library = library;
						psy_list_append(&self->eventdrivers, eventdriverentry);
						psy_eventdriver_connect(eventdriver, self,
							(EVENTDRIVERWORKFN)eventdrivers_ondriverinput);
						psy_eventdriver_open(eventdriver);						
					}
				}
			}
			if (!eventdriver) {
				psy_library_deallocate(library);
			}
		}
	}
	return eventdriver;
}

intptr_t psy_audio_eventdrivers_guid(psy_audio_EventDrivers* self,
	const char* path)
{	
	if (strcmp("kbd", path) == 0) {
		return PSY_EVENTDRIVER_KBD_GUID;
	} else {
		psy_Library library;
		intptr_t rv;

		rv = -1;
		psy_library_init(&library);
		psy_library_load(&library, path);
		if (!psy_library_empty(&library)) {
			pfneventdriver_info fpeventdriverinfo;

			fpeventdriverinfo = (pfneventdriver_info)
				psy_library_functionpointer(&library,
					"psy_eventdriver_moduleinfo");
			if (fpeventdriverinfo) {
				psy_EventDriverInfo* eventdriverinfo;

				eventdriverinfo = fpeventdriverinfo();
				if (eventdriverinfo) {
					rv = eventdriverinfo->guid;
				}
			}
		}
		psy_library_dispose(&library);
		return rv;
	}
	return -1;
}

void psy_audio_eventdrivers_register(psy_audio_EventDrivers* self,
	intptr_t guid, const char* path)
{
	if (path) {
		psy_table_insert(&self->guids, (uintptr_t)guid, (void*)(uintptr_t)strdup(path));
	}
}

psy_EventDriver* psy_audio_eventdrivers_loadbyguid(psy_audio_EventDrivers* self, intptr_t guid)
{
	const char* path;

	path = (const char*)psy_table_at(&self->guids, guid);
	if (path) {
		return psy_audio_eventdrivers_load(self, path);
	}
	return NULL;
}

void psy_audio_eventdrivers_restart(psy_audio_EventDrivers* self, intptr_t id,
	psy_Property* configuration)
{	
	psy_EventDriver* eventdriver;

	eventdriver = psy_audio_eventdrivers_driver(self, id);
	if (eventdriver) {
		psy_eventdriver_close(eventdriver);
		psy_eventdriver_configure(eventdriver, configuration);		
		psy_eventdriver_open(eventdriver);
	}
}

void psy_audio_eventdrivers_restart_all(psy_audio_EventDrivers* self)
{
	psy_List* p;	

	for (p = self->eventdrivers; p != NULL; psy_list_next(&p)) {
		psy_audio_EventDriverEntry* eventdriverentry;
		psy_EventDriver* eventdriver;
		
		eventdriverentry = (psy_audio_EventDriverEntry*)psy_list_entry(p);
		eventdriver = eventdriverentry->eventdriver;
		if (eventdriver) {
			psy_eventdriver_close(eventdriver);
			psy_eventdriver_configure(eventdriver, NULL);			
			psy_eventdriver_open(eventdriver);
		}
	}
}

void psy_audio_eventdrivers_remove(psy_audio_EventDrivers* self, intptr_t id)
{
	psy_EventDriver* eventdriver;

	eventdriver = psy_audio_eventdrivers_driver(self, id);	
	if (eventdriver) {
		psy_audio_EventDriverEntry* eventdriverentry;
		psy_List* p;

		psy_eventdriver_close(eventdriver);
		psy_eventdriver_dispose(eventdriver);
#if defined _CRTDBG_MAP_ALLOC
		free(eventdriver);
#else
		psy_eventdriver_deallocate(eventdriver);
#endif
		if (eventdriver == self->kbddriver) {
			self->kbddriver = NULL;
		}
		eventdriverentry = psy_audio_eventdrivers_entry(self, id);
		if (eventdriverentry && eventdriverentry->library) {
			psy_library_unload(eventdriverentry->library);
			psy_library_deallocate(eventdriverentry->library);			
			eventdriverentry->library = NULL;
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

uintptr_t psy_audio_eventdrivers_size(psy_audio_EventDrivers* self)
{
	uintptr_t rv = 0;
	psy_List* p;
	
	for (p = self->eventdrivers; p != NULL; psy_list_next(&p), ++rv);
	return rv;
}

psy_audio_EventDriverEntry* psy_audio_eventdrivers_entry(psy_audio_EventDrivers* self, intptr_t id)
{
	psy_List* p;
	intptr_t c = 0;

	for (p = self->eventdrivers; p != NULL && id != c; psy_list_next(&p), ++c);
	
	return p ? ((psy_audio_EventDriverEntry*) (p->entry)) : 0;
}

psy_EventDriver* psy_audio_eventdrivers_driver(psy_audio_EventDrivers* self, intptr_t id)
{
	psy_List* p;
	intptr_t c = 0;

	for (p = self->eventdrivers; p != NULL && id != c; psy_list_next(&p), ++c);
	
	return p ? ((psy_audio_EventDriverEntry*) (p->entry))->eventdriver : 0;
}

void eventdrivers_ondriverinput(psy_audio_EventDrivers* self, psy_EventDriver* sender)
{
	psy_signal_emit(&self->signal_input, sender, 0);
}

void psy_audio_eventdrivers_setcmds(psy_audio_EventDrivers* self, psy_Property* cmds)
{
	psy_List* p;

	self->cmds = cmds;
	for (p = self->eventdrivers; p != NULL; psy_list_next(&p)) {
		psy_audio_EventDriverEntry* eventdriverentry;
		psy_EventDriver* eventdriver;
		
		eventdriverentry = (psy_audio_EventDriverEntry*)psy_list_entry(p);
		eventdriver = eventdriverentry->eventdriver;
		if (eventdriver) {
			psy_eventdriver_setcmddef(eventdriver, cmds);
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
			psy_eventdriver_idle(eventdriver);
		}
	}
}

void psy_audio_eventdrivers_sendcmd(psy_audio_EventDrivers* self,
	const char* section, psy_EventDriverCmd cmd)
{
	psy_audio_cmddriver_setcmd(self->cmddriver, section, cmd);
	psy_signal_emit(&self->signal_input, self->cmddriver, 0);
}

void psy_audio_eventdrivers_connect(psy_audio_EventDrivers* self, void* context,
	fp_eventdriver_input fp)
{
	psy_signal_connect(&self->signal_input, context, fp);
}


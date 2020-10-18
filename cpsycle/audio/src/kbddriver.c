// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"
#include "../../detail/os.h"

#include "patternevent.h"

#include "kbddriver.h"
#include "../../driver/eventdriver.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "cmdsnotes.h"

#if defined DIVERSALIS__OS__MICROSOFT    
#include <windows.h>
#endif

typedef struct {
	psy_EventDriver driver;
	int (*error)(int, const char*);
	EventDriverData lastinput;	
} KbdDriver;

static void driver_free(psy_EventDriver*);
static int driver_init(psy_EventDriver*);
static int driver_open(psy_EventDriver*);
static int driver_close(psy_EventDriver*);
static int driver_dispose(psy_EventDriver*);
static void driver_configure(psy_EventDriver* self) { }
static void driver_write(psy_EventDriver*, EventDriverData input);
static void driver_cmd(psy_EventDriver*, const char* section, EventDriverData,
	EventDriverCmd*);
static EventDriverCmd driver_getcmd(psy_EventDriver*, const char* section);
static void setcmddef(psy_EventDriver*, psy_Properties*);
static void driver_idle(psy_EventDriver* self) { }

int onerror(int err, const char* msg)
{
#if defined DIVERSALIS__OS__MICROSOFT    
	MessageBox(0, msg, "Kbd driver", MB_OK | MB_ICONERROR);
	return 0;
#else
    fprintf(stderr, "Kbd driver %s", msg);
	return 0;
#endif    
}

psy_EventDriver* create_kbd_driver(void)
{
	KbdDriver* kbd = (KbdDriver*)malloc(sizeof(KbdDriver));
	if (kbd) {
		memset(kbd, 0, sizeof(KbdDriver));
		kbd->driver.open = driver_open;
		kbd->driver.free = driver_free;
		kbd->driver.open = driver_open;
		kbd->driver.close = driver_close;
		kbd->driver.dispose = driver_dispose;
		kbd->driver.configure = driver_configure;
		kbd->driver.error = onerror;
		kbd->driver.write = driver_write;
		kbd->driver.cmd = driver_cmd;
		kbd->driver.getcmd = driver_getcmd;
		kbd->driver.setcmddef = setcmddef;
		kbd->driver.idle = driver_idle;
		driver_init(&kbd->driver);
		return &kbd->driver;
	}
	return NULL;
}

void driver_free(psy_EventDriver* driver)
{
	free(driver);
}

int driver_init(psy_EventDriver* driver)
{
	KbdDriver* self = (KbdDriver*) driver;
	
	self->driver.properties = NULL;
	psy_signal_init(&driver->signal_input);
	return 0;
}

int driver_dispose(psy_EventDriver* driver)
{
	KbdDriver* self = (KbdDriver*) driver;
	psy_properties_free(self->driver.properties);
	self->driver.properties = NULL;
	psy_signal_dispose(&driver->signal_input);
	return 1;
}

int driver_open(psy_EventDriver* driver)
{
	KbdDriver* self;

	self = (KbdDriver*)(driver);
	self->lastinput.message = 0;
	return 0;
}

int driver_close(psy_EventDriver* driver)
{
	return 0;
}

void driver_write(psy_EventDriver* driver, EventDriverData input)
{	
	KbdDriver* self;	

	self = (KbdDriver*)(driver);
	self->lastinput = input;	
	psy_signal_emit(&self->driver.signal_input, self, 0);
}

void driver_cmd(psy_EventDriver* driver, const char* sectionname,
	EventDriverData input, EventDriverCmd* cmd)
{		
	KbdDriver* self;
	EventDriverCmd kbcmd;
	psy_Properties* section;

	self = (KbdDriver*)(driver);
	cmd->id = -1;
	kbcmd.id = -1;
	if (!sectionname) {
		return;
	}
	section = psy_properties_findsection(driver->properties, sectionname);
	if (!section) {
		return;
	}
	if (input.message == EVENTDRIVER_KEYDOWN) {
		psy_Properties* p;

		for (p = section->children; p != NULL; p = psy_properties_next(p)) {
			if (psy_properties_as_int(p) == input.param1) {
				break;
			}
		}
		if (p) {
			kbcmd.id = p->item.id;
		}		
		if (kbcmd.id == CMD_NOTE_STOP) {
			cmd->id = kbcmd.id;
			cmd->data.param1 = NOTECOMMANDS_RELEASE;			
		} else
		if (kbcmd.id == CMD_NOTE_TWEAKM) {
			cmd->id = kbcmd.id;
			cmd->data.param1 = NOTECOMMANDS_TWEAK;			
		} else
		if (kbcmd.id == CMD_NOTE_TWEAKS) {
			cmd->id = kbcmd.id;
			cmd->data.param1 = NOTECOMMANDS_TWEAKSLIDE;			
		} else
		if (kbcmd.id == CMD_NOTE_MIDICC) {
			cmd->id = kbcmd.id;
			cmd->data.param1 = NOTECOMMANDS_MIDICC;
		} else
		if (kbcmd.id != -1) {
			cmd->id = kbcmd.id;
			cmd->data.param1 = kbcmd.id + input.param2;
		}
	} else {
		psy_Properties* p;

		for (p = section->children; p != NULL; p = psy_properties_next(p)) {
			if (psy_properties_as_int(p) == input.param1) {
				break;
			}
		}
		if (p) {
			kbcmd.id = p->item.id;
		}
		if (kbcmd.id <= NOTECOMMANDS_RELEASE) {
			cmd->id = NOTECOMMANDS_RELEASE;
			cmd->data.param1 = NOTECOMMANDS_RELEASE;
		}
	}
}

EventDriverCmd driver_getcmd(psy_EventDriver* driver, const char* section)
{	
	KbdDriver* self;
	EventDriverCmd cmd;	
		
	self = (KbdDriver*)(driver);
	driver_cmd(driver, section, self->lastinput, &cmd);	
	return cmd;
}

void setcmddef(psy_EventDriver* driver, psy_Properties* cmddef)
{
	psy_properties_free(driver->properties);
	driver->properties = psy_properties_clone(cmddef, 1);
	driver_configure(driver);
}

// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"
#include "../../detail/os.h"

#include "inputmap.h"
#include "patternevent.h"

#include "kbddriver.h"
#include "../../driver/eventdriver.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#if defined DIVERSALIS__OS__MICROSOFT    
#include <windows.h>
#endif

enum {
	CMD_NOTE_C_0,
	CMD_NOTE_CS0,
	CMD_NOTE_D_0,
	CMD_NOTE_DS0,
	CMD_NOTE_E_0,
	CMD_NOTE_F_0,
	CMD_NOTE_FS0,
	CMD_NOTE_G_0,
	CMD_NOTE_GS0,
	CMD_NOTE_A_0,
	CMD_NOTE_AS0,
	CMD_NOTE_B_0,
	CMD_NOTE_C_1, ///< 12
	CMD_NOTE_CS1,
	CMD_NOTE_D_1,
	CMD_NOTE_DS1,
	CMD_NOTE_E_1,
	CMD_NOTE_F_1,
	CMD_NOTE_FS1,
	CMD_NOTE_G_1,
	CMD_NOTE_GS1,
	CMD_NOTE_A_1,
	CMD_NOTE_AS1,
	CMD_NOTE_B_1,
	CMD_NOTE_C_2, ///< 24
	CMD_NOTE_CS2,
	CMD_NOTE_D_2,
	CMD_NOTE_DS2,
	CMD_NOTE_E_2,
	CMD_NOTE_F_2,
	CMD_NOTE_FS2,
	CMD_NOTE_G_2,
	CMD_NOTE_GS2,
	CMD_NOTE_A_2,
	CMD_NOTE_AS2,
	CMD_NOTE_B_2,
	CMD_NOTE_C_3, ///< 36
	CMD_NOTE_CS3,
	CMD_NOTE_D_3,
	CMD_NOTE_DS3,
	CMD_NOTE_E_3,
	CMD_NOTE_F_3,
	CMD_NOTE_FS3,
	CMD_NOTE_G_3,
	CMD_NOTE_GS3,
	CMD_NOTE_A_3,
	CMD_NOTE_AS3,
	CMD_NOTE_B_3,
	CMD_NOTE_STOP = 120,	///< NOTE STOP
	CMD_NOTE_TWEAKM = 121,	///< tweak
	//CMD_NOTE_TweakE = 122,	///< tweak effect. Old! No longer used.
	CMD_NOTE_MIDICC = 123,	///< Mcm Command (MIDI CC)
	CMD_NOTE_TWEAKS = 124,	///< tweak slide command
};

typedef struct {
	psy_EventDriver driver;
	psy_audio_Inputs noteinputs;
	int (*error)(int, const char*);
	EventDriverData lastinput;	
} KbdDriver;

static void driver_free(psy_EventDriver*);
static int driver_init(psy_EventDriver*);
static int driver_open(psy_EventDriver*);
static int driver_close(psy_EventDriver*);
static int driver_dispose(psy_EventDriver*);
static void driver_configure(psy_EventDriver*);
static void driver_write(psy_EventDriver*, EventDriverData input);
static void driver_cmd(psy_EventDriver* driver, EventDriverData,
	EventDriverCmd* cmd);
static EventDriverCmd driver_getcmd(psy_EventDriver*, psy_Properties* section);
static void setcmddef(psy_EventDriver*, psy_Properties*);

static void driver_makeinputs(KbdDriver*, psy_Properties*);

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
	KbdDriver* kbd = (KbdDriver*) malloc(sizeof(KbdDriver));
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
	driver_init(&kbd->driver);
	return &kbd->driver;			
}

void driver_free(psy_EventDriver* driver)
{
	free(driver);
}

int driver_init(psy_EventDriver* driver)
{
	KbdDriver* self = (KbdDriver*) driver;
	
	psy_audio_inputs_init(&self->noteinputs);
	self->driver.properties = 0;
//	init_properties(driver);
	psy_signal_init(&driver->signal_input);
	psy_audio_inputs_init(&self->noteinputs);
	return 0;
}

int driver_dispose(psy_EventDriver* driver)
{
	KbdDriver* self = (KbdDriver*) driver;
	psy_properties_free(self->driver.properties);
	self->driver.properties = 0;
	psy_audio_inputs_dispose(&self->noteinputs);
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

void driver_cmd(psy_EventDriver* driver, EventDriverData input, EventDriverCmd* cmd)
{		
	KbdDriver* self;
	EventDriverCmd kbcmd;

	self = (KbdDriver*)(driver);
	cmd->id = -1;	
	if (input.message == EVENTDRIVER_KEYDOWN) {
		kbcmd.id = psy_audio_inputs_cmd(&self->noteinputs, input.param1);		
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
		kbcmd.id = psy_audio_inputs_cmd(&self->noteinputs, input.param1);
		if (kbcmd.id <= NOTECOMMANDS_RELEASE) {
			cmd->id = NOTECOMMANDS_RELEASE;
			cmd->data.param1 = NOTECOMMANDS_RELEASE;
		}
	}
}

EventDriverCmd driver_getcmd(psy_EventDriver* driver, psy_Properties* section)
{	
	KbdDriver* self;
	EventDriverCmd cmd;	
		
	self = (KbdDriver*)(driver);
	driver_cmd(driver, self->lastinput, &cmd);	
	return cmd;
}

void driver_makeinputs(KbdDriver* self, psy_Properties* notes)
{
	psy_Properties* p;
	
	for (p = notes->children; p != NULL; p = psy_properties_next(p)) {				
		if (psy_properties_id(p) != -1) {
			psy_audio_inputs_define(&self->noteinputs, psy_properties_as_int(p),
				psy_properties_id(p), psy_properties_key(p));
		}
	}
}

void driver_configure(psy_EventDriver* driver)
{
	psy_Properties* notes;
	psy_Properties* general;
	psy_Properties* trackerview;

	notes = psy_properties_findsection(driver->properties, "notes");
	if (notes) {
		driver_makeinputs((KbdDriver*)driver, notes);
	}
	general = psy_properties_findsection(driver->properties, "generalcmds");
	if (general) {
		driver_makeinputs((KbdDriver*)driver, general);
	}
	trackerview = psy_properties_findsection(driver->properties, "trackercmds");
	if (trackerview) {
		driver_makeinputs((KbdDriver*)driver, trackerview);
	}
}

void setcmddef(psy_EventDriver* driver, psy_Properties* cmddef)
{
	driver->properties = psy_properties_clone(cmddef, 1);
	driver_configure(driver);
}

// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"
#include "event.h"

#include "kbddriver.h"
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include "inputmap.h"

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
	EventDriver driver;
	Inputs noteinputs;
	int (*error)(int, const char*);	
} KbdDriver;

static void driver_free(EventDriver*);
static int driver_init(EventDriver*);
static void driver_connect(EventDriver*, void* context, EVENTDRIVERWORKFN callback, void* handle);
static int driver_open(EventDriver*);
static int driver_close(EventDriver*);
static int driver_dispose(EventDriver*);
static void updateconfiguration(EventDriver*);
static void driver_write(EventDriver* driver, unsigned char* data, int size);

static void init_properties(EventDriver*);
static void apply_properties(KbdDriver*);
static void driver_makenotes(EventDriver*);
static void driver_makenoteinputs(KbdDriver*, Properties*);
static int cmdnote(const char* key);

int onerror(int err, const char* msg)
{
	MessageBox(0, msg, "Kbd driver", MB_OK | MB_ICONERROR);
	return 0;
}

EventDriver* create_kbd_driver(void)
{
	KbdDriver* kbd = (KbdDriver*) malloc(sizeof(KbdDriver));
	memset(kbd, 0, sizeof(KbdDriver));	
	kbd->driver.open = driver_open;
	kbd->driver.free = driver_free;	
	kbd->driver.connect = driver_connect;
	kbd->driver.open = driver_open;
	kbd->driver.close = driver_close;
	kbd->driver.dispose = driver_dispose;
	kbd->driver.updateconfiguration = updateconfiguration;	
	kbd->driver.error = onerror;	
	kbd->driver.write = driver_write;
	driver_init(&kbd->driver);
	return &kbd->driver;			
}

void driver_free(EventDriver* driver)
{
	free(driver);
}

int driver_init(EventDriver* driver)
{
	KbdDriver* self = (KbdDriver*) driver;
	
	inputs_init(&self->noteinputs);
	init_properties(driver);
	return 0;
}

int driver_dispose(EventDriver* driver)
{
	KbdDriver* self = (KbdDriver*) driver;
	properties_free(self->driver.properties);
	self->driver.properties = 0;
	inputs_dispose(&self->noteinputs);
	return 1;
}

void driver_connect(EventDriver* driver, void* context, EVENTDRIVERWORKFN callback, void* handle)
{
	driver->_callbackContext = context;
	driver->_pCallback = callback;	
}

int driver_open(EventDriver* driver)
{
	return 0;
}

int driver_close(EventDriver* driver)
{
	return 0;
}

void driver_write(EventDriver* driver, unsigned char* data, int size)
{	
	KbdDriver* self;
	int cmd;

	self = (KbdDriver*)(driver);
	
	cmd = inputs_cmd(&self->noteinputs, *((unsigned int*)data));
	if (cmd != -1) {
		int base;
		int note;
	
		base = 48;
		note = cmd + base;
		driver->_pCallback(driver->_callbackContext, 2,
				(unsigned char*)&note, 4);
	}
}

void init_properties(EventDriver* self)
{		
	self->properties = properties_create();
	properties_append_string(self->properties, "name", "Kbd Driver");
	properties_append_string(self->properties, "version", "1.0");	
	properties_append_int(self->properties, "bitdepth", 16, 0, 32);
	properties_append_int(self->properties, "samplerate", 44100, 0, 0);
	properties_append_int(self->properties, "dither", 0, 0, 1);	
	properties_append_int(self->properties, "numsamples", 256, 128, 8193);
	driver_makenotes(self);
}

void driver_makenotes(EventDriver* self)
{
	Properties* notes;
	Properties* p;

	notes = properties_createsection(self->properties, "notes");
	properties_settext(notes, "Notes");
	properties_append_int(notes, "cmd_note_c_0", encodeinput('Z', 0, 0), 0, 0);
	properties_append_int(notes, "cmd_note_cs0", encodeinput('S', 0, 0), 0, 0);
	properties_append_int(notes, "cmd_note_d_0", encodeinput('X', 0, 0), 0, 0);
	properties_append_int(notes, "cmd_note_ds0", encodeinput('D', 0, 0), 0, 0);
	properties_append_int(notes, "cmd_note_e_0", encodeinput('C', 0, 0), 0, 0);
	properties_append_int(notes, "cmd_note_f_0", encodeinput('V', 0, 0), 0, 0);
	properties_append_int(notes, "cmd_note_fs0", encodeinput('G', 0, 0), 0, 0);
	properties_append_int(notes, "cmd_note_g_0", encodeinput('B', 0, 0), 0, 0);
	properties_append_int(notes, "cmd_note_gs0", encodeinput('H', 0, 0), 0, 0);
	properties_append_int(notes, "cmd_note_a_0", encodeinput('N', 0, 0), 0, 0);
	properties_append_int(notes, "cmd_note_as0", encodeinput('J', 0, 0), 0, 0);
	properties_append_int(notes, "cmd_note_b_0", encodeinput('M', 0, 0), 0, 0);
	properties_append_int(notes, "cmd_note_c_1", encodeinput('Q', 0, 0), 0, 0);
	properties_append_int(notes, "cmd_note_cs1", encodeinput('2', 0, 0), 0, 0);
	properties_append_int(notes, "cmd_note_d_1", encodeinput('W', 0, 0), 0, 0);
	properties_append_int(notes, "cmd_note_ds1", encodeinput('3', 0, 0), 0, 0);
	properties_append_int(notes, "cmd_note_e_1", encodeinput('E', 0, 0), 0, 0);
	properties_append_int(notes, "cmd_note_f_1", encodeinput('R', 0, 0), 0, 0);
	properties_append_int(notes, "cmd_note_fs1", encodeinput('5', 0, 0), 0, 0);
	properties_append_int(notes, "cmd_note_g_1", encodeinput('T', 0, 0), 0, 0);
	properties_append_int(notes, "cmd_note_gs1", encodeinput('6', 0, 0), 0, 0);
	properties_append_int(notes, "cmd_note_a_1", encodeinput('Y', 0, 0), 0, 0);
	properties_append_int(notes, "cmd_note_as1", encodeinput('7', 0, 0), 0, 0);
	properties_append_int(notes, "cmd_note_b_1", encodeinput('U', 0, 0), 0, 0);		
	properties_append_int(notes, "cmd_note_c_2", encodeinput('I', 0, 0), 0, 0);
	properties_append_int(notes, "cmd_note_cs2", encodeinput('9', 0, 0), 0, 0);
	properties_append_int(notes, "cmd_note_d_2", encodeinput('O', 0, 0), 0, 0);
	properties_append_int(notes, "cmd_note_ds2", encodeinput('0', 0, 0), 0, 0);
	properties_append_int(notes, "cmd_note_e_2", encodeinput('P', 0, 0), 0, 0);
	properties_append_int(notes, "cmd_note_stop", encodeinput('1', 0, 0), 0, 0);
	// special
	properties_append_int(notes, "cmd_note_tweakm", encodeinput(192, 0, 0), 0, 0);
	properties_append_int(notes, "cmd_note_tweaks", encodeinput(192, 0, 1), 0, 0);
	for (p = notes->children; p != 0; p = properties_next(p)) {
		properties_sethint(p, PROPERTY_HINT_INPUT);
	}	
	driver_makenoteinputs((KbdDriver*)self, notes);
}

void driver_makenoteinputs(KbdDriver* self, Properties* notes)
{
	Properties* p;

	inputs_init(&self->noteinputs);
	for (p = notes->children; p != 0; p = properties_next(p)) {
		int cmd;
		
		cmd = cmdnote(properties_key(p));
		if (cmd != -1) {
			inputs_define(&self->noteinputs, properties_value(p), cmd);
		}
	}
}

int cmdnote(const char* key)
{	
	if (strcmp("cmd_note_c_0", key) == 0) { return CMD_NOTE_C_0; } else
	if (strcmp("cmd_note_cs0", key) == 0) { return CMD_NOTE_CS0; } else
	if (strcmp("cmd_note_d_0", key) == 0) { return CMD_NOTE_D_0; } else
	if (strcmp("cmd_note_ds0", key) == 0) { return CMD_NOTE_DS0; } else
	if (strcmp("cmd_note_e_0", key) == 0) { return CMD_NOTE_E_0; } else
	if (strcmp("cmd_note_f_0", key) == 0) { return CMD_NOTE_F_0; } else
	if (strcmp("cmd_note_fs0", key) == 0) { return CMD_NOTE_FS0; } else
	if (strcmp("cmd_note_g_0", key) == 0) { return CMD_NOTE_G_0; } else
	if (strcmp("cmd_note_gs0", key) == 0) { return CMD_NOTE_GS0; } else
	if (strcmp("cmd_note_a_0", key) == 0) { return CMD_NOTE_A_0; } else
	if (strcmp("cmd_note_as0", key) == 0) { return CMD_NOTE_AS0; } else
	if (strcmp("cmd_note_b_0", key) == 0) { return CMD_NOTE_B_0; } else
	if (strcmp("cmd_note_c_1", key) == 0) { return CMD_NOTE_C_1; } else
	if (strcmp("cmd_note_cs1", key) == 0) { return CMD_NOTE_CS1; } else
	if (strcmp("cmd_note_d_1", key) == 0) { return CMD_NOTE_D_1; } else
	if (strcmp("cmd_note_ds1", key) == 0) { return CMD_NOTE_DS1; } else
	if (strcmp("cmd_note_e_1", key) == 0) { return CMD_NOTE_E_1; } else
	if (strcmp("cmd_note_f_1", key) == 0) { return CMD_NOTE_F_1; } else
	if (strcmp("cmd_note_fs1", key) == 0) { return CMD_NOTE_FS1; } else
	if (strcmp("cmd_note_g_1", key) == 0) { return CMD_NOTE_G_1; } else
	if (strcmp("cmd_note_gs1", key) == 0) { return CMD_NOTE_GS1; } else
	if (strcmp("cmd_note_a_1", key) == 0) { return CMD_NOTE_A_1; } else
	if (strcmp("cmd_note_as1", key) == 0) { return CMD_NOTE_AS1; } else
	if (strcmp("cmd_note_b_1", key) == 0) { return CMD_NOTE_B_1; } else
	if (strcmp("cmd_note_c_2", key) == 0) { return CMD_NOTE_C_2; } else
	if (strcmp("cmd_note_cs2", key) == 0) { return CMD_NOTE_CS2; } else
	if (strcmp("cmd_note_d_2", key) == 0) { return CMD_NOTE_D_2; } else
	if (strcmp("cmd_note_ds2", key) == 0) { return CMD_NOTE_DS2; } else
	if (strcmp("cmd_note_e_2", key) == 0) { return CMD_NOTE_E_2; } else
	if (strcmp("cmd_note_f_2", key) == 0) { return CMD_NOTE_F_2; } else
	if (strcmp("cmd_note_fs2", key) == 0) { return CMD_NOTE_FS2; } else
	if (strcmp("cmd_note_g_2", key) == 0) { return CMD_NOTE_G_2; } else
	if (strcmp("cmd_note_gs2", key) == 0) { return CMD_NOTE_GS2; } else
	if (strcmp("cmd_note_a_2", key) == 0) { return CMD_NOTE_A_2; } else
	if (strcmp("cmd_note_as2", key) == 0) { return CMD_NOTE_AS2; } else
	if (strcmp("cmd_note_b_2", key) == 0) { return CMD_NOTE_B_2; } else
	if (strcmp("cmd_note_c_3", key) == 0) { return CMD_NOTE_C_3; } else
	if (strcmp("cmd_note_cs3", key) == 0) { return CMD_NOTE_CS3; } else
	if (strcmp("cmd_note_d_3", key) == 0) { return CMD_NOTE_D_3; } else
	if (strcmp("cmd_note_ds3", key) == 0) { return CMD_NOTE_DS3; } else
	if (strcmp("cmd_note_e_3", key) == 0) { return CMD_NOTE_E_3; } else
	if (strcmp("cmd_note_f_3", key) == 0) { return CMD_NOTE_F_3; } else
	if (strcmp("cmd_note_fs3", key) == 0) { return CMD_NOTE_FS3; } else
	if (strcmp("cmd_note_g_3", key) == 0) { return CMD_NOTE_G_3; } else
	if (strcmp("cmd_note_gs3", key) == 0) { return CMD_NOTE_GS3; } else
	if (strcmp("cmd_note_a_3", key) == 0) { return CMD_NOTE_A_3; } else
	if (strcmp("cmd_note_as3", key) == 0) { return CMD_NOTE_AS3; } else
	if (strcmp("cmd_note_b_3", key) == 0) { return CMD_NOTE_B_3; } else
	if (strcmp("cmd_note_stop", key) == 0) { return CMD_NOTE_STOP; } else
	if (strcmp("cmd_note_tweakm", key) == 0) { return  CMD_NOTE_TWEAKM; } else	
	if (strcmp("cmd_note_midicc", key) == 0) { return CMD_NOTE_MIDICC; } else
	if (strcmp("cmd_note_tweaks", key) == 0) { return  CMD_NOTE_TWEAKS; }	
	return -1;
}

void updateconfiguration(EventDriver* self)
{
	apply_properties((KbdDriver*)self);
}

void apply_properties(KbdDriver* self)
{	
	if (self->driver.properties) {
	
	}
}
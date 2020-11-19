// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "../eventdriver.h"

#include <windows.h>
#include <mmsystem.h>
#include <process.h>
#include <stdio.h>

#include "../../detail/portable.h"

#define DEVICE_NONE 0
#define PSY_EVENTDRIVER_MMEMIDI_GUID 0x0002

typedef struct {
	psy_EventDriver driver;	
	HMIDIIN hMidiIn;
	int deviceid;		
	int (*error)(int, const char*);
	psy_EventDriverData lastinput;	
	HANDLE hEvent;
	psy_Property* cmddef;
	psy_Property* configuration;
} psy_MmeMidiDriver;

static void driver_free(psy_EventDriver*);
static int driver_init(psy_EventDriver*);
static int driver_open(psy_EventDriver*);
static int driver_close(psy_EventDriver*);
static int driver_dispose(psy_EventDriver*);
static const psy_EventDriverInfo* driver_info(psy_EventDriver*);
static void driver_configure(psy_EventDriver*, psy_Property*);
static const psy_Property* driver_configuration(const psy_EventDriver*);
static void driver_cmd(psy_EventDriver*, const char* section, psy_EventDriverData input, psy_EventDriverCmd*);
static psy_EventDriverCmd driver_getcmd(psy_EventDriver*, const char* section);
static void setcmddef(psy_EventDriver*, psy_Property*);
static void driver_idle(psy_EventDriver* self) { }

static void init_properties(psy_EventDriver* self);
static int onerror(int err, const char* msg);

static CALLBACK MidiCallback(HMIDIIN handle, unsigned int uMsg,
	DWORD_PTR dwInstance, DWORD_PTR dwParam1, DWORD_PTR dwParam2);

static psy_EventDriverVTable vtable;
static int vtable_initialized = 0;

static void vtable_init(void)
{
	if (!vtable_initialized) {
		vtable.open = driver_open;
		vtable.deallocate = driver_free;
		vtable.open = driver_open;
		vtable.close = driver_close;
		vtable.dispose = driver_dispose;
		vtable.info = driver_info;
		vtable.configure = driver_configure;
		vtable.configuration = driver_configuration;
		vtable.error = onerror;
		vtable.cmd = driver_cmd;
		vtable.getcmd = driver_getcmd;
		vtable.setcmddef = setcmddef;
		vtable.idle = driver_idle;
		vtable_initialized = 1;
	}
}

int onerror(int err, const char* msg)
{
	MessageBox(0, msg, "Windows WaveOut MME driver", MB_OK | MB_ICONERROR);
	return 0;
}

EXPORT psy_EventDriverInfo const * __cdecl psy_eventdriver_moduleinfo(void)
{
	static psy_EventDriverInfo info;

	info.guid = PSY_EVENTDRIVER_MMEMIDI_GUID;
	info.Flags = 0;
	info.Name = "Windows MME Driver";
	info.ShortName = "MME";
	info.Version = 0;
	return &info;
}

EXPORT psy_EventDriver* __cdecl psy_eventdriver_create(void)
{
	psy_MmeMidiDriver* mme;
	
	mme = (psy_MmeMidiDriver*)malloc(sizeof(psy_MmeMidiDriver));
	if (mme) {			
		driver_init(&mme->driver);
		return &mme->driver;
	}
	return NULL;
}

void driver_free(psy_EventDriver* driver)
{
	free(driver);
}

int driver_init(psy_EventDriver* driver)
{
	psy_MmeMidiDriver* self = (psy_MmeMidiDriver*) driver;
	memset(self, 0, sizeof(psy_MmeMidiDriver));
	vtable_init();
	self->driver.vtable = &vtable;
	self->deviceid = DEVICE_NONE;
	self->hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	self->hMidiIn = 0;	
	init_properties(&self->driver);
	psy_signal_init(&driver->signal_input);
	return 0;
}

int driver_dispose(psy_EventDriver* driver)
{
	psy_MmeMidiDriver* self = (psy_MmeMidiDriver*) driver;
	psy_property_deallocate(self->configuration);
	self->configuration = NULL;
	CloseHandle(self->hEvent);
	psy_signal_dispose(&driver->signal_input);
	return 0;
}

const psy_EventDriverInfo* driver_info(psy_EventDriver* self)
{
	return psy_eventdriver_moduleinfo();
}

void init_properties(psy_EventDriver* context)
{		
	psy_MmeMidiDriver* self;

	self = (psy_MmeMidiDriver*)context;
	psy_Property* devices;
	char key[256];
	int i;
	int n;	

	psy_snprintf(key, 256, "mmemidi-guid-%d", PSY_EVENTDRIVER_MMEMIDI_GUID);
	self->configuration = psy_property_allocinit_key(key);
	psy_property_sethint(psy_property_append_int(self->configuration,
		"guid", PSY_EVENTDRIVER_MMEMIDI_GUID, 0, 0),
		PSY_PROPERTY_HINT_HIDE);
	psy_property_append_string(self->configuration, "name", "winmme midi");
	psy_property_append_string(self->configuration, "version", "1.0");
	devices = psy_property_append_choice(self->configuration, "device", 0);
	psy_property_append_int(devices, "0:None", 0, 0, 0);
	n = midiInGetNumDevs();	
	for (i = 0; i < n; ++i) {
		char text[256];
		
		MIDIINCAPS caps;
		midiInGetDevCaps(i, &caps, sizeof(MIDIINCAPS));
		psy_snprintf(text, 256, "%d:%s", i, caps.szPname);
		psy_property_append_int(devices, text, i, 0, 0);
	}
	self->cmddef = psy_property_append_section(self->configuration, "cmds");
}

void driver_configure(psy_EventDriver* driver, psy_Property* config)
{
	psy_MmeMidiDriver* self;
	self = (psy_MmeMidiDriver*)driver;
	psy_Property* p;

	if (self->configuration && config) {
		psy_property_sync(self->configuration, config);
	}
	p = psy_property_at(self->configuration, "device", PSY_PROPERTY_TYPE_NONE);
	if (p) {
		self->deviceid = psy_property_item_int(p);
	}	
}

int driver_open(psy_EventDriver* driver)
{
	psy_MmeMidiDriver* self = (psy_MmeMidiDriver*)driver;	
	unsigned int success = 1;

	self->lastinput.message = -1;
	if (self->deviceid != 0) {
		if (midiInOpen (&self->hMidiIn, self->deviceid - 1, (DWORD_PTR)MidiCallback,
				(DWORD_PTR)driver, CALLBACK_FUNCTION)) {
			psy_eventdriver_error(driver, 0, "Cannot open MIDI device");
			success = 0;
		} else {		
			if (midiInStart(self->hMidiIn)) {
				psy_eventdriver_error(driver, 0, "Cannot start MIDI device");
				success = 0;
			}
		}
	}	         
	return success;
}

int driver_close(psy_EventDriver* driver)
{
	psy_MmeMidiDriver* self = (psy_MmeMidiDriver*)driver;
	unsigned int success = 1;

	if (self->deviceid != 0 && self->hMidiIn && midiInClose(self->hMidiIn)) {
		psy_eventdriver_error(driver, 0, "Cannot close MIDI device");
		success = 0;
	}

	return success;
}

CALLBACK MidiCallback(HMIDIIN handle, unsigned int uMsg, DWORD_PTR dwInstance, DWORD_PTR dwParam1, DWORD_PTR dwParam2)
{			
	psy_EventDriver* driver = (psy_EventDriver*)dwInstance;	
	psy_MmeMidiDriver* self = (psy_MmeMidiDriver*) driver;	
	switch(uMsg) {
		// normal data message
		case MIM_DATA:
		{
			int cmd;
			int lsb;
			int msb;
			unsigned char* data;

			data = (unsigned char*) &dwParam1;
			lsb = data[0] & 0x0F;
			msb = (data[0] & 0xF0) >> 4;
			switch (msb) {
				case 0x9:
					// Note On/Off
					cmd = data[2] > 0 ? data[1] - 48 : 120;
					// channel = lsb;
					self->lastinput.param1 = cmd;
					self->lastinput.param2 = 48;
					psy_signal_emit(&self->driver.signal_input, self, 0);
				default:
				break;
			}
		}
		break;
		default:
		break;
	}
}

void driver_cmd(psy_EventDriver* driver, const char* section, psy_EventDriverData input, psy_EventDriverCmd* cmd)
{		
	cmd->id = input.param1;
	cmd->data.param1 = input.param1;
}

psy_EventDriverCmd driver_getcmd(psy_EventDriver* driver, const char* section)
{
	psy_EventDriverCmd cmd;
	psy_MmeMidiDriver* self = (psy_MmeMidiDriver*) driver;	
			
	driver_cmd(driver, section, self->lastinput, &cmd);	
	return cmd;
}

/*
void driver_configure(psy_EventDriver* driver)
{
	psy_Property* notes;

	notes = psy_property_findsection(driver->properties, "notes");
	if (notes) {
		driver_makenoteinputs((KbdDriver*)driver, notes);
	}
}
*/

void setcmddef(psy_EventDriver* driver, psy_Property* cmddef)
{
/*	psy_Property* notes;

	driver->properties = psy_property_clone(cmddef);
	notes = psy_property_findsection(driver->properties, "notes");
	if (notes) {
		driver_makenoteinputs((KbdDriver*)driver, notes);
	} */
}

const psy_Property* driver_configuration(const psy_EventDriver* driver)
{
	psy_MmeMidiDriver* self;

	self = (psy_MmeMidiDriver*)driver;
	return self->configuration;
}

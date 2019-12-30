// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "../eventdriver.h"

#include <windows.h>
#include <mmsystem.h>
#include <process.h>
#include <stdio.h>

#define DEVICE_NONE 0

typedef struct {
	EventDriver driver;	
	HMIDIIN hMidiIn;
	int deviceid;		
	int (*error)(int, const char*);
	EventDriverData lastinput;	
	HANDLE hEvent;
} MmeMidiDriver;

static void driver_free(EventDriver*);
static int driver_init(EventDriver*);
static int driver_open(EventDriver*);
static int driver_close(EventDriver*);
static int driver_dispose(EventDriver*);
static void driver_configure(EventDriver*);
static void driver_cmd(EventDriver*, EventDriverData input, EventDriverCmd*);
static EventDriverCmd driver_getcmd(EventDriver*, psy_Properties* section);
static void setcmddef(EventDriver*, psy_Properties*);

static void init_properties(EventDriver* self);
static void apply_properties(MmeMidiDriver* self);

static CALLBACK MidiCallback(HMIDIIN handle, unsigned int uMsg,
	DWORD_PTR dwInstance, DWORD_PTR dwParam1, DWORD_PTR dwParam2);

int onerror(int err, const char* msg)
{
	MessageBox(0, msg, "Windows WaveOut MME driver", MB_OK | MB_ICONERROR);
	return 0;
}

EXPORT EventDriverInfo const * __cdecl GetPsycleEventDriverInfo(void)
{
	static EventDriverInfo info;
	info.Flags = 0;
	info.Name = "Windows MME Driver";
	info.ShortName = "MME";
	info.Version = 0;
	return &info;
}

EXPORT EventDriver* __cdecl eventdriver_create(void)
{
	MmeMidiDriver* mme = (MmeMidiDriver*) malloc(sizeof(MmeMidiDriver));
	memset(mme, 0, sizeof(MmeMidiDriver));
	mme->deviceid = DEVICE_NONE;
	mme->driver.open = driver_open;
	mme->driver.free = driver_free;
	mme->driver.open = driver_open;
	mme->driver.close = driver_close;
	mme->driver.dispose = driver_dispose;
	mme->driver.configure = driver_configure;
	mme->driver.error = onerror;
	mme->driver.cmd = driver_cmd;
	mme->driver.getcmd = driver_getcmd;
	mme->driver.setcmddef = setcmddef;
	mme->hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	driver_init(&mme->driver);
	return &mme->driver;
}

void driver_free(EventDriver* driver)
{
	free(driver);
}

int driver_init(EventDriver* driver)
{
	MmeMidiDriver* self = (MmeMidiDriver*) driver;

	self->hMidiIn = 0;	
	init_properties(&self->driver);
	psy_signal_init(&driver->signal_input);
	return 0;
}

int driver_dispose(EventDriver* driver)
{
	MmeMidiDriver* self = (MmeMidiDriver*) driver;
	properties_free(self->driver.properties);
	self->driver.properties = 0;
	CloseHandle(self->hEvent);
	psy_signal_dispose(&driver->signal_input);
	return 0;
}

void init_properties(EventDriver* self)
{		
	psy_Properties* devices;
	int i;
	int n;	

	self->properties = psy_properties_create();
		
	psy_properties_append_string(self->properties, "name", "winmme midi");
	psy_properties_append_string(self->properties, "version", "1.0");
	devices = psy_properties_append_choice(self->properties, "device", 0);		 
	psy_properties_append_int(devices, "0:None", 0, 0, 0);
	n = midiInGetNumDevs();	
	for (i = 0; i < n; ++i) {
		char text[256];
		
		MIDIINCAPS caps;
		midiInGetDevCaps(i, &caps, sizeof(MIDIINCAPS));
#if defined _MSC_VER > 1200
		_snprintf_s(text, strlen(caps.szPname), 256, "%d:%s", i, caps.szPname);
#else
		_snprintf(text, 256, "%d:%s", i, caps.szPname);
#endif
		psy_properties_append_int(devices, text, i, 0, 0);
	}
}

void apply_properties(MmeMidiDriver* self)
{	
	if (self->driver.properties) {
		psy_Properties* p;

		p = psy_properties_read(self->driver.properties, "device");
		if (p) {
			self->deviceid = psy_properties_value(p);
		}
	}
}

void driver_configure(EventDriver* self)
{
	apply_properties((MmeMidiDriver*)self);
}

int driver_open(EventDriver* driver)
{
	MmeMidiDriver* self = (MmeMidiDriver*) driver;	
	unsigned int success = 1;

	self->lastinput.message = -1;
	if (self->deviceid != 0) {
		if (midiInOpen (&self->hMidiIn, self->deviceid - 1, (DWORD_PTR)MidiCallback,
				(DWORD_PTR)driver, CALLBACK_FUNCTION)) {
			driver->error(0, "Cannot open MIDI device");
			success = 0;
		} else {		
			if (midiInStart(self->hMidiIn)) {
				driver->error(0, "Cannot start MIDI device");
				success = 0;
			}
		}
	}
	         
	return success;
}

int driver_close(EventDriver* driver)
{
	MmeMidiDriver* self = (MmeMidiDriver*) driver;
	unsigned int success = 1;


	if (self->deviceid != 0 && self->hMidiIn && midiInClose(self->hMidiIn)) {
		driver->error(0, "Cannot close MIDI device");
		success = 0;
	}

	return success;
}

CALLBACK MidiCallback(HMIDIIN handle, unsigned int uMsg, DWORD_PTR dwInstance, DWORD_PTR dwParam1, DWORD_PTR dwParam2)
{			
	EventDriver* driver = (EventDriver*)dwInstance;	
	MmeMidiDriver* self = (MmeMidiDriver*) driver;	
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

void driver_cmd(EventDriver* driver, EventDriverData input, EventDriverCmd* cmd)
{		
	cmd->id = input.param1;
	cmd->data.param1 = input.param1;
}

EventDriverCmd driver_getcmd(EventDriver* driver, psy_Properties* section)
{
	EventDriverCmd cmd;
	MmeMidiDriver* self = (MmeMidiDriver*) driver;	
			
	driver_cmd(driver, self->lastinput, &cmd);	
	return cmd;
}

/*
void driver_configure(EventDriver* driver)
{
	psy_Properties* notes;

	notes = psy_properties_findsection(driver->properties, "notes");
	if (notes) {
		driver_makenoteinputs((KbdDriver*)driver, notes);
	}
}
*/

void setcmddef(EventDriver* driver, psy_Properties* cmddef)
{
/*	psy_Properties* notes;

	driver->properties = psy_properties_clone(cmddef);
	notes = psy_properties_findsection(driver->properties, "notes");
	if (notes) {
		driver_makenoteinputs((KbdDriver*)driver, notes);
	} */
}

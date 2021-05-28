/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"

#include "../eventdriver.h"

/* platform */
#include <windows.h>
#include <mmsystem.h>
#include <process.h>
#include "../../detail/portable.h"

#define DEVICE_NONE 0
#define PSY_EVENTDRIVER_MMEMIDI_GUID 0x0002

typedef struct {
	psy_EventDriver driver;	
	HMIDIIN hMidiIn;
	uint32_t deviceid;		
	int (*error)(int, const char*);
	psy_EventDriverCmd lastinput;
	HANDLE hEvent;	
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
static void driver_cmd(psy_EventDriver*, const char* section,
	psy_EventDriverInput, psy_EventDriverCmd*);
static psy_EventDriverCmd driver_getcmd(psy_EventDriver*, const char* section);
static void setcmddef(psy_EventDriver* self, const psy_Property* property) {  }
static void driver_idle(psy_EventDriver* self) { }

static psy_EventDriverInput driver_input(psy_EventDriver* context)
{
	psy_EventDriverInput input;

	psy_MmeMidiDriver* self = (psy_MmeMidiDriver*)context;
	if (self->lastinput.type == psy_EVENTDRIVER_MIDI) {
		input.message = self->lastinput.midi.byte0;
		input.param1 = self->lastinput.midi.byte1;
		input.param2 = self->lastinput.midi.byte2;
	} else {
		input.message = 0;
		input.param1 = 0;
		input.param2 = 0;
	}
	return input;
}

static void driver_makeconfig(psy_EventDriver*);
static int onerror(int err, const char* msg);

static CALLBACK MidiCallback(HMIDIIN handle, unsigned int uMsg,
	DWORD_PTR dwInstance, DWORD_PTR dwParam1, DWORD_PTR dwParam2);

static psy_EventDriverVTable vtable;
static bool vtable_initialized = FALSE;

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
		vtable.input = driver_input;
		vtable_initialized = TRUE;
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
	psy_MmeMidiDriver* self;
	
	assert(driver);

	self = (psy_MmeMidiDriver*)driver;
	memset(self, 0, sizeof(psy_MmeMidiDriver));
	vtable_init();
	self->driver.vtable = &vtable;
	self->deviceid = DEVICE_NONE;
	self->hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	self->hMidiIn = 0;	
	driver_makeconfig(&self->driver);
	self->driver.callback = NULL;
	self->driver.callbackcontext = NULL;
	self->lastinput.type = psy_EVENTDRIVER_CMD;
	self->lastinput.id = -1;
	return 0;
}

int driver_dispose(psy_EventDriver* driver)
{
	psy_MmeMidiDriver* self;

	assert(driver);
	
	self = (psy_MmeMidiDriver*)driver;
	if (self->hMidiIn) {
		driver_close(driver);
	}
	psy_property_deallocate(self->configuration);
	self->configuration = NULL;	
	CloseHandle(self->hEvent);	
	return 0;
}

const psy_EventDriverInfo* driver_info(psy_EventDriver* self)
{
	return psy_eventdriver_moduleinfo();
}

void driver_makeconfig(psy_EventDriver* context)
{		
	psy_MmeMidiDriver* self;
	psy_Property* devices;
	char key[256];
	uint32_t i;
	uint32_t n;

	self = (psy_MmeMidiDriver*)context;
	psy_snprintf(key, 256, "mmemidi-guid-%d", PSY_EVENTDRIVER_MMEMIDI_GUID);
	self->configuration = psy_property_preventtranslate(
		psy_property_allocinit_key(key));
	psy_property_sethint(psy_property_append_int(self->configuration,
		"guid", PSY_EVENTDRIVER_MMEMIDI_GUID, 0, 0),
		PSY_PROPERTY_HINT_HIDE);
	psy_property_append_str(self->configuration, "name", "winmme midi");
	psy_property_append_str(self->configuration, "version", "1.0");
	devices = psy_property_append_choice(self->configuration, "device", 0);
	psy_property_sethint(devices, PSY_PROPERTY_HINT_COMBO);
	n = midiInGetNumDevs();	
	for (i = 0; i < n; ++i) {
		char text[256];
		
		MIDIINCAPS caps;
		midiInGetDevCaps(i, &caps, sizeof(MIDIINCAPS));
		psy_snprintf(text, 256, "%d:%s", i, caps.szPname);
		psy_property_append_int(devices, text, i, 0, 0);
	}
}

void driver_configure(psy_EventDriver* driver, psy_Property* config)
{
	psy_MmeMidiDriver* self;
	psy_Property* p;

	self = (psy_MmeMidiDriver*)driver;
	if (self->configuration && config) {
		psy_property_sync(self->configuration, config);
	}
	p = psy_property_at(self->configuration, "device",
		PSY_PROPERTY_TYPE_NONE);
	if (p) {
		self->deviceid = (uint32_t)psy_property_item_int(p);
	}
}

int driver_open(psy_EventDriver* driver)
{
	psy_MmeMidiDriver* self = (psy_MmeMidiDriver*)driver;	
	int success = 1;

	self->lastinput.type = psy_EVENTDRIVER_CMD;
	self->lastinput.id = -1;
	if (self->deviceid != 0) {
		if (midiInOpen (&self->hMidiIn, self->deviceid, (DWORD_PTR)MidiCallback,
				(DWORD_PTR)driver, CALLBACK_FUNCTION)) {
			psy_eventdriver_error(driver, 0, "Cannot open MIDI device");
			success = 0;
		} else if (midiInStart(self->hMidiIn)) {
			psy_eventdriver_error(driver, 0, "Cannot start MIDI device");
			success = 0;
		}
	}	         
	return success;
}

int driver_close(psy_EventDriver* driver)
{
	psy_MmeMidiDriver* self = (psy_MmeMidiDriver*)driver;
	int success = 1;

	if (self->deviceid != 0 && self->hMidiIn && midiInClose(self->hMidiIn)) {
		psy_eventdriver_error(driver, 0, "Cannot close MIDI device");
		success = 0;
	}
	self->hMidiIn = NULL;
	return success;
}

void driver_cmd(psy_EventDriver* driver, const char* section,
	psy_EventDriverInput input, psy_EventDriverCmd* cmd)
{	
	psy_MmeMidiDriver* self;

	if (!cmd) {
		return;
	}
	self = (psy_MmeMidiDriver*)driver;
	if (psy_strlen(section) == 0) {
		cmd->type = psy_EVENTDRIVER_MIDI;
		cmd->midi.byte0 = (uint8_t)input.message;
		cmd->midi.byte1 = (uint8_t)input.param1;
		cmd->midi.byte2 = (uint8_t)input.param1;
		*cmd = self->lastinput;
	} else {
		cmd->id = -1;
		cmd->type = psy_EVENTDRIVER_CMD;
	}
}

psy_EventDriverCmd driver_getcmd(psy_EventDriver* context, const char* section)
{	
	psy_MmeMidiDriver* self;
	psy_EventDriverCmd cmd;

	self = (psy_MmeMidiDriver*)(context);
	if (psy_strlen(section) == 0) {
		return self->lastinput;
	}
	cmd.id = -1;
	cmd.type = psy_EVENTDRIVER_CMD;	
	return cmd;
}

const psy_Property* driver_configuration(const psy_EventDriver* driver)
{
	psy_MmeMidiDriver* self;

	self = (psy_MmeMidiDriver*)driver;
	return self->configuration;
}

/* mme midi callback */
CALLBACK MidiCallback(HMIDIIN handle, unsigned int msg, DWORD_PTR instance,
	DWORD_PTR param1, DWORD_PTR param2)
{
	psy_MmeMidiDriver* self;

	self = (psy_MmeMidiDriver*)instance;

	/* reset lastinput */
	self->lastinput.type = psy_EVENTDRIVER_CMD;
	self->lastinput.id = -1;
	/* set lastinput */
	switch (msg) {
	/* normal data message */
	case MIM_DATA:
		self->lastinput.type = psy_EVENTDRIVER_MIDI;
		self->lastinput.midi.byte0 = LOBYTE(LOWORD(param1));
		self->lastinput.midi.byte1 = HIBYTE(LOWORD(param1));
		self->lastinput.midi.byte2 = LOBYTE(HIWORD(param1));
		if (self->driver.callback) {
			self->driver.callback(self->driver.callbackcontext, &self->driver);
		}		
		break;
	default:
		break;
	}
}

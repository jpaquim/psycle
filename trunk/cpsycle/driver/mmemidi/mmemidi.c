// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "../eventdriver.h"

#include <windows.h>
#include <mmsystem.h>
#include <process.h>
#include <stdio.h>

#include "../../audio/src/cmdsnotes.h"
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
static void driver_cmd(psy_EventDriver*, const char* section, psy_EventDriverInput input, psy_EventDriverCmd*);
static psy_EventDriverCmd driver_getcmd(psy_EventDriver*, const char* section);
static const char* driver_target(psy_EventDriver*);
static void setcmddef(psy_EventDriver*, const psy_Property*);
static void driver_idle(psy_EventDriver* self) { }

static psy_EventDriverInput driver_input(psy_EventDriver* context)
{
	psy_EventDriverInput input;

	psy_MmeMidiDriver* self = (psy_MmeMidiDriver*)context;
	input.message = self->lastinput.midi.byte0;
	input.param1 = self->lastinput.midi.byte1;
	input.param2 = self->lastinput.midi.byte2;
	return input;
}

static void init_properties(psy_EventDriver* self);
static int clearcmddefenum(psy_MmeMidiDriver*, psy_Property*, int level);
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
		vtable.target = driver_target;
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
	init_properties(&self->driver);
	psy_signal_init(&driver->signal_input);
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
	self->cmddef = psy_property_append_section(self->configuration, "cmds");
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

CALLBACK MidiCallback(HMIDIIN handle, unsigned int msg, DWORD_PTR instance,
	DWORD_PTR param1, DWORD_PTR param2)
{			
	psy_MmeMidiDriver* self;
	
	self = (psy_MmeMidiDriver*)instance;	

	switch(msg) {
		// normal data message
		case MIM_DATA:		
			self->lastinput.id = CMD_NOTE_MIDIEV;
			self->lastinput.midi.byte0 = LOBYTE(LOWORD(param1));
			self->lastinput.midi.byte1 = HIBYTE(LOWORD(param1));
			self->lastinput.midi.byte2 = LOBYTE(HIWORD(param1));
			psy_signal_emit(&self->driver.signal_input, self, 0);					
			break;
		default:
			break;
	}
}

void driver_cmd(psy_EventDriver* driver, const char* section,
	psy_EventDriverInput input, psy_EventDriverCmd* cmd)
{		
	cmd->id = -1;
}

psy_EventDriverCmd driver_getcmd(psy_EventDriver* driver, const char* sectionname)
{	
	psy_MmeMidiDriver* self;
	psy_Property* section;
	psy_EventDriverCmd cmd;
	psy_Property* property = NULL;
	psy_List* p;
	
	self = (psy_MmeMidiDriver*)driver;

	cmd.id = -1;
	if (!sectionname) {
		return cmd;
	}	
	section = psy_property_findsection(self->configuration, sectionname);
	if (!section) {
		self->lastinput;
	}
	for (p = psy_property_begin(section); p != NULL;
			psy_list_next(&p)) {
		unsigned char byte0;
		unsigned char byte1;
		unsigned char byte2;

		property = (psy_Property*)psy_list_entry(p);
		if (psy_property_item_int(property) != 0) {
			byte0 = LOBYTE(LOWORD(psy_property_item_int(property)));
			byte1 = HIBYTE(LOWORD(psy_property_item_int(property)));
			byte2 = LOBYTE(HIWORD(psy_property_item_int(property)));
			if (byte0 == self->lastinput.midi.byte0 &&
				byte1 == self->lastinput.midi.byte1) {
				// byte2 == self->lastinput.midi.byte2) {
				cmd.id = property->item.id;
				cmd.midi.byte0 = byte0;
				cmd.midi.byte1 = byte1;
				cmd.midi.byte2 = byte2;
				return cmd;
				break;
			}
		}
		property = NULL;
	}
	return self->lastinput;
}

const char* driver_target(psy_EventDriver* self)
{
	return NULL;
}

void setcmddef(psy_EventDriver* driver, const psy_Property* cmddef)
{
	if (cmddef && cmddef->children) {
		psy_MmeMidiDriver* self = (psy_MmeMidiDriver*)driver;

		if (self->cmddef) {
			psy_property_remove(self->configuration, self->cmddef);
		}
		self->cmddef = psy_property_clone(cmddef);
		psy_property_append_property(self->configuration,
			self->cmddef);
		psy_property_settext(self->cmddef, "cmds.keymap");
		if (!psy_property_empty(self->cmddef)) {
			psy_property_enumerate(self->cmddef, self,
				(psy_PropertyCallback)clearcmddefenum);
		}
	}
}

int clearcmddefenum(psy_MmeMidiDriver* self, psy_Property* property, int level)
{
	psy_property_sethint(property, PSY_PROPERTY_HINT_EDIT);
	if (psy_property_type(property) == PSY_PROPERTY_TYPE_INTEGER) {
		psy_property_setitem_int(property, 0);
	}
	return 1;
}


const psy_Property* driver_configuration(const psy_EventDriver* driver)
{
	psy_MmeMidiDriver* self;

	self = (psy_MmeMidiDriver*)driver;
	return self->configuration;
}

// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "../eventdriver.h"

#include <windows.h>
#include <mmsystem.h>
#include <process.h>
#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>
#pragma comment (lib, "dinput8.lib")
#pragma comment (lib, "dxguid.lib")
#include <stdlib.h>
#include <stdio.h>
#include <hashtbl.h>

#include "../../detail/portable.h"

#define DEVICE_NONE 0
#define PSY_EVENTDRIVER_DXJOYSTICK_GUID 0x0003

#define INPUT_BUTTON_FIRST 0
#define INPUT_BUTTON_LAST 127
#define INPUT_MOVE_UP 128
#define INPUT_MOVE_DOWN 129
#define INPUT_MOVE_LEFT 130
#define INPUT_MOVE_RIGHT 131

typedef struct {
	psy_EventDriver driver;
	LPDIRECTINPUT8 di;
	LPDIRECTINPUTDEVICE8 joystick;
	int count;
	psy_Table inputs;	
	uint32_t deviceid;
	bool active;
	int (*error)(int, const char*);
	psy_EventDriverInput lastinput;	
	HANDLE hEvent;
	psy_Property* devices;
	psy_Property* cmddef;
	DIJOYSTATE2 state;
	psy_Property* configuration;
} DXJoystickDriver;

static void driver_free(psy_EventDriver*);
static int driver_init(psy_EventDriver*);
static int driver_open(psy_EventDriver*);
static int driver_close(psy_EventDriver*);
static int driver_dispose(psy_EventDriver*);
static const psy_EventDriverInfo* driver_info(psy_EventDriver*);
static void driver_configure(psy_EventDriver*, psy_Property*);
static const psy_Property* driver_configuration(const psy_EventDriver*);
static void driver_cmd(psy_EventDriver*, const char* section,
	psy_EventDriverInput input, psy_EventDriverCmd*);
static psy_EventDriverCmd driver_getcmd(psy_EventDriver*, const char* section);
static const char* driver_target(psy_EventDriver*);
static void driver_setcmddef(psy_EventDriver*, const psy_Property*);
static void driver_setcmddefaults(DXJoystickDriver*, psy_Property*);
static void driver_idle(psy_EventDriver* self);

static psy_EventDriverInput driver_input(psy_EventDriver* context)
{
	// psy_EventDriverInput input;

	DXJoystickDriver* self = (DXJoystickDriver*)context;
	return self->lastinput;
}

static void init_properties(psy_EventDriver* self);

static CALLBACK MidiCallback(HMIDIIN handle, unsigned int uMsg,
	DWORD_PTR dwInstance, DWORD_PTR dwParam1, DWORD_PTR dwParam2);

static HRESULT poll(DXJoystickDriver*, DIJOYSTATE2* js);

static BOOL CALLBACK
enumCallback(const DIDEVICEINSTANCE* instance, VOID* context);

static BOOL CALLBACK
staticSetGameControllerAxesRanges(LPCDIDEVICEOBJECTINSTANCE devObjInst, LPVOID pvRef);

static int driver_setformat(DXJoystickDriver*);

static psy_EventDriverVTable vtable;
static int vtable_initialized = 0;

static int driver_onerror(int err, const char* msg);

static void vtable_init(void)
{
	if (!vtable_initialized) {
		vtable.open = driver_open;
		vtable.deallocate = driver_free;
		vtable.close = driver_close;
		vtable.dispose = driver_dispose;
		vtable.configure = driver_configure;
		vtable.configuration = driver_configuration;
		vtable.info = driver_info;
		vtable.error = driver_onerror;
		vtable.cmd = driver_cmd;
		vtable.getcmd = driver_getcmd;
		vtable.target = driver_target;
		vtable.setcmddef = driver_setcmddef;
		vtable.idle = driver_idle;
		vtable.input = driver_input;
		vtable_initialized = 1;
	}
}

int driver_onerror(int err, const char* msg)
{
	MessageBox(0, msg, "Windows WaveOut MME driver", MB_OK | MB_ICONERROR);
	return 0;
}

EXPORT psy_EventDriverInfo const * __cdecl psy_eventdriver_moduleinfo(void)
{
	static psy_EventDriverInfo info;

	info.guid = PSY_EVENTDRIVER_DXJOYSTICK_GUID;
	info.Flags = 0;
	info.Name = "DirectX Joystick Driver";
	info.ShortName = "DX Joystick";
	info.Version = 0;
	return &info;
}

EXPORT psy_EventDriver* __cdecl psy_eventdriver_create(void)
{
	DXJoystickDriver* dxjoystick = (DXJoystickDriver*)malloc(sizeof(DXJoystickDriver));
	if (dxjoystick) {		
		driver_init(&dxjoystick->driver);
		return &dxjoystick->driver;
	}
	return NULL;
}

void driver_free(psy_EventDriver* driver)
{
	free(driver);
}

const psy_EventDriverInfo* driver_info(psy_EventDriver* self)
{
	return psy_eventdriver_moduleinfo();
}

int driver_init(psy_EventDriver* driver)
{
	DXJoystickDriver* self = (DXJoystickDriver*)driver;
	HRESULT hr;

	memset(self, 0, sizeof(DXJoystickDriver));
	vtable_init();
	self->driver.vtable = &vtable;	
	self->active = 0;
	psy_table_init(&self->inputs);
	// Create a DirectInput device
	if (FAILED(hr = DirectInput8Create(GetModuleHandle(NULL), DIRECTINPUT_VERSION,
		&IID_IDirectInput8, (VOID**)&self->di, NULL))) {
		return 1;
	}
	init_properties(&self->driver);
	psy_signal_init(&driver->signal_input);
	return 0;
}

int driver_dispose(psy_EventDriver* driver)
{
	DXJoystickDriver* self = (DXJoystickDriver*) driver;	
	psy_property_deallocate(self->configuration);
	self->configuration = 0;
	CloseHandle(self->hEvent);
	psy_signal_dispose(&driver->signal_input);
	psy_table_disposeall(&self->inputs, (psy_fp_disposefunc)NULL);
	return 0;
}

void init_properties(psy_EventDriver* context)
{		
	DXJoystickDriver* self;
	char key[256];
	HRESULT hr;

	self = (DXJoystickDriver*)context;
	psy_snprintf(key, 256, "dxjoystick-guid-%d", PSY_EVENTDRIVER_DXJOYSTICK_GUID);
	self->configuration = psy_property_preventtranslate(psy_property_allocinit_key(key));
	psy_property_sethint(psy_property_append_int(self->configuration,
		"guid", PSY_EVENTDRIVER_DXJOYSTICK_GUID, 0, 0),
		PSY_PROPERTY_HINT_HIDE);
	psy_property_append_str(self->configuration, "name", "directx joystick");
	psy_property_append_str(self->configuration, "version", "1.0");
	self->devices = psy_property_append_choice(self->configuration, "device", 0);
	psy_property_append_int(self->devices, "0:None", 0, 0, 0);
	self->count = 0;
	if (FAILED(hr = self->di->lpVtbl->EnumDevices(self->di,
			DI8DEVCLASS_GAMECTRL, enumCallback,
			self, DIEDFL_ATTACHEDONLY))) {
	}
	self->cmddef = psy_property_append_section(self->configuration, "cmds");
}

BOOL CALLBACK
enumCallback(const DIDEVICEINSTANCE* instance, VOID* context)
{
	DXJoystickDriver* self;
	DIDEVICEINSTANCE* input;
	char text[256];

	self = (DXJoystickDriver*)context;	
	psy_snprintf(text, 256, "%d:%s", self->count, instance->tszProductName);
	input = (DIDEVICEINSTANCE*)malloc(sizeof(DIDEVICEINSTANCE));
	*input = *instance;
	psy_table_insert(&self->inputs, self->count, input);
	psy_property_append_int(self->devices, text, self->count, 0, 0);
	++self->count;
	return DIENUM_CONTINUE;
}

void driver_configure(psy_EventDriver* context, psy_Property* configuration)
{
	DXJoystickDriver* self;

	self = (DXJoystickDriver*)context;
	if (configuration) {
		psy_property_sync(self->configuration, configuration);
	}
	if (self->configuration) {
		psy_Property* p;

		p = psy_property_at(self->configuration, "device", PSY_PROPERTY_TYPE_NONE);
		if (p) {
			self->deviceid = (uint32_t)psy_property_item_int(p);
		}
	}
}

int driver_open(psy_EventDriver* driver)
{
	HRESULT hr;
	DIDEVICEINSTANCE* instance;

	DXJoystickDriver* self = (DXJoystickDriver*)driver;
	unsigned int success = 1;

	self->lastinput.message = -1;
	// Obtain an interface to the enumerated joystick.

	instance = (DIDEVICEINSTANCE*)psy_table_at(&self->inputs, 0);
	if (!instance) {
		return 0;
	}
	hr = self->di->lpVtbl->CreateDevice(self->di, &instance->guidInstance,
		&self->joystick, NULL);

	// If it failed, then we can't use this joystick. (Maybe the user unplugged
	// it while we were in the middle of enumerating it.)
	if (FAILED(hr)) {
		psy_eventdriver_error(&self->driver, 0, "Cannot open DirectX Input device");
		return 0;
	}
	driver_setformat(self);
	// set range and dead zone of joystick axes
	hr = self->joystick->lpVtbl->EnumObjects(self->joystick,
		&staticSetGameControllerAxesRanges, self->joystick, DIDFT_AXIS);
	if (FAILED(hr)) {
		psy_eventdriver_error(&self->driver, 0, "Unable to set axis ranges of game controllers");
		return 0;
	}
	self->active = TRUE;
	memset(&self->state, 0, sizeof(DIJOYSTATE2));
	return success;

}

int driver_setformat(DXJoystickDriver* self)
{
	DIDEVCAPS capabilities;
	HRESULT hr;

	// Set the data format to "simple joystick" - a predefined data format 
	//
	// A data format specifies which controls on a device we are interested in,
	// and how they should be reported. This tells DInput that we will be
	// passing a DIJOYSTATE2 structure to IDirectInputDevice::GetDeviceState().
	if (FAILED(hr = self->joystick->lpVtbl->SetDataFormat(self->joystick,
		&c_dfDIJoystick2))) {
		return hr;
	}

	// Set the cooperative level to let DInput know how this device should
	// interact with the system and with other DInput applications.
	if (FAILED(hr = self->joystick->lpVtbl->SetCooperativeLevel(self->joystick,
		NULL, DISCL_EXCLUSIVE |
		DISCL_FOREGROUND))) {
		return hr;
	}

	// Determine how many axis the joystick has (so we don't error out setting
	// properties for unavailable axis)
	capabilities.dwSize = sizeof(DIDEVCAPS);
	if (FAILED(hr = self->joystick->lpVtbl->GetCapabilities(self->joystick, &capabilities))) {
		return hr;
	}
	return 0;
}

int driver_close(psy_EventDriver* driver)
{
	DXJoystickDriver* self = (DXJoystickDriver*) driver;
	unsigned int success = 1;
	
	if (self->joystick) {
		self->joystick->lpVtbl->Unacquire(self->joystick);
	}
	return success;
}

CALLBACK MidiCallback(HMIDIIN handle, unsigned int uMsg, DWORD_PTR dwInstance, DWORD_PTR dwParam1, DWORD_PTR dwParam2)
{			
	psy_EventDriver* driver = (psy_EventDriver*)dwInstance;	
	DXJoystickDriver* self = (DXJoystickDriver*) driver;	
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

void driver_cmd(psy_EventDriver* driver, const char* sectionname,
	psy_EventDriverInput input, psy_EventDriverCmd* cmd)
{		
	DXJoystickDriver* self = (DXJoystickDriver*)driver;
	psy_Property* section;

	if (!sectionname) {
		return;
	}
	cmd->id = -1;
	section = psy_property_findsection(self->configuration, sectionname);
	if (!section) {
		return;
	}
	if (input.message != FALSE) {
		psy_Property* property = NULL;
		psy_List* p;

		for (p = psy_property_begin(section); p != NULL;
				psy_list_next(&p)) {			
			property = (psy_Property*)psy_list_entry(p);
			if (psy_property_item_int(property) == input.param1) {
				break;
			}
			property = NULL;
		}
		if (property) {
			cmd->id = property->item.id;			
		}
	}
}

psy_EventDriverCmd driver_getcmd(psy_EventDriver* driver, const char* section)
{
	psy_EventDriverCmd cmd;
	DXJoystickDriver* self = (DXJoystickDriver*)driver;
			
	driver_cmd(driver, section, self->lastinput, &cmd);	
	return cmd;
}

const char* driver_target(psy_EventDriver* self)
{
	return NULL;
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

void driver_setcmddef(psy_EventDriver* driver, const psy_Property* cmddef)
{	
	DXJoystickDriver* self = (DXJoystickDriver*)driver;

	if (cmddef && cmddef->children) {
		if (self->cmddef) {
			psy_property_remove(self->configuration, self->cmddef);
		}
		self->cmddef = psy_property_clone(cmddef);
		psy_property_append_property(self->configuration,
			self->cmddef);
		psy_property_settext(self->cmddef, "cmds.keymap");
		driver_setcmddefaults(self, self->cmddef);
	}			
}

void driver_setcmddefaults(DXJoystickDriver* self, psy_Property* cmddef)
{
	psy_Property* section;

	section = psy_property_find(cmddef, "general", PSY_PROPERTY_TYPE_SECTION);
	if (section) {
		psy_property_set_int(section, "cmd_editmachine", INPUT_BUTTON_FIRST + 0);
		psy_property_set_int(section, "cmd_editpattern", INPUT_BUTTON_FIRST + 1);
		psy_property_set_int(section, "cmd_help", INPUT_BUTTON_FIRST + 2);
	}
	section = psy_property_find(cmddef, "tracker", PSY_PROPERTY_TYPE_SECTION);
	if (section) {
		psy_property_set_int(section, "cmd_navup", INPUT_MOVE_UP);
		psy_property_set_int(section, "cmd_navdown", INPUT_MOVE_DOWN);
		psy_property_set_int(section, "cmd_navleft", INPUT_MOVE_LEFT);
		psy_property_set_int(section, "cmd_navright", INPUT_MOVE_RIGHT);
	}
}

void driver_idle(psy_EventDriver* driver)
{
	DIJOYSTATE2 state;
	DXJoystickDriver* self;
	int i;	

	self = (DXJoystickDriver*)driver;
	poll(self, &state);
	for (i = 0; i < 128; ++i) {
		if (self->state.rgbButtons[i] != state.rgbButtons[i]) {
			self->lastinput.message = state.rgbButtons[i];
			self->lastinput.param1 = INPUT_BUTTON_FIRST + i;
			psy_signal_emit(&self->driver.signal_input, self, 0);
		}
	}
	if (state.lY < 0) {
		self->lastinput.message = TRUE;
		self->lastinput.param1 = INPUT_MOVE_UP;
		self->lastinput.param2 = self->state.lY;
		psy_signal_emit(&self->driver.signal_input, self, 0);
	} else if (state.lY > 0) {
		self->lastinput.message = TRUE;
		self->lastinput.param1 = INPUT_MOVE_DOWN;
		self->lastinput.param2 =  state.lY;
		psy_signal_emit(&self->driver.signal_input, self, 0);
	}
	if (state.lX < 0) {
		self->lastinput.message = TRUE;
		self->lastinput.param1 = INPUT_MOVE_LEFT;
		self->lastinput.param2 = self->state.lX;
		psy_signal_emit(&self->driver.signal_input, self, 0);
	} else if (state.lX > 0) {
		self->lastinput.message = TRUE;
		self->lastinput.param1 = INPUT_MOVE_RIGHT;
		self->lastinput.param2 = state.lX;
		psy_signal_emit(&self->driver.signal_input, self, 0);
	}
	self->state = state;
}

HRESULT poll(DXJoystickDriver* self, DIJOYSTATE2* js)
{
	HRESULT     hr;

	if (self->joystick == NULL) {
		return S_OK;
	}
	// Poll the device to read the current state
	hr = self->joystick->lpVtbl->Poll(self->joystick);
	if (FAILED(hr)) {
		// DInput is telling us that the input stream has been
		// interrupted. We aren't tracking any state between polls, so
		// we don't have any special reset that needs to be done. We
		// just re-acquire and try again.
		hr = self->joystick->lpVtbl->Acquire(self->joystick);
		while (hr == DIERR_INPUTLOST) {
			hr = self->joystick->lpVtbl->Acquire(self->joystick);
		}

		// If we encounter a fatal error, return failure.
		if ((hr == DIERR_INVALIDPARAM) || (hr == DIERR_NOTINITIALIZED)) {
			return E_FAIL;
		}

		// If another application has control of this device, return successfully.
		// We'll just have to wait our turn to use the joystick.
		if (hr == DIERR_OTHERAPPHASPRIO) {
			return S_OK;
		}
	}

	// Get the input's device state
	if (FAILED(hr = self->joystick->lpVtbl->GetDeviceState(
		self->joystick,
		sizeof(DIJOYSTATE2), js))) {
		return hr; // The device should have been acquired during the Poll()
	}

	return S_OK;
}

BOOL CALLBACK staticSetGameControllerAxesRanges(LPCDIDEVICEOBJECTINSTANCE devObjInst, LPVOID pvRef)
{
	// the game controller
	LPDIRECTINPUTDEVICE8 gameController = (LPDIRECTINPUTDEVICE8)pvRef;
	gameController->lpVtbl->Acquire(gameController);

	// structure to hold game controller range properties
	DIPROPRANGE gameControllerRange;

	// set the range to -100 and 100
	gameControllerRange.lMin = -100;
	gameControllerRange.lMax = 100;

	// set the size of the structure
	gameControllerRange.diph.dwSize = sizeof(DIPROPRANGE);
	gameControllerRange.diph.dwHeaderSize = sizeof(DIPROPHEADER);

	// set the object that we want to change		
	gameControllerRange.diph.dwHow = DIPH_BYID;
	gameControllerRange.diph.dwObj = devObjInst->dwType;

	// now set the range for the axis		
	if (FAILED(gameController->lpVtbl->SetProperty(gameController,
		DIPROP_RANGE, &gameControllerRange.diph))) {
		return DIENUM_STOP;
	}

	// structure to hold game controller axis dead zone
	DIPROPDWORD gameControllerDeadZone;

	// set the dead zone to 10%
	gameControllerDeadZone.dwData = 1000;

	// set the size of the structure
	gameControllerDeadZone.diph.dwSize = sizeof(DIPROPDWORD);
	gameControllerDeadZone.diph.dwHeaderSize = sizeof(DIPROPHEADER);

	// set the object that we want to change
	gameControllerDeadZone.diph.dwHow = DIPH_BYID;
	gameControllerDeadZone.diph.dwObj = devObjInst->dwType;

	// now set the dead zone for the axis
	if (FAILED(gameController->lpVtbl->SetProperty(gameController,
		DIPROP_DEADZONE, &gameControllerDeadZone.diph)))
		return DIENUM_STOP;

	return DIENUM_CONTINUE;
}

const psy_Property* driver_configuration(const psy_EventDriver* driver)
{
	DXJoystickDriver* self;

	self = (DXJoystickDriver*)driver;
	return self->configuration;
}

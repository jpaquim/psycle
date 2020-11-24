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

#include "../../detail/portable.h"

#if defined DIVERSALIS__OS__MICROSOFT    
#include <windows.h>
#endif

static psy_EventDriverInfo const* psy_audio_kbddriver_info(void)
{
	static psy_EventDriverInfo info;
	info.guid = PSY_EVENTDRIVER_KBD_GUID;
	info.Flags = 0;
	info.Name = "kbd";
	info.ShortName = "kbd";
	info.Version = 0;
	return &info;
}

typedef struct {
	psy_EventDriver driver;
	int (*error)(int, const char*);
	psy_EventDriverInput lastinput;
	psy_Property* cmddef;
	psy_Property* configuration;
	bool shift;
	int chordmode;
} KbdDriver;

static void driver_free(psy_EventDriver*);
static int driver_init(psy_EventDriver*);
static int driver_open(psy_EventDriver*);
static int driver_close(psy_EventDriver*);
static int driver_dispose(psy_EventDriver*);
static const psy_EventDriverInfo* driver_info(psy_EventDriver*);
static void driver_configure(psy_EventDriver*, psy_Property*);
static const psy_Property* driver_configuration(const psy_EventDriver*);
static void driver_write(psy_EventDriver*, psy_EventDriverInput input);
static void driver_cmd(psy_EventDriver*, const char* section, psy_EventDriverInput,
	psy_EventDriverCmd*);
static psy_EventDriverCmd driver_getcmd(psy_EventDriver*, const char* section);
static void setcmddef(psy_EventDriver*, psy_Property*);
static void driver_idle(psy_EventDriver* self) { }
static int onerror(int err, const char* msg);
static void init_properties(psy_EventDriver*);

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
		vtable.write = driver_write;
		vtable.cmd = driver_cmd;
		vtable.getcmd = driver_getcmd;
		vtable.setcmddef = setcmddef;
		vtable.idle = driver_idle;
		vtable_initialized = 1;
	}
}

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

psy_EventDriver* psy_audio_kbddriver_create(void)
{
	KbdDriver* kbd = (KbdDriver*)malloc(sizeof(KbdDriver));
	if (kbd) {			
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
	
	memset(self, 0, sizeof(KbdDriver));
	vtable_init();
	self->driver.vtable = &vtable;
	init_properties(&self->driver);
	psy_signal_init(&driver->signal_input);
	return 0;
}

int driver_dispose(psy_EventDriver* driver)
{
	KbdDriver* self = (KbdDriver*) driver;
	psy_property_deallocate(self->configuration);
	self->configuration = NULL;
	psy_signal_dispose(&driver->signal_input);
	return 1;
}

void init_properties(psy_EventDriver* context)
{
	KbdDriver* self;
	char key[256];

	self = (KbdDriver*)context;
	psy_snprintf(key, 256, "kbd-guid-%d", PSY_EVENTDRIVER_KBD_GUID);
	self->configuration = psy_property_preventtranslate(psy_property_allocinit_key(key));
	psy_property_sethint(psy_property_append_int(self->configuration,
		"guid", PSY_EVENTDRIVER_KBD_GUID, 0, 0),
		PSY_PROPERTY_HINT_HIDE);
	psy_property_settext(
		psy_property_append_string(self->configuration, "name", "kbd"),
		"settingsview.name");
	psy_property_settext(
		psy_property_append_string(self->configuration, "version", "1.0"),
		"settingsview.version");
	self->cmddef = psy_property_settext(
		psy_property_append_section(self->configuration, "cmds"),
		"cmds.keymap");
}

int driver_open(psy_EventDriver* driver)
{
	KbdDriver* self;

	self = (KbdDriver*)(driver);
	self->lastinput.message = 0;
	self->chordmode = 0;
	self->shift = 0;
	return 0;
}

int driver_close(psy_EventDriver* driver)
{
	return 0;
}

const psy_EventDriverInfo* driver_info(psy_EventDriver* self)
{
	return psy_audio_kbddriver_info();
}

void driver_write(psy_EventDriver* driver, psy_EventDriverInput input)
{	
	KbdDriver* self;
	uintptr_t keycode;	
	bool ctrl;

	assert(driver);

	self = (KbdDriver*)(driver);

	// patternview chordmode
	psy_audio_decodeinput((uintptr_t)input.param1, &keycode, &self->shift, &ctrl);
	if (keycode == 0x11 /* psy_ui_KEY_CONTROL */) {
		return;
	}
	if (input.message == psy_EVENTDRIVER_KEYUP && keycode == 0x10 /* psy_ui_KEY_SHIFT */) {
		return;
	}
	if (input.message == psy_EVENTDRIVER_KEYDOWN) {
		bool noteedit;

		noteedit = psy_eventdriver_hostevent(driver, PSY_EVENTDRIVER_PATTERNEDIT, 0, 0)
			&& psy_eventdriver_hostevent(driver, PSY_EVENTDRIVER_NOTECOLUMN, 0, 0);
		if (noteedit && self->shift && !self->chordmode && keycode != 16 /* Shift Key*/) {
			psy_EventDriverCmd cmd;
			psy_EventDriverInput testnote;

			cmd.id = -1;
			testnote = input;
			testnote.param1 = psy_audio_encodeinput(keycode, FALSE, ctrl);
			driver_cmd(driver, "notes", testnote, &cmd);
			if (cmd.id != -1 && cmd.id < psy_audio_NOTECOMMANDS_RELEASE) {
				self->chordmode = TRUE;
				psy_eventdriver_hostevent(driver, PSY_EVENTDRIVER_SETCHORDMODE, 1, 0);
			}
		} else if (noteedit) {
			psy_EventDriverCmd cmd;
			psy_EventDriverInput testnote;

			cmd.id = -1;
			testnote = input;
			testnote.param1 = psy_audio_encodeinput(keycode, FALSE, ctrl);
			driver_cmd(driver, "notes", testnote, &cmd);
			if (cmd.id == psy_audio_NOTECOMMANDS_RELEASE) {
				psy_eventdriver_hostevent(driver, PSY_EVENTDRIVER_INSERTNOTEOFF, 1, 0);
				return;
			}
		}
	}
	if (!self->shift) {
		self->chordmode = FALSE;
	}
	// handle chordmode
	if (self->chordmode != FALSE) {
		// remove shift		
		input.param1 = psy_audio_encodeinput(keycode, FALSE, ctrl);
	}
	self->lastinput = input;
	psy_signal_emit(&self->driver.signal_input, self, 0);
}

void driver_cmd(psy_EventDriver* driver, const char* sectionname,
	psy_EventDriverInput input, psy_EventDriverCmd* cmd)
{		
	KbdDriver* self;
	psy_EventDriverCmd kbcmd;
	psy_Property* section;

	self = (KbdDriver*)(driver);
	cmd->id = -1;
	kbcmd.id = -1;
	if (!sectionname) {
		return;
	}
	section = psy_property_findsection(self->configuration, sectionname);
	if (!section) {
		return;
	}	
	if (input.message == psy_EVENTDRIVER_KEYDOWN) {
		psy_List* p;
		psy_Property* property = NULL;

		for (p = psy_property_children(section); p != NULL;
				psy_list_next(&p)) {			
			property = (psy_Property*)psy_list_entry(p);
			if (psy_property_item_int(property) == input.param1) {
				break;
			}
			property = NULL;
		}
		if (property) {
			kbcmd.id = property->item.id;
			cmd->id = kbcmd.id;
		}		
	} else {
		psy_List* p;
		psy_Property* property = NULL;

		for (p = psy_property_children(section); p != NULL;
				psy_list_next(&p)) {
			property = (psy_Property*)psy_list_entry(p);
			if (psy_property_item_int(property) == input.param1) {				
				break;
			}
			property = NULL;
		}
		if (property) {
			kbcmd.id = property->item.id;
		}
		if (kbcmd.id <= psy_audio_NOTECOMMANDS_RELEASE) {
			cmd->id = psy_audio_NOTECOMMANDS_RELEASE;
		}
	}
}

psy_EventDriverCmd driver_getcmd(psy_EventDriver* driver, const char* section)
{	
	KbdDriver* self;
	psy_EventDriverCmd cmd;	
		
	self = (KbdDriver*)(driver);
	driver_cmd(driver, section, self->lastinput, &cmd);	
	return cmd;
}

void setcmddef(psy_EventDriver* driver, psy_Property* cmddef)
{
	if (cmddef && cmddef->children) {
		KbdDriver* self = (KbdDriver*)driver;

		if (self->cmddef) {
			psy_property_remove(self->configuration, self->cmddef);
		}
		self->cmddef = psy_property_clone(cmddef);
		psy_property_append_property(self->configuration,
			self->cmddef);
		psy_property_settext(self->cmddef, "cmds.keymap");		
	}
}

void driver_configure(psy_EventDriver* driver, psy_Property* configuration)
{
	KbdDriver* self;

	self = (KbdDriver*)driver;
	if (configuration && self->configuration) {
		psy_property_sync(self->configuration, configuration);
	}
}

const psy_Property* driver_configuration(const psy_EventDriver* driver)
{
	KbdDriver* self;

	self = (KbdDriver*)driver;
	return self->configuration;
}

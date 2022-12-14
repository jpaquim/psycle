/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "../../detail/os.h"
#include "patternevent.h"
#include "cmddriver.h"
#include "cmdsnotes.h"
#include "../../driver/eventdriver.h"
/* std */
#include <stdio.h>
/* platform */
#include "../../detail/portable.h"

#if defined DIVERSALIS__OS__MICROSOFT    
#include <windows.h>
#endif

static psy_EventDriverInfo const* psy_audio_cmddriver_info(void)
{
	static psy_EventDriverInfo info;
	info.guid = PSY_EVENTDRIVER_CMD_GUID;
	info.Flags = 0;
	info.Name = "cmd";
	info.ShortName = "cmd";
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
	psy_EventDriverCmd lastcmd;
	char* section;
} CmdDriver;

/* prototypes */
static void driver_free(psy_EventDriver*);
static int driver_init(psy_EventDriver*);
static int driver_open(psy_EventDriver*);
static int driver_close(psy_EventDriver*);
static int driver_dispose(psy_EventDriver*);
static const psy_EventDriverInfo* driver_info(psy_EventDriver*);
static void driver_configure(psy_EventDriver*, psy_Property*);
static const psy_Property* driver_configuration(const psy_EventDriver*);
static void driver_write(psy_EventDriver*, psy_EventDriverInput);
static void driver_cmd(psy_EventDriver*, const char* section, psy_EventDriverInput,
	psy_EventDriverCmd*);
static psy_EventDriverCmd driver_getcmd(psy_EventDriver*, const char* section);
static void setcmddef(psy_EventDriver*, const psy_Property*);
static void driver_idle(psy_EventDriver* self) { }
static int onerror(int err, const char* msg);
static void driver_makeconfig(psy_EventDriver*);
static psy_EventDriverInput driver_input(psy_EventDriver* context)
{
	CmdDriver* self = (CmdDriver*)context;
	return self->lastinput;
}
/* vtable */
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
		vtable.write = driver_write;
		vtable.cmd = driver_cmd;
		vtable.getcmd = driver_getcmd;		
		vtable.setcmddef = setcmddef;
		vtable.idle = driver_idle;
		vtable.input = driver_input;
		vtable_initialized = TRUE;
	}
}

/* implementation */
int onerror(int err, const char* msg)
{
#if defined DIVERSALIS__OS__MICROSOFT    
	MessageBox(0, msg, "cmd driver", MB_OK | MB_ICONERROR);
	return 0;
#else
    fprintf(stderr, "cmd driver %s", msg);
	return 0;
#endif    
}

void psy_audio_cmddriver_setcmd(psy_EventDriver* driver, const char* section, psy_EventDriverCmd cmd)
{
	CmdDriver* self = (CmdDriver*)driver;

	psy_strreset(&self->section, section);
	self->lastcmd = cmd;
}

psy_EventDriver* psy_audio_cmddriver_create(void)
{
	CmdDriver* cmd = (CmdDriver*)malloc(sizeof(CmdDriver));
	if (cmd) {			
		driver_init(&cmd->driver);
		return &cmd->driver;
	}
	return NULL;
}

void driver_free(psy_EventDriver* driver)
{
	driver_dispose(driver);
	free(driver);
}

int driver_init(psy_EventDriver* driver)
{
	CmdDriver* self = (CmdDriver*) driver;
	
	memset(self, 0, sizeof(CmdDriver));
	vtable_init();
	self->driver.vtable = &vtable;
	self->section = NULL;
	self->driver.callback = NULL;
	self->driver.callbackcontext = NULL;
	driver_makeconfig(&self->driver);		
	return 0;
}

int driver_dispose(psy_EventDriver* driver)
{
	CmdDriver* self = (CmdDriver*) driver;
	psy_property_deallocate(self->configuration);
	self->configuration = NULL;	
	free(self->section);
	self->section = NULL;
	return 1;
}

void driver_makeconfig(psy_EventDriver* context)
{
	CmdDriver* self;
	char key[256];

	self = (CmdDriver*)context;
	psy_snprintf(key, 256, "cmd-guid-%d", PSY_EVENTDRIVER_CMD_GUID);
	self->configuration = psy_property_preventtranslate(psy_property_allocinit_key(key));
	psy_property_hide(psy_property_append_int(self->configuration,
		"guid", PSY_EVENTDRIVER_CMD_GUID, 0, 0));
	psy_property_set_text(
		psy_property_append_str(self->configuration, "name", "cmd"),
		"settings.name");
	psy_property_set_text(
		psy_property_append_str(self->configuration, "version", "1.0"),
		"settings.version");
	self->cmddef = psy_property_set_text(
		psy_property_append_section(self->configuration, "cmds"),
		"cmds.keymap");
}

int driver_open(psy_EventDriver* driver)
{
	CmdDriver* self;

	self = (CmdDriver*)(driver);
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
	return psy_audio_cmddriver_info();
}

void driver_write(psy_EventDriver* driver, psy_EventDriverInput input)
{	
	CmdDriver* self;
	uint32_t keycode;	
	bool ctrl;
	bool alt;
	bool up;

	assert(driver);

	self = (CmdDriver*)(driver);

	/* patternview chordmode */
	psy_audio_decodeinput((uint32_t)input.param1, &keycode, &self->shift, &ctrl, &alt, &up);
	if (keycode == 0x11 /* psy_ui_KEY_CONTROL */) {
		return;
	}	
	self->lastinput = input;
	if (driver->callback) {
		driver->callback(driver->callbackcontext, driver);
	}	
}

void driver_cmd(psy_EventDriver* driver, const char* sectionname,
	psy_EventDriverInput input, psy_EventDriverCmd* cmd)
{		
	CmdDriver* self;
	psy_EventDriverCmd kbcmd;
	psy_Property* section;
	psy_List* p;
	psy_Property* property = NULL;

	self = (CmdDriver*)(driver);
	cmd->id = -1;
	kbcmd.id = -1;
	if (!sectionname) {
		return;
	}
	section = psy_property_findsection(self->configuration, sectionname);
	if (!section) {
		return;
	}			

	for (p = psy_property_begin(section); p != NULL;
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
}

psy_EventDriverCmd driver_getcmd(psy_EventDriver* driver, const char* section)
{	
	CmdDriver* self;
	psy_EventDriverCmd rv;	
	
	rv.id = -1;
	self = (CmdDriver*)(driver);	
	if (!section || strcmp(self->section, section) == 0) {
		rv = self->lastcmd;
	}
	return rv;
}

void setcmddef(psy_EventDriver* driver, const psy_Property* cmddef)
{
	if (cmddef && cmddef->children) {
		CmdDriver* self = (CmdDriver*)driver;

		if (self->cmddef) {
			psy_property_remove(self->configuration, self->cmddef);
		}
		self->cmddef = psy_property_clone(cmddef);
		psy_property_append_property(self->configuration,
			self->cmddef);
		psy_property_set_text(self->cmddef, "cmds.keymap");		
	}
}

void driver_configure(psy_EventDriver* driver, psy_Property* configuration)
{
	CmdDriver* self;

	self = (CmdDriver*)driver;
	if (configuration && self->configuration) {
		psy_property_sync(self->configuration, configuration);
	}
}

const psy_Property* driver_configuration(const psy_EventDriver* driver)
{
	CmdDriver* self;

	self = (CmdDriver*)driver;
	return self->configuration;
}

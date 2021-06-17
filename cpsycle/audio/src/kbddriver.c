// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"


#include "kbddriver.h"
/* driver */
#include "../../driver/eventdriver.h"
/* platform */
#include "../../detail/os.h"
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
static void setcmddef(psy_EventDriver*, const psy_Property*);
static void driver_idle(psy_EventDriver* self) { }

static psy_EventDriverInput driver_input(psy_EventDriver* context)
{
	KbdDriver* self = (KbdDriver*)context;
	return self->lastinput;
}

static int onerror(int err, const char* msg);
static void driver_makeconfig(psy_EventDriver*);

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
	self->driver.callback = NULL;
	self->driver.callbackcontext = NULL;
	driver_makeconfig(&self->driver);
	return 0;
}

int driver_dispose(psy_EventDriver* driver)
{
	KbdDriver* self = (KbdDriver*) driver;
	psy_property_deallocate(self->configuration);
	self->configuration = NULL;	
	return 1;
}

void driver_makeconfig(psy_EventDriver* context)
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
		psy_property_append_str(self->configuration, "name", "kbd"),
		"settingsview.name");
	psy_property_settext(
		psy_property_append_str(self->configuration, "version", "1.0"),
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

void driver_write(psy_EventDriver* context, psy_EventDriverInput input)
{	
	KbdDriver* self;
	uint32_t keycode;
	bool shift;
	bool ctrl;
	bool alt;
	bool up;

	assert(context);

	self = (KbdDriver*)(context);
	/* reset lastcmd */	
	/* filter control key */
	psy_audio_decodeinput((uint32_t)input.param1, &keycode, &shift, &ctrl, &alt, &up);
	if (keycode == 0x11 /* psy_ui_KEY_CONTROL */) {	
		return;
	}
	self->lastinput = input;
	if (context->callback) {
		context->callback(context->callbackcontext, context);
	}
}

void driver_cmd(psy_EventDriver* driver, const char* sectionname,
	psy_EventDriverInput input, psy_EventDriverCmd* cmd)
{		
	KbdDriver* self;
	psy_EventDriverCmd kbcmd;
	psy_Property* section;
	psy_List* p;
	psy_Property* property;

	self = (KbdDriver*)(driver);
	cmd->type = psy_EVENTDRIVER_CMD;
	cmd->id = -1;
	kbcmd.id = -1;
	if (!sectionname) {
		return;
	}
	section = psy_property_findsection(self->configuration, sectionname);
	if (!section) {
		return;
	}			
	property = NULL;

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

psy_EventDriverCmd driver_getcmd(psy_EventDriver* context, const char* section)
{	
	KbdDriver* self;
	psy_EventDriverCmd cmd;	
		
	self = (KbdDriver*)(context);	
	driver_cmd(context, section, self->lastinput, &cmd);	
	return cmd;
}

void setcmddef(psy_EventDriver* driver, const psy_Property* cmddef)
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

// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "silentdriver.h"
#include "../../driver/driver.h"
#include <stdlib.h>
#include <string.h>

static void driver_deallocate(psy_AudioDriver*);
static int driver_init(psy_AudioDriver*);
static void driver_connect(psy_AudioDriver*, void* context, AUDIODRIVERWORKFN callback,
	void* handle);
static int driver_open(psy_AudioDriver*);
static void driver_configure(psy_AudioDriver*, psy_Properties*);
static int driver_close(psy_AudioDriver*);
static int driver_dispose(psy_AudioDriver*);
static uintptr_t samplerate(psy_AudioDriver*);
static const char* capturename(psy_AudioDriver*, int index);
static int numcaptures(psy_AudioDriver*);
static const char* playbackname(psy_AudioDriver*, int index);
static int numplaybacks(psy_AudioDriver*);

static void init_properties(psy_AudioDriver* driver);

psy_AudioDriver* psy_audio_create_silent_driver(void)
{
	psy_AudioDriver* driver = malloc(sizeof(psy_AudioDriver));	
	driver_init(driver);
	return driver;
}

void driver_deallocate(psy_AudioDriver* self)
{
	self->dispose(self);
	free(self);
}

int driver_init(psy_AudioDriver* driver)
{
	memset(driver, 0, sizeof(psy_AudioDriver));
	driver->open = driver_open;
	driver->deallocate = driver_deallocate;	
	driver->connect = driver_connect;
	driver->open = driver_open;
	driver->close = driver_close;
	driver->dispose = driver_dispose;
	driver->configure = driver_configure;
	driver->samplerate = samplerate;
	driver->capturename = (psy_audiodriver_fp_capturename) capturename;
	driver->numcaptures = (psy_audiodriver_fp_numcaptures)numcaptures;
	driver->playbackname = (psy_audiodriver_fp_playbackname)playbackname;
	driver->numplaybacks = (psy_audiodriver_fp_numplaybacks)numplaybacks;
	init_properties(driver);
	return 0;
}

int driver_dispose(psy_AudioDriver* driver)
{
	properties_free(driver->properties);
	driver->properties = 0;
	return 0;
}

void driver_connect(psy_AudioDriver* driver, void* context, AUDIODRIVERWORKFN callback,
	void* handle)
{
	driver->_pCallback = callback;
	driver->_callbackContext = context;
}

int driver_open(psy_AudioDriver* driver)
{
	return 0;
}

int driver_close(psy_AudioDriver* driver)
{
	return 0;
}

void driver_configure(psy_AudioDriver* driver, psy_Properties* config)
{

}

uintptr_t samplerate(psy_AudioDriver* self)
{
	return 44100;
}

void init_properties(psy_AudioDriver* self)
{		
	self->properties = psy_properties_create();
	psy_properties_sethint(
		psy_properties_append_string(self->properties, "name", "Silent AudioDriver"),
		PSY_PROPERTY_HINT_READONLY);
	psy_properties_sethint(
		psy_properties_append_string(self->properties, "vendor", "Psycledelics"),
		PSY_PROPERTY_HINT_READONLY);
	psy_properties_sethint(
		psy_properties_append_string(self->properties, "version", "1.0"),
		PSY_PROPERTY_HINT_READONLY);
}

const char* capturename(psy_AudioDriver* driver, int index)
{
	return "";
}

int numcaptures(psy_AudioDriver* driver)
{
	return 0;
}

const char* playbackname(psy_AudioDriver* driver, int index)
{
	return "";
}

int numplaybacks(psy_AudioDriver* driver)
{
	return 0;
}
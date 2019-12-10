// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "silentdriver.h"
#include <stdlib.h>
#include <string.h>

static void driver_free(Driver*);
static int driver_init(Driver*);
static void driver_connect(Driver*, void* context, AUDIODRIVERWORKFN callback,
	void* handle);
static int driver_open(Driver*);
static void driver_configure(Driver*, Properties*);
static int driver_close(Driver*);
static int driver_dispose(Driver*);
static unsigned int samplerate(Driver*);

static void init_properties(Driver* driver);

Driver* create_silent_driver(void)
{
	Driver* driver = malloc(sizeof(Driver));	
	driver_init(driver);
	return driver;
}

void driver_free(Driver* driver)
{
	free(driver);
}

int driver_init(Driver* driver)
{
	memset(driver, 0, sizeof(Driver));
	driver->open = driver_open;
	driver->free = driver_free;	
	driver->connect = driver_connect;
	driver->open = driver_open;
	driver->close = driver_close;
	driver->dispose = driver_dispose;
	driver->configure = driver_configure;
	driver->samplerate = samplerate;
	init_properties(driver);
	return 0;
}

int driver_dispose(Driver* driver)
{
	properties_free(driver->properties);
	driver->properties = 0;
	return 0;
}

void driver_connect(Driver* driver, void* context, AUDIODRIVERWORKFN callback,
	void* handle)
{
	driver->_pCallback = callback;
	driver->_callbackContext = context;
}

int driver_open(Driver* driver)
{
	return 0;
}

int driver_close(Driver* driver)
{
	return 0;
}

void driver_configure(Driver* driver, Properties* config)
{

}

unsigned int samplerate(Driver* self)
{
	return 44100;
}

void init_properties(Driver* self)
{		
	self->properties = properties_create();
	properties_sethint(
		properties_append_string(self->properties, "name", "Silent Driver"),
		PROPERTY_HINT_READONLY);
	properties_sethint(
		properties_append_string(self->properties, "vendor", "Psycedelics"),
		PROPERTY_HINT_READONLY);
	properties_sethint(
		properties_append_string(self->properties, "version", "1.0"),
		PROPERTY_HINT_READONLY);
}

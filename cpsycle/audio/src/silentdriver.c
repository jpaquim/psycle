// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#include "silentdriver.h"
#include <stdlib.h>
#include <string.h>

static void driver_free(Driver* driver);
static int driver_init(Driver* driver);
static void driver_connect(Driver* driver, void* context, AUDIODRIVERWORKFN callback, void* handle);
static int driver_open(Driver* driver);
static int driver_close(Driver* driver);
static int driver_dispose(Driver* driver);

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
	driver->init = driver_init;
	driver->connect = driver_connect;
	driver->open = driver_open;
	driver->close = driver_close;
	driver->dispose = driver_dispose;
	init_properties(driver);

	return 0;
}

int driver_dispose(Driver* driver)
{
	properties_free(driver->properties);
	driver->properties = 0;
	return 0;
}

void driver_connect(Driver* driver, void* context, AUDIODRIVERWORKFN callback, void* handle)
{
}

int driver_open(Driver* driver)
{
	return 0;
}

int driver_close(Driver* driver)
{
	return 0;
}

void init_properties(Driver* driver)
{	
	Properties* property;	

	driver->properties = properties_create_string("name", "Silent Driver");
	properties_append_string(driver->properties, "version", "1.0");
	property = properties_append_choice(driver->properties, "device", -1);	
	properties_append_int(driver->properties, "bitdepth", 16, 0, 32);
	properties_append_int(driver->properties, "samplerate", 44100, 0, 0);
	properties_append_int(driver->properties, "dither", 0, 0, 1);	
	properties_append_int(driver->properties, "numsamples", 256, 128, 8193);	
}

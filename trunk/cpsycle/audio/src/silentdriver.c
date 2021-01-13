// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "silentdriver.h"
#include "../../driver/audiodriver.h"
#include <stdlib.h>
#include <string.h>
#include "../../detail/portable.h"

#define PSY_AUDIODRIVER_SILENTDRIVER_GUID 0x0006

typedef struct {
	psy_AudioDriver driver;
	psy_Property* configuration;
} SilentDriver;

static void driver_deallocate(psy_AudioDriver*);
static int driver_init(SilentDriver*);
static int driver_open(psy_AudioDriver*);
static void driver_configure(psy_AudioDriver*, psy_Property*);
static const psy_Property* driver_configuration(const psy_AudioDriver*);
static int driver_close(psy_AudioDriver*);
static int driver_dispose(psy_AudioDriver*);
static psy_dsp_big_hz_t samplerate(psy_AudioDriver*);
static const char* capturename(psy_AudioDriver*, int index);
static int numcaptures(psy_AudioDriver*);
static const char* playbackname(psy_AudioDriver*, int index);
static int numplaybacks(psy_AudioDriver*);
static const psy_AudioDriverInfo* driver_info(psy_AudioDriver*);
static const psy_Property* driver_configuration(const struct psy_AudioDriver*);
static uint32_t playposinsamples(psy_AudioDriver*);

static void init_properties(SilentDriver* driver);

static psy_AudioDriverVTable vtable;
static int vtable_initialized = 0;

static void vtable_init(void)
{
	if (!vtable_initialized) {
		vtable.open = driver_open;
		vtable.deallocate = driver_deallocate;
		vtable.open = driver_open;
		vtable.close = driver_close;
		vtable.dispose = driver_dispose;
		vtable.configure = driver_configure;
		vtable.configuration = driver_configuration;
		vtable.samplerate = (psy_audiodriver_fp_samplerate)samplerate;
		vtable.capturename = (psy_audiodriver_fp_capturename)capturename;
		vtable.numcaptures = (psy_audiodriver_fp_numcaptures)numcaptures;
		vtable.playbackname = (psy_audiodriver_fp_playbackname)playbackname;
		vtable.numplaybacks = (psy_audiodriver_fp_numplaybacks)numplaybacks;
		vtable.info = (psy_audiodriver_fp_info)driver_info;
		vtable.playposinsamples = playposinsamples;
		vtable_initialized = 1;
	}
}

static const psy_AudioDriverInfo* GetPsycleDriverInfo(void)
{
	static psy_AudioDriverInfo info;

	info.guid = PSY_AUDIODRIVER_SILENTDRIVER_GUID;
	info.Flags = 0;
	info.Name = "SilentAudioDriver";
	info.ShortName = "Silent";
	info.Version = 0;
	return &info;
}

psy_AudioDriver* psy_audio_create_silent_driver(void)
{
	SilentDriver* silent;
	
	silent = (SilentDriver*)malloc(sizeof(SilentDriver));
	if (silent) {
		driver_init(silent);
		return &silent->driver;
	}
	return NULL;
}

void driver_deallocate(psy_AudioDriver* self)
{
	driver_dispose(self);
	free(self);
}

int driver_init(SilentDriver* self)
{	
	memset(&self->driver, 0, sizeof(psy_AudioDriver));
	vtable_init();
	self->driver.vtable = &vtable;	
	init_properties(self);
	psy_signal_init(&self->driver.signal_stop);
	return 0;
}

int driver_dispose(psy_AudioDriver* driver)
{
	SilentDriver* self;

	self = (SilentDriver*)driver;
	psy_property_deallocate(self->configuration);
	self->configuration = NULL;
	psy_signal_dispose(&driver->signal_stop);
	return 0;
}

int driver_open(psy_AudioDriver* driver)
{
	return 0;
}

int driver_close(psy_AudioDriver* driver)
{
	return 0;
}

void driver_configure(psy_AudioDriver* driver, psy_Property* config)
{

}

psy_dsp_big_hz_t samplerate(psy_AudioDriver* self)
{
	return (psy_dsp_big_hz_t)44100.0;
}

void init_properties(SilentDriver* self)
{		
	char key[256];

	psy_snprintf(key, 256, "silent-guid-%d",
		PSY_AUDIODRIVER_SILENTDRIVER_GUID);
	self->configuration = psy_property_preventtranslate(psy_property_settext(
		psy_property_allocinit_key(key), "Silent"));
	psy_property_setreadonly(
		psy_property_append_string(self->configuration, "name", "Silent AudioDriver"),
		TRUE);
	psy_property_setreadonly(
		psy_property_append_string(self->configuration, "vendor", "Psycledelics"),
		TRUE);
	psy_property_setreadonly(
		psy_property_append_string(self->configuration, "version", "1.0"),
		TRUE);
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

const psy_AudioDriverInfo* driver_info(psy_AudioDriver* self)
{
	return GetPsycleDriverInfo();
}

const psy_Property* driver_configuration(const psy_AudioDriver* driver)
{
	SilentDriver* self;

	self = (SilentDriver*)driver;
	return self->configuration;
}

uint32_t playposinsamples(psy_AudioDriver* driver)
{
	return 0;
}

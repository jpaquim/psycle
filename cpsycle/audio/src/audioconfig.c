/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "audioconfig.h"
/* platform */
#include "../../detail/portable.h"


/* prototypes */
static void audioconfig_make_section(AudioConfig*,
	psy_Property* parent);
static void audioconfig_make_threads(AudioConfig*);
static void audioconfig_make_configuration(AudioConfig*, psy_Property*);
static void audioconfig_make_driver_list(AudioConfig*);

/* implementation */
void audioconfig_init(AudioConfig* self, psy_Property* parent,
	psy_audio_Player* player)
{
	self->player = player;
	self->audioenabled = TRUE;
	audioconfig_make_section(self, parent);
	psy_signal_init(&self->signal_changed);
}

void audioconfig_dispose(AudioConfig* self)
{
	assert(self);

	psy_signal_dispose(&self->signal_changed);
}

void audioconfig_make_section(AudioConfig* self, psy_Property* parent)
{
	assert(self);

	self->inputoutput = psy_property_settext(
		psy_property_append_section(parent, "inputoutput"),
		"Input/Output");
	audioconfig_make_driver_list(self);
	audioconfig_make_threads(self);
	self->driver_configure = psy_property_settext(
		psy_property_append_section(self->inputoutput, "configure"),
		"settingsview.configure");
	self->driver_configure->item.save = 0;	
	audioconfig_make_driver_configurations(self, FALSE /* full*/ );
}

void audioconfig_make_driver_configurations(AudioConfig* self,
	bool full)
{
	psy_Property* drivers;
	psy_Property* driverconfigurations;

	assert(self);

	driverconfigurations = psy_property_at(self->inputoutput,
		"configurations", PSY_PROPERTY_TYPE_NONE);
	if (driverconfigurations) {
		psy_property_remove(self->inputoutput, driverconfigurations);
	}
	self->driverconfigurations = psy_property_sethint(
		psy_property_append_section(self->inputoutput,
		"configurations"),
		PSY_PROPERTY_HINT_HIDE);
	if (!full) {
		return;
	}
	drivers = psy_property_at(self->inputoutput, "audiodrivers",
		PSY_PROPERTY_TYPE_NONE);
	if (drivers) {
		psy_List* p;

		for (p = psy_property_begin(drivers); p != NULL; psy_list_next(&p)) {
			psy_Property* property;
			
			property = (psy_Property*)psy_list_entry(p);
			audioconfig_make_configuration(self, property);			
		}
	}	
}

void audioconfig_makeconfiguration_driverkey(AudioConfig* self,
	const char* key)
{
	assert(self);

	if (key && self->driverconfigurations) {
		psy_Property* drivers;	

		drivers = psy_property_at(self->inputoutput, "audiodrivers",
			PSY_PROPERTY_TYPE_NONE);
		if (drivers) {
			psy_List* p;

			for (p = psy_property_begin(drivers); p != NULL;
					psy_list_next(&p)) {
				psy_Property* property;

				property = (psy_Property*)psy_list_entry(p);
				if (strcmp(key, psy_property_key(property)) == 0) {
					audioconfig_make_configuration(self, property);
					break;
				}
			}
		}
	}
}

void audioconfig_make_threads(AudioConfig* self)
{	
	psy_Property* threads;

	threads = psy_property_settext(
		psy_property_append_section(self->inputoutput, "threads"),
		"Audio Threads");
	psy_property_set_id(psy_property_settext(
		psy_property_append_int(threads, "num", 0, 0, 99),
		"Use the value 0 to autodetect your cpu threads"),
		PROPERTY_ID_NUMAUDIOTHREADS);
}

void audioconfig_make_configuration(AudioConfig* self,
	psy_Property* property)
{	
	assert(property);

	if (psy_property_type(property) == PSY_PROPERTY_TYPE_STRING) {
		const char* path;

		path = psy_property_item_str(property);
		if (path) {
			psy_Library library;

			psy_library_init(&library);
			psy_library_load(&library, path);
			if (library.module) {
				pfndriver_create fpdrivercreate;

				fpdrivercreate = (pfndriver_create)
					psy_library_functionpointer(&library,
						"driver_create");
				if (fpdrivercreate) {
					psy_AudioDriver* driver;

					driver = fpdrivercreate();
					if (driver &&
							psy_audiodriver_configuration(driver) &&
							!psy_property_empty(
							psy_audiodriver_configuration(driver))) {
						psy_property_append_property(
							self->driverconfigurations,
							psy_property_clone(
								psy_audiodriver_configuration(driver)));
						psy_audiodriver_dispose(driver);
					}
				}
			}
			psy_library_dispose(&library);
		}
	}
}

void audioconfig_make_driver_list(AudioConfig* self)
{	
	assert(self);

	/* change number to set startup driver, if no psycle.ini found */
#if defined(DIVERSALIS__OS__MICROSOFT)	
	/* 2 : directx */
	self->drivers = psy_property_set_id(psy_property_settext(
		psy_property_append_choice(self->inputoutput,
			"audiodrivers", 2),
		"settingsview.audio-drivers"),
		PROPERTY_ID_AUDIODRIVERS);
#elif defined(DIVERSALIS__OS__LINUX)
	/* 1 : alsa */
	self->drivers = psy_property_settext(psy_property_append_choice(
		self->inputoutput, "audiodrivers", 1),
		"settingsview.audio-drivers");
#endif		
	psy_property_append_str(self->drivers, "silent", "silentdriver");
#if defined(DIVERSALIS__OS__MICROSOFT)
	/*
	** output target for the audio driver dlls is {solutiondir}/Debug or 
	** {solutiondir}/Release
	** if they aren't found, check if direcories fit and if
	** dlls are compiled
	*/
	psy_property_append_str(self->drivers, "mme", ".\\mme.dll");
	psy_property_append_str(self->drivers, "directx", ".\\directx.dll");
	psy_property_append_str(self->drivers, "wasapi", ".\\wasapi.dll");
	psy_property_append_str(self->drivers, "asio", ".\\asiodriver.dll");
#elif defined(DIVERSALIS__OS__LINUX)
	psy_property_append_str(self->drivers, "alsa",
		"../../driver/alsa/libpsyalsa.so");
		psy_property_append_str(self->drivers, "jack",
		"../../driver/jack/libpsyjack.so");
#endif
}

const char* audioconfig_driver_path(AudioConfig* self)
{
	psy_Property* p;

	assert(self);

	if (p = psy_property_at(self->inputoutput, "audiodrivers",
		PSY_PROPERTY_TYPE_NONE)) {
		if (p = psy_property_at_choice(p)) {
			return psy_property_item_str(p);
		}
	}
	return NULL;
}

const char* audioconfig_driverkey(AudioConfig* self)
{
	psy_Property* p;
	const char* rv = 0;

	assert(self);

	if (p = psy_property_at(self->inputoutput, "audiodrivers",
		PSY_PROPERTY_TYPE_NONE)) {
		if (p = psy_property_at_choice(p)) {
			rv = psy_property_key(p);
		}
	}
	return rv;
}

void audioconfig_driverconfigure_section(AudioConfig* self)
{
	assert(self);

	if (self->driver_configure) {
		psy_Property* driversection;

		psy_property_clear(self->driver_configure);
		if (psy_audiodriver_configuration(self->player->driver)) {
			psy_property_append_property(self->driver_configure,
				psy_property_clone(psy_audiodriver_configuration(
					self->player->driver)));
		}
		if (psy_audiodriver_configuration(self->player->driver)) {
			driversection = psy_property_find(self->driverconfigurations,
				psy_property_key(
					psy_audiodriver_configuration(self->player->driver)),
				PSY_PROPERTY_TYPE_NONE);
			if (driversection) {
				psy_property_remove(self->driverconfigurations,
					driversection);
			}
			psy_property_append_property(self->driverconfigurations,
				psy_property_clone(psy_audiodriver_configuration(
					self->player->driver)));
		}
	}
}

void audioconfig_onaudiodriverselect(AudioConfig* self, bool enable)
{
	psy_Property* driversection = NULL;

	psy_audio_player_unloaddriver(self->player);
	psy_audio_player_loaddriver(self->player, audioconfig_driver_path(self),
		NULL /*no config*/, FALSE /*do not open yet*/);
	if (psy_audiodriver_configuration(self->player->driver)) {
		driversection = psy_property_find(self->driverconfigurations,
			psy_property_key(psy_audiodriver_configuration(
				self->player->driver)),
			PSY_PROPERTY_TYPE_NONE);
	}
	if (enable) {
		psy_audio_player_restart_driver(self->player, driversection);
	} else if (self->player->driver) {
		psy_audiodriver_configure(self->player->driver, driversection);
	}
	psy_property_clear(self->driver_configure);
	if (psy_audiodriver_configuration(self->player->driver)) {
		psy_property_append_property(self->driver_configure,
			psy_property_clone(psy_audiodriver_configuration(
				self->player->driver)));
	}
}

void audioconfig_on_edit_audio_driver_configuration(AudioConfig* self, bool enabled)
{
	if (psy_audiodriver_configuration(self->player->driver) != NULL) {
		psy_Property* driversection;

		driversection = psy_property_find(self->driver_configure,
			psy_property_key(psy_audiodriver_configuration(
				self->player->driver)),
			PSY_PROPERTY_TYPE_NONE);
		if (driversection) {
			if (enabled) {
				psy_audio_player_restart_driver(self->player, driversection);
			} else if (self->player->driver) {
				psy_audiodriver_configure(self->player->driver, driversection);
			}			
		}
	}
}

uintptr_t audioconfig_numthreads(const AudioConfig* self)
{
	psy_Property* p;

	assert(self);

	if (p = psy_property_at(self->inputoutput, "threads",
			PSY_PROPERTY_TYPE_NONE)) {		
		return psy_property_at_int(p, "num", 0);		
	}
	return 0;
}

/* events */
uintptr_t audioconfig_on_changed(AudioConfig* self, psy_Property*
	property)
{
	uintptr_t rebuild_level;

	assert(self);
	assert(property);

	rebuild_level = psy_INDEX_INVALID;
	if (psy_property_hasid(property, PROPERTY_ID_NUMAUDIOTHREADS)) {
		psy_audio_player_stop_threads(self->player);
		psy_audio_player_start_threads(self->player, psy_property_item_int(property));			
	} else if (psy_property_hasid(property, PROPERTY_ID_AUDIODRIVERS)) {
		audioconfig_onaudiodriverselect(self, self->audioenabled);
		rebuild_level = 1;			
	} else {
		psy_signal_emit(&self->signal_changed, self, 1, property);
	}
	return rebuild_level;
}

bool audioconfig_hasproperty(const AudioConfig* self,
	psy_Property* property)
{
	assert(self);
	assert(self->inputoutput);

	return psy_property_in_section(property, self->inputoutput);
}

psy_Property* audioconfig_drivers(AudioConfig* self)
{
	assert(self);

	return self->drivers;
}

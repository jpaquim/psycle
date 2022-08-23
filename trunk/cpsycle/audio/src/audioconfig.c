/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "audioconfig.h"

/* platform */
#include "../../detail/portable.h"
#include "../../detail/psyconf.h"


/* prototypes */
static void audioconfig_make(AudioConfig*,
	psy_Property* parent);
static void audioconfig_make_threads(AudioConfig*);
static void audioconfig_make_configuration(AudioConfig*, psy_Property*);
static void audioconfig_make_driver_list(AudioConfig*);
static void audioconfig_on_set_num_threads(AudioConfig*, psy_Property* sender);
static void audioconfig_on_driver_select(AudioConfig*, psy_Property* sender);
static void audioconfig_select_driver(AudioConfig*, bool enable);
static void audioconfig_on_edit_audio_driver_configuration(AudioConfig*,
	psy_Property* sender);

/* implementation */
void audioconfig_init(AudioConfig* self, psy_Property* parent,
	psy_audio_Player* player)
{
	self->player = player;
	self->audioenabled = TRUE;
	audioconfig_make(self, parent);	
}

void audioconfig_dispose(AudioConfig* self)
{
	assert(self);	
}

void audioconfig_make(AudioConfig* self, psy_Property* parent)
{
	assert(self);

	self->inputoutput = psy_property_set_text(
		psy_property_append_section(parent, "inputoutput"),
		"settings.io.input-output");
	audioconfig_make_driver_list(self);
	audioconfig_make_threads(self);
	self->driver_configure = psy_property_set_text(
		psy_property_append_section(self->inputoutput, "configure"),
		"settings.configure");
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
	self->driverconfigurations = psy_property_hide(
		psy_property_append_section(self->inputoutput,
		"configurations"));
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

	threads = psy_property_set_text(
		psy_property_append_section(self->inputoutput, "threads"),
		"Audio Threads");
	psy_property_connect(psy_property_set_id(psy_property_set_text(
		psy_property_append_int(threads, "num", 0, 0, 99),
		"Use the value 0 to autodetect your cpu threads"),
		PROPERTY_ID_NUMAUDIOTHREADS),
		self, audioconfig_on_set_num_threads);
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
						psy_Property* configuration;

						configuration = psy_property_append_property(
							self->driverconfigurations,
							psy_property_clone(
								psy_audiodriver_configuration(driver)));
						psy_property_hide(configuration);
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
	uintptr_t default_driver_index;

	assert(self);

	/* change number to set startup driver, if no psycle.ini found */
#if defined(DIVERSALIS__OS__MICROSOFT)	
	default_driver_index = 2;  /* 2 : directx */
#elif defined(DIVERSALIS__OS__LINUX)
	default_driver_index = 1;  /* 1 : alsa */
#else
	default_driver_index = 0;  /* 0 : silent */
#endif
	self->drivers = psy_property_set_hint(
		psy_property_set_id(psy_property_set_text(
		psy_property_append_choice(self->inputoutput,
			"audiodrivers", default_driver_index),
		"settings.audio-drivers"),
		PROPERTY_ID_AUDIODRIVERS),
		PSY_PROPERTY_HINT_COMBO);
	psy_property_connect(self->drivers,
		self, audioconfig_on_driver_select);
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
		PSYCLE_AUDIO_DRIVER_DIR"/alsa/libpsyalsa.so");
		psy_property_append_str(self->drivers, "jack",
		PSYCLE_AUDIO_DRIVER_DIR"/jack/libpsyjack.so");
#endif
}

const char* audioconfig_driver_path(AudioConfig* self)
{
	psy_Property* p;

	assert(self);

	if ((p = psy_property_at(self->inputoutput, "audiodrivers",
			PSY_PROPERTY_TYPE_NONE))) {
		if ((p = psy_property_at_choice(p))) {
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

	if ((p = psy_property_at(self->inputoutput, "audiodrivers",
		PSY_PROPERTY_TYPE_NONE))) {
		if ((p = psy_property_at_choice(p))) {
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
			psy_property_connect_children(self->driver_configure, TRUE,
				self, audioconfig_on_edit_audio_driver_configuration);
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

void audioconfig_select_driver(AudioConfig* self, bool enable)
{
	psy_Property* driversection = NULL;

	psy_audio_player_unloaddriver(self->player);
	psy_audio_player_loaddriver(self->player, audioconfig_driver_path(self),
		NULL /* no config */, FALSE /* do not open yet */);	
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
		psy_property_connect_children(self->driver_configure, TRUE,
			self, audioconfig_on_edit_audio_driver_configuration);
	}
	psy_property_rebuild(self->driver_configure);
}

void audioconfig_on_edit_audio_driver_configuration(AudioConfig* self,
	psy_Property* sender)
{	
	if (psy_audiodriver_configuration(self->player->driver) != NULL) {
		psy_Property* driversection;
		
		driversection = psy_property_find(self->driver_configure,
			psy_property_key(psy_audiodriver_configuration(
				self->player->driver)),
			PSY_PROPERTY_TYPE_NONE);
		if (driversection) {
			if (psy_audio_player_enabled(self->player)) {
				psy_audio_player_restart_driver(self->player, driversection);
			} else if (self->player->driver) {
				psy_audiodriver_configure(self->player->driver, driversection);
			}			
		}
	}
}

uintptr_t audioconfig_num_threads(const AudioConfig* self)
{
	psy_Property* p;

	assert(self);

	if ((p = psy_property_at(self->inputoutput, "threads",
			PSY_PROPERTY_TYPE_NONE))) {
		return psy_property_at_int(p, "num", 0);		
	}
	return 0;
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

void audioconfig_on_set_num_threads(AudioConfig* self, psy_Property* sender)
{ 
	psy_audio_player_stop_threads(self->player); 
}

void audioconfig_on_driver_select(AudioConfig* self, psy_Property* sender)
{
	audioconfig_select_driver(self, self->audioenabled);	
}

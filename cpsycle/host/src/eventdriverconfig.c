/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "eventdriverconfig.h"
/* host */
#include "cmdproperties.h"
#include "resources/resource.h"
/* ui */
#include <uiopendialog.h>
#include <uisavedialog.h>
/* file */
#include <dir.h>
/* platform */
#include "../../detail/portable.h"


/* prototypes */
static void eventdriverconfig_make_input(EventDriverConfig*);
static void eventdriverconfig_make_driver_list(EventDriverConfig*);
static void eventdriverconfig_reset(EventDriverConfig*);
static void eventdriverconfig_load(EventDriverConfig*);
static void eventdriverconfig_save(EventDriverConfig*);
static const char* eventdriverconfig_driver_path(EventDriverConfig*);
static void eventdriverconfig_on_edit_driver_configuration(EventDriverConfig*,
	psy_Property* sender);
static void eventdriverconfig_add_event_driver(EventDriverConfig*);
static void eventdriverconfig_remove_event_driver(EventDriverConfig*);
static void eventdriverconfig_on_add_event_driver(EventDriverConfig*,
	psy_Property* sender);
static void eventdriverconfig_on_remove_event_driver(EventDriverConfig*,
	psy_Property* sender);
static void eventdriverconfig_on_load_config(EventDriverConfig*,
	psy_Property* sender);
static void eventdriverconfig_on_save_config(EventDriverConfig*,
	psy_Property* sender);
static void eventdriverconfig_on_reset_config(EventDriverConfig*,
	psy_Property* sender);
static void eventdriverconfig_on_active_driver(EventDriverConfig*,
	psy_Property* sender);

/* implementation */
void eventdriverconfig_init(EventDriverConfig* self, psy_Property* parent,
	psy_audio_Player* player)
{
	self->player = player;
	self->config = parent;
	self->installeddriver = NULL;
	self->activedrivers = NULL;
	eventdriverconfig_make_input(self);	
}

void eventdriverconfig_dispose(EventDriverConfig* self)
{
	assert(self);
		
	self->player = NULL;
	self->config = NULL;
	self->installeddriver = NULL;
	self->activedrivers = NULL;
}

void eventdriverconfig_make_input(EventDriverConfig* self)
{
	assert(self);

	self->eventinputs = psy_property_set_text(
		psy_property_append_section(self->config, "eventinput"),
		"Event Input");
	psy_property_set_icon(self->eventinputs, IDB_NOTES_LIGHT,
		IDB_NOTES_DARK);
	eventdriverconfig_make_driver_list(self);
	self->eventdriverconfigurations = psy_property_hide(
		psy_property_append_section(self->eventinputs, "configurations"));
}

void eventdriverconfig_make_driver_list(EventDriverConfig* self)
{
	assert(self);

	psy_property_set_text(self->eventinputs, "settingsview.event-input");
	/* change number to set startup driver, if no psycle.ini found */
	self->installeddriver = psy_property_set_text(
		psy_property_append_choice(self->eventinputs, "installeddriver", 0),
		"Input Drivers");
	psy_property_append_str(self->installeddriver, "kbd", "kbd");
#if defined(DIVERSALIS__OS__MICROSOFT)
	psy_property_set_id(psy_property_append_str(self->installeddriver, "mmemidi", ".\\mmemidi.dll"), 200);
	psy_property_append_str(self->installeddriver, "dxjoystick", ".\\dxjoystick.dll");
#endif
	psy_property_connect(
	psy_property_set_id(psy_property_set_text(
		psy_property_append_action(self->eventinputs, "addeventdriver"),
		"Add to active drivers"),
		PROPERTY_ID_ADDEVENTDRIVER),
		self, eventdriverconfig_on_add_event_driver);
	self->activedrivers = psy_property_connect(psy_property_enableappend(psy_property_set_id(
			psy_property_set_text(psy_property_append_choice(
				self->eventinputs, "activedrivers", 0),
				"Active Drivers"), PROPERTY_ID_ACTIVEEVENTDRIVERS)),
			self, eventdriverconfig_on_active_driver);
	psy_property_connect(
		psy_property_set_id(psy_property_set_text(
			psy_property_append_action(self->eventinputs, "removeeventdriver"),
			"Remove active driver"),
			PROPERTY_ID_REMOVEEVENTDRIVER),
		self, eventdriverconfig_on_remove_event_driver);
	self->eventdriverconfigure = psy_property_set_text(
		psy_property_append_section(self->eventinputs,
			"configure"), "Configure");
}

void eventdriverconfig_register_event_drivers(EventDriverConfig* self)
{
	psy_List* p;

	assert(self);
	assert(self->installeddriver);

	for (p = psy_property_begin(self->installeddriver); p != NULL; psy_list_next(&p)) {
		psy_Property* property;
		intptr_t guid;

		property = (psy_Property*)psy_list_entry(p);
		if (psy_property_type(property) == PSY_PROPERTY_TYPE_STRING) {
			guid = psy_audio_eventdrivers_guid(&self->player->eventdrivers,
				psy_property_item_str(property));
			if (guid != -1) {
				psy_audio_eventdrivers_register(&self->player->eventdrivers,
					guid, psy_property_item_str(property));
			}
		}
	}
}

void eventdriverconfig_config(EventDriverConfig* self)
{
	psy_List* p;

	assert(self && self->activedrivers);
		
	for (p = psy_property_begin(self->activedrivers); p != NULL;
			psy_list_next(&p)) {
		psy_Property* property;

		property = (psy_Property*)psy_list_entry(p);
		if (psy_property_type(property) == PSY_PROPERTY_TYPE_INTEGER) {
			psy_EventDriver* driver;

			driver = psy_audio_eventdrivers_loadbyguid(
				&self->player->eventdrivers,
				psy_property_item_int(property));
			if (driver) {
				psy_Property* cmds;

				cmds = cmdproperties_create();
				psy_eventdriver_setcmddef(driver, cmds);
				psy_property_deallocate(cmds);
			}
		}
	}	
}

void eventdriverconfig_reset(EventDriverConfig* self)
{	

	assert(self && self->activedrivers);
	
	if (eventdriverconfig_selected_driver(self)) {
		psy_eventdriver_configure(eventdriverconfig_selected_driver(self), NULL);
		eventdriverconfig_show_active(self,
			psy_property_item_int(self->activedrivers));					
	}
}

void eventdriverconfig_load(EventDriverConfig* self)
{
	psy_EventDriver* eventdriver;

	assert(self);

	eventdriver = eventdriverconfig_selected_driver(self);
	if (eventdriver && psy_eventdriver_configuration(eventdriver)) {
		psy_ui_OpenDialog opendialog;

		psy_ui_opendialog_init_all(&opendialog, 0,
			"Load Event Driver Configuration",
			"Psycle Event Driver Configuration|*.psk", "PSK",
			psy_dir_config());
		if (psy_ui_opendialog_execute(&opendialog)) {			
			psy_Property* local;
			psy_PropertyReader propertyreader;

			local = psy_property_clone(psy_eventdriver_configuration(eventdriver));			
			psy_propertyreader_init(&propertyreader, local,
				psy_path_full(psy_ui_opendialog_path(
					&opendialog)));
			psy_propertyreader_load(&propertyreader);
			psy_propertyreader_dispose(&propertyreader);
			psy_eventdriver_configure(eventdriver, local);
			if (self->activedrivers) {
				eventdriverconfig_show_active(self,
					psy_property_item_int(self->activedrivers));
			}
			/* eventdriverconfig_update_driverconfigure_section(self); */
			psy_property_deallocate(local);
		}
		psy_ui_opendialog_dispose(&opendialog);
	}
}

void eventdriverconfig_save(EventDriverConfig* self)
{
	psy_EventDriver* eventdriver;

	assert(self);

	eventdriver = eventdriverconfig_selected_driver(self);
	if (eventdriver && psy_eventdriver_configuration(eventdriver)) {
		psy_ui_SaveDialog dialog;
		int success;

		psy_ui_savedialog_init_all(&dialog, 0,
			"Save Event Driver Configuration",
			"Psycle Event Driver Configuration|*.psk", "PSK",
			psy_dir_config());
		success = psy_ui_savedialog_execute(&dialog);
		if (success) {			
			psy_PropertyWriter propertywriter;

			psy_propertywriter_init(&propertywriter,
				psy_eventdriver_configuration(eventdriver),
				psy_path_full(psy_ui_savedialog_path(&dialog)));
			psy_propertywriter_save(&propertywriter);
			psy_propertywriter_dispose(&propertywriter);
		}
		psy_ui_savedialog_dispose(&dialog);
	}
}

void eventdriverconfig_update_active(EventDriverConfig* self)
{
	uintptr_t numdrivers;
	uintptr_t i;

	assert(self && self->activedrivers);
				

	psy_property_clear(self->activedrivers);
	numdrivers = psy_audio_player_numeventdrivers(self->player);
	for (i = 0; i < numdrivers; ++i) {
		psy_audio_EventDriverEntry* entry;

		entry = psy_audio_eventdrivers_entry(&self->player->eventdrivers, i);
		if (entry && entry->eventdriver) {
			const psy_EventDriverInfo* driverinfo;

			driverinfo = psy_eventdriver_info(entry->eventdriver);
			if (driverinfo) {
				char key[40];

				psy_snprintf(key, 40, "guid-%d", (int)i);
				psy_property_set_text(
					psy_property_append_int(self->activedrivers, key,
						driverinfo->guid, 0, 0),
					driverinfo->Name);
			}
		}
	}
	psy_property_rebuild(self->activedrivers);
}

void eventdriverconfig_make(EventDriverConfig* self)
{
	uintptr_t numdrivers;
	uintptr_t i;

	assert(self);

	psy_property_clear(self->eventdriverconfigurations);
	numdrivers = psy_audio_player_numeventdrivers(self->player);
	for (i = 0; i < numdrivers; ++i) {
		psy_audio_EventDriverEntry* entry;

		entry = psy_audio_eventdrivers_entry(&self->player->eventdrivers, i);
		if (entry) {
			psy_EventDriver* driver;

			driver = entry->eventdriver;
			if (driver && psy_eventdriver_configuration(driver)) {
				char key[256];
				psy_Property* section;

				psy_snprintf(key, 256, "configuration-%i", i);
				section = psy_property_append_section(
					self->eventdriverconfigurations, key);
				psy_property_append_property(section, psy_property_clone(
					psy_eventdriver_configuration(driver)));
			}
		}
	}
}

void eventdriverconfig_read(EventDriverConfig* self)
{
	assert(self);

	if (self->eventdriverconfigurations) {
		uintptr_t numdrivers;
		psy_List* p;
		uintptr_t i;

		numdrivers = psy_audio_eventdrivers_size(&self->player->eventdrivers);
		for (p = psy_property_begin(self->eventdriverconfigurations), i = 0;
			p != NULL && i != numdrivers; psy_list_next(&p), ++i) {
			psy_Property* configuration;

			configuration = (psy_Property*)psy_list_entry(p);
			if (psy_property_empty(configuration)) {
				continue;
			}
			configuration = (psy_Property*)psy_list_entry(
				psy_property_begin(configuration));
			if (!psy_property_empty(configuration)) {
				intptr_t guid;

				guid = psy_property_at_int(configuration, "guid", -1);
				if (guid != -1) {
					psy_EventDriver* driver;
					const psy_EventDriverInfo* driverinfo;

					driver = psy_audio_eventdrivers_driver(&self->player->eventdrivers, i);
					if (driver) {
						driverinfo = psy_eventdriver_info(driver);
						if (!driverinfo) {
							continue;
						}
						if (driverinfo->guid == guid) {
							psy_eventdriver_configure(driver, configuration);
						}
					}
				}
			}
		}
	}
}

intptr_t eventdriverconfig_current(EventDriverConfig* self)
{
	assert(self && self->activedrivers);
	
	return psy_property_item_int(self->activedrivers);	
}

const char* eventdriverconfig_driver_path(EventDriverConfig* self)
{			
	psy_Property* choice;

	assert(self && self->activedrivers);

	choice = psy_property_at_choice(self->activedrivers);
	if (choice) {
		return psy_property_item_str(choice);
	}	
	return NULL;
}

void eventdriverconfig_show_active(EventDriverConfig* self, intptr_t driverid)
{
	psy_EventDriver* eventdriver;

	assert(self && self->eventdriverconfigure);
	
	psy_property_clear(self->eventdriverconfigure);
	eventdriver = psy_audio_player_eventdriver(self->player, driverid);
	if (eventdriver && psy_eventdriver_configuration(eventdriver)) {
		psy_property_connect(psy_property_set_id(psy_property_set_text(
			psy_property_append_action(self->eventdriverconfigure, "defaults"),
			"Defaults"), PROPERTY_ID_EVENTDRIVERCONFIGDEFAULTS),
			self, eventdriverconfig_on_reset_config);
		psy_property_connect(psy_property_set_id(psy_property_set_text(
			psy_property_append_action(self->eventdriverconfigure, "load"),
			"Load"), PROPERTY_ID_EVENTDRIVERCONFIGLOAD),
			self, eventdriverconfig_on_load_config);
		psy_property_connect(psy_property_set_id(psy_property_set_text(
			psy_property_append_action(self->eventdriverconfigure, "save"),
			"Save"), PROPERTY_ID_EVENTDRIVERCONFIGKEYMAPSAVE),
			self, eventdriverconfig_on_save_config);
		psy_property_prevent_save(self->eventdriverconfigure);
		psy_property_append_property(self->eventdriverconfigure,
			psy_property_clone(psy_eventdriver_configuration(eventdriver)));
		psy_property_connect_children(self->eventdriverconfigure, TRUE,
			self, eventdriverconfig_on_edit_driver_configuration);
	}
	psy_property_rebuild(self->eventdriverconfigure);
}

psy_EventDriver* eventdriverconfig_selected_driver(EventDriverConfig* self)
{	
	assert(self && self->activedrivers);

	return psy_audio_player_eventdriver(self->player,
		psy_property_item_int(self->activedrivers));
}

void eventdriverconfig_on_edit_driver_configuration(EventDriverConfig* self,
	psy_Property* sender)
{
	psy_Property* driversection;
	psy_EventDriver* driver;

	driver = psy_audio_player_eventdriver(self->player,
		psy_property_item_int(self->activedrivers));
	driversection = psy_property_find(
		self->eventdriverconfigure,
		psy_property_key(psy_eventdriver_configuration(driver)),
		PSY_PROPERTY_TYPE_NONE);
	if (driversection) {
		psy_audio_player_restart_event_driver(self->player,
			psy_property_item_int(self->activedrivers),
			driversection);
	}
}

void eventdriverconfig_add_event_driver(EventDriverConfig* self)
{
	psy_Property* installeddriver;

	assert(self);

	installeddriver = psy_property_at(self->eventinputs,
		"installeddriver", PSY_PROPERTY_TYPE_CHOICE);
	if (installeddriver) {
		psy_Property* choice;

		choice = psy_property_at_choice(installeddriver);
		if (choice) {
			psy_EventDriver* driver;
			psy_Property* activedrivers;

			driver = psy_audio_player_loadeventdriver(self->player,
				psy_property_item_str(choice));
			if (driver) {
				psy_eventdriver_setcmddef(driver,
					self->player->eventdrivers.cmds);
			}
			eventdriverconfig_update_active(self);
			activedrivers = psy_property_at(self->eventinputs,
				"activedrivers", PSY_PROPERTY_TYPE_CHOICE);
			if (activedrivers) {
				psy_property_set_item_int(activedrivers,
					psy_audio_player_numeventdrivers(self->player) - 1);
				eventdriverconfig_show_active(self,
					psy_property_item_int(activedrivers));
			}
		}
	}
}

void eventdriverconfig_remove_event_driver(EventDriverConfig* self)
{
	assert(self);

	psy_audio_player_remove_event_driver(self->player,
		psy_property_item_int(self->activedrivers));
	eventdriverconfig_update_active(self);
	if (psy_property_item_int(self->activedrivers) > 0) {
		psy_property_set_item_int(self->activedrivers,
			psy_property_item_int(self->activedrivers) - 1);
	}
	eventdriverconfig_show_active(self,
		psy_property_item_int(self->activedrivers));
}

/* events */
void eventdriverconfig_on_add_event_driver(EventDriverConfig* self,
	psy_Property* sender)
{
	assert(self);

	eventdriverconfig_add_event_driver(self);
}

void eventdriverconfig_on_remove_event_driver(EventDriverConfig* self,
	psy_Property* sender)
{
	assert(self);

	eventdriverconfig_remove_event_driver(self);
}

void eventdriverconfig_on_reset_config(EventDriverConfig* self,
	psy_Property* sender)
{
	assert(self);

	eventdriverconfig_reset(self);
}

void eventdriverconfig_on_load_config(EventDriverConfig* self,
	psy_Property* sender)
{
	assert(self);

	eventdriverconfig_load(self);
}

void eventdriverconfig_on_save_config(EventDriverConfig* self,
	psy_Property* sender)
{
	assert(self);

	eventdriverconfig_save(self);
}

void eventdriverconfig_on_active_driver(EventDriverConfig* self,
	psy_Property* sender)
{
	assert(self);

	eventdriverconfig_show_active(self, psy_property_item_int(sender));
}

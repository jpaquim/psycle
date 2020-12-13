// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "eventdriverconfig.h"
// host
#include "cmdproperties.h"
// ui
#include <uiapp.h>
#include <uiopendialog.h>
#include <uisavedialog.h>
// file
#include <dir.h>
// std
#include <stdio.h>
#include <string.h>
// platform
#include "../../detail/portable.h"

void eventdriverconfig_init(EventDriverConfig* self, psy_Property* parent,
	psy_audio_Player* player)
{
	self->player = player;
	self->config = parent;
	eventdriverconfig_makeeventinput(self);
}

void eventdriverconfig_makeeventinput(EventDriverConfig* self)
{
	assert(self);

	self->eventinputs = psy_property_settext(
		psy_property_append_section(self->config, "eventinput"),
		"Event Input");
	eventdriverconfig_makeeventdriverlist(self);
	self->eventdriverconfigurations = psy_property_sethint(
		psy_property_append_section(self->eventinputs, "configurations"),
		PSY_PROPERTY_HINT_HIDE);
}

void eventdriverconfig_makeeventdriverlist(EventDriverConfig* self)
{
	psy_Property* installed;

	assert(self);

	psy_property_settext(self->eventinputs, "settingsview.event-input");
	// change number to set startup driver, if no psycle.ini found
	installed = psy_property_settext(
		psy_property_append_choice(self->eventinputs, "installeddriver", 0),
		"Input Drivers");
	psy_property_append_string(installed, "kbd", "kbd");
#if defined(DIVERSALIS__OS__MICROSOFT)
	psy_property_append_string(installed, "mmemidi", ".\\mmemidi.dll");
	psy_property_append_string(installed, "dxjoystick", ".\\dxjoystick.dll");
#endif
	psy_property_setid(psy_property_settext(
		psy_property_append_action(self->eventinputs, "addeventdriver"),
		"Add to active drivers"),
		PROPERTY_ID_ADDEVENTDRIVER);
	psy_property_enableappend(psy_property_setid(psy_property_settext(
		psy_property_append_choice(self->eventinputs, "activedrivers", 0),
		"Active Drivers"), PROPERTY_ID_ACTIVEEVENTDRIVERS));
	psy_property_setid(psy_property_settext(
		psy_property_append_action(self->eventinputs, "removeeventdriver"),
		"Remove active driver"),
		PROPERTY_ID_REMOVEEVENTDRIVER);
	self->eventdriverconfigure = psy_property_settext(
		psy_property_append_section(self->eventinputs,
			"configure"),
		"Configure");
}

void eventdriverconfig_registereventdrivers(EventDriverConfig* self)
{
	psy_Property* drivers;

	assert(self);

	drivers = psy_property_at(self->eventinputs, "installeddriver",
		PSY_PROPERTY_TYPE_NONE);
	if (drivers) {
		psy_List* p;

		for (p = psy_property_children(drivers); p != NULL; psy_list_next(&p)) {
			psy_Property* property;
			int guid;

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
}

void eventdriverconfig_configeventdrivers(EventDriverConfig* self)
{
	psy_Property* activedrivers;

	assert(self);

	activedrivers = psy_property_at(self->eventinputs, "activedrivers",
		PSY_PROPERTY_TYPE_NONE);
	if (activedrivers) {
		psy_List* p;

		for (p = psy_property_children(activedrivers); p != NULL;
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
}

void eventdriverconfig_reseteventdriverconfiguration(EventDriverConfig* self)
{
	psy_EventDriver* eventdriver;

	assert(self);

	eventdriver = eventdriverconfig_selectedeventdriver(self);
	if (eventdriver && psy_eventdriver_configuration(eventdriver)) {
		psy_Property* activedrivers;

		psy_eventdriver_configure(eventdriver, NULL);
		activedrivers = psy_property_at(self->eventinputs, "activedrivers",
			PSY_PROPERTY_TYPE_CHOICE);
		if (activedrivers) {
			eventdriverconfig_showactiveeventdriverconfig(self,
				psy_property_item_int(activedrivers));
			//eventdriverconfig_update_driverconfigure_section(self);
		}
	}
}

void eventdriverconfig_loadeventdriverconfiguration(EventDriverConfig* self)
{
	psy_EventDriver* eventdriver;

	assert(self);

	eventdriver = eventdriverconfig_selectedeventdriver(self);
	if (eventdriver && psy_eventdriver_configuration(eventdriver)) {
		psy_ui_OpenDialog opendialog;

		psy_ui_opendialog_init_all(&opendialog, 0,
			"Load Event Driver Configuration",
			"Psycle Event Driver Configuration|*.psk", "PSK",
			psy_dir_config());
		if (psy_ui_opendialog_execute(&opendialog)) {
			psy_Property* activedrivers;
			psy_Property* local;

			local = psy_property_clone(psy_eventdriver_configuration(eventdriver));
			propertiesio_load(local, psy_ui_opendialog_filename(&opendialog), FALSE);
			psy_eventdriver_configure(eventdriver, local);
			activedrivers = psy_property_at(self->eventinputs, "activedrivers",
				PSY_PROPERTY_TYPE_CHOICE);
			if (activedrivers) {
				eventdriverconfig_showactiveeventdriverconfig(self,
					psy_property_item_int(activedrivers));
			}
			// eventdriverconfig_update_driverconfigure_section(self);
			psy_property_deallocate(local);
		}
		psy_ui_opendialog_dispose(&opendialog);
	}
}

void eventdriverconfig_saveeventdriverconfiguration(EventDriverConfig* self)
{
	psy_EventDriver* eventdriver;

	assert(self);

	eventdriver = eventdriverconfig_selectedeventdriver(self);
	if (eventdriver && psy_eventdriver_configuration(eventdriver)) {
		psy_ui_SaveDialog dialog;
		int success;

		psy_ui_savedialog_init_all(&dialog, 0,
			"Save Event Driver Configuration",
			"Psycle Event Driver Configuration|*.psk", "PSK",
			psy_dir_config());
		success = psy_ui_savedialog_execute(&dialog);
		if (success) {
			propertiesio_save(psy_eventdriver_configuration(eventdriver),
				psy_ui_savedialog_filename(&dialog));
		}
		psy_ui_savedialog_dispose(&dialog);
	}
}

void eventdriverconfig_updateactiveeventdriverlist(EventDriverConfig* self)
{
	psy_Property* drivers;

	assert(self);

	drivers = psy_property_at(self->eventinputs, "activedrivers",
		PSY_PROPERTY_TYPE_NONE);
	if (drivers) {
		uintptr_t numdrivers;
		uintptr_t i;

		psy_property_clear(drivers);
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
					psy_property_settext(
						psy_property_append_int(drivers, key,
							driverinfo->guid, 0, 0),
						driverinfo->Name);
				}
			}
		}
	}
}

void eventdriverconfig_makeeventdriverconfigurations(EventDriverConfig* self)
{
	int numdrivers;
	int i;

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

void eventdriverconfig_readeventdriverconfigurations(EventDriverConfig* self)
{
	assert(self);

	if (self->eventdriverconfigurations) {
		uintptr_t numdrivers;
		psy_List* p;
		uintptr_t i;

		numdrivers = psy_audio_eventdrivers_size(&self->player->eventdrivers);
		for (p = psy_property_children(self->eventdriverconfigurations), i = 0;
			p != NULL && i != numdrivers; psy_list_next(&p), ++i) {
			psy_Property* configuration;

			configuration = (psy_Property*)psy_list_entry(p);
			if (psy_property_empty(configuration)) {
				continue;
			}
			configuration = (psy_Property*)psy_list_entry(
				psy_property_children(configuration));
			if (!psy_property_empty(configuration)) {
				int guid;

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

int eventdriverconfig_curreventdriverconfiguration(EventDriverConfig* self)
{
	psy_Property* p;

	assert(self);

	if (p = psy_property_at(self->eventinputs, "activedrivers",
		PSY_PROPERTY_TYPE_NONE)) {
		return psy_property_item_int(p);
	}
	return 0;
}

const char* eventdriverconfig_eventdriverpath(EventDriverConfig* self)
{
	psy_Property* activedrivers;

	assert(self);

	activedrivers = psy_property_at(self->eventinputs, "activedrivers",
		PSY_PROPERTY_TYPE_NONE);
	if (activedrivers) {
		psy_Property* choice;

		choice = psy_property_at_choice(activedrivers);
		if (choice) {
			return psy_property_item_str(choice);
		}
	}
	return NULL;
}

void eventdriverconfig_showactiveeventdriverconfig(EventDriverConfig* self, int driverid)
{
	psy_EventDriver* eventdriver;

	assert(self && self->eventdriverconfigure);

	psy_property_clear(self->eventdriverconfigure);
	psy_property_setid(psy_property_settext(
		psy_property_append_action(self->eventdriverconfigure, "defaults"),
		"Defaults"), PROPERTY_ID_EVENTDRIVERCONFIGDEFAULTS);
	psy_property_setid(psy_property_settext(
		psy_property_append_action(self->eventdriverconfigure, "load"),
		"Load"), PROPERTY_ID_EVENTDRIVERCONFIGLOAD);
	psy_property_setid(psy_property_settext(
		psy_property_append_action(self->eventdriverconfigure, "save"),
		"Save"), PROPERTY_ID_EVENTDRIVERCONFIGKEYMAPSAVE);
	eventdriver = psy_audio_player_eventdriver(self->player, driverid);
	if (eventdriver) {
		psy_property_preventsave(self->eventdriverconfigure);
		psy_property_append_property(self->eventdriverconfigure,
			psy_property_clone(psy_eventdriver_configuration(eventdriver)));
	}
}

psy_EventDriver* eventdriverconfig_selectedeventdriver(EventDriverConfig* self)
{
	psy_EventDriver* rv;
	psy_Property* activedrivers;

	rv = NULL;
	activedrivers = psy_property_at(self->eventinputs, "activedrivers",
		PSY_PROPERTY_TYPE_CHOICE);
	if (activedrivers) {
		rv = psy_audio_player_eventdriver(self->player,
			psy_property_item_int(activedrivers));
	}
	return rv;
}

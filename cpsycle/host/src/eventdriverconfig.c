/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "eventdriverconfig.h"
/* host */
#include "cmdproperties.h"
/* ui */
#include <uiopendialog.h>
#include <uisavedialog.h>
/* file */
#include <dir.h>
/* platform */
#include "../../detail/portable.h"

void eventdriverconfig_init(EventDriverConfig* self, psy_Property* parent,
	psy_audio_Player* player)
{
	self->player = player;
	self->config = parent;
	self->installeddriver = NULL;
	self->activedrivers = NULL;
	eventdriverconfig_makeeventinput(self);
	psy_signal_init(&self->signal_changed);
}

void eventdriverconfig_dispose(EventDriverConfig* self)
{
	assert(self);
	
	psy_signal_dispose(&self->signal_changed);
	self->player = NULL;
	self->config = NULL;
	self->installeddriver = NULL;
	self->activedrivers = NULL;
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
	assert(self);

	psy_property_settext(self->eventinputs, "settingsview.event-input");
	/* change number to set startup driver, if no psycle.ini found */
	self->installeddriver = psy_property_settext(
		psy_property_append_choice(self->eventinputs, "installeddriver", 0),
		"Input Drivers");
	psy_property_append_str(self->installeddriver, "kbd", "kbd");
#if defined(DIVERSALIS__OS__MICROSOFT)
	psy_property_append_str(self->installeddriver, "mmemidi", ".\\mmemidi.dll");
	psy_property_append_str(self->installeddriver, "dxjoystick", ".\\dxjoystick.dll");
#endif
	psy_property_setid(psy_property_settext(
		psy_property_append_action(self->eventinputs, "addeventdriver"),
		"Add to active drivers"),
		PROPERTY_ID_ADDEVENTDRIVER);
	self->activedrivers = psy_property_enableappend(psy_property_setid(
		psy_property_settext(psy_property_append_choice(
			self->eventinputs, "activedrivers", 0),
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
	psy_List* p;

	assert(self && self->installeddriver);	

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
	
	if (eventdriverconfig_selectedeventdriver(self)) {
		psy_eventdriver_configure(eventdriverconfig_selectedeventdriver(self), NULL);
		eventdriverconfig_show_active(self,
			psy_property_item_int(self->activedrivers));					
	}
}

void eventdriverconfig_load(EventDriverConfig* self)
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
				psy_property_settext(
					psy_property_append_int(self->activedrivers, key,
						driverinfo->guid, 0, 0),
					driverinfo->Name);
			}
		}
	}
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

const char* eventdriverconfig_eventdriverpath(EventDriverConfig* self)
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
		psy_property_setid(psy_property_settext(
			psy_property_append_action(self->eventdriverconfigure, "defaults"),
			"Defaults"), PROPERTY_ID_EVENTDRIVERCONFIGDEFAULTS);
		psy_property_setid(psy_property_settext(
			psy_property_append_action(self->eventdriverconfigure, "load"),
			"Load"), PROPERTY_ID_EVENTDRIVERCONFIGLOAD);
		psy_property_setid(psy_property_settext(
			psy_property_append_action(self->eventdriverconfigure, "save"),
			"Save"), PROPERTY_ID_EVENTDRIVERCONFIGKEYMAPSAVE);		
		psy_property_preventsave(self->eventdriverconfigure);
		psy_property_append_property(self->eventdriverconfigure,
			psy_property_clone(psy_eventdriver_configuration(eventdriver)));		
	}
}

psy_EventDriver* eventdriverconfig_selectedeventdriver(EventDriverConfig* self)
{	
	assert(self && self->activedrivers);

	return psy_audio_player_eventdriver(self->player,
		psy_property_item_int(self->activedrivers));
}
// events
bool eventdriverconfig_onchanged(EventDriverConfig* self, psy_Property*
	property)
{
	assert(self);

	psy_signal_emit(&self->signal_changed, self, 1, property);
	return TRUE;
}

bool eventdriverconfig_hasproperty(const EventDriverConfig* self,
	psy_Property* property)
{
	assert(self && self->eventinputs);

	return psy_property_in_section(property, self->eventinputs);
}

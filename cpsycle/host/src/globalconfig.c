/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "globalconfig.h"

/* host */
#include "pluginscanthread.h"
#include "resources/resource.h"
/* ui */
#include <uicomponent.h> /* Translator */
/* audio */
#include <player.h>


/* prototypes */
static void globalconfig_make(GlobalConfig*, psy_Property*);
static void globalconfig_on_enable_audio(GlobalConfig*, psy_Property* sender);
static void globalconfig_on_plugin_scan(GlobalConfig*, psy_Property* sender);

/* implementation */
void globalconfig_init(GlobalConfig* self, psy_Property* parent,
	psy_audio_Player* player, struct PluginScanThread* pluginscanthread)
{
	assert(self);
	assert(parent);
	assert(player);
	assert(pluginscanthread);

	self->parent = parent;
	self->player = player;
	self->pluginscanthread = pluginscanthread;
	globalconfig_make(self, parent);	
}

void globalconfig_dispose(GlobalConfig* self)
{
	assert(self);

	languageconfig_dispose(&self->language);	
}

void globalconfig_make(GlobalConfig* self, psy_Property* parent)
{	
	assert(self);

	self->global = psy_property_set_text(
		psy_property_append_section(parent, "global"),
		"settingsview.global.configuration");
	psy_property_set_icon(self->global, IDB_SETTINGS_LIGHT, IDB_SETTINGS_DARK);
	psy_property_connect(psy_property_set_id(psy_property_set_text(
		psy_property_append_bool(self->global, "enableaudio", TRUE),
		"settingsview.global.enable-audio"),
		PROPERTY_ID_ENABLEAUDIO),
		self, globalconfig_on_enable_audio);
	psy_property_connect(psy_property_set_id(psy_property_set_text(
		psy_property_append_action(self->global, "regenerateplugincache"),
		"settingsview.global.regenerate-plugincache"),
		PROPERTY_ID_REGENERATEPLUGINCACHE),
		self, globalconfig_on_plugin_scan);
	languageconfig_init(&self->language, self->global, psy_ui_translator());
	self->import_config = psy_property_set_id(
		psy_property_set_text(psy_property_append_action(self->global,
			"importconfig"), "settingsview.global.importconfig"),
		PROPERTY_ID_IMPORTCONFIG);

}

bool globalconfig_audio_enabled(const GlobalConfig* self)
{
	assert(self);

	return psy_property_at_bool(self->global, "enableaudio", TRUE);
}

void globalconfig_enable_audio(GlobalConfig* self, bool on)
{
	assert(self);
	
	psy_property_set_bool(self->global, "enableaudio", on);	
}

void globalconfig_on_enable_audio(GlobalConfig* self, psy_Property* sender)
{
	assert(self);	
	
	if (globalconfig_audio_enabled(self)) {
		psy_audio_player_enable_audio(self->player);
	} else {
		psy_audio_player_disable_audio(self->player);
	}
}

void globalconfig_on_plugin_scan(GlobalConfig* self, psy_Property* sender)
{	
	pluginscanthread_start(self->pluginscanthread);
}

bool globalconfig_hasproperty(const GlobalConfig* self, psy_Property* property)
{
	assert(self);

	return psy_property_in_section(property, self->global);
}

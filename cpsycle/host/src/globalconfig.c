/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "globalconfig.h"

/* ui */
#include <uicomponent.h> /* Translator */
#include <uiapp.h> /* Styles */

static void globalconfig_make(GlobalConfig*, psy_Property*);

void globalconfig_init(GlobalConfig* self, psy_Property* parent)
{
	assert(self && parent);

	self->parent = parent;
	psy_signal_init(&self->signal_changed);
	globalconfig_make(self, parent);	
}

void globalconfig_dispose(GlobalConfig* self)
{
	assert(self);

	languageconfig_dispose(&self->language);
	psy_signal_dispose(&self->signal_changed);
}

void globalconfig_make(GlobalConfig* self, psy_Property* parent)
{
	assert(self);

	self->global = psy_property_settext(
		psy_property_append_section(parent, "global"),
		"settingsview.global.configuration");
	psy_property_set_id(psy_property_settext(
		psy_property_append_bool(self->global, "enableaudio", TRUE),
		"settingsview.global.enable-audio"),
		PROPERTY_ID_ENABLEAUDIO);
	psy_property_set_id(psy_property_settext(
		psy_property_append_action(self->global, "regenerateplugincache"),
		"settingsview.global.regenerate-plugincache"),
		PROPERTY_ID_REGENERATEPLUGINCACHE);
	languageconfig_init(&self->language, self->global, psy_ui_translator());
	psy_property_set_id(psy_property_settext(
		psy_property_append_action(self->global, "importconfig"),
		"settingsview.global.importconfig"),
		PROPERTY_ID_IMPORTCONFIG);
	
}

/* events */
uintptr_t globalconfig_on_changed(GlobalConfig* self, psy_Property*
	property)
{
	languageconfig_on_changed(&self->language, property);
	psy_signal_emit(&self->signal_changed, self, 1, property);
	return psy_INDEX_INVALID;
}

bool globalconfig_hasproperty(const GlobalConfig* self,
	psy_Property* property)
{
	assert(self && self->general);

	return psy_property_in_section(property, self->global);
}

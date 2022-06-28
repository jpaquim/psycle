/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "visualconfig.h"

/* ui */
#include <uicomponent.h> /* Translator */
#include <uiapp.h> /* Styles */

static void visualconfig_make(VisualConfig*, psy_Property*);

void visualconfig_init(VisualConfig* self, psy_Property* parent)
{
	assert(self && parent);

	self->parent = parent;
	psy_signal_init(&self->signal_changed);
	visualconfig_make(self, parent);
}

void visualconfig_dispose(VisualConfig* self)
{
	assert(self);

	patternviewconfig_dispose(&self->patview);
	machineviewconfig_dispose(&self->macview);
	machineparamconfig_dispose(&self->macparam);
	psy_signal_dispose(&self->signal_changed);
}

void visualconfig_make(VisualConfig* self, psy_Property* parent)
{
	assert(self);

	self->visual = psy_property_settext(
		psy_property_append_section(parent, "visual"),
		"settingsview.visual.visual");
	psy_property_set_id(psy_property_settext(
		psy_property_append_action(self->visual, "loadskin"),
		"settingsview.visual.load-skin"),
		PROPERTY_ID_LOADSKIN);
	psy_property_set_id(psy_property_settext(
		psy_property_append_action(self->visual, "defaultskin"),
		"settingsview.visual.default-skin"),
		PROPERTY_ID_DEFAULTSKIN);
	self->defaultfont = psy_property_set_id(psy_property_settext(
		psy_property_append_font(self->visual, "defaultfont",
			PSYCLE_DEFAULT_FONT),
		"settingsview.visual.default-font"),
		PROPERTY_ID_DEFAULTFONT);
	self->apptheme = psy_property_set_id(psy_property_settext(
		psy_property_append_choice(self->visual,
			"apptheme", 1),
		"settingsview.visual.apptheme"),
		PROPERTY_ID_APPTHEME);
	psy_property_settext(
		psy_property_append_int(self->apptheme, "light", psy_ui_LIGHTTHEME, 0, 2),
		"settingsview.visual.light");
	psy_property_settext(
		psy_property_append_int(self->apptheme, "dark", psy_ui_DARKTHEME, 0, 2),
		"settingsview.visual.dark");
	psy_property_settext(
		psy_property_append_int(self->apptheme, "win98", psy_ui_WIN98THEME, 0, 2),
		"Windows 98");

	patternviewconfig_init(&self->patview, self->visual,
		PSYCLE_SKINS_DEFAULT_DIR);
	machineviewconfig_init(&self->macview, self->visual);
	machineparamconfig_init(&self->macparam, self->visual);
}

/* events */
uintptr_t visualconfig_on_changed(VisualConfig* self, psy_Property*
	property)
{	
	psy_signal_emit(&self->signal_changed, self, 1, property);
	return psy_INDEX_INVALID;
}

bool visualconfig_hasproperty(const VisualConfig* self,
	psy_Property* property)
{
	assert(self);

	assert(self->visual);

	return psy_property_in_section(property, self->visual);
}

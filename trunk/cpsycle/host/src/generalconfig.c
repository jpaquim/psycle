// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "generalconfig.h"

static void generalconfig_make(GeneralConfig*, psy_Property*);

void generalconfig_init(GeneralConfig* self, psy_Property* parent)
{
	assert(self && parent);

	self->parent = parent;
	generalconfig_make(self, parent);
	psy_signal_init(&self->signal_changed);
}

void generalconfig_dispose(GeneralConfig* self)
{
	assert(self);

	psy_signal_dispose(&self->signal_changed);
}

void generalconfig_make(GeneralConfig* self, psy_Property* parent)
{
	assert(self);

	self->general = psy_property_settext(
		psy_property_append_section(parent, "general"),
		"settingsview.general");
	psy_property_sethint(psy_property_settext(
		psy_property_append_string(self->general, "version", "alpha"),
		"settingsview.version"),
		PSY_PROPERTY_HINT_HIDE);	
	psy_property_settext(
		psy_property_append_bool(self->general, "showaboutatstart", TRUE),
		"settingsview.show-about-at-startup");
	psy_property_settext(
		psy_property_append_bool(self->general, "showsonginfoonload", TRUE),
		"settingsview.show-song-info-on-load");
	psy_property_settext(
		psy_property_append_bool(self->general, "showmaximizedatstart", TRUE),
		"settingsview.show-maximized-at-startup");
	psy_property_setid(psy_property_settext(
		psy_property_append_bool(self->general, "showstepsequencer", TRUE),
		"settingsview.show-sequencestepbar"),
		PROPERTY_ID_SHOWSTEPSEQUENCER);
	psy_property_settext(
		psy_property_append_bool(self->general, "saverecentsongs", TRUE),
		"settingsview.save-recent-songs");
	psy_property_settext(
		psy_property_append_bool(self->general, "playsongafterload", TRUE),
		"settingsview.play-song-after-load");
	psy_property_settext(
		psy_property_append_bool(self->general, "showpatternnames", FALSE),
		"sequencerview.show-pattern-names");
}

bool generalconfig_showsonginfoonload(const GeneralConfig* self)
{
	assert(self);

	return psy_property_at_bool(self->general, "showsonginfoonload", TRUE);
}

bool generalconfig_showaboutatstart(const GeneralConfig* self)
{
	assert(self);

	return psy_property_at_bool(self->general, "showaboutatstart", TRUE);
}

bool generalconfig_showmaximizedatstart(const GeneralConfig* self)
{
	assert(self);

	return psy_property_at_bool(self->general, "showmaximizedatstart", TRUE);
}

bool generalconfig_saverecentsongs(const GeneralConfig* self)
{
	assert(self);

	return psy_property_at_bool(self->general, "saverecentsongs", TRUE);
}

bool generalconfig_playsongafterload(const GeneralConfig* self)
{
	assert(self);

	return psy_property_at_bool(self->general, "playsongafterload", TRUE);
}

bool generalconfig_showingpatternnames(const GeneralConfig* self)
{
	assert(self);

	return psy_property_at_bool(self->general, "showpatternnames", TRUE);
}

bool generalconfig_showstepsequencer(const GeneralConfig* self)
{
	assert(self);

	return psy_property_at_bool(self->general, "showstepsequencer", FALSE);
}
// events
bool generalconfig_onchanged(GeneralConfig* self, psy_Property*
	property)
{
	psy_signal_emit(&self->signal_changed, self, 1, property);
	return TRUE;
}

bool generalconfig_hasproperty(const GeneralConfig* self,
	psy_Property* property)
{
	assert(self && self->general);

	return psy_property_insection(property, self->general);
}

void generalconfig_showpatternnames(GeneralConfig* self)
{
	assert(self);

	psy_signal_emit(&self->signal_changed, self, 1,
		psy_property_set_bool(self->general, "showpatternnames", TRUE));
}

void generalconfig_showpatternids(GeneralConfig* self)
{
	assert(self);

	psy_signal_emit(&self->signal_changed, self, 1,
		psy_property_set_bool(self->general, "showpatternnames", FALSE));
}

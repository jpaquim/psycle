// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

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
		"settingsview.general.general");
	psy_property_sethint(psy_property_settext(
		psy_property_append_str(self->general, "version", "alpha"),
		"settingsview.general.version"),
		PSY_PROPERTY_HINT_HIDE);	
	psy_property_settext(
		psy_property_append_bool(self->general, "showaboutatstart", TRUE),
		"settingsview.general.show-about-at-startup");
	psy_property_settext(
		psy_property_append_bool(self->general, "showsonginfoonload", TRUE),
		"settingsview.general.show-song-info-on-load");
	psy_property_settext(
		psy_property_append_bool(self->general, "showmaximizedatstart", TRUE),
		"settingsview.general.show-maximized-at-startup");
	psy_property_setid(psy_property_settext(
		psy_property_append_bool(self->general, "showsequenceedit", FALSE),
		"settingsview.general.show-sequenceedit"),
		PROPERTY_ID_SHOWSEQUENCEEDIT);
	psy_property_setid(psy_property_settext(
		psy_property_append_bool(self->general, "showstepsequencer", TRUE),
		"settingsview.general.show-sequencestepbar"),
		PROPERTY_ID_SHOWSTEPSEQUENCER);
	psy_property_setid(psy_property_settext(
		psy_property_append_bool(self->general, "showplaylist", FALSE),
		"settingsview.general.show-playlist"),
		PROPERTY_ID_SHOWPLAYLIST);
	psy_property_settext(
		psy_property_append_bool(self->general, "saverecentsongs", TRUE),
		"settingsview.general.save-recent-songs");
	psy_property_settext(
		psy_property_append_bool(self->general, "playsongafterload", TRUE),
		"settingsview.general.play-song-after-load");
	psy_property_settext(
		psy_property_append_bool(self->general, "showpatternnames", FALSE),
		"settingsview.general.show-pattern-names");
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

bool generalconfig_showsequenceedit(const GeneralConfig* self)
{
	assert(self);

	return psy_property_at_bool(self->general, "showsequenceedit", FALSE);
}

void generalconfig_setsequenceeditshowstate(GeneralConfig* self, bool state)
{
	assert(self);

	psy_property_set_bool(self->general, "showsequenceedit", state);
}

bool generalconfig_showstepsequencer(const GeneralConfig* self)
{
	assert(self);

	return psy_property_at_bool(self->general, "showstepsequencer", FALSE);
}

bool generalconfig_showplaylist(const GeneralConfig* self)
{
	assert(self);

	return psy_property_at_bool(self->general, "showplaylist", TRUE);
}

void generalconfig_setplaylistshowstate(GeneralConfig* self, bool state)
{
	assert(self);

	psy_property_set_bool(self->general, "showplaylist", state);
}

void generalconfig_setstepsequencershowstate(GeneralConfig* self, bool state)
{
	assert(self);

	psy_property_set_bool(self->general, "showstepsequencer", state);
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

bool generalconfig_showminiview(const GeneralConfig* self)
{
	assert(self);

	return psy_property_at_bool(self->general, "showminiview", FALSE);
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
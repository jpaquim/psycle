/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "compatconfig.h"

static void compatconfig_make(CompatConfig*, psy_Property*);

void compatconfig_init(CompatConfig* self, psy_Property* parent,
	psy_audio_MachineFactory* machinefactory)
{
	assert(self && parent);

	self->parent = parent;
	self->machinefactory = machinefactory;
	compatconfig_make(self, parent);
	psy_signal_init(&self->signal_changed);
}

void compatconfig_dispose(CompatConfig* self)
{
	assert(self);

	psy_signal_dispose(&self->signal_changed);
}

void compatconfig_make(CompatConfig* self, psy_Property* parent)
{
	assert(self);

	self->compatibility = psy_property_settext(
		psy_property_append_section(parent,
		"compatibility"),
		"settingsview.compatibility");	
	psy_property_settext(
		psy_property_append_bool(self->compatibility, "loadnewgamefxblitz", 0),
		"newmachine.jme-version-unknown");
}

void compatconfig_setloadnewblitz(CompatConfig* self, bool mode)
{
	assert(self);

	psy_property_set_bool(self->compatibility, "loadnewgamefxblitz",
		mode != FALSE);
	if (mode != FALSE) {
		psy_audio_machinefactory_loadnewgamefxandblitzifversionunknown(
			self->machinefactory);
	} else {
		psy_audio_machinefactory_loadoldgamefxandblitzifversionunknown(
			self->machinefactory);
	}
}

bool compatconfig_loadnewblitz(const CompatConfig* self)
{
	assert(self);

	return psy_property_at_bool(self->compatibility, "loadnewgamefxblitz", 0);
}
/* events */
bool compatconfig_onchanged(CompatConfig* self, psy_Property*
	property)
{
	psy_signal_emit(&self->signal_changed, self, 1, property);
	return TRUE;
}

bool compatconfig_hasproperty(const CompatConfig* self, psy_Property* property)
{
	assert(self && self->compatibility);

	return psy_property_insection(property, self->compatibility);
}

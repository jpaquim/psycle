/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "compatconfig.h"
/* host */
#include "resources/resource.h"


/* prototypes */
static void compatconfig_make(CompatConfig*, psy_Property*);
static void compatconfig_on_blitz_compatibility(CompatConfig*,
	psy_Property* sender);

/* implementation */
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

	self->compatibility = psy_property_set_text(
		psy_property_append_section(parent,
		"compatibility"),
		"settingsview.compatibility");
	psy_property_set_icon(self->compatibility, IDB_TRAIL_SIGN_LIGHT,
		IDB_TRAIL_SIGN_DARK);
	psy_property_connect(psy_property_set_text(
		psy_property_append_bool(self->compatibility, "loadnewgamefxblitz", 0),
		"newmachine.jme-version-unknown"),
		self, compatconfig_on_blitz_compatibility);
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

void compatconfig_on_blitz_compatibility(CompatConfig* self, psy_Property* sender)
{
	compatconfig_setloadnewblitz(self, psy_property_item_bool(sender));	
}

bool compatconfig_connect(CompatConfig* self, const char* key, void* context,
	void* fp)
{
	psy_Property* p;

	assert(self);

	p = compatconfig_property(self, key);
	if (p) {
		psy_property_connect(p, context, fp);
		return TRUE;
	}
	return FALSE;
}

psy_Property* compatconfig_property(CompatConfig* self, const char* key)
{
	assert(self);

	return psy_property_at(self->compatibility, key, PSY_PROPERTY_TYPE_NONE);
}

/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "metronomeconfig.h"

static void metronomeconfig_make(MetronomeConfig*, psy_Property*);

void metronomeconfig_init(MetronomeConfig* self, psy_Property* parent)
{
	assert(self && parent);

	self->parent = parent;
	metronomeconfig_make(self, parent);
	psy_signal_init(&self->signal_changed);
}

void metronomeconfig_dispose(MetronomeConfig* self)
{
	assert(self);

	psy_signal_dispose(&self->signal_changed);
}

void metronomeconfig_make(MetronomeConfig* self, psy_Property* parent)
{
	assert(self);

	self->metronome = psy_property_setid(psy_property_settext(
		psy_property_append_section(parent, "metronome"),
		"settingsview.metronome.metronome"), PROPERTY_ID_METRONOME);
	psy_property_setid(psy_property_settext(
		psy_property_append_bool(self->metronome, "showmetronome", FALSE),
		"settingsview.metronome.show"), PROPERTY_ID_SHOWMETRONOME);
	psy_property_settext(
		psy_property_append_int(self->metronome, "machine", 0x3F, 0, 0x40),
		"settingsview.metronome.machine");
	psy_property_settext(
		psy_property_append_int(self->metronome, "note", 48, 0, 119),
		"settingsview.metronome.note");
}

/* Properties */
uint8_t metronomeconfig_note(const MetronomeConfig* self)
{
	assert(self);

	return (uint8_t)(psy_property_at_int(self->metronome, "note", 48));
}

uintptr_t metronomeconfig_machine(const MetronomeConfig* self)
{
	assert(self);

	return (psy_property_at_int(self->metronome, "machine", 0x3F));		
}

bool metronomeconfig_showmetronomebar(const MetronomeConfig* self)
{
	assert(self);

	return (psy_property_at_bool(self->metronome, "showmetronome", FALSE));
}

/* events */
bool metronomeconfig_onchanged(MetronomeConfig* self, psy_Property*
	property)
{
	psy_signal_emit(&self->signal_changed, self, 1, property);
	return TRUE;
}

bool metronomeconfig_hasproperty(const MetronomeConfig* self, psy_Property* property)
{
	assert(self && self->metronome);

	return psy_property_insection(property, self->metronome);
}

/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


/* host */
#include "resources/resource.h"
/* audio */
#include <player.h>

#include "metronomeconfig.h"

static void metronomeconfig_make(MetronomeConfig*, psy_Property*);
static void metronomeconfig_on_property_changed(MetronomeConfig*,
	psy_Property* sender);

void metronomeconfig_init(MetronomeConfig* self, psy_Property* parent,
	psy_audio_Player* player)
{
	assert(self && parent && player);	

	self->parent = parent;
	self->player = player;
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

	self->metronome = psy_property_set_id(psy_property_set_text(
		psy_property_append_section(parent, "metronome"),
		"settingsview.metronome.metronome"), PROPERTY_ID_METRONOME);
	psy_property_set_icon(self->metronome, IDB_METRONOME_LIGHT,
		IDB_METRONOME_DARK);
	psy_property_connect(psy_property_set_id(psy_property_set_text(
		psy_property_append_bool(self->metronome, "showmetronome", FALSE),
		"settingsview.metronome.show"), PROPERTY_ID_SHOWMETRONOME),
		self, metronomeconfig_on_property_changed);
	psy_property_connect(psy_property_set_text(
		psy_property_append_int(self->metronome, "machine", 0x3F, 0, 0x40),
		"settingsview.metronome.machine"),
		self, metronomeconfig_on_property_changed);
	psy_property_connect(psy_property_set_text(
		psy_property_append_int(self->metronome, "note", 48, 0, 119),
		"settingsview.metronome.note"),
		self, metronomeconfig_on_property_changed);
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

void metronomeconfig_on_property_changed(MetronomeConfig* self, psy_Property* sender)
{
	self->player->sequencer.metronome_event.note =
		metronomeconfig_note(self);
	self->player->sequencer.metronome_event.mach =
		(uint8_t)
		metronomeconfig_machine(self);
	psy_signal_emit(&self->signal_changed, self, 1, sender);
}

bool metronomeconfig_connect(MetronomeConfig* self, const char* key, void* context,
	void* fp)
{
	psy_Property* p;

	assert(self);

	p = metronomeconfig_property(self, key);
	if (p) {
		psy_property_connect(p, context, fp);
		return TRUE;
	}
	return FALSE;
}

psy_Property* metronomeconfig_property(MetronomeConfig* self, const char* key)
{
	assert(self);

	return psy_property_at(self->metronome, key, PSY_PROPERTY_TYPE_NONE);
}

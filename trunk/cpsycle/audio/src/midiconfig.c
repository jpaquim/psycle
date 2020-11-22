// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "midiconfig.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

void psy_audio_midiconfiggroup_init(psy_audio_MidiConfigGroup* self)
{
	assert(self);

	self->record = FALSE;
	self->type = psy_audio_MIDICONFIG_T_COMMAND;
	self->command = 0x01;
	self->from = 0;
	self->to = 0xFF;
}

psy_audio_MidiConfigGroup* psy_audio_midiconfiggroup_alloc(void)
{
	return (psy_audio_MidiConfigGroup*)malloc(sizeof(
		psy_audio_MidiConfigGroup));
}

psy_audio_MidiConfigGroup* psy_audio_midiconfiggroup_allocinit(void)
{
	psy_audio_MidiConfigGroup* rv;

	rv = psy_audio_midiconfiggroup_alloc();
	if (rv) {
		psy_audio_midiconfiggroup_init(rv);
	}
	return rv;
}

static void psy_audio_midiconfig_setdefaultsettings(psy_audio_MidiConfig*);

void psy_audio_midiconfig_init(psy_audio_MidiConfig* self)
{
	assert(self);

	self->groups = NULL;
	psy_audio_midiconfig_setdefaultsettings(self);
}

void psy_audio_midiconfig_dispose(psy_audio_MidiConfig* self)
{
	assert(self);

	psy_list_deallocate(&self->groups, (psy_fp_disposefunc)NULL);	
}

void psy_audio_midiconfig_setdefaultsettings(psy_audio_MidiConfig* self)
{
	uintptr_t i;
	
	assert(self);

	psy_list_deallocate(&self->groups, (psy_fp_disposefunc)NULL);
	self->groups = NULL;
	for (i = 0; i < 16; ++i) {
		psy_list_append(&self->groups,
			psy_audio_midiconfiggroup_allocinit());
	}
	// enable velocity, raw and the default gen and inst selection
	psy_audio_midiconfiggroup_init(&self->pitch);
	self->pitch.record = TRUE;
	self->pitch.type = psy_audio_MIDICONFIG_T_COMMAND; // 0 is cmd
	self->pitch.command = 0xC;
	psy_audio_midiconfiggroup_init(&self->velocity);	
	self->raw = TRUE;
	self->gen_select_with = psy_audio_MIDICONFIG_MS_USE_SELECTED;
	self->inst_select_with = psy_audio_MIDICONFIG_MS_USE_SELECTED;
}

void psy_audio_midiconfig_configure(psy_audio_MidiConfig* self,
	psy_Property* configuration)
{	
	assert(self);

	if (configuration) {
		psy_Property* property;

		property = psy_property_at(configuration, "macselect",
			PSY_PROPERTY_TYPE_CHOICE);
		if (property) {
			if (psy_property_item_int(property) >= 0 &&
					psy_property_item_int(property) <=
					psy_audio_MIDICONFIG_MS_MIDI_CHAN) {
				self->gen_select_with = psy_property_item_int(property);
			}
		}
		property = psy_property_at(configuration, "auxselect",
			PSY_PROPERTY_TYPE_CHOICE);
		if (property) {
			if (psy_property_item_int(property) >= 0 &&
					psy_property_item_int(property) <=
					psy_audio_MIDICONFIG_MS_MIDI_CHAN) {
				self->inst_select_with = psy_property_item_int(property);
			}
		}
	}
}

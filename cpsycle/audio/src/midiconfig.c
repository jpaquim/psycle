// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "midiconfig.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "../../detail/portable.h"

void psy_audio_midiconfiggroup_init(psy_audio_MidiConfigGroup* self,
	int grouptype, int command)
{
	assert(self);

	self->grouptype = grouptype;
	self->record = FALSE;
	self->type = psy_audio_MIDICONFIG_T_COMMAND;
	self->command = command;
	self->from = 0;
	self->to = 0xFF;
	if (grouptype == psy_audio_MIDICONFIG_GT_VELOCITY) {
		self->message = 0;
	} else if (grouptype == psy_audio_MIDICONFIG_GT_PITCHWHEEL) {
		self->message = 1;
	} else {
		self->message = 0;
	}
}

psy_audio_MidiConfigGroup* psy_audio_midiconfiggroup_alloc(void)
{
	return (psy_audio_MidiConfigGroup*)malloc(sizeof(
		psy_audio_MidiConfigGroup));
}

psy_audio_MidiConfigGroup* psy_audio_midiconfiggroup_allocinit(int grouptype,
	int command)
{
	psy_audio_MidiConfigGroup* rv;

	rv = psy_audio_midiconfiggroup_alloc();
	if (rv) {
		psy_audio_midiconfiggroup_init(rv, grouptype, command);
	}
	return rv;
}

const char* psy_audio_midiconfiggroup_tostring(const psy_audio_MidiConfigGroup*
	self)
{	
	psy_snprintf(((psy_audio_MidiConfigGroup*)(self))->encode, 64,
		"%d %d %d %d %d %d ", self->message, self->type, self->command,
		(int)self->record, self->from, self->to);
	return self->encode;
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

void psy_audio_midiconfig_reset(psy_audio_MidiConfig* self)
{
	assert(self);

	psy_audio_midiconfig_setdefaultsettings(self);
}

void psy_audio_midiconfig_setdefaultsettings(psy_audio_MidiConfig* self)
{	
	assert(self);

	psy_list_deallocate(&self->groups, (psy_fp_disposefunc)NULL);
	self->groups = NULL;
	psy_list_append(&self->groups,
		psy_audio_midiconfiggroup_allocinit(psy_audio_MIDICONFIG_GT_VELOCITY,
			0x0c));
	psy_list_append(&self->groups,
		psy_audio_midiconfiggroup_allocinit(psy_audio_MIDICONFIG_GT_PITCHWHEEL,
			0x01));		
	self->raw = TRUE;
	self->gen_select_with = psy_audio_MIDICONFIG_MS_USE_SELECTED;
	self->inst_select_with = psy_audio_MIDICONFIG_MS_USE_SELECTED;
}

void psy_audio_midiconfig_configure(psy_audio_MidiConfig* self,
	const psy_Property* configuration, bool datastr)
{	
	assert(self);

	if (configuration) {
		const psy_Property* property;

		psy_audio_midiconfig_reset(self);
		property = psy_property_at_const(configuration, "macselect",
			PSY_PROPERTY_TYPE_CHOICE);
		if (property) {
			if (psy_property_item_int(property) >= 0 &&
				psy_property_item_int(property) <=
				psy_audio_MIDICONFIG_MS_MIDI_CHAN) {
				self->gen_select_with = psy_property_item_int(property);
			}
		}
		property = psy_property_at_const(configuration, "auxselect",
			PSY_PROPERTY_TYPE_CHOICE);
		if (property) {
			if (psy_property_item_int(property) >= 0 &&
				psy_property_item_int(property) <=
				psy_audio_MIDICONFIG_MS_MIDI_CHAN) {
				self->inst_select_with = psy_property_item_int(property);
			}
		}
		self->raw = psy_property_at_bool(configuration, "recordrawmidiasmcm", 0);
		if (datastr) {
			// read and configure controller groups
			property = psy_property_find_const(configuration, "controllerdata",
				PSY_PROPERTY_TYPE_STRING);
			if (property) {
				char* text;
				char seps[] = " ";
				char* token;
				int vals[6];
				int c;
				int i;

				assert(self);

				text = psy_strdup(psy_property_item_str(property));
				if (!text) {
					return;
				}
				token = strtok(text, seps);
				c = 0; i = 0;
				while (token != 0) {
					vals[c] = atoi(token);
					if (c == 5) {
						if (psy_audio_midiconfig_at(self, i)) {
							psy_audio_MidiConfigGroup* group;

							group = psy_audio_midiconfig_at(self, i);
							group->message = vals[0];
							if (i == 1) {
								group->command = vals[2];
							}
							group->record = vals[3];
							group->from = vals[4];
							group->to = vals[5];
						} else {
							psy_audio_MidiConfigGroup group;

							group.message = vals[0];
							group.type = vals[1];
							group.command = vals[2];
							group.record = vals[3];
							group.from = vals[4];
							group.to = vals[5];
							psy_audio_midiconfig_add_controller(self, group);
						}
						c = 0;
						++i;
					} else {
						++c;
					}
					token = strtok(0, seps);
				}
				free(text);
			}
		} else {
			const psy_List* i;
			int c;
			const psy_Property* controllers;

			controllers = psy_property_find_const(configuration, "controllers",
				PSY_PROPERTY_TYPE_SECTION);
			if (!controllers) {
				return;
			}
			for (i = psy_property_begin_const(controllers), c = 0; i != NULL;
					psy_list_next_const(&i), ++c) {
				const psy_Property* controller;

				controller = (psy_Property*)psy_list_entry_const(i);
				if (controller) {
					psy_audio_MidiConfigGroup newgroup;

					newgroup.message = (int)psy_property_at_int(controller, "message", 0);
					newgroup.type = (int)psy_property_at_int(controller, "type", 0);
					newgroup.command = (int)psy_property_at_int(controller, "cmd", 0);
					newgroup.record = (int)psy_property_at_bool(controller, "record", 0);
					newgroup.from = (int)psy_property_at_int(controller, "from", 0);
					newgroup.to = (int)psy_property_at_int(controller, "to", 0);
					if (psy_audio_midiconfig_at(self, c)) {
						psy_audio_MidiConfigGroup* group;

						group = psy_audio_midiconfig_at(self, c);
						group->message = newgroup.message;
						group->type = newgroup.type;
						group->command = newgroup.command;
						group->record = newgroup.record;
						group->from = newgroup.from;
						group->to = newgroup.to;
					} else {
						psy_audio_midiconfig_add_controller(self, newgroup);
					}
				}
			}		
		}
	}
}

void psy_audio_midiconfig_add_controller(psy_audio_MidiConfig* self,
	psy_audio_MidiConfigGroup group)
{
	psy_audio_MidiConfigGroup* newgroup;
	uintptr_t numcontrollers;
	int command;

	numcontrollers = psy_list_size(self->groups);
	if (numcontrollers == 0) {
		// Velocity Controller
		group.grouptype = psy_audio_MIDICONFIG_GT_VELOCITY;
		command = 0x0c;
	} if (numcontrollers == 1) {
		// Pitchwheel Controller
		group.grouptype = psy_audio_MIDICONFIG_GT_PITCHWHEEL;
		command = 0x01;
	} else {
		command = (int)numcontrollers - 2;
	}
	newgroup = psy_audio_midiconfiggroup_allocinit(group.grouptype, command);
	if (newgroup) {
		*newgroup = group;
		psy_list_append(&self->groups, newgroup);
	}
}

void psy_audio_midiconfig_removecontroller(psy_audio_MidiConfig* self, intptr_t id)
{
	assert(self);

	// doesn't remove velocity or pitchwheel	
	if (self->groups && id > 1) {
		psy_List* i;

		i = psy_list_at(self->groups, id);
		if (i) {
			free(i->entry);
			psy_list_remove(&self->groups, i);
		}
	}
}

char_dyn_t* psy_audio_midiconfig_controllers_tostring(
	const psy_audio_MidiConfig* self)
{
	char_dyn_t* rv = NULL;
	const psy_List* i;
		
	for (i = self->groups; i != NULL; psy_list_next_const(&i)) {
		const psy_audio_MidiConfigGroup* group;

		group = (psy_audio_MidiConfigGroup*)psy_list_entry_const(i);
		rv = psy_strcat_realloc(rv, psy_audio_midiconfiggroup_tostring(group));
	}	
	return rv;
}

/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "midiviewconfig.h"
/* host */
#include "resources/resource.h"
/* platform */
#include "../../detail/portable.h"


/* prototypes */
static void midiviewconfig_make(MidiViewConfig*, psy_Property*);
static void midiviewconfig_on_controller_changed(MidiViewConfig* self, psy_Property* sender);
static void midiviewconfig_on_add_controller_map(MidiViewConfig*,
	psy_Property* sender);
static void midiviewconfig_on_remove_controller_maps(MidiViewConfig*,
	psy_Property* sender);
static void midiviewconfig_remove_controller_map(MidiViewConfig*,
	psy_Property* group);

/* implementation */
void midiviewconfig_init(MidiViewConfig* self, psy_Property* parent,
	psy_audio_Player* player)
{
	assert(self && parent && player);

	self->parent = parent;
	self->player = player;
	midiviewconfig_make(self, parent);	
}

void midiviewconfig_dispose(MidiViewConfig* self)
{
	
}

void midiviewconfig_make(MidiViewConfig* self, psy_Property* parent)
{
	psy_Property* choice;

	self->controllers = psy_property_set_text(
		psy_property_append_section(parent, "controllers"),
		"settings.midicontrollers.controllers");
	psy_property_set_icon(self->controllers, IDB_WIRES_LIGHT,
		IDB_WIRES_DARK);
	/* macselect */
	choice = psy_property_set_hint(psy_property_set_text(
		psy_property_append_choice(self->controllers,
			"macselect", 0),
		"settings.midicontrollers.macselect"),
		PSY_PROPERTY_HINT_COMBO);
	psy_property_set_text(
		psy_property_append_str(choice, "inpsycle", ""),
		"settings.midicontrollers.select-inpsycle");
	psy_property_set_text(
		psy_property_append_str(choice, "bybank", ""),
		"settings.midicontrollers.select-bybank");
	psy_property_set_text(
		psy_property_append_str(choice, "bybank", ""),
		"settings.midicontrollers.select-byprogram");
	psy_property_set_text(
		psy_property_append_str(choice, "bybank", ""),
		"settings.midicontrollers.select-bychannel");
	/* auxselect */
	choice = psy_property_set_hint(psy_property_set_text(
		psy_property_append_choice(self->controllers,
			"auxselect", 0),
		"settings.midicontrollers.auxselect"),
		PSY_PROPERTY_HINT_COMBO);
	psy_property_set_text(
		psy_property_append_str(choice, "inpsycle", ""),
		"settings.midicontrollers.select-inpsycle");
	psy_property_set_text(
		psy_property_append_str(choice, "bybank", ""),
		"settings.midicontrollers.select-bybank");
	psy_property_set_text(
		psy_property_append_str(choice, "bybank", ""),
		"settings.midicontrollers.select-byprogram");
	psy_property_set_text(
		psy_property_append_str(choice, "bybank", ""),
		"settings.midicontrollers.select-bychannel");
	psy_property_set_text(
		psy_property_append_bool(self->controllers,
			"recordrawmidiasmcm", TRUE),
		"settings.midicontrollers.recordrawmidiasmcm");
	/* map controllers */
	psy_property_connect(
		psy_property_set_id(psy_property_set_text(
			psy_property_append_action(self->controllers,
			"addcontroller"),
			"settings.midicontrollers.add"),
		PROPERTY_ID_ADDCONTROLLERMAP),
		self, midiviewconfig_on_add_controller_map);
	psy_property_connect(
		psy_property_set_id(psy_property_set_text(
			psy_property_append_action(self->controllers,
				"removecontrollers"),
			"settings.midicontrollers.remove"),
			PROPERTY_ID_REMOVECONTROLLERMAP),
		self, midiviewconfig_on_remove_controller_maps);
	psy_property_hide(psy_property_append_str(self->controllers,
		"controllerdata", ""));
	psy_property_prevent_save(psy_property_set_text(
		psy_property_append_section(self->controllers,
			"controllers"), "Mapping"));
}

void midiviewconfig_make_controllers(MidiViewConfig* self)
{
	char text[256];

	psy_Property* controllers;

	controllers = psy_property_find(self->controllers, "controllers",
		PSY_PROPERTY_TYPE_SECTION);
	if (controllers) {
		psy_List* i;
		int c;

		psy_property_clear(controllers);
		for (c = 0, i = self->player->midiinput.midiconfig.groups; i != NULL;
			psy_list_next(&i), ++c)
		{
			psy_audio_MidiConfigGroup* midigroup;
			psy_Property* group;
			psy_Property* choice;
			psy_Property* range;
			bool isvelocity;
			bool ispitchwheel;
			bool hasmessage;

			isvelocity = c == 0;
			ispitchwheel = c == 1;
			hasmessage = c > 1;

			midigroup = (psy_audio_MidiConfigGroup*)psy_list_entry(i);
			psy_snprintf(text, 256, "controller%X", (c + 1));
			group = psy_property_set_text(
				psy_property_append_section(controllers, text),
				(isvelocity)
				? "Velocity"
				: (ispitchwheel)
				? "Pitch Wheel"
				: "Controller");
			psy_property_hide(psy_property_append_int(group, "id", c, 0, 0));
			if (hasmessage) {
				psy_property_set_hint(psy_property_set_text(psy_property_append_bool(
					group, "select", midigroup->record),
					"settings.midicontrollers.select"),
					PSY_PROPERTY_HINT_CHECK);
			}
			psy_property_set_text(psy_property_append_bool(group, "record",
				midigroup->record), "Map");
			psy_property_set_hint(psy_property_setreadonly(psy_property_set_text(
				psy_property_append_int(group, "message", midigroup->message, 0, 127),
				"Controller Number"), (!hasmessage)), PSY_PROPERTY_HINT_EDITHEX);
			if (hasmessage || ispitchwheel) {
				choice = psy_property_set_hint(psy_property_set_text(
					psy_property_append_choice(group, "type", midigroup->type),
					"to"), PSY_PROPERTY_HINT_COMBO);
				psy_property_append_str(choice, "cmd", "");
				psy_property_append_str(choice, "twk", "");
				psy_property_append_str(choice, "tws", "");
				psy_property_append_str(choice, "mcm", "");
			}			
			psy_property_set_hint(psy_property_set_text(psy_property_append_int(
				group, "cmd", midigroup->command, 0, 0xF), "value"),
				PSY_PROPERTY_HINT_EDITHEX);
			range = psy_property_set_hint(psy_property_append_section(
				group, "range"), PSY_PROPERTY_HINT_RANGE);
			psy_property_set_hint(psy_property_append_int(
				range, "from", midigroup->from, 0, 0xFF),
				PSY_PROPERTY_HINT_EDITHEX);
			psy_property_set_hint(psy_property_append_int(
				range, "to", midigroup->to, 0, 0xFF),
				PSY_PROPERTY_HINT_EDITHEX);			
		}
		psy_property_rebuild(controllers);
	}
	midiviewconfig_make_controller_save(self);
}

void midiviewconfig_make_controller_save(MidiViewConfig* self)
{
	char_dyn_t* str;
	psy_Property* controllers;

	controllers = psy_property_find(self->controllers, "controllerdata",
		PSY_PROPERTY_TYPE_STRING);
	if (!controllers) {
		return;
	}
	str = psy_audio_midiconfig_controllers_tostring(
		psy_audio_player_midiconfig(self->player));
	psy_property_set_item_str(controllers, str);
	free(str);
}

/* events */

void midiviewconfig_on_add_controller_map(MidiViewConfig* self,
	psy_Property* sender)
{
	psy_audio_MidiConfigGroup group;

	psy_audio_midiconfiggroup_init(&group, psy_audio_MIDICONFIG_GT_CUSTOM, 1);
	psy_audio_midiconfig_add_controller(&self->player->midiinput.midiconfig, group);
	midiviewconfig_make_controllers(self);
}

void midiviewconfig_on_remove_controller_maps(MidiViewConfig* self,
	psy_Property* sender)
{
	psy_Property* controllers;
	psy_List* p;
	psy_List* q;	
	bool removed;

	assert(self);

	q = NULL;
	removed = FALSE;

	controllers = psy_property_at(self->controllers, "controllers",
		PSY_PROPERTY_TYPE_SECTION);
	if (!controllers) {
		return;
	}
	for (p = psy_property_begin(controllers); p != NULL; p = q) {
		psy_Property* group;
		bool selected;

		q = p->next;
		group = (psy_Property*)p->entry;
		selected = psy_property_at_bool(group, "select", FALSE);
		if (selected) {
			midiviewconfig_remove_controller_map(self, group);
			removed = TRUE;
		}
	}
	if (removed) {
		psy_property_rebuild(controllers);
	}
}

void midiviewconfig_remove_controller_map(MidiViewConfig* self,
	psy_Property* group)
{
	intptr_t id;

	assert(self);
	assert(group);

	id = psy_property_at_int(group, "id", -1);
	if (id != -1) {
		psy_Property* parent;

		parent = psy_property_parent(group);		
		psy_audio_midiconfig_remove_controller(
			&self->player->midiinput.midiconfig, id);
		if (parent) {
			psy_property_remove(parent, group);
		}
		// midiviewconfig_make_controller_save(self);
	}
}

void midiviewconfig_on_controller_changed(MidiViewConfig* self,
	psy_Property* sender)
{	
	assert(self);
	
	if (psy_property_in_section(sender, self->controllers)) {
		psy_audio_player_midi_configure(self->player,
			self->controllers, FALSE);
		midiviewconfig_make_controller_save(self);
	}	
}

bool midiviewconfig_has_property(const MidiViewConfig* self,
	psy_Property* property)
{
	assert(self && self->controllers);

	return psy_property_in_section(property, self->controllers);
}

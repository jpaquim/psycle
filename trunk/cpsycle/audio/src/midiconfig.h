/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#ifndef psy_audio_MIDICONFIG_H
#define psy_audio_MIDICONFIG_H

#include "../../detail/psydef.h"


/* container */
#include <list.h>
#include <properties.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
	psy_audio_MIDICONFIG_MS_USE_SELECTED = 0,
	psy_audio_MIDICONFIG_MS_BANK,
	psy_audio_MIDICONFIG_MS_PROGRAM,
	psy_audio_MIDICONFIG_MS_MIDI_CHAN
} psy_audio_midiconfig_selector_t;

enum {
	psy_audio_MIDICONFIG_T_COMMAND = 0,
	psy_audio_MIDICONFIG_T_TWEAK,
	psy_audio_MIDICONFIG_T_TWEAKSLIDE,
	psy_audio_MIDICONFIG_T_MCM,
	psy_audio_MIDICONFIG_NUM_TYPES
};

enum {
	psy_audio_MIDICONFIG_GT_VELOCITY,
	psy_audio_MIDICONFIG_GT_PITCHWHEEL,
	psy_audio_MIDICONFIG_GT_CUSTOM		
};


typedef struct psy_audio_MidiConfigGroup {
	int grouptype;
	bool record;
	int type;
	int command;
	int from;
	int to;
	int message;
	char encode[64];
} psy_audio_MidiConfigGroup;

void psy_audio_midiconfiggroup_init(psy_audio_MidiConfigGroup*, int grouptype,
	int command);

psy_audio_MidiConfigGroup* psy_audio_midiconfiggroup_alloc(void);
psy_audio_MidiConfigGroup* psy_audio_midiconfiggroup_allocinit(int grouptype,
	int command);

INLINE bool psy_audio_midiconfiggroup_record(const psy_audio_MidiConfigGroup* self)
{
	assert(self);

	return self->record;
}

INLINE int  psy_audio_midiconfiggroup_type(const psy_audio_MidiConfigGroup* self)
{
	assert(self);

	return self->type;
}

INLINE int  psy_audio_midiconfiggroup_command(const psy_audio_MidiConfigGroup* self)
{ 
	assert(self);

	return self->command;
}
INLINE int  psy_audio_midiconfiggroup_from(const psy_audio_MidiConfigGroup* self)
{
	assert(self);

	return self->from;
}

INLINE int  psy_audio_midiconfiggroup_to(const psy_audio_MidiConfigGroup* self)
{
	assert(self);

	return self->to;
}

const char* psy_audio_midiconfiggroup_tostring(const psy_audio_MidiConfigGroup* self);

typedef struct psy_audio_MidiConfig {
	psy_List* groups;	
	bool raw;
	psy_audio_midiconfig_selector_t	gen_select_with;
	psy_audio_midiconfig_selector_t	inst_select_with;
} psy_audio_MidiConfig;

void psy_audio_midiconfig_init(psy_audio_MidiConfig*);
void psy_audio_midiconfig_dispose(psy_audio_MidiConfig*);
void psy_audio_midiconfig_reset(psy_audio_MidiConfig*);

void psy_audio_midiconfig_configure(psy_audio_MidiConfig*,
	const psy_Property* configuration, bool datastr);

void psy_audio_midiconfig_add_controller(psy_audio_MidiConfig*,
	psy_audio_MidiConfigGroup group);
void psy_audio_midiconfig_remove_controller(psy_audio_MidiConfig* self, intptr_t id);
char_dyn_t* psy_audio_midiconfig_controllers_tostring(const psy_audio_MidiConfig* self);

INLINE uintptr_t psy_audio_midiconfig_numcontrollers(const
	psy_audio_MidiConfig* self)
{
	assert(self);

	return psy_list_size(self->groups);
}

INLINE psy_audio_MidiConfigGroup* psy_audio_midiconfig_at(const
	psy_audio_MidiConfig* self, int index)
{
	psy_List* i;

	assert(self);

	i = psy_list_at(self->groups, index);
	if (i) {
		return (psy_audio_MidiConfigGroup*)psy_list_entry(i);
	}
	return NULL;
}

#ifdef __cplusplus
}
#endif

#endif /* psy_audio_MIDICONFIG */

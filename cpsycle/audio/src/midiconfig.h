// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#ifndef psy_audio_MIDICONFIG_H
#define psy_audio_MIDICONFIG_H

#include "../../detail/psydef.h"

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

typedef struct psy_audio_MidiConfigGroup {
	bool record;
	int type;
	int command;
	int from;
	int to;
} psy_audio_MidiConfigGroup;

void psy_audio_midiconfiggroup_init(psy_audio_MidiConfigGroup*);

psy_audio_MidiConfigGroup* psy_audio_midiconfiggroup_alloc(void);
psy_audio_MidiConfigGroup* psy_audio_midiconfiggroup_allocinit(void);

INLINE bool psy_audio_midiconfiggroup_record(const psy_audio_MidiConfigGroup* self)
{
	return self->record;
}

INLINE int  psy_audio_midiconfiggroup_type(const psy_audio_MidiConfigGroup* self)
{
	return self->type;
}

INLINE int  psy_audio_midiconfiggroup_command(const psy_audio_MidiConfigGroup* self)
{ 
	return self->command;
}
INLINE int  psy_audio_midiconfiggroup_from(const psy_audio_MidiConfigGroup* self)
{
	return self->from;
}

INLINE int  psy_audio_midiconfiggroup_to(const psy_audio_MidiConfigGroup* self)
{
	return self->to;
}

typedef struct psy_audio_MidiConfig {
	psy_List* groups;
	psy_audio_MidiConfigGroup velocity;
	psy_audio_MidiConfigGroup pitch;
	bool raw;
	psy_audio_midiconfig_selector_t	gen_select_with;
	psy_audio_midiconfig_selector_t	inst_select_with;
} psy_audio_MidiConfig;

void psy_audio_midiconfig_init(psy_audio_MidiConfig*);
void psy_audio_midiconfig_dispose(psy_audio_MidiConfig*);

void psy_audio_midiconfig_configure(psy_audio_MidiConfig*, psy_Property*
	configuration);

#ifdef __cplusplus
}
#endif

#endif /* psy_audio_MIDICONFIG */

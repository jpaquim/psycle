// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#ifndef psy_audio_MIDIINPUT_H
#define psy_audio_MIDIINPUT_H

#include "constants.h"
#include "midiconfig.h"
#include "eventdrivers.h"
#include "machines.h"

#ifdef __cplusplus
extern "C" {
#endif

#define psy_audio_MAX_MIDI_CHANNELS 16
#define psy_audio_MAX_CONTROLLERS   127
#define psy_audio_MAX_PARAMETERS    127

typedef struct psy_audio_MidiInputStats
{	
	/// bitmapped channel active map	(CLEAR AFTER READ)
	uint32_t channelmap;
	/// counter strobe for the channel map list
	uintptr_t channelmapupdate;
	/// 32 bits of boolean info (see FLAGS, CLEAR AFTER READ)
	uint32_t flags;
} psy_audio_MidiInputStats;

void psy_audio_midiinputstats_init(psy_audio_MidiInputStats*);

typedef struct psy_audio_MidiInput {
	psy_audio_MidiConfig midiconfig;	
	/// channel->instrument map
	uintptr_t channelinstmap[MAX_INSTRUMENTS];
	/// midi channel->generator map
	uintptr_t channelgeneratormap[psy_audio_MAX_MIDI_CHANNELS];
	/// statistics information
	psy_audio_MidiInputStats stats;
} psy_audio_MidiInput;

// init dispose
void psy_audio_midiinput_init(psy_audio_MidiInput*);
void psy_audio_midiinput_dispose(psy_audio_MidiInput*);

void psy_audio_midiinput_configure(psy_audio_MidiInput*, psy_Property*
	configuration);
bool psy_audio_midiinput_workinput(psy_audio_MidiInput*,
	psy_EventDriverMidiData mididata, psy_audio_Machines*,
	psy_audio_PatternEvent* rv);
int psy_audio_midiinput_instmap(psy_audio_MidiInput*, int channel);
int psy_audio_midiinput_genmap(const psy_audio_MidiInput*, int channel);


#ifdef __cplusplus
}
#endif

#endif /* psy_audio_MIDIINPUT_H */

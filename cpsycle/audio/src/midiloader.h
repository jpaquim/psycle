/*
** This source is free software; you can redistribute itand /or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2, or (at your option) any later version.
** copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net
*/

#ifndef psy_audio_MIDILOADER_H
#define psy_audio_MIDILOADER_H

#include "../../detail/psydef.h"

// local
#include "pattern.h"
#include "machine.h"
#include "midifiledefs.h"
#include "songio.h"
#include "sequence.h"

#ifdef __cplusplus
extern "C" {
#endif

/* MidiLoader
**
** Loads a Midi File:
** - for each midi track a track in the sequence is appended
** - the midi channel is set to the patternevent mach parameter
** - midi controller events are stored in track 0 as multi cmd
** - midi polyphony is mapped to tracker channels 1..64 (MAX_POLYPHONY)
** - noteoffs are created if the following noteon time is greater than
**   the noteoff time
** - note on volume is mapped to 0x0..0x80 MAX
** - no machines are added and must set in the machineview manually
*/

#define MAX_MIDIFILE_POLYPHONY 64

typedef struct MidiChannel {
	psy_audio_PatternEvent tracknote;
	bool noteoff;
	psy_dsp_big_beat_t time;
} MidiChannel;

typedef struct MidiTrackState
{
	uint8_t channel;
	uintptr_t trackidx;
	uintptr_t automationchannel;
	psy_dsp_big_beat_t position;
	psy_dsp_big_beat_t patternoffset;
	psy_audio_PatternNode* patternnode;
	psy_audio_Pattern* pattern;
	psy_audio_SequenceTrack* track;	
	MidiChannel channels[MAX_MIDIFILE_POLYPHONY];	
	uint8_t runningstatus;
	uint8_t byte1;
	bool hasrunningstatus;
} MidiTrackState;

void miditrackstate_init(MidiTrackState*);
void miditrackstate_dispose(MidiTrackState*);

void miditrackstate_reset(MidiTrackState*);

typedef struct MidiLoader {
	/* internal */
	PsyFile* fp;
	MTHD mthd;
	MidiTrackState currtrack;	
	/* references */
	psy_audio_SongFile* songfile;
} MidiLoader;

void midiloader_init(MidiLoader*, psy_audio_SongFile*);
void midiloader_dispose(MidiLoader*);

int midiloader_load(MidiLoader*);

#ifdef __cplusplus
}
#endif

#endif /* psy_audio_MIDILOADER_H */

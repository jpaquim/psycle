// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#ifndef psy_audio_INSTRUMENT_H
#define psy_audio_INSTRUMENT_H

#include "patternevent.h"
#include "samples.h"

#include <adsr.h>
#include <multifilter.h>
#include <list.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	double low;
	double high; // including
	double min;
	double max;
} psy_audio_FrequencyRange;

void frequencyrange_init(psy_audio_FrequencyRange* self,
	double low, double high, double min, double max);

/// When a new note comes to play in a channel, and there is still one playing
/// in it, do this on the currently playing note:
typedef enum {
	///  [Note Cut]	(This one actually does a very fast fadeout)
	psy_audio_NNA_STOP = 0x0,
	///  [Ignore]
	psy_audio_NNA_CONTINUE = 0x1,
	///  [Note off]
	psy_audio_NNA_NOTEOFF = 0x2,
	///  [Note fade]
	psy_audio_NNA_FADEOUT = 0x3
} psy_audio_NewNoteAction;

typedef struct {
	psy_audio_SampleIndex sampleindex;
	psy_audio_FrequencyRange freqrange;
	psy_audio_ParameterRange keyrange;
	psy_audio_ParameterRange velocityrange;	
	int use_keyrange;
	int use_velrange;
	int use_freqrange;
} psy_audio_InstrumentEntry;

void instrumententry_init(psy_audio_InstrumentEntry*);
psy_audio_InstrumentEntry* instrumententry_alloc(void);
psy_audio_InstrumentEntry* instrumententry_allocinit(void);

typedef struct psy_audio_Instrument {	
	char* name;
	bool loop;
	uintptr_t lines;
	/// Action to take on the playing voice when any new note comes in the same channel.
	psy_audio_NewNoteAction nna;
	/// [0..1.0f] Global volume affecting all samples of the instrument.
	psy_dsp_amp_t globalvolume;
	psy_dsp_ADSRSettings volumeenvelope;
	psy_dsp_ADSRSettings filterenvelope;	
	float filtercutoff;	
	float filterres;		
	float filtermodamount;	
	FilterType filtertype;		
	int randompan;
	unsigned char _RCUT;
	unsigned char _RRES;
	psy_List* entries;
	uintptr_t index;
	psy_Signal signal_addentry;
	psy_Signal signal_removeentry;
	psy_Signal signal_namechanged;
} psy_audio_Instrument;

void instrument_init(psy_audio_Instrument*);
void instrument_dispose(psy_audio_Instrument*);
psy_audio_Instrument* instrument_alloc(void);
psy_audio_Instrument* instrument_allocinit(void);
void instrument_load(psy_audio_Instrument*, const char* path);
void instrument_setname(psy_audio_Instrument*, const char* name);
void instrument_setindex(psy_audio_Instrument*, uintptr_t index);
uintptr_t instrument_index(psy_audio_Instrument*);
const char* instrument_name(psy_audio_Instrument*);
void instrument_setnna(psy_audio_Instrument*, psy_audio_NewNoteAction nna);
psy_audio_NewNoteAction instrument_nna(psy_audio_Instrument*);
psy_List* instrument_entriesintersect(psy_audio_Instrument*, uintptr_t key,
	uintptr_t velocity, double frequency);
void instrument_clearentries(psy_audio_Instrument*);
void instrument_addentry(psy_audio_Instrument*,
	const psy_audio_InstrumentEntry* entry);
void instrument_removeentry(psy_audio_Instrument*,
	uintptr_t numentry);
psy_audio_InstrumentEntry* instrument_entryat(psy_audio_Instrument*,
	uintptr_t numentry);
const psy_List* instrument_entries(psy_audio_Instrument*);

#ifdef __cplusplus
}
#endif

#endif /* psy_audio_INSTRUMENT_H */

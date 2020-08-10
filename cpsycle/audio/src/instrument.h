// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#ifndef psy_audio_INSTRUMENT_H
#define psy_audio_INSTRUMENT_H

#include "patternevent.h"
#include "samples.h"

#include <adsr.h>
#include <filter.h>
#include <list.h>

#ifdef __cplusplus
extern "C" {
#endif

// psy_audio_Instrument, psy_audio_InstrumentEntry
//
// aim: an instrument is a selection of samples with some extra features added
//      around it. Samples are composed with help of instrument entries.
//  	A 'psy_audio_InstrumentEntry' stores a 'psy_audio_SampleIndex', the
//      note and the velocity range to allow a dynamically choice.
//
// Structure:
// psy_audio_Instrument <@>----- psy_audio_InstrumentEntry
//                             * 

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

void psy_audio_instrumententry_init(psy_audio_InstrumentEntry*);
psy_audio_InstrumentEntry* psy_audio_instrumententry_alloc(void);
psy_audio_InstrumentEntry* psy_audio_instrumententry_allocinit(void);

typedef struct psy_audio_Instrument {	
	char* name;
	bool loop;
	uintptr_t lines;
	/// Action to take on the playing voice when any new note comes in the same
	/// channel.
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

void psy_audio_instrument_init(psy_audio_Instrument*);
void psy_audio_instrument_dispose(psy_audio_Instrument*);
psy_audio_Instrument* psy_audio_instrument_alloc(void);
psy_audio_Instrument* psy_audio_instrument_allocinit(void);
void psy_audio_instrument_load(psy_audio_Instrument*, const char* path);
void psy_audio_instrument_setname(psy_audio_Instrument*, const char* name);
void psy_audio_instrument_setindex(psy_audio_Instrument*, uintptr_t index);
uintptr_t psy_audio_instrument_index(psy_audio_Instrument*);
const char* psy_audio_instrument_name(psy_audio_Instrument*);
void psy_audio_instrument_setnna(psy_audio_Instrument*,
	psy_audio_NewNoteAction);
psy_audio_NewNoteAction psy_audio_instrument_nna(psy_audio_Instrument*);
psy_List* psy_audio_instrument_entriesintersect(psy_audio_Instrument*,
	uintptr_t key, uintptr_t velocity, double frequency);
void psy_audio_instrument_clearentries(psy_audio_Instrument*);
void psy_audio_instrument_addentry(psy_audio_Instrument*,
	const psy_audio_InstrumentEntry* entry);
void psy_audio_instrument_removeentry(psy_audio_Instrument*,
	uintptr_t numentry);
psy_audio_InstrumentEntry* psy_audio_instrument_entryat(psy_audio_Instrument*,
	uintptr_t numentry);
const psy_List* psy_audio_instrument_entries(psy_audio_Instrument*);

INLINE psy_dsp_amp_t psy_audio_instrument_volume(psy_audio_Instrument* self)
{
	return self->globalvolume;
}

#ifdef __cplusplus
}
#endif

#endif /* psy_audio_INSTRUMENT_H */

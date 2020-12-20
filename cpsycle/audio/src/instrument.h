// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#ifndef psy_audio_INSTRUMENT_H
#define psy_audio_INSTRUMENT_H

#include "patternevent.h"
#include "samples.h"

#include <envelope.h>
#include <filter.h>
#include <list.h>

#ifdef __cplusplus
extern "C" {
#endif

// psy_audio_Instrument, psy_audio_InstrumentEntry
//
// An instrument is a selection of samples with some extra features added
// around it. Samples are composed with help of instrument entries.
// A 'psy_audio_InstrumentEntry' stores a 'psy_audio_SampleIndex', the
// note and the velocity range to allow a dynamically choice.
//
// Structure:
// psy_audio_Instrument <@>----- psy_audio_InstrumentEntry

typedef struct {
	double low;
	double high; // including
	double min;
	double max;
} psy_audio_FrequencyRange;

void psy_audio_frequencyrange_init(psy_audio_FrequencyRange* self,
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

/// In some cases, the default NNA is not adequate. This option lets choose one type of element
/// that, if it is equal than the currently playing, will apply the DCAction intead.
/// A common example is using NNA NOTEOFF, DCT NOTE and DCA STOP
typedef enum psy_audio_DupeCheck {	
	psy_audio_DUPECHECK_NONE = 0x0,
	psy_audio_DUPECHECK_NOTE,
	psy_audio_DUPECHECK_SAMPLE,
	psy_audio_DUPECHECK_INSTRUMENT	
} psy_audio_DupeCheck;

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
	bool enabled;
	char* name;
	bool loop;
	uintptr_t lines;	
	
	/// envelope range = [0.0f..1.0f]
	psy_dsp_EnvelopeSettings volumeenvelope;
	/// envelope range = [-1.0f..1.0f]
	psy_dsp_EnvelopeSettings panenvelope;
	/// envelope range = [-1.0f..1.0f]
	psy_dsp_EnvelopeSettings pitchenvelope;
	/// envelope range = [0.0f..1.0f]
	psy_dsp_EnvelopeSettings filterenvelope;

	/// [0..1.0f] Global volume affecting all samples of the instrument.
	psy_dsp_amp_t globalvolume;
	/// [0..1.0f] Fadeout speed. Decreasing amount for each tracker tick.
	float volumefadespeed;
	// Paninng
	bool panenabled;
	/// Initial panFactor (if enabled) [-1..1]
	float initpan;
	bool surround;
	/// Note number for center pan position
	uint8_t notemodpancenter;
	/// -32..32. 1/256th of panFactor change per seminote.
	int8_t notemodpansep;

	/// Cutoff Frequency [0..1]
	float filtercutoff;
	unsigned char _RCUT;
	/// Resonance [0..1]	
	float filterres;
	unsigned char _RRES;
	float filtermodamount;
	/// Filter Type. See psy_dsp_FilterType
	psy_dsp_FilterType filtertype;		
	int randompan;

	// Randomness. Applies on new notes.

	/// Random Volume % [ 0.0 -> No randomize. 1.0 = randomize full scale.]
	float randomvolume;
	/// Random Panning (same)
	float randompanning;
	/// Random CutOff (same)
	float randomcutoff;
	/// Random Resonance (same)
	float randomresonance;

	/// Action to take on the playing voice when any new note comes in the same
	/// channel.
	psy_audio_NewNoteAction nna;
	/// Check to do when a new event comes in the channel.
	psy_audio_DupeCheck dct;
	/// Action to take on the playing voice when the action defined by DCT comes in the same channel 
	/// (like the same note value).
	psy_audio_NewNoteAction dca;

	/// List of InstrumentEntries selecting for a note, frequency or/and velocity
	/// a sample to be played
	/// replaced psycle-mfc: NotePair m_AssignNoteToSample[NOTE_MAP_SIZE];
	///	     Table of mapped notes to samples
	///      (note number=first, sample number=second)
	///      \todo Could it be interesting to map other things like
	///       volume,panning, cutoff...?
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

const char* psy_audio_instrument_tostring(const psy_audio_Instrument*);
void psy_audio_instrument_fromstring(psy_audio_Instrument*, const char*);

#ifdef __cplusplus
}
#endif

#endif /* psy_audio_INSTRUMENT_H */

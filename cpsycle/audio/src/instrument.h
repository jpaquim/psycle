// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#ifndef psy_audio_INSTRUMENT_H
#define psy_audio_INSTRUMENT_H

// local
#include "patternevent.h"
#include "samples.h"
// dsp
#include <envelope.h>
#include <filter.h>

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

/// In some cases, the default NNA is not adequate. This option lets choose
/// one type of element that, if it is equal than the currently playing, will
/// apply the DCAction intead.
/// A common example is using NNA NOTEOFF, DCT NOTE and DCA STOP
typedef enum psy_audio_DupeCheck {	
	psy_audio_DUPECHECK_NONE = 0x0,
	psy_audio_DUPECHECK_NOTE,
	psy_audio_DUPECHECK_SAMPLE,
	psy_audio_DUPECHECK_INSTRUMENT	
} psy_audio_DupeCheck;

typedef struct {	
	psy_audio_SampleIndex sampleindex;
	uint8_t fixedkey;
	psy_audio_FrequencyRange freqrange;
	psy_audio_ParameterRange keyrange;
	psy_audio_ParameterRange velocityrange;	
	bool use_keyrange;
	bool use_velrange;
	bool use_freqrange;
} psy_audio_InstrumentEntry;

void psy_audio_instrumententry_init(psy_audio_InstrumentEntry*);
psy_audio_InstrumentEntry* psy_audio_instrumententry_alloc(void);
psy_audio_InstrumentEntry* psy_audio_instrumententry_allocinit(void);

bool psy_audio_instrumententry_intersect(psy_audio_InstrumentEntry*,
	uintptr_t key, uintptr_t velocity, double frequency);

typedef struct psy_audio_Instrument {
	bool enabled;
	char* name;
	bool loop;
	uintptr_t lines;	
	
	/// envelope range = [0.0f..1.0f]
	psy_dsp_Envelope volumeenvelope;
	/// envelope range = [-1.0f..1.0f]
	psy_dsp_Envelope panenvelope;
	/// envelope range = [-1.0f..1.0f]
	psy_dsp_Envelope pitchenvelope;
	/// envelope range = [0.0f..1.0f]
	psy_dsp_Envelope filterenvelope;

	/// [0..1.0f] Global volume affecting all samples of the instrument.
	psy_dsp_amp_t globalvolume;
	/// [0..1.0f] Fadeout speed. Decreasing amount for each tracker tick.
	float volumefadespeed;

	// Paninng
	bool panenabled;	
	/// Default position for panning ( 0..1 ) 0left 1 right.
	/// psycle mfc: todo wrong comment? (Initial panFactor (if enabled) [-1..1])
	float initpan;
	bool surround;
	/// Note number for center pan position
	uint8_t notemodpancenter;
	/// -32..32. 1/256th of panFactor change per seminote.
	int8_t notemodpansep;

	/// Cutoff Frequency [0..1]
	float filtercutoff;	
	/// Resonance [0..1]	
	float filterres;
	unsigned char _RRES;
	// [-1.0 .. 1.0]
	float filtermodamount;
	/// Filter Type. See psy_dsp_FilterType
	psy_dsp_FilterType filtertype;	

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
	/// Action to take on the playing voice when the action defined by DCT
	/// comes in the same channel (like the same note value).
	psy_audio_NewNoteAction dca;

	/// List of InstrumentEntries selecting for a note, frequency or/and
	/// velocity a sample to be played
	/// replaced psycle-mfc: NotePair m_AssignNoteToSample[NOTE_MAP_SIZE];
	///	     Table of mapped notes to samples
	///      (note number=first, sample number=second)
	///      \todo Could it be interesting to map other things like
	///       volume,panning, cutoff...?
	psy_List* entries;

	// Signals
	psy_Signal signal_addentry;
	psy_Signal signal_removeentry;
	psy_Signal signal_namechanged;
} psy_audio_Instrument;

void psy_audio_instrument_init(psy_audio_Instrument*);
void psy_audio_instrument_dispose(psy_audio_Instrument*);
psy_audio_Instrument* psy_audio_instrument_alloc(void);
psy_audio_Instrument* psy_audio_instrument_allocinit(void);
void psy_audio_instrument_deallocate(psy_audio_Instrument*);

void psy_audio_instrument_load(psy_audio_Instrument*, const char* path);
void psy_audio_instrument_setname(psy_audio_Instrument*, const char* name);
void psy_audio_instrument_setindex(psy_audio_Instrument*, uintptr_t index);
// uintptr_t psy_audio_instrument_index(psy_audio_Instrument*);
const char* psy_audio_instrument_name(psy_audio_Instrument*);
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
/// [0..1.0f] Global volume affecting all samples of the instrument.
INLINE psy_dsp_amp_t psy_audio_instrument_volume(psy_audio_Instrument* self)
{
	return self->globalvolume;
}

const char* psy_audio_instrument_tostring(const psy_audio_Instrument*);
void psy_audio_instrument_fromstring(psy_audio_Instrument*, const char*);

// Properties

INLINE float psy_audio_instrument_volumefadespeed(const
	psy_audio_Instrument* self)
{
	return self->volumefadespeed;
}

INLINE void psy_audio_instrument_setvolumefadespeed(
	psy_audio_Instrument* self, float value)
{
	self->volumefadespeed = value;
}

INLINE float psy_audio_instrument_filtercutoff(
	const psy_audio_Instrument* self)
{
	return self->filtercutoff;
}

INLINE void psy_audio_instrument_setfiltercutoff(psy_audio_Instrument* self,
	float value)
{
	self->filtercutoff = value;
}

INLINE const float psy_audio_instrument_filterresonance(
	const psy_audio_Instrument* self)
{
	return self->filterres;
}

INLINE void psy_audio_instrument_setfilterresonance(psy_audio_Instrument* self,
	float value)
{
	self->filterres = value;
}

INLINE psy_dsp_FilterType psy_audio_instrument_filtertype(
	const psy_audio_Instrument* self)
{
	return self->filtertype;
}

INLINE void psy_audio_instrument_setfiltertype(psy_audio_Instrument* self,
	psy_dsp_FilterType value)
{
	self->filtertype = value;
}

INLINE float psy_audio_instrument_randomvolume(
	const psy_audio_Instrument* self)
{
	return self->randomvolume;
}

INLINE bool psy_audio_instrument_randomvolume_enabled(
	const psy_audio_Instrument* self)
{
	return self->randomvolume != 0.f;
}

INLINE void psy_audio_instrument_setrandomvolume(psy_audio_Instrument* self,
	float value)
{
	self->randomvolume = value;
}

INLINE float psy_audio_instrument_randompanning(
	const psy_audio_Instrument* self)
{
	return self->randompanning;
}

INLINE void psy_audio_instrument_setrandompanning(psy_audio_Instrument* self,
	float value)
{
	self->randompanning = value;
}

INLINE float psy_audio_instrument_randomcutoff(
	const psy_audio_Instrument* self)
{
	return self->randomcutoff;
}

INLINE void psy_audio_instrument_setrandomcutoff(psy_audio_Instrument* self,
	float value)
{
	self->randomcutoff = value;
}

INLINE float psy_audio_instrument_randomresonance(
	const psy_audio_Instrument* self)
{
	return self->randomresonance;
}

INLINE void psy_audio_instrument_setrandomresonance(psy_audio_Instrument* self,
	float value)
{
	self->randomresonance = value;
}

INLINE psy_audio_NewNoteAction psy_audio_instrument_nna(
	const psy_audio_Instrument* self)
{
	return self->nna;
}

INLINE void psy_audio_instrument_setnna(psy_audio_Instrument* self,
	psy_audio_NewNoteAction value)
{
	self->nna = value;
}

INLINE psy_audio_DupeCheck psy_audio_instrument_dct(
	const psy_audio_Instrument* self)
{
	return self->dct;
}

INLINE void psy_audio_instrument_setdct(psy_audio_Instrument* self,
	psy_audio_DupeCheck value)
{
	self->dct = value;
}

INLINE psy_audio_NewNoteAction psy_audio_instrument_dca(
	psy_audio_Instrument* self)
{
	return self->dca;
}

INLINE void psy_audio_instrument_setdca(psy_audio_Instrument* self,
	psy_audio_NewNoteAction value)
{
	self->dca = value;
}

INLINE float psy_audio_instrument_pan(const psy_audio_Instrument* self)
{
	return self->initpan;
}

INLINE void psy_audio_instrument_setpan(psy_audio_Instrument* self, float value)
{
	self->initpan = value;
}

INLINE bool psy_audio_instrument_panenabled(const psy_audio_Instrument* self)
{
	return self->panenabled;
}

INLINE void psy_audio_instrument_enablepan(psy_audio_Instrument* self)
{
	self->panenabled = TRUE;
}

INLINE void psy_audio_instrument_preventpan(psy_audio_Instrument* self)
{
	self->panenabled = FALSE;
}

///\return note number for center pan position
INLINE uint8_t psy_audio_instrument_notemodpancenter(const psy_audio_Instrument* self)
{
	return self->notemodpancenter;
}

/// sets note number for center pan position
INLINE void psy_audio_instrument_setnotemodpancenter(psy_audio_Instrument* self, uint8_t value)
{
	self->notemodpancenter = value;
}

///\return -32..32. 1/256th of panFactor change per seminote.
INLINE int8_t psy_audio_instrument_notemodpansep(const psy_audio_Instrument* self)
{
	return self->notemodpansep;
}

/// sets -32..32. 1/256th of panFactor change per seminote.
INLINE void psy_audio_instrument_setnotemodpansep(psy_audio_Instrument* self, int8_t value)
{
	self->notemodpansep = value;
}

#ifdef __cplusplus
}
#endif

#endif /* psy_audio_INSTRUMENT_H */

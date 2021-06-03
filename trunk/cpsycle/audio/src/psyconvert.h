/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net
*/

#ifndef psy_audio_PSYCONVERT_H
#define psy_audio_PSYCONVERT_H

#include "../../detail/psydef.h"

#include "instrument.h"
#include "pattern.h"
/* dsp */
#include <envelope.h>
#include <filter.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Compatibility methods/structs for song fileio */

/* Patterns */

#define PSY2_NOTECOMMANDS_EMPTY 255
#define PSY2_NOTECOMMANDS_RELEASE 120
#define PSY2_NOTECOMMANDS_TWEAK 121
#define PSY2_NOTECOMMANDS_MIDICC 123
#define PSY2_NOTECOMMANDS_TWEAKSLIDE 124

#define PSY2_EVENT_SIZE 5

typedef struct psy_audio_LegacyPatternEntry {
	uint8_t _note;
	uint8_t _inst;
	uint8_t _mach;
	uint8_t _cmd;
	uint8_t _parameter;
} psy_audio_LegacyPatternEntry;

INLINE void psy_audio_legacypatternentry_init_all(
	psy_audio_LegacyPatternEntry* self,
	uint8_t note, uint8_t inst, uint8_t machine, uint8_t cmd, uint8_t param)
{
	self->_note = note;
	self->_inst = inst;
	self->_mach = machine;
	self->_cmd = cmd;
	self->_parameter = param;
}

uint8_t _note;
uint8_t _inst;
uint8_t _mach;
uint8_t _cmd;
uint8_t _parameter;

typedef unsigned char* psy_audio_LegacyPattern;

/* legacy pattern */
psy_audio_LegacyPattern psy_audio_allocoldpattern(psy_audio_Pattern* pattern, uintptr_t lpb,
	int* rv_patternlines);
void psy_audio_convert_legacypattern(
	struct psy_audio_Pattern* dst, psy_audio_LegacyPattern src,
	uint32_t numtracks, uint32_t numrows,
	uint32_t lpb);

const psy_audio_LegacyPatternEntry* psy_audio_ptrackline_const(const
	psy_audio_LegacyPattern, int track, int line);
psy_audio_LegacyPatternEntry* psy_audio_ptrackline(psy_audio_LegacyPattern,
	int track, int line);

/* Instruments */

#define PSY3_LEGACY_MAX_INSTRUMENTS 256

struct psy_audio_Instrument;

typedef struct psy_audio_LegacyInstrument {
	/* 
	** \name Loop stuff
	** \{
	*/
	uint8_t _loop;
	int32_t _lines;
	/* \} */

	/* 
	** \verbatim
	** NNA values overview:
	** 
	** 0 = Note Cut      [Fast Release 'Default']
	** 1 = Note Release  [Release Stage]
	** 2 = Note Continue [No NNA]
	** \endverbatim
	*/
	uint8_t _NNA;

	int32_t sampler_to_use; /* Sampler machine index for lockinst. */
	uint8_t _LOCKINST;	/* Force this instrument number to change the selected machine to use a specific sampler when editing (i.e. when using the pc or midi keyboards, not the notes already existing in a pattern) */

	/*
	** \name Amplitude Envelope overview:
	** \{
	** Attack Time [in Samples at 44.1Khz, independently of the real samplerate]
	*/
	int32_t ENV_AT;
	/* Decay Time [in Samples at 44.1Khz, independently of the real samplerate] */
	int32_t ENV_DT;
	/* Sustain Level [in %] */
	int32_t ENV_SL;
	/* Release Time [in Samples at 44.1Khz, independently of the real samplerate] */
	int32_t ENV_RT;
	/* \} */

	/*
	** \name Filter 
	** \{
	** Attack Time [in Samples at 44.1Khz]
	*/
	int32_t ENV_F_AT;
	/* Decay Time [in Samples at 44.1Khz] */
	int32_t ENV_F_DT;
	/* Sustain Level [0..128] */
	int32_t ENV_F_SL;
	/* Release Time [in Samples at 44.1Khz] */
	int32_t ENV_F_RT;

	/* Cutoff Frequency [0-127] */
	int32_t ENV_F_CO;
	/* Resonance [0-127] */
	int32_t ENV_F_RQ;
	/* EnvAmount [-128,128] */  /* todo: is this right? Or 0 .. 128? */
	int32_t ENV_F_EA;
	/* Filter Type. See psycle::helpers::dsp::FilterType. [0..6] */
	psy_dsp_FilterType ENV_F_TP;
	/* \} */

	uint8_t _RPAN;
	uint8_t _RCUT;
	uint8_t _RRES;	
} psy_audio_LegacyInstrument;

void psy_audio_legacyinstrument_init(psy_audio_LegacyInstrument*);

psy_audio_LegacyInstrument psy_audio_legacyinstrument(const psy_audio_Instrument*);
void psy_audio_convert_legacy_to_instrument(struct psy_audio_Instrument* dst,
	psy_audio_LegacyInstrument src);


#define LEGACY_NOTE_MAP_SIZE 120

/* A Note pair(note number = first, and sample number = second) */
typedef struct LegacyNotePair {
	uint8_t first;
	uint8_t second;
} LegacyNotePair;

INLINE LegacyNotePair legacynotepair_make(uint8_t first, uint8_t second)
{
	LegacyNotePair rv;
	
	rv.first = first;
	rv.second = second;	
	return rv;
}

typedef struct LegacyNoteMap {
	LegacyNotePair map[LEGACY_NOTE_MAP_SIZE];
} LegacyNoteMap;

LegacyNoteMap psy_audio_legacynotemap(psy_List* entries);

#ifdef __cplusplus
}
#endif

#endif /* psy_audio_PSYCONVERT_H */

// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#if !defined(INSTRUMENT_H)
#define INSTRUMENT_H

#include "patternevent.h"
#include "samples.h"

#include <adsr.h>
#include <multifilter.h>
#include <list.h>

typedef enum {
	NNA_STOP = 0x0,		///  [Note Cut]	(This one actually does a very fast fadeout)
	NNA_CONTINUE = 0x1,	///  [Ignore]
	NNA_NOTEOFF = 0x2,	///  [Note off]
	NNA_FADEOUT = 0x3	///  [Note fade]
} psy_audio_NewNoteAction;

typedef struct {
	SampleIndex sampleindex;
	psy_audio_ParameterRange keyrange;
	psy_audio_ParameterRange velocityrange;
} psy_audio_InstrumentEntry;

void instrumententry_init(psy_audio_InstrumentEntry*);
psy_audio_InstrumentEntry* instrumententry_alloc(void);
psy_audio_InstrumentEntry* instrumententry_allocinit(void);

typedef struct {	
	char* name;
	/// Action to take on the playing voice when any new note comes in the same channel.
	psy_audio_NewNoteAction nna;
	psy_dsp_ADSRSettings volumeenvelope;
	psy_dsp_ADSRSettings filterenvelope;	
	float filtercutoff;	
	float filterres;		
	float filtermodamount;	
	FilterType filtertype;		
	unsigned char _RPAN;
	unsigned char _RCUT;
	unsigned char _RRES;
	psy_List* entries;
	uintptr_t index;
	psy_Signal signal_addentry;
	psy_Signal signal_removeentry;
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
psy_List* instrument_entriesintersect(psy_audio_Instrument*, uintptr_t key, uintptr_t velocity);
void instrument_clearentries(psy_audio_Instrument*);
void instrument_addentry(psy_audio_Instrument*, const psy_audio_InstrumentEntry* entry);
void instrument_removeentry(psy_audio_Instrument*, uintptr_t numentry);
psy_audio_InstrumentEntry* instrument_entryat(psy_audio_Instrument*, uintptr_t numentry);
const psy_List* instrument_entries(psy_audio_Instrument*);

#endif

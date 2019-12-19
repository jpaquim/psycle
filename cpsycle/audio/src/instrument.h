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
} NewNoteAction;

typedef struct {
	SampleIndex sampleindex;
	ParameterRange keyrange;
	ParameterRange velocityrange;
} InstrumentEntry;

void instrumententry_init(InstrumentEntry*);
InstrumentEntry* instrumententry_alloc(void);
InstrumentEntry* instrumententry_allocinit(void);

typedef struct {	
	char* name;
	/// Action to take on the playing voice when any new note comes in the same channel.
	NewNoteAction nna;
	ADSRSettings volumeenvelope;
	ADSRSettings filterenvelope;	
	float filtercutoff;	
	float filterres;		
	float filtermodamount;	
	FilterType filtertype;		
	unsigned char _RPAN;
	unsigned char _RCUT;
	unsigned char _RRES;
	List* entries;
	uintptr_t index;
	psy_Signal signal_addentry;
	psy_Signal signal_removeentry;
} Instrument;

void instrument_init(Instrument*);
void instrument_dispose(Instrument*);
Instrument* instrument_alloc(void);
Instrument* instrument_allocinit(void);
void instrument_load(Instrument*, const char* path);
void instrument_setname(Instrument*, const char* name);
void instrument_setindex(Instrument*, uintptr_t index);
uintptr_t instrument_index(Instrument*);
const char* instrument_name(Instrument*);
void instrument_setnna(Instrument*, NewNoteAction nna);
NewNoteAction instrument_nna(Instrument*);
List* instrument_entriesintersect(Instrument*, uintptr_t key, uintptr_t velocity);
void instrument_clearentries(Instrument*);
void instrument_addentry(Instrument*, const InstrumentEntry* entry);
void instrument_removeentry(Instrument*, uintptr_t numentry);
InstrumentEntry* instrument_entryat(Instrument*, uintptr_t numentry);
const List* instrument_entries(Instrument*);

#endif

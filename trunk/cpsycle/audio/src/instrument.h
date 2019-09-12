// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#if !defined(INSTRUMENT_H)
#define INSTRUMENT_H

#include <adsr.h>

typedef enum {
	NNA_STOP = 0x0,		///  [Note Cut]	(This one actually does a very fast fadeout)
	NNA_CONTINUE = 0x1,	///  [Ignore]
	NNA_NOTEOFF = 0x2,	///  [Note off]
	NNA_FADEOUT = 0x3	///  [Note fade]
} NewNoteAction;

typedef enum {
	F_LOWPASS12 = 0,
	F_HIGHPASS12 = 1,
	F_BANDPASS12 = 2,
	F_BANDREJECT12 = 3,
	F_NONE = 4,//This one is kept here because it is used in load/save. Also used in Sampulse instrument filter as "use channel default"
	F_ITLOWPASS = 5,
	F_MPTLOWPASSE = 6,
	F_MPTHIGHPASSE = 7,
	F_LOWPASS12E = 8,
	F_HIGHPASS12E = 9,
	F_BANDPASS12E = 10,
	F_BANDREJECT12E = 11,

	F_NUMFILTERS
} FilterType;

typedef struct {	
	char* name;
	/// Action to take on the playing voice when any new note comes in the same channel.
	NewNoteAction nna;
	ADSREnvelopeSettings volumeenvelope;
	ADSREnvelopeSettings filterenvelope;	
	float filtercutoff;	
	float filterres;		
	float filtermodamount;	
	FilterType filtertype;		
	unsigned char _RPAN;
	unsigned char _RCUT;
	unsigned char _RRES;
} Instrument;

void instrument_init(Instrument*);
void instrument_dispose(Instrument*);
void instrument_load(Instrument*, const char* path);
void instrument_setname(Instrument*, const char* name);
const char* instrument_name(Instrument*);
void instrument_setnna(Instrument*, NewNoteAction nna);
NewNoteAction instrument_nna(Instrument*);

#endif
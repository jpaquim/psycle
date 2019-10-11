// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#include "duplicator2.h"
#include "pattern.h"
#include <windows.h>

static void work(Duplicator2* self, BufferContext* bc) { }
static void sequencertick(Duplicator2*);
static List* sequencerinsert(Duplicator2*, List* events);
static const CMachineInfo* info(Duplicator2*);
static void parametertweak(Duplicator2*, int par, int val);
static int describevalue(Duplicator2*, char* txt, int const param, int const value);
static int value(Duplicator2*, int const param);
static void setvalue(Duplicator2* self, int const param, int const value) { }
static void dispose(Duplicator2*);
static unsigned int numinputs(Duplicator2* self) { return 0; }
static unsigned int numoutputs(Duplicator2* self) { return 0; }

static int transpose(int note, int offset);

static CMachineParameter const paraOutput0 = { "Output Machine 0", "Output Machine 0", -1,	0x7E, MPF_STATE, 0 };
static CMachineParameter const paraOutput1 = { "Output Machine 1", "Output Machine 1", -1,	0x7E, MPF_STATE, 0 };
static CMachineParameter const paraOutput2 = { "Output Machine 2", "Output Machine 2", -1,	0x7E, MPF_STATE, 0 };
static CMachineParameter const paraOutput3 = { "Output Machine 3", "Output Machine 3", -1,	0x7E, MPF_STATE, 0 };
static CMachineParameter const paraOutput4 = { "Output Machine 4", "Output Machine 4", -1,	0x7E, MPF_STATE, 0 };
static CMachineParameter const paraOutput5 = { "Output Machine 5", "Output Machine 5", -1,	0x7E, MPF_STATE, 0 };
static CMachineParameter const paraOutput6 = { "Output Machine 6", "Output Machine 6", -1,	0x7E, MPF_STATE, 0 };
static CMachineParameter const paraOutput7 = { "Output Machine 7", "Output Machine 7", -1,	0x7E, MPF_STATE, 0 };
static CMachineParameter const paraOffset0 = { "Note Offset 0", "Note Offset 0", -48, 48, MPF_STATE, 0 };
static CMachineParameter const paraOffset1 = { "Note Offset 1", "Note Offset 1", -48, 48, MPF_STATE, 0 };
static CMachineParameter const paraOffset2 = { "Note Offset 2", "Note Offset 2", -48, 48, MPF_STATE, 0 };
static CMachineParameter const paraOffset3 = { "Note Offset 3", "Note Offset 3", -48, 48, MPF_STATE, 0 };
static CMachineParameter const paraOffset4 = { "Note Offset 4", "Note Offset 4", -48, 48, MPF_STATE, 0 };
static CMachineParameter const paraOffset5 = { "Note Offset 5", "Note Offset 5", -48, 48, MPF_STATE, 0 };
static CMachineParameter const paraOffset6 = { "Note Offset 6", "Note Offset 6", -48, 48, MPF_STATE, 0 };
static CMachineParameter const paraOffset7 = { "Note Offset 7", "Note Offset 7", -48, 48, MPF_STATE, 0 };
static CMachineParameter const paraLowNote0 = { "Low Note 0", "Low Note 0", NOTECOMMANDS_C0, NOTECOMMANDS_B9, MPF_STATE, 0 };
static CMachineParameter const paraLowNote1 = { "Low Note 1", "Low Note 1", NOTECOMMANDS_C0, NOTECOMMANDS_B9, MPF_STATE, 0 };
static CMachineParameter const paraLowNote2 = { "Low Note 2", "Low Note 2", NOTECOMMANDS_C0, NOTECOMMANDS_B9, MPF_STATE, 0 };
static CMachineParameter const paraLowNote3 = { "Low Note 3", "Low Note 3", NOTECOMMANDS_C0, NOTECOMMANDS_B9, MPF_STATE, 0 };
static CMachineParameter const paraLowNote4 = { "Low Note 4", "Low Note 4", NOTECOMMANDS_C0, NOTECOMMANDS_B9, MPF_STATE, 0 };
static CMachineParameter const paraLowNote5 = { "Low Note 5", "Low Note 5", NOTECOMMANDS_C0, NOTECOMMANDS_B9, MPF_STATE, 0 };
static CMachineParameter const paraLowNote6 = { "Low Note 6", "Low Note 6", NOTECOMMANDS_C0, NOTECOMMANDS_B9, MPF_STATE, 0 };
static CMachineParameter const paraLowNote7 = { "Low Note 7", "Low Note 7", NOTECOMMANDS_C0, NOTECOMMANDS_B9, MPF_STATE, 0 };
static CMachineParameter const paraHighNote0 = { "High Note 0", "High Note 0", NOTECOMMANDS_C0, NOTECOMMANDS_B9, MPF_STATE, 0 };
static CMachineParameter const paraHighNote1 = { "High Note 1", "High Note 1", NOTECOMMANDS_C0, NOTECOMMANDS_B9, MPF_STATE, 0 };
static CMachineParameter const paraHighNote2 = { "High Note 2", "High Note 2", NOTECOMMANDS_C0, NOTECOMMANDS_B9, MPF_STATE, 0 };
static CMachineParameter const paraHighNote3 = { "High Note 3", "High Note 3", NOTECOMMANDS_C0, NOTECOMMANDS_B9, MPF_STATE, 0 };
static CMachineParameter const paraHighNote4 = { "High Note 4", "High Note 4", NOTECOMMANDS_C0, NOTECOMMANDS_B9, MPF_STATE, 0 };
static CMachineParameter const paraHighNote5 = { "High Note 5", "High Note 5", NOTECOMMANDS_C0, NOTECOMMANDS_B9, MPF_STATE, 0 };
static CMachineParameter const paraHighNote6 = { "High Note 6", "High Note 6", NOTECOMMANDS_C0, NOTECOMMANDS_B9, MPF_STATE, 0 };
static CMachineParameter const paraHighNote7 = { "High Note 7", "High Note 7", NOTECOMMANDS_C0, NOTECOMMANDS_B9, MPF_STATE, 0 };

static CMachineParameter const *pParameters[] = {
	&paraOutput0,
	&paraOutput1,	
	&paraOutput2,
	&paraOutput3,
	&paraOutput4,
	&paraOutput5,
	&paraOutput6,
	&paraOutput7,
	&paraOffset0,
	&paraOffset1,	
	&paraOffset2,
	&paraOffset3,
	&paraOffset4,
	&paraOffset5,
	&paraOffset6,
	&paraOffset7,
	&paraLowNote0,
	&paraLowNote1,	
	&paraLowNote2,
	&paraLowNote3,
	&paraLowNote4,
	&paraLowNote5,
	&paraLowNote6,
	&paraLowNote7,
	&paraHighNote0,
	&paraHighNote1,
	&paraHighNote2,
	&paraHighNote3,
	&paraHighNote4,
	&paraHighNote5,
	&paraHighNote6,
	&paraHighNote7,
};

static CMachineInfo const MacInfo = {
	MI_VERSION,
	0x0250,
	GENERATOR | 32 | 64,
	sizeof pParameters / sizeof *pParameters,
	pParameters,
	"Note Duplicator 2"
		#ifndef NDEBUG
		" (debug build)"
		#endif
		,
	"Note Duplicator 2",
	"Psycledelics",
	"help",
	4
};

const CMachineInfo* duplicator2_info(void)
{
	return &MacInfo;
}

void duplicator2_init(Duplicator2* self, MachineCallback callback)
{	
	int i;

	machine_init(&self->machine, callback);	
	self->machine.work = work;	
	self->machine.sequencertick = sequencertick;
	self->machine.sequencerinsert = sequencerinsert;
	self->machine.info = info;
	self->machine.parametertweak = parametertweak;
	self->machine.describevalue = describevalue;
	self->machine.setvalue = setvalue;
	self->machine.value = value;
	self->machine.dispose = dispose;
	self->machine.numinputs = numinputs;
	self->machine.numoutputs = numoutputs;
	for (i = 0; i < NUMMACHINES; ++i) {
		self->macoutput[i] = -1;
		self->noteoffset[i] = 0;
	}
	self->isticking = 0;
	duplicatormap_init(&self->map);
	memset(self->lowkey, 0, sizeof(self->lowkey));
	memset(self->highkey, 119, sizeof(self->highkey));
}

void dispose(Duplicator2* self)
{
	machine_dispose(&self->machine);
	duplicatormap_dispose(&self->map);
}

void sequencertick(Duplicator2* self)
{
	self->isticking = 0; // Prevent possible loops of Duplicators
}

List* sequencerinsert(Duplicator2* self, List* events)
{			
	List* p;	
	List* insert = 0;

	if (!self->isticking) {
		self->isticking = 1; // Prevent possible loops of Duplicators
		for (p = events; p != 0; p = p->next) {		
			int i;

			for (i = 0; i < NUMMACHINES; i++) {						
				if (self->macoutput[i] != -1) {					
					int note;
					PatternEntry* duplicatorentry;

					duplicatorentry = (PatternEntry*)p->entry;
					duplicatormap_allocate(&self->map, duplicatorentry->track, i,
						self->macoutput[i]);
					note = duplicatorentry->event.note;
					if (note < NOTECOMMANDS_RELEASE) {						
						note = transpose(note, self->noteoffset[i]);
					}					
					if ((note >= self->lowkey[i] && note <= self->highkey[i]) ||
							note >= NOTECOMMANDS_RELEASE) {
						PatternEntry* entry;						
							
						entry = patternentry_clone(duplicatorentry);
						entry->event.mach = self->macoutput[i];
						entry->event.note = note;
						entry->track = duplicatormap_at(&self->map, 
							duplicatorentry->track, i);												
						list_append(&insert, entry);												
					}
					if (duplicatorentry->event.note >= NOTECOMMANDS_RELEASE) {
						duplicatormap_remove(&self->map, duplicatorentry->track,
							i, self->macoutput[i]);
					}
				}
			}
		}
	}
	return insert;
}

int transpose(int note, int offset)
{
	int rv = note + offset;

	if (note >= NOTECOMMANDS_RELEASE) {
		rv = 119;
	} else
	if (note < 0) {
		rv = 0;
	}
	return rv;
}

const CMachineInfo* info(Duplicator2* self)
{	
	return &MacInfo;
}

void parametertweak(Duplicator2* self, int param, int value)
{
	if (param >= 0 && param < NUMMACHINES) {
		self->macoutput[param] = value;
	} else if (param >= NUMMACHINES && param < NUMMACHINES * 2) {
		self->noteoffset[param - NUMMACHINES] = value;			
	} else if (param >= NUMMACHINES && param < NUMMACHINES * 3) {
		self->lowkey[param - NUMMACHINES * 2] = value;			
	} if (param >= NUMMACHINES && param< NUMMACHINES * 4) {
		self->highkey[param - NUMMACHINES * 3] = value;			
	}			
}

int describevalue(Duplicator2* self, char* txt, int const param, int const value)
{ 
	return 0;
}

int value(Duplicator2* self, int const param)
{	
	if (param >= 0 && param < NUMMACHINES) {
		return self->macoutput[param];
	} else if (param >= NUMMACHINES && param < NUMMACHINES * 2) {
		return self->noteoffset[param - NUMMACHINES];
	} else if (param >= NUMMACHINES && param < NUMMACHINES * 3) {
		return self->lowkey[param - NUMMACHINES * 2];
	} else if (param >= NUMMACHINES && param < NUMMACHINES * 4) {
		return self->highkey[param - NUMMACHINES * 3];
	}
	return 0;
}

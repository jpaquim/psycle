// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "duplicator2.h"
#include "pattern.h"
#include "songio.h"
#include <string.h>
#include <stdlib.h>
#include <portable.h>

static void work(Duplicator2* self, BufferContext* bc) { }
static void sequencertick(Duplicator2*);
static List* sequencerinsert(Duplicator2*, List* events);
static const MachineInfo* info(Duplicator2*);
static void parametertweak(Duplicator2*, int par, int val);
static int describevalue(Duplicator2*, char* txt, int param, int value);
static int parametervalue(Duplicator2*, int param);
static void parameterrange(Duplicator2*, int param, int* minval, int* maxval);
static int parameterlabel(Duplicator2*, char* txt, int param);
static int parametername(Duplicator2*, char* txt, int param);
static unsigned int numparameters(Duplicator2*);
static unsigned int numparametercols(Duplicator2*);
static void dispose(Duplicator2*);
static unsigned int numinputs(Duplicator2* self) { return 0; }
static unsigned int numoutputs(Duplicator2* self) { return 0; }
static void loadspecific(Duplicator2*, struct SongFile*, unsigned int slot);
static void savespecific(Duplicator2*, struct SongFile*, unsigned int slot);

static int transpose(int note, int offset);

static MachineInfo const MacInfo = {
	MI_VERSION,
	0x0250,
	GENERATOR | 32 | 64,	
	"Note Duplicator 2"
		#ifndef NDEBUG
		" (debug build)"
		#endif
		,
	"Note Duplicator 2",
	"Psycledelics",
	"help",	
	MACH_DUPLICATOR2
};

const MachineInfo* duplicator2_info(void)
{
	return &MacInfo;
}

void duplicator2_init(Duplicator2* self, MachineCallback callback)
{	
	Machine* base = (Machine*)self;
	int i;	

	custommachine_init(&self->custommachine, callback);
	base->work = work;	
	base->sequencertick = sequencertick;
	base->sequencerinsert = sequencerinsert;
	base->info = info;
	base->parametertweak = parametertweak;
	base->describevalue = describevalue;	
	base->parametervalue = parametervalue;
	base->dispose = dispose;
	base->numinputs = numinputs;
	base->numoutputs = numoutputs;
	base->parameterrange = parameterrange;
	base->numparameters = numparameters;
	base->numparametercols = numparametercols;
	base->parameterlabel = parameterlabel;
	base->parametername = parametername;	
	base->loadspecific = loadspecific;
	base->savespecific = savespecific;
	for (i = 0; i < 16; ++i) {
		self->macoutput[i] = -1;
		self->noteoffset[i] = 0;
		self->lowkey[i] = NOTECOMMANDS_C0;
		self->highkey[i] = NOTECOMMANDS_B9;
	}
	self->isticking = 0;
	duplicatormap_init(&self->map);	
	base->seteditname(base, "Note Duplicator 2");	
}

void dispose(Duplicator2* self)
{	
	duplicatormap_dispose(&self->map);
	custommachine_dispose(&self->custommachine);
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

			for (i = 0; i < 16; i++) {						
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

const MachineInfo* info(Duplicator2* self)
{	
	return &MacInfo;
}

void parametertweak(Duplicator2* self, int param, int value)
{
	if (param >= 0 && param < 16) {
		self->macoutput[param] = value;
	} else if (param >= 16 && param < 16 * 2) {
		self->noteoffset[param - 16] = value;			
	} else if (param >= 16 && param < 16 * 3) {
		self->lowkey[param - 16 * 2] = value;			
	} if (param >= 16 && param< 16 * 4) {
		self->highkey[param - 16 * 3] = value;			
	}			
}

int describevalue(Duplicator2* self, char* txt, int param, int value)
{ 
	return 0;
}

int parametervalue(Duplicator2* self, int param)
{	
	if (param >= 0 && param < 16) {
		return self->macoutput[param];
	} else if (param >= 16 && param < 16 * 2) {
		return self->noteoffset[param - 16];
	} else if (param >= 16 && param < 16 * 3) {
		return self->lowkey[param - 16 * 2];
	} else if (param >= 16 && param < 16 * 4) {
		return self->highkey[param - 16 * 3];
	}
	return 0;
}

void parameterrange(Duplicator2* self, int param, int* minval, int* maxval)
{
	if (param < 16) {
		*minval = -1;
		*maxval = 0x7E;
	} else
	if (param < 2 * 16) {
		*minval = -48;
		*maxval = 48;
	} else	
	if (param < 4 * 16) {
		*minval = NOTECOMMANDS_C0;
		*maxval = NOTECOMMANDS_B9;
	}
}

int parameterlabel(Duplicator2* self, char* txt, int param)
{
	return parametername(self, txt, param);
}

int parametername(Duplicator2* self, char* txt, int param)
{
	txt[0] = '\0';
	if (param < 16) {
		psy_snprintf(txt, 128, "%s %d", "Output Machine ", param);
	} else 
	if (param < 2 * 16) {
		psy_snprintf(txt, 128, "%s %d", "Note Offset ", param);
	} else
	if (param < 3 * 16) {
		psy_snprintf(txt, 128, "%s %d", "Low Note ", param);
		
	} else
	if (param < 4 * 16) {
		psy_snprintf(txt, 128, "%s %d", "High Note ", param);
	}
	return 1;
}

unsigned int numparameters(Duplicator2* self)
{
	return 16 * 4;
}

unsigned int numparametercols(Duplicator2* self)
{
	return 4;	
}

void loadspecific(Duplicator2* self, struct SongFile* songfile, unsigned int slot)
{
	uint32_t size;
	
	psyfile_read(songfile->file, &size, sizeof size); // size of this part params to load
	//TODO: endianess
	psyfile_read(songfile->file, self->macoutput, 16 * sizeof(short));
	psyfile_read(songfile->file, self->noteoffset, 16 * sizeof(short));
	psyfile_read(songfile->file, self->lowkey, 16 * sizeof(short));
	psyfile_read(songfile->file, self->highkey, 16 * sizeof(short));	
}

void savespecific(Duplicator2* self, struct SongFile* songfile, unsigned int slot)
{
	uint32_t size;
	
	size = sizeof self->macoutput + sizeof self->noteoffset + sizeof self->lowkey + sizeof self->highkey;
	psyfile_write(songfile->file, &size, sizeof size); // size of this part params to save
	//TODO: endianess
	psyfile_write(songfile->file, &self->macoutput[0], 16 * sizeof(short));
	psyfile_write(songfile->file, &self->noteoffset[0], 16 * sizeof(short));
	psyfile_write(songfile->file, &self->lowkey[0], 16 * sizeof(short));
	psyfile_write(songfile->file, &self->highkey[0], 16 * sizeof(short));
}

// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "duplicator2.h"
#include "pattern.h"
#include "songio.h"
#include <string.h>
#include <stdlib.h>
#include <portable.h>

static void work(psy_audio_Duplicator2* self, psy_audio_BufferContext* bc) { }
static void sequencertick(psy_audio_Duplicator2*);
static psy_List* sequencerinsert(psy_audio_Duplicator2*, psy_List* events);
static const psy_audio_MachineInfo* info(psy_audio_Duplicator2*);
static void parametertweak(psy_audio_Duplicator2*, int par, int val);
static int describevalue(psy_audio_Duplicator2*, char* txt, int param, int value);
static int parametervalue(psy_audio_Duplicator2*, int param);
static void parameterrange(psy_audio_Duplicator2*, int param, int* minval, int* maxval);
static int parameterlabel(psy_audio_Duplicator2*, char* txt, int param);
static int parametername(psy_audio_Duplicator2*, char* txt, int param);
static unsigned int numparameters(psy_audio_Duplicator2*);
static unsigned int numparametercols(psy_audio_Duplicator2*);
static void dispose(psy_audio_Duplicator2*);
static unsigned int numinputs(psy_audio_Duplicator2* self) { return 0; }
static unsigned int numoutputs(psy_audio_Duplicator2* self) { return 0; }
static void loadspecific(psy_audio_Duplicator2*, struct psy_audio_SongFile*, unsigned int slot);
static void savespecific(psy_audio_Duplicator2*, struct psy_audio_SongFile*, unsigned int slot);

static int transpose(int note, int offset);

static psy_audio_MachineInfo const MacInfo = {
	MI_VERSION,
	0x0250,
	GENERATOR | 32 | 64,
	MACHMODE_GENERATOR,
	"Duplicator 2"
		#ifndef NDEBUG
		" (debug build)"
		#endif
		,	
	"Duplicator 2",
	"Psycledelics",
	"help",	
	MACH_DUPLICATOR2
};

const psy_audio_MachineInfo* duplicator2_info(void)
{
	return &MacInfo;
}

static MachineVtable vtable;
static int vtable_initialized = 0;

static void vtable_init(psy_audio_Duplicator2* self)
{
	if (!vtable_initialized) {
		vtable = *self->custommachine.machine.vtable;		
		vtable.work = (fp_machine_work) work;	
		vtable.info = (fp_machine_info) info;
		vtable.sequencertick = (fp_machine_sequencertick) sequencertick;
		vtable.sequencerinsert = (fp_machine_sequencerinsert) sequencerinsert;	
		vtable.parametertweak = (fp_machine_parametertweak) parametertweak;
		vtable.describevalue = (fp_machine_describevalue) describevalue;	
		vtable.parametervalue = (fp_machine_parametervalue) parametervalue;
		vtable.describevalue = (fp_machine_describevalue) describevalue;
		vtable.parameterrange = (fp_machine_parameterrange) parameterrange;
		vtable.numparameters = (fp_machine_numparameters) numparameters;
		vtable.numparametercols = (fp_machine_numparametercols) numparametercols;
		vtable.parameterlabel = (fp_machine_parameterlabel) parameterlabel;
		vtable.parametername = (fp_machine_parametername) parametername;
		vtable.dispose = (fp_machine_dispose) dispose;
		vtable.numinputs = (fp_machine_numinputs) numinputs;
		vtable.numoutputs = (fp_machine_numoutputs) numoutputs;	
		vtable.loadspecific = (fp_machine_loadspecific) loadspecific;
		vtable.savespecific = (fp_machine_savespecific) savespecific;
		vtable_initialized = 1;
	}
}

void duplicator2_init(psy_audio_Duplicator2* self, MachineCallback callback)
{	
	psy_audio_Machine* base = (psy_audio_Machine*)self;
	int i;	

	custommachine_init(&self->custommachine, callback);
	vtable_init(self);
	self->custommachine.machine.vtable = &vtable;
	for (i = 0; i < 16; ++i) {
		self->macoutput[i] = -1;
		self->noteoffset[i] = 0;
		self->lowkey[i] = NOTECOMMANDS_C0;
		self->highkey[i] = NOTECOMMANDS_B9;
	}
	self->isticking = 0;
	duplicatormap_init(&self->map);	
	base->vtable->seteditname(base, "Note psy_audio_Duplicator 2");	
}

void dispose(psy_audio_Duplicator2* self)
{	
	duplicatormap_dispose(&self->map);
	custommachine_dispose(&self->custommachine);
}

void sequencertick(psy_audio_Duplicator2* self)
{
	self->isticking = 0; // Prevent possible loops of Duplicators
}

psy_List* sequencerinsert(psy_audio_Duplicator2* self, psy_List* events)
{			
	psy_List* p;	
	psy_List* insert = 0;

	if (!self->isticking) {
		self->isticking = 1; // Prevent possible loops of Duplicators
		for (p = events; p != 0; p = p->next) {		
			int i;

			for (i = 0; i < 16; i++) {						
				if (self->macoutput[i] != -1) {					
					int note;
					psy_audio_PatternEntry* duplicatorentry;

					duplicatorentry = (psy_audio_PatternEntry*)p->entry;
					duplicatormap_allocate(&self->map, duplicatorentry->track, i,
						self->macoutput[i]);
					note = duplicatorentry->event.note;
					if (note < NOTECOMMANDS_RELEASE) {						
						note = transpose(note, self->noteoffset[i]);
					}					
					if ((note >= self->lowkey[i] && note <= self->highkey[i]) ||
							note >= NOTECOMMANDS_RELEASE) {
						psy_audio_PatternEntry* entry;						
							
						entry = patternentry_clone(duplicatorentry);
						entry->event.mach = self->macoutput[i];
						entry->event.note = note;
						entry->track = duplicatormap_at(&self->map, 
							duplicatorentry->track, i);												
						psy_list_append(&insert, entry);												
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

const psy_audio_MachineInfo* info(psy_audio_Duplicator2* self)
{	
	return &MacInfo;
}

void parametertweak(psy_audio_Duplicator2* self, int param, int value)
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

int describevalue(psy_audio_Duplicator2* self, char* txt, int param, int value)
{ 
	return 0;
}

int parametervalue(psy_audio_Duplicator2* self, int param)
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

void parameterrange(psy_audio_Duplicator2* self, int param, int* minval, int* maxval)
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

int parameterlabel(psy_audio_Duplicator2* self, char* txt, int param)
{
	return parametername(self, txt, param);
}

int parametername(psy_audio_Duplicator2* self, char* txt, int param)
{
	txt[0] = '\0';
	if (param < 16) {
		psy_snprintf(txt, 128, "%s %d", "Output psy_audio_Machine ", param);
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

unsigned int numparameters(psy_audio_Duplicator2* self)
{
	return 16 * 4;
}

unsigned int numparametercols(psy_audio_Duplicator2* self)
{
	return 4;	
}

void loadspecific(psy_audio_Duplicator2* self, struct psy_audio_SongFile* songfile, unsigned int slot)
{
	uint32_t size;
	
	psyfile_read(songfile->file, &size, sizeof size); // size of this part params to load
	//TODO: endianess
	psyfile_read(songfile->file, self->macoutput, 16 * sizeof(short));
	psyfile_read(songfile->file, self->noteoffset, 16 * sizeof(short));
	psyfile_read(songfile->file, self->lowkey, 16 * sizeof(short));
	psyfile_read(songfile->file, self->highkey, 16 * sizeof(short));	
}

void savespecific(psy_audio_Duplicator2* self, struct psy_audio_SongFile* songfile, unsigned int slot)
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

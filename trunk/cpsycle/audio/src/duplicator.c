// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "duplicator.h"
#include "pattern.h"
#include <string.h>
#include <stdlib.h>
#include "songio.h"
#include <portable.h>

static void work(psy_audio_Duplicator* self, psy_audio_BufferContext* bc) { }
static void sequencertick(psy_audio_Duplicator*);
static psy_List* sequencerinsert(psy_audio_Duplicator*, psy_List* events);
static const psy_audio_MachineInfo* info(psy_audio_Duplicator*);
static void parametertweak(psy_audio_Duplicator*, int par, int val);
static int describevalue(psy_audio_Duplicator*, char* txt, int param, int value);
static int parametervalue(psy_audio_Duplicator*, int param);
static void parameterrange(psy_audio_Duplicator*, int param, int* minval, int* maxval);
static int parameterlabel(psy_audio_Duplicator*, char* txt, int param);
static int parametername(psy_audio_Duplicator*, char* txt, int param);
static uintptr_t numparameters(psy_audio_Duplicator*);
static unsigned int numparametercols(psy_audio_Duplicator*);
static void dispose(psy_audio_Duplicator*);
static uintptr_t numinputs(psy_audio_Duplicator* self) { return 0; }
static uintptr_t numoutputs(psy_audio_Duplicator* self) { return 0; }
static void loadspecific(psy_audio_Duplicator*, struct psy_audio_SongFile*,
	uintptr_t slot);
static void savespecific(psy_audio_Duplicator*, struct psy_audio_SongFile*,
	uintptr_t slot);

static int transpose(int note, int offset);

static psy_audio_MachineInfo const MacInfo = {
	MI_VERSION,
	0x0250,
	GENERATOR | 32 | 64,
	MACHMODE_GENERATOR,
	"Duplicator"
		#ifndef NDEBUG
		" (debug build)"
		#endif
		,
	"Duplicator",
	"Psycledelics",
	"help",	
	MACH_DUPLICATOR
};

const psy_audio_MachineInfo* duplicator_info(void)
{
	return &MacInfo;
}

static MachineVtable vtable;
static int vtable_initialized = 0;

static void vtable_init(psy_audio_Duplicator* self)
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

void duplicator_init(psy_audio_Duplicator* self, MachineCallback callback)
{	
	psy_audio_Machine* base = &self->custommachine.machine;
	int i;

	custommachine_init(&self->custommachine, callback);
	vtable_init(self);
	self->custommachine.machine.vtable = &vtable;
	for (i = 0; i < NUMMACHINES; ++i) {
		self->macoutput[i] = -1;
		self->noteoffset[i] = 0;
	}
	self->isticking = 0;
	base->vtable->seteditname(base, "Note psy_audio_Duplicator");
	duplicatormap_init(&self->map);
}

void dispose(psy_audio_Duplicator* self)
{		
	duplicatormap_dispose(&self->map);
	custommachine_dispose(&self->custommachine);
}

void sequencertick(psy_audio_Duplicator* self)
{
	self->isticking = 0; // Prevent possible loops of Duplicators
}

psy_List* sequencerinsert(psy_audio_Duplicator* self, psy_List* events)
{			
	psy_List* p;	
	psy_List* insert = 0;

	if (!self->isticking) {
		self->isticking = 1; // Prevent possible loops of Duplicators
		for (p = events; p != 0; p = p->next) {		
			int i;

			for (i = 0; i < NUMMACHINES; i++) {						
				if (self->macoutput[i] != -1) {
					int note;
					psy_audio_PatternEntry* duplicatorentry;
					psy_audio_PatternEntry* entry;

					duplicatorentry = (psy_audio_PatternEntry*)p->entry;
					duplicatormap_allocate(&self->map, duplicatorentry->track, i,
						self->macoutput[i]);
					note = duplicatorentry->event.note;
					if (note < NOTECOMMANDS_RELEASE) {						
						note = transpose(note, self->noteoffset[i]);
					}
					entry = patternentry_clone(duplicatorentry);
					if (entry) {						
						entry->event.mach = self->macoutput[i];
						entry->event.note = note;						
						entry->track = duplicatormap_at(&self->map,
							duplicatorentry->track, i);						 						
						psy_list_append(&insert, entry);						
					}
					if (entry->event.note >= NOTECOMMANDS_RELEASE) {
						duplicatormap_remove(&self->map, duplicatorentry->track, i,
							self->macoutput[i]);						
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

const psy_audio_MachineInfo* info(psy_audio_Duplicator* self)
{	
	return &MacInfo;
}

void parametertweak(psy_audio_Duplicator* self, int param, int value)
{
	if (param >= 0 && param < NUMMACHINES) {
		self->macoutput[param] = value;
	} else
	if (param >= NUMMACHINES && param < NUMMACHINES * 2) {	
		self->noteoffset[param - NUMMACHINES] = value;
	}

}

int describevalue(psy_audio_Duplicator* self, char* txt, int param, int value)
{ 
	return 0;
}

int parametervalue(psy_audio_Duplicator* self, int param)
{	
	if (param >= 0 && param < NUMMACHINES) {
		return self->macoutput[param];
	} else
	if (param >= NUMMACHINES && param < NUMMACHINES * 2) {
		return self->noteoffset[param - NUMMACHINES];
	}
	return 0;
}

void parameterrange(psy_audio_Duplicator* self, int param, int* minval, int* maxval)
{
	if (param < 8) {
		*minval = -1;
		*maxval = 0x7E;
	} else {
		*minval = -48;
		*maxval = 48;
	}
}

int parameterlabel(psy_audio_Duplicator* self, char* txt, int param)
{
	return parametername(self, txt, param);
}

int parametername(psy_audio_Duplicator* self, char* txt, int param)
{
	txt[0] = '\0';
	if (param < 8) {
		psy_snprintf(txt, 128, "%s %d", "Output psy_audio_Machine ", param);
	} else {
		psy_snprintf(txt, 128, "%s %d", "Note Offset ", param);
	}
	return 1;
}

uintptr_t numparameters(psy_audio_Duplicator* self)
{
	return 16;
}

unsigned int numparametercols(psy_audio_Duplicator* self)
{
	return 2;	
}

void loadspecific(psy_audio_Duplicator* self, struct psy_audio_SongFile* songfile,
	uintptr_t slot)
{
	uint32_t size;
	psyfile_read(songfile->file, &size, sizeof size); // size of this part params to load
	//TODO: endianess
	psyfile_read(songfile->file, &self->macoutput[0], NUMMACHINES * sizeof(short));
	psyfile_read(songfile->file, &self->noteoffset[0], NUMMACHINES * sizeof(short));	
}

void savespecific(psy_audio_Duplicator* self, struct psy_audio_SongFile* songfile,
	uintptr_t slot)
{
	uint32_t size;
	
	size = sizeof self->macoutput + sizeof self->noteoffset;
	psyfile_write(songfile->file, &size, sizeof size); // size of this part params to save
	//TODO: endianess
	psyfile_write(songfile->file, &self->macoutput[0], NUMMACHINES * sizeof(short));
	psyfile_write(songfile->file, &self->noteoffset[0], NUMMACHINES * sizeof(short));
}

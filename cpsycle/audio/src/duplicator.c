// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "duplicator.h"
#include "pattern.h"
#include <string.h>
#include <stdlib.h>
#include "songio.h"
#include "constants.h"
#include "../../detail/portable.h"
#include "plugin_interface.h"

// TODO: change will break songfile load/save
#define DUPLICATOR_NUMOUTPUTS 8

static void dispose(psy_audio_Duplicator*);
static void work(psy_audio_Duplicator* self, psy_audio_BufferContext* bc) { }
static void sequencertick(psy_audio_Duplicator*);
static psy_List* sequencerinsert(psy_audio_Duplicator*, psy_audio_PatternNode* events);
static const psy_audio_MachineInfo* info(psy_audio_Duplicator*);
static psy_audio_MachineParam* parameter(psy_audio_Duplicator*, uintptr_t param);
static uintptr_t numparameters(psy_audio_Duplicator*);
static unsigned int numparametercols(psy_audio_Duplicator*);
static uintptr_t numinputs(psy_audio_Duplicator* self) { return 0; }
static uintptr_t numoutputs(psy_audio_Duplicator* self) { return 0; }
static int loadspecific(psy_audio_Duplicator*, psy_audio_SongFile*,
	uintptr_t slot);
static int savespecific(psy_audio_Duplicator*, psy_audio_SongFile*,
	uintptr_t slot);
static void stop(psy_audio_Duplicator*);

static int transpose(int note, int offset);
static void initparameters(psy_audio_Duplicator*);
static void disposeparameters(psy_audio_Duplicator*);

static psy_audio_MachineInfo const macinfo = {
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
	MACH_DUPLICATOR,
	0,
	0,
	""
};

const psy_audio_MachineInfo* psy_audio_duplicator_info(void)
{
	return &macinfo;
}

static MachineVtable vtable;
static int vtable_initialized = 0;

static void vtable_init(psy_audio_Duplicator* self)
{
	if (!vtable_initialized) {
		vtable = *self->custommachine.machine.vtable;
		vtable.work = (fp_machine_work)work;	
		vtable.info = (fp_machine_info)info;
		vtable.sequencertick = (fp_machine_sequencertick)sequencertick;
		vtable.sequencerinsert = (fp_machine_sequencerinsert)sequencerinsert;
		vtable.parameter = (fp_machine_parameter)parameter;
		vtable.numparameters = (fp_machine_numparameters)numparameters;
		vtable.numparametercols = (fp_machine_numparametercols)
			numparametercols;
		vtable.dispose = (fp_machine_dispose)dispose;
		vtable.numinputs = (fp_machine_numinputs)numinputs;
		vtable.numoutputs = (fp_machine_numoutputs)numoutputs;	
		vtable.loadspecific = (fp_machine_loadspecific)loadspecific;
		vtable.savespecific = (fp_machine_savespecific)savespecific;
		vtable.stop = (fp_machine_stop)stop;
		vtable_initialized = 1;
	}
}

void psy_audio_duplicator_init(psy_audio_Duplicator* self,
	psy_audio_MachineCallback* callback)
{
	psy_audio_custommachine_init(&self->custommachine, callback);
	vtable_init(self);
	psy_audio_duplicator_base(self)->vtable = &vtable;
	self->isticking = 0;
	psy_audio_machine_seteditname(psy_audio_duplicator_base(self), "Note Duplicator");
	psy_audio_duplicatormap_init(&self->map, DUPLICATOR_NUMOUTPUTS,
		MAX_TRACKS);
	initparameters(self);
}

void dispose(psy_audio_Duplicator* self)
{
	assert(self);
	
	disposeparameters(self);
	psy_audio_custommachine_dispose(&self->custommachine);
	psy_audio_duplicatormap_dispose(&self->map);
}

void initparameters(psy_audio_Duplicator* self)
{
	int gbp;

	psy_table_init(&self->parameters);
	for (gbp = 0; gbp < (int)psy_audio_duplicatormap_numoutputs(&self->map); ++gbp) {
		char name[128];
		psy_audio_IntMachineParam* param;
		psy_audio_DuplicatorOutput* output;
				
		output = psy_audio_duplicatormap_output(&self->map, gbp);
		if (output) {
			psy_snprintf(name, 128, "%s %d", "Output Machine ", gbp);
			param = psy_audio_intmachineparam_allocinit(name, name, MPF_STATE,
				(int32_t*)&output->machine, -1, 0x7E);
			psy_table_insert(&self->parameters, gbp, (void*)param);
			psy_snprintf(name, 128, "%s %d", "Note Offset ", gbp);
			param = psy_audio_intmachineparam_allocinit(name, name, MPF_STATE,
				&output->offset, -48, 48);
			psy_table_insert(&self->parameters,
				psy_audio_duplicatormap_numoutputs(&self->map) + gbp,
				(void*)param);
		}
	}	
}

void disposeparameters(psy_audio_Duplicator* self)
{
	psy_table_disposeall(&self->parameters, (psy_fp_disposefunc)
		psy_audio_intmachineparam_dispose);	
}

void sequencertick(psy_audio_Duplicator* self)
{
	// Prevent possible loops of Duplicators
	// isticking = 0: allows duplicator to enter notes
	self->isticking = 0;
}

psy_List* sequencerinsert(psy_audio_Duplicator* self, psy_audio_PatternNode* events)
{			
	psy_List* p;	
	psy_List* insert = 0;

	if (!self->isticking) {
		// isticking = 1, prevents for this tick duplicator to insert further
		// notes than these ones to avoid possible loops of duplicators
		self->isticking = 1;
		for (p = events; p != NULL; psy_list_next(&p)) {
			psy_TableIterator it;
	
			for (it = psy_audio_duplicatormap_begin(&self->map);
					!psy_tableiterator_equal(&it, psy_table_end());
					psy_tableiterator_inc(&it)) {			
				psy_audio_DuplicatorOutput* output;

				output = (psy_audio_DuplicatorOutput*) psy_tableiterator_value(
					&it);
				if (output->machine != -1) {					
					psy_audio_PatternEntry* patternentry;					
					psy_audio_PatternEntry* newentry;
					int note;
					uintptr_t mapchannel;
					patternentry = psy_audio_patternnode_entry(p);
					newentry = psy_audio_patternentry_clone(patternentry);
					note = psy_audio_patternentry_front(patternentry)->note;					
					if (note < psy_audio_NOTECOMMANDS_RELEASE) {						
						note = transpose(note, output->offset);
					}
					mapchannel = psy_audio_duplicatormap_channel(&self->map,
						patternentry->track, output);
					if (newentry) {						
						psy_audio_patternentry_front(newentry)->mach = output->machine;
						psy_audio_patternentry_front(newentry)->note = note;						
						newentry->track = mapchannel;
						psy_list_append(&insert, newentry);
					}
					if (psy_audio_patternentry_front(patternentry)->note >= 
							psy_audio_NOTECOMMANDS_RELEASE) {
						psy_audio_duplicatormap_release(&self->map,
							patternentry->track, mapchannel, output);
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

	if (note >= psy_audio_NOTECOMMANDS_RELEASE) {
		rv = psy_audio_NOTECOMMANDS_B9;
	} else
	if (note < 0) {
		rv = psy_audio_NOTECOMMANDS_C0;
	}
	return rv;
}

const psy_audio_MachineInfo* info(psy_audio_Duplicator* self)
{	
	return &macinfo;
}

uintptr_t numparameters(psy_audio_Duplicator* self)
{
	return psy_audio_duplicatormap_numoutputs(&self->map) *
		numparametercols(self);
}

unsigned int numparametercols(psy_audio_Duplicator* self)
{
	return 2;	
}

int loadspecific(psy_audio_Duplicator* self, psy_audio_SongFile* songfile,
	uintptr_t slot)
{
	uint32_t size;
	int32_t i;
	int16_t macoutput[DUPLICATOR_NUMOUTPUTS];
	int16_t noteoffset[DUPLICATOR_NUMOUTPUTS];
	int status;

	// size of this part params to load
	if (status = psyfile_read(songfile->file, &size, sizeof(size))) {
		return status;
	}
	// TODO: endianess
	if (status = psyfile_read(songfile->file, &macoutput[0],
		DUPLICATOR_NUMOUTPUTS * sizeof(int16_t))) {
		return status;
	}
	if (status = psyfile_read(songfile->file, &noteoffset[0],
		DUPLICATOR_NUMOUTPUTS * sizeof(int16_t))) {
		return status;
	}
	for (i = 0; i < DUPLICATOR_NUMOUTPUTS; ++i) {
		psy_audio_DuplicatorOutput* output;

		output = psy_audio_duplicatormap_output(&self->map, i);			
		if (output) {
			output->machine = macoutput[i];
			output->offset = noteoffset[i];
		}
	}
	disposeparameters(self);
	initparameters(self);
	return PSY_OK;
}

int savespecific(psy_audio_Duplicator* self, psy_audio_SongFile* songfile,
	uintptr_t slot)
{
	uint32_t size;
	int16_t macoutput[DUPLICATOR_NUMOUTPUTS];
	int16_t noteoffset[DUPLICATOR_NUMOUTPUTS];
	int32_t i;
	int status;
	
	for (i = 0; i < DUPLICATOR_NUMOUTPUTS; ++i) {
		psy_audio_DuplicatorOutput* output;

		output = psy_audio_duplicatormap_output(&self->map, i);
		macoutput[i] = output->machine;
		noteoffset[i] = output->offset;
	}
	size = sizeof(macoutput) + sizeof(noteoffset);
	// size of this part params to save
	if (status = psyfile_write(songfile->file, &size, sizeof(size))) {
		return status;
	}
	// TODO: endianess
	if (status = psyfile_write(songfile->file, &macoutput[0], DUPLICATOR_NUMOUTPUTS *
			sizeof(int16_t))) {
		return status;
	}
	if (status = psyfile_write(songfile->file, &noteoffset[0], DUPLICATOR_NUMOUTPUTS *
			sizeof(int16_t))) {
		return status;
	}
	return PSY_OK;
}

void stop(psy_audio_Duplicator* self)
{
	psy_audio_duplicatormap_clear(&self->map);
}

psy_audio_MachineParam* parameter(psy_audio_Duplicator* self, uintptr_t param)
{
	return (psy_audio_MachineParam*) psy_table_at(&self->parameters, param);
}

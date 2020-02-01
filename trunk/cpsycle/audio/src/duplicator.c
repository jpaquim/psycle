// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

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

static void work(psy_audio_Duplicator* self, psy_audio_BufferContext* bc) { }
static void sequencertick(psy_audio_Duplicator*);
static psy_List* sequencerinsert(psy_audio_Duplicator*, PatternNode* events);
static const psy_audio_MachineInfo* info(psy_audio_Duplicator*);
static void parametertweak(psy_audio_Duplicator*, uintptr_t param, float val);
static int describevalue(psy_audio_Duplicator*, char* rv, uintptr_t param,
	int value);
static float parametervalue(psy_audio_Duplicator*, uintptr_t param);
static void parameterrange(psy_audio_Duplicator*, uintptr_t param, int* minval,
	int* maxval);
static int parameterlabel(psy_audio_Duplicator*, char* rv, uintptr_t param);
static int parametername(psy_audio_Duplicator*, char* rv, uintptr_t param);
static uintptr_t numparameters(psy_audio_Duplicator*);
static unsigned int numparametercols(psy_audio_Duplicator*);
static void dispose(psy_audio_Duplicator*);
static uintptr_t numinputs(psy_audio_Duplicator* self) { return 0; }
static uintptr_t numoutputs(psy_audio_Duplicator* self) { return 0; }
static void loadspecific(psy_audio_Duplicator*, psy_audio_SongFile*,
	uintptr_t slot);
static void savespecific(psy_audio_Duplicator*, psy_audio_SongFile*,
	uintptr_t slot);
static void stop(psy_audio_Duplicator*);

static int transpose(int note, int offset);
static uintptr_t parameterrow(psy_audio_Duplicator*, uintptr_t param);

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
	MACH_DUPLICATOR
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
		vtable.numparametercols = (fp_machine_numparametercols)
			numparametercols;
		vtable.parameterlabel = (fp_machine_parameterlabel) parameterlabel;
		vtable.parametername = (fp_machine_parametername) parametername;
		vtable.dispose = (fp_machine_dispose) dispose;
		vtable.numinputs = (fp_machine_numinputs) numinputs;
		vtable.numoutputs = (fp_machine_numoutputs) numoutputs;	
		vtable.loadspecific = (fp_machine_loadspecific) loadspecific;
		vtable.savespecific = (fp_machine_savespecific) savespecific;
		vtable.stop = (fp_machine_stop) stop;
		vtable_initialized = 1;
	}
}

void psy_audio_duplicator_init(psy_audio_Duplicator* self,
	MachineCallback callback)
{
	custommachine_init(&self->custommachine, callback);
	vtable_init(self);
	psy_audio_duplicator_base(self)->vtable = &vtable;
	self->isticking = 0;
	psy_audio_machine_seteditname(psy_audio_duplicator_base(self), "Note Duplicator");
	psy_audio_duplicatormap_init(&self->map, DUPLICATOR_NUMOUTPUTS,
		MAX_TRACKS);
}

void dispose(psy_audio_Duplicator* self)
{					
	custommachine_dispose(&self->custommachine);
	psy_audio_duplicatormap_dispose(&self->map);
}

void sequencertick(psy_audio_Duplicator* self)
{
	// Prevent possible loops of Duplicators
	// isticking = 0: allows duplicator to enter notes
	self->isticking = 0;
}

psy_List* sequencerinsert(psy_audio_Duplicator* self, PatternNode* events)
{			
	psy_List* p;	
	psy_List* insert = 0;

	if (!self->isticking) {
		// isticking = 1, prevents for this tick duplicator to insert further
		// notes than these ones to avoid possible loops of duplicators
		self->isticking = 1;
		for (p = events; p != 0; p = p->next) {
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
					int mapchannel;
					patternentry = psy_audio_patternnode_entry(p);
					newentry = patternentry_clone(patternentry);
					note = patternentry_front(patternentry)->note;					
					if (note < NOTECOMMANDS_RELEASE) {						
						note = transpose(note, output->offset);
					}
					mapchannel = psy_audio_duplicatormap_channel(&self->map,
						patternentry->track, output);
					if (newentry) {						
						patternentry_front(newentry)->mach = output->machine;
						patternentry_front(newentry)->note = note;						
						newentry->track = mapchannel;
						psy_list_append(&insert, newentry);
					}
					if (patternentry_front(patternentry)->note >= 
							NOTECOMMANDS_RELEASE) {
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

	if (note >= NOTECOMMANDS_RELEASE) {
		rv = NOTECOMMANDS_B9;
	} else
	if (note < 0) {
		rv = NOTECOMMANDS_C0;
	}
	return rv;
}

const psy_audio_MachineInfo* info(psy_audio_Duplicator* self)
{	
	return &macinfo;
}

void parametertweak(psy_audio_Duplicator* self, uintptr_t param, float value)
{	
	psy_audio_DuplicatorOutput* output;	
	
	output = psy_audio_duplicatormap_output(&self->map, param %
		psy_audio_duplicatormap_numoutputs(&self->map));
	switch (parameterrow(self, param)) {
		case 0:
			output->machine = machine_parametervalue_scaled(
				psy_audio_duplicator_base(self), param, value);
		break;
		case 1:
			output->offset = machine_parametervalue_scaled(
				psy_audio_duplicator_base(self), param, value);
		break;		
		default:			
		break;
	}
}

int describevalue(psy_audio_Duplicator* self, char* rv, uintptr_t param,
	int value)
{ 
	return 0;
}

float parametervalue(psy_audio_Duplicator* self, uintptr_t param)
{	
	psy_audio_DuplicatorOutput* output;
	
	output = psy_audio_duplicatormap_output(&self->map,
		param % psy_audio_duplicatormap_numoutputs(&self->map));
	switch (parameterrow(self, param)) {
		case 0:
			return machine_parametervalue_normed(psy_audio_duplicator_base(self), param,
				output->machine);
		break;
		case 1:
			return machine_parametervalue_normed(psy_audio_duplicator_base(self), param,
				output->offset);
		break;		
		default:			
		break;
	}
	return 0;
}

void parameterrange(psy_audio_Duplicator* self, uintptr_t param, int* minval,
	int* maxval)
{
	switch (parameterrow(self, param)) {
		case 0:
			*minval = -1;
			*maxval = 0x7E;
		break;
		case 1:		
			*minval = -48;
			*maxval = 48;
		break;	
		default:
			*minval = 0;
			*maxval = 0;
		break;
	}
}

int parameterlabel(psy_audio_Duplicator* self, char* rv, uintptr_t param)
{
	return parametername(self, rv, param);
}

int parametername(psy_audio_Duplicator* self, char* rv, uintptr_t param)
{
	switch (parameterrow(self, param)) {
		case 0:
			psy_snprintf(rv, 128, "%s %d", "Output Machine ", param);
		break;
		case 1:
			psy_snprintf(rv, 128, "%s %d", "Note Offset ",
				param % psy_audio_duplicatormap_numoutputs(&self->map));
		break;		
		default:
			rv[0] = '\0';
		break;
	}	
	return 1;	
}

uintptr_t parameterrow(psy_audio_Duplicator* self, uintptr_t param)
{
	return param / psy_audio_duplicatormap_numoutputs(&self->map);
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

void loadspecific(psy_audio_Duplicator* self, psy_audio_SongFile* songfile,
	uintptr_t slot)
{
	uint32_t size;
	int32_t i;
	int16_t macoutput[DUPLICATOR_NUMOUTPUTS];
	int16_t noteoffset[DUPLICATOR_NUMOUTPUTS];

	// size of this part params to load
	psyfile_read(songfile->file, &size, sizeof(size));
	// TODO: endianess
	psyfile_read(songfile->file, &macoutput[0],
		DUPLICATOR_NUMOUTPUTS * sizeof(int16_t));
	psyfile_read(songfile->file, &noteoffset[0],
		DUPLICATOR_NUMOUTPUTS * sizeof(int16_t));
	for (i = 0; i < DUPLICATOR_NUMOUTPUTS; ++i) {
		psy_audio_DuplicatorOutput* output;

		output = psy_audio_duplicatormap_output(&self->map, i);			
		if (output) {
			output->machine = macoutput[i];
			output->offset = noteoffset[i];
		}
	}
}

void savespecific(psy_audio_Duplicator* self, psy_audio_SongFile* songfile,
	uintptr_t slot)
{
	uint32_t size;
	int16_t macoutput[DUPLICATOR_NUMOUTPUTS];
	int16_t noteoffset[DUPLICATOR_NUMOUTPUTS];
	int32_t i;
	
	for (i = 0; i < DUPLICATOR_NUMOUTPUTS; ++i) {
		psy_audio_DuplicatorOutput* output;

		output = psy_audio_duplicatormap_output(&self->map, i);
		macoutput[i] = output->machine;
		noteoffset[i] = output->offset;
	}
	size = sizeof(macoutput) + sizeof(noteoffset);
	// size of this part params to save
	psyfile_write(songfile->file, &size, sizeof(size));
	// TODO: endianess
	psyfile_write(songfile->file, &macoutput[0], DUPLICATOR_NUMOUTPUTS *
		sizeof(int16_t));
	psyfile_write(songfile->file, &noteoffset[0], DUPLICATOR_NUMOUTPUTS *
		sizeof(int16_t));
}

void stop(psy_audio_Duplicator* self)
{
	psy_audio_duplicatormap_clear(&self->map);
}


// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "duplicator2.h"
#include "pattern.h"
#include "constants.h"
#include "songio.h"
#include <string.h>
#include <stdlib.h>
#include <portable.h>

// TODO: change will break songfile load/save
#define DUPLICATOR2_NUMOUTPUTS 16

static void work(psy_audio_Duplicator2* self, psy_audio_BufferContext* bc) { }
static void sequencertick(psy_audio_Duplicator2*);
static psy_List* sequencerinsert(psy_audio_Duplicator2*, PatternNode* events);
static const psy_audio_MachineInfo* info(psy_audio_Duplicator2*);
static void parametertweak(psy_audio_Duplicator2*, uintptr_t param, int val);
static int describevalue(psy_audio_Duplicator2*, char* rv, uintptr_t param,
	int value);
static int parametervalue(psy_audio_Duplicator2*, uintptr_t param);
static void parameterrange(psy_audio_Duplicator2*, uintptr_t param, int* minval,
	int* maxval);
static int parameterlabel(psy_audio_Duplicator2*, char* rv, uintptr_t param);
static int parametername(psy_audio_Duplicator2*, char* rv, uintptr_t param);
static uintptr_t numparameters(psy_audio_Duplicator2*);
static uintptr_t numparametercols(psy_audio_Duplicator2*);
static void dispose(psy_audio_Duplicator2*);
static uintptr_t numinputs(psy_audio_Duplicator2* self) { return 0; }
static uintptr_t numoutputs(psy_audio_Duplicator2* self) { return 0; }
static void loadspecific(psy_audio_Duplicator2*, psy_audio_SongFile*,
	uintptr_t slot);
static void savespecific(psy_audio_Duplicator2*, psy_audio_SongFile*,
	uintptr_t slot);
static void stop(psy_audio_Duplicator2*);

static int transpose(int note, int offset);
static uintptr_t parameterrow(psy_audio_Duplicator2*, uintptr_t param);

static psy_audio_MachineInfo const macinfo = {
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

const psy_audio_MachineInfo* psy_audio_duplicator2_info(void)
{
	return &macinfo;
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

void psy_audio_duplicator2_init(psy_audio_Duplicator2* self,
	MachineCallback callback)
{
	custommachine_init(&self->custommachine, callback);
	vtable_init(self);
	self->custommachine.machine.vtable = &vtable;
	self->isticking = 0;
	psy_audio_duplicatormap_init(&self->map, DUPLICATOR2_NUMOUTPUTS,
		MAX_TRACKS);
	machine_seteditname(psy_audio_duplicator2_base(self), "Note Duplicator 2");
}

void dispose(psy_audio_Duplicator2* self)
{	
	psy_audio_duplicatormap_dispose(&self->map);
	custommachine_dispose(&self->custommachine);
}

void sequencertick(psy_audio_Duplicator2* self)
{
	// Prevent possible loops of Duplicators
	// isticking = 0: allows duplicator to enter notes
	self->isticking = 0;
}

psy_List* sequencerinsert(psy_audio_Duplicator2* self, PatternNode* events)
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
					int note;
					int mapchannel;

					patternentry = psy_audio_patternnode_entry(p);
					note = patternentry_front(patternentry)->note;
					mapchannel = psy_audio_duplicatormap_channel(
						&self->map, patternentry->track, output);
					if (note < NOTECOMMANDS_RELEASE) {						
						note = transpose(note, output->offset);
					}					
					if ((note >= output->lowkey && note <= output->highkey) ||
							note >= NOTECOMMANDS_RELEASE) {
						psy_audio_PatternEntry* newentry;						
							
						newentry = patternentry_clone(patternentry);
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

const psy_audio_MachineInfo* info(psy_audio_Duplicator2* self)
{	
	return &macinfo;
}

void parametertweak(psy_audio_Duplicator2* self, uintptr_t param, int value)
{
	psy_audio_DuplicatorOutput* output;
	
	output = psy_audio_duplicatormap_output(&self->map, param %
		psy_audio_duplicatormap_numoutputs(&self->map));
	assert(output);
	switch (parameterrow(self, param)) {
		case 0:
			output->machine = value;
		break;
		case 1:
			output->offset = value;
		break;
		case 2:
			output->lowkey = value;
		break;
		case 3:
			output->highkey = value;
		break;
		default:			
		break;
	}	
}

int describevalue(psy_audio_Duplicator2* self, char* rv, uintptr_t param,
	int value)
{ 
	return 0;
}

int parametervalue(psy_audio_Duplicator2* self, uintptr_t param)
{		
	psy_audio_DuplicatorOutput* output;
	
	output = psy_audio_duplicatormap_output(&self->map, param %
		psy_audio_duplicatormap_numoutputs(&self->map));
	assert(output);			
	switch (parameterrow(self, param)) {
		case 0:
			return output->machine;
		break;
		case 1:
			return output->offset;
		break;
		case 2:
			return output->lowkey;
		break;
		case 3:
			return output->highkey;
		break;
		default:			
		break;
	}
	return 0;
}

void parameterrange(psy_audio_Duplicator2* self, uintptr_t param, int* minval,
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
		case 2:
		case 3:
			*minval = NOTECOMMANDS_C0;
			*maxval = NOTECOMMANDS_B9;
		break;
		default:
			*minval = 0;
			*maxval = 0;
		break;
	}	
}

int parameterlabel(psy_audio_Duplicator2* self, char* rv, uintptr_t param)
{
	return parametername(self, rv, param);
}

int parametername(psy_audio_Duplicator2* self, char* rv, uintptr_t param)
{		
	switch (parameterrow(self, param)) {
		case 0:
			psy_snprintf(rv, 128, "%s %d", "Output Machine ", param);
		break;
		case 1:
			psy_snprintf(rv, 128, "%s %d", "Note Offset ",
				param % psy_audio_duplicatormap_numoutputs(&self->map));
		break;
		case 2:
			psy_snprintf(rv, 128, "%s %d", "Low Note ",
				param % psy_audio_duplicatormap_numoutputs(&self->map));
		break;
		case 3:
			psy_snprintf(rv, 128, "%s %d", "High Note ",
				param % psy_audio_duplicatormap_numoutputs(&self->map));
		break;		
		default:
			rv[0] = '\0';
		break;
	}	
	return 1;
}

uintptr_t parameterrow(psy_audio_Duplicator2* self, uintptr_t param)
{
	return param / psy_audio_duplicatormap_numoutputs(&self->map);	
}

uintptr_t numparameters(psy_audio_Duplicator2* self)
{
	return psy_audio_duplicatormap_numoutputs(&self->map) *
		numparametercols(self);
}

uintptr_t numparametercols(psy_audio_Duplicator2* self)
{
	return 4;	
}

void loadspecific(psy_audio_Duplicator2* self, psy_audio_SongFile* songfile,
	uintptr_t slot)
{
	uint32_t size;
	int16_t macoutput[DUPLICATOR2_NUMOUTPUTS];
	int16_t noteoffset[DUPLICATOR2_NUMOUTPUTS];
	int16_t lowkey[DUPLICATOR2_NUMOUTPUTS];
	int16_t highkey[DUPLICATOR2_NUMOUTPUTS];
	int32_t i;

	// size of this part params to load
	psyfile_read(songfile->file, &size, sizeof(size));
	// TODO: endianess
	psyfile_read(songfile->file, macoutput, DUPLICATOR2_NUMOUTPUTS *
		sizeof(int16_t));
	psyfile_read(songfile->file, noteoffset, DUPLICATOR2_NUMOUTPUTS *
		sizeof(int16_t));
	psyfile_read(songfile->file, lowkey, DUPLICATOR2_NUMOUTPUTS *
		sizeof(int16_t));
	psyfile_read(songfile->file, highkey, DUPLICATOR2_NUMOUTPUTS *
		sizeof(int16_t));
	for (i = 0; i < DUPLICATOR2_NUMOUTPUTS; ++i) {
		psy_audio_DuplicatorOutput* output;

		output = psy_audio_duplicatormap_output(&self->map, i);			
		if (output) {
			psy_audio_duplicatoroutput_setall(output, macoutput[i],
				noteoffset[i], lowkey[i], highkey[i]);
		}
	}
}

void savespecific(psy_audio_Duplicator2* self, psy_audio_SongFile* songfile,
	unsigned int slot)
{
	uint32_t size;
	int16_t macoutput[DUPLICATOR2_NUMOUTPUTS];
	int16_t noteoffset[DUPLICATOR2_NUMOUTPUTS];
	int16_t lowkey[DUPLICATOR2_NUMOUTPUTS];
	int16_t highkey[DUPLICATOR2_NUMOUTPUTS];
	int32_t i;
	
	for (i = 0; i < DUPLICATOR2_NUMOUTPUTS; ++i) {
		psy_audio_DuplicatorOutput* output;

		output = (psy_audio_DuplicatorOutput*) psy_table_at(&self->map.outputs,
			i);
		macoutput[i] = output->machine;
		noteoffset[i] = output->offset;
		lowkey[i] = output->lowkey;
		highkey[i] = output->highkey;
	}	
	size = sizeof(macoutput) + sizeof(noteoffset) + sizeof(lowkey) + sizeof(highkey);
	// size of this part params to save
	psyfile_write(songfile->file, &size, sizeof(size));
	// TODO: endianess
	psyfile_write(songfile->file, &macoutput[0],
		DUPLICATOR2_NUMOUTPUTS * sizeof(int16_t));
	psyfile_write(songfile->file, &noteoffset[0],
		DUPLICATOR2_NUMOUTPUTS * sizeof(int16_t));
	psyfile_write(songfile->file, &lowkey[0],
		DUPLICATOR2_NUMOUTPUTS * sizeof(int16_t));
	psyfile_write(songfile->file, &highkey[0],
		DUPLICATOR2_NUMOUTPUTS * sizeof(int16_t));
}

void stop(psy_audio_Duplicator2* self)
{
	psy_audio_duplicatormap_clear(&self->map);
}

psy_audio_Machine* psy_audio_duplicator2_base(psy_audio_Duplicator2* self)
{
	return &(self->custommachine.machine);
}

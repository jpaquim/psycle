// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "master.h"
#include "machines.h"
#include <string.h>
#include <math.h>
#include "songio.h"
#include <portable.h>

static int master_mode(psy_audio_Master* self) { return MACHMODE_MASTER; }
static void master_dispose(psy_audio_Master*);

static int parametertype(psy_audio_Master*, uintptr_t param);
static uintptr_t numparameters(psy_audio_Master*);
static unsigned int numparametercols(psy_audio_Master*);
static void parametertweak(psy_audio_Master*, uintptr_t param, int val);	
static void parameterrange(psy_audio_Master*, uintptr_t param, int* minval, int* maxval);
static int parameterlabel(psy_audio_Master*, char* txt, uintptr_t param);
static int parametername(psy_audio_Master*, char* txt, uintptr_t param);
static int describevalue(psy_audio_Master*, char* txt, uintptr_t param, int value);
static int parametervalue(psy_audio_Master*, uintptr_t param);
static const psy_audio_MachineInfo* info(psy_audio_Master*);
static uintptr_t numinputs(psy_audio_Master*);
static uintptr_t numoutputs(psy_audio_Master*);
static int intparamvalue(float value);
static float floatparamvalue(int value);
static void master_loadspecific(psy_audio_Master*, struct psy_audio_SongFile*,
	uintptr_t slot);
static void master_savespecific(psy_audio_Master*, struct psy_audio_SongFile*,
	uintptr_t slot);
static psy_dsp_amp_range_t amprange(psy_audio_Master* self)
{
	return PSY_DSP_AMP_RANGE_IGNORE;
}

static psy_audio_MachineInfo const MacInfo = {
	MI_VERSION,
	0x0250,
	EFFECT | 32 | 64,
	MACHMODE_FX,
	"Master"
		#ifndef NDEBUG
		" (debug build)"
		#endif
		,
	"Master",
	"Psycledelics",
	"help",
	MACH_MASTER,
	0,
	0
};

const psy_audio_MachineInfo* master_info(void) { return &MacInfo; }

static MachineVtable vtable;
static int vtable_initialized = 0;

static void vtable_init(psy_audio_Master* self)
{
	if (!vtable_initialized) {
		vtable = *self->machine.vtable;
		vtable.mode = (fp_machine_mode) master_mode;
		vtable.info = (fp_machine_info) info;
		vtable.dispose = (fp_machine_dispose) master_dispose;
		vtable.info = (fp_machine_info) info;
		vtable.parametertweak = (fp_machine_parametertweak) parametertweak;
		vtable.describevalue = (fp_machine_describevalue) describevalue;
		vtable.parametervalue = (fp_machine_parametervalue) parametervalue;
		vtable.numinputs = (fp_machine_numinputs) numinputs;
		vtable.numoutputs = (fp_machine_numoutputs) numoutputs;
		vtable.loadspecific = (fp_machine_loadspecific) master_loadspecific;
		vtable.savespecific = (fp_machine_savespecific) master_savespecific;
		// Parameter
		vtable.parametertype = (fp_machine_parametertype) parametertype;
		vtable.numparametercols = (fp_machine_numparametercols) numparametercols;
		vtable.numparameters = (fp_machine_numparameters) numparameters;
		vtable.parameterrange = (fp_machine_parameterrange) parameterrange;
		vtable.parametertweak = (fp_machine_parametertweak) parametertweak;	
		vtable.parameterlabel = (fp_machine_parameterlabel) parameterlabel;
		vtable.parametername = (fp_machine_parametername) parametername;
		vtable.describevalue = (fp_machine_describevalue) describevalue;
		vtable.parametervalue = (fp_machine_parametervalue) parametervalue;
		vtable.amprange = (fp_machine_amprange) amprange;
		vtable_initialized = 1;
	}
}

void master_init(psy_audio_Master* self, MachineCallback callback)
{
	memset(self, 0, sizeof(psy_audio_Master));
	machine_init(&self->machine, callback);	
	vtable_init(self);
	self->machine.vtable = &vtable;
}

void master_dispose(psy_audio_Master* self)
{		
	machine_dispose(&self->machine);
}

void parametertweak(psy_audio_Master* self, uintptr_t param, int value)
{
	if (param == 0) {
		psy_audio_Machines* machines = self->machine.vtable->machines(&self->machine);
		if (machines) {			
			machines_setvolume(machines,
				floatparamvalue(value) * floatparamvalue(value) * 4.f);
		}
	} else {
		psy_audio_MachineSockets* sockets;
		WireSocket* p;
		uintptr_t c = 1;
		psy_audio_Machines* machines = self->machine.vtable->machines(&self->machine);
		
		sockets = connections_at(&machines->connections, MASTER_INDEX);
		if (sockets) {
			for (p = sockets->inputs; p != 0 && c != param; p = p->next, ++c);
			if (p) {
				psy_audio_WireSocketEntry* input_entry;

				input_entry = (psy_audio_WireSocketEntry*) p->entry;
				input_entry->volume =
					floatparamvalue(value) * floatparamvalue(value) * 4.f;					
			}
		}		
	}
}

int describevalue(psy_audio_Master* self, char* txt, uintptr_t param, int value)
{ 	
	if (param == 0) {
		psy_audio_Machines* machines = self->machine.callback.machines(
			self->machine.callback.context);

		psy_dsp_amp_t db = (psy_dsp_amp_t)(20 * 
			log10(machines_volume(machines)));
		psy_snprintf(txt, 10, "%.2f dB", db);
		return 1;
	} else {
		psy_audio_MachineSockets* sockets;
		WireSocket* p;
		uintptr_t c = 1;
		psy_audio_Machines* machines = self->machine.vtable->machines(&self->machine);
		
		sockets = connections_at(&machines->connections, MASTER_INDEX);
		if (sockets) {
			for (p = sockets->inputs; p != 0 && c != param; p = p->next, ++c);
			if (p) {				
				psy_audio_WireSocketEntry* input_entry;
				psy_dsp_amp_t db;

				input_entry = (psy_audio_WireSocketEntry*) p->entry;
				db = (psy_dsp_amp_t)(20 * log10(input_entry->volume));
				psy_snprintf(txt, 10, "%.2f dB", db);
				return 1;
			}			
		}
	}
	return 0;
}

int parametervalue(psy_audio_Master* self, uintptr_t param)
{	
	if (param == 0) {
		psy_audio_Machines* machines = self->machine.callback.machines(
			self->machine.callback.context);

		if (machines) {
			return intparamvalue(
				(float)sqrt(machines_volume(machines)) * 0.5f);
		}
	} else {
		psy_audio_MachineSockets* sockets;
		WireSocket* input_socket;
		uintptr_t c = 1;
		psy_audio_Machines* machines = self->machine.vtable->machines(&self->machine);
		
		sockets = connections_at(&machines->connections, MASTER_INDEX);
		if (sockets) {
			for (input_socket = sockets->inputs; input_socket != 0 && c != param;
					input_socket = input_socket->next, ++c);
			if (input_socket) {
				psy_audio_WireSocketEntry* input_entry;

				input_entry = (psy_audio_WireSocketEntry*) input_socket->entry;
				return intparamvalue(
					(float)sqrt(input_entry->volume) * 0.5f);
			}
		}
	}
	return 0;
}

int intparamvalue(float value)
{	
	return (int)((value * 65535.f));	
}

float floatparamvalue(int value)
{
	return value / 65535.f;	
}

const psy_audio_MachineInfo* info(psy_audio_Master* self)
{	
	return &MacInfo;
}

uintptr_t numinputs(psy_audio_Master* self)
{
	return 2;
}

uintptr_t numoutputs(psy_audio_Master* self)
{
	return 2;
}

int parametertype(psy_audio_Master* self, uintptr_t param)
{
	return MPF_STATE;
}

void parameterrange(psy_audio_Master* self, uintptr_t param, int* minval, int* maxval)
{
	*minval = 0;
	*maxval = 65535;
}

uintptr_t numparameters(psy_audio_Master* self)
{
	return 13;
}

unsigned int numparametercols(psy_audio_Master* self)
{
	return 4;
}

int parameterlabel(psy_audio_Master* self, char* txt, uintptr_t param)
{
	psy_snprintf(txt, 128, "%s", "Vol");
	return 1;
}

int parametername(psy_audio_Master* self, char* txt, uintptr_t param)
{
	psy_snprintf(txt, 128, "%s", "Vol");
	return 1;
}

void master_loadspecific(psy_audio_Master* self, psy_audio_SongFile* songfile,
	uintptr_t slot)
{	
	uint32_t size;
	int32_t outdry = 256;
	unsigned char decreaseOnClip = 0;

	psyfile_read(songfile->file, &size, sizeof size ); // size of this part params to load
	psyfile_read(songfile->file, &outdry, sizeof outdry);
	psyfile_read(songfile->file, &decreaseOnClip, sizeof decreaseOnClip);

	machines_setvolume(&songfile->song->machines, outdry / (psy_dsp_amp_t) 256);
}

void master_savespecific(psy_audio_Master* self, psy_audio_SongFile* songfile,
	uintptr_t slot)
{
	uint32_t size;
	int32_t outdry = 256;
	unsigned char decreaseOnClip = 0;
				
	size = sizeof outdry + sizeof decreaseOnClip;
	psyfile_write(songfile->file, &size, sizeof size); // size of this part params to save
	psyfile_write(songfile->file, &outdry, sizeof outdry);
	psyfile_write(songfile->file, &decreaseOnClip, sizeof decreaseOnClip); 
}

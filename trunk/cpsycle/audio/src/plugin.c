// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "plugin.h"
#include "pattern.h"
#include "songio.h"
#include <stdlib.h>
#include <string.h>
#include <portable.h>
#include <math.h>

typedef CMachineInfo * (*GETINFO)(void);
typedef CMachineInterface * (*CREATEMACHINE)(void);

static Machine* clone(Plugin*);
static int hostevent(Plugin*, int const eventNr, int const val1, float const val2);
static void generateaudio(Plugin*, BufferContext*);
static void seqtick(Plugin*, int channel, const PatternEvent* event);
static void stop(Plugin*);
static void sequencerlinetick(Plugin*);
static MachineInfo* info(Plugin*);
static int parametertype(Plugin* self, int par);
static unsigned int numparametercols(Plugin*);
static unsigned int numparameters(Plugin*);
static void parameterrange(Plugin*, int numparam, int* minval, int* maxval);
static int parameterlabel(Plugin*, char* txt, int param);
static int parametername(Plugin*, char* txt, int param);
static void parametertweak(Plugin*, int par, int val);
static int parameterlabel(Plugin*, char* txt, int param);
static int parametername(Plugin*, char* txt, int param);
static int describevalue(Plugin*, char* txt, int param, int value);
static int parametervalue(Plugin*, int param);
static void dispose(Plugin*);
static unsigned int numinputs(Plugin*);
static unsigned int numoutputs(Plugin*);
static void loadspecific(Plugin*, struct SongFile*, unsigned int slot);
static void savespecific(Plugin*, struct SongFile*, unsigned int slot);
static void setcallback(Plugin*, MachineCallback);

static MachineVtable vtable;
static int vtable_initialized = 0;

static void vtable_init(Plugin* self)
{
	if (!vtable_initialized) {
		vtable = *self->custommachine.machine.vtable;
		vtable.clone = (fp_machine_clone) clone;
		vtable.hostevent = (fp_machine_hostevent) hostevent;
		vtable.seqtick = (fp_machine_seqtick) seqtick;
		vtable.stop = (fp_machine_stop) stop;
		vtable.sequencerlinetick = (fp_machine_sequencerlinetick) sequencerlinetick;
		vtable.info = (fp_machine_info) info;
		vtable.numparametercols = (fp_machine_numparametercols) numparametercols;
		vtable.numparameters = (fp_machine_numparameters) numparameters;
		vtable.parameterrange = (fp_machine_parameterrange) parameterrange;
		vtable.parametertype = (fp_machine_parametertype) parametertype;
		vtable.parametername = (fp_machine_parametername) parametername;
		vtable.parameterlabel = (fp_machine_parameterlabel) parameterlabel;
		vtable.parametertweak = (fp_machine_parametertweak) parametertweak;
		vtable.parameterlabel = (fp_machine_parameterlabel) parameterlabel;
		vtable.parametername = (fp_machine_parametername) parametername;
		vtable.describevalue = (fp_machine_describevalue) describevalue;
		vtable.parametervalue = (fp_machine_parametervalue) parametervalue;
		vtable.dispose = (fp_machine_dispose) dispose;
		vtable.generateaudio = (fp_machine_generateaudio) generateaudio;
		vtable.numinputs = (fp_machine_numinputs) numinputs;
		vtable.numoutputs = (fp_machine_numoutputs) numoutputs;
		vtable.loadspecific = (fp_machine_loadspecific) loadspecific;
		vtable.savespecific = (fp_machine_savespecific) savespecific;	
		vtable.setcallback = (fp_machine_setcallback) setcallback;	
		vtable_initialized = 1;
	}
}

void plugin_init(Plugin* self, MachineCallback callback, const char* path)
{
	GETINFO GetInfo;
	Machine* base = &self->custommachine.machine;
	
	custommachine_init(&self->custommachine, callback);	
	library_init(&self->library);
	library_load(&self->library, path);			
	self->mi = 0;
	self->plugininfo = 0;
	vtable_init(self);
	self->custommachine.machine.vtable = &vtable;
	GetInfo = (GETINFO)library_functionpointer(&self->library, "GetInfo");
	if (!GetInfo) {
		library_dispose(&self->library);		
	} else {		
		self->mi = mi_create(self->library.module);		
		if (!self->mi) {
			library_dispose(&self->library);			
		} else {						
			CMachineInfo* pInfo = GetInfo();
			if (pInfo) {
				int gbp;	
				mi_resetcallback(self->mi);
				mi_setcallback(self->mi, &callback);
				mi_init(self->mi);
				for (gbp = 0; gbp< pInfo->numParameters; gbp++) {
					mi_parametertweak(self->mi, gbp, 
						pInfo->Parameters[gbp]->DefValue);
				}
				self->plugininfo = machineinfo_allocinit();
				machineinfo_setnativeinfo(self->plugininfo, pInfo, MACH_PLUGIN,
					self->library.path, 0);				
				base->vtable->seteditname(base, pInfo->ShortName);
			}
		}
	}
	if (!base->vtable->editname(base)) {
		base->vtable->seteditname(base, "Plugin");
	}
}

void dispose(Plugin* self)
{		
	if (self->library.module != 0 && self->mi) {
		mi_dispose(self->mi);
		library_dispose(&self->library);
		self->mi = 0;		
	}			
	if (self->plugininfo) {
		machineinfo_dispose(self->plugininfo);
		free(self->plugininfo);
		self->plugininfo = 0;
	}	
	custommachine_dispose(&self->custommachine);
}

Machine* clone(Plugin* self)
{
	Plugin* rv;

	rv = malloc(sizeof(Plugin));
	if (rv) {
		plugin_init(rv, self->custommachine.machine.callback, self->library.path);
	}
	return rv ? &rv->custommachine.machine : 0;
}

int plugin_psycle_test(const char* path, MachineInfo* info)
{	
	int rv = 0;

	if (path && strcmp(path, "") != 0) {
		GETINFO GetInfo;
		Library library;

		library_init(&library);
		library_load(&library, path);					
		GetInfo =(GETINFO)library_functionpointer(&library, "GetInfo");
		if (GetInfo != NULL) {	
			CMachineInfo* nativeinfo = GetInfo();
			if (nativeinfo) {				
				machineinfo_setnativeinfo(info, nativeinfo, MACH_PLUGIN,
					library.path, 0);
				rv = 1;
			}
		}
		library_dispose(&library);
	}
	return rv;	
}

void seqtick(Plugin* self, int channel, const PatternEvent* event)
{
	mi_seqtick(self->mi, channel, event->note, event->inst, event->cmd,
		event->parameter);
}

void stop(Plugin* self)
{
	mi_stop(self->mi);
}

void generateaudio(Plugin* self, BufferContext* bc)
{	
	mi_work(self->mi, buffer_at(bc->output, 0), buffer_at(bc->output, 1),
		bc->numsamples, bc->numtracks);
}

int hostevent(Plugin* self, int const eventNr, int const val1, float const val2)
{
	return mi_hostevent(self->mi, eventNr, val1, val2);
}

void sequencerlinetick(Plugin* self)
{
	mi_sequencertick(self->mi);
}

MachineInfo* info(Plugin* self)
{		
	return self->plugininfo;
}

void parametertweak(Plugin* self, int par, int val)
{
	mi_parametertweak(self->mi, par, val);
}

int describevalue(Plugin* self, char* txt, int param, int value)
{ 
	return mi_describevalue(self->mi, txt, param, value);
}

int parametervalue(Plugin* self, int param)
{
	return mi_val(self->mi, param);
}

unsigned int numinputs(Plugin* self)
{
	if (info(self)) {
		return self->plugininfo->mode == MACHMODE_FX ? 2 : 0;
	} else {
		return 0;
	}
}

unsigned int numoutputs(Plugin* self)
{
	if (info(self)) {
		return 2;
	} else {
		return 0;
	}
}

void setcallback(Plugin* self, MachineCallback callback)
{
	if (self->mi) {				
		mi_setcallback(self->mi, &callback);
	}
}

void loadspecific(Plugin* self, struct SongFile* songfile, unsigned int slot)
{
	unsigned int size;

	psyfile_read(songfile->file, &size, sizeof(size)); // size of whole structure
	if(size) {
		unsigned int count;
		unsigned int i;

		psyfile_read(songfile->file, &count, sizeof(count));  // size of vars
		for (i = 0; i < count; i++) {
			int temp;
			
			psyfile_read(songfile->file, &temp, sizeof(temp));
			self->custommachine.machine.vtable->parametertweak(
				&self->custommachine.machine, i, temp);
		}
		size -= sizeof(count) + sizeof(int)*count;
		if(size) {
			unsigned int size2 = 0;
			unsigned char* pData;
			
			size2 = mi_getdatasize(self->mi);
			// This way we guarantee that the plugin will have enough bytes,
			// even if it does not fit what it reads.
			pData = (unsigned char*)malloc(max(size,size2));
			psyfile_read(songfile->file, pData, size); // Read internal data.			
			mi_putdata(self->mi, pData); // Internal load
			free(pData);
		}						
	}	
}			

void savespecific(Plugin* self, struct SongFile* songfile, unsigned int slot)
{
	uint32_t count = self->custommachine.machine.vtable->numparameters(
		&self->custommachine.machine);
	uint32_t size2 = 0;
	uint32_t size;
	uint32_t i;

	size2 = mi_getdatasize(self->mi);
	size = size2 + sizeof(count) + sizeof(int) * count;
	psyfile_write(songfile->file, &size, sizeof(size));
	psyfile_write(songfile->file, &count, sizeof(count));
	for (i = 0; i < count; ++i) {
		int temp;
		
		temp = self->custommachine.machine.vtable->parametervalue(
			&self->custommachine.machine, i);
		psyfile_write(songfile->file, &temp, sizeof temp);
	}
	if (size2) {
		unsigned char* pData;

		pData = malloc(size2);
		mi_getdata(self->mi, pData);
		psyfile_write(songfile->file, pData, size2); //data chunk
		free(pData);
		pData = 0;
	}
}

int parametertype(Plugin* self, int param)
{
	int rv = MPF_STATE;
	GETINFO GetInfo;

	GetInfo =(GETINFO)library_functionpointer(&self->library, "GetInfo");
	if (GetInfo != NULL) {	
		CMachineInfo* pInfo = GetInfo();
		if (pInfo) {	
			if (param < pInfo->numParameters) {
				rv = pInfo->Parameters[param]->Flags;
			}
		}
	}
	return rv;
}

void parameterrange(Plugin* self, int param, int* minval, int* maxval)
{	
	GETINFO GetInfo;

	*minval = 0;
	*maxval = 0;
	GetInfo =(GETINFO)library_functionpointer(&self->library, "GetInfo");
	if (GetInfo != NULL) {	
		CMachineInfo* pInfo = GetInfo();
		if (pInfo) {	
			if (param < pInfo->numParameters) {
				*minval = pInfo->Parameters[param]->MinValue;
				*maxval = pInfo->Parameters[param]->MaxValue;				
			}
		}
	}	
}

int parameterlabel(Plugin* self, char* txt, int param)
{
	int rv = 0;
	GETINFO GetInfo;

	GetInfo =(GETINFO)library_functionpointer(&self->library, "GetInfo");
	if (GetInfo != NULL) {	
		CMachineInfo* pInfo = GetInfo();
		if (pInfo) {	
			if (param < pInfo->numParameters) {
				psy_snprintf(txt, 128, "%s", pInfo->Parameters[param]->Description);
				rv = 1;
			}
		}
	}
	return rv;
}

int parametername(Plugin* self, char* txt, int param)
{
	int rv = 0;
	GETINFO GetInfo;

	GetInfo =(GETINFO)library_functionpointer(&self->library, "GetInfo");
	if (GetInfo != NULL) {	
		CMachineInfo* pInfo = GetInfo();
		if (pInfo) {	
			if (param < pInfo->numParameters) {
				psy_snprintf(txt, 128, "%s", pInfo->Parameters[param]->Name);
				rv = 1;
			}
		}
	}
	return rv;
}

unsigned int numparametercols(Plugin* self)
{
	int rv = 0;
	GETINFO GetInfo;

	GetInfo =(GETINFO)library_functionpointer(&self->library, "GetInfo");
	if (GetInfo != NULL) {	
		CMachineInfo* pInfo = GetInfo();
		if (pInfo) {	
			rv = pInfo->numparametercols;				
		}
	}
	return rv;
}

unsigned int numparameters(Plugin* self)
{
	int rv = 0;
	GETINFO GetInfo;

	GetInfo =(GETINFO)library_functionpointer(&self->library, "GetInfo");
	if (GetInfo != NULL) {	
		CMachineInfo* pInfo = GetInfo();
		if (pInfo) {	
			rv = pInfo->numParameters;
		}
	}
	return rv;
}


// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "plugin.h"
#include "pattern.h"
#include "songio.h"
#include <stdlib.h>
#include <string.h>

typedef CMachineInfo * (*GETINFO)(void);
typedef CMachineInterface * (*CREATEMACHINE)(void);

static Machine* clone(void*);
static int hostevent(Plugin*, int const eventNr, int const val1, float const val2);
static void generateaudio(Plugin*, BufferContext*);
static void seqtick(Plugin*, int channel, const PatternEvent* event);
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
static const char* editname(Plugin*);
static void seteditname(Plugin*, const char* name);
		
void plugin_init(Plugin* self, MachineCallback callback, const char* path)
{
	GETINFO GetInfo;
	
	machine_init(&self->machine, callback);	
	library_init(&self->library);
	library_load(&self->library, path);		
	self->pan = 0.5;
	self->mi = 0;
	self->plugininfo = 0;
	self->machine.clone = clone;
	self->machine.hostevent = hostevent;
	self->machine.seqtick = seqtick;
	self->machine.sequencerlinetick = sequencerlinetick;
	self->machine.info = info;
	self->machine.numparametercols = numparametercols;
	self->machine.numparameters = numparameters;
	self->machine.parameterrange = parameterrange;
	self->machine.parametertype = parametertype;
	self->machine.parametername = parametername;
	self->machine.parameterlabel = parameterlabel;
	self->machine.parametertweak = parametertweak;
	self->machine.parameterlabel = parameterlabel;
	self->machine.parametername = parametername;
	self->machine.describevalue = describevalue;	
	self->machine.parametervalue = parametervalue;
	self->machine.dispose = dispose;
	self->machine.generateaudio = generateaudio;
	self->machine.numinputs = numinputs;
	self->machine.numoutputs = numoutputs;
	self->machine.loadspecific = loadspecific;
	self->machine.savespecific = savespecific;
	self->machine.editname = editname;
	self->machine.seteditname = seteditname;
	self->machine.setcallback = setcallback;
	self->editname = 0;
			
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
				self->editname = strdup(pInfo->ShortName);
			}
		}
	}
	if (!self->editname) {
		self->editname = strdup("Plugin");
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
	free(self->editname);
	machine_dispose(&self->machine);
}

Machine* clone(Plugin* self)
{
	Plugin* rv;

	rv = malloc(sizeof(Plugin));
	if (rv) {
		plugin_init(rv, self->machine.callback, self->library.path);
	}
	return rv ? &rv->machine : 0;
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
		return ((info(self)->Flags & 3) == 3) ? 0 : 2;
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
			self->machine.parametertweak(self, i, temp);
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
	uint32_t count = self->machine.numparameters(&self->machine);
	uint32_t size2 = 0;
	uint32_t size;
	uint32_t i;

	size2 = mi_getdatasize(self->mi);
	size = size2 + sizeof(count) + sizeof(int) * count;
	psyfile_write(songfile->file, &size, sizeof(size));
	psyfile_write(songfile->file, &count, sizeof(count));
	for (i = 0; i < count; ++i) {
		int temp;
		
		temp = self->machine.parametervalue(&self->machine, i);
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
				_snprintf(txt, 128, "%s", pInfo->Parameters[param]->Description);
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
				_snprintf(txt, 128, "%s", pInfo->Parameters[param]->Name);
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

const char* editname(Plugin* self)
{
	return self->editname;
}

void seteditname(Plugin* self, const char* name)
{
	free(self->editname);
	self->editname = strdup(name);
}

// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#include "plugin.h"
#include "pattern.h"
#include <stdlib.h>
#include <string.h>

typedef CMachineInfo * (*GETINFO)(void);
typedef CMachineInterface * (*CREATEMACHINE)(void);

static Machine* clone(void*);
static int hostevent(Plugin*, int const eventNr, int const val1, float const val2);
static void generateaudio(Plugin*, BufferContext*);
static void seqtick(Plugin*, int channel, const PatternEvent* event);
static void sequencerlinetick(Plugin*);
static CMachineInfo* info(Plugin*);
static void parametertweak(Plugin*, int par, int val);
static int describevalue(Plugin*, char* txt, int const param, int const value);
static int value(Plugin*, int const param);
static void setvalue(Plugin*, int const param, int const value);
static void dispose(Plugin*);
static unsigned int numinputs(Plugin*);
static unsigned int numoutputs(Plugin*);
static void loadspecific(Plugin* self, RiffFile* file);
static void setcallback(Plugin* self, MachineCallback callback);
		
void plugin_init(Plugin* self, MachineCallback callback, const char* path)
{
	CMachineInfo* pInfo = 0;	

	machine_init(&self->machine, callback);	
	library_init(&self->library);
	library_load(&self->library, path);	
	self->pan = 0.5;
	self->mi = 0;
	self->machine.clone = clone;
	self->machine.hostevent = hostevent;
	self->machine.seqtick = seqtick;
	self->machine.sequencerlinetick = sequencerlinetick;
	self->machine.info = info;
	self->machine.parametertweak = parametertweak;
	self->machine.describevalue = describevalue;
	self->machine.setvalue = setvalue;
	self->machine.value = value;
	self->machine.dispose = dispose;
	self->machine.generateaudio = generateaudio;
	self->machine.numinputs = numinputs;
	self->machine.numoutputs = numoutputs;
	self->machine.loadspecific = loadspecific;
	self->machine.setcallback = setcallback;

	pInfo = info(self);	
	if (!pInfo)
	{
		library_dispose(&self->library);		
	} else {		
		self->mi = mi_create(self->library.module);		
		if (!self->mi) {
			library_dispose(&self->library);			
		} else {
			int gbp;	
			mi_resetcallback(self->mi);
			mi_setcallback(self->mi, &callback);
			mi_init(self->mi);
			for (gbp = 0; gbp< pInfo->numParameters; gbp++) {
				mi_parametertweak(self->mi, gbp, pInfo->Parameters[gbp]->DefValue);
			}					
		}
	}	
}

void dispose(Plugin* self)
{		
	if (self->library.module != 0 && self->mi) {
		mi_dispose(self->mi);
		library_dispose(&self->library);
		self->mi = 0;		
	}		
	machine_dispose(&self->machine);
}

Machine* clone(Plugin* self)
{
	Plugin* rv;

	rv = malloc(sizeof(Plugin));
	plugin_init(rv, self->machine.callback, self->library.path);
	return &rv->machine;
}


CMachineInfo* plugin_psycle_test(const char* path)
{	
	CMachineInfo* rv = 0;	
	
	if (path && strcmp(path, "") != 0) {
		GETINFO GetInfo;
		Library library;

		library_init(&library);
		library_load(&library, path);					
		GetInfo =(GETINFO)library_functionpointer(&library, "GetInfo");
		if (GetInfo != NULL) {	
			CMachineInfo* pInfo = GetInfo();
			if (pInfo) {
				rv = (CMachineInfo*) malloc(sizeof(CMachineInfo));
				rv->Author = _strdup(pInfo->Author);
				rv->Command = _strdup(pInfo->Command);
				rv->Flags = pInfo->Flags;
				rv->Name = _strdup(pInfo->Name);
				rv->numCols = pInfo->numCols;
				rv->numParameters = pInfo->numParameters;
				rv->ShortName = _strdup(pInfo->ShortName);
				rv->APIVersion = pInfo->APIVersion;
				rv->PlugVersion = pInfo->PlugVersion;
			}
		}
		library_dispose(&library);
	}
	return rv;
}

void seqtick(Plugin* self, int channel, const PatternEvent* event)
{
	mi_seqtick(self->mi, channel, event->note, event->inst, event->cmd, event->parameter);
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


CMachineInfo* info(Plugin* self)
{
	CMachineInfo* pInfo = 0;
	
	if (self->library.module != 0) {
		GETINFO GetInfo;

		GetInfo =(GETINFO)library_functionpointer(&self->library, "GetInfo");
		if (GetInfo != 0) {	
			pInfo = GetInfo();
		}
	}
	return pInfo;
}

void parametertweak(Plugin* self, int par, int val)
{
	mi_parametertweak(self->mi, par, val);
}

int describevalue(Plugin* self, char* txt, int const param, int const value)
{ 
	return mi_describevalue(self->mi, txt, param, value);
}

int value(Plugin* self, int const param)
{
	return mi_val(self->mi, param);
}

void setvalue(Plugin* self, int const param, int const value)
{
	mi_setval(self->mi, param, value);
}

unsigned int numinputs(Plugin* self)
{
	if (info(self)) {
		return (info(self)->Flags & 3 == 3) ? 0 : 2;
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

void loadspecific(Plugin* self, RiffFile* file)
{
	unsigned int size;

	rifffile_read(file, &size, sizeof(size)); // size of whole structure
	if(size) {
		unsigned int count;
		unsigned int i;

		rifffile_read(file, &count, sizeof(count));  // size of vars
		for (i = 0; i < count; i++) {
			int temp;
			
			rifffile_read(file, &temp, sizeof(temp));			
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
			rifffile_read(file, pData, size); // Read internal data.			
			mi_putdata(self->mi, pData); // Internal load
			free(pData);
		}						
	}	
}			

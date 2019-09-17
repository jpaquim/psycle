// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net
#include "plugin.h"
#include "pattern.h"

typedef CMachineInfo * (*GETINFO)(void);
typedef CMachineInterface * (*CREATEMACHINE)(void);

static int hostevent(Plugin* self, int const eventNr, int const val1, float const val2);
static void generateaudio(Plugin* self, BufferContext*);
static void seqtick(Plugin* self, int channel, const PatternEvent* event);
static void sequencertick(Plugin* self);
static CMachineInfo* info(Plugin* self);
static void parametertweak(Plugin* self, int par, int val);
static int describevalue(Plugin*, char* txt, int const param, int const value);
static int value(Plugin*, int const param);
static void setvalue(Plugin*, int const param, int const value);
static void dispose(Plugin* self);
static int mode(Plugin* self);
static unsigned int numinputs(Plugin*);
static unsigned int numoutputs(Plugin*);
		
void plugin_init(Plugin* self, const char* path)
{
	CMachineInfo* pInfo;
	int err;

	machine_init(&self->machine);	
	self->dll = LoadLibrary(path);
	err = GetLastError();	
	self->mi = 0;
	self->machine.hostevent = hostevent;
	self->machine.seqtick = seqtick;
	self->machine.sequencertick = sequencertick;
	self->machine.info = info;
	self->machine.parametertweak = parametertweak;
	self->machine.describevalue = describevalue;
	self->machine.setvalue = setvalue;
	self->machine.value = value;
	self->machine.dispose = dispose;
	self->machine.mode = mode;
	self->machine.generateaudio = generateaudio;
	self->machine.numinputs = numinputs;
	self->machine.numoutputs = numoutputs;
	pInfo = info(self);	
	if (!pInfo)
	{
		FreeLibrary(self->dll);
		self->dll = NULL;		
	} else {		
		self->mi = mi_create(self->dll);
		if (!self->mi) {
			FreeLibrary(self->dll);			
		} else {
			int gbp;	
			mi_init(self->mi);
			for (gbp = 0; gbp< pInfo->numParameters; gbp++) {
				mi_parametertweak(self->mi, gbp, pInfo->Parameters[gbp]->DefValue);
			}		
		}
	}	
}

void dispose(Plugin* self)
{	
	if (self->dll) {
		FreeLibrary(self->dll);
		self->mi = 0;
		self->dll = 0;
	}		
	machine_dispose(&self->machine);
}


CMachineInfo* plugin_psycle_test(const char* path)
{
	GETINFO GetInfo;
	CMachineInfo* rv;
	HINSTANCE dll;	
	int err;

	rv = 0;
	dll = LoadLibrary(path);
	err = GetLastError();					
	GetInfo =(GETINFO)GetProcAddress(dll, "GetInfo");
	if (GetInfo != NULL) {	
		CMachineInfo* pInfo = GetInfo();
		if (pInfo) {
			rv = (CMachineInfo*) malloc(sizeof(CMachineInfo));
			rv->Author = strdup(pInfo->Author);
			rv->Command = strdup(pInfo->Command);
			rv->Flags = pInfo->Flags;
			rv->Name = strdup(pInfo->Name);
			rv->numCols = pInfo->numCols;
			rv->numParameters = pInfo->numParameters;
			rv->ShortName = strdup(pInfo->ShortName);
			rv->APIVersion = pInfo->APIVersion;
			rv->PlugVersion = pInfo->PlugVersion;
		}
	}
	FreeLibrary(dll);	
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

void sequencertick(Plugin* self)
{
	mi_sequencertick(self->mi);
}


CMachineInfo* info(Plugin* self)
{
	CMachineInfo* pInfo = 0;
	
	if (self->dll) {
		GETINFO GetInfo;
		GetInfo =(GETINFO)GetProcAddress(self->dll, "GetInfo");
		if (GetInfo != NULL) {	
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

int mode(Plugin* self)
{
	if (self->machine.info(self) && (self->machine.info(self)->Flags == 3 )) {
		return MACHMODE_GENERATOR;
	} else {
		return MACHMODE_FX;
	}	
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


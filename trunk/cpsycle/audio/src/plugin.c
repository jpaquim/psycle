// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net
#include "plugin.h"

typedef CMachineInfo * (*GETINFO)(void);
typedef CMachineInterface * (*CREATEMACHINE)(void);

static void work(Plugin* self, int numsamples, int tracks);
static int hostevent(Plugin* self, int const eventNr, int const val1, float const val2);
static void seqtick(Plugin* self, int channel, int note, int ins, int cmd, int val);
static void sequencertick(Plugin* self);
static CMachineInfo* info(Plugin* self);
static void parametertweak(Plugin* self, int par, int val);
static int describevalue(Plugin*, char* txt, int const param, int const value);
static int value(Plugin*, int const param);
static void setvalue(Plugin*, int const param, int const value);
static void dispose(Plugin* self);
static int mode(Plugin* self);
		
void plugin_init(Plugin* self, const char* path)
{
	CMachineInfo* pInfo;
	int err;

	machine_init(&self->machine);	
///	CMachineInterface* _pInterface;
	self->dll = LoadLibrary(path);
	err = GetLastError();	
	self->mi = 0;
	self->machine.work = work;
	self->machine.mi_hostevent = hostevent;
	self->machine.seqtick = seqtick;
	self->machine.sequencertick = sequencertick;
	self->machine.info = info;
	self->machine.parametertweak = parametertweak;
	self->machine.describevalue = describevalue;
	self->machine.setvalue = setvalue;
	self->machine.value = value;
	self->machine.dispose = dispose;
	self->machine.mode = mode;
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
			self->machine.numInputs = 2; //self->machine.mode(self) == MACHMODE_FX ? 2 : 0;;
			self->machine.numOutputs = 2;
			self->machine.inputs = (float**)malloc(sizeof(float*)*self->machine.numInputs);
			self->machine.outputs = (float**)malloc(sizeof(float*)*self->machine.numOutputs);
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
		free(self->machine.inputs);
		free(self->machine.outputs);
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
			rv->Version = pInfo->Version;
		}
	}
	FreeLibrary(dll);	
	return rv;
}

void work(Plugin* self, int numsamples, int tracks)
{	
	mi_work(self->mi, self->machine.outputs[0], self->machine.outputs[1], numsamples, tracks);	
}

void seqtick(Plugin* self, int channel, int note, int ins, int cmd, int val)
{
	mi_seqtick(self->mi, channel, note, ins, cmd, val);
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

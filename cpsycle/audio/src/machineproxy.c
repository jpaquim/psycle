// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "machineproxy.h"
#include "machines.h"
#include "pattern.h"
#include <string.h>
#include <operations.h>
#include <stdlib.h>
#include <windows.h>
#include <excpt.h>

// proxy

static Buffer* machineproxy_mix(MachineProxy*, int slot, unsigned int amount, MachineSockets*, Machines*);
static void machineproxy_work(MachineProxy*, BufferContext*);
static void machineproxy_generateaudio(MachineProxy*, BufferContext* bc);
static void machineproxy_dispose(MachineProxy*);
static int machineproxy_mode(MachineProxy*);
static int machineproxy_numinputs(MachineProxy*);
static unsigned int machineproxy_numoutputs(MachineProxy*);
static void machineproxy_parametertweak(MachineProxy*, int par, int val);
static int machineproxy_describevalue(MachineProxy*, char* txt, int const param, int const value);
static int machineproxy_value(MachineProxy*, int const param);
static const CMachineInfo* machineproxy_info(MachineProxy*);
static unsigned int machineproxy_numparameters(MachineProxy*);
static unsigned int machineproxy_numcols(MachineProxy*);
static const CMachineParameter* machineproxy_parameter(MachineProxy*, unsigned int par);
static int machineproxy_paramviewoptions(MachineProxy*);
static void machineproxy_loadspecific(MachineProxy*, PsyFile* file, unsigned int slot, Machines* machines);
static unsigned int machineproxy_samplerate(MachineProxy*);
static unsigned int machineproxy_bpm(MachineProxy*);
static unsigned int machineproxy_slot(MachineProxy*);
static void machineproxy_setslot(MachineProxy*, int slot);
static struct Samples* machineproxy_samples(MachineProxy*);
static struct Machines* machineproxy_machines(MachineProxy*);
static struct Instruments* machineproxy_instruments(MachineProxy*);
static void machineproxy_setcallback(MachineProxy*, MachineCallback);
static int machineproxy_parameterlabel(MachineProxy*, char* txt, int param);
static int machineproxy_parametername(MachineProxy*, char* txt, int param);
static int machineproxy_haseditor(MachineProxy*);
static void machineproxy_seteditorhandle(MachineProxy*, void* handle);
static void machineproxy_editorsize(MachineProxy*, int* width, int* height);
static void machineproxy_editoridle(MachineProxy*);

static int FilterException(MachineProxy* proxy, const char* msg, int code, struct _EXCEPTION_POINTERS *ep) 
{	
	char txt[512];
	proxy->crashed = 1;	
		
	if (proxy->client->info(proxy->client)) {
		_snprintf(txt, 512, "%s crashed \n\r %s", proxy->client->info(proxy->client)->ShortName, msg);
	} else {
		_snprintf(txt, 512, "Machine crashed");
	}
	MessageBox(0, txt, "Psycle Host Exception", MB_OK | MB_ICONERROR);
	return EXCEPTION_EXECUTE_HANDLER;
}

void machineproxy_init(MachineProxy* self, Machine* client)
{
	machine_init(&self->machine, client->callback);
	self->crashed = 0;
	self->client = client;
	self->machine.mix = machineproxy_mix;
	self->machine.work = machineproxy_work;
	self->machine.generateaudio = machineproxy_generateaudio;
	self->machine.dispose = machineproxy_dispose;
	self->machine.mode = machineproxy_mode;
	self->machine.numinputs = machineproxy_numinputs;
	self->machine.numoutputs = machineproxy_numoutputs;
	self->machine.parametertweak = machineproxy_parametertweak;
	self->machine.describevalue = machineproxy_describevalue;
	self->machine.value = machineproxy_value;
	self->machine.info = machineproxy_info;
	self->machine.numparameters = machineproxy_numparameters;
	self->machine.numcols = machineproxy_numcols;
	self->machine.parameter = machineproxy_parameter;
	self->machine.paramviewoptions = machineproxy_paramviewoptions;
	self->machine.parameterlabel = machineproxy_parameterlabel;
	self->machine.parametername = machineproxy_parametername;
	self->machine.loadspecific = machineproxy_loadspecific;
	self->machine.samplerate = machineproxy_samplerate;
	self->machine.bpm = machineproxy_bpm;
	self->machine.machines = machineproxy_machines;
	self->machine.instruments = machineproxy_instruments;
	self->machine.samples = machineproxy_samples;
	self->machine.setcallback = machineproxy_setcallback;
	self->machine.setslot = machineproxy_setslot;
	self->machine.slot = machineproxy_slot;
	self->machine.haseditor = machineproxy_haseditor;
	self->machine.seteditorhandle = machineproxy_seteditorhandle;
	self->machine.editorsize = machineproxy_editorsize;
	self->machine.editoridle = machineproxy_editoridle;
}

Buffer* machineproxy_mix(MachineProxy* self, int slot, unsigned int amount, MachineSockets* sockets, Machines* machines)
{
	Buffer* rv = 0;

	if (self->crashed == 0) {
		__try {
			rv = self->client->mix(self->client, slot, amount, sockets, machines);
		} __except(FilterException(self, "mix", GetExceptionCode(), GetExceptionInformation())) {			
		}
	}
	return rv;
}

void machineproxy_work(MachineProxy* self, BufferContext* bc)
{
	if (self->crashed == 0) {
		__try {
			self->client->work(self->client, bc);
		} __except(FilterException(self, "work", GetExceptionCode(), GetExceptionInformation())) {
		}
	}
}

void machineproxy_generateaudio(MachineProxy* self, BufferContext* bc)
{
	if (self->crashed == 0) {
		__try {
			self->client->generateaudio(self->client, bc);
		} __except(FilterException(self, "generateaudio", GetExceptionCode(), GetExceptionInformation())) {
		}		
	}
}

void machineproxy_dispose(MachineProxy* self)
{
	if (self->crashed == 0) {
		__try {
			self->client->dispose(self->client);
			free(self->client);
		} __except(FilterException(self, "dispose", GetExceptionCode(), GetExceptionInformation())) {
		}
	}
}

int machineproxy_mode(MachineProxy* self)
{
	int rv = 0;

	if (self->crashed == 0) {
		__try {
			rv = self->client->mode(self->client);
		} __except(FilterException(self, "mode", GetExceptionCode(), GetExceptionInformation())) {			
		}
	}
	return rv;
}

int machineproxy_numinputs(MachineProxy* self)
{
	int rv = 0;

	if (self->crashed == 0) {
		__try {
			rv = self->client->numinputs(self->client);
		} __except(FilterException(self, "numinputs", GetExceptionCode(), GetExceptionInformation())) {			
		}
	}
	return rv;
}

unsigned int machineproxy_numoutputs(MachineProxy* self)
{ 
	unsigned int rv = 0;

	if (self->crashed == 0) {
		__try {
			rv = self->client->numoutputs(self->client);
		} __except(FilterException(self, "numoutputs",  GetExceptionCode(), GetExceptionInformation())) {			
		}
	}
	return rv;
}	

void machineproxy_parametertweak(MachineProxy* self, int par, int val)
{
	if (self->crashed == 0) {
		__try {
			self->client->parametertweak(self->client, par, val);
		} __except(FilterException(self, "parametertweak", GetExceptionCode(), GetExceptionInformation())) {		
		}	
	}
}

int machineproxy_describevalue(MachineProxy* self, char* txt, int const param, int const value)
{
	int rv = 0;

	txt[0] = '\0';
	if (self->crashed == 0) {
		__try {
			rv = self->client->describevalue(self->client, txt, param, value);
		} __except(FilterException(self, "describevalue", GetExceptionCode(), GetExceptionInformation())) {			
		}
	}
	return rv;
}

int machineproxy_value(MachineProxy* self, int const param)
{
	int rv = 0;

	if (self->crashed == 0) {
		__try {
			rv = self->client->value(self->client, param);
		} __except(FilterException(self, "value", GetExceptionCode(), GetExceptionInformation())) {			
		}
	}
	return rv;
}

const CMachineInfo* machineproxy_info(MachineProxy* self)
{ 
	const CMachineInfo* rv = 0;

	if (self->crashed == 0) {
		__try {
			rv = self->client->info(self->client);
		} __except(FilterException(self, "info", GetExceptionCode(), GetExceptionInformation())) {			
		}
	}
	return rv;
}

unsigned int machineproxy_numparameters(MachineProxy* self)
{
	unsigned int rv = 0;

	if (self->crashed == 0) {
		__try {
			rv = self->client->numparameters(self->client);
		} __except(FilterException(self, "numparameters", GetExceptionCode(), GetExceptionInformation())) {			
		}
	}
	return rv;
}

unsigned int machineproxy_numcols(MachineProxy* self)
{ 
	unsigned int rv = 0;

	if (self->crashed == 0) {
		__try {
			rv = self->client->numcols(self->client);
		} __except(FilterException(self, "numcols", GetExceptionCode(), GetExceptionInformation())) {			
		}
	}
	return rv;
}

const CMachineParameter* machineproxy_parameter(MachineProxy* self, unsigned int par)
{
	const CMachineParameter* rv = 0;

	if (self->crashed == 0) {
		__try {
			rv = self->client->parameter(self->client, par);
		} __except(FilterException(self, "parameter", GetExceptionCode(), GetExceptionInformation())) {			
		}
	}
	return rv;
}

int machineproxy_paramviewoptions(MachineProxy* self)
{
	int rv = 0;

	if (self->crashed == 0) {
		__try {
			rv = self->client->paramviewoptions(self->client);
		} __except(FilterException(self, "paramviewoptions", GetExceptionCode(), GetExceptionInformation())) {
			rv = 0;
		}
	}
	return rv;
}

void machineproxy_loadspecific(MachineProxy* self, PsyFile* file, unsigned int slot, Machines* machines)
{
	if (self->crashed == 0) {
		__try {
			self->client->loadspecific(self->client, file, slot, machines);
		} __except(FilterException(self,"loadspecific",  GetExceptionCode(), GetExceptionInformation())) {
		}	
	}
}

unsigned int machineproxy_samplerate(MachineProxy* self)
{
	unsigned int rv = 0;

	if (self->crashed == 0) {
		__try {
			rv = self->client->samplerate(self->client);
		} __except(FilterException(self, "samplerate", GetExceptionCode(), GetExceptionInformation())) {			
		}
	}
	return rv;
}

unsigned int machineproxy_bpm(MachineProxy* self)
{
	unsigned int rv = 0;

	if (self->crashed == 0) {
		__try {
			rv = self->client->bpm(self->client);
		} __except(FilterException(self, "bpm", GetExceptionCode(), GetExceptionInformation())) {			
		}
	}
	return rv;
}

struct Samples* machineproxy_samples(MachineProxy* self)
{
	struct Samples* rv = 0;

	if (self->crashed == 0) {
		__try {
			rv = self->client->samples(self->client);
		} __except(FilterException(self, "samples", GetExceptionCode(), GetExceptionInformation())) {			
		}
	}
	return rv;
}

struct Machines* machineproxy_machines(MachineProxy* self)
{
	struct Machines* rv = 0;

	if (self->crashed == 0) {
		__try { 
			rv = self->client->machines(self->client);
		} __except(FilterException(self, "machines", GetExceptionCode(), GetExceptionInformation())) {			
		}
	}
	return rv;
}

struct Instruments* machineproxy_instruments(MachineProxy* self)
{
	struct Instruments* rv = 0;

	if (self->crashed == 0) {
		__try { 
			rv = self->client->instruments(self->client); 
		} __except(FilterException(self, "instruments", GetExceptionCode(), GetExceptionInformation())) {			
		}
	}
	return rv;
}

void machineproxy_setcallback(MachineProxy* self, MachineCallback callback)
{ 
	if (self->crashed == 0) {
		__try { 
			self->client->setcallback(self->client, callback);
		} __except(FilterException(self, "setcallback", GetExceptionCode(), GetExceptionInformation())) {
		}
	}
}

unsigned int machineproxy_slot(MachineProxy* self)
{
	unsigned int rv = 0;

	if (self->crashed == 0) {
		__try { 
			rv = self->client->slot(self->client);
		} __except(FilterException(self, "slot", GetExceptionCode(), GetExceptionInformation())) {			
		}
	}
	return rv;
}

void machineproxy_setslot(MachineProxy* self, int slot)
{
	if (self->crashed == 0) {
		__try { 
			self->client->setslot(self->client, slot);
		} __except(FilterException(self, "setslot", GetExceptionCode(), GetExceptionInformation())) {
		}
	}
}

int machineproxy_parameterlabel(MachineProxy* self, char* txt, int param)
{
	int rv = 0;

	txt[0] = '\0'; 
	if (self->crashed == 0) {
		__try { 
			rv = self->client->parameterlabel(self->client, txt, param);			
		} __except(FilterException(self, "parameterlabel", GetExceptionCode(), GetExceptionInformation())) {
			txt = '\0';
		}
	}
	return rv;
}

int machineproxy_parametername(MachineProxy* self, char* txt, int param)
{
	int rv = 0;

	txt[0] = '\0'; 
	if (self->crashed == 0) {
		__try { 
			rv = self->client->parametername(self->client, txt, param);			
		} __except(FilterException(self, "parametername", GetExceptionCode(), GetExceptionInformation())) {
			txt = '\0';			
		}
	}
	return rv;
}

int machineproxy_haseditor(MachineProxy* self)
{ 
	int rv = 0;
	
	if (self->crashed == 0) {
		__try { 
			rv = self->client->haseditor(self->client);
		} __except(FilterException(self, "haseditor", GetExceptionCode(), GetExceptionInformation())) {			
		}
	}
	return rv;
}

void machineproxy_seteditorhandle(MachineProxy* self, void* handle)
{
	if (self->crashed == 0) {
		__try { 
			self->client->seteditorhandle(self->client, handle);
		} __except(FilterException(self, "seteditorhandle", GetExceptionCode(), GetExceptionInformation())) {			
		}
	}	
}

void machineproxy_editorsize(MachineProxy* self, int* width, int* height)
{
	if (self->crashed == 0) {
		__try { 
			self->client->editorsize(self->client, width, height);
		} __except(FilterException(self, "editorsize", GetExceptionCode(), GetExceptionInformation())) {
			*width = 0;
			*height = 0;
		}
	}	
}

static void machineproxy_editoridle(MachineProxy* self)
{
	if (self->crashed == 0) {
		__try { 
			self->client->editoridle(self->client);
		} __except(FilterException(self, "editoridle", GetExceptionCode(), GetExceptionInformation())) {

		}
	}
}
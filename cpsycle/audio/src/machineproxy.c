// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"
#include "../../detail/os.h"

#include "machineproxy.h"
#include "machines.h"
#include "pattern.h"
#include "songio.h"
#include <string.h>
#include <operations.h>
#include <stdlib.h>
#if defined DIVERSALIS__OS__MICROSOFT  
#include <windows.h>
#include <excpt.h>
#endif
#include <portable.h>

// proxy

static Buffer* machineproxy_mix(MachineProxy*, size_t slot, unsigned int amount, MachineSockets*, Machines*);
static void machineproxy_work(MachineProxy*, BufferContext*);
static void machineproxy_generateaudio(MachineProxy*, BufferContext* bc);
static void machineproxy_dispose(MachineProxy*);
static int machineproxy_mode(MachineProxy*);
static int machineproxy_numinputs(MachineProxy*);
static unsigned int machineproxy_numoutputs(MachineProxy*);
static void machineproxy_parametertweak(MachineProxy*, int par, int val);
static void machineproxy_patterntweak(MachineProxy* self, int par, int val);
static int machineproxy_describevalue(MachineProxy*, char* txt, int const param, int const value);
static int machineproxy_parametervalue(MachineProxy*, int const param);
static void machineproxy_setpanning(MachineProxy*, amp_t);
static amp_t machineproxy_panning(MachineProxy*);
static void machineproxy_mute(MachineProxy*);
static void machineproxy_unmute(MachineProxy*);
static int machineproxy_muted(MachineProxy*);
static void machineproxy_bypass(MachineProxy*);
static void machineproxy_unbypass(MachineProxy*);
static int machineproxy_bypassed(MachineProxy*);
static const MachineInfo* machineproxy_info(MachineProxy*);
static int machineproxy_parametertype(MachineProxy*, int param);
static void machineproxy_parameterrange(MachineProxy*, int numparam, int* minval, int* maxval);
static unsigned int machineproxy_numparameters(MachineProxy*);
static unsigned int machineproxy_numparametercols(MachineProxy*);
static int machineproxy_paramviewoptions(MachineProxy*);
static void machineproxy_loadspecific(MachineProxy*, SongFile*, unsigned int slot);
static void machineproxy_savespecific(MachineProxy*, SongFile*, unsigned int slot);
static unsigned int machineproxy_samplerate(MachineProxy*);
static unsigned int machineproxy_bpm(MachineProxy*);
static uintptr_t machineproxy_slot(MachineProxy*);
static void machineproxy_setslot(MachineProxy*, uintptr_t slot);
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
static const char* machineproxy_editname(MachineProxy* self);
static void machineproxy_seteditname(MachineProxy* self, const char* name);

#if defined DIVERSALIS__OS__MICROSOFT
static int FilterException(MachineProxy* proxy, const char* msg, int code, struct _EXCEPTION_POINTERS *ep) 
{	
	char txt[512];
	proxy->crashed = 1;	
		
	if (proxy->client->vtable->info(proxy->client)) {
		psy_snprintf(txt, 512, "%s crashed \n\r %s", proxy->client->vtable->info(proxy->client)->ShortName, msg);
	} else {
		psy_snprintf(txt, 512, "Machine crashed");
	}
	MessageBox(0, txt, "Psycle Host Exception", MB_OK | MB_ICONERROR);
	return EXCEPTION_EXECUTE_HANDLER;
}
#endif

static MachineVtable vtable;
static int vtable_initialized = 0;

static void vtable_init(MachineProxy* self)
{
	if (!vtable_initialized) {
		vtable = *self->machine.vtable;
		vtable.mix = (fp_machine_mix) machineproxy_mix;
		vtable.work = (fp_machine_work) machineproxy_work;
		vtable.generateaudio = (fp_machine_generateaudio) machineproxy_generateaudio;
		vtable.dispose = (fp_machine_dispose) machineproxy_dispose;
		vtable.mode = (fp_machine_mode) machineproxy_mode;
		vtable.numinputs = (fp_machine_numinputs) machineproxy_numinputs;
		vtable.numoutputs = (fp_machine_numoutputs) machineproxy_numoutputs;
		vtable.parametertweak = (fp_machine_parametertweak) machineproxy_parametertweak;
		vtable.patterntweak = (fp_machine_patterntweak) machineproxy_patterntweak;
		vtable.describevalue = (fp_machine_describevalue) machineproxy_describevalue;
		vtable.parametervalue = (fp_machine_parametervalue) machineproxy_parametervalue;
		vtable.setpanning = (fp_machine_setpanning) machineproxy_setpanning;
		vtable.panning = (fp_machine_panning) machineproxy_panning;
		vtable.mute = (fp_machine_mute) machineproxy_mute;
		vtable.unmute = (fp_machine_unmute) machineproxy_unmute;
		vtable.muted = (fp_machine_muted) machineproxy_muted;
		vtable.bypass = (fp_machine_bypass) machineproxy_bypass;
		vtable.unbypass = (fp_machine_unbypass) machineproxy_unbypass;
		vtable.bypassed = (fp_machine_bypassed) machineproxy_bypassed;
		vtable.info = (fp_machine_info) machineproxy_info;
		vtable.parameterrange = (fp_machine_parameterrange) machineproxy_parameterrange;
		vtable.parametertype = (fp_machine_parametertype) machineproxy_parametertype;
		vtable.numparameters = (fp_machine_numparameters) machineproxy_numparameters;
		vtable.numparametercols = (fp_machine_numparametercols) machineproxy_numparametercols;	
		vtable.paramviewoptions = (fp_machine_paramviewoptions) machineproxy_paramviewoptions;
		vtable.parameterlabel = (fp_machine_parameterlabel) machineproxy_parameterlabel;
		vtable.parametername = (fp_machine_parametername) machineproxy_parametername;
		vtable.loadspecific = (fp_machine_loadspecific) machineproxy_loadspecific;
		vtable.savespecific = (fp_machine_savespecific) machineproxy_savespecific;
		vtable.samplerate = (fp_machine_samplerate) machineproxy_samplerate;
		vtable.bpm = (fp_machine_bpm) machineproxy_bpm;
		vtable.machines = (fp_machine_machines) machineproxy_machines;
		vtable.instruments = (fp_machine_instruments) machineproxy_instruments;
		vtable.samples = (fp_machine_samples) machineproxy_samples;
		vtable.setcallback = (fp_machine_setcallback) machineproxy_setcallback;
		vtable.setslot = (fp_machine_setslot) machineproxy_setslot;
		vtable.slot = (fp_machine_slot) machineproxy_slot;
		vtable.haseditor = (fp_machine_haseditor) machineproxy_haseditor;
		vtable.seteditorhandle = (fp_machine_seteditorhandle) machineproxy_seteditorhandle;
		vtable.editorsize = (fp_machine_editorsize) machineproxy_editorsize;
		vtable.editoridle = (fp_machine_editoridle) machineproxy_editoridle;
		vtable.seteditname = (fp_machine_seteditname) machineproxy_seteditname;
		vtable.editname = (fp_machine_editname) machineproxy_editname;
		vtable_initialized = 1;
	}
}

void machineproxy_init(MachineProxy* self, Machine* client)
{
	machine_init(&self->machine, client->callback);
	vtable_init(self);
	self->machine.vtable = &vtable;
	self->crashed = 0;
	self->client = client;	
}

Buffer* machineproxy_mix(MachineProxy* self, size_t slot, unsigned int amount, MachineSockets* sockets, Machines* machines)
{
	Buffer* rv = 0;

	if (self->crashed == 0) {
#if defined DIVERSALIS__OS__MICROSOFT
		__try 
#endif		
		{
			rv = self->client->vtable->mix(self->client, slot, amount, sockets, machines);
		} 
#if defined DIVERSALIS__OS__MICROSOFT
		__except(FilterException(self, "mix", GetExceptionCode(), GetExceptionInformation())) {			
		}
#endif		
	}
	return rv;
}

void machineproxy_work(MachineProxy* self, BufferContext* bc)
{
	if (self->crashed == 0) {
#if defined DIVERSALIS__OS__MICROSOFT        
		__try
#endif				
		{
			self->client->vtable->work(self->client, bc);
		}
#if defined DIVERSALIS__OS__MICROSOFT		
		__except(FilterException(self, "work", GetExceptionCode(), GetExceptionInformation())) {
		}
#endif		
	}
}

void machineproxy_generateaudio(MachineProxy* self, BufferContext* bc)
{
	if (self->crashed == 0) {
#if defined DIVERSALIS__OS__MICROSOFT        
		__try
#endif		
		{
			self->client->vtable->generateaudio(self->client, bc);
		}
#if defined DIVERSALIS__OS__MICROSOFT		
		__except(FilterException(self, "generateaudio", GetExceptionCode(), GetExceptionInformation())) {
		}
#endif		
	}
}

void machineproxy_dispose(MachineProxy* self)
{
	if (self->crashed == 0) {
#if defined DIVERSALIS__OS__MICROSOFT        
		__try
#endif		
		{
			self->client->vtable->dispose(self->client);
			free(self->client);
		}
#if defined DIVERSALIS__OS__MICROSOFT		
		__except(FilterException(self, "dispose", GetExceptionCode(), GetExceptionInformation())) {
		}
#endif		
	}
}

int machineproxy_mode(MachineProxy* self)
{
	int rv = 0;

	if (self->crashed == 0) {
#if defined DIVERSALIS__OS__MICROSOFT        
		__try
#endif		
		{
			rv = self->client->vtable->mode(self->client);
		}
#if defined DIVERSALIS__OS__MICROSOFT		
		__except(FilterException(self, "mode", GetExceptionCode(), GetExceptionInformation())) {			
		}
#endif		
	}
	return rv;
}

int machineproxy_numinputs(MachineProxy* self)
{
	int rv = 0;

	if (self->crashed == 0) {
#if defined DIVERSALIS__OS__MICROSOFT        
		__try
#endif		
		{
			rv = self->client->vtable->numinputs(self->client);
		}
#if defined DIVERSALIS__OS__MICROSOFT		
		__except(FilterException(self, "numinputs", GetExceptionCode(), GetExceptionInformation())) {			
		}
#endif		
	}
	return rv;
}

unsigned int machineproxy_numoutputs(MachineProxy* self)
{ 
	unsigned int rv = 0;

	if (self->crashed == 0) {
#if defined DIVERSALIS__OS__MICROSOFT        
		__try
#endif		
		{
			rv = self->client->vtable->numoutputs(self->client);
		}
#if defined DIVERSALIS__OS__MICROSOFT		
		__except(FilterException(self, "numoutputs",  GetExceptionCode(), GetExceptionInformation())) {			
		}
#endif		
	}
	return rv;
}	

void machineproxy_parametertweak(MachineProxy* self, int par, int val)
{
	if (self->crashed == 0) {
#if defined DIVERSALIS__OS__MICROSOFT        
		__try
#endif		
		{
			self->client->vtable->parametertweak(self->client, par, val);
		}
#if defined DIVERSALIS__OS__MICROSOFT		
		__except(FilterException(self, "parametertweak", GetExceptionCode(), GetExceptionInformation())) {		
		}
#endif		
	}
}

void machineproxy_patterntweak(MachineProxy* self, int par, int val)
{
	if (self->crashed == 0) {
#if defined DIVERSALIS__OS__MICROSOFT        
		__try
#endif		
		{
			self->client->vtable->patterntweak(self->client, par, val);
		}
#if defined DIVERSALIS__OS__MICROSOFT		
		__except(FilterException(self, "parametertweak", GetExceptionCode(), GetExceptionInformation())) {		
		}
#endif		
	}
}

int machineproxy_describevalue(MachineProxy* self, char* txt, int const param, int const value)
{
	int rv = 0;

	txt[0] = '\0';
	if (self->crashed == 0) {
#if defined DIVERSALIS__OS__MICROSOFT        
		__try
#endif		
		{
			rv = self->client->vtable->describevalue(self->client, txt, param, value);
		}
#if defined DIVERSALIS__OS__MICROSOFT		
		__except(FilterException(self, "describevalue", GetExceptionCode(), GetExceptionInformation())) {			
		}
#endif		
	}
	return rv;
}

int machineproxy_parametervalue(MachineProxy* self, int const param)
{
	int rv = 0;

	if (self->crashed == 0) {
#if defined DIVERSALIS__OS__MICROSOFT        
		__try
#endif		
		{
			rv = self->client->vtable->parametervalue(self->client, param);
		}
#if defined DIVERSALIS__OS__MICROSOFT		
		__except(FilterException(self, "parametervalue", GetExceptionCode(), GetExceptionInformation())) {			
		}
#endif		
	}
	return rv;
}

void machineproxy_setpanning(MachineProxy* self, amp_t panning)
{	
	if (self->crashed == 0) {
#if defined DIVERSALIS__OS__MICROSOFT        
		__try
#endif		
		{
			self->client->vtable->setpanning(self->client, panning);
		}
#if defined DIVERSALIS__OS__MICROSOFT		
		__except(FilterException(self, "setpanning", GetExceptionCode(), GetExceptionInformation())) {			
		}
#endif		
	}	
}

amp_t machineproxy_panning(MachineProxy* self)
{
	amp_t rv = 0;

	if (self->crashed == 0) {
#if defined DIVERSALIS__OS__MICROSOFT        
		__try
#endif		
		{
			rv = self->client->vtable->panning(self->client);
		}
#if defined DIVERSALIS__OS__MICROSOFT		
		__except(FilterException(self, "panning", GetExceptionCode(), GetExceptionInformation())) {			
		}
#endif		
	}
	return rv;
}

void machineproxy_mute(MachineProxy* self)
{
	if (self->crashed == 0) {
#if defined DIVERSALIS__OS__MICROSOFT        
		__try
#endif		
		{
			self->client->vtable->mute(self->client);
		}
#if defined DIVERSALIS__OS__MICROSOFT		
		__except (FilterException(self, "mute", GetExceptionCode(), GetExceptionInformation())) {
		}
#endif		
	}
}

void machineproxy_unmute(MachineProxy* self)
{
	if (self->crashed == 0) {
#if defined DIVERSALIS__OS__MICROSOFT        
		__try
#endif		
		{
			self->client->vtable->unmute(self->client);
		}
#if defined DIVERSALIS__OS__MICROSOFT		
		__except (FilterException(self, "unmute", GetExceptionCode(), GetExceptionInformation())) {
		}
#endif		
	}
}

int machineproxy_muted(MachineProxy* self)
{
	int rv = 0;

	if (self->crashed == 0) {
#if defined DIVERSALIS__OS__MICROSOFT        
		__try
#endif		
		{
			rv = self->client->vtable->muted(self->client);
		}
#if defined DIVERSALIS__OS__MICROSOFT		
		__except (FilterException(self, "muted", GetExceptionCode(), GetExceptionInformation())) {
		}
#endif		
	}
	return rv;
}

void machineproxy_bypass(MachineProxy* self)
{
	if (self->crashed == 0) {
#if defined DIVERSALIS__OS__MICROSOFT        
		__try
#endif		
		{
			self->client->vtable->bypass(self->client);
		}
#if defined DIVERSALIS__OS__MICROSOFT		
		__except (FilterException(self, "bypass", GetExceptionCode(), GetExceptionInformation())) {
		}
#endif		
	}
}

void machineproxy_unbypass(MachineProxy* self)
{
	if (self->crashed == 0) {
#if defined DIVERSALIS__OS__MICROSOFT        
		__try
#endif		
		{
			self->client->vtable->unbypass(self->client);
		}
#if defined DIVERSALIS__OS__MICROSOFT		
		__except (FilterException(self, "unbypass", GetExceptionCode(), GetExceptionInformation())) {
		}
#endif		
	}
}

int machineproxy_bypassed(MachineProxy* self)
{
	int rv = 0;

	if (self->crashed == 0) {
#if defined DIVERSALIS__OS__MICROSOFT        
		__try
#endif		
		{
			rv = self->client->vtable->bypassed(self->client);
		}
#if defined DIVERSALIS__OS__MICROSOFT		
		__except (FilterException(self, "panning", GetExceptionCode(), GetExceptionInformation())) {
		}
#endif		
	}
	return rv;
}

const MachineInfo* machineproxy_info(MachineProxy* self)
{ 
	const MachineInfo* rv = 0;

	if (self->crashed == 0) {
#if defined DIVERSALIS__OS__MICROSOFT        
		__try
#endif		
		{
			rv = self->client->vtable->info(self->client);
		}
#if defined DIVERSALIS__OS__MICROSOFT		
		__except(FilterException(self, "info", GetExceptionCode(), GetExceptionInformation())) {			
		}
#endif		
	}
	return rv;
}

unsigned int machineproxy_numparameters(MachineProxy* self)
{
	unsigned int rv = 0;

	if (self->crashed == 0) {
#if defined DIVERSALIS__OS__MICROSOFT        
		__try
#endif		
		{
			rv = self->client->vtable->numparameters(self->client);
		}
#if defined DIVERSALIS__OS__MICROSOFT		
		__except(FilterException(self, "numparameters", GetExceptionCode(), GetExceptionInformation())) {			
		}
#endif		
	}
	return rv;
}

unsigned int machineproxy_numparametercols(MachineProxy* self)
{ 
	unsigned int rv = 0;

	if (self->crashed == 0) {
#if defined DIVERSALIS__OS__MICROSOFT        
		__try
#endif		
		{
			rv = self->client->vtable->numparametercols(self->client);
		}
#if defined DIVERSALIS__OS__MICROSOFT		
		__except(FilterException(self, "numparametercols", GetExceptionCode(), GetExceptionInformation())) {			
		}
#endif		
	}
	return rv;
}

int machineproxy_paramviewoptions(MachineProxy* self)
{
	int rv = 0;

	if (self->crashed == 0) {
#if defined DIVERSALIS__OS__MICROSOFT        
		__try
#endif		
		{
			rv = self->client->vtable->paramviewoptions(self->client);
		}
#if defined DIVERSALIS__OS__MICROSOFT		
		__except(FilterException(self, "paramviewoptions", GetExceptionCode(), GetExceptionInformation())) {
			rv = 0;
		}
#endif		
	}
	return rv;
}

void machineproxy_loadspecific(MachineProxy* self, SongFile* songfile, unsigned int slot)
{
	if (self->crashed == 0) {
#if defined DIVERSALIS__OS__MICROSOFT        
		__try
#endif		
		{
			self->client->vtable->loadspecific(self->client, songfile, slot);
		}
#if defined DIVERSALIS__OS__MICROSOFT		
		__except(FilterException(self,"loadspecific",  GetExceptionCode(), GetExceptionInformation())) {
		}
#endif		
	}
}

void machineproxy_savespecific(MachineProxy* self, SongFile* songfile, unsigned int slot)
{
	if (self->crashed == 0) {
#if defined DIVERSALIS__OS__MICROSOFT        
		__try
#endif		
		{
			self->client->vtable->savespecific(self->client, songfile, slot);
		}
#if defined DIVERSALIS__OS__MICROSOFT		
		__except(FilterException(self,"loadspecific",  GetExceptionCode(), GetExceptionInformation())) {
		}
#endif		
	}
}

unsigned int machineproxy_samplerate(MachineProxy* self)
{
	unsigned int rv = 0;

	if (self->crashed == 0) {
#if defined DIVERSALIS__OS__MICROSOFT        
		__try
#endif		
		{
			rv = self->client->vtable->samplerate(self->client);
		}
#if defined DIVERSALIS__OS__MICROSOFT		
		__except(FilterException(self, "samplerate", GetExceptionCode(), GetExceptionInformation())) {			
		}
#endif		
	}
	return rv;
}

unsigned int machineproxy_bpm(MachineProxy* self)
{
	unsigned int rv = 0;

	if (self->crashed == 0) {
#if defined DIVERSALIS__OS__MICROSOFT        
		__try
#endif		
		{
			rv = self->client->vtable->bpm(self->client);
		}
#if defined DIVERSALIS__OS__MICROSOFT		
		__except(FilterException(self, "bpm", GetExceptionCode(), GetExceptionInformation())) {			
		}
#endif		
	}
	return rv;
}

struct Samples* machineproxy_samples(MachineProxy* self)
{
	struct Samples* rv = 0;

	if (self->crashed == 0) {
#if defined DIVERSALIS__OS__MICROSOFT        
		__try
#endif		
		{
			rv = self->client->vtable->samples(self->client);
		}
#if defined DIVERSALIS__OS__MICROSOFT		
		__except(FilterException(self, "samples", GetExceptionCode(), GetExceptionInformation())) {			
		}
#endif		
	}
	return rv;
}

struct Machines* machineproxy_machines(MachineProxy* self)
{
	struct Machines* rv = 0;

	if (self->crashed == 0) {
#if defined DIVERSALIS__OS__MICROSOFT        
		__try
#endif		
		{ 
			rv = self->client->vtable->machines(self->client);
		}
#if defined DIVERSALIS__OS__MICROSOFT		
		__except(FilterException(self, "machines", GetExceptionCode(), GetExceptionInformation())) {			
		}
#endif		
	}
	return rv;
}

struct Instruments* machineproxy_instruments(MachineProxy* self)
{
	struct Instruments* rv = 0;

	if (self->crashed == 0) {
#if defined DIVERSALIS__OS__MICROSOFT        
		__try
#endif		
		{ 
			rv = self->client->vtable->instruments(self->client); 
		}
#if defined DIVERSALIS__OS__MICROSOFT		
		__except(FilterException(self, "instruments", GetExceptionCode(), GetExceptionInformation())) {			
		}
#endif		
	}
	return rv;
}

void machineproxy_setcallback(MachineProxy* self, MachineCallback callback)
{ 
	if (self->crashed == 0) {
#if defined DIVERSALIS__OS__MICROSOFT        
		__try
#endif		
		{ 
            
			self->client->vtable->setcallback(self->client, callback);
		}
#if defined DIVERSALIS__OS__MICROSOFT		
		__except(FilterException(self, "setcallback", GetExceptionCode(), GetExceptionInformation())) {
		}
#endif		
	}
}

uintptr_t machineproxy_slot(MachineProxy* self)
{
	uintptr_t rv = 0;

	if (self->crashed == 0) {
#if defined DIVERSALIS__OS__MICROSOFT        
		__try
#endif		
		{ 
			rv = self->client->vtable->slot(self->client);
		}
#if defined DIVERSALIS__OS__MICROSOFT		
		__except(FilterException(self, "slot", GetExceptionCode(), GetExceptionInformation())) {			
		}
#endif		
	}
	return rv;
}

void machineproxy_setslot(MachineProxy* self, uintptr_t slot)
{
	if (self->crashed == 0) {
#if defined DIVERSALIS__OS__MICROSOFT        
		__try
#endif		
		{ 
			self->client->vtable->setslot(self->client, slot);
		}
#if defined DIVERSALIS__OS__MICROSOFT		
		__except(FilterException(self, "setslot", GetExceptionCode(), GetExceptionInformation())) {
		}
#endif		
	}
}

int machineproxy_parametertype(MachineProxy* self, int param)
{
	int rv = MPF_STATE;
	
	if (self->crashed == 0) {
#if defined DIVERSALIS__OS__MICROSOFT        
		__try
#endif		
		{ 
			rv = self->client->vtable->parametertype(self->client, param);			
		}
#if defined DIVERSALIS__OS__MICROSOFT		
		__except(FilterException(self, "parametertype", GetExceptionCode(), GetExceptionInformation())) {			
		}
#endif		
	}
	return rv;
}

void machineproxy_parameterrange(MachineProxy* self, int numparam, int* minval, int* maxval)
{
	if (self->crashed == 0) {
#if defined DIVERSALIS__OS__MICROSOFT        
		__try
#endif		
		{ 
			self->client->vtable->parameterrange(self->client, numparam, minval, maxval);
		}
#if defined DIVERSALIS__OS__MICROSOFT		
		__except(FilterException(self, "parameterrange", GetExceptionCode(), GetExceptionInformation())) {
		}
#endif		
	}
}

int machineproxy_parameterlabel(MachineProxy* self, char* txt, int param)
{
	int rv = 0;

	txt[0] = '\0'; 
	if (self->crashed == 0) {
#if defined DIVERSALIS__OS__MICROSOFT        
		__try
#endif		
		{ 
			rv = self->client->vtable->parameterlabel(self->client, txt, param);			
		}
#if defined DIVERSALIS__OS__MICROSOFT		
		__except(FilterException(self, "parameterlabel", GetExceptionCode(), GetExceptionInformation())) {
			txt = '\0';
		}
#endif		
	}
	return rv;
}

int machineproxy_parametername(MachineProxy* self, char* txt, int param)
{
	int rv = 0;

	txt[0] = '\0'; 
	if (self->crashed == 0) {
#if defined DIVERSALIS__OS__MICROSOFT        
		__try
#endif		
		{ 
			rv = self->client->vtable->parametername(self->client, txt, param);			
		}
#if defined DIVERSALIS__OS__MICROSOFT		
		__except(FilterException(self, "parametername", GetExceptionCode(), GetExceptionInformation())) {
			txt = '\0';			
		}
#endif		
	}
	return rv;
}

int machineproxy_haseditor(MachineProxy* self)
{ 
	int rv = 0;
	
	if (self->crashed == 0) {
#if defined DIVERSALIS__OS__MICROSOFT        
		__try
#endif		
		{ 
			rv = self->client->vtable->haseditor(self->client);
		}
#if defined DIVERSALIS__OS__MICROSOFT		
		__except(FilterException(self, "haseditor", GetExceptionCode(), GetExceptionInformation())) {			
		}
#endif		
	}
	return rv;
}

void machineproxy_seteditorhandle(MachineProxy* self, void* handle)
{
	if (self->crashed == 0) {
#if defined DIVERSALIS__OS__MICROSOFT        
		__try
#endif		
		{ 
			self->client->vtable->seteditorhandle(self->client, handle);
		}
#if defined DIVERSALIS__OS__MICROSOFT		
		__except(FilterException(self, "seteditorhandle", GetExceptionCode(), GetExceptionInformation())) {			
		}
#endif		
	}	
}

void machineproxy_editorsize(MachineProxy* self, int* width, int* height)
{
	if (self->crashed == 0) {
#if defined DIVERSALIS__OS__MICROSOFT        
		__try
#endif		
		{ 
			self->client->vtable->editorsize(self->client, width, height);
		}
#if defined DIVERSALIS__OS__MICROSOFT		
		__except(FilterException(self, "editorsize", GetExceptionCode(), GetExceptionInformation())) {
			*width = 0;
			*height = 0;
		}
#endif		
	}	
}

static void machineproxy_editoridle(MachineProxy* self)
{
	if (self->crashed == 0) {
#if defined DIVERSALIS__OS__MICROSOFT        
		__try
#endif		
		{ 
			self->client->vtable->editoridle(self->client);
		}		
#if defined DIVERSALIS__OS__MICROSOFT		
		__except(FilterException(self, "editoridle", GetExceptionCode(), GetExceptionInformation())) {
		}
#endif		
	}
}

const char* machineproxy_editname(MachineProxy* self)
{
	const char* rv = 0;

	if (self->crashed == 0) {
#if defined DIVERSALIS__OS__MICROSOFT        
		__try
#endif		
		{
			rv = self->client->vtable->editname(self->client);
		}
#if defined DIVERSALIS__OS__MICROSOFT		
		__except (FilterException(self, "editname", GetExceptionCode(), GetExceptionInformation())) {
		}
#endif		
	}
	return rv;
}

void machineproxy_seteditname(MachineProxy* self, const char* name)
{
	if (self->crashed == 0) {
#if defined DIVERSALIS__OS__MICROSOFT        
		__try
#endif		
		{
			self->client->vtable->seteditname(self->client, name);
		}
#if defined DIVERSALIS__OS__MICROSOFT		
		__except (FilterException(self, "seteditname", GetExceptionCode(), GetExceptionInformation())) {
		}
#endif		
	}
}

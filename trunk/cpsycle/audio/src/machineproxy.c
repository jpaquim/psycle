// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

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
#include "../../detail/portable.h"
#include "plugin_interface.h"

struct psy_audio_Preset;

// proxy
static psy_audio_Buffer* machineproxy_mix(psy_audio_MachineProxy*,
	uintptr_t slot, uintptr_t amount, psy_audio_MachineSockets*,
	psy_audio_Machines*, struct psy_audio_Player*);
static void machineproxy_work(psy_audio_MachineProxy*, psy_audio_BufferContext*);
static void machineproxy_generateaudio(psy_audio_MachineProxy*, psy_audio_BufferContext*);
static void machineproxy_seqtick(psy_audio_MachineProxy*, uintptr_t channel,
	const psy_audio_PatternEvent*);
static void machineproxy_sequencertick(psy_audio_MachineProxy*);
static void machineproxy_newline(psy_audio_MachineProxy*);
static psy_List* machineproxy_sequencerinsert(psy_audio_MachineProxy*, psy_List* events);
static void machineproxy_stop(psy_audio_MachineProxy*);
static void machineproxy_dispose(psy_audio_MachineProxy*);
static void machineproxy_reload(psy_audio_MachineProxy*);
static psy_audio_Machine* machineproxy_clone(psy_audio_MachineProxy*);
static int machineproxy_mode(psy_audio_MachineProxy*);
static const char* machineproxy_modulepath(psy_audio_MachineProxy*);
static uintptr_t machineproxy_shellidx(psy_audio_MachineProxy*);
static uintptr_t machineproxy_numinputs(psy_audio_MachineProxy*);
static uintptr_t machineproxy_numoutputs(psy_audio_MachineProxy*);
// parameter
static psy_audio_MachineParam* machineproxy_parameter(psy_audio_MachineProxy*,
	uintptr_t param);
static psy_audio_MachineParam* machineproxy_tweakparameter(psy_audio_MachineProxy*,
	uintptr_t param);
static void machineproxy_setpanning(psy_audio_MachineProxy*, psy_dsp_amp_t);
static psy_dsp_amp_t machineproxy_panning(psy_audio_MachineProxy*);
static void machineproxy_mute(psy_audio_MachineProxy*);
static void machineproxy_unmute(psy_audio_MachineProxy*);
static int machineproxy_muted(psy_audio_MachineProxy*);
static void machineproxy_bypass(psy_audio_MachineProxy*);
static void machineproxy_unbypass(psy_audio_MachineProxy*);
static int machineproxy_bypassed(psy_audio_MachineProxy*);
static const psy_audio_MachineInfo* machineproxy_info(psy_audio_MachineProxy*);
static uintptr_t machineproxy_numparameters(psy_audio_MachineProxy*);
static uintptr_t machineproxy_numtweakparameters(psy_audio_MachineProxy*);
static uintptr_t machineproxy_numparametercols(psy_audio_MachineProxy*);
static void machineproxy_loadspecific(psy_audio_MachineProxy*, psy_audio_SongFile*,
	uintptr_t slot);
static void machineproxy_loadwiremapping(psy_audio_MachineProxy*, psy_audio_SongFile*,
	uintptr_t slot);
static void machineproxy_savespecific(psy_audio_MachineProxy*, psy_audio_SongFile*,
	uintptr_t slot);
static void machineproxy_savewiremapping(psy_audio_MachineProxy*, psy_audio_SongFile*,
	uintptr_t slot);
static void machineproxy_postload(psy_audio_MachineProxy*, psy_audio_SongFile*,
	uintptr_t slot);
static uintptr_t machineproxy_samplerate(psy_audio_MachineProxy*);
static psy_dsp_beat_t machineproxy_bpm(psy_audio_MachineProxy*);
static psy_dsp_beat_t machineproxy_beatspertick(psy_audio_MachineProxy*);
static psy_dsp_beat_t machineproxy_beatspersample(psy_audio_MachineProxy*);
static psy_dsp_beat_t machineproxy_currbeatsperline(psy_audio_MachineProxy*);
static uintptr_t machineproxy_slot(psy_audio_MachineProxy*);
static void machineproxy_setslot(psy_audio_MachineProxy*, uintptr_t slot);
static struct psy_audio_Samples* machineproxy_samples(psy_audio_MachineProxy*);
static struct psy_audio_Machines* machineproxy_machines(psy_audio_MachineProxy*);
static struct psy_audio_Instruments* machineproxy_instruments(psy_audio_MachineProxy*);
static struct psy_audio_MachineFactory* machineproxy_machinefactory(psy_audio_MachineProxy*);
static void machineproxy_output(psy_audio_MachineProxy*, const char* text);
static bool machineproxy_addcapture(psy_audio_MachineProxy*, int index);
static bool machineproxy_removecapture(psy_audio_MachineProxy*, int index);
static void machineproxy_readbuffers(psy_audio_MachineProxy*, int index, float** pleft, float** pright, int numsamples);
static const char* machineproxy_capturename(psy_audio_MachineProxy*, int index);
static int machineproxy_numcaptures(psy_audio_MachineProxy*);
static const char* machineproxy_playbackname(psy_audio_MachineProxy*, int index);
static int machineproxy_numplaybacks(psy_audio_MachineProxy*);

static void machineproxy_setcallback(psy_audio_MachineProxy*, psy_audio_MachineCallback*);
static int machineproxy_haseditor(psy_audio_MachineProxy*);
static void machineproxy_seteditorhandle(psy_audio_MachineProxy*, void* handle);
static void machineproxy_editorsize(psy_audio_MachineProxy*, int* width, int* height);
static void machineproxy_editoridle(psy_audio_MachineProxy*);
static void machineproxy_setposition(psy_audio_MachineProxy*, intptr_t x, intptr_t y);
static void machineproxy_position(psy_audio_MachineProxy*, intptr_t* x, intptr_t* y);
static const char* machineproxy_editname(psy_audio_MachineProxy*);
static void machineproxy_seteditname(psy_audio_MachineProxy*, const char* name);
static psy_audio_Buffer* machineproxy_buffermemory(psy_audio_MachineProxy*);
static uintptr_t machineproxy_buffermemorysize(psy_audio_MachineProxy*);
static void machineproxy_setbuffermemorysize(psy_audio_MachineProxy*, uintptr_t);
static psy_dsp_amp_range_t machineproxy_amprange(psy_audio_MachineProxy*);
// MachineParameter calls
static void machineproxy_param_tweak(psy_audio_MachineProxy* self, psy_audio_MachineParam* param, float val);
static void machineproxy_param_reset(psy_audio_MachineProxy* self, psy_audio_MachineParam* param);
static float machineproxy_param_normvalue(psy_audio_MachineProxy* self, psy_audio_MachineParam* param);
static void machineproxy_param_range(psy_audio_MachineProxy* self, psy_audio_MachineParam* param, intptr_t* minval, intptr_t* maxval);
static int machineproxy_param_type(psy_audio_MachineProxy* self, psy_audio_MachineParam* param);
static int machineproxy_param_label(psy_audio_MachineProxy* self, psy_audio_MachineParam* param, char* text);
static int machineproxy_param_name(psy_audio_MachineProxy* self, psy_audio_MachineParam* param, char* text);
static int machineproxy_param_describe(psy_audio_MachineProxy* self, psy_audio_MachineParam* param, char* text);
// data
static void machineproxy_putdata(psy_audio_MachineProxy*, uint8_t* data);
static uint8_t* machineproxy_data(psy_audio_MachineProxy*);
static uintptr_t machineproxy_datasize(psy_audio_MachineProxy*);
// programs
static void machineproxy_programname(psy_audio_MachineProxy*, int bnkidx, int prgIdx, char* val);
static int machineproxy_numprograms(psy_audio_MachineProxy*);
static void machineproxy_setcurrprogram(psy_audio_MachineProxy*, int prgIdx);
static int machineproxy_currprogram(psy_audio_MachineProxy*);
static void machineproxy_bankname(psy_audio_MachineProxy*, int bnkidx, char* val);
static int machineproxy_numbanks(psy_audio_MachineProxy*);
static void machineproxy_setcurrbank(psy_audio_MachineProxy*, int prgIdx);
static int machineproxy_currbank(psy_audio_MachineProxy*);
static void machineproxy_currentpreset(psy_audio_MachineProxy*, struct psy_audio_Preset*);
static void machineproxy_setpresets(psy_audio_MachineProxy*, struct psy_audio_Presets*);
static struct psy_audio_Presets* machineproxy_presets(psy_audio_MachineProxy*);
static bool machineproxy_acceptpresets(psy_audio_MachineProxy*);
static void machineproxy_command(psy_audio_MachineProxy*);

#if defined DIVERSALIS__OS__MICROSOFT
static int FilterException(psy_audio_MachineProxy* proxy, const char* msg, int code,
	struct _EXCEPTION_POINTERS *ep) 
{	
	char text[512];
	proxy->crashed = 1;	
		
	if (psy_audio_machine_info(proxy->client)) {
		psy_snprintf(text, 512, "%u: %s crashed \n\r %s",
			(unsigned int)proxy->client->vtable->slot(proxy->client),
			proxy->client->vtable->info(proxy->client)->ShortName, msg);
	} else {
		psy_snprintf(text, 512, "Machine crashed");
	}
	psy_audio_machine_output(psy_audio_machineproxy_base(proxy), text);
	// MessageBox(0, txt, "Psycle Host Exception", MB_OK | MB_ICONERROR);
	return EXCEPTION_EXECUTE_HANDLER;
}
#endif

static MachineVtable vtable;
static int vtable_initialized = 0;

static void vtable_init(psy_audio_MachineProxy* self)
{
	if (!vtable_initialized) {
		vtable = *self->machine.vtable;
		vtable.mix = (fp_machine_mix) machineproxy_mix;
		vtable.work = (fp_machine_work) machineproxy_work;
		vtable.generateaudio = (fp_machine_generateaudio)
			machineproxy_generateaudio;
		vtable.seqtick = (fp_machine_seqtick) machineproxy_seqtick;
		vtable.sequencertick = (fp_machine_sequencertick)
			machineproxy_sequencertick;
		vtable.newline = (fp_machine_newline)
			machineproxy_newline;
		vtable.sequencerinsert = (fp_machine_sequencerinsert)
			machineproxy_sequencerinsert;
		vtable.stop = (fp_machine_stop) machineproxy_stop;
		vtable.dispose = (fp_machine_dispose) machineproxy_dispose;
		vtable.reload = (fp_machine_reload) machineproxy_reload;
		vtable.clone = (fp_machine_clone)machineproxy_clone;
		vtable.mode = (fp_machine_mode) machineproxy_mode;
		vtable.modulepath = (fp_machine_modulepath)machineproxy_modulepath;
		vtable.shellidx = (fp_machine_shellidx)machineproxy_shellidx;
		vtable.numinputs = (fp_machine_numinputs) machineproxy_numinputs;
		vtable.numoutputs = (fp_machine_numoutputs) machineproxy_numoutputs;
		vtable.setpanning = (fp_machine_setpanning) machineproxy_setpanning;
		vtable.panning = (fp_machine_panning) machineproxy_panning;
		vtable.mute = (fp_machine_mute) machineproxy_mute;
		vtable.unmute = (fp_machine_unmute) machineproxy_unmute;
		vtable.muted = (fp_machine_muted) machineproxy_muted;
		vtable.bypass = (fp_machine_bypass) machineproxy_bypass;
		vtable.unbypass = (fp_machine_unbypass) machineproxy_unbypass;
		vtable.bypassed = (fp_machine_bypassed) machineproxy_bypassed;
		vtable.info = (fp_machine_info) machineproxy_info;
		vtable.parameter = (fp_machine_parameter)
			machineproxy_parameter;
		vtable.tweakparameter = (fp_machine_tweakparameter)
			machineproxy_tweakparameter;
		vtable.numparameters = (fp_machine_numparameters)
			machineproxy_numparameters;
		vtable.numtweakparameters = (fp_machine_numtweakparameters)
			machineproxy_numtweakparameters;
		vtable.numparametercols = (fp_machine_numparametercols)
			machineproxy_numparametercols;		
		vtable.loadspecific = (fp_machine_loadspecific)
			machineproxy_loadspecific;
		vtable.loadwiremapping = (fp_machine_loadwiremapping)
			machineproxy_loadwiremapping;
		vtable.savespecific = (fp_machine_savespecific)
			machineproxy_savespecific;
		vtable.savewiremapping = (fp_machine_savewiremapping)
			machineproxy_savewiremapping;
		vtable.postload = (fp_machine_postload)
			machineproxy_postload;
		vtable.samplerate = (fp_machine_samplerate)
			machineproxy_samplerate;
		vtable.bpm = (fp_machine_bpm) machineproxy_bpm;
		vtable.beatspertick = (fp_machine_beatspertick) machineproxy_beatspertick;
		vtable.beatspersample= (fp_machine_beatspersample) machineproxy_beatspersample;
		vtable.currbeatsperline = (fp_machine_currbeatsperline)machineproxy_currbeatsperline;
		vtable.machines = (fp_machine_machines)
			machineproxy_machines;
		vtable.instruments = (fp_machine_instruments)
			machineproxy_instruments;
		vtable.machinefactory = (fp_machine_machinefactory)
			machineproxy_machinefactory;
		vtable.output = (fp_machine_output)
			machineproxy_output;
		vtable.addcapture = (fp_machine_addcapture) machineproxy_addcapture;
		vtable.removecapture = (fp_machine_removecapture) machineproxy_removecapture;
		vtable.readbuffers = (fp_machine_readbuffers) machineproxy_readbuffers;
		vtable.capturename = (fp_machine_capturename) machineproxy_capturename;
		vtable.numcaptures = (fp_machine_numcaptures) machineproxy_numcaptures;
		vtable.playbackname = (fp_machine_playbackname) machineproxy_playbackname;
		vtable.numplaybacks = (fp_machine_numplaybacks) machineproxy_numplaybacks;
		vtable.samples = (fp_machine_samples)
			machineproxy_samples;
		vtable.setcallback = (fp_machine_setcallback)
			machineproxy_setcallback;
		vtable.setslot = (fp_machine_setslot) machineproxy_setslot;
		vtable.slot = (fp_machine_slot) machineproxy_slot;
		vtable.haseditor = (fp_machine_haseditor) machineproxy_haseditor;
		vtable.seteditorhandle = (fp_machine_seteditorhandle)
			machineproxy_seteditorhandle;
		vtable.editorsize = (fp_machine_editorsize) machineproxy_editorsize;
		vtable.editoridle = (fp_machine_editoridle) machineproxy_editoridle;
		vtable.setposition = (fp_machine_setposition)machineproxy_setposition;
		vtable.position = (fp_machine_position)machineproxy_position;
		vtable.seteditname = (fp_machine_seteditname) machineproxy_seteditname;
		vtable.editname = (fp_machine_editname) machineproxy_editname;
		vtable.buffermemory = (fp_machine_buffermemory) machineproxy_buffermemory;
		vtable.buffermemorysize = (fp_machine_buffermemorysize)
			machineproxy_buffermemorysize;
		vtable.setbuffermemorysize = (fp_machine_setbuffermemorysize)
			machineproxy_setbuffermemorysize;
		vtable.amprange = (fp_machine_amprange) machineproxy_amprange;
		vtable.putdata = (fp_machine_putdata) machineproxy_putdata;
		vtable.data = (fp_machine_data) machineproxy_data;
		vtable.datasize = (fp_machine_datasize) machineproxy_datasize;
		vtable.programname = (fp_machine_programname) machineproxy_programname;
		vtable.numprograms = (fp_machine_numprograms) machineproxy_numprograms;
		vtable.setcurrprogram = (fp_machine_setcurrprogram) machineproxy_setcurrprogram;
		vtable.currprogram = (fp_machine_currprogram) machineproxy_currprogram;
		vtable.bankname = (fp_machine_bankname) machineproxy_bankname;
		vtable.numbanks = (fp_machine_numbanks) machineproxy_numbanks;
		vtable.setcurrbank = (fp_machine_setcurrbank) machineproxy_setcurrbank;
		vtable.currbank = (fp_machine_currbank) machineproxy_currbank;
		vtable.currentpreset = (fp_machine_currentpreset)machineproxy_currentpreset;
		vtable.setpresets = (fp_machine_setpresets)machineproxy_setpresets;
		vtable.presets = (fp_machine_presets)machineproxy_presets;
		vtable.acceptpresets = (fp_machine_acceptpresets)machineproxy_acceptpresets;
		vtable.command = (fp_machine_command)machineproxy_command;
		vtable.parameter_tweak = (fp_machine_param_tweak)machineproxy_param_tweak;
		vtable.parameter_reset = (fp_machine_param_reset)machineproxy_param_reset;
		vtable.parameter_normvalue = (fp_machine_param_normvalue)machineproxy_param_normvalue;
		vtable.parameter_range = (fp_machine_param_range)machineproxy_param_range;
		vtable.parameter_type = (fp_machine_param_type)machineproxy_param_type;
		vtable.parameter_label = (fp_machine_param_label)machineproxy_param_label;
		vtable.parameter_name = (fp_machine_param_name)machineproxy_param_name;
		vtable.parameter_describe = (fp_machine_param_describe)machineproxy_param_describe;
		vtable_initialized = 1;
	}
}

void psy_audio_machineproxy_init(psy_audio_MachineProxy* self, psy_audio_Machine* client)
{
	psy_audio_machine_init(&self->machine, client->callback);
	vtable_init(self);
	self->machinedispose = self->machine.vtable->dispose;
	self->machine.vtable = &vtable;
	self->crashed = 0;
	self->client = client;	
}

psy_audio_MachineProxy* psy_audio_machineproxy_alloc(void)
{
	return (psy_audio_MachineProxy*)malloc(sizeof(psy_audio_MachineProxy));
}

psy_audio_MachineProxy* psy_audio_machineproxy_allocinit(psy_audio_Machine* client)
{
	psy_audio_MachineProxy* rv;

	rv = psy_audio_machineproxy_alloc();
	if (rv) {
		psy_audio_machineproxy_init(rv, client);
	}
	return rv;
}

psy_audio_Buffer* machineproxy_mix(psy_audio_MachineProxy* self, uintptr_t slot,
	uintptr_t amount, psy_audio_MachineSockets* sockets,
	psy_audio_Machines* machines,
	struct psy_audio_Player* player)
{
	psy_audio_Buffer* rv = 0;

	if (self->crashed == 0) {
#if defined DIVERSALIS__OS__MICROSOFT
		__try 
#endif		
		{
			rv = self->client->vtable->mix(self->client, slot, amount, sockets,
				machines, player);
		} 
#if defined DIVERSALIS__OS__MICROSOFT
		__except(FilterException(self, "mix", GetExceptionCode(),
			GetExceptionInformation())) {			
		}
#endif		
	}
	return rv;
}

void machineproxy_work(psy_audio_MachineProxy* self, psy_audio_BufferContext* bc)
{
	if (self->crashed == 0) {
#if defined DIVERSALIS__OS__MICROSOFT        
		__try
#endif				
		{
			psy_audio_machine_work(self->client, bc);			
		}
#if defined DIVERSALIS__OS__MICROSOFT		
		__except(FilterException(self, "work", GetExceptionCode(),
			GetExceptionInformation())) {
		}
#endif		
	}
}

void machineproxy_generateaudio(psy_audio_MachineProxy* self, psy_audio_BufferContext* bc)
{
	if (self->crashed == 0) {
#if defined DIVERSALIS__OS__MICROSOFT        
		__try
#endif		
		{
			psy_audio_machine_generateaudio(self->client, bc);
			if (self->client->err) {
				self->crashed = 1;
#if defined DIVERSALIS__OS__MICROSOFT                 
				FilterException(self, "generateaudio", self->client->err, 0);
#endif                
			}
		}
#if defined DIVERSALIS__OS__MICROSOFT		
		__except(FilterException(self, "generateaudio", GetExceptionCode(),
			GetExceptionInformation())) {
		}
#endif		
	}
}

void machineproxy_seqtick(psy_audio_MachineProxy* self, uintptr_t channel,
	const psy_audio_PatternEvent* event)
{
	if (self->crashed == 0) {
#if defined DIVERSALIS__OS__MICROSOFT        
		__try
#endif		
		{
			psy_audio_machine_seqtick(self->client, channel, event);
		}
#if defined DIVERSALIS__OS__MICROSOFT		
		__except(FilterException(self, "seqtick", GetExceptionCode(),
			GetExceptionInformation())) {
		}
#endif		
	}

}

void machineproxy_sequencertick(psy_audio_MachineProxy* self)
{
	if (self->crashed == 0) {
#if defined DIVERSALIS__OS__MICROSOFT        
		__try
#endif		
		{
			psy_audio_machine_sequencertick(self->client);
		}
#if defined DIVERSALIS__OS__MICROSOFT		
		__except(FilterException(self, "sequencertick", GetExceptionCode(),
			GetExceptionInformation())) {
		}
#endif		
	}

}

void machineproxy_newline(psy_audio_MachineProxy* self)
{
	if (self->crashed == 0) {
#if defined DIVERSALIS__OS__MICROSOFT        
		__try
#endif		
		{
			psy_audio_machine_newline(self->client);
		}
#if defined DIVERSALIS__OS__MICROSOFT		
		__except(FilterException(self, "newline", GetExceptionCode(),
			GetExceptionInformation())) {
		}
#endif		
	}

}

void machineproxy_stop(psy_audio_MachineProxy* self)
{
	if (self->crashed == 0) {
#if defined DIVERSALIS__OS__MICROSOFT        
		__try
#endif		
		{
			psy_audio_machine_stop(self->client);
		}
#if defined DIVERSALIS__OS__MICROSOFT		
		__except(FilterException(self, "stop", GetExceptionCode(),
			GetExceptionInformation())) {
		}
#endif		
	}

}

void machineproxy_dispose(psy_audio_MachineProxy* self)
{
	if (self->crashed == 0) {
#if defined DIVERSALIS__OS__MICROSOFT        
		__try
#endif		
		{
			psy_audio_machine_dispose(self->client);
			free(self->client);
			self->machinedispose(&self->machine);
			
		}
#if defined DIVERSALIS__OS__MICROSOFT		
		__except(FilterException(self, "dispose", GetExceptionCode(),
			GetExceptionInformation())) {
		}
#endif		
	}
}

void machineproxy_reload(psy_audio_MachineProxy* self)
{
	if (self->crashed == 0) {
#if defined DIVERSALIS__OS__MICROSOFT        
		__try
#endif		
		{
			psy_audio_machine_reload(self->client);			

		}
#if defined DIVERSALIS__OS__MICROSOFT		
		__except (FilterException(self, "reload", GetExceptionCode(),
			GetExceptionInformation())) {
		}
#endif		
	}
}

int machineproxy_mode(psy_audio_MachineProxy* self)
{
	int rv = 0;

	if (self->crashed == 0) {
#if defined DIVERSALIS__OS__MICROSOFT        
		__try
#endif		
		{
			rv = psy_audio_machine_mode(self->client);
		}
#if defined DIVERSALIS__OS__MICROSOFT		
		__except(FilterException(self, "mode", GetExceptionCode(),
			GetExceptionInformation())) {			
		}
#endif		
	}
	return rv;
}

const char* machineproxy_modulepath(psy_audio_MachineProxy* self)
{
	const char* rv = 0;

	if (self->crashed == 0) {
#if defined DIVERSALIS__OS__MICROSOFT        
		__try
#endif		
		{
			rv = psy_audio_machine_modulepath(self->client);
		}
#if defined DIVERSALIS__OS__MICROSOFT		
		__except (FilterException(self, "modulepath", GetExceptionCode(),
			GetExceptionInformation())) {
		}
#endif		
	}
	return rv;
}

uintptr_t machineproxy_shellidx(psy_audio_MachineProxy* self)
{
	uintptr_t rv = 0;

	if (self->crashed == 0) {
#if defined DIVERSALIS__OS__MICROSOFT        
		__try
#endif		
		{
			rv = psy_audio_machine_shellidx(self->client);
		}
#if defined DIVERSALIS__OS__MICROSOFT		
		__except (FilterException(self, "shellidx", GetExceptionCode(),
			GetExceptionInformation())) {
		}
#endif		
	}
	return rv;
}

uintptr_t machineproxy_numinputs(psy_audio_MachineProxy* self)
{
	uintptr_t rv = 0;

	if (self->crashed == 0) {
#if defined DIVERSALIS__OS__MICROSOFT        
		__try
#endif		
		{
			rv = psy_audio_machine_numinputs(self->client);
		}
#if defined DIVERSALIS__OS__MICROSOFT		
		__except(FilterException(self, "numinputs", GetExceptionCode(),
			GetExceptionInformation())) {			
		}
#endif		
	}
	return rv;
}

uintptr_t machineproxy_numoutputs(psy_audio_MachineProxy* self)
{ 
	uintptr_t rv = 0;

	if (self->crashed == 0) {
#if defined DIVERSALIS__OS__MICROSOFT        
		__try
#endif		
		{
			rv = psy_audio_machine_numoutputs(self->client);
		}
#if defined DIVERSALIS__OS__MICROSOFT		
		__except(FilterException(self, "numoutputs",  GetExceptionCode(),
			GetExceptionInformation())) {			
		}
#endif		
	}
	return rv;
}

psy_audio_MachineParam* machineproxy_parameter(psy_audio_MachineProxy* self, uintptr_t param)
{
	psy_audio_MachineParam* rv = 0;
	if (self->crashed == 0) {
#if defined DIVERSALIS__OS__MICROSOFT        
		__try
#endif		
		{
			rv = psy_audio_machine_parameter(self->client, param);
		}
#if defined DIVERSALIS__OS__MICROSOFT		
		__except (FilterException(self, "parameter", GetExceptionCode(),
			GetExceptionInformation())) {
		}
#endif		
	}
	return rv;
}

psy_audio_MachineParam* machineproxy_tweakparameter(psy_audio_MachineProxy* self, uintptr_t param)
{
	psy_audio_MachineParam* rv = 0;
	if (self->crashed == 0) {
#if defined DIVERSALIS__OS__MICROSOFT        
		__try
#endif		
		{
			rv = psy_audio_machine_tweakparameter(self->client, param);
		}
#if defined DIVERSALIS__OS__MICROSOFT		
		__except (FilterException(self, "tweakparameter", GetExceptionCode(),
			GetExceptionInformation())) {
		}
#endif		
	}
	return rv;
}

void machineproxy_setpanning(psy_audio_MachineProxy* self, psy_dsp_amp_t panning)
{	
	if (self->crashed == 0) {
#if defined DIVERSALIS__OS__MICROSOFT        
		__try
#endif		
		{
			psy_audio_machine_setpanning(self->client, panning);
		}
#if defined DIVERSALIS__OS__MICROSOFT		
		__except(FilterException(self, "setpanning", GetExceptionCode(),
			GetExceptionInformation())) {			
		}
#endif		
	}	
}

psy_dsp_amp_t machineproxy_panning(psy_audio_MachineProxy* self)
{
	psy_dsp_amp_t rv = 0;

	if (self->crashed == 0) {
#if defined DIVERSALIS__OS__MICROSOFT        
		__try
#endif		
		{
			rv = self->client->vtable->panning(self->client);
		}
#if defined DIVERSALIS__OS__MICROSOFT		
		__except(FilterException(self, "panning", GetExceptionCode(),
			GetExceptionInformation())) {			
		}
#endif		
	}
	return rv;
}

void machineproxy_mute(psy_audio_MachineProxy* self)
{
	if (self->crashed == 0) {
#if defined DIVERSALIS__OS__MICROSOFT        
		__try
#endif		
		{
			psy_audio_machine_mute(self->client);
		}
#if defined DIVERSALIS__OS__MICROSOFT		
		__except (FilterException(self, "mute", GetExceptionCode(),
			GetExceptionInformation())) {
		}
#endif		
	}
}

void machineproxy_unmute(psy_audio_MachineProxy* self)
{
	if (self->crashed == 0) {
#if defined DIVERSALIS__OS__MICROSOFT        
		__try
#endif		
		{
			self->client->vtable->unmute(self->client);
		}
#if defined DIVERSALIS__OS__MICROSOFT		
		__except (FilterException(self, "unmute", GetExceptionCode(),
			GetExceptionInformation())) {
		}
#endif		
	}
}

int machineproxy_muted(psy_audio_MachineProxy* self)
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
		__except (FilterException(self, "muted", GetExceptionCode(),
			GetExceptionInformation())) {
		}
#endif		
	}
	return rv;
}

void machineproxy_bypass(psy_audio_MachineProxy* self)
{
	if (self->crashed == 0) {
#if defined DIVERSALIS__OS__MICROSOFT        
		__try
#endif		
		{
			psy_audio_machine_bypass(self->client);
		}
#if defined DIVERSALIS__OS__MICROSOFT		
		__except (FilterException(self, "bypass", GetExceptionCode(),
			GetExceptionInformation())) {
		}
#endif		
	}
}

void machineproxy_unbypass(psy_audio_MachineProxy* self)
{
	if (self->crashed == 0) {
#if defined DIVERSALIS__OS__MICROSOFT        
		__try
#endif		
		{
			self->client->vtable->unbypass(self->client);
		}
#if defined DIVERSALIS__OS__MICROSOFT		
		__except (FilterException(self, "unbypass", GetExceptionCode(),
			GetExceptionInformation())) {
		}
#endif		
	}
}

int machineproxy_bypassed(psy_audio_MachineProxy* self)
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
		__except (FilterException(self, "panning", GetExceptionCode(),
			GetExceptionInformation())) {
		}
#endif		
	}
	return rv;
}

const psy_audio_MachineInfo* machineproxy_info(psy_audio_MachineProxy* self)
{ 
	const psy_audio_MachineInfo* rv = 0;

	if (self->crashed == 0) {
#if defined DIVERSALIS__OS__MICROSOFT        
		__try
#endif		
		{
			rv = psy_audio_machine_info(self->client);
		}
#if defined DIVERSALIS__OS__MICROSOFT		
		__except(FilterException(self, "info", GetExceptionCode(),
			GetExceptionInformation())) {			
		}
#endif		
	}
	return rv;
}

uintptr_t machineproxy_numparameters(psy_audio_MachineProxy* self)
{
	uintptr_t rv = 0;

	if (self->crashed == 0) {
#if defined DIVERSALIS__OS__MICROSOFT        
		__try
#endif		
		{
			rv = psy_audio_machine_numparameters(self->client);
		}
#if defined DIVERSALIS__OS__MICROSOFT		
		__except(FilterException(self, "numparameters", GetExceptionCode(),
			GetExceptionInformation())) {			
		}
#endif		
	}
	return rv;
}

uintptr_t machineproxy_numtweakparameters(psy_audio_MachineProxy* self)
{
	uintptr_t rv = 0;

	if (self->crashed == 0) {
#if defined DIVERSALIS__OS__MICROSOFT        
		__try
#endif		
		{
			rv = psy_audio_machine_numtweakparameters(self->client);
		}
#if defined DIVERSALIS__OS__MICROSOFT		
		__except (FilterException(self, "numtweakparameters", GetExceptionCode(),
			GetExceptionInformation())) {
		}
#endif		
	}
	return rv;
}

uintptr_t machineproxy_numparametercols(psy_audio_MachineProxy* self)
{ 
	uintptr_t rv = 0;

	if (self->crashed == 0) {
#if defined DIVERSALIS__OS__MICROSOFT        
		__try
#endif		
		{
			rv = psy_audio_machine_numparametercols(self->client);
		}
#if defined DIVERSALIS__OS__MICROSOFT		
		__except(FilterException(self, "numparametercols", GetExceptionCode(),
			GetExceptionInformation())) {			
		}
#endif		
	}
	return rv;
}

void machineproxy_loadspecific(psy_audio_MachineProxy* self,
	psy_audio_SongFile* songfile, uintptr_t slot)
{
	if (self->crashed == 0) {
#if defined DIVERSALIS__OS__MICROSOFT        
		__try
#endif		
		{
			psy_audio_machine_loadspecific(self->client, songfile, slot);
		}
#if defined DIVERSALIS__OS__MICROSOFT		
		__except(FilterException(self,"loadspecific",  GetExceptionCode(),
			GetExceptionInformation())) {
		}
#endif		
	}
}

void machineproxy_loadwiremapping(psy_audio_MachineProxy* self,
	psy_audio_SongFile* songfile, uintptr_t slot)
{
	if (self->crashed == 0) {
#if defined DIVERSALIS__OS__MICROSOFT        
		__try
#endif		
		{
			psy_audio_machine_loadwiremapping(self->client, songfile, slot);
		}
#if defined DIVERSALIS__OS__MICROSOFT		
		__except (FilterException(self, "loadwiremapping", GetExceptionCode(),
			GetExceptionInformation())) {
		}
#endif		
	}
}

void machineproxy_savespecific(psy_audio_MachineProxy* self,
	psy_audio_SongFile* songfile, uintptr_t slot)
{
	if (self->crashed == 0) {
#if defined DIVERSALIS__OS__MICROSOFT        
		__try
#endif		
		{
			psy_audio_machine_savespecific(self->client, songfile, slot);
		}
#if defined DIVERSALIS__OS__MICROSOFT		
		__except(FilterException(self,"loadspecific",  GetExceptionCode(),
			GetExceptionInformation())) {
		}
#endif		
	}
}

void machineproxy_savewiremapping(psy_audio_MachineProxy* self,
	psy_audio_SongFile* songfile, uintptr_t slot)
{
	if (self->crashed == 0) {
#if defined DIVERSALIS__OS__MICROSOFT        
		__try
#endif		
		{
			psy_audio_machine_savewiremapping(self->client, songfile, slot);
		}
#if defined DIVERSALIS__OS__MICROSOFT		
		__except (FilterException(self, "savewiremapping", GetExceptionCode(),
			GetExceptionInformation())) {
		}
#endif		
	}
}

void machineproxy_postload(psy_audio_MachineProxy* self,
	psy_audio_SongFile* songfile, uintptr_t slot)
{
	if (self->crashed == 0) {
#if defined DIVERSALIS__OS__MICROSOFT        
		__try
#endif		
		{
			psy_audio_machine_postload(self->client, songfile, slot);
		}
#if defined DIVERSALIS__OS__MICROSOFT		
		__except (FilterException(self, "loadspecific", GetExceptionCode(),
			GetExceptionInformation())) {
		}
#endif		
	}
}

uintptr_t machineproxy_samplerate(psy_audio_MachineProxy* self)
{
	uintptr_t rv = 0;

	if (self->crashed == 0) {
#if defined DIVERSALIS__OS__MICROSOFT        
		__try
#endif		
		{
			rv = psy_audio_machine_samplerate(self->client);
		}
#if defined DIVERSALIS__OS__MICROSOFT		
		__except(FilterException(self, "samplerate", GetExceptionCode(),
			GetExceptionInformation())) {			
		}
#endif		
	}
	return rv;
}

psy_dsp_beat_t machineproxy_bpm(psy_audio_MachineProxy* self)
{
	psy_dsp_beat_t rv = 0;

	if (self->crashed == 0) {
#if defined DIVERSALIS__OS__MICROSOFT        
		__try
#endif		
		{
			rv = psy_audio_machine_bpm(self->client);
		}
#if defined DIVERSALIS__OS__MICROSOFT		
		__except(FilterException(self, "bpm", GetExceptionCode(),
			GetExceptionInformation())) {
		}
#endif		
	}
	return rv;
}

psy_dsp_beat_t machineproxy_beatspertick(psy_audio_MachineProxy* self)
{
	psy_dsp_beat_t rv = 0;

	if (self->crashed == 0) {
#if defined DIVERSALIS__OS__MICROSOFT        
		__try
#endif		
		{
			rv = psy_audio_machine_beatspertick(self->client);
		}
#if defined DIVERSALIS__OS__MICROSOFT		
		__except (FilterException(self, "beatspertick", GetExceptionCode(),
			GetExceptionInformation())) {
		}
#endif		
	}
	return rv;
}

psy_dsp_beat_t machineproxy_beatspersample(psy_audio_MachineProxy* self)
{
	psy_dsp_beat_t rv = 0;

	if (self->crashed == 0) {
#if defined DIVERSALIS__OS__MICROSOFT        
		__try
#endif		
		{
			rv = psy_audio_machine_beatspersample(self->client);
		}
#if defined DIVERSALIS__OS__MICROSOFT		
		__except(FilterException(self, "beatspersample", GetExceptionCode(),
			GetExceptionInformation())) {
		}
#endif		
	}
	return rv;
}

psy_dsp_beat_t machineproxy_currbeatsperline(psy_audio_MachineProxy* self)
{
	psy_dsp_beat_t rv = 0;

	if (self->crashed == 0) {
#if defined DIVERSALIS__OS__MICROSOFT        
		__try
#endif		
		{
			rv = psy_audio_machine_currbeatsperline(self->client);
		}
#if defined DIVERSALIS__OS__MICROSOFT		
		__except (FilterException(self, "currbeatsperline", GetExceptionCode(),
			GetExceptionInformation())) {
		}
#endif		
	}
	return rv;
}

struct psy_audio_Samples* machineproxy_samples(psy_audio_MachineProxy* self)
{
	struct psy_audio_Samples* rv = 0;

	if (self->crashed == 0) {
#if defined DIVERSALIS__OS__MICROSOFT        
		__try
#endif		
		{
			rv = psy_audio_machine_samples(self->client);
		}
#if defined DIVERSALIS__OS__MICROSOFT		
		__except(FilterException(self, "samples", GetExceptionCode(),
			GetExceptionInformation())) {			
		}
#endif		
	}
	return rv;
}

struct psy_audio_Machines* machineproxy_machines(psy_audio_MachineProxy* self)
{
	struct psy_audio_Machines* rv = 0;

	if (self->crashed == 0) {
#if defined DIVERSALIS__OS__MICROSOFT        
		__try
#endif		
		{ 
			rv = psy_audio_machine_machines(self->client);
		}
#if defined DIVERSALIS__OS__MICROSOFT		
		__except(FilterException(self, "machines", GetExceptionCode(),
			GetExceptionInformation())) {			
		}
#endif		
	}
	return rv;
}

struct psy_audio_Instruments* machineproxy_instruments(psy_audio_MachineProxy* self)
{
	struct psy_audio_Instruments* rv = 0;

	if (self->crashed == 0) {
#if defined DIVERSALIS__OS__MICROSOFT        
		__try
#endif		
		{ 
			rv = self->client->vtable->instruments(self->client); 
		}
#if defined DIVERSALIS__OS__MICROSOFT		
		__except(FilterException(self, "instruments", GetExceptionCode(),
			GetExceptionInformation())) {			
		}
#endif		
	}
	return rv;
}

struct psy_audio_MachineFactory* machineproxy_machinefactory(psy_audio_MachineProxy* self)
{
	struct psy_audio_MachineFactory* rv = 0;

	if (self->crashed == 0) {
#if defined DIVERSALIS__OS__MICROSOFT        
		__try
#endif		
		{
			rv = self->client->vtable->machinefactory(self->client);
		}
#if defined DIVERSALIS__OS__MICROSOFT		
		__except (FilterException(self, "machinefactory", GetExceptionCode(),
			GetExceptionInformation())) {
		}
#endif		
	}
	return rv;
}

void machineproxy_output(psy_audio_MachineProxy* self, const char* text)
{	
#if defined DIVERSALIS__OS__MICROSOFT        
		__try
#endif		
		{ 
			psy_audio_machine_output(self->client, text);
		}
#if defined DIVERSALIS__OS__MICROSOFT		
		__except(FilterException(self, "output", GetExceptionCode(),
			GetExceptionInformation())) {			
		}
#endif			
}

bool machineproxy_addcapture(psy_audio_MachineProxy* self, int index)
{
	bool rv = 0;

	if (self->crashed == 0) {
#if defined DIVERSALIS__OS__MICROSOFT        
		__try
#endif		
		{
			rv = self->client->vtable->addcapture(self->client, index);
		}
#if defined DIVERSALIS__OS__MICROSOFT		
		__except (FilterException(self, "addcapture", GetExceptionCode(),
			GetExceptionInformation())) {
		}
#endif		
	}
	return rv;
}

bool machineproxy_removecapture(psy_audio_MachineProxy* self, int index)
{
	bool rv = 0;

	if (self->crashed == 0) {
#if defined DIVERSALIS__OS__MICROSOFT        
		__try
#endif		
		{
			rv = self->client->vtable->removecapture(self->client, index);
		}
#if defined DIVERSALIS__OS__MICROSOFT		
		__except (FilterException(self, "removecapture", GetExceptionCode(),
			GetExceptionInformation())) {
		}
#endif		
	}
	return rv;
}

void machineproxy_readbuffers(psy_audio_MachineProxy* self, int index, float**pleft, float** pright, int numsamples)
{	
	if (self->crashed == 0) {
#if defined DIVERSALIS__OS__MICROSOFT        
		__try
#endif		
		{
			self->client->vtable->readbuffers(self->client, index, pleft, pright, numsamples);
		}
#if defined DIVERSALIS__OS__MICROSOFT		
		__except (FilterException(self, "readbuffers", GetExceptionCode(),
			GetExceptionInformation())) {
		}
#endif		
	}	
}

void machineproxy_setcallback(psy_audio_MachineProxy* self, psy_audio_MachineCallback* callback)
{ 
	if (self->crashed == 0) {
#if defined DIVERSALIS__OS__MICROSOFT        
		__try
#endif		
		{ 
            
			psy_audio_machine_setcallback(self->client, callback);
		}
#if defined DIVERSALIS__OS__MICROSOFT		
		__except(FilterException(self, "setcallback", GetExceptionCode(),
			GetExceptionInformation())) {
		}
#endif		
	}
}

uintptr_t machineproxy_slot(psy_audio_MachineProxy* self)
{
	uintptr_t rv = 0;

	if (self->crashed == 0) {
#if defined DIVERSALIS__OS__MICROSOFT        
		__try
#endif		
		{ 
			rv = psy_audio_machine_slot(self->client);
		}
#if defined DIVERSALIS__OS__MICROSOFT		
		__except(FilterException(self, "slot", GetExceptionCode(),
			GetExceptionInformation())) {			
		}
#endif		
	}
	return rv;
}

void machineproxy_setslot(psy_audio_MachineProxy* self, uintptr_t slot)
{
	if (self->crashed == 0) {
#if defined DIVERSALIS__OS__MICROSOFT        
		__try
#endif		
		{ 
			self->client->vtable->setslot(self->client, slot);
		}
#if defined DIVERSALIS__OS__MICROSOFT		
		__except(FilterException(self, "setslot", GetExceptionCode(),
			GetExceptionInformation())) {
		}
#endif		
	}
}

int machineproxy_haseditor(psy_audio_MachineProxy* self)
{ 
	int rv = 0;
	
	if (self->crashed == 0) {
#if defined DIVERSALIS__OS__MICROSOFT        
		__try
#endif		
		{ 
			rv = psy_audio_machine_haseditor(self->client);
		}
#if defined DIVERSALIS__OS__MICROSOFT		
		__except(FilterException(self, "haseditor", GetExceptionCode(),
			GetExceptionInformation())) {			
		}
#endif		
	}
	return rv;
}

void machineproxy_seteditorhandle(psy_audio_MachineProxy* self, void* handle)
{
	if (self->crashed == 0) {
#if defined DIVERSALIS__OS__MICROSOFT        
		__try
#endif		
		{ 
			psy_audio_machine_seteditorhandle(self->client, handle);
		}
#if defined DIVERSALIS__OS__MICROSOFT		
		__except(FilterException(self, "seteditorhandle", GetExceptionCode(),
			GetExceptionInformation())) {			
		}
#endif		
	}	
}

void machineproxy_editorsize(psy_audio_MachineProxy* self, int* width, int* height)
{
	if (self->crashed == 0) {
#if defined DIVERSALIS__OS__MICROSOFT        
		__try
#endif		
		{ 
			self->client->vtable->editorsize(self->client, width, height);
		}
#if defined DIVERSALIS__OS__MICROSOFT		
		__except(FilterException(self, "editorsize", GetExceptionCode(),
				GetExceptionInformation())) {
			*width = 0;
			*height = 0;
		}
#endif		
	}	
}

static void machineproxy_editoridle(psy_audio_MachineProxy* self)
{
	if (self->crashed == 0) {
#if defined DIVERSALIS__OS__MICROSOFT        
		__try
#endif		
		{ 
			self->client->vtable->editoridle(self->client);
		}		
#if defined DIVERSALIS__OS__MICROSOFT		
		__except(FilterException(self, "editoridle", GetExceptionCode(),
			GetExceptionInformation())) {
		}
#endif		
	}
}

void machineproxy_setposition(psy_audio_MachineProxy* self, intptr_t x, intptr_t y)
{
	if (self->crashed == 0) {
#if defined DIVERSALIS__OS__MICROSOFT        
		__try
#endif		
		{
			self->client->vtable->setposition(self->client, x, y);
		}
#if defined DIVERSALIS__OS__MICROSOFT		
		__except (FilterException(self, "setposition", GetExceptionCode(),
			GetExceptionInformation())) {
		}
#endif		
	}
}

void machineproxy_position(psy_audio_MachineProxy* self, intptr_t* x, intptr_t* y)
{
	if (self->crashed == 0) {
#if defined DIVERSALIS__OS__MICROSOFT        
		__try
#endif		
		{
			self->client->vtable->position(self->client, x, y);
		}
#if defined DIVERSALIS__OS__MICROSOFT		
		__except (FilterException(self, "position", GetExceptionCode(),
			GetExceptionInformation())) {
		}
#endif		
	}
}

const char* machineproxy_editname(psy_audio_MachineProxy* self)
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
		__except (FilterException(self, "editname", GetExceptionCode(),
			GetExceptionInformation())) {
		}
#endif		
	}
	return rv;
}

void machineproxy_seteditname(psy_audio_MachineProxy* self, const char* name)
{
	if (self->crashed == 0) {
#if defined DIVERSALIS__OS__MICROSOFT        
		__try
#endif		
		{
			psy_audio_machine_seteditname(self->client, name);
		}
#if defined DIVERSALIS__OS__MICROSOFT		
		__except (FilterException(self, "seteditname", GetExceptionCode(),
			GetExceptionInformation())) {
		}
#endif		
	}
}

psy_audio_Buffer* machineproxy_buffermemory(psy_audio_MachineProxy* self)
{
	psy_audio_Buffer* rv = 0;

	if (self->crashed == 0) {
#if defined DIVERSALIS__OS__MICROSOFT        
		__try
#endif		
		{
			rv = psy_audio_machine_buffermemory(self->client);
		}
#if defined DIVERSALIS__OS__MICROSOFT		
		__except (FilterException(self, "buffermemory", GetExceptionCode(),
			GetExceptionInformation())) {
		}
#endif		
	}
	return rv;
}

uintptr_t machineproxy_buffermemorysize(psy_audio_MachineProxy* self)
{
	uintptr_t rv = 0;

	if (self->crashed == 0) {
#if defined DIVERSALIS__OS__MICROSOFT        
		__try
#endif		
		{
			rv = psy_audio_machine_buffermemorysize(self->client);
		}
#if defined DIVERSALIS__OS__MICROSOFT		
		__except (FilterException(self, "buffermemorysize", GetExceptionCode(),
			GetExceptionInformation())) {
		}
#endif		
	}
	return rv;
}

void machineproxy_setbuffermemorysize(psy_audio_MachineProxy* self, uintptr_t size)
{
if (self->crashed == 0) {
#if defined DIVERSALIS__OS__MICROSOFT        
		__try
#endif		
		{
			psy_audio_machine_setbuffermemorysize(self->client, size);
		}
#if defined DIVERSALIS__OS__MICROSOFT		
		__except (FilterException(self, "setbuffermemorysize",
			GetExceptionCode(), GetExceptionInformation())) {
		}
#endif		
	}
}

psy_dsp_amp_range_t machineproxy_amprange(psy_audio_MachineProxy* self)
{
	psy_dsp_amp_range_t rv = 0;

	if (self->crashed == 0) {
#if defined DIVERSALIS__OS__MICROSOFT        
		__try
#endif		
		{
			rv = psy_audio_machine_amprange(self->client);
		}
#if defined DIVERSALIS__OS__MICROSOFT		
		__except (FilterException(self, "amprange", GetExceptionCode(),
			GetExceptionInformation())) {
		}
#endif		
	}
	return rv;
}

psy_List* machineproxy_sequencerinsert(psy_audio_MachineProxy* self, psy_List* events)
{
	psy_List* rv = 0;

	if (self->crashed == 0) {
#if defined DIVERSALIS__OS__MICROSOFT        
		__try
#endif		
		{
			rv = psy_audio_machine_sequencerinsert(self->client, events);
		}
#if defined DIVERSALIS__OS__MICROSOFT		
		__except (FilterException(self, "sequencerinsert", GetExceptionCode(),
			GetExceptionInformation())) {
		}
#endif		
	}
	
	return rv;
}

void machineproxy_putdata(psy_audio_MachineProxy* self, uint8_t* data)
{
	if (self->crashed == 0) {
#if defined DIVERSALIS__OS__MICROSOFT        
		__try
#endif		
		{
			psy_audio_machine_putdata(self->client, data);
		}
#if defined DIVERSALIS__OS__MICROSOFT		
		__except (FilterException(self, "putdata",
			GetExceptionCode(), GetExceptionInformation())) {
		}
#endif		
	}
}

uint8_t* machineproxy_data(psy_audio_MachineProxy* self)
{
	uint8_t* rv = 0;

	if (self->crashed == 0) {
#if defined DIVERSALIS__OS__MICROSOFT        
		__try
#endif		
		{
			rv = psy_audio_machine_data(self->client);
		}
#if defined DIVERSALIS__OS__MICROSOFT		
		__except (FilterException(self, "data", GetExceptionCode(),
			GetExceptionInformation())) {
		}
#endif		
	}
	return rv;
}

uintptr_t machineproxy_datasize(psy_audio_MachineProxy* self)
{
	uintptr_t rv = 0;

	if (self->crashed == 0) {
#if defined DIVERSALIS__OS__MICROSOFT        
		__try
#endif		
		{
			rv = psy_audio_machine_datasize(self->client);
		}
#if defined DIVERSALIS__OS__MICROSOFT		
		__except (FilterException(self, "datasize", GetExceptionCode(),
			GetExceptionInformation())) {
		}
#endif		
	}
	return rv;
}

void machineproxy_programname(psy_audio_MachineProxy* self, int bnkidx, int prgIdx, char* val)
{
	if (self->crashed == 0) {
#if defined DIVERSALIS__OS__MICROSOFT        
		__try
#endif		
		{
			psy_audio_machine_programname(self->client, bnkidx, prgIdx, val);
		}
#if defined DIVERSALIS__OS__MICROSOFT		
		__except (FilterException(self, "programname",
			GetExceptionCode(), GetExceptionInformation())) {
		}
#endif		
	}
}

int machineproxy_numprograms(psy_audio_MachineProxy* self)
{
	int rv = 0;

	if (self->crashed == 0) {
#if defined DIVERSALIS__OS__MICROSOFT        
		__try
#endif		
		{
			rv = psy_audio_machine_numprograms(self->client);
		}
#if defined DIVERSALIS__OS__MICROSOFT		
		__except (FilterException(self, "numprograms", GetExceptionCode(),
			GetExceptionInformation())) {
		}
#endif		
	}
	return rv;
}

void machineproxy_bankname(psy_audio_MachineProxy* self, int bnkidx, char* val)
{
	if (self->crashed == 0) {
#if defined DIVERSALIS__OS__MICROSOFT        
		__try
#endif		
		{
			psy_audio_machine_bankname(self->client, bnkidx, val);
		}
#if defined DIVERSALIS__OS__MICROSOFT		
		__except (FilterException(self, "bankname",
			GetExceptionCode(), GetExceptionInformation())) {
		}
#endif		
	}
}

int machineproxy_numbanks(psy_audio_MachineProxy* self)
{
	int rv = 0;

	if (self->crashed == 0) {
#if defined DIVERSALIS__OS__MICROSOFT        
		__try
#endif		
		{
			rv = psy_audio_machine_numbanks(self->client);
		}
#if defined DIVERSALIS__OS__MICROSOFT		
		__except (FilterException(self, "numbanks", GetExceptionCode(),
			GetExceptionInformation())) {
		}
#endif		
	}
	return rv;
}

void machineproxy_setcurrprogram(psy_audio_MachineProxy* self, int prgIdx)
{
	if (self->crashed == 0) {
#if defined DIVERSALIS__OS__MICROSOFT        
		__try
#endif		
		{
			psy_audio_machine_setcurrprogram(self->client, prgIdx);
		}
#if defined DIVERSALIS__OS__MICROSOFT		
		__except (FilterException(self, "programname",
			GetExceptionCode(), GetExceptionInformation())) {
		}
#endif		
	}
}

int machineproxy_currprogram(psy_audio_MachineProxy* self)
{
	int rv = 0;

	if (self->crashed == 0) {
#if defined DIVERSALIS__OS__MICROSOFT        
		__try
#endif		
		{
			rv = psy_audio_machine_currprogram(self->client);
		}
#if defined DIVERSALIS__OS__MICROSOFT		
		__except (FilterException(self, "currprogram", GetExceptionCode(),
			GetExceptionInformation())) {
		}
#endif		
	}
	return rv;
}

void machineproxy_setcurrbank(psy_audio_MachineProxy* self, int prgIdx)
{
	if (self->crashed == 0) {
#if defined DIVERSALIS__OS__MICROSOFT        
		__try
#endif		
		{
			psy_audio_machine_setcurrbank(self->client, prgIdx);
		}
#if defined DIVERSALIS__OS__MICROSOFT		
		__except (FilterException(self, "bankname",
			GetExceptionCode(), GetExceptionInformation())) {
		}
#endif		
	}
}

int machineproxy_currbank(psy_audio_MachineProxy* self)
{
	int rv = 0;

	if (self->crashed == 0) {
#if defined DIVERSALIS__OS__MICROSOFT        
		__try
#endif		
		{
			rv = psy_audio_machine_currbank(self->client);
		}
#if defined DIVERSALIS__OS__MICROSOFT		
		__except (FilterException(self, "currbank", GetExceptionCode(),
			GetExceptionInformation())) {
		}
#endif		
	}
	return rv;
}

void machineproxy_currentpreset(psy_audio_MachineProxy* self, struct psy_audio_Preset* preset)
{	
	if (self->crashed == 0) {
#if defined DIVERSALIS__OS__MICROSOFT        
		__try
#endif		
		{
			psy_audio_machine_currentpreset(self->client, preset);
		}
#if defined DIVERSALIS__OS__MICROSOFT		
		__except (FilterException(self, "currentpreset", GetExceptionCode(),
			GetExceptionInformation())) {
		}
#endif		
	}
}

void machineproxy_setpresets(psy_audio_MachineProxy* self, struct psy_audio_Presets* presets)
{
	if (self->crashed == 0) {
#if defined DIVERSALIS__OS__MICROSOFT        
		__try
#endif		
		{
			psy_audio_machine_setpresets(self->client, presets);
		}
#if defined DIVERSALIS__OS__MICROSOFT		
		__except (FilterException(self, "setpresets", GetExceptionCode(),
			GetExceptionInformation())) {
		}
#endif		
	}
}

struct psy_audio_Presets* machineproxy_presets(psy_audio_MachineProxy* self)
{
	struct psy_audio_Presets* rv = 0;

	if (self->crashed == 0) {
#if defined DIVERSALIS__OS__MICROSOFT        
		__try
#endif		
		{
			rv = psy_audio_machine_presets(self->client);
		}
#if defined DIVERSALIS__OS__MICROSOFT		
		__except (FilterException(self, "presets", GetExceptionCode(),
			GetExceptionInformation())) {
		}
#endif		
	}
	return rv;
}

bool machineproxy_acceptpresets(psy_audio_MachineProxy* self)
{
	bool rv = FALSE;

	if (self->crashed == 0) {
#if defined DIVERSALIS__OS__MICROSOFT        
		__try
#endif		
		{
			return psy_audio_machine_acceptpresets(self->client);
		}
#if defined DIVERSALIS__OS__MICROSOFT		
		__except (FilterException(self, "acceptpresets", GetExceptionCode(),
			GetExceptionInformation())) {
		}
#endif		
	}
	return rv;
}

void machineproxy_command(psy_audio_MachineProxy* self)
{
	if (self->crashed == 0) {
#if defined DIVERSALIS__OS__MICROSOFT        
		__try
#endif		
		{
			psy_audio_machine_command(self->client);
		}
#if defined DIVERSALIS__OS__MICROSOFT		
		__except (FilterException(self, "command", GetExceptionCode(),
			GetExceptionInformation())) {
		}
#endif		
	}
}


const char* machineproxy_capturename(psy_audio_MachineProxy* self, int index)
{
	const char* rv = 0;

	if (self->crashed == 0) {
#if defined DIVERSALIS__OS__MICROSOFT        
		__try
#endif		
		{
			rv = psy_audio_machine_capturename(self->client, index);
		}
#if defined DIVERSALIS__OS__MICROSOFT		
		__except (FilterException(self, "capturename", GetExceptionCode(),
			GetExceptionInformation())) {
		}
#endif		
	}
	return rv;
}

int machineproxy_numcaptures(psy_audio_MachineProxy* self)
{
	int rv = 0;

	if (self->crashed == 0) {
#if defined DIVERSALIS__OS__MICROSOFT        
		__try
#endif		
		{
			rv = psy_audio_machine_numcaptures(self->client);
		}
#if defined DIVERSALIS__OS__MICROSOFT		
		__except (FilterException(self, "numcaptures", GetExceptionCode(),
			GetExceptionInformation())) {
		}
#endif		
	}
	return rv;
}

const char* machineproxy_playbackname(psy_audio_MachineProxy* self, int index)
{
	const char* rv = 0;

	if (self->crashed == 0) {
#if defined DIVERSALIS__OS__MICROSOFT        
		__try
#endif		
		{
			rv = psy_audio_machine_playbackname(self->client, index);
		}
#if defined DIVERSALIS__OS__MICROSOFT		
		__except (FilterException(self, "playbackname", GetExceptionCode(),
			GetExceptionInformation())) {
		}
#endif		
	}
	return rv;
}

int machineproxy_numplaybacks(psy_audio_MachineProxy* self)
{
	int rv = 0;

	if (self->crashed == 0) {
#if defined DIVERSALIS__OS__MICROSOFT        
		__try
#endif		
		{
			rv = psy_audio_machine_numplaybacks(self->client);
		}
#if defined DIVERSALIS__OS__MICROSOFT		
		__except (FilterException(self, "numplaybacks", GetExceptionCode(),
			GetExceptionInformation())) {
		}
#endif		
	}
	return rv;
}

void machineproxy_param_tweak(psy_audio_MachineProxy* self, psy_audio_MachineParam* param, float val)
{	
	if (self->crashed == 0) {
#if defined DIVERSALIS__OS__MICROSOFT        
		__try
#endif		
		{
			psy_audio_machine_parameter_tweak(self->client, param, val);
		}
#if defined DIVERSALIS__OS__MICROSOFT		
		__except (FilterException(self, "parameter tweak", GetExceptionCode(),
			GetExceptionInformation())) {
		}
#endif		
	}	
}

void machineproxy_param_reset(psy_audio_MachineProxy* self, psy_audio_MachineParam* param)
{
	if (self->crashed == 0) {
#if defined DIVERSALIS__OS__MICROSOFT        
		__try
#endif		
		{
			psy_audio_machine_parameter_reset(self->client, param);
		}
#if defined DIVERSALIS__OS__MICROSOFT		
		__except (FilterException(self, "parameter reset", GetExceptionCode(),
			GetExceptionInformation())) {
		}
#endif		
	}
}

float machineproxy_param_normvalue(psy_audio_MachineProxy* self, psy_audio_MachineParam* param)
{
	float rv = 0;

	if (self->crashed == 0) {
#if defined DIVERSALIS__OS__MICROSOFT        
		__try
#endif		
		{
			rv = psy_audio_machine_parameter_normvalue(self->client, param);
		}
#if defined DIVERSALIS__OS__MICROSOFT		
		__except (FilterException(self, "parameter normvalue", GetExceptionCode(),
			GetExceptionInformation())) {
		}
#endif		
	}
	return rv;
}

void machineproxy_param_range(psy_audio_MachineProxy* self, psy_audio_MachineParam* param, intptr_t* minval, intptr_t* maxval)
{	
	if (self->crashed == 0) {
#if defined DIVERSALIS__OS__MICROSOFT        
		__try
#endif		
		{
			psy_audio_machine_parameter_range(self->client, param, minval, maxval);
		}
#if defined DIVERSALIS__OS__MICROSOFT		
		__except (FilterException(self, "parameter range", GetExceptionCode(),
			GetExceptionInformation())) {
		}
#endif		
	}	
}

int machineproxy_param_type(psy_audio_MachineProxy* self, psy_audio_MachineParam* param)
{
	int rv = 0;

	if (self->crashed == 0) {
#if defined DIVERSALIS__OS__MICROSOFT        
		__try
#endif		
		{
			rv = psy_audio_machine_parameter_type(self->client, param);
		}
#if defined DIVERSALIS__OS__MICROSOFT		
		__except (FilterException(self, "parameter type", GetExceptionCode(),
			GetExceptionInformation())) {
		}
#endif		
	}
	return rv;
}

int machineproxy_param_label(psy_audio_MachineProxy* self, psy_audio_MachineParam* param, char* text)
{
	int rv = 0;

	if (self->crashed == 0) {
#if defined DIVERSALIS__OS__MICROSOFT        
		__try
#endif		
		{
			rv = psy_audio_machine_parameter_label(self->client, param, text);
		}
#if defined DIVERSALIS__OS__MICROSOFT		
		__except (FilterException(self, "parameter label", GetExceptionCode(),
			GetExceptionInformation())) {
		}
#endif		
	}
	return rv;
}

int machineproxy_param_name(psy_audio_MachineProxy* self, psy_audio_MachineParam* param, char* text)
{
	int rv = 0;

	if (self->crashed == 0) {
#if defined DIVERSALIS__OS__MICROSOFT        
		__try
#endif		
		{
			rv = psy_audio_machine_parameter_name(self->client, param, text);
		}
#if defined DIVERSALIS__OS__MICROSOFT		
		__except (FilterException(self, "parameter name", GetExceptionCode(),
			GetExceptionInformation())) {
		}
#endif		
	}
	return rv;
}

int machineproxy_param_describe(psy_audio_MachineProxy* self, psy_audio_MachineParam* param, char* text)
{
	int rv = 0;

	if (self->crashed == 0) {
#if defined DIVERSALIS__OS__MICROSOFT        
		__try
#endif		
		{
			rv = psy_audio_machine_parameter_describe(self->client, param, text);
		}
#if defined DIVERSALIS__OS__MICROSOFT		
		__except (FilterException(self, "parameter describe", GetExceptionCode(),
			GetExceptionInformation())) {
		}
#endif		
	}
	return rv;
}

psy_audio_Machine* machineproxy_clone(psy_audio_MachineProxy* self)
{
	psy_audio_Machine* rv = NULL;

	if (self->crashed == 0) {
#if defined DIVERSALIS__OS__MICROSOFT        
		__try
#endif		
		{
			rv = psy_audio_machine_clone(self->client);
			if (rv) {
				psy_audio_MachineProxy* proxy;

				proxy = psy_audio_machineproxy_allocinit(rv);
				if (proxy) {
					rv = psy_audio_machineproxy_base(proxy);
				} else {
					machine_base_dispose(rv);
					free(rv);
					rv = 0;
				}
			}
		}
#if defined DIVERSALIS__OS__MICROSOFT		
		__except (FilterException(self, "machine clone", GetExceptionCode(),
			GetExceptionInformation())) {
		}
#endif	
	}
	return rv;
}

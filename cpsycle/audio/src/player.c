// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "player.h"
#include "exclusivelock.h"
#include "math.h"
#include "master.h"
#include "silentdriver.h"
#include <operations.h>
#include <stdlib.h>
#include <string.h>
#include <rms.h>
#include <multifilter.h>
#include <windows.h>

#if !defined(FALSE)
#define FALSE 0
#endif

#if !defined(TRUE)
#define TRUE 1
#endif

static float bufferdriver[65535];
static void* mainframe;

static void player_initdriver(Player*);
static void player_initeventdriver(Player*);
static void player_initrms(Player*);
static void player_initsignals(Player*);
static void player_disposerms(Player*);
static void player_unloaddriver(Player*);
static void player_unloadeventdriver(Player * self);
static float* Work(Player*, int* numsamples);
static void WorkMidi(Player*, int cmd, unsigned char* data, unsigned int size);
static void player_workpath(Player*, unsigned int amount);
static Buffer* player_mix(Player*, unsigned int slot, unsigned int amount);
static void player_filldriver(Player*, float* buffer, unsigned int amount);
static RMSVol* player_rmsvol(Player*, unsigned int slot);
static void player_resetvumeters(Player*);
static void erase_all_nans_infinities_and_denormals(float * sample);

void player_init(Player* self, Song* song, void* handle)
{			
	self->song = song;	
	self->numsongtracks = 16;	
	sequencer_init(&self->sequencer, &song->sequence, &song->machines);
	mainframe = handle;
	player_initdriver(self);
	player_initeventdriver(self);
	player_initsignals(self);
	player_initrms(self);	
}

void player_initdriver(Player* self)
{	
	self->driver = 0;
	lock_init();
	library_init(&self->drivermodule);
	player_loaddriver(self, 0);		
}

void player_initeventdriver(Player* self)
{
	self->eventdriver = 0;
	library_init(&self->eventdrivermodule);
	player_loadeventdriver(self, "..\\driver\\mmemidi\\Debug\\mmemidi.dll");
}

void player_initsignals(Player* self)
{
	signal_init(&self->signal_numsongtrackschanged);
	signal_init(&self->signal_lpbchanged);
}

void player_initrms(Player* self)
{
	self->vumode = VUMETER_RMS;
	self->resetvumeters = 0;
	table_init(&self->rms);
}

void player_dispose(Player* self)
{			
	player_unloaddriver(self);
	player_unloadeventdriver(self);
	lock_dispose();	
	signal_dispose(&self->signal_lpbchanged);
	sequencer_dispose(&self->sequencer);		
	player_disposerms(self);
}

void player_disposerms(Player* self)
{
	TableIterator it;

	for (it = table_begin(&self->rms); !tableiterator_equal(&it, table_end());
			tableiterator_inc(&it)) {
		free(tableiterator_value(&it));
	}
	table_dispose(&self->rms);
}

void player_loaddriver(Player* self, const char* path)
{
	Driver* driver = 0;
	
	player_unloaddriver(self);	
	if (path) {
		library_load(&self->drivermodule, path);	
		if (self->drivermodule.module) {
			pfndriver_create fpdrivercreate;

			fpdrivercreate = (pfndriver_create)
				library_functionpointer(&self->drivermodule, "driver_create");
			if (fpdrivercreate) {
				driver = fpdrivercreate();		
				driver->init(driver);
				driver->connect(driver, self, Work, mainframe);								
			}
			lock_enable();
		}		
	}
	if (!driver) {
		driver = create_silent_driver();
		driver->init(driver);
		lock_disable();
	}	
	sequencer_setsamplerate(&self->sequencer, driver->samplerate(driver));
	rmsvol_setsamplerate(driver->samplerate(driver));
	multifilter_inittables(driver->samplerate(driver));
	self->driver = driver;
	self->driver->open(self->driver);
}

void player_loadeventdriver(Player* self, const char* path)
{
	EventDriver* eventdriver = 0;

	player_unloadeventdriver(self);
	if (path) {
		library_load(&self->eventdrivermodule, path);
		if (self->eventdrivermodule.module) {
			pfneventdriver_create fpeventdrivercreate;

			fpeventdrivercreate = (pfneventdriver_create)
				library_functionpointer(&self->eventdrivermodule,
					"eventdriver_create");
			if (fpeventdrivercreate) {
				eventdriver = fpeventdrivercreate();
				eventdriver->init(eventdriver);
				eventdriver->connect(eventdriver, self, WorkMidi, mainframe);
			}
			// lock_enable();
		}
	}
	if (!eventdriver) {
		// eventdriver = create_silent_driver();
		// eventdriver->init(driver);
		// lock_disable();
	}
	self->eventdriver = eventdriver;
	if (self->eventdriver) {
		self->eventdriver->open(self->eventdriver);
	}
}

void player_unloaddriver(Player* self)
{
	if (self->driver) {
		self->driver->close(self->driver);		
		self->driver->dispose(self->driver);
		self->driver->free(self->driver);		
		library_unload(&self->drivermodule);		
	}
	self->driver = 0;
}

void player_unloadeventdriver(Player* self)
{
	if (self->eventdriver) {
		self->eventdriver->close(self->eventdriver);
		self->eventdriver->dispose(self->eventdriver);
		self->eventdriver->free(self->eventdriver);
		library_unload(&self->eventdrivermodule);
	}
	self->eventdriver = 0;
}

void player_reloaddriver(Player* self, const char* path)
{		
	player_unloaddriver(self);
	player_loaddriver(self, path);		
}

void player_restartdriver(Player* self)
{	
	self->driver->close(self->driver);	
	self->driver->updateconfiguration(self->driver);
	self->driver->open(self->driver);	
}

void player_restarteventdriver(Player* self)
{	
	self->eventdriver->close(self->eventdriver);	
	self->eventdriver->updateconfiguration(self->eventdriver);
	self->eventdriver->open(self->eventdriver);	
}

void player_setsong(Player* self, Song* song)
{
	self->song = song;
	sequencer_reset(&self->sequencer, &song->sequence, &song->machines);
	sequencer_setsamplerate(&self->sequencer,
		self->driver->samplerate(self->driver));
}

void player_start(Player* self)
{	
	sequencer_setposition(&self->sequencer, 0.0f);
	sequencer_start(&self->sequencer);	
}

void player_stop(Player* self)
{
	sequencer_stop(&self->sequencer);
}

int player_playing(Player* self)
{
	return sequencer_playing(&self->sequencer);
}

beat_t player_position(Player* self)
{
	return sequencer_position(&self->sequencer);
}

void player_setbpm(Player* self, beat_t bpm)
{
	sequencer_setbpm(&self->sequencer, bpm);	
}

beat_t player_bpm(Player* self)
{
	return sequencer_bpm(&self->sequencer);
}

void player_setlpb(Player* self, unsigned int lpb)
{
	sequencer_setlpb(&self->sequencer, lpb);
	signal_emit(&self->signal_lpbchanged, self, 1, lpb);
}

unsigned int player_lpb(Player* self)
{
	return sequencer_lpb(&self->sequencer);
}

real* Work(Player* self, int* numsamples)
{		
	unsigned int amount;
	unsigned int numsamplex;
	float* psamples;
	
	psamples = bufferdriver;
	numsamplex = *numsamples;	
	amount = numsamplex > MAX_STREAM_SIZE ? MAX_STREAM_SIZE : numsamplex;
	lock_enter();
	do {
		sequencer_frametick(&self->sequencer, amount);
		player_workpath(self, amount);
		player_filldriver(self, psamples, amount);
		numsamplex -= amount;		
		psamples  += (2*amount);
	}  while (numsamplex > 0);
	lock_leave();
	return bufferdriver;
}

static void panbuffer(Buffer* buffer, float pan, unsigned int amount);

void player_workpath(Player* self, unsigned int amount)
{
	MachinePath* path;
	path = machines_path(&self->song->machines);
	if (path) {
		for ( ; path != 0; path = path->next) {
			unsigned int slot;						
			Buffer* output;

			slot = (int)path->entry;				
			output = player_mix(self, slot, amount);								
			if (slot != MASTER_INDEX) {				
				Machine* machine;
				BufferContext bc;

				machine = machines_at(&self->song->machines, slot);
				if (machine && output) {
					List* events;
					RMSVol* rms;

					events = sequencer_timedevents(&self->sequencer, slot, amount);
					rms = player_rmsvol(self, slot);					
					buffercontext_init(&bc, events, output, output, amount,
						self->numsongtracks, rms);
					machine->work(machine, &bc);
					buffer_pan(output, machine_panning(machine), amount);
					if (self->vumode == VUMETER_RMS && buffer_numchannels(
							bc.output) >= 2) {
						rmsvol_tick(rms, bc.output->samples[0], bc.output->samples[1],
							bc.numsamples);
					}
					signal_emit(&machine->signal_worked, machine, 2, slot, &bc);
					list_free(events);
				}									
			}			
		}							
	}
	if (self->resetvumeters) {
		player_resetvumeters(self);
	}
}

void player_resetvumeters(Player* self)
{	
	table_dispose(&self->rms);
	table_init(&self->rms);
	self->resetvumeters = 0;	
}

static void addsamples(Buffer* dst, Buffer* source, unsigned int numsamples, float vol);

Buffer* player_mix(Player* self, unsigned int slot, unsigned int amount)
{
	MachineConnections* connections;				
	Buffer* output;
	
	connections = machines_connections(&self->song->machines, slot);
	output = machines_outputs(&self->song->machines, slot);

	if (output) {
		buffer_clearsamples(output, amount);
		if (connections) {
			MachineConnection* connection;
			
			for (connection = connections->inputs; connection != 0;
					connection = connection->next) {
				MachineConnectionEntry* source = 
					(MachineConnectionEntry*)connection->entry;
				if (source->slot != -1) {
					addsamples(
						output, 
						machines_outputs(&self->song->machines,
							source->slot),
						amount,
						1.0f);
				}						
			}								
		}
	}
	return output;
}


void addsamples(Buffer* dst, Buffer* source, unsigned int numsamples, float vol)
{
	unsigned int channel;

	if (source) {
		for (channel = 0; channel < source->numchannels && 
			channel < dst->numchannels; ++channel) {
				dsp_add(
					source->samples[channel],
					dst->samples[channel],
					numsamples,
					vol);
				dsp_erase_all_nans_infinities_and_denormals(
					dst->samples[channel], numsamples);					
		}
	}
}

void player_filldriver(Player* self, float* buffer, unsigned int amount)
{
	Buffer* masteroutput;
	masteroutput = machines_outputs(&self->song->machines, MASTER_INDEX);
	if (masteroutput) {		
		Machine* master;

		master = machines_master(&self->song->machines);
		if (master) {
			BufferContext bc;
			RMSVol* rms;

			rms = player_rmsvol(self, MASTER_INDEX);
			buffercontext_init(&bc, 0, masteroutput, masteroutput, amount,
				self->numsongtracks, rms);
			buffer_mulsamples(masteroutput, amount,
				machines_volume(&self->song->machines));			
			if (self->vumode == VUMETER_RMS) {
				rmsvol_tick(rms, masteroutput->samples[0], masteroutput->samples[1],
					amount);		
			}
			signal_emit(&master->signal_worked, master, 2, MASTER_INDEX, &bc);			
		}
		dsp_interleave(buffer, masteroutput->samples[0],
			masteroutput->samples[1], amount);
	}
}

RMSVol* player_rmsvol(Player* self, unsigned int slot)
{
	RMSVol* rv;

	if (!table_exists(&self->rms, slot)) {
		rv = (RMSVol*)malloc(sizeof(RMSVol));
		rmsvol_init(rv);
		table_insert(&self->rms, slot, rv);
	} else {
		rv = table_at(&self->rms, slot);
	}
	return rv;
}

void player_setnumsongtracks(Player* self, unsigned int numsongtracks)
{
	if (numsongtracks >= 1 && numsongtracks <= 64) {
		self->numsongtracks = numsongtracks;	
		signal_emit(&self->signal_numsongtrackschanged, self, 1,
			self->numsongtracks);
	}
}

unsigned int player_numsongtracks(Player* self)
{
	return self->numsongtracks;
}

void player_setvumetermode(Player* self, VUMeterMode mode)
{
	self->vumode = mode;
	self->resetvumeters = 1;
}

VUMeterMode player_vumetermode(Player* self)
{
	return self->vumode;
}

void WorkMidi(Player* self, int cmd, unsigned char* data, unsigned int size)
{
//	char text[20];

	// _snprintf(text, 20, "%02X %02X %02X %02X; ", data[0], data[1], data[2], data[3]);	
	// MessageBox(0, text, "Midi In", MB_OK);
	if (cmd == 1) {  // MIDI DATA
		int lsb;
		int msb;

		lsb = data[0] & 0x0F;
		msb = (data[0] & 0xF0) >> 4;

		switch (msb) {
			case 0x9:
			{
				// Note On/Off
				PatternEvent event;

				event.note = data[2] > 0 ? data[1] : NOTECOMMANDS_RELEASE;
				event.inst = 255;
				event.mach = lsb;
				event.cmd = 0;
				event.parameter = 0;

				sequencer_addinputevent(&self->sequencer, &event, 0);				
			}
			default:
			break;			
		}
	}
}

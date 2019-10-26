// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "player.h"
#include "exclusivelock.h"
#include "math.h"
#include "master.h"
#include "silentdriver.h"
#include "kbddriver.h"
#include <operations.h>
#include <stdlib.h>
#include <string.h>
#include <rms.h>
#include <multifilter.h>

#if !defined(FALSE)
#define FALSE 0
#endif

#if !defined(TRUE)
#define TRUE 1
#endif

static amp_t bufferdriver[65535];
static void* mainframe;

static void player_initdriver(Player*);
static void player_initkbddriver(Player*);
static void player_initrms(Player*);
static void player_initsignals(Player*);
static void player_disposerms(Player*);
static void player_unloaddriver(Player*);
static void player_unloadeventdrivers(Player*);
static amp_t* work(Player*, int* numsamples);
static void workeventinput(Player*, int cmd, unsigned char* data, unsigned int size);
static void player_workpath(Player*, unsigned int amount);
static void player_filldriver(Player*, amp_t* buffer, unsigned int amount);
static RMSVol* player_rmsvol(Player*, unsigned int slot);
static void player_resetvumeters(Player*);

// player init and dispose

void player_init(Player* self, Song* song, void* handle)
{			
	self->song = song;	
	self->numsongtracks = 16;
	self->recordingnotes = 0;
	sequencer_init(&self->sequencer, &song->sequence, &song->machines);
	mainframe = handle;
	player_initdriver(self);	
	eventdrivers_init(&self->eventdrivers, self, workeventinput, handle);
	player_initsignals(self);
	player_initrms(self);	
}

void player_initdriver(Player* self)
{	
	self->driver = 0;	
	library_init(&self->drivermodule);
	player_loaddriver(self, 0);		
}

void player_initsignals(Player* self)
{
	signal_init(&self->signal_numsongtrackschanged);
	signal_init(&self->signal_lpbchanged);
	signal_init(&self->signal_inputevent);
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
	library_dispose(&self->drivermodule);
	eventdrivers_dispose(&self->eventdrivers);	
	signal_dispose(&self->signal_numsongtrackschanged);
	signal_dispose(&self->signal_lpbchanged);
	signal_dispose(&self->signal_inputevent);
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

// driver callbacks

// sound driver callback

amp_t* work(Player* self, int* numsamples)
{		
	unsigned int amount;
	unsigned int numsamplex;
	amp_t* psamples;
	
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

void player_workpath(Player* self, unsigned int amount)
{
	MachinePath* path;
	path = machines_path(&self->song->machines);
	if (path) {
		for ( ; path != 0; path = path->next) {
			unsigned int slot;									
			Machine* machine;

			slot = (int)path->entry;
			machine = machines_at(&self->song->machines, slot);
			if (machine && !table_exists(&self->song->machines.connections.sends, slot)) {
				Buffer* output;

				output = machine->mix(machine, slot, amount,
					connections_at(&self->song->machines.connections, slot),
					&self->song->machines);
				if (output && slot != MASTER_INDEX) {				
					BufferContext bc;										
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

void player_filldriver(Player* self, amp_t* buffer, unsigned int amount)
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
		rv = rmsvol_allocinit();		
		table_insert(&self->rms, slot, rv);
	} else {
		rv = (RMSVol*) table_at(&self->rms, slot);
	}
	return rv;
}

// event driver callback

void workeventinput(Player* self, int cmd, unsigned char* data, unsigned int size)
{	
	int validevent = 0;
	PatternEvent event;
	
	switch (cmd) {
		case 1:// MIDI DATA
		{
			int lsb;
			int msb;

			lsb = data[0] & 0x0F;
			msb = (data[0] & 0xF0) >> 4;
			switch (msb) {
				case 0x9:
					// Note On/Off
					event.note = data[2] > 0 ? data[1] : NOTECOMMANDS_RELEASE;
					event.inst = 255;
					event.mach = lsb;
					event.cmd = 0;
					event.parameter = 0;
					validevent = 1;
				default:
				break;
			}
		}
		break;
		case 2:	
			patternevent_init(&event, data[0], 255, 0, 0, 0);
			validevent = 1;
		break;
		default:		
		break;
	}
	if (validevent) {
		sequencer_addinputevent(&self->sequencer, &event, 0);
		if (self->recordingnotes && sequencer_playing(&self->sequencer)) {
			sequencer_recordinputevent(&self->sequencer, &event, 0, 
				player_position(self));			
		} else {			
			signal_emit(&self->signal_inputevent, self, 1, &event);
		}
	}
}

// general setter and getter

void player_setsong(Player* self, Song* song)
{
	self->song = song;
	sequencer_reset(&self->sequencer, &song->sequence, &song->machines);
	sequencer_setsamplerate(&self->sequencer,
		self->driver->samplerate(self->driver));
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

// sequencer setter and getter

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

// Driver load, unload, restart, ..., methods

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
				driver->connect(driver, self, work, mainframe);
			}
			lock_enable();
		}		
	}
	if (!driver) {
		driver = create_silent_driver();		
		lock_disable();
	}	
	sequencer_setsamplerate(&self->sequencer, driver->samplerate(driver));
	rmsvol_setsamplerate(driver->samplerate(driver));
	multifilter_inittables(driver->samplerate(driver));
	self->driver = driver;
	self->driver->open(self->driver);
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

// Event Recording

void player_startrecordingnotes(Player* self)
{
	self->recordingnotes = 1;
}

void player_stoprecordingnotes(Player* self)
{
	self->recordingnotes = 0;
}

int player_recordingnotes(Player* self)
{
	return self->recordingnotes;
}

// EventDriver load, unload, restart, ..., methods

void player_loadeventdriver(Player* self, const char* path)
{
	eventdrivers_load(&self->eventdrivers, path);
}

void player_restarteventdriver(Player* self, int id)
{
	eventdrivers_restart(&self->eventdrivers, id);
}

void player_removeeventdriver(Player * self, int id)
{
	eventdrivers_remove(&self->eventdrivers, id);
}

EventDriver* player_kbddriver(Player* self)
{
	return self->eventdrivers.kbddriver;
}

EventDriver* player_eventdriver(Player* self, int id) 
{
	return eventdrivers_driver(&self->eventdrivers, id);
}

unsigned int player_numeventdrivers(Player* self)
{
	return eventdrivers_size(&self->eventdrivers);
}

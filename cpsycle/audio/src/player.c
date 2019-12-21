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
#include <windows.h>
#include <portable.h>

#if !defined(FALSE)
#define FALSE 0
#endif

#if !defined(TRUE)
#define TRUE 1
#endif

static psy_dsp_amp_t bufferdriver[65535];
static void* mainframe;

static void player_initdriver(Player*);
static void player_initkbddriver(Player*);
static void player_initrms(Player*);
static void player_initsignals(Player*);
static void player_disposerms(Player*);
static void player_unloaddriver(Player*);
static void player_unloadeventdrivers(Player*);
static psy_dsp_amp_t* work(Player*, int* numsamples, int* stop);
static void player_workamount(Player*, uintptr_t amount,
	uintptr_t* numsamplex, psy_dsp_amp_t** psamples);
static void player_eventdriverinput(Player*, EventDriver* sender);
static void workeventinput(Player*, int cmd, unsigned char* data, unsigned int size);
static void player_workpath(Player*, uintptr_t amount);
static void player_filldriver(Player*, psy_dsp_amp_t* buffer, uintptr_t amount);
static RMSVol* player_rmsvol(Player*, size_t slot);
static void player_resetvumeters(Player*);
static void player_dostop(Player*);
static void notifylinetick(Player*);

// player init and dispose

void player_init(Player* self, Song* song, void* handle)
{			
	self->song = song;	
	self->numsongtracks = 16;
	self->recordingnotes = 0;
	self->multichannelaudition = 0;
	sequencer_init(&self->sequencer, &song->sequence, &song->machines);
	mainframe = handle;
	player_initdriver(self);	
	eventdrivers_init(&self->eventdrivers, handle);
	psy_signal_connect(&self->eventdrivers.signal_input, self,
		player_eventdriverinput);
	player_initsignals(self);
	player_initrms(self);
	table_init(&self->notestotracks);
	table_init(&self->trackstonotes);
}

void player_initdriver(Player* self)
{	
	self->driver = 0;	
	library_init(&self->drivermodule);
	player_loaddriver(self, 0, 0);		
}

void player_initsignals(Player* self)
{
	psy_signal_init(&self->signal_numsongtrackschanged);
	psy_signal_init(&self->signal_lpbchanged);
	psy_signal_init(&self->signal_inputevent);
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
	psy_signal_dispose(&self->signal_numsongtrackschanged);
	psy_signal_dispose(&self->signal_lpbchanged);
	psy_signal_dispose(&self->signal_inputevent);	
	sequencer_dispose(&self->sequencer);		
	player_disposerms(self);
	table_dispose(&self->notestotracks);
	table_dispose(&self->trackstonotes);
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

psy_dsp_amp_t* work(Player* self, int* numsamples, int* hostisplaying)
{	
	uintptr_t maxamount;
	uintptr_t amount;
	uintptr_t numsamplex;
	psy_dsp_amp_t* psamples;
	
	psamples = bufferdriver;
	numsamplex = *numsamples;
	maxamount = numsamplex > MAX_STREAM_SIZE ? MAX_STREAM_SIZE : numsamplex;
	lock_enter();
	do {		
		amount = maxamount;
		if (amount > numsamplex) {
			amount = numsamplex;
		}
		if (self->sequencer.playing) {
			self->sequencer.linetickcount -= sequencer_frametooffset(&self->sequencer, amount);
			if (self->sequencer.linetickcount <= 0) {				
				amount = sequencer_frames(&self->sequencer, -self->sequencer.linetickcount);
			}
		}
		if (amount > 0) {
			player_workamount(self, amount, &numsamplex, &psamples);
		}
		if (self->sequencer.linetickcount <= 0) {
			notifylinetick(self);
			self->sequencer.linetickcount = 
				1 / (self->sequencer.lpb * self->sequencer.lpbspeed);
			sequencer_linetick(&self->sequencer);
		}
	} while (numsamplex > 0);
	lock_leave();
	*hostisplaying = sequencer_playing(&self->sequencer);	
	return bufferdriver;
}

void notifylinetick(Player* self)
{
	if (self->song) {
		TableIterator it;
		
		for (it = machines_begin(&self->song->machines); 
			!tableiterator_equal(&it, table_end());		
				tableiterator_inc(&it)) {			
			Machine* machine;

			machine = (Machine*)tableiterator_value(&it);					
			machine->vtable->sequencerlinetick(machine);				
		}
	}
}

void player_workamount(Player* self, uintptr_t amount, uintptr_t* numsamplex,
					   psy_dsp_amp_t** psamples)
{
	sequencer_frametick(&self->sequencer, amount);
	player_workpath(self, amount);		
	player_filldriver(self, *psamples, amount);
	*numsamplex -= amount;
	*psamples  += (2*amount);
}

void player_workpath(Player* self, uintptr_t amount)
{
	MachinePath* path;
	path = machines_path(&self->song->machines);
	if (path) {
		for ( ; path != 0; path = path->next) {
			size_t slot;									
			Machine* machine;

			slot = (size_t)path->entry;
			machine = machines_at(&self->song->machines, slot);
			if (machine && !table_exists(&self->song->machines.connections.sends, slot)) {
				Buffer* output;

				output = machine->vtable->mix(machine, slot, amount,
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
					machine->vtable->work(machine, &bc);
					buffer_pan(output, machine->vtable->panning(machine), amount);
					if (self->vumode == VUMETER_RMS && buffer_numchannels(
							bc.output) >= 2) {
						rmsvol_tick(rms, bc.output->samples[0], bc.output->samples[1],
							bc.numsamples);
					}
					psy_signal_emit(&machine->signal_worked, machine, 2,
						slot, &bc);
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

void player_filldriver(Player* self, psy_dsp_amp_t* buffer, uintptr_t amount)
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
			psy_signal_emit(&master->signal_worked, master, 2,
				MASTER_INDEX, &bc);
		}	
		dsp.interleave(buffer, masteroutput->samples[0],
			masteroutput->samples[1], amount);
	}
}

RMSVol* player_rmsvol(Player* self, size_t slot)
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

void player_eventdriverinput(Player* self, EventDriver* sender)
{
	Properties* notes;
	int cmd;	
	
	notes = properties_find(self->eventdrivers.cmds, "notes");
	cmd = sender->getcmd(sender, notes);
	if (cmd != -1 && cmd <  255) {		
		int base = 48;
		unsigned char note;
		uintptr_t track = 0;
		PatternEvent event;

		if (cmd < NOTECOMMANDS_RELEASE) {
			note = (unsigned char) cmd + base;
		} else {
			note = cmd;
		}		
		patternevent_init(&event, note, 255, 
			(unsigned char) (
			self->song
				? machines_slot(&self->song->machines)
				: 0),
			0, 0);
		if (self->multichannelaudition) {
			if (event.note < NOTECOMMANDS_RELEASE) {
				if (table_exists(&self->notestotracks, event.note)) {
					track = (uintptr_t) table_at(&self->notestotracks, event.note);
				} else {							
					while (table_exists(&self->trackstonotes, track)) {
						++track;
					}
					table_insert(&self->notestotracks, event.note, (void*)track);
					table_insert(&self->trackstonotes, track,
						(void*)(uintptr_t)event.note);
				}
			} else
			if (event.note == NOTECOMMANDS_RELEASE) {				
				if (table_exists(&self->notestotracks, note)) {
					track = (uintptr_t) table_at(&self->notestotracks, note);
					table_remove(&self->notestotracks, note);
					table_remove(&self->trackstonotes, track);
				}
			}
		}
		sequencer_addinputevent(&self->sequencer, &event, track);
		if (self->recordingnotes && sequencer_playing(&self->sequencer)) {
			sequencer_recordinputevent(&self->sequencer, &event, 0, 
				player_position(self));			
		} else {			
			psy_signal_emit(&self->signal_inputevent, self, 1, &event);
		}
	}
}

void workeventinput(Player* self, int cmd, unsigned char* data, unsigned int size)
{	
/*	int validevent = 0;
	uintptr_t note = 0;
	PatternEvent event;
	
	switch (cmd) {
		// MIDI DATA
		case 1:
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
					note = data[1];
				default:
				break;
			}
		}
		break;
		// Pattern Data
		case 2:
			patternevent_init(&event, data[0], 255, 0, 0, 0);
			note = data[1];
			validevent = 1;
		break;
		default:		
		break;
	}
	if (validevent) {
		uintptr_t track = 0;

		if (self->multichannelaudition) {
			if (event.note < NOTECOMMANDS_RELEASE) {
				if (table_exists(&self->notestotracks, event.note)) {
					track = (uintptr_t) table_at(&self->notestotracks, event.note);
				} else {							
					while (table_exists(&self->trackstonotes, track)) {
						++track;
					}
					table_insert(&self->notestotracks, event.note, (void*)track);
					table_insert(&self->trackstonotes, track,
						(void*)(uintptr_t)event.note);
				}
			} else
			if (event.note == NOTECOMMANDS_RELEASE) {				
				if (table_exists(&self->notestotracks, note)) {
					track = (uintptr_t) table_at(&self->notestotracks, note);
					table_remove(&self->notestotracks, note);
					table_remove(&self->trackstonotes, track);
				}
			}
		}
		sequencer_addinputevent(&self->sequencer, &event, track);
		if (self->recordingnotes && sequencer_playing(&self->sequencer)) {
			sequencer_recordinputevent(&self->sequencer, &event, 0, 
				player_position(self));			
		} else {			
			signal_emit(&self->signal_inputevent, self, 1, &event);
		}		
	}*/
}

// general setter and getter

void player_setsong(Player* self, Song* song)
{
	self->song = song;
	sequencer_reset(&self->sequencer, &song->sequence, &song->machines);
	sequencer_setsamplerate(&self->sequencer,
		self->driver->samplerate(self->driver));	
}

void player_setnumsongtracks(Player* self, uintptr_t numsongtracks)
{
	if (numsongtracks >= 1 && numsongtracks <= 64) {
		self->numsongtracks = numsongtracks;	
		psy_signal_emit(&self->signal_numsongtrackschanged, self, 1,
			self->numsongtracks);
	}
}

uintptr_t player_numsongtracks(Player* self)
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
	sequencer_start(&self->sequencer);	
}

void player_stop(Player* self)
{
	sequencer_stop(&self->sequencer);
	player_dostop(self);
}

void player_dostop(Player* self)
{
	if (self->song) {
		TableIterator it;
		
		for (it = machines_begin(&self->song->machines);
				!tableiterator_equal(&it, table_end());
				tableiterator_inc(&it)) {
			Machine* machine;

			machine = (Machine*)tableiterator_value(&it);
			machine->vtable->stop(machine);			
		}
	}
}

int player_playing(Player* self)
{
	return sequencer_playing(&self->sequencer);
}

void player_setposition(Player* self, psy_dsp_beat_t offset)
{
	sequencer_setposition(&self->sequencer, offset);	
}

psy_dsp_beat_t player_position(Player* self)
{
	return sequencer_position(&self->sequencer);
}

void player_setbpm(Player* self, psy_dsp_beat_t bpm)
{
	sequencer_setbpm(&self->sequencer, bpm);	
}

psy_dsp_beat_t player_bpm(Player* self)
{
	return sequencer_bpm(&self->sequencer);
}

void player_setlpb(Player* self, uintptr_t lpb)
{
	sequencer_setlpb(&self->sequencer, lpb);
	psy_signal_emit(&self->signal_lpbchanged, self, 1, lpb);
}

uintptr_t player_lpb(Player* self)
{
	return sequencer_lpb(&self->sequencer);
}

// Driver set, get, load, unload, restart, ..., methods

void player_setaudiodriver(Player* self, Driver* driver)
{
	self->driver = driver;
	driver->connect(driver, self, work, mainframe);
}

Driver* player_audiodriver(Player* self)
{
	return self->driver;
}

void player_loaddriver(Player* self, const char* path, Properties* config)
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
	if (self->driver && config) {
		self->driver->configure(self->driver, config);
		//self->driver->properties = properties_clone(config);
	}
	self->driver->open(self->driver);
}

void player_unloaddriver(Player* self)
{
	if (self->driver) {
		self->driver->close(self->driver);		
		self->driver->dispose(self->driver);
		self->driver->free(self->driver);
//		free(self->driver);
		library_unload(&self->drivermodule);		
	}
	self->driver = 0;
}

void player_reloaddriver(Player* self, const char* path, Properties* config)
{		
	player_unloaddriver(self);
	player_loaddriver(self, path, config);		
}

void player_restartdriver(Player* self, Properties* config)
{	
	if (self->driver) {
		self->driver->close(self->driver);	
		self->driver->configure(self->driver, config);
		self->driver->open(self->driver);
	}
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


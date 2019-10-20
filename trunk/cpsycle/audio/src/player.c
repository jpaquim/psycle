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
static void player_initeventdriver(Player*);
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
static EventDriverEntry* player_eventdriverentry(Player*, int id);

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
	self->eventdrivers = 0;	
	self->kbddriver = 0;
	player_initkbddriver(self);	
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
	player_unloadeventdrivers(self);
	lock_dispose();	
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

void player_loadeventdriver(Player* self, const char* path)
{
	EventDriver* eventdriver = 0;	
	
	if (path) {
		if (strcmp(path, "kbd") == 0) {
			if (!self->kbddriver) {
				player_initkbddriver(self);
			}
		} else {
			Library* library;

			library = library_allocinit();			
			library_load(library, path);
			if (library->module) {
				pfneventdriver_create fpeventdrivercreate;

				fpeventdrivercreate = (pfneventdriver_create)
					library_functionpointer(library, "eventdriver_create");
				if (fpeventdrivercreate) {
					EventDriverEntry* eventdriverentry;

					eventdriver = fpeventdrivercreate();					
					eventdriver->connect(eventdriver, self, workeventinput, mainframe);
					eventdriverentry = (EventDriverEntry*) malloc(sizeof(EventDriverEntry));
					eventdriverentry->eventdriver = eventdriver;
					eventdriverentry->library = library;
					list_append(&self->eventdrivers, eventdriverentry);				
					eventdriver->open(eventdriver);
				}
			} else {
				library_dispose(library);
				free(library);
			}
		}
	}
}

void player_initkbddriver(Player* self)
{
	EventDriver* eventdriver;
	EventDriverEntry* eventdriverentry;

	eventdriver = create_kbd_driver();	
	self->kbddriver = eventdriver;
	eventdriver->connect(eventdriver, self, workeventinput, mainframe);

	eventdriverentry = (EventDriverEntry*) malloc(sizeof(EventDriverEntry));
	eventdriverentry->eventdriver = eventdriver;
	eventdriverentry->library = 0;
	list_append(&self->eventdrivers, eventdriverentry);
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

void player_restarteventdriver(Player* self, int id)
{	
	EventDriver* eventdriver;

	eventdriver = player_eventdriver(self, id);
	if (eventdriver) {
		eventdriver->close(eventdriver);	
		eventdriver->updateconfiguration(eventdriver);
		eventdriver->open(eventdriver);	
	}
}

void player_removeeventdriver(Player * self, int id)
{
	EventDriver* eventdriver;

	eventdriver = player_eventdriver(self, id);	
	if (eventdriver) {
		EventDriverEntry* eventdriverentry;
		List* p;

		eventdriver->close(eventdriver);
		eventdriver->dispose(eventdriver);
		eventdriver->free(eventdriver);
		if (eventdriver == self->kbddriver) {
			self->kbddriver = 0;
		}
		eventdriverentry = player_eventdriverentry(self, id);
		if (eventdriverentry && eventdriverentry->library) {
			library_unload(eventdriverentry->library);
			library_dispose(eventdriverentry->library);
			free(eventdriverentry->library);
			eventdriverentry->library = 0;
		}		
		for (p = self->eventdrivers; p != 0; p = p->next) {
			if (((EventDriverEntry*)p->entry)->eventdriver == eventdriver) {
				list_remove(&self->eventdrivers, p);
				break;
			}
		}
		free(eventdriverentry);
	}
}

void player_unloadeventdrivers(Player* self)
{
	List* p;	

	for (p = self->eventdrivers; p != 0; p = p->next) {
		EventDriverEntry* eventdriverentry;
		EventDriver* eventdriver;
		
		eventdriverentry = (EventDriverEntry*)p->entry;
		eventdriver = eventdriverentry->eventdriver;
		eventdriver->close(eventdriver);
		eventdriver->dispose(eventdriver);
		eventdriver->free(eventdriver);
		if (eventdriverentry && eventdriverentry->library) {
			library_unload(eventdriverentry->library);
			library_dispose(eventdriverentry->library);
			free(eventdriverentry->library);
		}
		free(eventdriverentry);
	}
	list_free(self->eventdrivers);
	self->eventdrivers = 0;
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
			if (machine) {			
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

void workeventinput(Player* self, int cmd, unsigned char* data, unsigned int size)
{	
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
				signal_emit(&self->signal_inputevent, self, 1, &event);
			}
			default:
			break;			
		}
	} else
	if (cmd == 2) { 
		PatternEvent event;

		event.note = data[0];
		event.inst = 255;
		event.mach = 0;
		event.cmd = 0;
		event.parameter = 0;

		sequencer_addinputevent(&self->sequencer, &event, 0);
		signal_emit(&self->signal_inputevent, self, 1, &event);
	}	
}

EventDriver* player_kbddriver(Player* self)
{
	return self->kbddriver;
}

EventDriver* player_eventdriver(Player* self, int id) 
{
	List* p;
	int c = 0;

	for (p = self->eventdrivers; p != 0 && id != c; p = p->next, ++c);
	
	return p ? ((EventDriverEntry*) (p->entry))->eventdriver : 0;
}

unsigned int player_numeventdrivers(Player* self)
{
	int rv = 0;
	List* p;
	
	for (p = self->eventdrivers; p != 0; p = p->next, ++rv);
	return rv;
}

EventDriverEntry* player_eventdriverentry(Player* self, int id)
{
	List* p;
	int c = 0;

	for (p = self->eventdrivers; p != 0 && id != c; p = p->next, ++c);
	
	return p ? ((EventDriverEntry*) (p->entry)) : 0;
}

// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

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

static void player_initdriver(psy_audio_Player*);
static void player_initkbddriver(psy_audio_Player*);
static void player_initrms(psy_audio_Player*);
static void player_initsignals(psy_audio_Player*);
static void player_disposerms(psy_audio_Player*);
static void player_unloaddriver(psy_audio_Player*);
static void player_unloadeventdrivers(psy_audio_Player*);
static psy_dsp_amp_t* work(psy_audio_Player*, int* numsamples, int* stop);
static void player_workamount(psy_audio_Player*, uintptr_t amount,
	uintptr_t* numsamplex, psy_dsp_amp_t** psamples);
static void player_oneventdriverinput(psy_audio_Player*, psy_EventDriver* sender);
static void workeventinput(psy_audio_Player*, int cmd, unsigned char* data, unsigned int size);
static void player_workpath(psy_audio_Player*, uintptr_t amount);
static void player_filldriver(psy_audio_Player*, psy_dsp_amp_t* buffer, uintptr_t amount);
static psy_dsp_RMSVol* player_rmsvol(psy_audio_Player*, size_t slot);
static void player_resetvumeters(psy_audio_Player*);
static void player_dostop(psy_audio_Player*);
static void notifylinetick(psy_audio_Player*);

// player init and dispose
void player_init(psy_audio_Player* self, psy_audio_Song* song, void* handle)
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
		player_oneventdriverinput);
	player_initsignals(self);
	player_initrms(self);
	psy_table_init(&self->notestotracks);
	psy_table_init(&self->trackstonotes);
	pattern_init(&self->patterndefaults);
	pattern_setlength(&self->patterndefaults, (psy_dsp_beat_t) 0.25f);
}

void player_initdriver(psy_audio_Player* self)
{	
	self->driver = 0;	
	psy_library_init(&self->drivermodule);
	player_loaddriver(self, 0, 0);		
}

void player_initsignals(psy_audio_Player* self)
{
	psy_signal_init(&self->signal_numsongtrackschanged);
	psy_signal_init(&self->signal_lpbchanged);
	psy_signal_init(&self->signal_inputevent);
}

void player_initrms(psy_audio_Player* self)
{
	self->vumode = VUMETER_RMS;
	self->resetvumeters = 0;
	psy_table_init(&self->rms);
}

void player_dispose(psy_audio_Player* self)
{			
	player_unloaddriver(self);
	psy_library_dispose(&self->drivermodule);
	eventdrivers_dispose(&self->eventdrivers);	
	psy_signal_dispose(&self->signal_numsongtrackschanged);
	psy_signal_dispose(&self->signal_lpbchanged);
	psy_signal_dispose(&self->signal_inputevent);	
	sequencer_dispose(&self->sequencer);		
	player_disposerms(self);
	psy_table_dispose(&self->notestotracks);
	psy_table_dispose(&self->trackstonotes);
	pattern_dispose(&self->patterndefaults);
}

void player_disposerms(psy_audio_Player* self)
{
	psy_TableIterator it;

	for (it = psy_table_begin(&self->rms); !psy_tableiterator_equal(&it, psy_table_end());
			psy_tableiterator_inc(&it)) {
		free(psy_tableiterator_value(&it));
	}
	psy_table_dispose(&self->rms);
}

// driver callbacks

// sound driver callback
psy_dsp_amp_t* work(psy_audio_Player* self, int* numsamples, int* hostisplaying)
{	
	uintptr_t maxamount;
	uintptr_t amount;
	uintptr_t numsamplex;
	psy_dsp_amp_t* psamples;
	
	psamples = bufferdriver;
	numsamplex = *numsamples;
	maxamount = numsamplex > MAX_STREAM_SIZE ? MAX_STREAM_SIZE : numsamplex;
	psy_audio_lock_enter();
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
	psy_audio_lock_leave();
	*hostisplaying = sequencer_playing(&self->sequencer);	
	return bufferdriver;
}

void notifylinetick(psy_audio_Player* self)
{
	if (self->song) {
		psy_TableIterator it;
		
		for (it = machines_begin(&self->song->machines); 
			!psy_tableiterator_equal(&it, psy_table_end());		
				psy_tableiterator_inc(&it)) {			
			psy_audio_Machine* machine;

			machine = (psy_audio_Machine*)psy_tableiterator_value(&it);					
			machine->vtable->sequencerlinetick(machine);				
		}
	}
}

void player_workamount(psy_audio_Player* self, uintptr_t amount, uintptr_t* numsamplex,
					   psy_dsp_amp_t** psamples)
{
	sequencer_frametick(&self->sequencer, amount);
	player_workpath(self, amount);		
	player_filldriver(self, *psamples, amount);
	*numsamplex -= amount;
	*psamples  += (2*amount);
}

void player_workpath(psy_audio_Player* self, uintptr_t amount)
{
	MachinePath* path;
	path = machines_path(&self->song->machines);
	if (path) {
		for ( ; path != 0; path = path->next) {
			size_t slot;									
			psy_audio_Machine* machine;

			slot = (size_t)path->entry;
			machine = machines_at(&self->song->machines, slot);
			if (machine && !psy_table_exists(&self->song->machines.connections.sends, slot)) {
				psy_audio_Buffer* output;

				output = machine_mix(machine, slot, amount,
					connections_at(&self->song->machines.connections, slot),
					&self->song->machines);
				if (output && slot != MASTER_INDEX) {				
					psy_audio_BufferContext bc;										
					psy_List* events;
					psy_dsp_RMSVol* rms;

					events = sequencer_timedevents(&self->sequencer, slot, amount);
					rms = player_rmsvol(self, slot);					
					psy_audio_buffercontext_init(&bc, events, output, output, amount,
						self->numsongtracks, rms);
					psy_audio_buffer_scale(output, machine_amprange(machine), amount);
					machine_work(machine, &bc);
					psy_audio_buffer_pan(output, machine->vtable->panning(machine), amount);
					if (self->vumode == VUMETER_RMS && psy_audio_buffer_numchannels(
							bc.output) >= 2) {
						psy_dsp_rmsvol_tick(rms, bc.output->samples[0],
							bc.output->samples[1],
							bc.numsamples);
					}
					psy_signal_emit(&machine->signal_worked, machine, 2,
						slot, &bc);
					psy_list_free(events);										
				}
			}			
		}							
	}
	if (self->resetvumeters) {
		player_resetvumeters(self);
	}
}

void player_resetvumeters(psy_audio_Player* self)
{	
	psy_table_dispose(&self->rms);
	psy_table_init(&self->rms);
	self->resetvumeters = 0;	
}

void player_filldriver(psy_audio_Player* self, psy_dsp_amp_t* buffer, uintptr_t amount)
{
	psy_audio_Buffer* masteroutput;	
	masteroutput = machines_outputs(&self->song->machines, MASTER_INDEX);
	if (masteroutput) {		
		psy_audio_Machine* master;

		master = machines_master(&self->song->machines);
		if (master) {
			psy_audio_BufferContext bc;
			psy_dsp_RMSVol* rms;

			rms = player_rmsvol(self, MASTER_INDEX);
			psy_audio_buffercontext_init(&bc, 0, masteroutput, masteroutput, amount,
				self->numsongtracks, rms);
			psy_audio_buffer_mulsamples(masteroutput, amount,
				machines_volume(&self->song->machines));			
			if (self->vumode == VUMETER_RMS) {
				psy_dsp_rmsvol_tick(rms, masteroutput->samples[0],
					masteroutput->samples[1],
					amount);		
			}
			psy_signal_emit(&master->signal_worked, master, 2,
				MASTER_INDEX, &bc);
		}
		psy_audio_buffer_scale(masteroutput, PSY_DSP_AMP_RANGE_NATIVE, amount);
		dsp.interleave(buffer, masteroutput->samples[0],
			masteroutput->samples[1], amount);
	}
}

psy_dsp_RMSVol* player_rmsvol(psy_audio_Player* self, size_t slot)
{
	psy_dsp_RMSVol* rv;

	if (!psy_table_exists(&self->rms, slot)) {
		rv = psy_dsp_rmsvol_allocinit();		
		psy_table_insert(&self->rms, slot, rv);
	} else {
		rv = (psy_dsp_RMSVol*) psy_table_at(&self->rms, slot);
	}
	return rv;
}

// event driver callback
void player_oneventdriverinput(psy_audio_Player* self, psy_EventDriver* sender)
{
	psy_Properties* notes;
	EventDriverCmd cmd;	
	
	notes = psy_properties_find(self->eventdrivers.cmds, "notes");
	cmd = sender->getcmd(sender, notes);
	if (cmd.id != -1 && cmd.data.param1 < 255) {		
		unsigned char note;
		psy_audio_Machine* machine;

		uintptr_t track = 0;
		psy_audio_PatternEvent event;

		if (cmd.id < NOTECOMMANDS_RELEASE) {
			note = (unsigned char) cmd.data.param1;
		} else {
			note = cmd.data.param1;
		}
		machine = machines_at(&self->song->machines,
			machines_slot(&self->song->machines));
		patternevent_init_all(&event,
			note,
			(uint16_t) (machine && machine_supports(machine,
					MACH_SUPPORTS_INSTRUMENTS)
				? instruments_slot(&self->song->instruments)
				: NOTECOMMANDS_INST_EMPTY),
			(uint8_t) (self->song
				? machines_slot(&self->song->machines)
				: 0),
			(uint8_t) NOTECOMMANDS_VOL_EMPTY,
			0, 0);		
		if (self->multichannelaudition) {
			if (event.note < NOTECOMMANDS_RELEASE) {
				if (psy_table_exists(&self->notestotracks, event.note)) {
					track = (uintptr_t) psy_table_at(&self->notestotracks,
						event.note);
				} else {							
					while (psy_table_exists(&self->trackstonotes, track)) {
						++track;
					}
					psy_table_insert(&self->notestotracks, event.note,
						(void*) track);
					psy_table_insert(&self->trackstonotes, track,
						(void*)(uintptr_t) event.note);
				}
			} else
			if (event.note == NOTECOMMANDS_RELEASE) {				
				if (psy_table_exists(&self->notestotracks, note)) {
					track = (uintptr_t) psy_table_at(&self->notestotracks,
						note);
					psy_table_remove(&self->notestotracks, note);
					psy_table_remove(&self->trackstonotes, track);
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

void workeventinput(psy_audio_Player* self, int cmd, unsigned char* data,
	unsigned int size)
{	
/*	int validevent = 0;
	uintptr_t note = 0;
	psy_audio_PatternEvent event;
	
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
		// psy_audio_Pattern Data
		case 2:
			patternevent_init_all(&event, data[0], 255, 0, 255, 0, 0);
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
				if (psy_table_exists(&self->notestotracks, event.note)) {
					track = (uintptr_t) psy_table_at(&self->notestotracks, event.note);
				} else {							
					while (psy_table_exists(&self->trackstonotes, track)) {
						++track;
					}
					psy_table_insert(&self->notestotracks, event.note, (void*)track);
					psy_table_insert(&self->trackstonotes, track,
						(void*)(uintptr_t)event.note);
				}
			} else
			if (event.note == NOTECOMMANDS_RELEASE) {				
				if (psy_table_exists(&self->notestotracks, note)) {
					track = (uintptr_t) psy_table_at(&self->notestotracks, note);
					psy_table_remove(&self->notestotracks, note);
					psy_table_remove(&self->trackstonotes, track);
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
void player_setsong(psy_audio_Player* self, psy_audio_Song* song)
{
	self->song = song;
	if (self->song) {
		sequencer_reset(&self->sequencer, &song->sequence, &song->machines);
		sequencer_setsamplerate(&self->sequencer,
			self->driver->samplerate(self->driver));
		player_setnumsongtracks(self, patterns_songtracks(&song->patterns));
	}
}

void player_setnumsongtracks(psy_audio_Player* self, uintptr_t numsongtracks)
{
	if (numsongtracks >= 1 && numsongtracks <= 64) {
		self->numsongtracks = numsongtracks;	
		psy_signal_emit(&self->signal_numsongtrackschanged, self, 1,
			self->numsongtracks);
	}
}

uintptr_t player_numsongtracks(psy_audio_Player* self)
{
	return self->numsongtracks;
}

void player_setvumetermode(psy_audio_Player* self, VUMeterMode mode)
{
	self->vumode = mode;
	self->resetvumeters = 1;
}

VUMeterMode player_vumetermode(psy_audio_Player* self)
{
	return self->vumode;
}

// sequencer setter and getter
void player_start(psy_audio_Player* self)
{		
	sequencer_start(&self->sequencer);	
}

void player_stop(psy_audio_Player* self)
{
	sequencer_stop(&self->sequencer);
	player_dostop(self);
}

void player_dostop(psy_audio_Player* self)
{
	if (self->song) {
		psy_TableIterator it;
		
		for (it = machines_begin(&self->song->machines);
				!psy_tableiterator_equal(&it, psy_table_end());
				psy_tableiterator_inc(&it)) {
			psy_audio_Machine* machine;

			machine = (psy_audio_Machine*)psy_tableiterator_value(&it);
			machine->vtable->stop(machine);			
		}
	}
}

int player_playing(psy_audio_Player* self)
{
	return sequencer_playing(&self->sequencer);
}

void player_setposition(psy_audio_Player* self, psy_dsp_beat_t offset)
{
	sequencer_setposition(&self->sequencer, offset);	
}

psy_dsp_beat_t player_position(psy_audio_Player* self)
{
	return sequencer_position(&self->sequencer);
}

void player_setbpm(psy_audio_Player* self, psy_dsp_beat_t bpm)
{
	sequencer_setbpm(&self->sequencer, bpm);	
}

psy_dsp_beat_t player_bpm(psy_audio_Player* self)
{
	return sequencer_bpm(&self->sequencer);
}

void player_setlpb(psy_audio_Player* self, uintptr_t lpb)
{
	sequencer_setlpb(&self->sequencer, lpb);
	psy_signal_emit(&self->signal_lpbchanged, self, 1, lpb);
}

uintptr_t player_lpb(psy_audio_Player* self)
{
	return sequencer_lpb(&self->sequencer);
}

// psy_AudioDriver set, get, load, unload, restart, ..., methods
void player_setaudiodriver(psy_audio_Player* self, psy_AudioDriver* driver)
{
	self->driver = driver;
	driver->connect(driver, self, work, mainframe);
}

psy_AudioDriver* player_audiodriver(psy_audio_Player* self)
{
	return self->driver;
}

void player_loaddriver(psy_audio_Player* self, const char* path, psy_Properties* config)
{
	psy_AudioDriver* driver = 0;
	
	player_unloaddriver(self);	
	if (path) {
		psy_library_load(&self->drivermodule, path);	
		if (self->drivermodule.module) {
			pfndriver_create fpdrivercreate;

			fpdrivercreate = (pfndriver_create)
				psy_library_functionpointer(&self->drivermodule, "driver_create");
			if (fpdrivercreate) {
				driver = fpdrivercreate();				
				driver->connect(driver, self, work, mainframe);
			}
			psy_audio_lock_enable();
		}		
	}
	if (!driver) {
		driver = psy_audio_create_silent_driver();
		psy_audio_lock_disable();
	}	
	sequencer_setsamplerate(&self->sequencer, driver->samplerate(driver));
	psy_dsp_rmsvol_setsamplerate(driver->samplerate(driver));
	psy_dsp_multifilter_inittables(driver->samplerate(driver));
	self->driver = driver;
	if (self->driver && config) {
		self->driver->configure(self->driver, config);
		//self->driver->properties = psy_properties_clone(config);
	}
	self->driver->open(self->driver);
}

void player_unloaddriver(psy_audio_Player* self)
{
	if (self->driver) {
		self->driver->close(self->driver);				
		self->driver->deallocate(self->driver);
		psy_library_unload(&self->drivermodule);		
	}
	self->driver = 0;
}

void player_reloaddriver(psy_audio_Player* self, const char* path, psy_Properties* config)
{		
	player_unloaddriver(self);
	player_loaddriver(self, path, config);		
}

void player_restartdriver(psy_audio_Player* self, psy_Properties* config)
{	
	if (self->driver) {
		self->driver->close(self->driver);	
		self->driver->configure(self->driver, config);
		self->driver->open(self->driver);
	}
}

// Event Recording
void player_startrecordingnotes(psy_audio_Player* self)
{
	self->recordingnotes = 1;
}

void player_stoprecordingnotes(psy_audio_Player* self)
{
	self->recordingnotes = 0;
}

int player_recordingnotes(psy_audio_Player* self)
{
	return self->recordingnotes;
}

// psy_EventDriver load, unload, restart, ..., methods
void player_loadeventdriver(psy_audio_Player* self, const char* path)
{
	eventdrivers_load(&self->eventdrivers, path);
}

void player_restarteventdriver(psy_audio_Player* self, int id)
{
	eventdrivers_restart(&self->eventdrivers, id);
}

void player_removeeventdriver(psy_audio_Player * self, int id)
{
	eventdrivers_remove(&self->eventdrivers, id);
}

psy_EventDriver* player_kbddriver(psy_audio_Player* self)
{
	return self->eventdrivers.kbddriver;
}

psy_EventDriver* player_eventdriver(psy_audio_Player* self, int id) 
{
	return eventdrivers_driver(&self->eventdrivers, id);
}

unsigned int player_numeventdrivers(psy_audio_Player* self)
{
	return eventdrivers_size(&self->eventdrivers);
}


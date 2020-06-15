// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "player.h"
#include "exclusivelock.h"
#include "math.h"
#include "master.h"
#include "silentdriver.h"
#include "kbddriver.h"
#include <rms.h>
#include <operations.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../../detail/portable.h"
#include "fileio.h"
#include "../../detail/psyconf.h"
#include "stdlib.h"
#include "../../detail/trace.h"

static psy_dsp_amp_t bufferdriver[65535];
static void* mainframe;
#ifdef PSYCLE_LOG_WORKEVENTS
static PsyFile logfile;
#endif

static void psy_audio_player_initdriver(psy_audio_Player*);
static void psy_audio_player_initkbddriver(psy_audio_Player*);
static void psy_audio_player_initsignals(psy_audio_Player*);
static void psy_audio_player_unloaddriver(psy_audio_Player*);
static void psy_audio_player_unloadeventdrivers(psy_audio_Player*);
static psy_dsp_amp_t* psy_audio_player_work(psy_audio_Player*, int* numsamples,
	int* stop);
static void psy_audio_player_workamount(psy_audio_Player*, uintptr_t amount,
	uintptr_t* numsamplex, psy_dsp_amp_t** psamples);
static void psy_audio_player_oneventdriverinput(psy_audio_Player*,
	psy_EventDriver* sender);
static void psy_audio_player_workpath(psy_audio_Player*, uintptr_t amount);
static void log_workevents(psy_List* events);
static void psy_audio_player_filldriver(psy_audio_Player*,
	psy_dsp_amp_t* buffer, uintptr_t amount);
static void psy_audio_player_ditherbuffer(psy_audio_Player*,
	psy_audio_Buffer* buffer, uintptr_t amount);
static void psy_audio_player_resetvumeters(psy_audio_Player*);
static void psy_audio_player_dostop(psy_audio_Player*);
static void psy_audio_player_notifylinetick(psy_audio_Player*);

// player init and dispose
void psy_audio_player_init(psy_audio_Player* self, psy_audio_Song* song,
	void* handle)
{			
	machinecallback_initempty(&self->machinecallback);
	psy_audio_machinefactory_init(&self->machinefactory, self->machinecallback,
		NULL);
	psy_audio_song_init(&self->emptysong, &self->machinefactory);
	self->song = song;	
	self->numsongtracks = 16;
	self->recordingnotes = 0;
	self->multichannelaudition = 0;
	self->dodither = FALSE;
	psy_dsp_dither_init(&self->dither);
	psy_audio_sequencer_init(&self->sequencer, &song->sequence,
		&song->machines);
	mainframe = handle;
	psy_audio_player_initdriver(self);
	eventdrivers_init(&self->eventdrivers, handle);
	psy_signal_connect(&self->eventdrivers.signal_input, self,
		psy_audio_player_oneventdriverinput);
	psy_audio_player_initsignals(self);
	self->vumode = VUMETER_RMS;
	psy_table_init(&self->notestotracks);
	psy_table_init(&self->trackstonotes);
	psy_table_init(&self->worked);
	psy_audio_pattern_init(&self->patterndefaults);
	psy_audio_pattern_setlength(&self->patterndefaults, (psy_dsp_beat_t) 0.25f);
#ifdef PSYCLE_LOG_WORKEVENTS
	psyfile_create(&logfile, "C:\\Users\\user\\psycle-workevent-log.txt", 1);
#endif
}

void psy_audio_player_initdriver(psy_audio_Player* self)
{	
	self->driver = 0;	
	psy_library_init(&self->drivermodule);
	psy_audio_player_loaddriver(self, 0, 0);
}

void psy_audio_player_initsignals(psy_audio_Player* self)
{
	psy_signal_init(&self->signal_numsongtrackschanged);
	psy_signal_init(&self->signal_lpbchanged);
	psy_signal_init(&self->signal_inputevent);
}

void psy_audio_player_dispose(psy_audio_Player* self)
{			
	psy_audio_player_unloaddriver(self);
	psy_library_dispose(&self->drivermodule);
	eventdrivers_dispose(&self->eventdrivers);	
	psy_signal_dispose(&self->signal_numsongtrackschanged);
	psy_signal_dispose(&self->signal_lpbchanged);
	psy_signal_dispose(&self->signal_inputevent);	
	psy_audio_sequencer_dispose(&self->sequencer);
	psy_table_dispose(&self->notestotracks);
	psy_table_dispose(&self->trackstonotes);
	psy_table_dispose(&self->worked);
	psy_audio_pattern_dispose(&self->patterndefaults);
	psy_dsp_dither_dispose(&self->dither);
#ifdef PSYCLE_LOG_WORKEVENTS
	psyfile_close(&logfile);
#endif
	psy_audio_song_dispose(&self->emptysong);
}

// driver callbacks

// sound driver callback
// - splits work to psy_audio_MAX_STREAM_SIZE parts or to let work begin on a line tick
// - player_workamount processes each spltted part
// - updates the sequencer line tick count
psy_dsp_amp_t* psy_audio_player_work(psy_audio_Player* self, int* numsamples,
	int* hostisplaying)
{	
	uintptr_t maxamount;
	uintptr_t amount;
	uintptr_t numsamplex;
	psy_dsp_amp_t* samples;
	
	samples = bufferdriver;
	numsamplex = *numsamples;
	maxamount = numsamplex > psy_audio_MAX_STREAM_SIZE ? psy_audio_MAX_STREAM_SIZE : numsamplex;
	psy_audio_exclusivelock_enter();
	do {		
		amount = maxamount;
		if (amount > numsamplex) {
			amount = numsamplex;
		}		
		if (self->sequencer.playing) {
			psy_dsp_beat_t t;			
			
			t = psy_audio_sequencer_frametooffset(&self->sequencer, amount);			
			if (self->sequencer.linetickcount <= t) {
				uintptr_t pre;
				psy_dsp_beat_t diff;

				pre = psy_audio_sequencer_frames(&self->sequencer,
					self->sequencer.linetickcount);
				diff = t - self->sequencer.linetickcount;
				if (pre) {
					psy_audio_player_workamount(self, pre, &numsamplex, &samples);
					amount -= pre;
				}				
				psy_audio_player_notifylinetick(self);
				psy_audio_sequencer_onlinetick(&self->sequencer);										
				self->sequencer.linetickcount -= diff;
			} else {
				self->sequencer.linetickcount -= t;
			}
		}
		if (amount > 0) {
			psy_audio_player_workamount(self, amount, &numsamplex, &samples);
		}		
	} while (numsamplex > 0);
	psy_audio_exclusivelock_leave();
	*hostisplaying = psy_audio_sequencer_playing(&self->sequencer);
	return bufferdriver;
}

void psy_audio_player_workamount(psy_audio_Player* self, uintptr_t amount,
	uintptr_t* numsamplex, psy_dsp_amp_t** psamples)
{
	psy_audio_sequencer_frametick(&self->sequencer, amount);
	if (self->song) {
		psy_audio_player_workpath(self, amount);
	}
	psy_audio_player_filldriver(self, *psamples, amount);
	*numsamplex -= amount;
	*psamples += (amount * 2);
}

// each machine gets notified a new tracker line has started
void psy_audio_player_notifylinetick(psy_audio_Player* self)
{
	if (self->song) {
		psy_TableIterator it;
		
		for (it = psy_audio_machines_begin(&self->song->machines); 
				!psy_tableiterator_equal(&it, psy_table_end());		
				psy_tableiterator_inc(&it)) {			
			psy_audio_machine_newline((psy_audio_Machine*)
				psy_tableiterator_value(&it));
		}
	}
}

void psy_audio_player_workpath(psy_audio_Player* self, uintptr_t amount)
{
	MachinePath* path;

	path = psy_audio_machines_path(&self->song->machines);
	if (path) {		
		for ( ; path != 0; path = path->next) {
			size_t slot;			
			
			slot = (size_t)path->entry;			
			if (slot == UINTPTR_MAX) {
				// delimits the machines that could be processed parallel
				// todo: add thread functions
				continue;				
			}
			if (!psy_audio_machines_ismixersend(&self->song->machines, slot)) {
				psy_audio_player_workmachine(self, amount, slot);
			}
		}		
	}	
}

void psy_audio_player_workmachine(psy_audio_Player* self, uintptr_t amount,
	uintptr_t slot)
{
	psy_audio_Machine* machine;

	machine = psy_audio_machines_at(&self->song->machines, slot);
	if (machine) {
		psy_audio_Buffer* output;

		output = psy_audio_machine_mix(machine, slot, amount,
			connections_at(&self->song->machines.connections, slot),
			&self->song->machines, self);
		if (output) {
			psy_audio_BufferContext bc;
			psy_List* events;

			events = psy_audio_sequencer_timedevents(&self->sequencer,
				slot, amount);
			psy_audio_buffercontext_init(&bc, events, output, output, amount,
				self->numsongtracks);
			psy_audio_buffer_scale(output, psy_audio_machine_amprange(machine),
				amount);
			psy_audio_machine_work(machine, &bc);			
#ifdef PSYCLE_LOG_WORKEVENTS
			log_workevents(events);
#endif
			psy_audio_buffer_pan(output, psy_audio_machine_panning(machine),
				amount);
			psy_audio_machine_updatememory(machine, &bc);
			psy_signal_emit(&machine->signal_worked, machine, 2, slot, &bc);
			psy_list_free(events);			
		}
	}
}

#ifdef PSYCLE_LOG_WORKEVENTS
void log_workevents(psy_List* events)
{
	if (logfile._file) {
		psy_List* p;
		char text[256];
		char temp;
		for (p = events; p != NULL; p = p->next) {
			psy_audio_PatternEntry* entry;			
			
			entry = psy_audio_patternnode_entry(p);
			psy_snprintf(text, 256, "Trk %d \n", (int)entry->track);
			psyfile_write(&logfile, text, strlen(text));
		}		
	}
}
#endif

void psy_audio_player_filldriver(psy_audio_Player* self, psy_dsp_amp_t* buffer,
	uintptr_t amount)
{
	psy_audio_Buffer* masteroutput;	
	masteroutput = psy_audio_machines_outputs(&self->song->machines, psy_audio_MASTER_INDEX);
	if (masteroutput) {		
		psy_audio_buffer_scale(masteroutput, PSY_DSP_AMP_RANGE_NATIVE, amount);
		if (self->dodither) {
			psy_audio_player_ditherbuffer(self, masteroutput, amount);			
		}		
		dsp.interleave(buffer, masteroutput->samples[0],
			masteroutput->samples[1], amount);
	}
}

void psy_audio_player_ditherbuffer(psy_audio_Player* self, psy_audio_Buffer* buffer,
	uintptr_t amount)
{
	uintptr_t channel;

	// dither needs PSY_DSP_AMP_RANGE_NATIVE
	for (channel = 0; channel < psy_audio_buffer_numchannels(buffer);
			++channel) {
		psy_dsp_dither_process(&self->dither,
			psy_audio_buffer_at(buffer, channel), amount);
	}
}

// event driver callback
void psy_audio_player_oneventdriverinput(psy_audio_Player* self,
	psy_EventDriver* sender)
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
		machine = psy_audio_machines_at(&self->song->machines,
			psy_audio_machines_slot(&self->song->machines));
		patternevent_init_all(&event,
			note,
			(note == NOTECOMMANDS_TWEAK)
			? (uint16_t) self->song->machines.tweakparam
			: (uint16_t) (machine && machine_supports(machine,
					MACH_SUPPORTS_INSTRUMENTS)
				? instruments_slot(&self->song->instruments).subslot
				: NOTECOMMANDS_INST_EMPTY),
			(uint8_t) (self->song
				? psy_audio_machines_slot(&self->song->machines)
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
		psy_audio_sequencer_addinputevent(&self->sequencer, &event, track);
		if (self->recordingnotes && psy_audio_sequencer_playing(&
				self->sequencer)) {
			psy_audio_sequencer_recordinputevent(&self->sequencer, &event, 0, 
				psy_audio_player_position(self));
		} else {			
			psy_signal_emit(&self->signal_inputevent, self, 1, &event);
		}
	}
}

// general setter and getter
void psy_audio_player_setsong(psy_audio_Player* self, psy_audio_Song* song)
{
	self->song = song;
	if (self->song) {
		psy_audio_sequencer_reset(&self->sequencer, &song->sequence,
			&song->machines, self->driver->samplerate(self->driver));		
		psy_audio_player_setnumsongtracks(self,
			patterns_songtracks(&song->patterns));
	}
}

void psy_audio_player_setnumsongtracks(psy_audio_Player* self,
	uintptr_t numsongtracks)
{
	if (numsongtracks >= 1 && numsongtracks <= 64) {
		self->numsongtracks = numsongtracks;	
		psy_signal_emit(&self->signal_numsongtrackschanged, self, 1,
			self->numsongtracks);
	}
}

void psy_audio_player_setvumetermode(psy_audio_Player* self, VUMeterMode mode)
{
	self->vumode = mode;
}

VUMeterMode psy_audio_player_vumetermode(psy_audio_Player* self)
{
	return self->vumode;
}

void psy_audio_player_enabledither(psy_audio_Player* self)
{
	self->dodither = TRUE;	
}

void psy_audio_player_disabledither(psy_audio_Player* self)
{
	self->dodither = FALSE;
}

void psy_audio_player_setdither(psy_audio_Player* self, uintptr_t depth,
	psy_dsp_DitherPdf pdf, psy_dsp_DitherNoiseShape noiseshaping)
{
	psy_dsp_dither_setbitdepth(&self->dither, depth);
	psy_dsp_dither_setpdf(&self->dither, pdf);
	psy_dsp_dither_setnoiseshaping(&self->dither, noiseshaping);
}

// sequencer setter and getter
void psy_audio_player_start(psy_audio_Player* self)
{		
	psy_audio_sequencer_start(&self->sequencer);
}

void psy_audio_player_stop(psy_audio_Player* self)
{
	psy_audio_sequencer_stop(&self->sequencer);
	psy_audio_player_dostop(self);
}

void psy_audio_player_dostop(psy_audio_Player* self)
{
	if (self->song) {
		psy_TableIterator it;
		
		for (it = psy_audio_machines_begin(&self->song->machines);
				!psy_tableiterator_equal(&it, psy_table_end());
				psy_tableiterator_inc(&it)) {
			psy_audio_Machine* machine;

			machine = (psy_audio_Machine*)psy_tableiterator_value(&it);
			machine->vtable->stop(machine);			
		}
		psy_audio_player_setbpm(self, self->song->properties.bpm);
		psy_audio_player_setlpb(self, self->song->properties.lpb);
	}
}

void psy_audio_player_setposition(psy_audio_Player* self, psy_dsp_beat_t offset)
{
	psy_audio_sequencer_setposition(&self->sequencer, offset);
}

void psy_audio_player_setlpb(psy_audio_Player* self, uintptr_t lpb)
{
	psy_audio_sequencer_setlpb(&self->sequencer, lpb);
	psy_signal_emit(&self->signal_lpbchanged, self, 1, lpb);
}

// psy_AudioDriver set, get, load, unload, restart, ..., methods
void psy_audio_player_setaudiodriver(psy_audio_Player* self, psy_AudioDriver* driver)
{
	self->driver = driver;
	driver->connect(driver, self, psy_audio_player_work, mainframe);
}

psy_AudioDriver* psy_audio_player_audiodriver(psy_audio_Player* self)
{
	return self->driver;
}

void psy_audio_player_loaddriver(psy_audio_Player* self, const char* path,
	psy_Properties* config)
{
	psy_AudioDriver* driver = 0;
	
	psy_audio_player_unloaddriver(self);
	if (path) {
        printf("driver path: %s\n", path);
		psy_library_load(&self->drivermodule, path);	
		if (self->drivermodule.module) {
			pfndriver_create fpdrivercreate;

			fpdrivercreate = (pfndriver_create)
				psy_library_functionpointer(&self->drivermodule, "driver_create");
			if (fpdrivercreate) {
				driver = fpdrivercreate();				
				driver->connect(driver, self, psy_audio_player_work, mainframe);
			}
			psy_audio_exclusivelock_enable();
		}		
	}
	if (!driver) {
		driver = psy_audio_create_silent_driver();
		psy_audio_exclusivelock_disable();
	}	
	psy_audio_sequencer_setsamplerate(&self->sequencer, driver->samplerate(
		driver));
	psy_dsp_rmsvol_setsamplerate(driver->samplerate(driver));
	self->driver = driver;
	if (self->driver && config) {
		self->driver->configure(self->driver, config);
		self->driver->open(self->driver);
	}
}

void psy_audio_player_unloaddriver(psy_audio_Player* self)
{
	if (self->driver) {
		self->driver->close(self->driver);				
		self->driver->deallocate(self->driver);
		psy_library_unload(&self->drivermodule);		
	}
	self->driver = 0;
}

void psy_audio_player_reloaddriver(psy_audio_Player* self, const char* path,
	psy_Properties* config)
{		
	psy_audio_player_unloaddriver(self);
	psy_audio_player_loaddriver(self, path, config);
}

void psy_audio_player_restartdriver(psy_audio_Player* self,
	psy_Properties* config)
{	
	if (self->driver) {
		self->driver->close(self->driver);	
		self->driver->configure(self->driver, config);
		self->driver->open(self->driver);
	}
}

// Event Recording
void psy_audio_player_startrecordingnotes(psy_audio_Player* self)
{
	self->recordingnotes = 1;
}

void psy_audio_player_stoprecordingnotes(psy_audio_Player* self)
{
	self->recordingnotes = 0;
}

int psy_audio_player_recordingnotes(psy_audio_Player* self)
{
	return self->recordingnotes;
}

// psy_EventDriver load, unload, restart, ..., methods
void psy_audio_player_loadeventdriver(psy_audio_Player* self, const char* path)
{
	eventdrivers_load(&self->eventdrivers, path);
}

void psy_audio_player_restarteventdriver(psy_audio_Player* self, int id)
{
	eventdrivers_restart(&self->eventdrivers, id);
}

void psy_audio_player_removeeventdriver(psy_audio_Player * self, int id)
{
	eventdrivers_remove(&self->eventdrivers, id);
}

psy_EventDriver* psy_audio_player_kbddriver(psy_audio_Player* self)
{
	return self->eventdrivers.kbddriver;
}

psy_EventDriver* psy_audio_player_eventdriver(psy_audio_Player* self, int id)
{
	return eventdrivers_driver(&self->eventdrivers, id);
}

unsigned int psy_audio_player_numeventdrivers(psy_audio_Player* self)
{
	return eventdrivers_size(&self->eventdrivers);
}

void psy_audio_player_setemptysong(psy_audio_Player* self)
{	
	psy_audio_exclusivelock_enter();
	psy_audio_player_setsong(self, &self->emptysong);
	dsp.clear(bufferdriver, 65535);
	psy_audio_exclusivelock_leave();
}

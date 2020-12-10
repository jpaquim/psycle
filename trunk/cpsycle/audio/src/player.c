// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "player.h"
#include "constants.h"
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
#include <time.h>
#include "../../detail/portable.h"
#include "fileio.h"
#include "../../detail/psyconf.h"
#include "stdlib.h"
#include "../../detail/trace.h"
#include "cmdsnotes.h"

static psy_dsp_amp_t bufferdriver[MAX_SAMPLES_WORKFN];
static void* mainframe;
#ifdef PSYCLE_LOG_WORKEVENTS
static PsyFile logfile;
#endif

static uint16_t midi_combinebytes(unsigned char data1, unsigned char data2)
{
	uint16_t rv_14bit;
	rv_14bit = (unsigned short)data2;
	rv_14bit <<= 7;
	rv_14bit |= (unsigned short)data1;
	return rv_14bit;
}

static void psy_audio_player_initdriver(psy_audio_Player*);
static void psy_audio_player_initkbddriver(psy_audio_Player*);
static void psy_audio_player_initsignals(psy_audio_Player*);
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
	assert(self);

	psy_audio_machinefactory_init(&self->machinefactory, NULL, NULL);
	psy_audio_song_init(&self->emptysong, &self->machinefactory);
	self->song = song;	
	self->recordingnotes = FALSE;
	self->recordnoteoff = FALSE;
	self->multichannelaudition = FALSE;
	self->dodither = FALSE;
	self->octave = 4;
	self->resyncplayposinsamples = 0;
	self->resyncplayposinbeats = 0.0;
	self->measure_cpu_usage = FALSE;
	psy_dsp_dither_init(&self->dither);
	psy_audio_sequencer_init(&self->sequencer, &song->sequence,
		&song->machines);
	mainframe = handle;
	psy_audio_midiinput_init(&self->midiinput, song);
	psy_audio_activechannels_init(&self->playon);
	psy_audio_player_initdriver(self);
	psy_audio_eventdrivers_init(&self->eventdrivers, handle);
	psy_signal_connect(&self->eventdrivers.signal_input, self,
		psy_audio_player_oneventdriverinput);
	psy_audio_player_initsignals(self);
	self->vumode = VUMETER_RMS;
	psy_table_init(&self->notestotracks);
	psy_table_init(&self->trackstonotes);
	psy_table_init(&self->worked);
	psy_audio_pattern_init(&self->patterndefaults);
	psy_audio_pattern_setlength(&self->patterndefaults, (psy_dsp_big_beat_t)
		0.25);
#ifdef PSYCLE_LOG_WORKEVENTS
	psyfile_create(&logfile, "C:\\Users\\user\\psycle-workevent-log.txt", 1);
#endif
}

void psy_audio_player_initdriver(psy_audio_Player* self)
{	
	assert(self);

	self->driver = 0;	
	psy_library_init(&self->drivermodule);
	psy_audio_player_loaddriver(self, 0, 0, TRUE);
}

void psy_audio_player_initsignals(psy_audio_Player* self)
{
	assert(self);

	psy_signal_init(&self->signal_numsongtrackschanged);
	psy_signal_init(&self->signal_lpbchanged);
	psy_signal_init(&self->signal_inputevent);
}

void psy_audio_player_dispose(psy_audio_Player* self)
{
	assert(self);

	psy_audio_player_unloaddriver(self);
	psy_library_dispose(&self->drivermodule);
	psy_audio_eventdrivers_dispose(&self->eventdrivers);	
	psy_signal_dispose(&self->signal_numsongtrackschanged);
	psy_signal_dispose(&self->signal_lpbchanged);
	psy_signal_dispose(&self->signal_inputevent);
	psy_audio_activechannels_dispose(&self->playon);
	psy_audio_sequencer_dispose(&self->sequencer);
	psy_table_dispose(&self->notestotracks);
	psy_table_dispose(&self->trackstonotes);
	psy_table_dispose(&self->worked);
	psy_audio_pattern_dispose(&self->patterndefaults);
	psy_dsp_dither_dispose(&self->dither);
#ifdef PSYCLE_LOG_WORKEVENTS
	psyfile_close(&logfile);
#endif
	psy_audio_midiinput_dispose(&self->midiinput);
	psy_audio_song_dispose(&self->emptysong);
	psy_audio_machinefactory_dispose(&self->machinefactory);
}

// driver callbacks

// sound driver callback
// - splits work to psy_audio_MAX_STREAM_SIZE parts or to let work begin on a
//   line tick
// - player_workamount processes each spltted part
// - updates the sequencer line tick count
psy_dsp_amp_t* psy_audio_player_work(psy_audio_Player* self, int* numsamples,
	int* hostisplaying)
{	
	uintptr_t maxamount;
	uintptr_t amount;
	uintptr_t numsamplex;
	psy_dsp_amp_t* samples;
	
	assert(self);

	samples = bufferdriver;
	numsamplex = *numsamples;
	maxamount = (numsamplex > psy_audio_MAX_STREAM_SIZE)
		? psy_audio_MAX_STREAM_SIZE
		: numsamplex;
	psy_audio_exclusivelock_enter();
	self->resyncplayposinsamples = psy_audiodriver_playposinsamples(
		self->driver);
	self->resyncplayposinbeats = psy_audio_player_position(self);
	do {		
		amount = maxamount;
		if (amount > numsamplex) {
			amount = numsamplex;
		}							
		if (self->sequencer.linetickcount <=
				psy_audio_sequencer_frametooffset(&self->sequencer, amount)) {
			if (self->sequencer.linetickcount > 0) {
				uintptr_t pre;

				pre = psy_audio_sequencer_frames(&self->sequencer,
					self->sequencer.linetickcount);
				if (pre) {
					pre--;
					if (pre) {
						psy_audio_player_workamount(self, pre, &numsamplex,
							&samples);
						amount -= pre;
						self->sequencer.linetickcount -=
							psy_audio_sequencer_frametooffset(
								&self->sequencer, pre);
					}
				}
			}
			psy_audio_player_notifylinetick(self);			
			psy_audio_sequencer_onnewline(&self->sequencer);			
		}		
		if (amount > 0) {			
			psy_audio_player_workamount(self, amount, &numsamplex, &samples);
			self->sequencer.linetickcount -=
				psy_audio_sequencer_frametooffset(&self->sequencer, amount);			
		}		
	} while (numsamplex > 0);
	psy_audio_exclusivelock_leave();
	*hostisplaying = psy_audio_sequencer_playing(&self->sequencer);
	return bufferdriver;
}

void psy_audio_player_workamount(psy_audio_Player* self, uintptr_t amount,
	uintptr_t* numsamplex, psy_dsp_amp_t** psamples)
{	
	assert(self);

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
	assert(self);

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

	assert(self);

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

	assert(self);

	machine = psy_audio_machines_at(&self->song->machines, slot);
	if (machine) {
		psy_audio_Buffer* output;

		output = psy_audio_machine_mix(machine, slot, amount,
			psy_audio_connections_at(&self->song->machines.connections, slot),
			&self->song->machines, self);
		if (output) {
			psy_audio_BufferContext bc;
			psy_List* events;
			psy_List* p;
			
			events = psy_audio_sequencer_timedevents(&self->sequencer,
				slot, amount);			
			if (psy_audio_player_playing(self)) {
				// update playon
				for (p = events; p != NULL; psy_list_next(&p)) {
					psy_audio_PatternEntry* patternentry;

					patternentry = (psy_audio_PatternEntry*)psy_list_entry(p);
					psy_audio_activechannels_write(&self->playon,
						patternentry->track,
						psy_audio_patternentry_front(patternentry));
				}
			}
			psy_audio_buffercontext_init(&bc, events, output, output, amount,
				self->sequencer.numsongtracks);
			psy_audio_buffer_scale(output, psy_audio_machine_amprange(machine),
				amount);
			if (self->measure_cpu_usage) {
				psy_audio_cputimeclock_measure(&machine->cpu_time);
			}
			psy_audio_machine_work(machine, &bc);
			psy_audio_buffer_pan(output, psy_audio_machine_panning(machine),
				amount);
			psy_audio_machine_updatememory(machine, &bc);
			psy_signal_emit(&machine->signal_worked, machine, 2, slot, &bc);
			if (self->measure_cpu_usage) {
				psy_audio_cputimeclock_stop(&machine->cpu_time);
				psy_audio_cputimeclock_update(&machine->cpu_time,
					amount, psy_audio_sequencer_samplerate(&self->sequencer));
			}									
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
		for (p = events; p != NULL; psy_list_next(&p)) {
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

	assert(self);

	masteroutput = psy_audio_machines_outputs(&self->song->machines,
		psy_audio_MASTER_INDEX);
	if (masteroutput) {		
		psy_audio_buffer_scale(masteroutput, PSY_DSP_AMP_RANGE_NATIVE, amount);
		if (self->dodither) {
			psy_audio_player_ditherbuffer(self, masteroutput, amount);			
		}		
		dsp.interleave(buffer, masteroutput->samples[0],
			masteroutput->samples[1], amount);
	}
}

void psy_audio_player_ditherbuffer(psy_audio_Player* self, psy_audio_Buffer*
	buffer, uintptr_t amount)
{
	uintptr_t channel;

	assert(self);

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
	psy_EventDriverCmd cmd;	

	assert(self);
	
	cmd = psy_eventdriver_getcmd(sender, "notes");
	if (cmd.id != -1 && cmd.id < 255) {		
		uintptr_t track = 0;
		psy_audio_PatternEvent ev;
				
		if (cmd.id == CMD_NOTE_MIDIEV) {
			psy_audio_patternevent_clear(&ev);
			if (!psy_audio_midiinput_workinput(&self->midiinput, cmd.midi,
				&self->song->machines, &ev)) {
				return;
			}
			if (ev.note >= psy_audio_NOTECOMMANDS_MIDI_SPP &&
					ev.note <= psy_audio_NOTECOMMANDS_MIDI_SYNC) {
				switch (ev.note) {
					case psy_audio_NOTECOMMANDS_MIDI_SPP: {
						uint16_t midibeats;

						midibeats = midi_combinebytes(ev.cmd, ev.parameter);
						psy_audio_sequencer_setposition(&self->sequencer,
							midibeats * 1/16.0);
						break; }
					case psy_audio_NOTECOMMANDS_MIDI_CLK_START:
						psy_audio_sequencer_clockstart(&self->sequencer);
						break;
					case psy_audio_NOTECOMMANDS_MIDI_CLK:
						psy_audio_sequencer_clock(&self->sequencer);
						break;
					case psy_audio_NOTECOMMANDS_MIDI_CLK_CONT:
						psy_audio_sequencer_clockcontinue(&self->sequencer);
						break;
					case psy_audio_NOTECOMMANDS_MIDI_CLK_STOP:
						psy_audio_sequencer_clockstop(&self->sequencer);
						break;
					default:
						break;
				}
				return;
			}
		} else {
			unsigned char note;
			uintptr_t mac = 0;
			psy_audio_Machine* machine;

			mac = psy_audio_machines_slot(&self->song->machines);
			if (cmd.id < psy_audio_NOTECOMMANDS_RELEASE) {
				note = cmd.id + self->octave * 12;
			} else {
				note = cmd.id;
			}
			machine = psy_audio_machines_at(&self->song->machines, mac);
			psy_audio_patternevent_init_all(&ev,
				note,
				(note == psy_audio_NOTECOMMANDS_TWEAK)
				? (uint16_t)self->song->machines.tweakparam
				: (uint16_t)(machine && machine_supports(machine,
					MACH_SUPPORTS_INSTRUMENTS)
					? psy_audio_instruments_selected(&self->song->instruments).subslot
					: psy_audio_NOTECOMMANDS_INST_EMPTY),
				(uint8_t)mac,
				(uint8_t)psy_audio_NOTECOMMANDS_VOL_EMPTY,
				0, 0);
		}		
		if (self->multichannelaudition) {
			if (ev.note < psy_audio_NOTECOMMANDS_RELEASE) {
				if (psy_table_exists(&self->notestotracks, ev.note)) {
					track = (uintptr_t) psy_table_at(&self->notestotracks,
						ev.note);
				} else {							
					while (psy_table_exists(&self->trackstonotes, track)) {
						++track;
					}
					psy_table_insert(&self->notestotracks, ev.note,
						(void*) track);
					psy_table_insert(&self->trackstonotes, track,
						(void*)(uintptr_t)ev.note);
				}
			} else if (ev.note == psy_audio_NOTECOMMANDS_RELEASE) {
				if (psy_table_exists(&self->notestotracks, ev.note)) {
					track = (uintptr_t) psy_table_at(&self->notestotracks,
						ev.note);
					psy_table_remove(&self->notestotracks, ev.note);
					psy_table_remove(&self->trackstonotes, track);
				}
			}
		}
		psy_audio_sequencer_addinputevent(&self->sequencer, &ev, track);
		if (self->recordingnotes && psy_audio_sequencer_playing(&
				self->sequencer)) {
			psy_dsp_big_beat_t offset;

			offset = psy_audio_sequencer_frametooffset(&self->sequencer,
				psy_audiodriver_playposinsamples(self->driver) -
				self->resyncplayposinsamples);
			if (offset < 0) {
				offset = 0;
			}
			if (self->recordnoteoff || ev.note != psy_audio_NOTECOMMANDS_RELEASE) {
				psy_audio_sequencer_recordinputevent(&self->sequencer, &ev, 0,
					self->resyncplayposinbeats);
			}			
		} else {			
			psy_signal_emit(&self->signal_inputevent, self, 1, &ev);
		}
	}
}

// general setter and getter
void psy_audio_player_setsong(psy_audio_Player* self, psy_audio_Song* song)
{
	assert(self);

	self->song = song;
	psy_audio_midiinput_setsong(&self->midiinput, song);
	if (self->song) {		
		psy_audio_sequencer_reset(&self->sequencer, &song->sequence,
			&song->machines, psy_audiodriver_samplerate(self->driver));		
		psy_audio_player_setnumsongtracks(self,
			psy_audio_patterns_songtracks(&song->patterns));
		psy_audio_player_setbpm(self, psy_audio_song_bpm(self->song));
		psy_audio_player_setlpb(self, psy_audio_song_lpb(self->song));
		
	}
}

void psy_audio_player_setnumsongtracks(psy_audio_Player* self,
	uintptr_t numsongtracks)
{
	assert(self);

	if (numsongtracks >= 1 && numsongtracks < MAX_TRACKS) {
		psy_audio_sequencer_setnumsongtracks(&self->sequencer,
			numsongtracks);
		psy_signal_emit(&self->signal_numsongtrackschanged, self, 1,
			numsongtracks);
	}
}

void psy_audio_player_setoctave(psy_audio_Player* self,
	uintptr_t octave)
{
	assert(self);

	if (octave >= 0 && octave < 8) {
		self->octave = octave;		
	}
}

void psy_audio_player_setvumetermode(psy_audio_Player* self, VUMeterMode mode)
{
	assert(self);

	self->vumode = mode;
}

VUMeterMode psy_audio_player_vumetermode(psy_audio_Player* self)
{
	assert(self);

	return self->vumode;
}

void psy_audio_player_enabledither(psy_audio_Player* self)
{
	assert(self);

	self->dodither = TRUE;	
}

void psy_audio_player_disabledither(psy_audio_Player* self)
{
	assert(self);

	self->dodither = FALSE;
}

void psy_audio_player_setdither(psy_audio_Player* self, uintptr_t depth,
	psy_dsp_DitherPdf pdf, psy_dsp_DitherNoiseShape noiseshaping)
{
	assert(self);

	psy_dsp_dither_setbitdepth(&self->dither, depth);
	psy_dsp_dither_setpdf(&self->dither, pdf);
	psy_dsp_dither_setnoiseshaping(&self->dither, noiseshaping);
}

// sequencer setter and getter
void psy_audio_player_start(psy_audio_Player* self)
{
	assert(self);

	psy_audio_activechannels_reset(&self->playon);
	psy_audio_sequencer_start(&self->sequencer);
}

void psy_audio_player_stop(psy_audio_Player* self)
{
	assert(self);
	
	psy_audio_sequencer_stop(&self->sequencer);
	psy_audio_player_dostop(self);
}

void psy_audio_player_dostop(psy_audio_Player* self)
{
	assert(self);

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
		psy_audio_activechannels_reset(&self->playon);
	}
}

void psy_audio_player_setposition(psy_audio_Player* self, psy_dsp_big_beat_t
	offset)
{
	assert(self);

	psy_audio_sequencer_setposition(&self->sequencer, offset);
}

void psy_audio_player_setlpb(psy_audio_Player* self, uintptr_t lpb)
{
	assert(self);

	if (self->song) {
		psy_audio_song_setlpb(self->song, lpb);
	}
	psy_audio_sequencer_setlpb(&self->sequencer, lpb);
	psy_signal_emit(&self->signal_lpbchanged, self, 1, lpb);	
}

// psy_AudioDriver set, get, load, unload, restart, ..., methods
void psy_audio_player_setaudiodriver(psy_audio_Player* self, psy_AudioDriver*
	driver)
{
	assert(self);

	self->driver = driver;
	psy_audiodriver_connect(driver, self, (AUDIODRIVERWORKFN)psy_audio_player_work,
		mainframe);
}

psy_AudioDriver* psy_audio_player_audiodriver(psy_audio_Player* self)
{
	assert(self);

	return self->driver;
}

void psy_audio_player_loaddriver(psy_audio_Player* self, const char* path,
	psy_Property* config, bool open)
{
	psy_AudioDriver* driver;
	
	assert(self);

	driver = NULL;
	psy_audio_player_unloaddriver(self);
	if (path) {
        printf("driver path: %s\n", path);
		psy_library_load(&self->drivermodule, path);	
		if (self->drivermodule.module) {
			pfndriver_create fpdrivercreate;

			fpdrivercreate = (pfndriver_create)
				psy_library_functionpointer(&self->drivermodule,
				"driver_create");
			printf("driver driver_create\n");
			if (fpdrivercreate) {
				driver = fpdrivercreate();				
				psy_audiodriver_connect(driver, self, (AUDIODRIVERWORKFN)
					psy_audio_player_work, mainframe);
				printf("driver driver_connect\n");
			}
			psy_audio_exclusivelock_enable();
		}		
	}
	if (!driver) {		
		printf("create silentdriver\n");
		driver = psy_audio_create_silent_driver();
		psy_audio_exclusivelock_disable();
	}	
	psy_audio_sequencer_setsamplerate(&self->sequencer, psy_audiodriver_samplerate(
		driver));
	psy_dsp_rmsvol_setsamplerate(psy_audiodriver_samplerate(driver));
	self->driver = driver;
	if (self->driver) {
		if (config) {
			printf("driver driver_configure\n");
			psy_audiodriver_configure(self->driver, config);
		}
		printf("driver driver_open\n");
		if (open) {
			psy_audiodriver_open(self->driver);
		}
	}	
}

void psy_audio_player_unloaddriver(psy_audio_Player* self)
{
	assert(self);

	if (self->driver) {
		psy_audiodriver_close(self->driver);
		psy_audiodriver_deallocate(self->driver);
		psy_library_unload(&self->drivermodule);
		self->driver = NULL;
	}	
}

void psy_audio_player_reloaddriver(psy_audio_Player* self, const char* path,
	psy_Property* config)
{
	assert(self);

	psy_audio_player_unloaddriver(self);
	psy_audio_player_loaddriver(self, path, config, TRUE);
}

void psy_audio_player_restartdriver(psy_audio_Player* self,
	psy_Property* config)
{
	assert(self);

	if (self->driver) {
		psy_audiodriver_close(self->driver);
		psy_audiodriver_configure(self->driver, config);
		psy_audiodriver_open(self->driver);
	}
}

// Event Recording
void psy_audio_player_startrecordingnotes(psy_audio_Player* self)
{
	assert(self);

	self->recordingnotes = 1;
}

void psy_audio_player_stoprecordingnotes(psy_audio_Player* self)
{
	assert(self);

	self->recordingnotes = 0;
}

int psy_audio_player_recordingnotes(psy_audio_Player* self)
{
	assert(self);

	return self->recordingnotes;
}

// psy_EventDriver load, unload, restart, ..., methods
psy_EventDriver* psy_audio_player_loadeventdriver(psy_audio_Player* self, const char* path)
{
	assert(self);

	return psy_audio_eventdrivers_load(&self->eventdrivers, path);
}

void psy_audio_player_restarteventdriver(psy_audio_Player* self, int id,
	psy_Property* configuration)
{
	assert(self);

	psy_audio_eventdrivers_restart(&self->eventdrivers, id, configuration);
}

void psy_audio_player_removeeventdriver(psy_audio_Player * self, int id)
{
	assert(self);

	psy_audio_eventdrivers_remove(&self->eventdrivers, id);
}

psy_EventDriver* psy_audio_player_kbddriver(psy_audio_Player* self)
{
	assert(self);

	return self->eventdrivers.kbddriver;
}

psy_EventDriver* psy_audio_player_eventdriver(psy_audio_Player* self, int id)
{
	assert(self);

	return psy_audio_eventdrivers_driver(&self->eventdrivers, id);
}

uintptr_t psy_audio_player_numeventdrivers(psy_audio_Player* self)
{
	assert(self);

	return psy_audio_eventdrivers_size(&self->eventdrivers);
}

void psy_audio_player_write_eventdrivers(psy_audio_Player* self, psy_EventDriverInput input)
{
	uintptr_t i;
	uintptr_t num;

	num = psy_audio_player_numeventdrivers(self);
	for (i = 0; i < num; ++i) {
		psy_EventDriver* driver;
		
		driver = psy_audio_player_eventdriver(self, i);
		if (driver) {
			psy_eventdriver_write(driver, input);
		}
	}
}

void psy_audio_player_setemptysong(psy_audio_Player* self)
{
	assert(self);

	psy_audio_exclusivelock_enter();
	psy_audio_player_setsong(self, &self->emptysong);
	dsp.clear(bufferdriver, MAX_SAMPLES_WORKFN);
	psy_audio_exclusivelock_leave();
}

void psy_audio_player_midiconfigure(psy_audio_Player* self, psy_Property*
	configuration, bool datastr)
{
	assert(self);

	psy_audio_midiinput_configure(&self->midiinput, configuration, datastr);
}

void psy_audio_player_idle(psy_audio_Player* self)
{
	psy_audio_eventdrivers_idle(&self->eventdrivers);	
}

void psy_audio_player_sendcmd(psy_audio_Player* self, const char* section,
	psy_EventDriverCmd cmd)
{
	psy_audio_eventdrivers_sendcmd(&self->eventdrivers, section, cmd);
}
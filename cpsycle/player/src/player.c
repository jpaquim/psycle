/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"
#include "../../detail/os.h"

/* audio */
#include <audioconfig.h>
#include <machinefactory.h>
#include <player.h>
#include <plugincatcher.h>
#include <songio.h>
#include <exclusivelock.h>
/* file */
#include <dir.h>
#ifdef DIVERSALIS__OS__MICROSOFT
#include <windows.h>
#include <conio.h>
#else
#include <unistd.h>
#include "coniotermios.h"
#define _MAX_PATH 4096
#endif
#include <dir.h>
/* std */
#include <stdio.h>
#include <string.h>

typedef struct CmdPlayer {
	/* inherits */
	psy_audio_MachineCallback machinecallback;
	/* internal */
	psy_audio_Player player;	
	psy_audio_Song* song;	
	psy_Property* config;
	psy_Property* directories;	
	AudioConfig audioconfig;
} CmdPlayer;

static void cmdplayer_init(CmdPlayer*);
static void cmdplayer_init_player(CmdPlayer*);
static void cmdplayer_dispose(CmdPlayer*);
static void cmdplayer_parse(CmdPlayer*, int argc, char* argv[]);
static void cmdplayer_printoutputdriverlist(CmdPlayer*);
static uintptr_t cmdplayer_outputdriverindex(CmdPlayer*, const char* key);
static void cmdplayer_loadandrun(CmdPlayer*, const char* path);
static void cmdplayer_restartdriver(CmdPlayer* self);
static void cmdplayer_run(CmdPlayer*);
static void cmdplayer_scanplugins(CmdPlayer*);
static void cmdplayer_makedirectories(CmdPlayer*);
static void cmdplayer_loadsong(CmdPlayer*, const char*);
static void cmdplayer_onscanfile(CmdPlayer*, psy_audio_PluginCatcher* sender,
	const char* path, int type);
static void cmdplayer_applysongproperties(CmdPlayer*);
static psy_audio_MachineCallback machinecallback(CmdPlayer*);
static void cmdplayer_idle(CmdPlayer*);
/// Machinecallback
static psy_audio_MachineCallback machinecallback(CmdPlayer*);
static void machinecallback_output(CmdPlayer*, const char* text);
static psy_audio_Samples* machinecallback_samples(CmdPlayer*);
static psy_audio_Instruments* machinecallback_instruments(CmdPlayer*);

static void usage(void) {
	printf(
	"Usage: player [options] <song file name>\n"
	"Plays a Psycle song file.\n\n"
	" -odrv <name>   name of the output driver to use.\n"
	" -odrv help     list of available output drivers to use.\n"
	"Report bugs to the bug tracker at http://sourceforge.net/projects/psycle\n"); 
	// printf(
	//"Usage: psycle-player [options] [--input-file] <song file name>\n"
	//"Plays a Psycle song file.\n\n"
	//" -odrv, --output-driver <name>   name of the output driver to use.\n"
	/*"                                 available: dummy"		
	"\n\n"
	" -odev, --output-device <name>   name of the output device the driver should use.\n"
	"                                 The default device will be used if this option is not specified.\n"
		#if defined PSYCLE__SYDNEY_AVAILABLE
	"                                 examples for sydney: hostname:port\n"
		#endif
		#if defined PSYCLE__GSTREAMER_AVAILABLE
	"                                 examples for gstreamer: autoaudiosink, gconfaudiosink.\n"
		#endif
		#if defined PSYCLE__ALSA_AVAILABLE
	"                                 examples for alsa: default, hw:0, plughw:0, pulse.\n"
		#endif
		#if defined PSYCLE__ESOUND_AVAILABLE
	"                                 examples for esound: hostname:port\n"
		#endif
	"\n"
	" -of,   --output-file <riff wave file name>\n"
	"                                 name of the output file to render to in riff-wave format.\n\n"
	" -if,   --input-file <song file name>\n"
	"                                 name of the song file to play.\n\n"
	" -w,    --wait                   play until enter or ctrl+" PSYCLE__PLAYER__EOF " (EOF) is pressed.\n\n"
	"        --help                   display this help and exit.\n"
	"        --version                output version information and exit.\n\n"
	"Report bugs to the bug tracker at http://sourceforge.net/projects/psycle\n");*/
}

int main(int argc, char *argv[])
{
	CmdPlayer cmdplayer;	
    
	if (argc == 1) {
		usage();		
	} else {		
		cmdplayer_init(&cmdplayer);
		cmdplayer_parse(&cmdplayer, argc, argv);
		cmdplayer_dispose(&cmdplayer);
	}
	return 0;
}

// MachineCallback VTable
static psy_audio_MachineCallbackVtable vtable;
static bool vtable_initialized = FALSE;

static void vtable_init(CmdPlayer* self)
{
	if (!vtable_initialized) {
		vtable = *self->machinecallback.vtable;
		vtable.output = (fp_mcb_output)machinecallback_output;
		vtable_initialized = TRUE;
	}
	self->machinecallback.vtable = &vtable;
}

void cmdplayer_init(CmdPlayer* self)
{
	psy_audio_init();
	psy_audio_machinecallback_init(&self->machinecallback);
	vtable_init(self);	    
	self->config = psy_property_allocinit_key(NULL);	    		   
	cmdplayer_makedirectories(self);	
	cmdplayer_init_player(self);
	self->song = NULL;		
	audioconfig_init(&self->audioconfig, self->config, &self->player);	
}

void cmdplayer_dispose(CmdPlayer* self)
{
	psy_audio_player_dispose(&self->player);
	if (self->song) {
		psy_audio_song_deallocate(self->song);
		self->song = NULL;
	}
	if (self->config) {
		psy_property_deallocate(self->config);
		self->config = NULL;
	}	
	audioconfig_dispose(&self->audioconfig);
	psy_audio_dispose();
}

void cmdplayer_restartdriver(CmdPlayer* self)
{
	psy_Property* driversection;

	driversection = NULL;
	// load
	printf("load driver %s:\n",
		audioconfig_driver_path(&self->audioconfig));
	psy_audio_player_loaddriver(&self->player,
		audioconfig_driver_path(&self->audioconfig),
		NULL /*no config*/, FALSE /*do not open yet*/);
	// configure	
	if (psy_audiodriver_configuration(self->player.driver)) {
		driversection = psy_property_find(self->audioconfig.driverconfigurations,
			psy_property_key(
				psy_audiodriver_configuration(self->player.driver)),
			PSY_PROPERTY_TYPE_NONE);
	}
	// start
	psy_audio_player_restart_driver(&self->player, driversection);
	if (self->player.driver) {
		printf("Audio driver %s \n",
			psy_audiodriver_info(self->player.driver)->Name);
	}
}

void cmdplayer_init_player(CmdPlayer* self)
{
	psy_audio_player_init(&self->player, &self->machinecallback, NULL);
	psy_audio_plugincatcher_set_directories(&self->player.plugincatcher,
		self->directories);
	psy_signal_connect(&self->player.plugincatcher.signal_scanfile, self,
		cmdplayer_onscanfile);
	if (psy_audio_plugincatcher_load(&self->player.plugincatcher) == PSY_ERRFILE) {
		printf("no plugin cache found, start scanning\n");
		cmdplayer_scanplugins(self);
	}	
}

void cmdplayer_scanplugins(CmdPlayer* self)
{		
	int status;

	psy_audio_plugincatcher_scan(&self->player.plugincatcher);
	if ((status = psy_audio_plugincatcher_save(&self->player.plugincatcher))) {
		printf("Error saving plugin scanner list:\n");
		printf("%s\n",  self->player.plugincatcher.sections.inipath);
	}
}

void cmdplayer_makedirectories(CmdPlayer* self)
{	
	self->directories = psy_property_set_text(
		psy_property_append_section(self->config, "directories"),
		"Directories");		
	psy_property_set_hint(psy_property_set_text(
		psy_property_append_str(
			self->directories,
			"song",
			"C:\\Programme\\Psycle\\Songs"),
		"Song directory"),
		PSY_PROPERTY_HINT_EDITDIR);
#ifdef DIVERSALIS__OS__MICROSOFT		
	psy_property_set_hint(psy_property_set_text(
		psy_property_append_str(
			self->directories,
			"plugins64",
			"C:\\Programme\\Psycle\\PsyclePlugins"),
		"Plug-in directory"),		
		PSY_PROPERTY_HINT_EDITDIR);
#else
	psy_property_set_hint(psy_property_set_text(
		psy_property_append_str(
			self->directories,
			"plugins64",
			"../../plugins/build"),
		"Plug-in directory"),		
		PSY_PROPERTY_HINT_EDITDIR);
#endif		
	psy_property_set_hint(psy_property_set_text(
		psy_property_append_str(
			self->directories,
			"vst",			
			"C:\\Programme\\Psycle\\VstPlugins"),
		"VST directories"),
		PSY_PROPERTY_HINT_EDITDIR);
}


typedef enum CmdPlayerParseState {
	CMDPLAYER_PARSE_START = 1,
	CMDPLAYER_PARSE_HELP = 2,	
	CMDPLAYER_PARSE_OUTPUTDRIVER = 3,
	CMDPLAYER_PARSE_END = 4
} CmdPlayerParseState;

void cmdplayer_parse(CmdPlayer* self, int argc, char* argv[])
{
	int c;
	int state;

	state = CMDPLAYER_PARSE_START;
	for (c = 1; c < argc && state != CMDPLAYER_PARSE_END; ++c) {
		if (argv[c][0] == '-') {
			// option
			char* option;

			option = strchr(argv[c], '-') + 1;
			if (option) {
				if (strcmp(option, "odrv") == 0) {
					state = CMDPLAYER_PARSE_OUTPUTDRIVER;
				}
			} else {
				state = CMDPLAYER_PARSE_START;
			}
		} else {
			if (state == CMDPLAYER_PARSE_START) {
				state = CMDPLAYER_PARSE_END;
				cmdplayer_loadandrun(self, argv[c]);
			} else if (state == CMDPLAYER_PARSE_OUTPUTDRIVER) {
				if (strcmp(argv[c], "help") == 0) {
					cmdplayer_printoutputdriverlist(self);
					state = CMDPLAYER_PARSE_END;
				} else {										
					uintptr_t idx;					
					
					idx = cmdplayer_outputdriverindex(self, argv[c]);					
					if (idx == psy_INDEX_INVALID) {
						printf("Driver not found\n");
						state = CMDPLAYER_PARSE_END;
					} else if (idx >= 0) {
						psy_property_set_choice(self->audioconfig.inputoutput,
							"audiodrivers", (intptr_t)idx);
						state = CMDPLAYER_PARSE_START;
					} else {
						state = CMDPLAYER_PARSE_END;
					}					
				}
			} else {
				state = CMDPLAYER_PARSE_END;
			}
		}		
	}
	if (state == CMDPLAYER_PARSE_OUTPUTDRIVER) {
		cmdplayer_printoutputdriverlist(self);
	}
}

uintptr_t cmdplayer_outputdriverindex(CmdPlayer* self, const char* key)
{
	psy_Property* driver;	
	int conv;

	conv = atoi(key);
	if (conv == 0) {
		if (key[0] == '0') {
			return 0;
		} else {
			driver = psy_property_find(self->audioconfig.drivers, key,
				PSY_PROPERTY_TYPE_NONE);
			if (driver) {							
				return psy_list_entry_index(
					psy_property_begin(self->audioconfig.drivers), driver);
			}
		}
	} else if (conv > 0) {
		return (uintptr_t)conv;
	}
	return psy_INDEX_INVALID;
}

void cmdplayer_printoutputdriverlist(CmdPlayer* self)
{
	psy_Property* drivers;

	printf("Output driver list:\n");
	drivers = audioconfig_drivers(&self->audioconfig);
	if (drivers) {
		psy_List* p;
		uintptr_t idx;

		for (idx = 0, p = psy_property_begin(drivers); p != NULL; psy_list_next(&p),
				++idx) {
			psy_Property* driver;

			driver = (psy_Property*)psy_list_entry(p);
			printf("%d: %s\n", (int)idx, psy_property_key(driver));
		}
	}
}

void cmdplayer_loadandrun(CmdPlayer* self, const char* path)
{
	cmdplayer_loadsong(self, path);
	printf("psycle: player: press q to stop.\n");
#if !defined _WIN32
	set_conio_terminal_mode();
#endif
	cmdplayer_run(self);	
}

void cmdplayer_loadsong(CmdPlayer* self, const char* path)
{	
	psy_audio_Song* song;	
	psy_audio_Song* old_song;
	psy_audio_SongFile songfile;

	psy_audio_player_stop(&self->player);
	old_song = self->song;
	psy_audio_exclusivelock_enter();	
	song = psy_audio_song_allocinit(&self->player.machinefactory);
	assert(song);
	psy_audio_songfile_init(&songfile);
	songfile.song = song;
	songfile.file = 0;
	psy_audio_player_setemptysong(&self->player);
	psy_audio_machinecallback_set_song(&self->machinecallback, song);	
	if (path) {
		printf("path: %s\n", path);
	}
	if (psy_audio_songfile_load(&songfile, path) != PSY_OK) {
		psy_audio_song_deallocate(song);
		if (songfile.err) {
			fprintf(stderr, "Couldn't load song\n");			
		}	
	} else {
		psy_audio_player_set_song(&self->player, song);
		self->song = song;
	}
	psy_audio_songfile_dispose(&songfile);
	psy_audio_exclusivelock_leave();
	if (old_song) {
		psy_audio_song_deallocate(old_song);
	}
	
	cmdplayer_applysongproperties(self);
}

void cmdplayer_applysongproperties(CmdPlayer* self)
{			
	psy_audio_player_setbpm(&self->player, self->song->properties.bpm);
	psy_audio_player_set_lpb(&self->player, self->song->properties.lpb);
}

void cmdplayer_run(CmdPlayer* self)
{
	char c;
	int progress;
	
	cmdplayer_restartdriver(self);
	psy_audio_exclusivelock_enter();		
	psy_audio_player_stop(&self->player);
	psy_audio_sequencer_stop_loop(&self->player.sequencer);
	psy_audio_player_setposition(&self->player, 0.0);
	psy_audio_player_start(&self->player);
	psy_audio_exclusivelock_leave();		
	c = '\0';
	progress = 4;
	while (psy_audio_player_playing(&self->player)) {
		if (kbhit()) {
			c = getch();
			if (c == 'q') {
				break;
			}			
		}	
		cmdplayer_idle(self);
		--progress;
		if (progress == 0) {
			printf(".");
			fflush(stdout);
			progress = 16;
		}
	}
}

void cmdplayer_idle(CmdPlayer* self)
{
#ifdef _WIN32
	Sleep(200);
#else
	usleep(20000);
#endif
}

void cmdplayer_onscanfile(CmdPlayer* self, psy_audio_PluginCatcher* sender,
	const char* path, int type)
{
	printf("scan: %s\n", path);
}

// machine callback interface implementation
void machinecallback_output(CmdPlayer* self, const char* text)
{
    printf("%s\n", text);
}

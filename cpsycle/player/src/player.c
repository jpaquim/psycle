// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"
#include "../../detail/os.h"

#include <stdio.h>
#include <machinefactory.h>
#include <operations.h>
#include <player.h>
#include <plugincatcher.h>
#include <songio.h>
#include <exclusivelock.h>
#ifdef _WIN32
#include <windows.h>
#include <conio.h>
#else
#include <unistd.h>
#include "coniotermios.h"
#define _MAX_PATH 4096
#endif
#include <dir.h>

typedef struct {
	psy_audio_MachineCallback machinecallback;
	psy_audio_Player player;
	psy_audio_Song* song;	
	psy_audio_PluginCatcher plugincatcher;
	psy_audio_MachineFactory machinefactory;
	psy_Property* config;
	psy_Property* directories;
	psy_Property* inputoutput;
	psy_Property* driverconfigure;
	int hasplugincache;
} CmdPlayer;

static void cmdplayer_init(CmdPlayer*);
static void cmdplayer_initenv(CmdPlayer*);
static void cmdplayer_initplugincatcherandmachinefactory(CmdPlayer*);
static void cmdplayer_dispose(CmdPlayer*);
static void cmdplayer_scanplugins(CmdPlayer*);
static void cmdplayer_makedirectories(CmdPlayer*);
static void cmdplayer_makeinputoutput(CmdPlayer*);
static const char* cmdplayer_driverpath(CmdPlayer*);
static void cmdplayer_setdriverlist(CmdPlayer*);
static void cmdplayer_loadsong(CmdPlayer*, const char* path);
static void cmdplayer_applysongproperties(CmdPlayer*);
static psy_audio_MachineCallback machinecallback(CmdPlayer*);
static void cmdplayer_idle(void);
/// Machinecallback
static psy_audio_MachineCallback machinecallback(CmdPlayer*);
static void machinecallback_output(CmdPlayer*, const char* text);

static void usage(void) {
	printf(
	"Usage: player <song file name>\n");

	/*printf(
	"Usage: psycle-player [options] [--input-file] <song file name>\n"
	"Plays a Psycle song file.\n\n"
	" -odrv, --output-driver <name>   name of the output driver to use.\n"
	"                                 available: dummy"
		///\todo simply use configuration.driver_map() to build the list
		#if defined PSYCLE__SYDNEY_AVAILABLE
			", sydney"
		#endif
		#if defined PSYCLE__GSTREAMER_AVAILABLE
			", gstreamer"
		#endif
		#if defined PSYCLE__JACK_AVAILABLE
			", jack"
		#endif
		#if defined PSYCLE__ALSA_AVAILABLE
			", alsa"
		#endif
		#if defined PSYCLE__ESOUND_AVAILABLE
			", esd"
		#endif
		#if defined PSYCLE__NET_AUDIO_AVAILABLE
			", netaudio"
		#endif
		#if defined PSYCLE__MICROSOFT_DIRECT_SOUND_AVAILABLE
			", dsound"
		#endif
		#if defined PSYCLE__MICROSOFT_MME_AVAILABLE
			", mmewaveout"
		#endif
		#if defined PSYCLE__STEINBERG_ASIO_AVAILABLE
			", asio2"
		#endif
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
	char c;  
    
	if (argc == 1) {
		usage();		
	} else {
		cmdplayer_init(&cmdplayer);		
		cmdplayer_loadsong(&cmdplayer, argv[1]);		
		printf("psycle: player: press q to stop.\n");
		psy_audio_sequencer_stoploop(&cmdplayer.player.sequencer);
		psy_audio_player_setposition(&cmdplayer.player, (psy_dsp_big_beat_t) 0.f);
		psy_audio_player_start(&cmdplayer.player);
#if !defined _WIN32
		set_conio_terminal_mode();
#endif
	c = '\0';
	while (psy_audio_player_playing(&cmdplayer.player)) {
		if (kbhit()) {
			c = getch();
			if (c == 'q') {
				break;
			}
		}		
		cmdplayer_idle();
	}
	cmdplayer_dispose(&cmdplayer);
	}
	return 0;
}

void cmdplayer_idle(void)
{
#ifdef _WIN32
	Sleep(200);
#else
	usleep(2000);
#endif
}

// MachineCallback VTable
static psy_audio_MachineCallbackVtable psy_audio_machinecallbackvtable_vtable;
static int psy_audio_machinecallbackvtable_initialized = 0;

static void psy_audio_machinecallbackvtable_init(CmdPlayer* self)
{
	if (!psy_audio_machinecallbackvtable_initialized) {
		psy_audio_machinecallbackvtable_vtable = *self->machinecallback.vtable;		
		psy_audio_machinecallbackvtable_vtable.output = (fp_mcb_output)
			machinecallback_output;
		psy_audio_machinecallbackvtable_initialized = 1;
	}
}

void cmdplayer_init(CmdPlayer* self)
{    
	psy_audio_machinecallback_init(&self->machinecallback, &self->player, NULL);
	psy_audio_machinecallbackvtable_init(self);
	self->machinecallback.vtable = &psy_audio_machinecallbackvtable_vtable;
    printf("init config\n");
	self->config = psy_property_allocinit_key(NULL);
    printf("init player\n");
	cmdplayer_initenv(self);
    printf("init lock\n");
	psy_audio_exclusivelock_init();
    printf("init dsp\n");
	psy_dsp_noopt_init(&dsp);    
    printf("init directories\n");
	cmdplayer_makedirectories(self);
	cmdplayer_makeinputoutput(self);
    printf("init driverlist\n");
	cmdplayer_setdriverlist(self);
    printf("init factory\n");
	cmdplayer_initplugincatcherandmachinefactory(self);
    printf("alloc song\n");
	self->song = psy_audio_song_allocinit(&self->machinefactory);	
	psy_audio_player_init(&self->player, self->song, (void*)0);
    printf("load driver \n");
	psy_audio_player_loaddriver(&self->player, cmdplayer_driverpath(self),
		NULL /*no config*/, TRUE /*open*/);
	if (self->player.driver) {
		printf("Audio driver %s \n",
			psy_audiodriver_info(self->player.driver)->Name);
	}
}

void cmdplayer_initenv(CmdPlayer* self)
{
	char workpath[_MAX_PATH];
	const char* env = 0;	
	
	env = pathenv();	
	if (env) {			
		insertpathenv(workdir(workpath));
	}
}

void cmdplayer_initplugincatcherandmachinefactory(CmdPlayer* self)
{
	plugincatcher_init(&self->plugincatcher);
	plugincatcher_setdirectories(&self->plugincatcher, self->directories);
	// psy_signal_connect(&self->plugincatcher.signal_scanprogress, self,
	//	workspace_onscanprogress);
	if (!plugincatcher_load(&self->plugincatcher)) {
		printf("no plugin cache found, start scanning\n");
		cmdplayer_scanplugins(self);
	}
	psy_audio_machinefactory_init(&self->machinefactory, &self->machinecallback, 
		&self->plugincatcher);
}

void cmdplayer_scanplugins(CmdPlayer* self)
{		
	plugincatcher_scan(&self->plugincatcher);	
	plugincatcher_save(&self->plugincatcher);
}

void cmdplayer_makedirectories(CmdPlayer* self)
{	
	self->directories = psy_property_settext(
		psy_property_append_section(self->config, "directories"),
		"Directories");
	psy_property_sethint(psy_property_settext(
		psy_property_append_string(
			self->directories,
			"song",
			"C:\\Programme\\Psycle\\Songs"),
		"Song directory"),
		PSY_PROPERTY_HINT_EDITDIR);
	psy_property_sethint(psy_property_settext(
		psy_property_append_string(
			self->directories,
			"plugins",
			"C:\\Programme\\Psycle\\PsyclePlugins"),
		"Plug-in directory"),
		PSY_PROPERTY_HINT_EDITDIR);
	psy_property_sethint(psy_property_settext(
		psy_property_append_string(
			self->directories,
			"vst",			
			"C:\\Programme\\Psycle\\VstPlugins"),
		"VST directories"),
		PSY_PROPERTY_HINT_EDITDIR);
}

void cmdplayer_makeinputoutput(CmdPlayer* self)
{		
	self->inputoutput = psy_property_append_section(self->config, "inputoutput");
		cmdplayer_setdriverlist(self);
	self->driverconfigure = psy_property_settext(
		psy_property_append_section(self->inputoutput, "configure"),
		"Configure");		
}

void cmdplayer_setdriverlist(CmdPlayer* self)
{
	psy_Property* drivers;

	psy_property_settext(self->inputoutput, "Input/Output");
	// change number to set startup driver, if no psycle.ini found
	drivers = psy_property_append_choice(self->inputoutput, "driver", 1); 
	psy_property_settext(drivers, "Audio Driver");
	psy_property_append_string(drivers, "silent", "silentdriver");
#if defined(_DEBUG)
	psy_property_append_string(drivers, "mme", "..\\driver\\mme\\Debug\\mme.dll");
	psy_property_append_string(drivers, "directx", "..\\driver\\directx\\Debug\\directx.dll");
#else
	psy_property_append_string(drivers, "mme", "..\\driver\\mme\\Release\\mme.dll");
	psy_property_append_string(drivers, "directx", "..\\driver\\directx\\Release\\directx.dll");	
#endif
}

void cmdplayer_dispose(CmdPlayer* self)
{
	psy_audio_player_dispose(&self->player);
	psy_audio_song_deallocate(self->song);	
	self->song = 0;	
	psy_property_deallocate(self->config);
	self->config = 0;	
	plugincatcher_dispose(&self->plugincatcher);
	psy_audio_machinefactory_dispose(&self->machinefactory);	
	psy_audio_exclusivelock_dispose();
}

const char* cmdplayer_driverpath(CmdPlayer* self)
{	
	const char* rv = 0;
	/*psy_Property* p;

	p = psy_property_at(self->inputoutput, "driver", PSY_PROPERTY_TYPE_NONE);
	if (p) {
		int choice;		
		int count;
		
		choice = psy_property_item_int(p);
		p = p->children;
		count = 0;
		while (p) {
			if (count == choice) {
				rv = psy_property_item_str(p);
				break;
			}
			p = psy_property_next(p);
			++count;
		}
	}*/
	return rv;
}

void cmdplayer_loadsong(CmdPlayer* self, const char* path)
{		
	psy_audio_Song* oldsong;
	psy_audio_SongFile songfile;

	psy_audio_player_stop(&self->player);
	oldsong = self->song;
	psy_audio_exclusivelock_enter();	
	self->song = psy_audio_song_allocinit(&self->machinefactory);	
	songfile.song = self->song;
	songfile.file = 0;
	psy_audio_songfile_init(&songfile);
	psy_audio_songfile_load(&songfile, path);	
	if (songfile.err) {
		fprintf(stderr, "Couldn't load song\n");
	}	
	psy_audio_player_setsong(&self->player, self->song);
	cmdplayer_applysongproperties(self);
	psy_audio_exclusivelock_leave();
	psy_audio_song_deallocate(oldsong);
	psy_audio_songfile_dispose(&songfile);
}

void cmdplayer_applysongproperties(CmdPlayer* self)
{			
	psy_audio_player_setbpm(&self->player, self->song->properties.bpm);
	psy_audio_player_setlpb(&self->player, self->song->properties.lpb);
}

// machine callback interface implementation

void machinecallback_output(CmdPlayer* self, const char* text)
{
    printf("%s\n", text);
}

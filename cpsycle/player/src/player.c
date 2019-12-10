// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include <stdio.h>
#include <operations.h>
#include <player.h>
#include <songio.h>
#include <exclusivelock.h>
#ifdef _WIN32
#include <windows.h>
#include <conio.h>
#else
#include "coniotermios.h"
#endif
#include <dir.h>

typedef struct {
	Player player;
	Song* song;	
	PluginCatcher plugincatcher;
	MachineFactory machinefactory;
	Properties* config;
	Properties* directories;
	Properties* inputoutput;
	Properties* driverconfigure;
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
static MachineCallback machinecallback(CmdPlayer*);
static void cmdplayer_idle(void);
/// Machinecallback
static MachineCallback machinecallback(CmdPlayer*);
static unsigned int machinecallback_samplerate(CmdPlayer*);
static int machinecallback_bpm(CmdPlayer*);
static Samples* machinecallback_samples(CmdPlayer*);
static Machines* machinecallback_machines(CmdPlayer*);
static Instruments* machinecallback_instruments(CmdPlayer*);

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

	printf("Psycle player - testversion\n");
	if (argc == 1) {
		usage();		
	} else {
		cmdplayer_init(&cmdplayer);		
		cmdplayer_loadsong(&cmdplayer, argv[1]);		
		printf("psycle: player: press q to stop.\n");
		sequencer_stoploop(&cmdplayer.player.sequencer);
		player_setposition(&cmdplayer.player, (beat_t) 0.f);
		player_start(&cmdplayer.player);		
#if !defined _WIN32
		set_conio_terminal_mode();
#endif
	c = '\0';
	while (player_playing(&cmdplayer.player)) {
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

void cmdplayer_init(CmdPlayer* self)
{
	cmdplayer_initenv(self);
	lock_init();
	dsp_noopt_init(&dsp);
	self->config = properties_create();
	cmdplayer_makedirectories(self);
	cmdplayer_makeinputoutput(self);
	cmdplayer_setdriverlist(self);
	cmdplayer_initplugincatcherandmachinefactory(self);
	self->song = song_allocinit(&self->machinefactory);	
	player_init(&self->player, self->song, (void*)0);	
	player_loaddriver(&self->player, cmdplayer_driverpath(self), 0);	
	printf("Audio driver %s \n", 
		properties_readstring(self->player.driver->properties, "name",
		"no description"));
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
	plugincatcher_init(&self->plugincatcher, self->directories);
	// signal_connect(&self->plugincatcher.signal_scanprogress, self,
	//	workspace_onscanprogress);
	if (!plugincatcher_load(&self->plugincatcher)) {
		printf("no plugin cache found, start scanning\n");
		cmdplayer_scanplugins(self);
	}
	machinefactory_init(&self->machinefactory, machinecallback(self), 
		&self->plugincatcher);
}

void cmdplayer_scanplugins(CmdPlayer* self)
{		
	plugincatcher_scan(&self->plugincatcher);	
	plugincatcher_save(&self->plugincatcher);
}

void cmdplayer_makedirectories(CmdPlayer* self)
{	
	self->directories = properties_settext(
		properties_createsection(self->config, "directories"),
		"Directories");
	properties_sethint(properties_settext(
		properties_append_string(
			self->directories,
			"song",
			"C:\\Programme\\Psycle\\Songs"),
		"Song directory"),
		PROPERTY_HINT_EDITDIR);
	properties_sethint(properties_settext(
		properties_append_string(
			self->directories,
			"plugins",
			"C:\\Programme\\Psycle\\PsyclePlugins"),
		"Plug-in directory"),
		PROPERTY_HINT_EDITDIR);
	properties_sethint(properties_settext(
		properties_append_string(
			self->directories,
			"vst",			
			"C:\\Programme\\Psycle\\VstPlugins"),
		"VST directories"),
		PROPERTY_HINT_EDITDIR);
}

void cmdplayer_makeinputoutput(CmdPlayer* self)
{		
	self->inputoutput = properties_createsection(self->config, "inputoutput");
		cmdplayer_setdriverlist(self);
	self->driverconfigure = properties_settext(
		properties_createsection(self->inputoutput, "configure"),
		"Configure");		
}

void cmdplayer_setdriverlist(CmdPlayer* self)
{
	Properties* drivers;

	properties_settext(self->inputoutput, "Input/Output");
	// change number to set startup driver, if no psycle.ini found
	drivers = properties_append_choice(self->inputoutput, "driver", 1); 
	properties_settext(drivers, "Driver");
	properties_append_string(drivers, "silent", "silentdriver");
#if defined(_DEBUG)
	properties_append_string(drivers, "mme", "..\\driver\\mme\\Debug\\mme.dll");
	properties_append_string(drivers, "directx", "..\\driver\\directx\\Debug\\directx.dll");
#else
	properties_append_string(drivers, "mme", "..\\driver\\mme\\Release\\mme.dll");
	properties_append_string(drivers, "directx", "..\\driver\\directx\\Release\\directx.dll");	
#endif
}

void cmdplayer_dispose(CmdPlayer* self)
{
	player_dispose(&self->player);	
	song_free(self->song);	
	self->song = 0;	
	properties_free(self->config);
	self->config = 0;	
	plugincatcher_dispose(&self->plugincatcher);
	machinefactory_dispose(&self->machinefactory);	
	lock_dispose();
}

const char* cmdplayer_driverpath(CmdPlayer* self)
{
	Properties* p;
	const char* rv = 0;

	p = properties_read(self->inputoutput, "driver");
	if (p) {
		int choice;		
		int count;
		
		choice = properties_value(p);
		p = p->children;
		count = 0;
		while (p) {
			if (count == choice) {
				rv = properties_valuestring(p);
				break;
			}
			p = properties_next(p);
			++count;
		}
	}
	return rv;
}

void cmdplayer_loadsong(CmdPlayer* self, const char* path)
{		
	Song* oldsong;
	SongFile songfile;

	player_stop(&self->player);
	oldsong = self->song;
	lock_enter();	
	self->song = song_allocinit(&self->machinefactory);	
	songfile.song = self->song;
	songfile.file = 0;	
	songfile_load(&songfile, path);	
	if (songfile.err) {
		fprintf(stderr, "Couldn't load song\n");
	}	
	player_setsong(&self->player, self->song);
	cmdplayer_applysongproperties(self);
	lock_leave();
	song_free(oldsong);
}

void cmdplayer_applysongproperties(CmdPlayer* self)
{			
	player_setbpm(&self->player, self->song->properties.bpm);	
	player_setlpb(&self->player, self->song->properties.lpb);
}

// callbacks
MachineCallback machinecallback(CmdPlayer* self)
{
	MachineCallback rv;

	rv.context = self;
	rv.samples = machinecallback_samples;
	rv.samplerate = machinecallback_samplerate;
	rv.bpm = machinecallback_bpm;	
	rv.machines = machinecallback_machines;
	rv.instruments = machinecallback_instruments;
	return rv;
}

Samples* machinecallback_samples(CmdPlayer* self)
{
	return &self->song->samples;
}

unsigned int machinecallback_samplerate(CmdPlayer* self)
{
	return self->player.driver->samplerate(self->player.driver);
}

int machinecallback_bpm(CmdPlayer* self)
{
	return (int) player_bpm(&self->player);
}

Machines* machinecallback_machines(CmdPlayer* self)
{
	return self->song ? &self->song->machines : 0;
}

Instruments* machinecallback_instruments(CmdPlayer* self)
{
	return self->song ? &self->song->instruments : 0;
}


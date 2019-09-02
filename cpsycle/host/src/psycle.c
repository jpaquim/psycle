// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#define WIN32_LEAN_AND_MEAN

#include <lua.h> 
#include "mainframe.h"
#include <ui_app.h>
#include "library.h"
#include <commctrl.h>   // includes the common control header
#include <properties.h>
#include "greet.h"
#include <song.h>
#include <player.h>
#include <math.h>
#include <plugin_interface.h>
#include <sequence.h>
#include <sample.h>

// #define _CRTDBG_MAP_ALLOC
// #include <crtdbg.h>

void InitConfig(void);

static Properties* hostconfig;
static Greet greet;
static Player player;

Song song;
static Sequencer sequencer;


UIMAIN
{    	
	static MainFrame main;		
	static ui_component sci;	 
//	library scilib;
	int err = 0;
	
	// Sample sample;

	UIINIT;	
/*	scilib = loadlibrary("SciLexer.dll");
	if (scilib.lib == 0) {
		ui_error("The Scintilla Module could not be loaded", "Error loading Scintilla");			 
	}	*/

	InitConfig();	
	song_init(&song);			
#if defined(DEBUG)
	player_init(&player, &song, "..\\driver\\mme\\Debug\\");
#else
	player_init(&player, &song, "..\\driver\\mme\\Release\\");
#endif
	// sample_init(&sample);
	// sample_load(&sample, "Chord.wav");
	// samples_insert(&player.song->samples, &sample, 0);
	InitMainFrame(&main, hostconfig, &player);
	// ui_component_resize(&main, 1000, 600);
	ui_component_settitle(&main.component, "Psycle");
	/*if (scilib.lib) {
		ui_classcomponent_init(&sci, &sci, &main.component, "Scintilla");
		ui_component_move(&sci, 10, 100);
		ui_component_resize(&sci, 200, 200);
	}     	
*/
	ui_component_show_state(&main.component, iCmdShow);	 	 	 
	//greet_init(&greet, &main.component);
	//ui_component_show(&greet.component);	 	 	 
	err = ui_run();		
	player_dispose(&player);	
	properties_free(hostconfig);
//	freelibrary(scilib);
	song_dispose(&song);
	UIDISPOSE;
//    _CrtDumpMemoryLeaks( );   
	return err;
}

void InitConfig(void)
{
	hostconfig = properties_create();
	properties_append_string(hostconfig, "version", "alpha");
	properties_append_string(hostconfig, "plugindir", "C:\\Programme\\Psycle\\PsyclePlugins");
	properties_append_string(hostconfig, "vstdir", "C:\\Programme\\Psycle\\VstPlugins");
}


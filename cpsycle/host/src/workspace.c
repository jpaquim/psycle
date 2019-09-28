// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#include "workspace.h"
#include <stdlib.h>
#include <string.h>

static void workspace_config(Workspace*);
static void workspace_removesong(Workspace*);
static void applysongproperties(Workspace*);
static Properties* workspace_makeproperties(Workspace*);
static void workspace_setdriverlist(Workspace*);
static void workspace_driverconfig(Workspace*);
static const char* workspace_driverpath(Workspace*);
static void workspace_configaudio(Workspace*);
static void workspace_setsong(Workspace*, Song*);
static Samples* machinecallback_samples(Workspace*);
static unsigned int machinecallback_samplerate(Workspace*);
static Machines* machinecallback_machines(Workspace*);
static int machinecallback_bpm(Workspace*);


void workspace_init(Workspace* self)
{	
	self->octave = 4;
	signal_init(&self->signal_octavechanged);
	self->inputoutput = 0;
	workspace_config(self);		
	self->machinecallback.context = self;
	self->machinecallback.samples = machinecallback_samples;
	self->machinecallback.samplerate = machinecallback_samplerate;
	self->machinecallback.bpm = machinecallback_bpm;	
	self->machinecallback.machines = machinecallback_machines;
	machinefactory_init(&self->machinefactory, self->machinecallback,
		self->config);
	self->song = (Song*) malloc(sizeof(Song));
	song_init(self->song, &self->machinefactory);	
	signal_init(&self->signal_songchanged);
	signal_init(&self->signal_configchanged);
	self->properties = 0;
	plugincatcher_init(&self->plugincatcher);
	workspace_scanplugins(self);
	undoredo_init(&self->undoredo);
}

void workspace_dispose(Workspace* self)
{	
	player_dispose(&self->player);	
	song_dispose(self->song);
	properties_free(self->config);
	properties_free(self->properties);
	signal_dispose(&self->signal_octavechanged);
	signal_dispose(&self->signal_songchanged);
	signal_dispose(&self->signal_configchanged);
	plugincatcher_dispose(&self->plugincatcher);
	undoredo_dispose(&self->undoredo);
}

void workspace_initplayer(Workspace* self)
{
	player_init(&self->player, self->song, self->mainhandle);
	workspace_driverconfig(self);
}
	
void workspace_configaudio(Workspace* self)
{			
	player_loaddriver(&self->player, workspace_driverpath(self));
	self->player.driver->open(self->player.driver);	
	workspace_driverconfig(self);	
}

const char* workspace_driverpath(Workspace* self)
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
			p = p->next;
			++count;
		}
	}
	return rv;
}

void workspace_driverconfig(Workspace* self)
{		
	self->driverconfigure->item.disposechildren = 0;
	self->driverconfigure->children = self->player.driver->properties;	
}

void workspace_scanplugins(Workspace* self)
{	
	Properties* property;
	Properties* directories;

	directories = properties_find(self->config, "directories");
	if (directories && directories->children) {		
		plugincatcher_scan(&self->plugincatcher, "sampler", MACH_SAMPLER);		
		property = properties_read(directories, "vstdir");
		if (property) {
			plugincatcher_scan(&self->plugincatcher,
				properties_valuestring(property), MACH_VST);
		}
		property = properties_read(directories, "plugindir");
		if (property) {
			plugincatcher_scan(&self->plugincatcher,
				properties_valuestring(property), MACH_PLUGIN);
		}
	}
}

void workspace_config(Workspace* self)
{
	Properties* p;

	Properties* general;
	Properties* visual;
	Properties* keyboard;
	Properties* directories;	
	Properties* midicontrollers;

	self->config = properties_create();	
	general = properties_createsection(self->config, "general");
	properties_settext(general, "General");
	p = properties_append_string(general, "version", "alpha");	
	properties_settext(p, "Version");	
	p->item.hint = PROPERTY_HINT_HIDE;
	p = properties_append_bool(general, "showaboutatstart", 1);
	properties_settext(p, "Show About at Startup");
	p = properties_append_bool(general, "showsonginfoonload", 1);
	properties_settext(p, "Show song info on Load");	

	visual = properties_createsection(self->config, "visual");	
	properties_settext(visual, "Visual");	
	p = properties_append_bool(visual, "linenumbers", 1);
	properties_settext(p, "Line numbers");
	keyboard = properties_createsection(self->config, "keyboard");
	properties_settext(keyboard, "Keyboard and Misc.");
	directories = properties_createsection(self->config, "directories");
	properties_settext(directories, "Directories");
	p = properties_append_string(directories, "plugindir",
		"C:\\Programme\\Psycle\\PsyclePlugins");
	properties_settext(p, "Plug-in directory");
	p->item.hint = PROPERTY_HINT_EDITDIR;
	p = properties_append_string(directories, "vstdir",
		"C:\\Programme\\Psycle\\VstPlugins");
	properties_settext(p, "VST directories");
	p->item.hint = PROPERTY_HINT_EDITDIR;
	// InputOutput
	{		
		self->inputoutput = properties_createsection(self->config, "inputoutput");
		workspace_setdriverlist(self);
		self->driverconfigure = properties_createsection(self->inputoutput, "configure");
		properties_settext(self->driverconfigure, "Configure");		
	}
	// MIDIControllers
	midicontrollers = properties_createsection(self->config, "midicontrollers");
	properties_settext(midicontrollers, "MIDI Controllers");
}

void workspace_setdriverlist(Workspace* self)
{
	Properties* drivers;

	properties_settext(self->inputoutput, "Input/Output");
	// change number to set startup driver, if no psycle.ini found
	drivers = properties_append_choice(self->inputoutput, "driver", 2); 
	properties_settext(drivers, "Driver");
	properties_append_string(drivers, "silent", "silentdriver");		
#if defined(DEBUG)
	properties_append_string(drivers, "mme", "..\\driver\\mme\\Debug\\mme.dll");
	properties_append_string(drivers, "directx", "..\\driver\\directx\\Debug\\directx.dll");
#else
	properties_append_string(drivers, "mme", "..\\driver\\mme\\Release\\mme.dll");
	properties_append_string(drivers, "directx", "..\\driver\\directx\\Release\\directx.dll");	
#endif
}

void workspace_configchanged(Workspace* self, Properties* property, Properties* choice)
{
	if (choice && (strcmp(properties_key(choice), "driver") == 0)) {
		player_reloaddriver(&self->player, properties_valuestring(property));		
		workspace_driverconfig(self);
	}
	signal_emit(&self->signal_configchanged, self, 1, property);
}

int workspace_showsonginfoonload(Workspace* self)
{	
	int rv = 1;
	if (self->config) {
		Properties* p;

		p = properties_find(self->config, "general");
		if (p && p->children) {			
			properties_readbool(p, "showsonginfoonload", &rv, 1);
		}
	}
	return rv;
}

int workspace_showaboutatstart(Workspace* self)
{	
	int rv = 1;
	if (self->config) {
		Properties* p;

		p = properties_find(self->config, "general");
		if (p && p->children) {			
			properties_readbool(p, "showaboutatstart", &rv, 1);
		}
	}
	return rv;
}

int workspace_showlinenumbers(Workspace* self)
{	
	int rv = 1;
	if (self->config) {
		Properties* p;

		p = properties_find(self->config, "visual");
		if (p && p->children) {			
			properties_readbool(p, "linenumbers", &rv, 1);
		}
	}
	return rv;
}

void workspace_newsong(Workspace* self)
{			
	Song* song;
	
	song = (Song*) malloc(sizeof(Song));
	song_init(song, &self->machinefactory);	
	workspace_setsong(self, song);
	properties_free(self->properties);
	self->properties = workspace_makeproperties(self);
}

void workspace_loadsong(Workspace* self, const char* path)
{	
	Song* song;

	properties_free(self->properties);
	song = malloc(sizeof(Song));
	song_init(song, &self->machinefactory);	
	song_load(song, path, &self->properties);
	workspace_setsong(self, song);
}

void workspace_setsong(Workspace* self, Song* song)
{
	suspendwork();
	workspace_removesong(self);
	self->song = song;
	player_stop(&self->player);	
	player_setsong(&self->player, self->song);
	applysongproperties(self);
	signal_emit(&self->signal_songchanged, self, 0);
	resumework();
}

Properties* workspace_makeproperties(Workspace* self)
{
	Properties* root;
	Properties* machines;	
	Properties* machine;
	
	root = properties_create();
	machines = properties_createsection(root, "machines");	
	machine = properties_createsection(machines, "machine");	
	properties_append_int(machine, "index", MASTER_INDEX, 0, 0);		
	properties_append_int(machine, "x", 320, 0, 0);
	properties_append_int(machine, "y", 200, 0, 0);
	return root;
}

void workspace_removesong(Workspace* self)
{
	if (self->song) {
		song_dispose(self->song);
		free(self->song);
		self->song = 0;
		self->octave = 4;
	}
}

void applysongproperties(Workspace* self)
{	
	double dTmp;

	properties_readdouble(self->song->properties, "bpm", &dTmp, 125.0);
	player_setbpm(&self->player, (float)dTmp);	
	player_setlpb(&self->player, properties_int(self->song->properties, 
		"lpb", 4));
}

Properties* workspace_pluginlist(Workspace* self)
{
	return self->plugincatcher.plugins;
}

void workspace_load_configuration(Workspace* self)
{	
	properties_load(self->config, "psycle.ini");	
	workspace_configaudio(self);
}

void workspace_save_configuration(Workspace* self)
{
	properties_save(self->config, "psycle.ini");	
}

void workspace_setoctave(Workspace* self, int octave)
{
	self->octave = octave;
	signal_emit(&self->signal_octavechanged, self, 1, octave);
}

int workspace_octave(Workspace* self)
{
	return self->octave;
}

void workspace_updatedriver(Workspace* self)
{
	player_restartdriver(&self->player);
}

void workspace_undo(Workspace* self)
{
	undoredo_undo(&self->undoredo);
}

void workspace_redo(Workspace* self)
{
	undoredo_redo(&self->undoredo);
}

Samples* machinecallback_samples(Workspace* self)
{
	return &self->song->samples;
}

unsigned int machinecallback_samplerate(Workspace* self)
{
	return self->player.driver->samplerate(self->player.driver);
}

int machinecallback_bpm(Workspace* self)
{
	return (int)player_bpm(&self->player);
}

Machines* machinecallback_machines(Workspace* self)
{
	return self->song ? &self->song->machines : 0;
}
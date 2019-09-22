// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#include "workspace.h"
#include <stdlib.h>

static void workspace_config(Workspace*);
static Samples* machinecallback_samples(Workspace*);
static void workspace_removesong(Workspace*);
static void applysongproperties(Workspace*);
static Properties* workspace_makeproperties(Workspace*);

void workspace_init(Workspace* self)
{	
	self->octave = 4;
	signal_init(&self->signal_octavechanged);
	workspace_config(self);		
	self->song = (Song*) malloc(sizeof(Song));
	song_init(self->song);
	self->machinecallback.context = self;
	self->machinecallback.samples = machinecallback_samples;
#if defined(DEBUG)
	player_init(&self->player, self->song, "..\\driver\\mme\\Debug\\");
#else
	player_init(&self->player, self->song, "..\\driver\\mme\\Release\\");
#endif
	signal_init(&self->signal_songchanged);
	signal_init(&self->signal_configchanged);
	self->properties = 0;
	plugincatcher_init(&self->plugincatcher);
	workspace_scanplugins(self);
	machinefactory_init(&self->machinefactory, self->machinecallback, self->config);
}

void workspace_dispose(Workspace* self)
{	
	player_dispose(&self->player);	
	song_dispose(self->song);
	properties_free(self->config);
	signal_dispose(&self->signal_octavechanged);
	signal_dispose(&self->signal_songchanged);
	signal_dispose(&self->signal_configchanged);
	plugincatcher_dispose(&self->plugincatcher);
}

void workspace_scanplugins(Workspace* self)
{	
	Properties* property;

	plugincatcher_scan(&self->plugincatcher, "sampler", MACH_SAMPLER);		
	property = properties_read(self->config, "vstdir");
	if (property) {
		plugincatcher_scan(&self->plugincatcher,
			properties_valuestring(property), MACH_VST);
	}
	property = properties_read(self->config, "plugindir");
	if (property) {
		plugincatcher_scan(&self->plugincatcher,
			properties_valuestring(property), MACH_PLUGIN);
	}
}

void workspace_config(Workspace* self)
{
	Properties* p;
	self->config = properties_create();
	p = properties_append_string(self->config, "version", "alpha");	
	p = properties_append_string(self->config, "plugindir",
		"C:\\Programme\\Psycle\\PsyclePlugins");
	p->item.hint = PROPERTY_HINT_EDITDIR;
	p = properties_append_string(self->config, "vstdir",
		"C:\\Programme\\Psycle\\VstPlugins");	
	p->item.hint = PROPERTY_HINT_EDITDIR;
	p = properties_append_bool(self->config, "showaboutatstart", 1);
	p = properties_append_bool(self->config, "showsonginfoonload", 1);
	p = properties_append_bool(self->config, "linenumbers", 1);
}

void workspace_newsong(Workspace* self)
{	
	suspendwork();
	workspace_removesong(self);
	self->song = (Song*) malloc(sizeof(Song));
	song_init(self->song);	
	player_setsong(&self->player, self->song);
	player_initmaster(&self->player);
	applysongproperties(self);
	properties_free(self->properties);
	self->properties = workspace_makeproperties(self);
	signal_emit(&self->signal_songchanged, self, 0);
	resumework();
}

Properties* workspace_makeproperties(Workspace* self)
{
	Properties* properties;
	Properties* machinesproperties;	
	Properties* machineproperties;
	
	properties = properties_create();	
	machinesproperties = properties_append_int(properties, "machines", 0, 0, 0);
	machinesproperties->children = properties_create();			
	machineproperties =
		properties_append_int(machinesproperties->children, "machine",
			MASTER_INDEX, 0, 0);
	machineproperties->children = properties_create();
	properties_append_int(machineproperties->children, "x", 320, 0, 0);
	properties_append_int(machineproperties->children, "y", 200, 0, 0);
	return properties;
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

void workspace_loadsong(Workspace* self, const char* path)
{	
	suspendwork();	
	workspace_removesong(self);
	self->song = (Song*) malloc(sizeof(Song));
	song_init(self->song);
	player_setsong(&self->player, self->song);	
	player_initmaster(&self->player);	
	song_load(self->song, path, &self->machinefactory, &self->properties);	 
	applysongproperties(self);
	signal_emit(&self->signal_songchanged, self, 0);
	resumework();
}

void applysongproperties(Workspace* self)
{
	int tmp;
	double dTmp;

	properties_readdouble(self->song->properties, "bpm", &dTmp, 125.0);
	player_setbpm(&self->player, (float)dTmp);
	properties_readint(self->song->properties, "lpb", &tmp, 4);
	player_setlpb(&self->player, tmp);
}

Properties* workspace_pluginlist(Workspace* self)
{
	return self->plugincatcher.plugins;
}

void workspace_load_configuration(Workspace* self)
{	
	properties_load(self->config, "psycle.ini");	
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

Samples* machinecallback_samples(Workspace* self)
{
	return &self->song->samples;
}

// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#include "workspace.h"

static void workspace_config(Workspace* self);
static Samples* machinecallback_samples(Workspace*);
static void workspace_removesong(Workspace* self);
static void applysongproperties(Workspace* self);

void workspace_init(Workspace* self)
{
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
	self->properties = 0;
	plugincatcher_init(&self->plugincatcher);
	workspace_scanplugins(self);
	machinefactory_init(&self->machinefactory, self->machinecallback, self->config);
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

void workspace_dispose(Workspace* self)
{
	player_dispose(&self->player);	
	song_dispose(self->song);
	properties_free(self->config);
	signal_dispose(&self->signal_songchanged);
	plugincatcher_dispose(&self->plugincatcher);
}

void workspace_config(Workspace* self)
{
	self->config = properties_create();
	properties_append_string(self->config, "version", "alpha");
	properties_append_string(self->config, "plugindir", "C:\\Programme\\Psycle\\PsyclePlugins");
	properties_append_string(self->config, "vstdir", "C:\\Programme\\Psycle\\VstPlugins");	
}

void workspace_newsong(Workspace* self)
{	
	suspendwork();
	workspace_removesong(self);
	self->song = (Song*) malloc(sizeof(Song));
	song_init(self->song);	
	player_initmaster(&self->player);
	applysongproperties(self);
	properties_free(self->properties);
	self->properties = 0;
	signal_emit(&self->signal_songchanged, self, 0);
	resumework();
}

void workspace_removesong(Workspace* self)
{
	if (self->song) {
		song_dispose(self->song);
		free(self->song);
		self->song = 0;		
	}
}

void workspace_loadsong(Workspace* self, const char* path)
{	
	suspendwork();	
	workspace_removesong(self);
	self->song = (Song*) malloc(sizeof(Song));
	song_init(self->song);
	self->player.song = self->song;
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

Samples* machinecallback_samples(Workspace* self)
{
	return &self->song->samples;
}
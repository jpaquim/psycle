// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#include "machinefactory.h"
#include "plugin.h"
#include "vstplugin.h"
#include "sampler.h"
#include "mixer.h"
#include "master.h"
#include "duplicator.h"
#include <stdlib.h>
#include <string.h>

static char* modulepath(MachineFactory*, int machtype, const char* path, char* fullpath);
static int pathhasextension(const char* path);
static int onpropertiesenum(MachineFactory*, Properties*, int level);

static const char* searchname;
static int searchtype;
static Properties* searchresult;

void machinefactory_init(MachineFactory* self, MachineCallback callback,
	PluginCatcher* catcher)
{
	self->machinecallback = callback;	
	self->catcher = catcher;
}

Machine* machinefactory_make(MachineFactory* self, MachineType type,
	const char* plugincatchername)
{
	char fullpath[_MAX_PATH];

	return machinefactory_makefrompath(self, type,
		modulepath(self, MACH_PLUGIN, plugincatchername, fullpath));
}

Machine* machinefactory_makefrompath(MachineFactory* self, MachineType type,
	const char* path)
{
	Machine* machine = 0;

	switch (type) {
		case MACH_MASTER:
		{
			Master* master = (Master*)malloc(sizeof(Master));
			master_init(master, self->machinecallback);		
			machine = &master->machine;
		}
		break;
		case MACH_DUMMY:
		{
			DummyMachine* dummy = (DummyMachine*)malloc(sizeof(DummyMachine));
			dummymachine_init(dummy, self->machinecallback);	
			machine = &dummy->machine;
		}
		break;
		case MACH_DUPLICATOR:
		{
			Duplicator* duplicator = (Duplicator*)malloc(sizeof(Duplicator));
			duplicator_init(duplicator, self->machinecallback);	
			machine = &duplicator->machine;
		}
		break;
		case MACH_MIXER:
		{
			Mixer* mixer = (Mixer*)malloc(sizeof(Mixer));
			mixer_init(mixer, self->machinecallback);		
			machine = &mixer->machine;
		}
		break;
		case MACH_SAMPLER:
		{
			Sampler* sampler = (Sampler*)malloc(sizeof(Sampler));
			sampler_init(sampler, self->machinecallback);		
			machine = &sampler->machine;
		}
		break;
		case MACH_VST:
		{
			VstPlugin* plugin;			

			plugin = (VstPlugin*)malloc(sizeof(VstPlugin));
			vstplugin_init(plugin, self->machinecallback, path);	
			if (plugin->machine.info(&plugin->machine)) {						
				machine = &plugin->machine;			
			} else {
				plugin->machine.dispose(plugin);
				free(plugin);
			}
		}
		break;		
		case MACH_PLUGIN:
		{
			Plugin* plugin;			
									;
			plugin = (Plugin*)malloc(sizeof(Plugin));			
			plugin_init(plugin, self->machinecallback, path);
			if (plugin->machine.info(&plugin->machine)) {						
				machine = &plugin->machine;			
			} else {
				plugin->machine.dispose(plugin);
				free(plugin);
			}
		}
		break;
		default:
		break;
	}	
	return machine;
}

char* modulepath(MachineFactory* self, int machtype, const char* path,
	char* fullpath)
{	
	if (!path) {
		*fullpath = '\0';
	} else
	if (pathhasextension(path)) {
		strcpy(fullpath, path);
	} else {
		searchname = path;
		searchtype = machtype;
		searchresult = 0;
		properties_enumerate(self->catcher->plugins, self, onpropertiesenum);
		if (searchresult) {
			properties_readstring(searchresult, "path", &fullpath, "");
		} else {
			strcpy(fullpath, path);
		}
	}
	return fullpath;
}

int onpropertiesenum(MachineFactory* self, Properties* property, int level)
{
	if (properties_type(property) == PROPERTY_TYP_SECTION) {
		const char* key = properties_key(property);
		key = key;
		if ((strcmp(properties_key(property), searchname) == 0) &&
				properties_int(property, "type", 0) == searchtype) {
			searchresult = property;
			return 0;			
		}
	}
	return 1;
}

int pathhasextension(const char* path)
{
	return strrchr(path, '.') != 0;
}

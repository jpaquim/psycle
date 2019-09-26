// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#include "machinefactory.h"
#include "plugin.h"
#include "vstplugin.h"
#include "sampler.h"
#include "mixer.h"
#include "master.h"
#include <stdlib.h>
#include <string.h>

static char* makefullpath(MachineFactory*, const char* path,
	const char* dirconfigkey, char* fullpath);

void machinefactory_init(MachineFactory* self, MachineCallback callback,
	Properties* configuration)
{
	self->machinecallback = callback;
	self->configuration = configuration;
	self->directories = properties_find(self->configuration, "directories");
}

Machine* machinefactory_make(MachineFactory* self, MachineType type,
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
			char fullpath[_MAX_PATH];

			plugin = (VstPlugin*)malloc(sizeof(VstPlugin));
			vstplugin_init(plugin, 
				self->machinecallback,
				makefullpath(self, path, "vstplugindir", fullpath));	
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
			char fullpath[_MAX_PATH];
						
			plugin = (Plugin*)malloc(sizeof(Plugin));			
			plugin_init(plugin,
				self->machinecallback,
				makefullpath(self, path, "plugindir", fullpath));
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

char* makefullpath(MachineFactory* self, const char* path,
	const char* dirconfigkey, char* fullpath)
{
	char* dir;

	fullpath[0] = '\0';
	if (self->configuration && (strrchr(path, '\\') == 0)) {
		properties_readstring(self->directories, dirconfigkey, &dir, "");
		_snprintf(fullpath, _MAX_PATH, "%s%s%s", dir, "\\", path);
	} else {
		_snprintf(fullpath, _MAX_PATH, "%s", path);
	}	
	return fullpath;
}

// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#include "machinefactory.h"
#include "plugin.h"
#include "vstplugin.h"
#include "sampler.h"
#include "mixer.h"

static char* makefullpath(MachineFactory*, const char* path, const char* dirconfigkey, char* fullpath);

void machinefactory_init(MachineFactory* self, MachineCallback callback,
	Properties* configuration)
{
	self->machinecallback = callback;
	self->configuration = configuration;
}

Machine* machinefactory_make(MachineFactory* self, MachineType type,
	const char* path)
{
	Machine* machine = 0;

	switch (type) {
		case MACH_DUMMY:
		{
			DummyMachine* dummy = (DummyMachine*)malloc(sizeof(DummyMachine));
			dummymachine_init(dummy);	
			machine = &dummy->machine;
		}
		break;
		case MACH_MIXER:
		{
			Mixer* mixer = (Mixer*)malloc(sizeof(Mixer));
			mixer_init(mixer);		
			machine = &mixer->machine;
		}
		break;
		case MACH_SAMPLER:
		{
			Sampler* sampler = (Sampler*)malloc(sizeof(Sampler));
			sampler_init(sampler);		
			machine = &sampler->machine;
		}
		break;
		case MACH_VST:
		{
			VstPlugin* plugin;
			char fullpath[MAX_PATH];

			plugin = (VstPlugin*)malloc(sizeof(VstPlugin));
			vstplugin_init(plugin, makefullpath(self, path, "vstplugindir", fullpath));	
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
			char fullpath[MAX_PATH];
						
			plugin = (Plugin*)malloc(sizeof(Plugin));			
			plugin_init(plugin, makefullpath(self, path, "plugindir", fullpath));
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
	if (machine) {
		machine->machinecallback = self->machinecallback;
	}
	
	return machine;
}

char* makefullpath(MachineFactory* self, const char* path, const char* dirconfigkey, char* fullpath)
{
	char* dir;

	fullpath[0] = '\0';
	if (self->configuration) {
		properties_readstring(self->configuration, "dir", &dir, "");
	}
	_snprintf(fullpath, MAX_PATH, "%s%s%s", dir, "\\", path);
	return fullpath;
}
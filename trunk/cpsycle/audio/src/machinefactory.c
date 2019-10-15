// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "machinefactory.h"

#include "dummy.h"
#include "duplicator.h"
#include "duplicator2.h"
#include "master.h"
#include "mixer.h"
#include "plugin.h"
#include "sampler.h"
#include "vstplugin.h"

#include <stdlib.h>
#include <string.h>

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

	if (!self->catcher) {
		return 0;
	}
	return machinefactory_makefrompath(self, type,
		plugincatcher_modulepath(self->catcher, MACH_PLUGIN,
		plugincatchername, fullpath));
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
		case MACH_DUPLICATOR2:
		{
			Duplicator2* duplicator2 = (Duplicator2*)malloc(sizeof(Duplicator2));
			duplicator2_init(duplicator2, self->machinecallback);	
			machine = &duplicator2->machine;
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

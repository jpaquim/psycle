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
#include "machineproxy.h"

#include <stdlib.h>
#include <string.h>

void machinefactory_init(MachineFactory* self, MachineCallback callback,
	PluginCatcher* catcher)
{
	self->machinecallback = callback;	
	self->catcher = catcher;
	self->options = MACHINEFACTORY_CREATEASPROXY;	
}

void machinefactory_dispose(MachineFactory* self)
{	
}

void machinefactory_setoptions(MachineFactory* self, 
	MachineFactoryOptions options)
{
	self->options = options;
}

MachineFactoryOptions machinefactory_options(MachineFactory* self)
{
	return self->options;
}

Machine* machinefactory_makemachine(MachineFactory* self, MachineType type,
	const char* plugincatchername)
{
	char fullpath[_MAX_PATH];

	if (!self->catcher) {
		return 0;
	}
	return machinefactory_makemachinefrompath(self, type,
		plugincatcher_modulepath(self->catcher, MACH_PLUGIN,
		plugincatchername, fullpath));	
}

Machine* machinefactory_makemachinefrompath(MachineFactory* self,
	MachineType type, const char* path)
{
	Machine* rv = 0;
	MachineProxy* proxy = 0;

	switch (type) {
		case MACH_MASTER:
		{
			Master* master = (Master*)malloc(sizeof(Master));
			master_init(master, self->machinecallback);		
			rv = &master->machine;
		}
		break;
		case MACH_DUMMY:
		{
			DummyMachine* dummy = (DummyMachine*)malloc(sizeof(DummyMachine));
			dummymachine_init(dummy, self->machinecallback);	
			rv = &dummy->machine;
		}
		break;
		case MACH_DUPLICATOR:
		{
			Duplicator* duplicator = (Duplicator*)malloc(sizeof(Duplicator));
			duplicator_init(duplicator, self->machinecallback);	
			rv = &duplicator->machine;
		}
		break;
		case MACH_DUPLICATOR2:
		{
			Duplicator2* duplicator2 = (Duplicator2*)malloc(sizeof(Duplicator2));
			duplicator2_init(duplicator2, self->machinecallback);	
			rv = &duplicator2->machine;
		}
		break;
		case MACH_MIXER:
		{
			Mixer* mixer = (Mixer*)malloc(sizeof(Mixer));
			mixer_init(mixer, self->machinecallback);		
			rv = &mixer->machine;
		}
		break;
		case MACH_SAMPLER:
		{
			Sampler* sampler = (Sampler*)malloc(sizeof(Sampler));
			sampler_init(sampler, self->machinecallback);		
			rv = &sampler->machine;
		}
		break;
		case MACH_VST:
		{
			VstPlugin* plugin;			

			plugin = (VstPlugin*)malloc(sizeof(VstPlugin));
			vstplugin_init(plugin, self->machinecallback, path);	
			if (plugin->machine.info(&plugin->machine)) {						
				rv = &plugin->machine;			
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
				rv = &plugin->machine;			
			} else {
				plugin->machine.dispose(plugin);
				free(plugin);
			}
		}
		break;
		default:
		break;
	}
	if ((rv && ((self->options & MACHINEFACTORY_CREATEASPROXY)
			== MACHINEFACTORY_CREATEASPROXY))) {
		proxy = malloc(sizeof(MachineProxy));
		machineproxy_init(proxy, rv);
		rv = &proxy->machine;
	}	
	return rv;
}

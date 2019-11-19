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
	self->options = 0; // MACHINEFACTORY_CREATEASPROXY;	
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
		plugincatcher_modulepath(self->catcher, type,
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
			if (master) {
				master_init(master, self->machinecallback);
				rv = (Machine*) master;
			} else {
				rv = 0;
			}
		}
		break;
		case MACH_DUMMY:
		{
			DummyMachine* dummy = (DummyMachine*)malloc(sizeof(DummyMachine));
			if (dummy) {
				dummymachine_init(dummy, self->machinecallback);
				rv = (Machine*) dummy;
			} else {
				rv = 0;
			}
		}
		break;
		case MACH_DUPLICATOR:
		{
			Duplicator* duplicator = (Duplicator*)malloc(sizeof(Duplicator));
			if (duplicator) {
				duplicator_init(duplicator, self->machinecallback);
				rv = (Machine*) duplicator;
			} else {
				rv = 0;
			}
		}
		break;
		case MACH_DUPLICATOR2:
		{
			Duplicator2* duplicator2 = (Duplicator2*)malloc(sizeof(Duplicator2));
			if (duplicator2) {
				duplicator2_init(duplicator2, self->machinecallback);
				rv = (Machine*) duplicator2;
			} else {
				rv = 0;
			}
		}
		break;
		case MACH_MIXER:
		{
			Mixer* mixer = (Mixer*)malloc(sizeof(Mixer));
			if (mixer) {
				mixer_init(mixer, self->machinecallback);
				rv = (Machine*) mixer;
			} else {
				rv = 0;
			}
		}
		break;
		case MACH_SAMPLER:
		{
			Sampler* sampler = (Sampler*)malloc(sizeof(Sampler));
			if (sampler) {
				sampler_init(sampler, self->machinecallback);
				rv = (Machine*) sampler;
			} else {
				rv = 0;
			}
		}
		break;
		case MACH_VST:
		{
			Machine* plugin;			

			plugin = (Machine*)malloc(sizeof(VstPlugin));
			if (plugin) {
				vstplugin_init((VstPlugin*)plugin, self->machinecallback, path);
				if (plugin->info(plugin)) {
					rv = plugin;
				} else {
					plugin->dispose(plugin);
					free(plugin);
				}
			} else {
				rv = 0;
			}
		}
		break;		
		case MACH_PLUGIN:
		{
			Machine* plugin;			
									
			plugin = (Machine*)malloc(sizeof(Plugin));
			if (plugin) {
				plugin_init((Plugin*)plugin, self->machinecallback, path);
				if (plugin->info(plugin)) {
					rv = plugin;
				} else {
					plugin->dispose(plugin);
					free(plugin);
				}
			} else {
				rv = 0;
			}
		}
		break;
		default:
			rv = 0;
		break;
	}
	if ((rv && ((self->options & MACHINEFACTORY_CREATEASPROXY)
			== MACHINEFACTORY_CREATEASPROXY))) {
		proxy = malloc(sizeof(MachineProxy));
		if (proxy) {
			machineproxy_init(proxy, rv);
			rv = &proxy->machine;
		} else {
			rv->dispose(rv);
			free(rv);
			rv = 0;
		}		
	}	
	return rv;
}

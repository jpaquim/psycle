// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"
#include "../../detail/os.h"

#include "machinefactory.h"

#include "dummy.h"
#include "duplicator.h"
#include "duplicator2.h"
#include "master.h"
#include "mixer.h"
#include "plugin.h"
#include "luaplugin.h"
#include "sampler.h"
#include "vstplugin.h"
#include "machineproxy.h"

#include <stdlib.h>
#include <string.h>

#if !defined DIVERSALIS__OS__MICROSOFT
#define _MAX_PATH 4096
#endif

void machinefactory_init(MachineFactory* self, MachineCallback callback,
	psy_audio_PluginCatcher* catcher)
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

psy_audio_Machine* machinefactory_makemachine(MachineFactory* self, MachineType type,
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

psy_audio_Machine* machinefactory_makemachinefrompath(MachineFactory* self,
	MachineType type, const char* path)
{
	psy_audio_Machine* rv = 0;
	psy_audio_MachineProxy* proxy = 0;

	switch (type) {
		case MACH_MASTER:
		{
			psy_audio_Master* master = (psy_audio_Master*)malloc(sizeof(psy_audio_Master));
			if (master) {
				master_init(master, self->machinecallback);
				rv = (psy_audio_Machine*) master;
			} else {
				rv = 0;
			}
		}
		break;
		case MACH_DUMMY:
		{
			psy_audio_DummyMachine* dummy = (psy_audio_DummyMachine*)malloc(sizeof(psy_audio_DummyMachine));
			if (dummy) {
				dummymachine_init(dummy, self->machinecallback);
				rv = (psy_audio_Machine*) dummy;
			} else {
				rv = 0;
			}
		}
		break;
		case MACH_DUPLICATOR:
		{
			psy_audio_Duplicator* duplicator = (psy_audio_Duplicator*)malloc(sizeof(psy_audio_Duplicator));
			if (duplicator) {
				duplicator_init(duplicator, self->machinecallback);
				rv = (psy_audio_Machine*) duplicator;
			} else {
				rv = 0;
			}
		}
		break;
		case MACH_DUPLICATOR2:
		{
			psy_audio_Duplicator2* duplicator2 = (psy_audio_Duplicator2*)malloc(sizeof(psy_audio_Duplicator2));
			if (duplicator2) {
				duplicator2_init(duplicator2, self->machinecallback);
				rv = (psy_audio_Machine*) duplicator2;
			} else {
				rv = 0;
			}
		}
		break;
		case MACH_MIXER:
		{
			psy_audio_Mixer* mixer = (psy_audio_Mixer*)malloc(sizeof(psy_audio_Mixer));
			if (mixer) {
				mixer_init(mixer, self->machinecallback);
				rv = (psy_audio_Machine*) mixer;
			} else {
				rv = 0;
			}
		}
		break;
		case MACH_SAMPLER:
		{
			psy_audio_Sampler* sampler = (psy_audio_Sampler*)malloc(sizeof(psy_audio_Sampler));
			if (sampler) {
				sampler_init(sampler, self->machinecallback);
				rv = (psy_audio_Machine*) sampler;
			} else {
				rv = 0;
			}
		}
		break;
		case MACH_VST:
		case MACH_VSTFX:
		{
			psy_audio_Machine* plugin;			

			plugin = (psy_audio_Machine*)malloc(sizeof(psy_audio_VstPlugin));
			if (plugin) {
				vstplugin_init((psy_audio_VstPlugin*)plugin, self->machinecallback, path);
				if (plugin->vtable->info(plugin)) {
					rv = plugin;
				} else {
					plugin->vtable->dispose(plugin);
					free(plugin);
				}
			} else {
				rv = 0;
			}
		}
		break;		
		case MACH_PLUGIN:
		{
			psy_audio_Machine* plugin;			
									
			plugin = (psy_audio_Machine*)malloc(sizeof(psy_audio_Plugin));
			if (plugin) {
				plugin_init((psy_audio_Plugin*)plugin, self->machinecallback, path);
				if (plugin->vtable->info(plugin)) {
					rv = plugin;
				} else {
					plugin->vtable->dispose(plugin);
					free(plugin);
				}
			} else {
				rv = 0;
			}
		}
		break;
		case MACH_LUA:
		{
			psy_audio_Machine* plugin;			
									
			plugin = (psy_audio_Machine*)malloc(sizeof(psy_audio_LuaPlugin));
			if (plugin) {
				luaplugin_init((psy_audio_LuaPlugin*)plugin, self->machinecallback, path);
				if (plugin->vtable->info(plugin)) {
					rv = plugin;
				} else {
					plugin->vtable->dispose(plugin);
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
		proxy = malloc(sizeof(psy_audio_MachineProxy));
		if (proxy) {
			machineproxy_init(proxy, rv);
			rv = &proxy->machine;
		} else {
			rv->vtable->dispose(rv);
			free(rv);
			rv = 0;
		}		
	}	
	return rv;
}

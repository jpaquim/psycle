// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"
#include "../../detail/os.h"

#include "machinefactory.h"

#include "dummy.h"
#include "duplicator.h"
#include "duplicator2.h"
#include "master.h"
#include "mixer.h"
#include "audiorecorder.h"
#include "plugin.h"
#include "luaplugin.h"
#include "sampler.h"
#include "xmsampler.h"
#include "vstplugin.h"
#include "ladspaplugin.h"
#include "machineproxy.h"
#include "virtualgenerator.h"

#include <stdlib.h>
#include <string.h>

#if !defined DIVERSALIS__OS__MICROSOFT
#define _MAX_PATH 4096
#endif

void psy_audio_machinefactory_init(psy_audio_MachineFactory* self, psy_audio_MachineCallback* callback,
	psy_audio_PluginCatcher* catcher)
{
	self->machinecallback = callback;
	self->catcher = catcher;
#ifdef PSYCLE_USE_MACHINEPROXY
	self->createasproxy = TRUE;
#else
	self->createasproxy = FALSE;
#endif
	self->loadnewgamefxblitz = 0;
	self->errstr = strdup("");
}

void psy_audio_machinefactory_dispose(psy_audio_MachineFactory* self)
{
	free(self->errstr);
	self->errstr = NULL;
}

void psy_audio_machinefactory_seterrorstr(psy_audio_MachineFactory* self, const char* str)
{
	if (str != self->errstr) {
		free(self->errstr);
		self->errstr = strdup(str);
	}
}

void psy_audio_machinefactory_createasproxy(psy_audio_MachineFactory* self)
{
	self->createasproxy = TRUE;
}

void psy_audio_machinefactory_createwithoutproxy(psy_audio_MachineFactory* self)
{
	self->createasproxy = FALSE;
}

psy_audio_Machine* psy_audio_machinefactory_makemachine(psy_audio_MachineFactory* self,
	MachineType type, const char* plugincatchername, uintptr_t instindex)
{
	char fullpath[_MAX_PATH];

	if (!self->catcher) {
		return 0;
	}
	return psy_audio_machinefactory_makemachinefrompath(self, type,
		plugincatcher_modulepath(self->catcher, type,
		self->loadnewgamefxblitz,
		plugincatchername, fullpath),
		plugincatcher_extractshellidx(plugincatchername),
		instindex);
}

psy_audio_Machine* psy_audio_machinefactory_makemachinefrompath(psy_audio_MachineFactory* self,
	MachineType type, const char* path, uintptr_t shellidx, uintptr_t instindex)
{
	psy_audio_Machine* rv = 0;

	switch (type) {
		case MACH_MASTER: {
			psy_audio_Master* master;
			
			master = (psy_audio_Master*)malloc(sizeof(psy_audio_Master));
			if (master) {
				psy_audio_master_init(master, self->machinecallback);
				rv = (psy_audio_Machine*) master;
			} else {
				rv = 0;
			}		
			break;
		}
		case MACH_DUMMY: {
			psy_audio_DummyMachine* dummy;
			
			dummy = (psy_audio_DummyMachine*)malloc(sizeof(psy_audio_DummyMachine));
			if (dummy) {
				psy_audio_dummymachine_init(dummy, self->machinecallback);
				rv = (psy_audio_Machine*) dummy;
			} else {
				rv = 0;
			}		
			break;
		}
		case MACH_DUPLICATOR: {
			psy_audio_Duplicator* duplicator;
			
			duplicator = (psy_audio_Duplicator*)malloc(sizeof(psy_audio_Duplicator));
			if (duplicator) {
				psy_audio_duplicator_init(duplicator, self->machinecallback);
				rv = (psy_audio_Machine*) duplicator;
			} else {
				rv = 0;
			}		
			break;
		}
		case MACH_DUPLICATOR2: {
			psy_audio_Duplicator2* duplicator2;
			
			duplicator2 = (psy_audio_Duplicator2*)malloc(sizeof(psy_audio_Duplicator2));
			if (duplicator2) {
				psy_audio_duplicator2_init(duplicator2, self->machinecallback);
				rv = (psy_audio_Machine*) duplicator2;
			} else {
				rv = 0;
			}		
			break;
		}
		case MACH_MIXER:
		{
			psy_audio_Mixer* mixer;
			
			mixer = (psy_audio_Mixer*)malloc(sizeof(psy_audio_Mixer));
			if (mixer) {
				psy_audio_mixer_init(mixer, self->machinecallback);
				rv = (psy_audio_Machine*) mixer;
			} else {
				rv = 0;
			}
		}
		break;
		case MACH_RECORDER: {
			psy_audio_AudioRecorder* recorder;
			
			recorder = (psy_audio_AudioRecorder*)malloc(sizeof(psy_audio_AudioRecorder));
			if (recorder) {
				psy_audio_audiorecorder_init(recorder, self->machinecallback);
				rv = (psy_audio_Machine*)recorder;
			}
			else {
				rv = 0;
			}		
			break;
		}
		case MACH_SAMPLER: {
			psy_audio_Sampler* sampler;

			sampler = psy_audio_sampler_allocinit(self->machinecallback);
			if (sampler) {
				rv = psy_audio_sampler_base(sampler);
			} else {
				rv = 0;
			}
			break;			
		}
		case MACH_XMSAMPLER: {
			psy_audio_XMSampler* sampler;
			
			sampler = psy_audio_xmsampler_allocinit(self->machinecallback);
			if (sampler) {
				//sampler->xmsamplerload = 1;
				rv = psy_audio_xmsampler_base(sampler);
			} else {
				rv = 0;
			}		
			break;
		}
		case MACH_VST:
		case MACH_VSTFX: {
			psy_audio_Machine* plugin;			

			plugin = (psy_audio_Machine*)malloc(sizeof(psy_audio_VstPlugin));
			if (plugin) {
				psy_audio_vstplugin_init((psy_audio_VstPlugin*)plugin, self->machinecallback, path);
				if (psy_audio_machine_info(plugin)) {
					rv = plugin;
				} else {
					psy_audio_machine_dispose(plugin);
					free(plugin);
				}
			} else {
				rv = 0;
			}		
			break;	
		}
		case MACH_PLUGIN: {
			psy_audio_Machine* plugin;

			plugin = (psy_audio_Machine*)malloc(sizeof(psy_audio_Plugin));
			if (plugin) {
				psy_audio_plugin_init((psy_audio_Plugin*)plugin,
					self->machinecallback, path,
					self->catcher->nativeroot);
				if (psy_audio_machine_info(plugin)) {
					rv = plugin;
				} else {
					psy_audio_machine_dispose(plugin);
					free(plugin);
				}
			} else {
				rv = 0;
			}
			break;
		}
		case MACH_LUA: {
			psy_audio_LuaPlugin* plugin;

			plugin = (psy_audio_LuaPlugin*)malloc(sizeof(psy_audio_LuaPlugin));
			if (plugin) {
				psy_audio_luaplugin_init(plugin, self->machinecallback, path);
				if (psy_audio_machine_info(psy_audio_luaplugin_base(plugin))) {
					rv = psy_audio_luaplugin_base(plugin);
				} else {
					psy_audio_machinefactory_seterrorstr(self, plugin->script.errstr);
					psy_audio_machine_dispose(psy_audio_luaplugin_base(plugin));
					free(plugin);
				}
			} else {
				rv = 0;
			}
			break;
		}
		case MACH_LADSPA: {
			psy_audio_Machine* plugin;

			plugin = (psy_audio_Machine*)malloc(sizeof(psy_audio_LadspaPlugin));
			if (plugin) {
				psy_audio_ladspaplugin_init((psy_audio_LadspaPlugin*)plugin,
					self->machinecallback, path, shellidx);
				if (psy_audio_machine_info(plugin)) {
					rv = plugin;
				} else {
					psy_audio_machine_dispose(plugin);
					free(plugin);
				}
			} else {
				rv = 0;
			}
			break; }
		case MACH_VIRTUALGENERATOR: {
			psy_audio_VirtualGenerator* virtualgenerator;

			virtualgenerator = (psy_audio_VirtualGenerator*)malloc(sizeof(psy_audio_VirtualGenerator));
			if (virtualgenerator) {
				psy_audio_virtualgenerator_init(virtualgenerator, self->machinecallback, shellidx, instindex);
				rv = &virtualgenerator->custommachine.machine;
			} else {
				rv = NULL;
			}

			break; }
		default:
			rv = 0;
			break;
	}
	if (rv && self->createasproxy) {
		psy_audio_MachineProxy* proxy;

		proxy = psy_audio_machineproxy_allocinit(rv);
		if (proxy) {
			rv = psy_audio_machineproxy_base(proxy);
		} else {
			machine_base_dispose(rv);
			free(rv);
			rv = 0;
		}		
	}	
	return rv;
}

void psy_audio_machinefactory_loadnewgamefxandblitzifversionunknown(
	psy_audio_MachineFactory* self)
{
	self->loadnewgamefxblitz = TRUE;
}

void psy_audio_machinefactory_loadoldgamefxandblitzifversionunknown(
	psy_audio_MachineFactory* self)
{
	self->loadnewgamefxblitz = FALSE;
}

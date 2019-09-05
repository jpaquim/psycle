#include "machinefactory.h"
#include "plugin.h"
#include "vstplugin.h"
#include "sampler.h"

Machine* machinefactory_make(MachineCallback callback, CMachineInfo* info,
	const char* path)
{
	Machine* machine = 0;

	if (info && ((info->Flags & 32) == 32)) {
		if (strcmp(info->Name, "Sampler") == 0) {						
			Sampler* sampler = (Sampler*)malloc(sizeof(Sampler));
			sampler_init(sampler);
			sampler->machine.machinecallback = callback;
			machine = &sampler->machine;
		}
	} else
	if (info && ((info->Flags & 16) == 16)) {		
		VstPlugin* plugin;
		
		plugin = (VstPlugin*)malloc(sizeof(VstPlugin));
		vstplugin_init(plugin, path);	
		if (plugin->machine.info(&plugin->machine)) {						
			machine = &plugin->machine;			
		} else {
			plugin->machine.dispose(machine);
			free(machine);
		}
	} else {
		Plugin* plugin;
		
		plugin = (Plugin*)malloc(sizeof(Plugin));
		plugin_init(plugin, path);	
		if (plugin->machine.info(&plugin->machine)) {						
			machine = &plugin->machine;			
		} else {
			plugin->machine.dispose(machine);
			free(machine);
		}
	}
	
	return machine;
}
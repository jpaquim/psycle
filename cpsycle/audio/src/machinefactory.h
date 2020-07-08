// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#ifndef psy_audio_MACHINEFACTORY_H
#define psy_audio_MACHINEFACTORY_H

#include "machine.h"
#include "plugincatcher.h"
#include <hashtbl.h>

#ifdef __cplusplus
extern "C" {
#endif

// psy_audio_MachineFactory
//
// aim: It creates machines using the MachineType and the plugincatchername
//      defined by 'PluginFinder'. (psy_audio_machinefactory_makemachine)
//      Therefore it needs a 'PluginFinder' to translate the name to the
//      module path. The plugincatchername ends with a shellindex ':index'
//      to select the right plugin inside a plugin bundle (e.g. 'vstbundle:2'),
//      Bundles can be Vst and Ladspa plugins, the others use as suffix ':0'.
//      Internal machines use no path and the path can be NULL.
//      Besides the factory can make a machine from a MachineType, module path
//      and a shellindex. (psy_audio_machinefactory_makemachinefrompath)
//      At creation it also uses the MachineCallback for the host and depending
//      on the configuration it wraps the machine inside a machineproxy,
//      that can handle exceptions. Compatibility options for blitz can be set.
//
// Structure: psy_audio_MachineFactory <>----- psy_audio_PluginFinder
//                    <>
//                     |
//              psy_audio_MachineCallback 


typedef struct psy_audio_MachineFactory {
	psy_audio_MachineCallback machinecallback;	
	psy_audio_PluginCatcher* catcher;
	bool loadnewgamefxblitz;
	bool createasproxy;
	char* errstr;
} psy_audio_MachineFactory;

void psy_audio_machinefactory_init(psy_audio_MachineFactory*, psy_audio_MachineCallback, psy_audio_PluginCatcher*);
void psy_audio_machinefactory_dispose(psy_audio_MachineFactory*);
psy_audio_Machine* psy_audio_machinefactory_makemachine(psy_audio_MachineFactory*, MachineType,
	const char* plugincatchername);
psy_audio_Machine* psy_audio_machinefactory_makemachinefrompath(psy_audio_MachineFactory*, MachineType,
	const char* path, uintptr_t shellidx);
void psy_audio_machinefactory_createasproxy(psy_audio_MachineFactory*);
void psy_audio_machinefactory_createwithoutproxy(psy_audio_MachineFactory*);
void psy_audio_machinefactory_loadnewgamefxandblitzifversionunknown(psy_audio_MachineFactory*);
void psy_audio_machinefactory_loadoldgamefxandblitzifversionunknown(psy_audio_MachineFactory*);

#ifdef __cplusplus
}
#endif

#endif /* psy_audio_MACHINEFACTORY_H */

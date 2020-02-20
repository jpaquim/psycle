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

typedef enum {
	MACHINEFACTORY_CREATEASPROXY = 1
} MachineFactoryOptions;

typedef struct psy_audio_MachineFactory {
	MachineCallback machinecallback;	
	psy_audio_PluginCatcher* catcher;
	MachineFactoryOptions options;	
} psy_audio_MachineFactory;

void machinefactory_init(psy_audio_MachineFactory*, MachineCallback, psy_audio_PluginCatcher*);
void machinefactory_dispose(psy_audio_MachineFactory*);
psy_audio_Machine* machinefactory_makemachine(psy_audio_MachineFactory*, MachineType,
	const char* plugincatchername, uintptr_t shellidx);
psy_audio_Machine* machinefactory_makemachinefrompath(psy_audio_MachineFactory*, MachineType,
	const char* path, uintptr_t shellidx);
void machinefactory_setoptions(psy_audio_MachineFactory*, MachineFactoryOptions options);
MachineFactoryOptions machinefactory_options(psy_audio_MachineFactory*);

#ifdef __cplusplus
}
#endif

#endif /* psy_audio_MACHINEFACTORY_H */

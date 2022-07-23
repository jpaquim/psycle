/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#ifndef psy_audio_LADSPAPLUGIN_H
#define psy_audio_LADSPAPLUGIN_H

#include "custommachine.h"
#include "library.h"
#include "ladspaparam.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	psy_audio_CustomMachine custommachine;	
	psy_Library library;
	psy_audio_MachineInfo* plugininfo;
	int key_;				
	const LADSPA_Descriptor* psDescriptor;
	/*const*/ LADSPA_Handle pluginHandle;	
	//psycle index, ladspa index
	psy_Table inportmap; // int x int
	psy_Table outportmap; // int x int
	int numInputs;
	int numOutputs;
	float* pOutSamplesL;
	float* pOutSamplesR;
	psy_Table parameters;
} psy_audio_LadspaPlugin;

void psy_audio_ladspaplugin_init(psy_audio_LadspaPlugin*, psy_audio_MachineCallback*, const char* path, uintptr_t shellidx);
int psy_audio_plugin_ladspa_test(const char* path, psy_audio_MachineInfo*, uintptr_t shellidx);

INLINE psy_audio_Machine* psy_audio_ladspaplugin_base(psy_audio_LadspaPlugin* self)
{
	return &(self->custommachine.machine);
}

#ifdef __cplusplus
}
#endif

#endif /* psy_audio_LADSPAPLUGIN_H */

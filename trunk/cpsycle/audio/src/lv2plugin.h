/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#ifndef psy_audio_LV2PLUGIN_H
#define psy_audio_LV2PLUGIN_H

#include "../../detail/psyconf.h"

#ifdef PSYCLE_USE_LV2

/* local */
#include "custommachine.h"
/* lilv */
#include "lilv/lilv.h"
/* lv2 */
#include "lv2/core/lv2.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct LV2Plugin {
	/* inherits */
	psy_audio_CustomMachine custommachine;
	/* internal */	
	psy_audio_MachineInfo* plugininfo;
	void* editorhandle;
	LilvWorld*        world;
	const LilvPlugin* plugin;
	LilvInstance*     instance;
	LilvNodes* 	required_features;
	psy_Table inportmap; // int x int
	psy_Table outportmap; // int x int
	int numInputs;
	int numOutputs;
	psy_Table parameters;
	LilvNode* lv2_InputPort;
	LilvNode* lv2_OutputPort;
	LilvNode* lv2_AudioPort;
	LilvNode* lv2_ControlPort;
	LilvNode* lv2_connectionOptional;
//	psy_Table tracknote;
//	psy_Table parameters;
} psy_audio_LV2Plugin;

void psy_audio_lv2plugin_init(psy_audio_LV2Plugin*, psy_audio_MachineCallback*,
	const char* path, uintptr_t shell_index);

bool psy_audio_lv2plugin_test(const char* path, psy_audio_MachineInfo* rv,
	uintptr_t shell_idx);

INLINE psy_audio_Machine* psy_audio_lv2plugin_base(psy_audio_LV2Plugin* self)
{
	return &(self->custommachine.machine);
}

#ifdef __cplusplus
}
#endif

#endif /* PSYCLE_USE_LV2 */

#endif /* psy_audio_LV2PLUGIN_H */


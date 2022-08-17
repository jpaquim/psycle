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
#include "lv2param.h"
/* lilv */
#include "lilv/lilv.h"
/* lv2 */
#include "lv2/core/lv2.h"

#ifdef __cplusplus
extern "C" {
#endif

/* LilvTestUriMap: Copyright 2007-2020 David Robillard <d@drobilla.net> */
typedef struct {
  char**   uris;
  uint32_t n_uris;
} LilvTestUriMap;

typedef struct LV2Plugin {
	/* inherits */
	psy_audio_CustomMachine custommachine;
	/* internal */	
	psy_audio_MachineInfo* plugininfo;
	void* editorhandle;
	LilvWorld*        world;
	const LilvPlugin* plugin;
	LilvInstance*     instance;	
	LilvTestUriMap    uri_table;
    LV2_URID_Map       map;
    LV2_Feature        map_feature;
    LV2_URID_Unmap     unmap;
    LV2_Feature        unmap_feature;
    const LV2_Feature *features[2];
	psy_Table inportmap; // int x int
	psy_Table outportmap; // int x int
	uintptr_t numInputs;
	uintptr_t numOutputs;
	psy_Table parameters;
	LilvNode* input_port_class;
	LilvNode* output_port_class;
	LilvNode* audio_port_class;
	LilvNode* control_port_class;	
//	psy_Table tracknote;	
} psy_audio_LV2Plugin;

int psy_audio_lv2plugin_init(psy_audio_LV2Plugin*, psy_audio_MachineCallback*,
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


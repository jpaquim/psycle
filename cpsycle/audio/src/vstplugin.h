// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#ifndef psy_audio_VSTPLUGIN_H
#define psy_audio_VSTPLUGIN_H

#include "../../detail/psyconf.h"

#ifdef PSYCLE_USE_VST2

// local
#include "custommachine.h"
#include "library.h"
#include "vstplugininterface.h"
#include "vstpluginparam.h"
#include "vstevents.h"

#ifdef __cplusplus
extern "C" {
#endif

struct AEffect;
struct VstTimeInfo;

typedef struct VstPlugin {
	// inherits
	psy_audio_CustomMachine custommachine;
	// internal data
	psy_Library library;	
	void* editorhandle;
	psy_audio_VstEvents vstevents;
	psy_audio_VstEvents vstoutevents;
	psy_audio_VstInterface mi;
	struct VstTimeInfo* vsttimeinfo;	
	psy_audio_MachineInfo* plugininfo;
	psy_Table tracknote;
	psy_Table parameters;
} psy_audio_VstPlugin;

int psy_audio_vstplugin_init(psy_audio_VstPlugin*, psy_audio_MachineCallback*,
	const char* path);

bool psy_audio_vstplugin_test(const char* path, psy_audio_MachineInfo* rv);

INLINE psy_audio_Machine* psy_audio_vstplugin_base(psy_audio_VstPlugin* self)
{
	return &(self->custommachine.machine);
}

#ifdef __cplusplus
}
#endif

#endif /* PSYCLE_USE_VST2 */

#endif /* psy_audio_VSTPLUGIN_H */

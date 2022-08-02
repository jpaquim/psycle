// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#ifndef psy_audio_VSTPLUGINPARAM_H
#define psy_audio_VSTPLUGINPARAM_H

#include "../../detail/psyconf.h"

#ifdef PSYCLE_USE_VST2

// local
#include "custommachine.h"
#include "vstplugininterface.h"

#ifdef __cplusplus
extern "C" {
#endif

struct AEffect;

typedef struct psy_audio_VstPluginParam {
	psy_audio_CustomMachineParam custommachineparam;
	struct AEffect* effect;
	uintptr_t index;
} psy_audio_VstPluginParam;

void psy_audio_vstpluginparam_init(psy_audio_VstPluginParam*, struct AEffect*,
	uintptr_t index);
void psy_audio_vstpluginparam_dispose(psy_audio_VstPluginParam*);
psy_audio_VstPluginParam* psy_audio_vstpluginparam_alloc(void);
psy_audio_VstPluginParam* psy_audio_vstpluginparam_allocinit(struct AEffect*,
	uintptr_t index);

#ifdef __cplusplus
}
#endif

#endif /* PSYCLE_USE_VST2 */

#endif /* psy_audio_VSTPLUGINPARAM_H */

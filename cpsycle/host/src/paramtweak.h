// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#if !defined(PARAMTWEAK)
#define PARAMTWEAK

#include "workspace.h"
#include <uicomponent.h>
#include <machine.h>
#include "skincoord.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct ParamTweak {
    // internal data
    float tweakbase;
    float tweakval;
    // references
    psy_audio_Machine* machine;
    uintptr_t paramindex;
    ParamSkin skin;
    psy_audio_MachineParam* param;
} ParamTweak;

void paramtweak_init(ParamTweak*);

void paramtweak_begin(ParamTweak*, psy_audio_Machine*,
    uintptr_t paramindex);
void paramtweak_end(ParamTweak*);
void paramtweak_onmousedown(ParamTweak*, psy_ui_MouseEvent*);
void paramtweak_onmousemove(ParamTweak*, psy_ui_MouseEvent*);

#ifdef __cplusplus
}
#endif

#endif /* PARAMTWEAK */

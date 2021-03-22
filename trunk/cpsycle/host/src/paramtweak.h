// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#if !defined(PARAMTWEAK)
#define PARAMTWEAK

#include "../../detail/psydef.h"

#ifdef __cplusplus
extern "C" {
#endif

// ParamTweak
//
// MouseEvent Helper for psy_audio_MachineParam ui components

// Forward Declarations
// audio
struct psy_audio_Machine;
struct psy_audio_MachineParam;
// ui
struct psy_ui_MouseEvent;

typedef struct ParamTweak {
    // internal data
    float tweakbase;
    float tweakval;
    bool active;
    // references
    struct psy_audio_Machine* machine;
    uintptr_t paramidx;
    struct psy_audio_MachineParam* param;
} ParamTweak;

void paramtweak_init(ParamTweak*);

// activates tweak
//     Either machine and paramidx or machineparam needs to be set
//     machineparam is used only if machine or paramidx are not valid
void paramtweak_begin(ParamTweak*, struct psy_audio_Machine*,
    uintptr_t paramindex, struct psy_audio_MachineParam*);
// deactivate tweak
void paramtweak_end(ParamTweak*);
// called by the uis to delegate the mouse events
void paramtweak_onmousedown(ParamTweak*, struct psy_ui_MouseEvent*);
void paramtweak_onmousemove(ParamTweak*, struct psy_ui_MouseEvent*);
//\ return tweak status
INLINE bool paramtweak_active(const ParamTweak* self)
{
    return self->active;
}

#ifdef __cplusplus
}
#endif

#endif /* PARAMTWEAK */

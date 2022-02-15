// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#if !defined(HEADERUI_H)
#define HEADERUI_H

// host
#include "paramtweak.h"
// ui
#include <uicomponent.h>

// HeaderUi
//
// Header to display a MachineParameter (MPF_HEADER)

#ifdef __cplusplus
extern "C" {
#endif

struct ParamSkin;

/* HeaderUi */
typedef struct HeaderUi {
	/* inherits */
	psy_ui_Component component;
	/* internal */
	uintptr_t paramidx;
	/* references */
	struct psy_audio_Machine* machine;
	struct psy_audio_MachineParam* param;
} HeaderUi;

void headerui_init(HeaderUi*, psy_ui_Component* parent,
	struct psy_audio_Machine*, uintptr_t paramidx,
	struct psy_audio_MachineParam*);

HeaderUi* headerui_alloc(void);
HeaderUi* headerui_allocinit(psy_ui_Component* parent,
	struct psy_audio_Machine*, uintptr_t paramidx,
	struct psy_audio_MachineParam*);

INLINE psy_ui_Component* headerui_base(HeaderUi* self)
{
	assert(self);

	return &self->component;
}

#ifdef __cplusplus
}
#endif

#endif /* HEADERUI_H */

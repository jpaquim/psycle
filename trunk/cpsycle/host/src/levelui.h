// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#if !defined(LEVELUI_H)
#define LEVELUI_H

// host
#include "paramtweak.h"
// ui
#include <uicomponent.h>

// LevelUi
//
// Level to display a MachineParameter (MPF_LEVEL)

#ifdef __cplusplus
extern "C" {
#endif

struct ParamSkin;

// LevelUi
typedef struct LevelUi {
	// inherits
	psy_ui_Component component;
	// internal	
	uintptr_t paramidx;
	// references
	struct ParamSkin* skin;	
	psy_ui_Component* view;
	struct psy_audio_Machine* machine;	
	struct psy_audio_MachineParam* param;	
} LevelUi;

void levelui_init(LevelUi*, psy_ui_Component* parent, psy_ui_Component* view,
	struct psy_audio_Machine*, uintptr_t paramidx,
	struct psy_audio_MachineParam*, struct ParamSkin*);

LevelUi* levelui_alloc(void);
LevelUi* levelui_allocinit(psy_ui_Component* parent, psy_ui_Component* view,
	struct psy_audio_Machine*, uintptr_t paramidx,
	struct psy_audio_MachineParam*, struct ParamSkin*);

INLINE psy_ui_Component* levelui_base(LevelUi* self)
{
	assert(self);

	return &self->component;
}

#ifdef __cplusplus
}
#endif

#endif /* LEVELUI_H */

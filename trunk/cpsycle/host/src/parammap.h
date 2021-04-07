// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#if !defined(PARAMMAP_H)
#define PARAMMAP_H

// host
#include "workspace.h"
// ui
#include <uilistbox.h>
#include <uilabel.h>
// audio
#include <machine.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct ParamMap {
	// inherits
	psy_ui_Component component;
	psy_ui_Label construction;
	// intern	
	// references
	psy_audio_Machine* machine;
	MachineParamConfig* config;	
} ParamMap;

void parammap_init(ParamMap*, psy_ui_Component* parent, psy_audio_Machine*,
	MachineParamConfig*);
void parammap_setmachine(ParamMap*, psy_audio_Machine*);

INLINE psy_ui_Component* parammap_base(ParamMap* self)
{
	return &self->component;
}

#ifdef __cplusplus
}
#endif

#endif /* PARAMMAP_H */

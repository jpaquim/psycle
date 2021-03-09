// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#if !defined(SWITCHUI_H)
#define SWITCHUI_H

// host
#include "paramview.h"
#include "workspace.h"

#ifdef __cplusplus
extern "C" {
#endif

// SwitchUi
typedef struct SwitchUi {
	// inherits
	psy_ui_Component component;
	// internal
	ParamTweak paramtweak;
	// references
	ParamSkin* skin;
	Workspace* workspace;
	psy_ui_Component* view;
	psy_audio_MachineParam* param;
} SwitchUi;

void switchui_init(SwitchUi*, psy_ui_Component* parent,
	psy_ui_Component* view, psy_audio_MachineParam*, ParamSkin*);

SwitchUi* switchui_alloc(void);
SwitchUi* switchui_allocinit(psy_ui_Component* parent, psy_ui_Component* view,
	psy_audio_MachineParam*, ParamSkin*);

#ifdef __cplusplus
}
#endif

#endif /* SLIDERUI_H */

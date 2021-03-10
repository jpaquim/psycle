// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#if !defined(KNOBUI_H)
#define KNOBUI_H

// host
#include "machineeditorview.h" // vst view
#include "machineframe.h"
#include "machineviewskin.h"
#include "workspace.h"
// ui
#include <uiedit.h>
#include <uinotebook.h>

#ifdef __cplusplus
extern "C" {
#endif

// KnobUi
typedef struct KnobUi {
	// inherits
	psy_ui_Component component;
	// internal
	ParamTweak paramtweak;
	// references
	ParamSkin* skin;
	Workspace* workspace;	
	psy_audio_MachineParam* param;
} KnobUi;

void knobui_init(KnobUi*, psy_ui_Component* parent,
	psy_ui_Component* view, psy_audio_MachineParam*, ParamSkin*);

KnobUi* knobui_alloc(void);
KnobUi* knobui_allocinit(psy_ui_Component* parent, psy_ui_Component* view,
	psy_audio_MachineParam*, ParamSkin*);

#ifdef __cplusplus
}
#endif

#endif /* KNOBUI_H */

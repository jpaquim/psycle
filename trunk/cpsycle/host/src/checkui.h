// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#if !defined(CHECKUI_H)
#define CHECKUI_H

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

// CheckUi
typedef struct CheckUi {
	// inherits
	psy_ui_Component component;
	// internal
	ParamTweak paramtweak;
	// references
	ParamSkin* skin;
	Workspace* workspace;
	psy_ui_Component* view;
	psy_audio_MachineParam* param;
} CheckUi;

void checkui_init(CheckUi*, psy_ui_Component* parent,
	psy_ui_Component* view, psy_audio_MachineParam*, ParamSkin*);

CheckUi* checkui_alloc(void);
CheckUi* checkui_allocinit(psy_ui_Component* parent,
	psy_ui_Component* view, psy_audio_MachineParam*,
	ParamSkin*);


#ifdef __cplusplus
}
#endif

#endif /* CHECKUI_H */

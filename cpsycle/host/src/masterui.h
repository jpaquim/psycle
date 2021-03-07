// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#if !defined(MASTERUI_H)
#define MASTERUI_H

// host
#include "machineui.h" // vst view
// ui
#include <uiedit.h>
#include <uinotebook.h>

#ifdef __cplusplus
extern "C" {
#endif

// MasterUi
typedef struct MasterUi {
	// inherits
	psy_ui_Component component;
	// internal
	MachineUiCommon intern;	
} MasterUi;

void masterui_init(MasterUi*, psy_ui_Component* parent, MachineViewSkin*,
	psy_ui_Component* view, Workspace*);

#ifdef __cplusplus
}
#endif

#endif /* MASTERUI_H */

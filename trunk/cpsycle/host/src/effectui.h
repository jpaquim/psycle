// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#if !defined(EFFECTUI_H)
#define EFFECTUI_H

// host
#include "machineui.h"

#ifdef __cplusplus
extern "C" {
#endif

// EffectUi
typedef struct EffectUi {
	// inherits
	psy_ui_Component component;
	// internal
	MachineUiCommon intern;	
} EffectUi;

void effectui_init(EffectUi*, psy_ui_Component* parent,
	uintptr_t slot, MachineViewSkin*,
	psy_ui_Component* view, psy_ui_Edit* editname, Workspace*);

#ifdef __cplusplus
}
#endif

#endif /* EFFECTUI_H */

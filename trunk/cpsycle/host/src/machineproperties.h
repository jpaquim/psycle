// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net


#if !defined(MACHINEPROPERTIES)
#define MACHINEPROPERTIES

#include "uilabel.h"
#include "uiedit.h"
#include "uibutton.h"
#include "pattern.h"
#include "workspace.h"

#ifdef __cplusplus
extern "C" {
#endif

struct MachineViewSkin;

typedef struct {
	psy_ui_Component component;		
	psy_ui_Button issolobypass;
	psy_ui_Button ismute;
	psy_ui_Button isbus;
	psy_ui_Label namelabel;	
	psy_ui_Edit nameedit;	
	psy_ui_Button applybutton;
	psy_ui_Button remove;
	psy_ui_Button cancel;
	uintptr_t macid;
	// references
	psy_audio_Machine* machine;
	psy_audio_Machines* machines;
	Workspace* workspace;
	struct MachineViewSkin* skin;
} MachineProperties;

void machineproperties_init(MachineProperties*, psy_ui_Component* parent, psy_audio_Machine*,
	struct MachineViewSkin* skin, Workspace*);
void machineproperties_setpattern(MachineProperties*, psy_audio_Machine*);

#ifdef __cplusplus
}
#endif

#endif

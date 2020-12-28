// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#if !defined(PRESETSBAR_H)
#define PRESETSBAR_H

#include "workspace.h"

#include <dir.h>
#include <presets.h>
#include <uibutton.h>
#include <uiedit.h>
#include <uilabel.h>
#include <uicombobox.h>
#include <presets.h>
#include <machine.h>

#ifdef __cplusplus
extern "C" {
#endif

// aim: manages machine presets 

typedef struct PresetsBar {
	psy_ui_Component component;
	psy_ui_Label bank;
	psy_ui_ComboBox bankselector;
	psy_ui_Label program;
	psy_ui_ComboBox programbox;
	psy_ui_Button importpresets;
	psy_ui_Button exportpresets;
	psy_ui_Button savepresets;
	psy_ui_Edit savename;
	psy_audio_Machine* machine;
	bool userpreset;
	psy_Path presetpath;	
	Workspace* workspace;
} PresetsBar;

void presetsbar_init(PresetsBar*, psy_ui_Component* parent, Workspace*);
void presetsbar_setmachine(PresetsBar*, psy_audio_Machine* machine);
void presetsbar_showprogram(PresetsBar*);

#ifdef __cplusplus
}
#endif

#endif /* PRESETSBAR_H */

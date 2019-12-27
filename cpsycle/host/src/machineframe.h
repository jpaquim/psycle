// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#if !defined(MACHINEFRAME_H)
#define MACHINEFRAME_H

#include "paramview.h"
#include "paramlistbox.h"

#include <presets.h>
#include <uibutton.h>
#include <uicombobox.h>
#include <presets.h>
#include <machine.h>

typedef struct {
	psy_ui_Component component;
	psy_ui_Button mute;
	ui_combobox presetsbox;
	psy_audio_Presets* presets;
} ParameterBar;

typedef struct {
	psy_ui_Component component;
	ParameterBar parameterbar;
	ParameterListBox parameterbox;
	psy_ui_Component* view;
	psy_audio_Presets* presets;
	psy_audio_Machine* machine;
} MachineFrame;

void parameterbar_init(ParameterBar*, psy_ui_Component* parent);

void machineframe_init(MachineFrame*, psy_ui_Component* parent);
MachineFrame* machineframe_alloc(void);
MachineFrame* machineframe_allocinit(psy_ui_Component* parent);

void machineframe_setview(MachineFrame* self, psy_ui_Component* view, psy_audio_Machine*);

#endif

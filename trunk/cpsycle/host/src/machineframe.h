// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#if !defined(MACHINEFRAME_H)
#define MACHINEFRAME_H

#include "paramview.h"

#include <presets.h>
#include <uibutton.h>
#include <uicombobox.h>
#include <presets.h>
#include <machine.h>

typedef struct {
	ui_component component;
	ui_button mute;
	ui_combobox presetsbox;
	Presets* presets;
} ParameterBar;

typedef struct {
	ui_component component;
	ParameterBar parameterbar;
	ui_component* view;
	Presets* presets;
	Machine* machine;
} MachineFrame;

void parameterbar_init(ParameterBar*, ui_component* parent);

void machineframe_init(MachineFrame*, ui_component* parent);
void machineframe_setview(MachineFrame* self, ui_component* view, Machine*);

#endif

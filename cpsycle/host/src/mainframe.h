// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#if !defined(MAINFRAME_H)
#define MAINFRAME_H

#include "uicomponent.h"
#include "machinebar.h"
#include "machineview.h"
#include "patternview.h"
#include "sequenceview.h"
#include "settingsview.h"
#include "properties.h"
#include "noteinputs.h"

typedef struct {
	ui_component component;
	MachineBar machinebar;
	MachineView machineview;
	PatternView patternview;
	SequenceView sequenceview;
	SettingsView settingsview;
	ui_component* activeview;
	int cx;
	int cy;
	NoteInputs noteinputs;
} MainFrame;

void InitMainFrame(MainFrame* self, Properties* properties, Player* player);

#endif

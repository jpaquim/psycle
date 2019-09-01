// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#if !defined(MAINFRAME_H)
#define MAINFRAME_H

#include "uicomponent.h"
#include "machinebar.h"
#include "machineview.h"
#include "patternview.h"
#include "pianoroll.h"
#include "sequenceview.h"
#include "settingsview.h"
#include "properties.h"
#include "noteinputs.h"
#include "tabbar.h"
#include "playbar.h"
#include "vumeter.h"
#include "timebar.h"
#include "linesperbeatbar.h"

typedef struct {
	ui_component component;
	TabBar tabbar;
	MachineBar machinebar;
	PlayBar playbar;
	MachineView machineview;
	PatternView patternview;
	Pianoroll pianoroll;
	SequenceView sequenceview;
	SettingsView settingsview;
	Vumeter vumeter;
	TimeBar timebar;
	LinesPerBeatBar linesperbeatbar;
	ui_component* activeview;
	int cx;
	int cy;
	Player* player;
	NoteInputs noteinputs;
} MainFrame;

void InitMainFrame(MainFrame* self, Properties* properties, Player* player);

#endif

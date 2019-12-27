// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#if !defined(GEAR_H)
#define GEAR_H

#include "uibutton.h"
#include "uilistbox.h"
#include "instrumentsbox.h"
#include "samplesbox.h"
#include "machinesbox.h"
#include "uinotebook.h"
#include "tabbar.h"
#include "workspace.h"


typedef struct {
	psy_ui_Component component;
	psy_ui_Button createreplace;
	psy_ui_Button del;
	psy_ui_Button parameters;
	psy_ui_Button properties;
	psy_ui_Button exchange;
	psy_ui_Button clone;
	psy_ui_Button showmaster;
} GearButtons;

typedef struct {
	psy_ui_Component component;
	TabBar tabbar;
	ui_notebook notebook;
	ui_listbox listbox;	
	MachinesBox machinesboxgen;
	MachinesBox machinesboxfx;
	InstrumentsBox instrumentsbox;
	SamplesBox samplesbox;	
	GearButtons buttons;	
	psy_audio_Machines* machines;
	Workspace* workspace;
} Gear;

void gear_init(Gear*, psy_ui_Component* parent, Workspace*);

#endif

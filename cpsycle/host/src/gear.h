// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#if !defined(GEAR_H)
#define GEAR_H

// host
#include "instrumentsbox.h"
#include "machinesbox.h"
#include "samplesbox.h"
#include "tabbar.h"
#include "workspace.h"
// ui
#include "uibutton.h"
#include "uilistbox.h"
#include "uinotebook.h"

#ifdef __cplusplus
extern "C" {
#endif

// Gear
// The Gear Rack is a convenient way to organise your machines. Use the
// buttons "Generators", "Effects" and "Instruments" to switch between the
// different tabs

typedef struct GearButtons {
	// inherits
	psy_ui_Component component;
	// ui elements
	psy_ui_Button createreplace;
	psy_ui_Button del;
	psy_ui_Button parameters;
	psy_ui_Button properties;
	psy_ui_Button exchange;
	psy_ui_Button clone;
	psy_ui_Button showmaster;
} GearButtons;

void gearbuttons_init(GearButtons*, psy_ui_Component* parent, Workspace*);

INLINE psy_ui_Component* gearbuttons_base(GearButtons* self)
{
	return &self->component;
}

typedef struct Gear {
	// inherits
	psy_ui_Component component;
	// ui elements
	TabBar tabbar;
	psy_ui_Component titlebar;
	psy_ui_Label title;
	psy_ui_Button hide;
	psy_ui_Notebook notebook;
	psy_ui_ListBox listbox;	
	MachinesBox machinesboxgen;
	MachinesBox machinesboxfx;
	InstrumentsBox instrumentsbox;
	SamplesBox samplesbox;	
	GearButtons buttons;
	// references
	psy_audio_Machines* machines;
	Workspace* workspace;	
} Gear;

void gear_init(Gear*, psy_ui_Component* parent, Workspace*);

INLINE psy_ui_Component* gear_base(Gear* self)
{
	return &self->component;
}

#ifdef __cplusplus
}
#endif

#endif /* GEAR_H */

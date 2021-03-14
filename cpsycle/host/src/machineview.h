// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#if !defined(MACHINEVIEW_H)
#define MACHINEVIEW_H

// host
#include "machineframe.h"
#include "machineeditorview.h" // vst view
#include "machineviewskin.h"
#include "machineui.h"
#include "machinestackview.h"
#include "machinewireview.h"
#include "machineproperties.h"
#include "newmachine.h"
#include "paramview.h"
#include "tabbar.h"
#include "workspace.h"
// ui
#include <uiedit.h>
#include <uinotebook.h>
#include <uiscroller.h>

#ifdef __cplusplus
extern "C" {
#endif

// MachineView
//
// Editor/Viewer for the machines, their connections and owner of the
// wiremonitors. Composite of Wire-/Stackview and Newmachine.

// MachineStackPane: displays stacks of the machines starting with their leafs
// MachineWireView:  displays and edits machines with their wires
// NewMachine:       displays available machines to select the new machine
// MachineView:      tabbed view with MachineStack-/MachineWireView and
//                   NewMachine
// MachineViewBar:   Statusbar that displays actions in the wireview and adds
//                   a selector for the mixer, if new wires are added to the
//                   mixer inputs or sent/returns
// Helper
// MachineUi:        Draws a machine with its buttons and vumeter and reacts to
//                   ui events to solo/mute/bypass or pan the machine
// MachineUiMatrix:  Container of MachineUis for the MachineStackPane with
//                   track and line as key and a MachineUi pointer as value

// MachineView

struct MiniView;

typedef struct MachineView {
	// inherits
	psy_ui_Component component;
	// ui elements
	TabBar tabbar;
	MachineProperties properties;
	psy_ui_Notebook notebook;	
	MachineWireView wireview;
	psy_ui_Scroller scroller;
	MachineStackView stackview;	
	NewMachine newmachine;
	MachineViewSkin skin;
	// references
	Workspace* workspace;
} MachineView;

void machineview_init(MachineView*, psy_ui_Component* parent,
	psy_ui_Component* tabbarparent, Workspace*);

INLINE psy_ui_Component* machineview_base(MachineView* self)
{
	return &self->component;
}

#ifdef __cplusplus
}
#endif

#endif /* MACHINEVIEW_H */

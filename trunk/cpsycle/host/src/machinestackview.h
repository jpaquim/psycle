// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#if !defined(MACHINESTACKVIEW_H)
#define MACHINESTACKVIEW_H

// host
#include "machineframe.h"
#include "machineeditorview.h" // vst view
#include "machineviewskin.h"
#include "machineui.h"
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

// MachineStackPane: displays stacks of the machines starting with their leafs

// MachineStackInputs
typedef struct MachineStackDesc {
	// inherits
	psy_ui_Component component;
	// internal
	psy_ui_Label inputs;
	psy_ui_Label effects;
} MachineStackDesc;

void machinestackdesc_init(MachineStackDesc*, psy_ui_Component* parent);


// MachineStackInputs
typedef struct MachineStackInputs {
	// inherits
	psy_ui_Component component;	
	// references
	psy_audio_Machines* machines;
	Workspace* workspace;
	MachineViewSkin* skin;	
} MachineStackInputs;

void machinestackinputs_init(MachineStackInputs*, psy_ui_Component* parent,
	psy_audio_Machines*, MachineViewSkin* skin, Workspace* workspace);
void machinestackinputs_setmachines(MachineStackInputs*, psy_audio_Machines*);

// MachineStackPane
typedef struct MachineStackPane {
	// inherits
	psy_ui_Component component;
	// internal data
	psy_ui_Component* dragmachineui;
	bool vudrawupdate;
	uintptr_t opcount;
	// references
	psy_audio_Machines* machines;
	Workspace* workspace;
	MachineViewSkin* skin;	
} MachineStackPane;

void machinestackpane_init(MachineStackPane*, psy_ui_Component* parent,	
	MachineViewSkin* skin, Workspace*);

void machinestackpane_updateskin(MachineStackPane*);


// MachineStackVolumes
typedef struct MachineStackVolumes {
	// inherits
	psy_ui_Component component;
} MachineStackVolumes;

void machinestackvolumes_init(MachineStackVolumes*, psy_ui_Component* parent);

// MachineStackPaneTrack
typedef struct MachineStackPaneTrack {
	// inherits
	psy_ui_Component component;
} MachineStackPaneTrack;

void machinestackpanetrack_init(MachineStackPaneTrack*, psy_ui_Component* parent,
	psy_ui_Component* view);

// MachineStackPane
typedef struct MachineStackView {
	// inherits
	psy_ui_Component component;
	// internal data
	MachineStackDesc desc;
	MachineStackInputs inputs;
	MachineStackPane pane;
	MachineStackVolumes volumes;
	psy_ui_Scroller scroller;
} MachineStackView;

void machinestackview_init(MachineStackView*, psy_ui_Component* parent,
	psy_ui_Component* tabbarparent, MachineViewSkin*, Workspace*);

void machinestackview_updateskin(MachineStackView*);

#ifdef __cplusplus
}
#endif

#endif /* MACHINESTACKVIEW_H */

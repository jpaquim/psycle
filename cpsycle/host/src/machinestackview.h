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


// MachineStackColumn
struct psy_audio_MachineParam;
struct psy_audio_Machines;

typedef struct MachineStackColumn {
	uintptr_t column;
	psy_List* chain;	
	psy_audio_WireMachineParam* wirevolume;
} MachineStackColumn;

void machinestackcolumn_init(MachineStackColumn*,
	uintptr_t column);
void machinestackcolumn_dispose(MachineStackColumn*);

void machinestackcolumn_setwire(MachineStackColumn* self, psy_audio_Wire wire,
	struct psy_audio_Machines* machines);
psy_audio_WireMachineParam* machinestackcolumn_wire(MachineStackColumn*);

// MachineStackState
typedef struct MachineStackState {
	psy_Table columns;
	psy_audio_Machines* machines;
	uintptr_t selected;
} MachineStackState;

void machinestackstate_init(MachineStackState*);
void machinestackstate_dispose(MachineStackState*);

MachineStackColumn* machinestackstate_insertcolumn(MachineStackState*,
	uintptr_t column);
MachineStackColumn* machinestackstate_column(MachineStackState*,
	uintptr_t column);
void machinestackstate_clear(MachineStackState*);
psy_List* machinestackstate_inputs(MachineStackState*);

struct MachineStackView;

// MachineStackInputs
typedef struct MachineStackDesc {
	// inherits
	psy_ui_Component component;
	// internal
	psy_ui_Label inputs;
	psy_ui_Label effects;
	psy_ui_Label outputs;
	psy_ui_Label volumes;
	// references
	struct MachineStackView* view;
} MachineStackDesc;

void machinestackdesc_init(MachineStackDesc*, psy_ui_Component* parent,
	struct MachineStackView*);

// MachineStackInputs
typedef struct MachineStackInputs {
	// inherits
	psy_ui_Component component;	
	// references
	psy_audio_Machines* machines;
	Workspace* workspace;
	MachineViewSkin* skin;
	MachineStackState* state;
} MachineStackInputs;

void machinestackinputs_init(MachineStackInputs*, psy_ui_Component* parent,
	psy_audio_Machines*, MachineStackState* state, MachineViewSkin* skin,
	Workspace* workspace);
void machinestackinputs_setmachines(MachineStackInputs*, psy_audio_Machines*);


// MachineStackOutputs
typedef struct MachineStackOutputs {
	// inherits
	psy_ui_Component component;
	// references
	psy_audio_Machines* machines;
	Workspace* workspace;
	MachineViewSkin* skin;
	MachineStackState* state;
} MachineStackOutputs;

void machinestackoutputs_init(MachineStackOutputs*, psy_ui_Component* parent,
	psy_audio_Machines*, MachineStackState* state, MachineViewSkin* skin,
	Workspace* workspace);
void machinestackoutputs_setmachines(MachineStackOutputs*, psy_audio_Machines*);

// MachineStackPane
typedef struct MachineStackPane {
	// inherits
	psy_ui_Component component;
	// signals
	psy_Signal signal_changed;
	// internal data
	psy_ui_Component* dragmachineui;
	bool vudrawupdate;
	uintptr_t opcount;
	// references
	psy_audio_Machines* machines;
	Workspace* workspace;
	MachineViewSkin* skin;
	MachineStackState* state;
} MachineStackPane;

void machinestackpane_init(MachineStackPane*, psy_ui_Component* parent,
	MachineStackState*, MachineViewSkin*, Workspace*);

void machinestackpane_updateskin(MachineStackPane*);


// MachineStackVolumes
typedef struct MachineStackVolumes {
	// inherits
	psy_ui_Component component;
	// internal
	// references
	MachineStackState* state;
	Workspace* workspace;
	MachineViewSkin* skin;
} MachineStackVolumes;

void machinestackvolumes_init(MachineStackVolumes*, psy_ui_Component* parent,
	MachineStackState*, MachineViewSkin* skin, Workspace*);

void machinestackvolumes_build(MachineStackVolumes*);

// MachineStackPaneTrack
typedef struct MachineStackPaneTrack {
	// inherits
	psy_ui_Component component;
	// internal
	uintptr_t column;
	// References
	MachineStackState* state;
	Workspace* workspace;
} MachineStackPaneTrack;

void machinestackpanetrack_init(MachineStackPaneTrack*, psy_ui_Component* parent,
	uintptr_t column, psy_ui_Component* view,
	MachineStackState*, Workspace*);

// MachineStackPane
typedef struct MachineStackView {
	// inherits
	psy_ui_Component component;
	// internal data
	MachineStackDesc desc;
	MachineStackInputs inputs;
	MachineStackOutputs outputs;
	MachineStackPane pane;
	MachineStackVolumes volumes;
	psy_ui_Scroller scroller;
	MachineStackState state;
	// internal
	Workspace* workspace;
} MachineStackView;

void machinestackview_init(MachineStackView*, psy_ui_Component* parent,
	psy_ui_Component* tabbarparent, MachineViewSkin*, Workspace*);

void machinestackview_addeffect(MachineStackView*, psy_Property* plugininfo);

void machinestackview_updateskin(MachineStackView*);

#ifdef __cplusplus
}
#endif

#endif /* MACHINESTACKVIEW_H */

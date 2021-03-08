// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#if !defined(MACHINESTACKVIEW_H)
#define MACHINESTACKVIEW_H

// host
#include "machineviewskin.h"
#include "machineui.h"
#include "workspace.h"
// ui
#include <uiscroller.h>

#ifdef __cplusplus
extern "C" {
#endif

// MachineStackView: displays stacks of the machines starting with their leafs

// MasterRouteParam
typedef struct MasterRouteParam {
	// inherits
	psy_audio_MachineParam machineparam;
	// internal
	uintptr_t macid;
	// references
	psy_audio_Machines* machines;
} MasterRouteParam;

void masterrouteparam_init(MasterRouteParam*, psy_audio_Machines*);
void masterrouteparam_dispose(MasterRouteParam*);

INLINE psy_audio_MachineParam* masterrouteparam_base(
	MasterRouteParam* self)
{
	return &(self->machineparam);
}

// MachineStackColumn
struct psy_audio_MachineParam;
struct psy_audio_Machines;

typedef struct MachineStackColumn {
	uintptr_t column;
	psy_List* chain;	
	psy_audio_WireMachineParam* wirevolume;
	MasterRouteParam masterroute;
	psy_audio_Machines* machines;
} MachineStackColumn;

void machinestackcolumn_init(MachineStackColumn*,
	uintptr_t column, psy_audio_Machines*);
void machinestackcolumn_dispose(MachineStackColumn*);

void machinestackcolumn_setwire(MachineStackColumn*, psy_audio_Wire);

INLINE psy_audio_WireMachineParam* machinestackcolumn_wire(MachineStackColumn* self)
{
	return self->wirevolume;
}

void machinestackcolumn_append(MachineStackColumn*, uintptr_t macid);
uintptr_t machinestackcolumn_append_effect(MachineStackColumn*, psy_audio_Machine*);

uintptr_t machinestackcolumn_lastbeforemaster(const MachineStackColumn*);
bool machinestackcolumn_connectedtomaster(const MachineStackColumn*);

// MachineStackState
typedef struct MachineStackState {
	psy_Table columns;
	psy_audio_Machines* machines;
	uintptr_t selected;
	psy_ui_Size size;
	bool update;	
} MachineStackState;

void machinestackstate_init(MachineStackState*);
void machinestackstate_dispose(MachineStackState*);

void machinestackstate_buildcolumns(MachineStackState*);
void machinestackstate_setmachines(MachineStackState*, psy_audio_Machines*);
MachineStackColumn* machinestackstate_insertcolumn(MachineStackState*,
	uintptr_t column);
MachineStackColumn* machinestackstate_column(MachineStackState*,
	uintptr_t column);
MachineStackColumn* machinestackstate_selectedcolumn(MachineStackState*);
uintptr_t machinestackstate_maxnumcolumns(const MachineStackState*);
void machinestackstate_clear(MachineStackState*);
psy_List* machinestackstate_inputs(MachineStackState*);

INLINE void machinestackstate_rebuildview(MachineStackState* self)
{
	self->update = TRUE;
}

INLINE bool machinestackstate_rebuildingview(const MachineStackState* self)
{
	return self->update;
}

INLINE void machinestackstate_endviewbuild(MachineStackState* self)
{
	self->update = FALSE;
}

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
	Workspace* workspace;
	MachineViewSkin* skin;
	MachineStackState* state;
} MachineStackInputs;

void machinestackinputs_init(MachineStackInputs*, psy_ui_Component* parent,
	MachineStackState*, MachineViewSkin*, Workspace*);

void machinestackinputs_build(MachineStackInputs*);
void machinestackinputs_updatevus(MachineStackInputs*);

// MachineStackOutputs

struct ParamSkin;

typedef struct MachineStackOutputs {
	// inherits
	psy_ui_Component component;
	// references	
	Workspace* workspace;
	struct ParamSkin* skin;
	MachineStackState* state;
} MachineStackOutputs;

void machinestackoutputs_init(MachineStackOutputs*, psy_ui_Component* parent,
	MachineStackState* state, ParamSkin* skin);

void machinestackoutputs_build(MachineStackOutputs*);

// MachineStackPane
typedef struct MachineStackPane {
	// inherits
	psy_ui_Component component;	
	// internal data	
	bool vudrawupdate;
	uintptr_t opcount;
	// references	
	Workspace* workspace;
	MachineViewSkin* skin;
	MachineStackState* state;
} MachineStackPane;

void machinestackpane_init(MachineStackPane*, psy_ui_Component* parent,
	MachineStackState*, MachineViewSkin*, Workspace*);

void machinestackpane_build(MachineStackPane*);
void machinestackpane_updatevus(MachineStackPane*);
void machinestackpane_updateskin(MachineStackPane*);

// MachineStackVolumes
typedef struct MachineStackVolumes {
	// inherits
	psy_ui_Component component;
	// internal
	// references
	MachineStackState* state;	
	struct ParamSkin* skin;
} MachineStackVolumes;

void machinestackvolumes_init(MachineStackVolumes*, psy_ui_Component* parent,
	MachineStackState*, struct ParamSkin* skin);

void machinestackvolumes_build(MachineStackVolumes*);

// MachineStackPaneTrack
typedef struct MachineStackPaneTrack {
	// inherits
	psy_ui_Component component;
	// psy_ui_Component client;
	// psy_ui_Scroller scroller;
	// internal
	uintptr_t column;
	// References
	MachineStackState* state;
	Workspace* workspace;
} MachineStackPaneTrack;

void machinestackpanetrack_init(MachineStackPaneTrack*, psy_ui_Component* parent,
	uintptr_t column, psy_ui_Component* view,
	MachineStackState*, Workspace*);

// MachineStackView
typedef struct MachineStackView {
	// inherits
	psy_ui_Component component;
	// internal data
	MachineStackDesc desc;
	psy_ui_Component columns;
	MachineStackInputs inputs;
	MachineStackOutputs outputs;
	MachineStackPane pane;
	MachineStackVolumes volumes;	
	psy_ui_Scroller scroller_columns;
	MachineStackState state;
	// internal
	Workspace* workspace;
} MachineStackView;

void machinestackview_init(MachineStackView*, psy_ui_Component* parent,
	psy_ui_Component* tabbarparent, MachineViewSkin*, Workspace*);

void machinestackview_addeffect(MachineStackView*,
	const psy_audio_MachineInfo*);

void machinestackview_updateskin(MachineStackView*);

#ifdef __cplusplus
}
#endif

#endif /* MACHINESTACKVIEW_H */

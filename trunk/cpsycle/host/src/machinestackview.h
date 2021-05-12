// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#if !defined(MACHINESTACKVIEW_H)
#define MACHINESTACKVIEW_H

// host
#include "machineviewskin.h"
#include "machineui.h"
#include "headerui.h"
#include "paramviews.h"
#include "workspace.h"
// ui
#include <uibutton.h>
#include <uiscroller.h>

#ifdef __cplusplus
extern "C" {
#endif

// MachineStackView: displays stacks of the machines starting with their leafs

struct MachineStackState;

// OutputRouteParam
typedef struct OutputRouteParam {
	// inherits
	psy_audio_MachineParam machineparam;
	// internal
	uintptr_t column;	
	// references
	psy_audio_Machines* machines;
	struct MachineStackState* state;
} OutputRouteParam;

void outputrouteparam_init(OutputRouteParam*, psy_audio_Machines*,
	uintptr_t column, struct MachineStackState*);
void outputrouteparam_dispose(OutputRouteParam*);

INLINE psy_audio_MachineParam* outputrouteparam_base(
	OutputRouteParam* self)
{
	return &(self->machineparam);
}

// MachineStackColumn
struct psy_audio_MachineParam;
struct psy_audio_Machines;

typedef struct MachineStackColumn {
	uintptr_t column;	
	uintptr_t inputroute;
	uintptr_t input;
	uintptr_t offset;
	psy_List* chain;	
	psy_audio_WireMachineParam* wirevolume;
	psy_audio_IntMachineParam level_param;
	psy_audio_IntMachineParam mute_param;
	OutputRouteParam outputroute;	
	struct MachineStackState* state;
} MachineStackColumn;

void machinestackcolumn_init(MachineStackColumn*, uintptr_t column,
	uintptr_t inputroute, uintptr_t input, struct MachineStackState*);
void machinestackcolumn_dispose(MachineStackColumn*);

void machinestackcolumn_setwire(MachineStackColumn*, psy_audio_Wire);

psy_audio_Wire machinestackcolumn_outputwire(MachineStackColumn*);
psy_audio_Wire machinestackcolumn_sendwire(MachineStackColumn*);
INLINE psy_audio_WireMachineParam* machinestackcolumn_wire(MachineStackColumn* self)
{
	return self->wirevolume;
}

void machinestackcolumn_append(MachineStackColumn*, uintptr_t macid);
uintptr_t machinestackcolumn_lastbeforemaster(const MachineStackColumn*);
uintptr_t machinestackcolumn_lastbeforeroute(const MachineStackColumn*);
uintptr_t machinestackcolumn_lastbeforeindex(const MachineStackColumn*,
	uintptr_t index);
uintptr_t machinestackcolumn_last(const MachineStackColumn*);
uintptr_t machinestackcolumn_at(const MachineStackColumn*,
	uintptr_t index);
uintptr_t machinestackcolumn_nextindex(const MachineStackColumn*,
	uintptr_t index);
bool machinestackcolumn_connectedtomaster(const MachineStackColumn*);

// MachineStackState
typedef struct MachineStackState {
	psy_Table columns;
	psy_audio_Machines* machines;
	uintptr_t selected;
	uintptr_t effectinsertpos;
	bool effectinsertright;
	psy_ui_Size effectsize;
	psy_ui_Size effectsizesmall;
	psy_ui_Size columnsize;
	bool update;
	bool columnselected;
	bool preventrebuild;
	bool rewire;
	psy_audio_Wire oldwire;
	psy_audio_Wire newwire;
	uintptr_t currlevel;
	bool drawvirtualgenerators;
	uintptr_t insertmachinemode;
	bool drawsmalleffects;		
	ParamViews* paramviews;
} MachineStackState;

void machinestackstate_init(MachineStackState*, ParamViews*);
void machinestackstate_dispose(MachineStackState*);

void machinestackstate_buildcolumns(MachineStackState*);
void machinestackstate_setmachines(MachineStackState*, psy_audio_Machines*);
MachineStackColumn* machinestackstate_insertcolumn(MachineStackState*,
	uintptr_t column, uintptr_t inputroute, uintptr_t input);
MachineStackColumn* machinestackstate_column(MachineStackState*,
	uintptr_t column);
MachineStackColumn* machinestackstate_selectedcolumn(MachineStackState*);
uintptr_t machinestackstate_maxnumcolumns(const MachineStackState*);
void machinestackstate_clear(MachineStackState*);
psy_List* machinestackstate_inputs(MachineStackState*);
psy_List* machinestackstate_buses(MachineStackState*);
void machinestackstate_check_wirechange(MachineStackState*);

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

// MachineStackDesc
typedef struct MachineStackDesc {
	// inherits
	psy_ui_Component component;
	// internal
	psy_ui_Label inputs;	
	psy_ui_Button effects;
	psy_ui_Button outputs;
	psy_ui_Button volumes;
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


// MachineStackPaneTrackClient
typedef struct MachineStackPaneTrackClient {
	psy_ui_Component component;
	// internal
	uintptr_t column;
	// references
	MachineStackState* state;
} MachineStackPaneTrackClient;

void machinestackpanetrackclient_init(MachineStackPaneTrackClient*,
	psy_ui_Component* parent, psy_ui_Component* view, uintptr_t column,
	MachineStackState*);

// MachineStackPaneTrack
typedef struct MachineStackPaneTrack {
	// inherits
	psy_ui_Component component;
	MachineStackPaneTrackClient client;
	psy_ui_Scroller scroller;
	// internal
	uintptr_t column;	
	// References
	MachineStackState* state;
	// references
	Workspace* workspace;	
} MachineStackPaneTrack;

void machinestackpanetrack_init(MachineStackPaneTrack*,
	psy_ui_Component* parent, psy_ui_Component* view, uintptr_t column,
	MachineStackState*, Workspace*);

MachineStackPaneTrack* machinestackpanetrack_alloc(void);
MachineStackPaneTrack* machinestackpanetrack_allocinit(
	psy_ui_Component* parent, psy_ui_Component* view, uintptr_t column,
	MachineStackState*, Workspace*);

// MachineStackView
typedef struct MachineStackView {
	// inherits
	psy_ui_Component component;
	// internal
	MachineStackDesc desc;
	psy_ui_Component columns;
	MachineStackInputs inputs;	
	MachineStackPane pane;
	psy_ui_Component spacer;	
	MachineStackOutputs outputs;
	MachineStackVolumes volumes;	
	psy_ui_Scroller scroller_columns;
	MachineStackState state;
	// references
	Workspace* workspace;	
	ParamViews* paramviews;
} MachineStackView;

void machinestackview_init(MachineStackView*, psy_ui_Component* parent,
	psy_ui_Component* tabbarparent, MachineViewSkin*,
	ParamViews*, Workspace*);

void machinestackview_addeffect(MachineStackView*,
	const psy_audio_MachineInfo*);
void machinestackview_updateskin(MachineStackView*);
void machinestackview_showvirtualgenerators(MachineStackView*);
void machinestackview_hidevirtualgenerators(MachineStackView*);
void machinestackview_idle(MachineStackView*);
void machinestackview_drawsmalleffects(MachineStackView*);
void machinestackview_drawfulleffects(MachineStackView*);

#ifdef __cplusplus
}
#endif

#endif /* MACHINESTACKVIEW_H */

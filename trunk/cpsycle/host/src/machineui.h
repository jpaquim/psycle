// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#if !defined(MACHINEUI_H)
#define MACHINEUI_H

// host
#include "machineeditorview.h" // vst view
#include "machineframe.h"
#include "machineviewskin.h"
#include "paramviews.h"
#include "workspace.h"
// ui
#include <uitextinput.h>
#include <uinotebook.h>

#ifdef __cplusplus
extern "C" {
#endif

// Components to draw a machine with its buttons and vumeter and reacting to
// ui events to solo/mute/bypass or pan the machine

typedef enum MachineViewDragMode {
	MACHINEVIEW_DRAG_NONE,
	MACHINEVIEW_DRAG_MACHINE,
	MACHINEVIEW_DRAG_NEWCONNECTION,
	MACHINEVIEW_DRAG_LEFTCONNECTION,
	MACHINEVIEW_DRAG_RIGHTCONNECTION,
	MACHINEVIEW_DRAG_PAN
} MachineViewDragMode;

typedef struct VuValues {
	/// output level for display (0 .. 1.f)	
	psy_dsp_amp_t volumedisplay;
	/// output peak level for display (0 .. 1.f)
	psy_dsp_amp_t volumemaxdisplay;
	/// output peak level display time (refreshrate * 60)
	int volumemaxcounterlife;
} VuValues;

void vuvalues_init(VuValues*);
void vuvalues_tickcounter(VuValues*);
void vuvalues_update(VuValues*, psy_audio_Buffer*);

typedef struct VuDisplay {
	// internal	
	VuValues vuvalues;
	// references
	MachineViewSkin* skin;
	MachineCoords* coords;
} VuDisplay;

void vudisplay_init(VuDisplay*,
	MachineViewSkin*, MachineCoords*);

void vudisplay_update(VuDisplay*, psy_audio_Buffer*);
void vudisplay_draw(VuDisplay*, psy_ui_Graphics*);

typedef enum MachineUiMode {
	MACHINEUIMODE_DRAW = 1,
	MACHINEUIMODE_DRAWSMALL,
	MACHINEUIMODE_BITMAP
} MachineUiMode;

/* MachineUiCommon */
typedef struct MachineUiCommon {
	/* internal */
	int mode;
	psy_ui_Colour font;
	psy_ui_Colour bgcolour;
	VuDisplay vu;
	uintptr_t slot;	
	char* restorename;
	bool machinepos;
	MachineViewDragMode dragmode;
	MachineUiMode drawmode;
	double mx;
	/* references */
	Workspace* workspace;
	psy_audio_Machine* machine;
	psy_audio_Machines* machines;
	psy_ui_Component* component;	
	MachineCoords* coords;
	MachineViewSkin* skin;
	ParamViews* paramviews;
} MachineUiCommon;

void machineuicommon_init(MachineUiCommon*, psy_ui_Component* component,
	uintptr_t slot, MachineViewSkin*, ParamViews*, Workspace*);

void machineuicommon_move(MachineUiCommon*, psy_ui_Point topleft);

/* global methods */
psy_ui_Component* machineui_create(psy_audio_Machine* machine,
	uintptr_t slot, MachineViewSkin* skin,
	psy_ui_Component* parent, psy_ui_Component* view,
	ParamViews* paramviews, bool machinepos, MachineUiMode drawmode,
	Workspace* workspace);

void machineui_drawhighlight(psy_ui_Graphics* g, psy_ui_RealRectangle position);

// vudraw optimization
// sets a global vu update flag
// machineui overwrites psy_ui_component_invalidate and if the flag is set
// the vumeter position is invalidated else all of the machineui
// methods called in ontimer of the wire-/stackview
void machineui_beginvuupdate(void);
void machineui_endvuupdate(void);
bool machineui_vuupdate(void);

#ifdef __cplusplus
}
#endif

#endif /* MACHINEUI_H */

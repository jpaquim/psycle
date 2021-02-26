// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#if !defined(MACHINEUI_H)
#define MACHINEUI_H

// host
#include "machineeditorview.h" // vst view
#include "machineframe.h"
#include "machineviewskin.h"
#include "workspace.h"
// ui
#include <uiedit.h>
#include <uinotebook.h>

#ifdef __cplusplus
extern "C" {
#endif

// MachineUi

// Draws a machine with its buttons and vumeter and reacts to
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
	psy_ui_RealRectangle position;
	VuValues vuvalues;
	// references
	MachineViewSkin* skin;
	MachineCoords* coords;
} VuDisplay;

void vudisplay_init(VuDisplay*,
	MachineViewSkin*, MachineCoords*);

void vudisplay_update(VuDisplay*, psy_audio_Buffer*);
void vudisplay_draw(VuDisplay*, psy_ui_Graphics*);

typedef struct MachineUiCommon {
	// internal
	int mode;
	psy_ui_Colour font;
	psy_ui_Colour bgcolour;
	VuDisplay vu;
	uintptr_t slot;
	MachineFrame* machineframe;
	ParamView* paramview;
	MachineEditorView* editorview;
	char* restorename;
	bool machinepos;
	MachineViewDragMode dragmode;
	double mx;
	// references
	Workspace* workspace;
	psy_audio_Machine* machine;
	psy_audio_Machines* machines;
	psy_ui_Component* view;
	psy_ui_Edit* editname;
	MachineCoords* coords;
	MachineViewSkin* skin;
} MachineUiCommon;

void machineuicommon_init(MachineUiCommon*, uintptr_t slot, MachineViewSkin*,
	psy_ui_Component* view, psy_ui_Edit* editname, Workspace*);

// MasterUi
typedef struct MasterUi {
	// inherits
	psy_ui_Component component;
	// internal
	MachineUiCommon intern;
} MasterUi;

void masterui_init(MasterUi*, MachineViewSkin*, psy_ui_Component* view,
	Workspace*);

// MachineUi
typedef struct MachineUi {
	// inherits
	psy_ui_Component component;
	// internal
	MachineUiCommon intern;
} MachineUi;

void machineui_init(MachineUi*, uintptr_t slot, MachineViewSkin*,
	psy_ui_Component* view, psy_ui_Edit* editname, Workspace*);

// vudraw optimization
// sets a global vu update flag
// machineui overwrites psy_ui_component_invalidate and if the flag is set
// the vumeter position is invalidated else all of the machineui
// methods called in ontimer of the wire-/stackview
void machineui_beginvuupdate(void);
void machineui_endvuupdate(void);


#ifdef __cplusplus
}
#endif

#endif /* MACHINEUI_H */

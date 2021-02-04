// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#if !defined(MACHINEUI_H)
#define MACHINEUI_H

// host
#include "machineframe.h"
#include "machineeditorview.h" // vst view
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

// MachineUi
typedef struct MachineUi {
	int mode;
	/// output level for display (0 .. 1.f)	
	psy_dsp_amp_t volumedisplay;
	/// output peak level for display (0 .. 1.f)
	psy_dsp_amp_t volumemaxdisplay;
	/// output peak level display time (refreshrate * 60)
	int volumemaxcounterlife;
	MachineCoords* coords;
	MachineViewSkin* skin;
	psy_audio_Machine* machine;
	psy_ui_Colour font;
	psy_ui_Colour bgcolour;
	psy_ui_RealRectangle position;
	psy_ui_RealRectangle vuposition;
	psy_ui_RealPoint topleft;
	uintptr_t slot;
	MachineFrame* machineframe;
	ParamView* paramview;
	MachineEditorView* editorview;
	char* restorename;
	bool machinepos;
	MachineViewDragMode dragmode;
	// references
	Workspace* workspace;
	psy_audio_Machines* machines;
	psy_ui_Component* view;
	double mx;
} MachineUi;

void machineui_init(MachineUi*, uintptr_t slot, MachineViewSkin*,
	psy_ui_Component* view, Workspace*);
void machineui_dispose(MachineUi*);

void machineui_update(MachineUi*);
void machineui_onframedestroyed(MachineUi*, psy_ui_Component* sender);
	psy_ui_RealSize machineui_size(const MachineUi*);
void machineui_move(MachineUi*, psy_ui_RealPoint dest);
const psy_ui_RealRectangle* machineui_position(const MachineUi*);
psy_ui_RealRectangle machineui_coordposition(MachineUi*, SkinCoord*);
psy_ui_RealPoint machineui_centerposition(MachineUi*);
void machineui_draw(MachineUi*, psy_ui_Graphics*, uintptr_t slot,
	bool vuupdate);
void machineui_drawbackground(MachineUi*, psy_ui_Graphics*);
void machineui_draweditname(MachineUi*, psy_ui_Graphics*);
void machineui_drawpanning(MachineUi*, psy_ui_Graphics*);
void machineui_drawmute(MachineUi*, psy_ui_Graphics*);
void machineui_drawbypassed(MachineUi*, psy_ui_Graphics*);
void machineui_drawsoloed(MachineUi*, psy_ui_Graphics*,
	psy_audio_Machines*);
void machineui_drawvu(MachineUi*, psy_ui_Graphics*);
void machineui_drawvudisplay(MachineUi*, psy_ui_Graphics*);
void machineui_drawvupeak(MachineUi*, psy_ui_Graphics*);
void machineui_drawhighlight(MachineUi*, psy_ui_Graphics*);
void machineui_updatevolumedisplay(MachineUi*);
void machineui_updatemaxvolumedisplay(MachineUi*);
void machineui_showparameters(MachineUi*, psy_ui_Component* parent);
void machineui_editname(MachineUi*, psy_ui_Edit*,
	psy_ui_RealPoint scrolloffset);
void machineui_onkeydown(MachineUi*, psy_ui_Component* sender,
	psy_ui_KeyEvent*);
void machineui_oneditchange(MachineUi*, psy_ui_Edit* sender);
void machineui_oneditfocuslost(MachineUi*, psy_ui_Component* sender);
void machineui_invalidate(MachineUi*, bool vuupdate);
void machineui_onmousedown(MachineUi*, psy_ui_MouseEvent*);
void machineui_onmousemove(MachineUi*, psy_ui_MouseEvent*);
void machineui_onmouseup(MachineUi*, psy_ui_MouseEvent*);
void machineui_onmousedoubleclick(MachineUi*, psy_ui_MouseEvent*);
bool machineui_hittestcoord(MachineUi*, psy_ui_RealPoint, int mode,
	SkinCoord*);
int machineui_hittestpan(MachineUi*, psy_ui_RealPoint, uintptr_t slot,
	double* dx);
psy_dsp_amp_t machineui_panvalue(MachineUi*, double dx, uintptr_t slot);


#ifdef __cplusplus
}
#endif

#endif /* MACHINEUI_H */

// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#if !defined(MACHINEVIEW_H)
#define MACHINEVIEW_H

// host
#include "machineframe.h"
#include "machineeditorview.h" // vst view
#include "machineviewskin.h"
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
// wiremonitors

// MachineStackView: displays stacks of the machines starting with their leafs
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
// MachineUiMatrix:  Container of MachineUis for the MachineStackView with
//                   track and line as key and a MachineUi pointer as value


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

// MachineUiMatrix
typedef psy_Table MachineUiTrack;

typedef struct MachineUiMatrix {
	psy_Table tracks;
	uintptr_t maxlines;
} MachineUiMatrix;

void machineuimatrix_init(MachineUiMatrix*);
void machineuimatrix_dispose(MachineUiMatrix*);

void machineuimatrix_insert(MachineUiMatrix*, uintptr_t trackidx,
	uintptr_t line, MachineUi*);
MachineUi* machineuimatrix_at(MachineUiMatrix*, uintptr_t trackidx,
	uintptr_t line);
uintptr_t machineuimatrix_numtracks(const MachineUiMatrix*);
uintptr_t machineuimatrix_numlines(const MachineUiMatrix*);

// MachineStackView
typedef struct MachineStackView {
	// inherits
	psy_ui_Component component;
	// internal data
	MachineUiMatrix matrix;
	psy_Table maxlevels;	
	uintptr_t dragslot;
	MachineUi* dragmachineui;
	bool vudrawupdate;
	uintptr_t opcount;
	// references
	psy_audio_Machines* machines;	
	Workspace* workspace;
	MachineViewSkin* skin;	
} MachineStackView;

void machinestackview_init(MachineStackView*, psy_ui_Component* parent,
	psy_ui_Component* tabbarparent, psy_ui_Scroller* scroller,
	MachineViewSkin* skin, Workspace*);

// MachineWireView

typedef struct MachineWireView {
	// inherits
	psy_ui_Component component;
	// internal data
	psy_Table machineuis;
	psy_List* wireframes;
	psy_ui_RealPoint dragpt;	
	uintptr_t dragslot;
	MachineUi* dragmachineui;
	MachineViewDragMode dragmode;
	uintptr_t selectedslot;	
	psy_audio_Wire dragwire;	
	psy_audio_Wire selectedwire;
	psy_audio_Wire hoverwire;	
	psy_audio_PluginCatcher plugincatcher;	
	psy_ui_Edit editname;
	int randominsert;
	int addeffect;
	bool mousemoved;
	bool showwirehover;
	bool vudrawupdate;
	bool drawvirtualgenerators;
	uintptr_t opcount;
	// references
	struct MachineViewBar* statusbar;
	psy_audio_Machines* machines;	
	Workspace* workspace;	
	MachineViewSkin* skin;
} MachineWireView;

void machinewireview_init(MachineWireView*, psy_ui_Component* parent,
	psy_ui_Component* tabbarparent, MachineViewSkin*, Workspace*);
void machinewireview_centermaster(MachineWireView*);
void machinewireview_configure(MachineWireView*, MachineViewConfig*);
void machinewireview_showvirtualgenerators(MachineWireView*);
void machinewireview_hidevirtualgenerators(MachineWireView*);

INLINE psy_ui_Component* machinewireview_base(MachineWireView* self)
{
	return &self->component;
}

// MachineViewBar
typedef struct MachineViewBar {
	// inherits
	psy_ui_Component component;
	// internal
	// ui elements
	psy_ui_CheckBox mixersend;
	psy_ui_Label status;
	// references	
	psy_audio_Machines* machines;
} MachineViewBar;

void machineviewbar_init(MachineViewBar*, psy_ui_Component* parent, Workspace*);

void machineviewbar_settext(MachineViewBar*, const char* text);

INLINE psy_ui_Component* machineviewbar_base(MachineViewBar* self)
{
	return &self->component;
}

// MachineView
typedef struct MachineView {
	// inherits
	psy_ui_Component component;
	// ui elements
	TabBar tabbar;
	psy_ui_Notebook notebook;	
	MachineWireView wireview;
	psy_ui_Scroller scroller;
	MachineStackView stackview;
	psy_ui_Scroller stackscroller;
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

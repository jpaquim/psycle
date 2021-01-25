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

// MachineWireView
//
// Editor for the machines and their connections

// MachineView
//
// tabbed view for MachineWireView and NewMachine 

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
	MachineFrame* frame;
	ParamView* paramview;
	MachineEditorView* editorview;
	char* restorename;
	// references
	Workspace* workspace;
} MachineUi;

enum {	
	MACHINEWIREVIEW_DRAG_NONE,
	MACHINEWIREVIEW_DRAG_MACHINE,
	MACHINEWIREVIEW_DRAG_NEWCONNECTION,
	MACHINEWIREVIEW_DRAG_LEFTCONNECTION,
	MACHINEWIREVIEW_DRAG_RIGHTCONNECTION,
	MACHINEWIREVIEW_DRAG_PAN,
};

typedef struct MachineWireView {
	// inherits
	psy_ui_Component component;
	// internal data
	psy_Table machineuis;
	psy_List* wireframes;	
	double mx;
	double my;
	uintptr_t dragslot;
	int dragmode;
	uintptr_t selectedslot;	
	psy_audio_Wire dragwire;	
	psy_audio_Wire selectedwire;
	psy_audio_Wire hoverwire;	
	psy_audio_PluginCatcher plugincatcher;
	MachineViewSkin skin;	
	psy_ui_Edit editname;
	int randominsert;
	int addeffect;
	bool mousemoved;
	bool showwirehover;
	bool vudrawupdate;
	bool drawvirtualgenerators;
	// references
	struct MachineViewBar* statusbar;
	psy_audio_Machines* machines;
	psy_ui_Scroller* scroller;
	Workspace* workspace;
} MachineWireView;

void machinewireview_init(MachineWireView*, psy_ui_Component* parent,
	psy_ui_Component* tabbarparent, psy_ui_Scroller* scroller, Workspace*);
void machinewireview_centermaster(MachineWireView*);
void machinewireview_configure(MachineWireView*, MachineViewConfig*);

INLINE psy_ui_Component* machinewireview_base(MachineWireView* self)
{
	return &self->component;
}

// MachineViewBar
typedef struct MachineViewBar {
	// inherits
	psy_ui_Component component;
	// ui elements
	psy_ui_CheckBox mixersend;
	psy_ui_Label status;
	// references
	Workspace* workspace;
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
	NewMachine newmachine;
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

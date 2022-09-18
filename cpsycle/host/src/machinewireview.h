/*
** This source is free software; you can redistribute itand /or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#if !defined(MACHINEWIREVIEW_H)
#define MACHINEWIREVIEW_H

/* host */
#include "machineframe.h"
#include "machineeditorview.h" /* vst view */
#include "machineui.h"
#include "machineviewmenu.h"
#include "machinestackview.h"
#include "newmachine.h"
#include "paramview.h"
#include "paramviews.h"
#include "wireframes.h"
#include "workspace.h"
/* ui */
#include <uitabbar.h>
#include <uitextarea.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
** MachineWirePane
**
** Displays and edits machines with their wires
*/


/* MachineWirePaneUis */

typedef struct MachineWirePaneUis {
	psy_Table machineuis;
	psy_ui_Component* view;
	psy_audio_Machines* machines;	
	ParamViews* paramviews;
	Workspace* workspace;
} MachineWirePaneUis;

void machinewireviewuis_init(MachineWirePaneUis*, psy_ui_Component*,
	ParamViews*, Workspace*);
void machinewireviewuis_dispose(MachineWirePaneUis*);

psy_ui_Component* machinewireviewwuis_at(MachineWirePaneUis*,
	uintptr_t slot);
void machinewireviewuis_remove(MachineWirePaneUis*, uintptr_t slot);
void machinewireviewuis_redrawvus(MachineWirePaneUis*);

/* MachineWirePane */
typedef struct MachineWirePane {
	/* inherits */
	psy_ui_Component component;
	/* internal */
	MachineWirePaneUis machineuis;
	WireFrames wireframes;
	psy_ui_RealPoint dragpt;
	uintptr_t dragslot;
	psy_ui_Component* dragmachineui;
	MachineViewDragMode dragmode;
	uintptr_t selectedslot;	
	psy_audio_Wire dragwire;	
	psy_audio_Wire hoverwire;	
	int randominsert;	
	bool mousemoved;
	bool showwirehover;	
	bool drawvirtualgenerators;	
	bool centermaster;	
	/* references */
	psy_audio_Machines* machines;	
	Workspace* workspace;		
	ParamViews* paramviews;
	MachineMenu* machine_menu;
} MachineWirePane;

void machinewirepane_init(MachineWirePane*, psy_ui_Component* parent,
	psy_ui_Component* tabbarparent, ParamViews*, MachineMenu*,
	Workspace*);

void machinewirepane_center_master(MachineWirePane*);
void machinewirepane_showvirtualgenerators(MachineWirePane*);
void machinewirepane_hidevirtualgenerators(MachineWirePane*);
void machinewirepane_idle(MachineWirePane*);

INLINE psy_ui_Component* machinewirepane_base(MachineWirePane* self)
{
	return &self->component;
}

/* MachineWireView */
typedef struct MachineWireView {
	/* inherits */	
	psy_ui_Scroller scroller;
	/* internal */
	MachineWirePane pane;
} MachineWireView;

void machinewireview_init(MachineWireView*, psy_ui_Component* parent,
	psy_ui_Component* tabbarparent, ParamViews*, MachineMenu*, Workspace*);

INLINE void machinewireview_center_master(MachineWireView* self)
{
	machinewirepane_center_master(&self->pane);
}

INLINE void machinewireview_showvirtualgenerators(MachineWireView* self)
{
	machinewirepane_showvirtualgenerators(&self->pane);
}

INLINE void machinewireview_hidevirtualgenerators(MachineWireView* self)
{
	machinewirepane_hidevirtualgenerators(&self->pane);
}

INLINE void machinewireview_idle(MachineWireView* self)
{
	machinewirepane_idle(&self->pane);
}

INLINE psy_ui_Component* machinewireview_base(MachineWireView* self)
{
	return psy_ui_scroller_base(&self->scroller);
}

#ifdef __cplusplus
}
#endif

#endif /* MACHINEWIREVIEW_H */

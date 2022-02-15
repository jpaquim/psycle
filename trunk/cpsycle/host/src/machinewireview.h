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
#include "machinestackview.h"
#include "newmachine.h"
#include "paramview.h"
#include "paramviews.h"
#include "workspace.h"
/* ui */
#include <uitabbar.h>
#include <uitextinput.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
** MachineWireView
**
** Displays and edits machines with their wires
*/

/* MachineWireViewUis */

typedef struct MachineWireViewUis {
	psy_Table machineuis;
	psy_ui_Component* view;
	psy_audio_Machines* machines;	
	ParamViews* paramviews;
	Workspace* workspace;
} MachineWireViewUis;

void machinewireviewuis_init(MachineWireViewUis*, psy_ui_Component*,
	ParamViews*, Workspace*);
void machinewireviewuis_dispose(MachineWireViewUis*);

psy_ui_Component* machinewireviewwuis_at(MachineWireViewUis*,
	uintptr_t slot);
void machinewireviewuis_remove(MachineWireViewUis*, uintptr_t slot);
void machinewireviewuis_redrawvus(MachineWireViewUis*);

/* MachineWireView */
typedef struct MachineWireView {
	/* inherits */
	psy_ui_Component component;
	/* internal */
	MachineWireViewUis machineuis;
	psy_List* wireframes;
	psy_ui_RealPoint dragpt;
	uintptr_t dragslot;
	psy_ui_Component* dragmachineui;
	MachineViewDragMode dragmode;
	uintptr_t selectedslot;	
	psy_audio_Wire dragwire;	
	psy_audio_Wire hoverwire;	
	psy_audio_PluginCatcher plugincatcher;	
	int randominsert;
	int addeffect;
	bool mousemoved;
	bool showwirehover;	
	bool drawvirtualgenerators;
	uintptr_t opcount;
	bool centermaster;	
	/* references */
	psy_audio_Machines* machines;	
	Workspace* workspace;		
	ParamViews* paramviews;
} MachineWireView;

void machinewireview_init(MachineWireView*, psy_ui_Component* parent,
	psy_ui_Component* tabbarparent, ParamViews*, Workspace*);

void machinewireview_centermaster(MachineWireView*);
void machinewireview_showvirtualgenerators(MachineWireView*);
void machinewireview_hidevirtualgenerators(MachineWireView*);
void machinewireview_idle(MachineWireView*);

INLINE psy_ui_Component* machinewireview_base(MachineWireView* self)
{
	return &self->component;
}

#ifdef __cplusplus
}
#endif

#endif /* MACHINEWIREVIEW_H */

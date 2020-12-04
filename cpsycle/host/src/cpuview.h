// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#if !defined(CPUVIEW_H)
#define CPUVIEW_H

// host
#include "labelpair.h"
#include "workspace.h"
// ui
#include <uicheckbox.h>
#include <uiscroller.h>

#ifdef __cplusplus
extern "C" {
#endif

// CPUView
//
// Displays information of the cpu and memory usage of the system and
// for each machine of a song
//
// todo: uses windows api methods to access process information, implement
//       unix platform informations aswell
//       machine cpu accumulating not finished for now

typedef struct CPUModuleView {
	// inherits
	psy_ui_Component component;
	// references
	Workspace* workspace;
} CPUModuleView;

void cpumoduleview_init(CPUModuleView*, psy_ui_Component* parent,
	Workspace* workspace);

typedef struct CPUView {
	// inherits
	psy_ui_Component component;
	// ui elements
	psy_ui_Component titlebar;
	psy_ui_Label title;
	psy_ui_Button hide;
	psy_ui_Label coreinfo;
	psy_ui_Component top;
	psy_ui_Margin topmargin;
	psy_ui_Component resources;
	psy_ui_Label resourcestitle;
	LabelPair resources_win;
	LabelPair resources_mem;
	LabelPair resources_swap;
	LabelPair resources_vmem;		
	psy_ui_Component performance;
	LabelPair audiothreads;
	LabelPair totaltime;
	LabelPair machines;
	LabelPair routing;
	psy_ui_CheckBox cpucheck;
	CPUModuleView modules;
	psy_ui_Scroller scroller;	
	// internal data
	uintptr_t lastnummachines;
	// references
	Workspace* workspace;	
} CPUView;

void cpuview_init(CPUView*, psy_ui_Component* parent, Workspace*);

INLINE psy_ui_Component* cpuview_base(CPUView* self)
{
	return &self->component;
}

#ifdef __cplusplus
}
#endif

#endif /* CPUVIEW_H */

// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#if !defined(NEWMACHINESECTION_H)
#define NEWMACHINESECTION_H

// host
#include "pluginsview.h"
#include "workspace.h"
// ui
#include <uiedit.h>
#include <uilabel.h>

#ifdef __cplusplus
extern "C" {
#endif

struct NewMachine;

typedef struct NewMachineSection {
	// inherits
	psy_ui_Component component;
	// signals
	psy_Signal signal_selected;
	psy_Signal signal_changed;
	psy_Signal signal_renamed;	
	// intern
	psy_ui_Component header;
	psy_ui_Label label;
	PluginsView pluginview;
	bool preventedit;
	// references
	psy_Property* section;
	psy_ui_Edit* edit;
	Workspace* workspace;
	NewMachineFilter* filter;
} NewMachineSection;

void newmachinesection_init(NewMachineSection* self, psy_ui_Component* parent,
	psy_Property* section, psy_ui_Edit*, NewMachineFilter* filter, Workspace*);

NewMachineSection* newmachinesection_alloc(void);
NewMachineSection* newmachinesection_allocinit(psy_ui_Component* parent,
	psy_Property* section, psy_ui_Edit*, NewMachineFilter* filter, Workspace*);

void newmachinesection_findplugins(NewMachineSection*);
const char* newmachinesection_key(const NewMachineSection*);
const char* newmachinesection_name(const NewMachineSection*);
void newmachinesection_mark(NewMachineSection*);
void newmachinsection_clearselection(NewMachineSection*);

#ifdef __cplusplus
}
#endif

#endif /* NEWMACHINESECTION_H */

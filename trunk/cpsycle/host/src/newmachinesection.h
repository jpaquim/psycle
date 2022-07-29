/*
** This source is free software; you can redistribute itand /or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#if !defined(NEWMACHINESECTION_H)
#define NEWMACHINESECTION_H

/* host */
#include "pluginsview.h"
#include "workspace.h"
/* ui */
#include <uitextarea.h>
#include <uilabel.h>

#ifdef __cplusplus
extern "C" {
#endif

struct NewMachine;

typedef struct NewMachineSection {
	/* inherits */
	psy_ui_Component component;
	/* signals */
	psy_Signal signal_selected;
	psy_Signal signal_changed;
	psy_Signal signal_renamed;	
	/* internal */
	psy_ui_Component header;
	psy_ui_TextArea name;
	PluginsView pluginview;	
	/* references */
	psy_Property* section;	
	Workspace* workspace;
	PluginFilter* filter;
} NewMachineSection;

void newmachinesection_init(NewMachineSection* self, psy_ui_Component* parent,
	psy_Property* section, PluginFilter* filter, Workspace*);

NewMachineSection* newmachinesection_alloc(void);
NewMachineSection* newmachinesection_allocinit(psy_ui_Component* parent,
	psy_Property* section, PluginFilter* filter, Workspace*);

void newmachinesection_find_plugins(NewMachineSection*);
const char* newmachinesection_key(const NewMachineSection*);
const char* newmachinesection_name(const NewMachineSection*);
void newmachinesection_mark(NewMachineSection*);
void newmachinsection_clear_selection(NewMachineSection*);

#ifdef __cplusplus
}
#endif

#endif /* NEWMACHINESECTION_H */

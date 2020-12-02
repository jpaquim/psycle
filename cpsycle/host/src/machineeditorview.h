// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net


#if !defined(MACHINEEDITORVIEW_H)
#define MACHINEEDITORVIEW_H

#include <uicomponent.h>
#include <machine.h>

#include "workspace.h"

#ifdef __cplusplus
extern "C" {
#endif

// Displays a plugin custom ui. So far only vsts use this view.

typedef struct {
   psy_ui_Component component;   
   psy_audio_Machine* machine;      
} MachineEditorView;

void machineeditorview_init(MachineEditorView*, psy_ui_Component* parent,
	psy_audio_Machine*, Workspace* workspace);
MachineEditorView* machineeditorview_alloc(void);
MachineEditorView* machineeditorview_allocinit(psy_ui_Component* parent,
	psy_audio_Machine*, Workspace*);

#ifdef __cplusplus
}
#endif

#endif /* MACHINEEDITORVIEW_H */

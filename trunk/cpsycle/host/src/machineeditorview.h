// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net


#if !defined(MACHINEEDITORVIEW_H)
#define MACHINEEDITORVIEW_H

#include <uicomponent.h>
#include <machine.h>

#include "workspace.h"

#ifdef __cplusplus
extern "C" {
#endif

// Displays a plugin custom ui instead the paramview. So far only vsts use
// this view.

typedef struct MachineEditorView {
	// inherits
   psy_ui_Component component;
   // references
   psy_audio_Machine* machine;
   Workspace* workspace;
} MachineEditorView;

void machineeditorview_init(MachineEditorView*, psy_ui_Component* parent,
	psy_audio_Machine*, Workspace* workspace);
MachineEditorView* machineeditorview_alloc(void);
MachineEditorView* machineeditorview_allocinit(psy_ui_Component* parent,
	psy_audio_Machine*, Workspace*);

INLINE psy_ui_Component* machineeditorview_base(MachineEditorView* self)
{
	assert(self);

	return &self->component;
}

#ifdef __cplusplus
}
#endif

#endif /* MACHINEEDITORVIEW_H */

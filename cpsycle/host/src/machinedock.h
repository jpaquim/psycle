// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net


#if !defined(MACHINEDOCK_H)
#define MACHINEDOCK_H

// host
#include "workspace.h"
#include "paramlistbox.h"
// ui
#include <uilabel.h>
#include <uiscrollbar.h>
// audio
#include <machine.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct MachineDockBox {
	// inherits
	psy_ui_Component component;
	// internal
	psy_ui_Label header;
	ParameterListBox parameters;
} MachineDockBox;

void machinedockbox_init(MachineDockBox*, psy_ui_Component* parent,
	uintptr_t slot, Workspace*);

typedef struct MachineDockPane {
	// inherit
	psy_ui_Component component;	
	// internal data
	psy_Table boxes;
	// references
	Workspace* workspace;
	psy_audio_Machines* machines;
} MachineDockPane;

void machinedockpane_init(MachineDockPane*, psy_ui_Component* parent, Workspace*);

typedef struct MachineDock {
	// inherit
	psy_ui_Component component;
	// internal
	// ui elements
	MachineDockPane pane;
	psy_ui_ScrollBar hscroll;
} MachineDock;

void machinedock_init(MachineDock*, psy_ui_Component* parent, Workspace*);

INLINE psy_ui_Component* machinedock_base(MachineDock* self)
{
	return &self->component;
}

#ifdef __cplusplus
}
#endif

#endif /* MACHINEDOCK_H */

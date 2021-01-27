// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#if !defined(MACHINEDOCK_H)
#define MACHINEDOCK_H

// host
#include "workspace.h"
#include "paramlistbox.h"
#include "tabbar.h"
// ui
#include <uibutton.h>
#include <uilabel.h>
#include <uiscrollbar.h>
// audio
#include <machine.h>

#ifdef __cplusplus
extern "C" {
#endif

// ParamRackBox

struct ParamRackBox;

typedef struct ParamRackBox {
	// inherits
	psy_ui_Component component;
	// internal
	psy_ui_Component header;
	psy_ui_Label title;
	psy_ui_Button inserteffect;
	ParameterListBox parameters;	
	psy_ui_Colour restorebgcolour;
	uintptr_t slot;
	// referenced
	Workspace* workspace;
	struct ParamRackBox* nextbox;
} ParamRackBox;

void paramrackbox_init(ParamRackBox*, psy_ui_Component* parent,
	uintptr_t slot, Workspace*);

void paramrackbox_select(ParamRackBox*);
void paramrackbox_deselect(ParamRackBox*);

// ParamRackMode
typedef enum ParamRackMode {
	MACHINEDOCK_ALL = 0,
	MACHINEDOCK_INPUTS = 1,
	MACHINEDOCK_OUTPUTS = 2,
	MACHINEDOCK_INCHAIN = 3,
	MACHINEDOCK_OUTCHAIN = 4
} ParamRackMode;

// ParamRackPane
typedef struct ParamRackPane {
	// inherit
	psy_ui_Component component;	
	// internal data
	psy_Table boxes;
	// references
	Workspace* workspace;
	psy_audio_Machines* machines;
	uintptr_t lastselected;
	ParamRackMode mode;
	ParamRackBox* lastinserted;
} ParamRackPane;

void paramrackpane_init(ParamRackPane*, psy_ui_Component* parent, Workspace*);

void paramrackpane_setmode(ParamRackPane*, ParamRackMode);

// ParamRack
typedef struct ParamRack {
	// inherit
	psy_ui_Component component;
	// internal
	// ui elements
	ParamRackPane pane;
	psy_ui_Component bottom;	
	TabBar modeselector;	
	psy_ui_ScrollBar hscroll;	
} ParamRack;

void paramrack_init(ParamRack*, psy_ui_Component* parent, Workspace*);

INLINE psy_ui_Component* paramrack_base(ParamRack* self)
{
	return &self->component;
}

#ifdef __cplusplus
}
#endif

#endif /* MACHINEDOCK_H */

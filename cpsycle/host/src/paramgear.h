// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#if !defined(PARAMRACK_H)
#define PARAMRACK_H

// host
#include "workspace.h"
#include "paramlistbox.h"
#include "intedit.h"
// ui
#include <uibutton.h>
#include <uilabel.h>
#include <uiscroller.h>
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
	// references
	Workspace* workspace;
	struct ParamRackBox* nextbox;
} ParamRackBox;

void paramrackbox_init(ParamRackBox*, psy_ui_Component* parent,
	uintptr_t slot, Workspace*);

void paramrackbox_select(ParamRackBox*);
void paramrackbox_deselect(ParamRackBox*);

// ParamRackMode
typedef enum ParamRackMode {
	PARAMRACK_NONE,
	PARAMRACK_ALL,
	PARAMRACK_INPUTS,
	PARAMRACK_OUTPUTS,
	PARAMRACK_INCHAIN,
	PARAMRACK_OUTCHAIN,
	PARAMRACK_LEVEL
} ParamRackMode;

typedef struct ParamRackBatchBar {
	// inherits
	psy_ui_Component component;
	// internal	
	psy_ui_Button solo;
	psy_ui_Button mute;
	psy_ui_Button remove;
	psy_ui_Button replace;
	psy_ui_Button select;
} ParamRackBatchBar;

void paramrackbatchbar_init(ParamRackBatchBar*, psy_ui_Component* parent);

typedef struct ParamRackModeBar {
	// inherits
	psy_ui_Component component;
	// signals
	psy_Signal signal_select;
	// internal	
	psy_ui_Button all;
	psy_ui_Button inputs;
	psy_ui_Button outputs;
	psy_ui_Button inchain;
	psy_ui_Button outchain;
	psy_ui_Button level;
} ParamRackModeBar;

void paramrackmodebar_init(ParamRackModeBar*, psy_ui_Component* parent);
void paramrackmodebar_setmode(ParamRackModeBar*, ParamRackMode);

// ParamRackPane
typedef struct ParamRackPane {
	// inherit
	psy_ui_Component component;	
	// internal
	psy_Table boxes;
	uintptr_t lastselected;
	ParamRackMode mode;
	uintptr_t level;
	// references
	Workspace* workspace;
	psy_audio_Machines* machines;	
	ParamRackBox* lastinserted;	
} ParamRackPane;

void paramrackpane_init(ParamRackPane*, psy_ui_Component* parent, Workspace*);

void paramrackpane_setmode(ParamRackPane*, ParamRackMode);

// ParamRack
typedef struct ParamRack {
	// inherit
	psy_ui_Component component;
	// internal
	ParamRackPane pane;
	psy_ui_Component bottom;
	ParamRackBatchBar batchbar;
	ParamRackModeBar modebar;	
	IntEdit leveledit;
	psy_ui_Scroller scroller;
	// references
	Workspace* workspace;
} ParamRack;

void paramrack_init(ParamRack*, psy_ui_Component* parent, Workspace*);

INLINE psy_ui_Component* paramrack_base(ParamRack* self)
{
	return &self->component;
}

#ifdef __cplusplus
}
#endif

#endif /* PARAMRACK_H */
